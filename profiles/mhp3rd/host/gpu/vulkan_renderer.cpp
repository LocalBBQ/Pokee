#include "vulkan_renderer.hpp"

#include "replacement_textures.hpp"
#include "texture_decode.hpp"
#include "triangle_indices.hpp"
#include "texture_pack.hpp"
#include "texture_pack_import.hpp"

#include "install/game_identity.hpp"
#include "install/user_data.hpp"

#include "perf/frame_stats.hpp"
#include "perf/perf_overlay.hpp"
#include "input/bindings.hpp"
#include "settings/settings.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <vulkan/vulkan.h>

#include "backends/imgui_impl_vulkan.h"
#include "imgui.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <set>
#include <vector>

namespace mhp3rd::gpu {
namespace {

constexpr std::uint32_t kPspWidth = 480u;
constexpr std::uint32_t kPspHeight = 272u;
// Auto resolution draws the game at the window's size in pixels, up to this
// many lines: 6 x 272, the largest multiple the menu offers.
constexpr std::uint32_t kMaxAutoLines = 6u * kPspHeight;
// Widest target, in pixels; every Vulkan device takes images this wide.
constexpr std::uint32_t kMaxTargetWidth = 8192u;
// Frames a new window size has to hold before the targets follow it, so
// dragging a window's edge does not rebuild them on every frame.
constexpr std::uint32_t kSettleFrames = 12u;
// A through-mode draw at least this wide (in PSP pixels) covers the screen:
// a fade, a backdrop or a copy of the picture, which spreads with the 3D view
// instead of keeping the interface's proportions.
constexpr float kScreenWideDraw = 470.0f;
constexpr VkDeviceSize kVertexBufferBytes = 16u * 1024u * 1024u;
constexpr std::size_t kMaxCachedTextures = 1024u;
// Descriptor sets for sampling render targets as textures: two per target
// (with its alpha, and with alpha forced to one for 5650 textures).
constexpr std::size_t kMaxFramebufferTextureSets = 64u;
// GPU timestamps: a pair around each command buffer a frame submits. A frame
// normally submits one; each GE block transfer that reads a framebuffer back
// splits it once more.
constexpr std::uint32_t kGpuTimerSegments = 16u;

// Compiled SPIR-V, generated from host/gpu/shaders by the build.
#include "ge_shaders.inc"

struct PushConstants {
    std::array<float, 16> transform{};
    std::array<float, 4> viewport{};        // x,y: target size; z: through flag; w: 1 fog + 2 lighting
    std::array<float, 4> texture_params{};  // x: enabled, y: function, z: alpha ref, w: alpha func
    std::array<float, 4> uv_transform{1.0f, 1.0f, 0.0f, 0.0f};
    std::array<float, 4> view_z{};          // row of view * world that gives view-space z, for fog
};

// 128 bytes is the most every Vulkan implementation has to accept.
static_assert(sizeof(PushConstants) == 128u, "PushConstants must fit the guaranteed push constant size");
constexpr float kPushFog = 1.0f;
constexpr float kPushLighting = 2.0f;

struct GpuVertex {
    float x{}, y{}, z{}, w{1.0f};
    float u{}, v{};
    std::uint32_t color{};
    float nx{}, ny{}, nz{};
};

constexpr float kNoClamp = 1e30f;

void set_uv_rect(GpuVertex &vertex, float u_min, float v_min, float u_max, float v_max) {
    vertex.nx = u_min;
    vertex.ny = v_min;
    vertex.nz = u_max;
    vertex.w = v_max;
}

// A through-mode tile is an axis-aligned rectangle of pixels showing an
// axis-aligned rectangle of texels, usually a piece of an atlas. At 480x272
// the raster samples the texels only between the centres of its edge pixels;
// at a higher internal resolution it also samples between those centres and
// the edges, where bilinear filtering pulls in the neighbouring atlas texels
// (or the far side of the texture) and leaves a faint grid along the tile
// edges. The six vertices from `first` get the texel range the 480x272
// raster samples, which the fragment shader clamps to. That range contains
// every sample a 480x272 target takes, so at x1 nothing changes.
void clamp_through_quad(std::vector<GpuVertex> &vertices, std::size_t first, float texture_width,
                        float texture_height) {
    if (first + 6u > vertices.size()) return;
    float x0 = vertices[first].x, x1 = x0, y0 = vertices[first].y, y1 = y0;
    for (std::size_t i = first; i < first + 6u; ++i) {
        x0 = std::min(x0, vertices[i].x);
        x1 = std::max(x1, vertices[i].x);
        y0 = std::min(y0, vertices[i].y);
        y1 = std::max(y1, vertices[i].y);
    }
    if (x1 - x0 < 1.0f || y1 - y0 < 1.0f) return;
    // Texture coordinates must follow x and y alone: one u at each vertical
    // edge, one v at each horizontal edge.
    float u_at_x0 = 0.0f, u_at_x1 = 0.0f, v_at_y0 = 0.0f, v_at_y1 = 0.0f;
    bool seen[4]{};
    for (std::size_t i = first; i < first + 6u; ++i) {
        const GpuVertex &vertex = vertices[i];
        const bool left = vertex.x == x0, top = vertex.y == y0;
        if (!left && vertex.x != x1) return;
        if (!top && vertex.y != y1) return;
        float &u = left ? u_at_x0 : u_at_x1;
        float &v = top ? v_at_y0 : v_at_y1;
        bool &u_seen = seen[left ? 0 : 1];
        bool &v_seen = seen[top ? 2 : 3];
        if (u_seen && u != vertex.u) return;
        if (v_seen && v != vertex.v) return;
        u = vertex.u;
        v = vertex.v;
        u_seen = v_seen = true;
    }
    const float u_min = std::min(u_at_x0, u_at_x1), u_max = std::max(u_at_x0, u_at_x1);
    const float v_min = std::min(v_at_y0, v_at_y1), v_max = std::max(v_at_y0, v_at_y1);
    // A range beyond the texture repeats it on purpose.
    if (u_min < 0.0f || v_min < 0.0f || u_max > texture_width || v_max > texture_height) return;
    // Half a pixel, in texels.
    const float inset_u = 0.5f * (u_max - u_min) / (x1 - x0);
    const float inset_v = 0.5f * (v_max - v_min) / (y1 - y0);
    for (std::size_t i = first; i < first + 6u; ++i)
        set_uv_rect(vertices[i], u_min + inset_u, v_min + inset_v, u_max - inset_u, v_max - inset_v);
}

// Lighting reaches the shaders in two std140 uniform blocks that live in the
// vertex buffer and are bound through dynamic offsets (set 1).
//
// The environment is what the game changes a few times a frame: the global
// ambient light, the four lights and the fog parameters. It is written once per
// change of GeState's environment version and shared by every draw after it.
struct EnvironmentBlock {
    std::array<float, 4> ambient{};
    std::array<float, 4> fog{};
    std::array<float, 4> fog_color{};
    std::array<std::array<float, 4>, 4> light_position{};
    std::array<std::array<float, 4>, 4> light_direction{};
    std::array<std::array<float, 4>, 4> light_attenuation{};
    std::array<std::array<float, 4>, 4> light_spot{};
    std::array<std::array<float, 4>, 4> light_ambient{};
    std::array<std::array<float, 4>, 4> light_diffuse{};
    std::array<std::array<float, 4>, 4> light_specular{};
};

static_assert(sizeof(EnvironmentBlock) == 496u, "EnvironmentBlock must match the std140 layout in ge.vert");

// What a lit draw adds: its world matrix and material. Consecutive draws of
// one mesh share it, so it is written only when it differs from the last one.
struct ObjectBlock {
    std::array<float, 16> world{};
    std::array<float, 4> flags{};             // y: vertex colour, w: material update mask
    std::array<float, 4> emissive{};          // w: specular power
    std::array<float, 4> material_ambient{};
    std::array<float, 4> material_diffuse{};  // w: separate specular
    std::array<float, 4> material_specular{}; // w: reverse normals
};

static_assert(sizeof(ObjectBlock) == 144u, "ObjectBlock must match the std140 layout in ge.vert");

// A GE colour register (0x00BBGGRR) as 0..1 floats, with an explicit alpha.
std::array<float, 4> unpack_color(std::uint32_t color, float alpha = 1.0f) {
    return {static_cast<float>(color & 0xFFu) / 255.0f, static_cast<float>((color >> 8u) & 0xFFu) / 255.0f,
            static_cast<float>((color >> 16u) & 0xFFu) / 255.0f, alpha};
}

// Pipeline variants the GE state can produce.
struct PipelineKey {
    bool blend{};
    std::uint32_t source_factor{};
    std::uint32_t destination_factor{};
    std::uint32_t equation{};
    bool depth_test{};
    bool depth_write{};
    std::uint32_t depth_function{};
    bool cull{};
    bool cull_clockwise{};
    std::uint32_t color_mask{VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
                             VK_COLOR_COMPONENT_A_BIT};

    auto operator<=>(const PipelineKey &) const = default;
};

// GU_FIX takes its factor from a colour register rather than from the source or
// destination pixel. Vulkan offers a single blend constant per attachment, so a
// fixed factor collapses to ONE or ZERO when the colour is white or black, and
// only the remaining cases need the constant itself.
constexpr std::uint32_t kFactorFixed = 10u;
constexpr std::uint32_t kFactorOne = 16u;
constexpr std::uint32_t kFactorZero = 17u;
constexpr std::uint32_t kFactorInverseConstant = 18u;

std::uint32_t resolve_fixed_factor(std::uint32_t factor, std::uint32_t color) {
    if (factor != kFactorFixed) return factor;
    const std::uint32_t rgb = color & 0x00FFFFFFu;
    if (rgb == 0x00FFFFFFu) return kFactorOne;
    if (rgb == 0u) return kFactorZero;
    return kFactorFixed;
}

VkBlendFactor to_blend_factor(std::uint32_t factor, bool source) {
    switch (factor) {
    // Factor 0 names the *other* pixel: the source side scales by the
    // destination colour and the destination side by the source colour.
    case 0u: return source ? VK_BLEND_FACTOR_DST_COLOR : VK_BLEND_FACTOR_SRC_COLOR;
    case 1u: return source ? VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR : VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
    case 2u: return VK_BLEND_FACTOR_SRC_ALPHA;
    case 3u: return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    case 4u: return VK_BLEND_FACTOR_DST_ALPHA;
    case 5u: return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
    case 6u: return VK_BLEND_FACTOR_SRC_ALPHA;             // doubled variants
    case 7u: return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    case 8u: return VK_BLEND_FACTOR_DST_ALPHA;
    case 9u: return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
    case kFactorFixed: return VK_BLEND_FACTOR_CONSTANT_COLOR;
    case kFactorOne: return VK_BLEND_FACTOR_ONE;
    case kFactorZero: return VK_BLEND_FACTOR_ZERO;
    case kFactorInverseConstant: return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
    default: return source ? VK_BLEND_FACTOR_ONE : VK_BLEND_FACTOR_ZERO;
    }
}

VkBlendOp to_blend_op(std::uint32_t equation) {
    switch (equation) {
    // GU_SUBTRACT is source minus destination; GU_REVERSE_SUBTRACT is the other
    // way round, and it is what darkening effects such as blob shadows use.
    case 1u: return VK_BLEND_OP_SUBTRACT;
    case 2u: return VK_BLEND_OP_REVERSE_SUBTRACT;
    case 3u: return VK_BLEND_OP_MIN;
    case 4u: return VK_BLEND_OP_MAX;
    default: return VK_BLEND_OP_ADD;
    }
}

VkCompareOp to_compare_op(std::uint32_t function) {
    switch (function) {
    case 0u: return VK_COMPARE_OP_NEVER;
    case 1u: return VK_COMPARE_OP_ALWAYS;
    case 2u: return VK_COMPARE_OP_EQUAL;
    case 3u: return VK_COMPARE_OP_NOT_EQUAL;
    case 4u: return VK_COMPARE_OP_LESS;
    case 5u: return VK_COMPARE_OP_LESS_OR_EQUAL;
    case 6u: return VK_COMPARE_OP_GREATER;
    default: return VK_COMPARE_OP_GREATER_OR_EQUAL;
    }
}

std::array<float, 16> multiply(const std::array<float, 16> &a, const std::array<float, 16> &b) {
    std::array<float, 16> result{};
    for (std::uint32_t column = 0; column < 4u; ++column) {
        for (std::uint32_t row = 0; row < 4u; ++row) {
            float sum = 0.0f;
            for (std::uint32_t k = 0; k < 4u; ++k) sum += a[k * 4u + row] * b[column * 4u + k];
            result[column * 4u + row] = sum;
        }
    }
    return result;
}

bool check(VkResult result, const char *what, std::string &error) {
    if (result == VK_SUCCESS) return true;
    error = std::string(what) + " failed with VkResult " + std::to_string(static_cast<int>(result));
    return false;
}

const char *present_mode_name(VkPresentModeKHR mode) {
    switch (mode) {
    case VK_PRESENT_MODE_IMMEDIATE_KHR: return "IMMEDIATE";
    case VK_PRESENT_MODE_MAILBOX_KHR: return "MAILBOX";
    case VK_PRESENT_MODE_FIFO_KHR: return "FIFO";
    case VK_PRESENT_MODE_FIFO_RELAXED_KHR: return "FIFO_RELAXED";
    default: return "OTHER";
    }
}

// Writes 4-byte pixels, top row first, as a 24-bit bottom-up BMP: no encoder
// needed and every viewer reads it.
bool write_bmp(const std::string &path, const std::uint8_t *pixels, std::uint32_t width, std::uint32_t height,
               bool bgra) {
    const std::uint32_t row_bytes = (width * 3u + 3u) & ~3u;
    const std::uint32_t image_bytes = row_bytes * height;
    std::vector<std::uint8_t> file(54u + image_bytes, 0u);
    const auto put32 = [&](std::size_t at, std::uint32_t value) {
        file[at] = static_cast<std::uint8_t>(value);
        file[at + 1u] = static_cast<std::uint8_t>(value >> 8u);
        file[at + 2u] = static_cast<std::uint8_t>(value >> 16u);
        file[at + 3u] = static_cast<std::uint8_t>(value >> 24u);
    };
    file[0] = 'B';
    file[1] = 'M';
    put32(2u, 54u + image_bytes);
    put32(10u, 54u);
    put32(14u, 40u);
    put32(18u, width);
    put32(22u, height);
    file[26] = 1u;
    file[28] = 24u;
    put32(34u, image_bytes);
    for (std::uint32_t y = 0; y < height; ++y) {
        const std::uint8_t *source = pixels + static_cast<std::size_t>(y) * width * 4u;
        std::uint8_t *destination = file.data() + 54u + static_cast<std::size_t>(height - 1u - y) * row_bytes;
        for (std::uint32_t x = 0; x < width; ++x) {
            destination[x * 3u + 0u] = source[x * 4u + (bgra ? 0u : 2u)];
            destination[x * 3u + 1u] = source[x * 4u + 1u];
            destination[x * 3u + 2u] = source[x * 4u + (bgra ? 2u : 0u)];
        }
    }
    std::ofstream out(path, std::ios::binary);
    if (!out) return false;
    out.write(reinterpret_cast<const char *>(file.data()), static_cast<std::streamsize>(file.size()));
    return static_cast<bool>(out);
}

struct PadTuning {
    float dead_zone{0.15f};
    float trigger{0.25f};
    settings::TriggerProfile triggers{settings::TriggerProfile::Standard};
    float right_stick{0.5f};
    settings::RightStick right_stick_mode{settings::RightStick::Camera};
    bool invert_x{};
    bool invert_y{};
    bool confirm_south{};
    bool trace{false};
};

// Read on every poll, so the in-game menu's changes apply at once.
PadTuning pad_tuning() {
    static const bool trace = std::getenv("MHP3RD_TRACE_PAD") != nullptr;
    const settings::Settings &player = settings::current();
    PadTuning value{};
    value.dead_zone = player.dead_zone;
    value.trigger = player.trigger;
    value.triggers = player.trigger_profile;
    value.right_stick = player.right_stick_zone;
    // The right stick is a real nub on this release, so driving the D-pad
    // from it as well would turn the camera twice.
    value.right_stick_mode = player.right_stick;
    value.invert_x = player.invert_camera_x;
    value.invert_y = player.invert_camera_y;
    // A PlayStation pad already carries the PSP's own face buttons, so the
    // positional mapping puts confirm on circle where the prompts want it.
    value.confirm_south = player.confirm_south;
    value.trace = trace;
    return value;
}

// Adds one gamepad's state to the pad bits and to the analog offsets the
// keyboard path also writes, so the two sources simply OR together.
void read_gamepad(SDL_Gamepad *device, PadState &pad, int &analog_x, int &analog_y) {
    const PadTuning tuning = pad_tuning();
    std::uint32_t &buttons = pad.buttons;
    const auto held = [&](SDL_GamepadButton button, std::uint32_t bit) {
        if (SDL_GetGamepadButton(device, button)) buttons |= bit;
    };
    held(SDL_GAMEPAD_BUTTON_DPAD_UP, 0x0010u);
    held(SDL_GAMEPAD_BUTTON_DPAD_RIGHT, 0x0020u);
    held(SDL_GAMEPAD_BUTTON_DPAD_DOWN, 0x0040u);
    held(SDL_GAMEPAD_BUTTON_DPAD_LEFT, 0x0080u);
    held(SDL_GAMEPAD_BUTTON_LEFT_SHOULDER, 0x0100u);
    held(SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER, 0x0200u);
    held(SDL_GAMEPAD_BUTTON_START, 0x0008u);
    held(SDL_GAMEPAD_BUTTON_BACK, 0x0001u);
    held(SDL_GAMEPAD_BUTTON_NORTH, 0x1000u);
    held(SDL_GAMEPAD_BUTTON_WEST, 0x8000u);
    // The game prompts "circle Enter / cross Back", and on a PlayStation pad
    // those are the same two buttons in the same two places.
    held(SDL_GAMEPAD_BUTTON_SOUTH, tuning.confirm_south ? 0x2000u : 0x4000u);
    held(SDL_GAMEPAD_BUTTON_EAST, tuning.confirm_south ? 0x4000u : 0x2000u);

    // The PSP triggers are digital, but hunters hold L for the camera all the
    // time, so the analog triggers press the same bits as the shoulders.
    const auto axis = [&](SDL_GamepadAxis id) {
        return std::clamp(static_cast<float>(SDL_GetGamepadAxis(device, id)) / 32767.0f, -1.0f, 1.0f);
    };
    // The trigger profiles copy other buttons for shooting: R on L2, where it
    // is held to aim, and the weapon's attack on R2 -- triangle for a bow,
    // circle for a bowgun. The buttons copied keep working.
    std::uint32_t left_trigger = 0x0100u;   // L
    std::uint32_t right_trigger = 0x0200u;  // R
    if (tuning.triggers == settings::TriggerProfile::Bows) {
        left_trigger = 0x0200u;
        right_trigger = 0x1000u;  // triangle
    } else if (tuning.triggers == settings::TriggerProfile::Bowguns) {
        left_trigger = 0x0200u;
        right_trigger = 0x2000u;  // circle
    }
    if (axis(SDL_GAMEPAD_AXIS_LEFT_TRIGGER) > tuning.trigger) buttons |= left_trigger;
    if (axis(SDL_GAMEPAD_AXIS_RIGHT_TRIGGER) > tuning.trigger) buttons |= right_trigger;

    const float right_x = axis(SDL_GAMEPAD_AXIS_RIGHTX);
    const float right_y = axis(SDL_GAMEPAD_AXIS_RIGHTY);
    // The HD release has its own right-stick camera, so the stick normally goes
    // there. Pressing the D-pad bits as well would turn the camera twice, hence
    // the either/or: the claw emulation is only for builds where that path is
    // not wanted.
    if (tuning.right_stick_mode == settings::RightStick::DPad) {
        if (right_x < -tuning.right_stick) buttons |= 0x0080u;
        if (right_x > tuning.right_stick) buttons |= 0x0020u;
        if (right_y < -tuning.right_stick) buttons |= 0x0010u;
        if (right_y > tuning.right_stick) buttons |= 0x0040u;
    }

    // Rescale the live range, otherwise leaving the dead zone snaps the stick
    // straight to a sixth of its travel.
    const auto deflect = [&](float x, float y, std::uint8_t &out_x, std::uint8_t &out_y) {
        const float length = std::sqrt(x * x + y * y);
        if (length <= tuning.dead_zone) return;
        const float scale = std::min((length - tuning.dead_zone) / (1.0f - tuning.dead_zone), 1.0f) / length;
        out_x = static_cast<std::uint8_t>(std::clamp(0x80 + static_cast<int>(x * scale * 127.0f), 0, 255));
        out_y = static_cast<std::uint8_t>(std::clamp(0x80 + static_cast<int>(y * scale * 127.0f), 0, 255));
    };
    if (tuning.right_stick_mode == settings::RightStick::Camera)
        deflect(tuning.invert_x ? -right_x : right_x, tuning.invert_y ? -right_y : right_y, pad.right_x, pad.right_y);

    const float left_x = axis(SDL_GAMEPAD_AXIS_LEFTX);
    const float left_y = axis(SDL_GAMEPAD_AXIS_LEFTY);
    std::uint8_t nub_x = 0x80u;
    std::uint8_t nub_y = 0x80u;
    deflect(left_x, left_y, nub_x, nub_y);
    analog_x += static_cast<int>(nub_x) - 0x80;
    analog_y += static_cast<int>(nub_y) - 0x80;
}

} // namespace

struct VulkanRenderer::Impl {
    struct Texture {
        VkImage image{};
        VkDeviceMemory memory{};
        VkImageView view{};
        VkDescriptorSet descriptor{};
        std::uint64_t last_used{};
        // The texture pack's image for this texture, looked up once when the
        // texture was uploaded; drawn instead once it is on the GPU.
        std::shared_ptr<Replacement> replacement;
        // How far down a 512-tall texture has been drawn, which decides how
        // much of it the pack's hash covers; see texture_pack.hpp.
        std::uint16_t max_seen_v{};
    };

    RendererConfig config;
    SDL_Window *window{};
    VkInstance instance{};
    VkSurfaceKHR surface{};
    VkPhysicalDevice physical_device{};
    VkDevice device{};
    std::uint32_t queue_family{};
    VkQueue queue{};
    VkSwapchainKHR swapchain{};
    VkFormat swapchain_format{VK_FORMAT_B8G8R8A8_UNORM};
    VkExtent2D swapchain_extent{};
    std::vector<VkImage> swapchain_images;
    VkCommandPool command_pool{};
    VkCommandBuffer command_buffer{};
    VkFence frame_fence{};
    // GPU time per frame (perf line): timestamps written at the start of each
    // command buffer of the frame and after its last draw, before the copy to
    // the window. Read after the frame fence; null when the queue cannot time
    // (timestampValidBits 0) or MHP3RD_NO_GPU_TIMESTAMPS is set.
    VkQueryPool gpu_timer{};
    double gpu_timer_ns_per_tick{};
    std::uint64_t gpu_timer_mask{};
    std::uint32_t gpu_timer_used{};     // queries written into this frame so far
    std::uint32_t gpu_timer_pending{};  // queries of the submitted frame, read at the next begin_frame
    bool gpu_timer_open{};
    void begin_gpu_segment(VkCommandBuffer commands) {
        if (gpu_timer == VK_NULL_HANDLE || gpu_timer_open || gpu_timer_used + 2u > 2u * kGpuTimerSegments) return;
        vkCmdWriteTimestamp(commands, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, gpu_timer, gpu_timer_used);
        gpu_timer_open = true;
    }
    void end_gpu_segment(VkCommandBuffer commands) {
        if (!gpu_timer_open) return;
        vkCmdWriteTimestamp(commands, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, gpu_timer, gpu_timer_used + 1u);
        gpu_timer_used += 2u;
        gpu_timer_open = false;
    }
    // After the fence of the frame that wrote them: adds that frame's GPU time.
    void collect_gpu_time() {
        if (gpu_timer == VK_NULL_HANDLE || gpu_timer_pending == 0u) return;
        std::array<std::uint64_t, 4u * kGpuTimerSegments> results{};
        const std::uint32_t count = gpu_timer_pending;
        gpu_timer_pending = 0u;
        const VkResult read = vkGetQueryPoolResults(
            device, gpu_timer, 0u, count, static_cast<std::size_t>(count) * 2u * sizeof(std::uint64_t),
            results.data(), 2u * sizeof(std::uint64_t),
            VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WITH_AVAILABILITY_BIT);
        if (read != VK_SUCCESS && read != VK_NOT_READY) return;
        std::uint64_t ticks = 0u;
        bool any = false;
        for (std::uint32_t pair = 0; pair + 1u < count; pair += 2u) {
            const std::uint64_t *begin = &results[pair * 2u];
            const std::uint64_t *end = &results[(pair + 1u) * 2u];
            if (begin[1] == 0u || end[1] == 0u) continue;  // not available
            ticks += (end[0] - begin[0]) & gpu_timer_mask;
            any = true;
        }
        if (any) perf::add_gpu_time(static_cast<double>(ticks) * gpu_timer_ns_per_tick / 1.0e6);
    }
    VkSemaphore image_available{};
    VkSemaphore render_finished{};
    VkPresentModeKHR present_mode{VK_PRESENT_MODE_FIFO_KHR};
    VkSurfaceFormatKHR surface_format{VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
    VkImageUsageFlags swapchain_usage{};
    std::vector<VkPresentModeKHR> present_modes;
    std::vector<VkImageView> swapchain_views;
    std::uint32_t swapchain_min_images{2u};
    // Set by a resize, a present mode change or an out-of-date swapchain;
    // the swapchain is rebuilt before the next acquire.
    bool swapchain_dirty{};

    // Display settings.
    settings::PresentMode requested_present{settings::PresentMode::Fifo};
    settings::Aspect aspect{settings::Aspect::Original};
    std::uint32_t requested_scale{2u};  // multiples of 480x272; 0: the window's size
    // A target size the window asks for, applied once it has held for
    // kSettleFrames frames; a changed setting applies at the next chance.
    VkExtent2D pending_extent{};
    std::uint32_t pending_frames{};
    bool resize_now{};
    // The framebuffers the game showed last: its interface is drawn into them.
    std::array<std::uint32_t, 2> display_addresses{};
    bool sharp_screen{};
    bool sharp_textures{};
    std::string device_name;

    // Dear ImGui draws in its own render pass over the finished swapchain
    // image, after the game frame and the performance overlay.
    VkRenderPass ui_render_pass{};
    std::vector<VkFramebuffer> ui_framebuffers;
    bool ui_ready{};
    ImDrawData *ui_draw_data{};
    std::function<bool(const SDL_Event &)> event_hook;
    bool game_input{true};
    bool suppress_held{};
    std::uint32_t suppressed_buttons{};
    // Keyboard and mouse (input/bindings.hpp). Mouse buttons are followed
    // through their events, so a scripted click counts like a real one.
    bool pointer_free{};
    bool scripted_input{};
    bool mouse_captured{};
    std::uint32_t mouse_buttons{};  // bit n: SDL mouse button n held
    MouseMotion mouse_motion{};
    std::array<bool, input::kKeyPositions> scripted_keys{};

    // Captures the pointer for the game when everything allows it and frees
    // it otherwise. Whatever the mouse did or held across a change is dropped,
    // so nothing stays pressed and the camera does not jump.
    void update_pointer(bool focused) {
        const bool minimized = (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED) != 0u;
        const bool wanted = settings::current().mouse && game_input && !pointer_free && !minimized &&
                            (focused || scripted_input);
        if (wanted == mouse_captured) return;
        mouse_captured = wanted;
        // A scripted run never takes the real pointer from the person at the machine.
        if (!scripted_input) SDL_SetWindowRelativeMouseMode(window, wanted);
        if (pad_tuning().trace) std::cout << "[pad] pointer " << (wanted ? "captured" : "free") << std::endl;
        mouse_buttons = 0u;
        mouse_motion = {};
        if (wanted) suppress_held = true;
    }

    // Window capture: the presented image is copied here and written after
    // its frame completes.
    std::string capture_path;
    VkBuffer capture_buffer{};
    VkDeviceMemory capture_memory{};
    VkExtent2D capture_extent{};
    bool capture_recorded{};

    // Performance overlay: drawn on the CPU, copied through a mapped staging
    // buffer into a small image and scaled onto the swapchain image after the
    // game frame, so screenshots of the window include it.
    bool overlay_visible{};
    bool overlay_ready{};
    VkImage overlay_image{};
    VkDeviceMemory overlay_memory{};
    VkImageView overlay_view{};
    VkBuffer overlay_staging{};
    VkDeviceMemory overlay_staging_memory{};
    void *overlay_mapped{};
    std::vector<std::uint32_t> overlay_pixels;

    // Frames the game writes to memory without the GE (upload_frame): copied
    // through a mapped staging buffer into an image the size of the frame,
    // then scaled into the target of the address the game shows.
    VkImage upload_image{};
    VkDeviceMemory upload_memory{};
    VkImageView upload_view{};
    VkExtent2D upload_extent{};
    VkBuffer upload_staging{};
    VkDeviceMemory upload_staging_memory{};
    void *upload_mapped{};
    void destroy_upload();
    bool create_upload(std::uint32_t width, std::uint32_t height, std::string &error);

    // One offscreen target per guest framebuffer address. The game draws into
    // several (double buffering, render to texture), and only the address passed
    // to sceDisplaySetFrameBuf is shown.
    struct Target {
        VkImage color{};
        VkDeviceMemory color_memory{};
        VkImageView color_view{};
        VkImage depth{};
        VkDeviceMemory depth_memory{};
        VkImageView depth_view{};
        VkFramebuffer framebuffer{};
        bool initialized{};
        // The guest's view of the buffer when it was last drawn to: row length
        // in pixels and pixel format (0:5650 1:5551 2:4444 3:8888).
        std::uint32_t stride{512u};
        std::uint32_t format{3u};
        std::uint64_t last_drawn_frame{};
        // Bumped by every draw into the target, so a copy made for sampling
        // knows when it is out of date.
        std::uint64_t draw_serial{};
        // One guest word from every 256 bytes of the buffer, read when it was
        // last drawn to. The renderer never writes guest memory, so a word that
        // has changed since means the game put something else there.
        std::vector<std::uint32_t> guest_words;
        // The copy that draws sample when the game textures from this buffer:
        // a render pass cannot read its own attachment.
        VkImage copy{};
        VkDeviceMemory copy_memory{};
        VkImageView copy_view{};
        VkImageView copy_opaque_view{};
        std::array<VkDescriptorSet, 2> copy_descriptors{};
        std::uint64_t copy_serial{};
        bool copy_valid{};
    };
    std::map<std::uint32_t, Target> targets;
    // Write-back of the displayed framebuffer to guest VRAM (write_back_frame):
    // present() scales the target to 480x272 and copies it into a mapped
    // buffer; begin_frame(), after the frame fence, takes the pixels; the next
    // write_back_frame() stores them in the guest's format.
    VkImage writeback_image{};
    VkDeviceMemory writeback_memory{};
    VkImageView writeback_view{};
    VkBuffer writeback_buffer{};
    VkDeviceMemory writeback_buffer_memory{};
    void *writeback_mapped{};
    struct WritebackFrame {
        std::uint32_t address{};
        std::uint32_t stride{};
        std::uint32_t format{};
    };
    WritebackFrame writeback_recorded{};  // copied by the frame in flight
    bool writeback_in_flight{};
    WritebackFrame writeback_ready{};     // pixels waiting in writeback_pixels
    bool writeback_has_pixels{};
    std::vector<std::uint32_t> writeback_pixels;
    bool create_writeback(std::string &error);
    void destroy_writeback();
    void record_writeback(std::uint32_t address);
    // Stores 480x272 RGBA pixels into the guest framebuffer `frame` describes.
    void store_frame(GuestMemory &memory, const WritebackFrame &frame, const std::uint32_t *pixels);
    // Numbers draws into targets across all of them, for Target::draw_serial.
    std::uint64_t target_draw_counter{};
    std::uint32_t current_target{};
    std::uint32_t last_drawn_target{};
    std::uint32_t presented_target{};
    // A copy of the frame shown when hold_frame() began; only its colour
    // image is used.
    Target held{};
    bool holding{};
    // Per-frame tally, so "no 3D" can be told from "3D drawn somewhere else".
    std::uint32_t frame_through_draws{};
    std::uint32_t frame_transformed_draws{};
    std::uint32_t frame_transformed_vertices{};
    std::uint32_t frame_onscreen_vertices{};
    std::uint32_t frame_behind_camera{};
    std::array<float, 3> frame_ndc_min{1e30f, 1e30f, 1e30f};
    std::array<float, 3> frame_ndc_max{-1e30f, -1e30f, -1e30f};
    std::map<std::uint32_t, std::uint32_t> frame_transformed_targets;
    // MHP3RD_TRACE_CAMERA: the view matrices this frame's transformed draws
    // used and how many vertices each of them covered. A frame holds a handful
    // (the scene, a reflection, a shadow pass), and the busiest one is the
    // camera the player sees, so the frame's line can report that one. Only
    // filled while the trace is on.
    std::vector<std::pair<std::array<float, 16>, std::uint32_t>> frame_views;
    // The yaw of the previous traced frame, so each line can carry the turn.
    float traced_yaw{};
    CameraReading reading{};
    bool pass_active{};
    VkRenderPass render_pass{};
    VkExtent2D target_extent{};

    VkShaderModule vertex_shader{};
    VkShaderModule fragment_shader{};
    VkPipelineLayout pipeline_layout{};
    VkDescriptorSetLayout descriptor_layout{};
    VkDescriptorPool descriptor_pool{};
    VkDescriptorSetLayout lighting_layout{};
    VkDescriptorSet lighting_descriptor{};  // binding 0: environment, binding 1: object
    VkDeviceSize uniform_alignment{256u};
    VkSampler sampler{};        // linear
    VkSampler sharp_sampler{};  // nearest, for the sharp texture setting
    // The same, clamped to the edge, for render targets sampled as textures:
    // the game's texture is usually larger than the 480x272 the target holds.
    VkSampler clamp_sampler{};
    VkSampler clamp_sharp_sampler{};
    std::map<PipelineKey, VkPipeline> pipelines;
    // The pipeline of the previous lookup: consecutive draws mostly share it.
    // MHP3RD_NO_LOOKUP_CACHE looks every draw up in the map, as before.
    PipelineKey last_pipeline_key{};
    VkPipeline last_pipeline{};

    VkBuffer vertex_buffer{};
    VkDeviceMemory vertex_memory{};
    void *vertex_mapped{};
    VkDeviceSize vertex_offset{};
    // The lighting blocks most recently written this frame, reused while the
    // state stays the same; begin_frame() drops them with the vertex buffer.
    std::uint64_t environment_version{};    // 0: none written this frame
    std::uint32_t environment_offset{};
    ObjectBlock last_object{};
    std::uint32_t object_offset{};
    bool object_valid{};
    // The material colours of the last lit draw, by GeState's material version.
    std::uint64_t material_version{};       // 0: none unpacked yet
    std::array<std::array<float, 4>, 4> material{};
    // What the command buffer has bound, so that unchanged state is not bound
    // again; begin_frame() and begin_pass() forget it.
    VkPipeline bound_pipeline{};
    std::array<std::uint32_t, 2> bound_lighting_offsets{};
    bool lighting_bound{};

    // Copies a uniform block into the vertex buffer at the next aligned offset.
    // Returns false, writing nothing, when the buffer is full.
    bool write_uniform(const void *data, std::size_t size, std::uint32_t &offset) {
        const VkDeviceSize at = (vertex_offset + uniform_alignment - 1u) / uniform_alignment * uniform_alignment;
        if (at + size > kVertexBufferBytes) return false;
        std::memcpy(static_cast<std::uint8_t *>(vertex_mapped) + at, data, size);
        offset = static_cast<std::uint32_t>(at);
        vertex_offset = at + size;
        return true;
    }
    void forget_bindings() {
        bound_pipeline = VK_NULL_HANDLE;
        lighting_bound = false;
    }

    Texture white_texture{};
    std::map<std::uint64_t, Texture> textures;
    // HD texture pack (texture_pack.hpp). `pack` is null while the setting
    // is off or no pack is installed; turning the setting on or off takes
    // effect at the next begin_frame().
    std::unique_ptr<TexturePack> pack;
    std::unique_ptr<TextureDumper> dumper;
    ReplacementTextures replacements;
    bool pack_wanted{};
    bool pack_applied{};
    bool pack_held{};    // an import is swapping the pack's folder: none is open
    bool pack_reload{};  // open the pack again, e.g. after an import
    TexturePackLocation pack_location;
    std::string pack_status;
    std::uint64_t replaced_draws{};  // this second, for MHP3RD_TRACE_TEXTURE_PACK
    void apply_texture_pack();
    [[nodiscard]] VkDescriptorSet texture_descriptor(const GuestMemory &memory, const DrawCall &call);
    std::uint64_t texture_clock{};
    // texture_key results for the display list being walked, by the state that
    // feeds the key; cleared by begin_display_list().
    struct TextureKeyInput {
        std::uint32_t address{};
        std::uint32_t buffer_width{};
        std::uint32_t size{};
        std::uint32_t format{};
        std::uint32_t clut_address{};
        std::uint32_t clut_format{};
        bool swizzled{};
        auto operator<=>(const TextureKeyInput &) const = default;
    };
    // The texture each input resolved to, while `textures_erased` has not
    // moved since: erasing is the only change that moves or frees a cached
    // texture. The previous draw's entry is kept at hand, as consecutive draws
    // mostly sample the same texture.
    struct ListTexture {
        std::uint64_t key{};
        Texture *texture{};
        std::uint64_t erased{};
    };
    std::map<TextureKeyInput, ListTexture> list_texture_keys;
    std::uint64_t textures_erased{};
    TextureKeyInput last_texture_input{};
    ListTexture *last_texture{};

    std::vector<GpuVertex> scratch;
    // Index list of a draw whose decoded vertices go straight into the vertex
    // buffer (see submit()).
    std::vector<std::uint16_t> direct_indices;
    // MHP3RD_CHECK_DIRECT_VERTICES: draws compared with the expansion, and
    // those that differed.
    std::uint64_t direct_checked{};
    std::uint64_t direct_mismatched{};
    PadState pad{};
    SDL_Gamepad *gamepad{};
    SDL_JoystickID gamepad_id{};
    bool recording{};
    bool quit{};
    bool ready{};
    std::uint64_t frames{};
    std::uint64_t draws{};

    // Only the first pad is used; a second one arriving is ignored rather than
    // stealing the stick from whoever is already playing.
    void open_gamepad(SDL_JoystickID id) {
        if (gamepad != nullptr) {
            // The virtual pad of MHP3RD_INPUT_SCRIPT takes over from a real
            // one, so a controller within reach does not steal a scripted run.
            const char *name = SDL_GetGamepadNameForID(id);
            if (name == nullptr || std::strcmp(name, "Yakumo input script") != 0) return;
            SDL_CloseGamepad(gamepad);
            gamepad = nullptr;
        }
        SDL_Gamepad *device = SDL_OpenGamepad(id);
        if (device == nullptr) {
            std::cout << "[pad] SDL_OpenGamepad failed: " << SDL_GetError() << "\n";
            return;
        }
        gamepad = device;
        gamepad_id = id;
        const char *name = SDL_GetGamepadName(device);
        const PadTuning tuning = pad_tuning();
        std::cout << "[pad] " << (name != nullptr ? name : "gamepad") << " connected; confirm on "
                  << (tuning.confirm_south ? "the south button" : "circle") << ", right stick "
                  << (tuning.right_stick_mode == settings::RightStick::DPad     ? "as D-pad"
                      : tuning.right_stick_mode == settings::RightStick::Camera ? "as camera"
                                                                                : "off")
                  << "\n";
    }

    // Picks up a pad that was already plugged in before the window existed, and
    // falls back to a still-connected second pad when the first one is unplugged.
    void scan_gamepads() {
        int count = 0;
        SDL_JoystickID *ids = SDL_GetGamepads(&count);
        if (ids != nullptr) {
            for (int i = 0; i < count && gamepad == nullptr; ++i) open_gamepad(ids[i]);
            SDL_free(ids);
        }
        if (gamepad != nullptr) return;
        // Say why there is no pad rather than staying silent: a stick with no
        // entry in SDL's mapping database enumerates as a joystick only, which
        // looks identical to "nothing plugged in" from the player's side.
        int joysticks = 0;
        SDL_JoystickID *sticks = SDL_GetJoysticks(&joysticks);
        if (sticks != nullptr) {
            for (int i = 0; i < joysticks; ++i) {
                if (SDL_IsGamepad(sticks[i])) continue;
                const char *name = SDL_GetJoystickNameForID(sticks[i]);
                std::cout << "[pad] " << (name != nullptr ? name : "joystick")
                          << " has no gamepad mapping, ignored\n";
            }
            SDL_free(sticks);
        }
        if (joysticks == 0) std::cout << "[pad] no gamepad connected, keyboard only\n";
    }

    void close_gamepad(SDL_JoystickID id) {
        if (gamepad == nullptr || id != gamepad_id) return;
        SDL_CloseGamepad(gamepad);
        gamepad = nullptr;
        gamepad_id = 0;
        std::cout << "[pad] gamepad disconnected\n";
        scan_gamepads();
    }

    [[nodiscard]] std::uint32_t find_memory_type(std::uint32_t mask, VkMemoryPropertyFlags properties) const {
        VkPhysicalDeviceMemoryProperties memory_properties{};
        vkGetPhysicalDeviceMemoryProperties(physical_device, &memory_properties);
        for (std::uint32_t i = 0; i < memory_properties.memoryTypeCount; ++i) {
            if ((mask & (1u << i)) != 0u &&
                (memory_properties.memoryTypes[i].propertyFlags & properties) == properties)
                return i;
        }
        return 0u;
    }

    bool create_image(std::uint32_t width, std::uint32_t height, VkFormat format, VkImageUsageFlags usage,
                      VkImage &image, VkDeviceMemory &memory, VkImageView &view, VkImageAspectFlags aspect,
                      std::string &error) {
        VkImageCreateInfo info{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
        info.imageType = VK_IMAGE_TYPE_2D;
        info.format = format;
        info.extent = {width, height, 1u};
        info.mipLevels = 1u;
        info.arrayLayers = 1u;
        info.samples = VK_SAMPLE_COUNT_1_BIT;
        info.tiling = VK_IMAGE_TILING_OPTIMAL;
        info.usage = usage;
        info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        if (!check(vkCreateImage(device, &info, nullptr, &image), "vkCreateImage", error)) return false;

        VkMemoryRequirements requirements{};
        vkGetImageMemoryRequirements(device, image, &requirements);
        VkMemoryAllocateInfo allocate{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
        allocate.allocationSize = requirements.size;
        allocate.memoryTypeIndex = find_memory_type(requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        if (!check(vkAllocateMemory(device, &allocate, nullptr, &memory), "vkAllocateMemory", error)) return false;
        vkBindImageMemory(device, image, memory, 0u);

        VkImageViewCreateInfo view_info{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
        view_info.image = image;
        view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        view_info.format = format;
        view_info.subresourceRange = {aspect, 0u, 1u, 0u, 1u};
        return check(vkCreateImageView(device, &view_info, nullptr, &view), "vkCreateImageView", error);
    }

    void transition(VkCommandBuffer commands, VkImage image, VkImageLayout from, VkImageLayout to,
                    VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT) {
        VkImageMemoryBarrier barrier{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
        barrier.oldLayout = from;
        barrier.newLayout = to;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange = {aspect, 0u, 1u, 0u, 1u};
        barrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
        vkCmdPipelineBarrier(commands, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0u,
                             0u, nullptr, 0u, nullptr, 1u, &barrier);
    }

    [[nodiscard]] VkSampler texture_sampler() const { return sharp_textures ? sharp_sampler : sampler; }
    [[nodiscard]] VkSampler framebuffer_sampler() const {
        return sharp_textures ? clamp_sharp_sampler : clamp_sampler;
    }
    [[nodiscard]] VkPresentModeKHR wanted_present_mode() const;
    bool create_swapchain(std::string &error);
    void destroy_swapchain_views();
    void recreate_swapchain();
    bool create_ui_framebuffers(std::string &error);
    void record_game_blit(VkImage source, VkImage destination);
    // Target size for the current settings and window.
    [[nodiscard]] VkExtent2D wanted_target_extent() const;
    // Rebuilds every target at `extent`, its picture scaled into it. Not while
    // a frame is being recorded.
    void resize_targets(VkExtent2D extent);
    // After a present: follows the window's size or a changed setting.
    void follow_window();
    // Fill makes every one of the game's 480x272 pixels wider (or taller)
    // than square. The interface is drawn at `fit_x` x `fit_y` of its size
    // about the screen's centre, which gives it square pixels again. False
    // when nothing needs fitting.
    [[nodiscard]] bool interface_fit(float &fit_x, float &fit_y) const;
    [[nodiscard]] bool shows(std::uint32_t address) const {
        return address != 0u && (address == display_addresses[0] || address == display_addresses[1]);
    }
    void submit_and_present(VkImage source, bool game_frame);
    void write_capture();
    void destroy_target(Target &target);
    void run_commands(const std::function<void(VkCommandBuffer)> &record);
    Target *target_for(std::uint32_t address, std::string &error);
    bool create_overlay(std::string &error);
    void record_overlay(VkImage destination);
    void update_display_info();
    void begin_pass(std::uint32_t address);
    void end_pass();
    VkPipeline pipeline_for(const PipelineKey &key);
    Texture &texture_for(const GuestMemory &memory, const DrawCall &call);
    void trace_framebuffer_texture(const DrawCall &call);
    // A render target the texture reads, with the texture's first texel as a
    // pixel position inside it; see framebuffer_texture().
    struct FramebufferTexture {
        Target *target{};
        std::uint32_t x{};
        std::uint32_t y{};
    };
    [[nodiscard]] FramebufferTexture find_framebuffer_texture(const GuestMemory &memory,
                                                              const TextureState &texture);
    VkDescriptorSet framebuffer_descriptor(Target &target, bool opaque);
    void snapshot_guest_words(const GuestMemory &memory, std::uint32_t address, Target &target);
    Texture create_texture(std::uint32_t width, std::uint32_t height, const std::uint32_t *pixels);
    void destroy_texture(Texture &texture);
};

VulkanRenderer::VulkanRenderer() : impl_(std::make_unique<Impl>()) {}
VulkanRenderer::~VulkanRenderer() { shutdown(); }

bool VulkanRenderer::available() const noexcept { return impl_ && impl_->ready; }
bool VulkanRenderer::quit_requested() const noexcept { return impl_ && impl_->quit; }
std::uint64_t VulkanRenderer::frames_presented() const noexcept { return impl_ ? impl_->frames : 0u; }
CameraReading VulkanRenderer::camera() const noexcept { return impl_ ? impl_->reading : CameraReading{}; }
std::uint64_t VulkanRenderer::draws_submitted() const noexcept { return impl_ ? impl_->draws : 0u; }

bool VulkanRenderer::initialize(const RendererConfig &config, std::string &error) {
    Impl &impl = *impl_;
    impl.config = config;
    const settings::Settings &player = settings::current();
    impl.requested_scale = std::min(player.internal_scale, settings::kMaxInternalScale);
    // Until the window's size is known, which Auto and Fill need.
    const std::uint32_t scale = impl.requested_scale != 0u ? impl.requested_scale : 2u;
    impl.target_extent = {kPspWidth * scale, kPspHeight * scale};
    impl.requested_present = player.present_mode;
    impl.aspect = player.aspect;
    impl.sharp_screen = player.sharp_screen;
    impl.sharp_textures = player.sharp_textures;
    const std::uint32_t window_scale = std::clamp<std::uint32_t>(player.window_scale, 1u, settings::kMaxWindowScale);

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        error = std::string("SDL_Init failed: ") + SDL_GetError();
        return false;
    }
    // A missing gamepad subsystem is not fatal; the keyboard still drives the pad.
    if (!SDL_InitSubSystem(SDL_INIT_GAMEPAD)) std::cout << "[pad] no gamepad support: " << SDL_GetError() << "\n";
    else impl.scan_gamepads();
    SDL_WindowFlags window_flags = SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE;
    if (player.fullscreen) window_flags |= SDL_WINDOW_FULLSCREEN;
    impl.window = SDL_CreateWindow(config.title.c_str(), static_cast<int>(kPspWidth * window_scale),
                                   static_cast<int>(kPspHeight * window_scale), window_flags);
    if (impl.window == nullptr) {
        error = std::string("SDL_CreateWindow failed: ") + SDL_GetError();
        return false;
    }

    std::uint32_t extension_count = 0u;
    const char *const *sdl_extensions = SDL_Vulkan_GetInstanceExtensions(&extension_count);
    std::vector<const char *> extensions(sdl_extensions, sdl_extensions + extension_count);
    extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);

    VkApplicationInfo application{VK_STRUCTURE_TYPE_APPLICATION_INFO};
    application.pApplicationName = "Yakumo";
    application.apiVersion = VK_API_VERSION_1_1;
    VkInstanceCreateInfo instance_info{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    instance_info.pApplicationInfo = &application;
    instance_info.enabledExtensionCount = static_cast<std::uint32_t>(extensions.size());
    instance_info.ppEnabledExtensionNames = extensions.data();
    // MoltenVK reports itself as a portability driver and refuses the instance
    // without this flag.
    instance_info.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    if (!check(vkCreateInstance(&instance_info, nullptr, &impl.instance), "vkCreateInstance", error)) return false;

    if (!SDL_Vulkan_CreateSurface(impl.window, impl.instance, nullptr, &impl.surface)) {
        error = std::string("SDL_Vulkan_CreateSurface failed: ") + SDL_GetError();
        return false;
    }

    std::uint32_t device_count = 0u;
    vkEnumeratePhysicalDevices(impl.instance, &device_count, nullptr);
    std::vector<VkPhysicalDevice> devices(device_count);
    vkEnumeratePhysicalDevices(impl.instance, &device_count, devices.data());
    if (devices.empty()) {
        error = "no Vulkan device found";
        return false;
    }
    impl.physical_device = devices.front();
    for (VkPhysicalDevice candidate : devices) {
        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(candidate, &properties);
        if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            impl.physical_device = candidate;
            break;
        }
    }

    std::uint32_t family_count = 0u;
    vkGetPhysicalDeviceQueueFamilyProperties(impl.physical_device, &family_count, nullptr);
    std::vector<VkQueueFamilyProperties> families(family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(impl.physical_device, &family_count, families.data());
    bool found_family = false;
    for (std::uint32_t i = 0; i < family_count; ++i) {
        VkBool32 present = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(impl.physical_device, i, impl.surface, &present);
        if ((families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0u && present == VK_TRUE) {
            impl.queue_family = i;
            found_family = true;
            break;
        }
    }
    if (!found_family) {
        error = "no graphics queue with presentation support";
        return false;
    }

    std::uint32_t device_extension_count = 0u;
    vkEnumerateDeviceExtensionProperties(impl.physical_device, nullptr, &device_extension_count, nullptr);
    std::vector<VkExtensionProperties> device_extensions(device_extension_count);
    vkEnumerateDeviceExtensionProperties(impl.physical_device, nullptr, &device_extension_count,
                                         device_extensions.data());
    std::vector<const char *> enabled_device_extensions{VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    for (const VkExtensionProperties &extension : device_extensions) {
        if (std::strcmp(extension.extensionName, "VK_KHR_portability_subset") == 0)
            enabled_device_extensions.push_back("VK_KHR_portability_subset");
    }

    const float priority = 1.0f;
    VkDeviceQueueCreateInfo queue_info{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
    queue_info.queueFamilyIndex = impl.queue_family;
    queue_info.queueCount = 1u;
    queue_info.pQueuePriorities = &priority;
    VkDeviceCreateInfo device_info{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
    device_info.queueCreateInfoCount = 1u;
    device_info.pQueueCreateInfos = &queue_info;
    device_info.enabledExtensionCount = static_cast<std::uint32_t>(enabled_device_extensions.size());
    device_info.ppEnabledExtensionNames = enabled_device_extensions.data();
    if (!check(vkCreateDevice(impl.physical_device, &device_info, nullptr, &impl.device), "vkCreateDevice", error))
        return false;
    vkGetDeviceQueue(impl.device, impl.queue_family, 0u, &impl.queue);

    // Swapchain.
    std::uint32_t format_count = 0u;
    vkGetPhysicalDeviceSurfaceFormatsKHR(impl.physical_device, impl.surface, &format_count, nullptr);
    std::vector<VkSurfaceFormatKHR> formats(format_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(impl.physical_device, impl.surface, &format_count, formats.data());
    // The GE's colours are already gamma-encoded, so they must reach the display
    // unchanged: prefer a plain 8-bit UNORM format. Gamescope (Steam Deck Game
    // Mode) lists an _SRGB format first, and presenting through it encodes the
    // colours a second time and washes the picture out.
    if (!formats.empty()) {
        impl.surface_format = formats.front();
        for (const VkSurfaceFormatKHR &candidate : formats) {
            if ((candidate.format == VK_FORMAT_B8G8R8A8_UNORM || candidate.format == VK_FORMAT_R8G8B8A8_UNORM) &&
                candidate.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                impl.surface_format = candidate;
                break;
            }
        }
    }
    impl.swapchain_format = impl.surface_format.format;
    std::uint32_t mode_count = 0u;
    vkGetPhysicalDeviceSurfacePresentModesKHR(impl.physical_device, impl.surface, &mode_count, nullptr);
    impl.present_modes.resize(mode_count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(impl.physical_device, impl.surface, &mode_count,
                                              impl.present_modes.data());
    if (!impl.create_swapchain(error)) return false;
    // No target exists yet: they are made at this size when first drawn.
    impl.target_extent = impl.wanted_target_extent();

    std::array<VkAttachmentDescription, 2> attachments{};
    attachments[0].format = VK_FORMAT_R8G8B8A8_UNORM;
    attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachments[1] = attachments[0];
    attachments[1].format = VK_FORMAT_D32_SFLOAT;
    attachments[1].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    VkAttachmentReference color_reference{0u, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    VkAttachmentReference depth_reference{1u, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};
    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1u;
    subpass.pColorAttachments = &color_reference;
    subpass.pDepthStencilAttachment = &depth_reference;
    VkRenderPassCreateInfo render_pass_info{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
    render_pass_info.attachmentCount = static_cast<std::uint32_t>(attachments.size());
    render_pass_info.pAttachments = attachments.data();
    render_pass_info.subpassCount = 1u;
    render_pass_info.pSubpasses = &subpass;
    if (!check(vkCreateRenderPass(impl.device, &render_pass_info, nullptr, &impl.render_pass), "vkCreateRenderPass",
               error))
        return false;

    // Shaders, descriptors and pipeline layout.
    const auto create_shader = [&](const std::uint32_t *code, std::size_t size, VkShaderModule &module) {
        VkShaderModuleCreateInfo info{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
        info.codeSize = size;
        info.pCode = code;
        return check(vkCreateShaderModule(impl.device, &info, nullptr, &module), "vkCreateShaderModule", error);
    };
    if (!create_shader(kGeVertexShader, sizeof(kGeVertexShader), impl.vertex_shader)) return false;
    if (!create_shader(kGeFragmentShader, sizeof(kGeFragmentShader), impl.fragment_shader)) return false;

    VkDescriptorSetLayoutBinding binding{};
    binding.binding = 0u;
    binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    binding.descriptorCount = 1u;
    binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    VkDescriptorSetLayoutCreateInfo layout_info{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    layout_info.bindingCount = 1u;
    layout_info.pBindings = &binding;
    if (!check(vkCreateDescriptorSetLayout(impl.device, &layout_info, nullptr, &impl.descriptor_layout),
               "vkCreateDescriptorSetLayout", error))
        return false;

    // Set 1: the lighting environment and the lit object, windows into the
    // vertex buffer.
    std::array<VkDescriptorSetLayoutBinding, 2> lighting_bindings{};
    lighting_bindings[0].binding = 0u;
    lighting_bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    lighting_bindings[0].descriptorCount = 1u;
    lighting_bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    lighting_bindings[1].binding = 1u;
    lighting_bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    lighting_bindings[1].descriptorCount = 1u;
    lighting_bindings[1].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    VkDescriptorSetLayoutCreateInfo lighting_layout_info{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    lighting_layout_info.bindingCount = static_cast<std::uint32_t>(lighting_bindings.size());
    lighting_layout_info.pBindings = lighting_bindings.data();
    if (!check(vkCreateDescriptorSetLayout(impl.device, &lighting_layout_info, nullptr, &impl.lighting_layout),
               "vkCreateDescriptorSetLayout", error))
        return false;

    const std::array<VkDescriptorPoolSize, 2> pool_sizes{
        VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                             static_cast<std::uint32_t>(kMaxCachedTextures + 1u + kMaxFramebufferTextureSets)},
        VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 2u},
    };
    VkDescriptorPoolCreateInfo pool_info{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = static_cast<std::uint32_t>(kMaxCachedTextures + 2u + kMaxFramebufferTextureSets);
    pool_info.poolSizeCount = static_cast<std::uint32_t>(pool_sizes.size());
    pool_info.pPoolSizes = pool_sizes.data();
    if (!check(vkCreateDescriptorPool(impl.device, &pool_info, nullptr, &impl.descriptor_pool),
               "vkCreateDescriptorPool", error))
        return false;

    VkPushConstantRange push_range{VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0u,
                                   sizeof(PushConstants)};
    VkPipelineLayoutCreateInfo pipeline_layout_info{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
    const std::array<VkDescriptorSetLayout, 2> set_layouts{impl.descriptor_layout, impl.lighting_layout};
    pipeline_layout_info.setLayoutCount = static_cast<std::uint32_t>(set_layouts.size());
    pipeline_layout_info.pSetLayouts = set_layouts.data();
    pipeline_layout_info.pushConstantRangeCount = 1u;
    pipeline_layout_info.pPushConstantRanges = &push_range;
    if (!check(vkCreatePipelineLayout(impl.device, &pipeline_layout_info, nullptr, &impl.pipeline_layout),
               "vkCreatePipelineLayout", error))
        return false;

    VkSamplerCreateInfo sampler_info{VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
    sampler_info.magFilter = VK_FILTER_LINEAR;
    sampler_info.minFilter = VK_FILTER_LINEAR;
    sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.maxLod = 1.0f;
    if (!check(vkCreateSampler(impl.device, &sampler_info, nullptr, &impl.sampler), "vkCreateSampler", error))
        return false;
    sampler_info.magFilter = VK_FILTER_NEAREST;
    sampler_info.minFilter = VK_FILTER_NEAREST;
    if (!check(vkCreateSampler(impl.device, &sampler_info, nullptr, &impl.sharp_sampler), "vkCreateSampler", error))
        return false;
    sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    if (!check(vkCreateSampler(impl.device, &sampler_info, nullptr, &impl.clamp_sharp_sampler), "vkCreateSampler",
               error))
        return false;
    sampler_info.magFilter = VK_FILTER_LINEAR;
    sampler_info.minFilter = VK_FILTER_LINEAR;
    if (!check(vkCreateSampler(impl.device, &sampler_info, nullptr, &impl.clamp_sampler), "vkCreateSampler", error))
        return false;

    // Command buffer, synchronization and the vertex staging buffer.
    VkCommandPoolCreateInfo command_pool_info{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
    command_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    command_pool_info.queueFamilyIndex = impl.queue_family;
    if (!check(vkCreateCommandPool(impl.device, &command_pool_info, nullptr, &impl.command_pool), "vkCreateCommandPool",
               error))
        return false;
    VkCommandBufferAllocateInfo command_info{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    command_info.commandPool = impl.command_pool;
    command_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    command_info.commandBufferCount = 1u;
    if (!check(vkAllocateCommandBuffers(impl.device, &command_info, &impl.command_buffer), "vkAllocateCommandBuffers",
               error))
        return false;
    VkFenceCreateInfo fence_info{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    vkCreateFence(impl.device, &fence_info, nullptr, &impl.frame_fence);
    VkSemaphoreCreateInfo semaphore_info{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    vkCreateSemaphore(impl.device, &semaphore_info, nullptr, &impl.image_available);
    vkCreateSemaphore(impl.device, &semaphore_info, nullptr, &impl.render_finished);

    // GPU timestamps, where the queue has them. Without them the perf line
    // reads "gpu n/a" and nothing else changes.
    {
        VkPhysicalDeviceProperties timer_properties{};
        vkGetPhysicalDeviceProperties(impl.physical_device, &timer_properties);
        const std::uint32_t valid_bits = families[impl.queue_family].timestampValidBits;
        const float period = timer_properties.limits.timestampPeriod;
        const char *why = nullptr;
        if (std::getenv("MHP3RD_NO_GPU_TIMESTAMPS") != nullptr) why = "turned off (MHP3RD_NO_GPU_TIMESTAMPS)";
        else if (valid_bits == 0u) why = "the graphics queue has no timestamps";
        else if (!(period > 0.0f)) why = "the device reports no timestamp period";
        if (why == nullptr) {
            VkQueryPoolCreateInfo query_info{VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO};
            query_info.queryType = VK_QUERY_TYPE_TIMESTAMP;
            query_info.queryCount = 2u * kGpuTimerSegments;
            if (vkCreateQueryPool(impl.device, &query_info, nullptr, &impl.gpu_timer) != VK_SUCCESS) {
                impl.gpu_timer = VK_NULL_HANDLE;
                why = "vkCreateQueryPool failed";
            }
        }
        if (why == nullptr) {
            impl.gpu_timer_ns_per_tick = static_cast<double>(period);
            impl.gpu_timer_mask = valid_bits >= 64u ? ~0ull : (1ull << valid_bits) - 1ull;
            std::cout << "[perf] GPU timestamps: " << valid_bits << " bits, " << period << " ns per tick\n";
        } else {
            perf::set_gpu_time_unavailable();
            std::cout << "[perf] no GPU time: " << why << "\n";
        }
    }

    VkBufferCreateInfo buffer_info{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    buffer_info.size = kVertexBufferBytes;
    buffer_info.usage =
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    if (!check(vkCreateBuffer(impl.device, &buffer_info, nullptr, &impl.vertex_buffer), "vkCreateBuffer", error))
        return false;
    VkMemoryRequirements requirements{};
    vkGetBufferMemoryRequirements(impl.device, impl.vertex_buffer, &requirements);
    VkMemoryAllocateInfo allocate{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    allocate.allocationSize = requirements.size;
    allocate.memoryTypeIndex = impl.find_memory_type(
        requirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    if (!check(vkAllocateMemory(impl.device, &allocate, nullptr, &impl.vertex_memory), "vkAllocateMemory", error))
        return false;
    vkBindBufferMemory(impl.device, impl.vertex_buffer, impl.vertex_memory, 0u);
    vkMapMemory(impl.device, impl.vertex_memory, 0u, kVertexBufferBytes, 0u, &impl.vertex_mapped);

    {
        VkPhysicalDeviceProperties device_properties{};
        vkGetPhysicalDeviceProperties(impl.physical_device, &device_properties);
        impl.uniform_alignment = std::max<VkDeviceSize>(device_properties.limits.minUniformBufferOffsetAlignment, 16u);
        VkDescriptorSetAllocateInfo lighting_info{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
        lighting_info.descriptorPool = impl.descriptor_pool;
        lighting_info.descriptorSetCount = 1u;
        lighting_info.pSetLayouts = &impl.lighting_layout;
        if (!check(vkAllocateDescriptorSets(impl.device, &lighting_info, &impl.lighting_descriptor),
                   "vkAllocateDescriptorSets", error))
            return false;
        const std::array<VkDescriptorBufferInfo, 2> lighting_buffers{
            VkDescriptorBufferInfo{impl.vertex_buffer, 0u, sizeof(EnvironmentBlock)},
            VkDescriptorBufferInfo{impl.vertex_buffer, 0u, sizeof(ObjectBlock)},
        };
        VkWriteDescriptorSet write{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
        write.dstSet = impl.lighting_descriptor;
        write.descriptorCount = 1u;
        write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        std::array<VkWriteDescriptorSet, 2> writes{write, write};
        for (std::uint32_t i = 0; i < 2u; ++i) {
            writes[i].dstBinding = i;
            writes[i].pBufferInfo = &lighting_buffers[i];
        }
        vkUpdateDescriptorSets(impl.device, static_cast<std::uint32_t>(writes.size()), writes.data(), 0u, nullptr);
    }

    const std::uint32_t white = 0xFFFFFFFFu;
    impl.white_texture = impl.create_texture(1u, 1u, &white);

    // Texture packs are optional too: without the GPU side, none is loaded.
    std::string replacement_error;
    if (impl.replacements.initialize({impl.physical_device, impl.device, impl.queue, impl.queue_family,
                                      impl.descriptor_layout},
                                     replacement_error)) {
        impl.replacements.set_sharp(impl.sharp_textures);
        impl.pack_wanted = player.texture_pack;
        impl.pack_applied = !impl.pack_wanted;
        impl.apply_texture_pack();
    } else {
        std::cout << "[texpack] unavailable: " << replacement_error << "\n";
    }
    if (const char *dump = std::getenv("MHP3RD_TEXTURE_DUMP"); dump != nullptr && *dump != '\0') {
        impl.dumper = std::make_unique<TextureDumper>(dump);
        std::cout << "[texpack] writing new textures to " << dump << "\n";
    }

    // The overlay is optional: without it the game still runs, only unmeasured
    // on screen.
    std::string overlay_error;
    impl.overlay_ready = impl.create_overlay(overlay_error);
    if (!impl.overlay_ready) std::cout << "[perf] overlay unavailable: " << overlay_error << "\n";
    impl.overlay_visible = impl.overlay_ready && perf::options().overlay;

    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(impl.physical_device, &properties);
    impl.device_name = properties.deviceName;
    std::cout << "Renderer: Vulkan on " << properties.deviceName << ", target " << impl.target_extent.width << "x"
              << impl.target_extent.height << "\n";
    impl.ready = true;
    return true;
}

bool VulkanRenderer::Impl::create_overlay(std::string &error) {
    if (!create_image(perf::kOverlayWidth, perf::kOverlayHeight, VK_FORMAT_R8G8B8A8_UNORM,
                      VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, overlay_image, overlay_memory,
                      overlay_view, VK_IMAGE_ASPECT_COLOR_BIT, error))
        return false;
    overlay_pixels.assign(static_cast<std::size_t>(perf::kOverlayWidth) * perf::kOverlayHeight, 0u);
    const VkDeviceSize bytes = overlay_pixels.size() * sizeof(std::uint32_t);
    VkBufferCreateInfo buffer_info{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    buffer_info.size = bytes;
    buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    if (!check(vkCreateBuffer(device, &buffer_info, nullptr, &overlay_staging), "vkCreateBuffer", error)) return false;
    VkMemoryRequirements requirements{};
    vkGetBufferMemoryRequirements(device, overlay_staging, &requirements);
    VkMemoryAllocateInfo allocate{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    allocate.allocationSize = requirements.size;
    allocate.memoryTypeIndex = find_memory_type(
        requirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    if (!check(vkAllocateMemory(device, &allocate, nullptr, &overlay_staging_memory), "vkAllocateMemory", error))
        return false;
    vkBindBufferMemory(device, overlay_staging, overlay_staging_memory, 0u);
    return check(vkMapMemory(device, overlay_staging_memory, 0u, bytes, 0u, &overlay_mapped), "vkMapMemory", error);
}

// Draws the overlay over the top-left corner of the swapchain image, which is
// in TRANSFER_DST layout with the game frame already blitted into it. The
// staging buffer is free to rewrite: the frame fence has been waited on before
// any recording of this frame started.
void VulkanRenderer::Impl::record_overlay(VkImage destination) {
    const std::uint32_t scale = perf::overlay_scale(swapchain_extent.height);
    const std::uint32_t inset = 4u * scale;
    const std::uint32_t width = perf::kOverlayWidth * scale;
    const std::uint32_t height = perf::kOverlayHeight * scale;
    if (inset + width > swapchain_extent.width || inset + height > swapchain_extent.height) return;

    perf::draw_overlay(overlay_pixels.data());
    std::memcpy(overlay_mapped, overlay_pixels.data(), overlay_pixels.size() * sizeof(std::uint32_t));
    transition(command_buffer, overlay_image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    VkBufferImageCopy copy{};
    copy.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 0u, 1u};
    copy.imageExtent = {perf::kOverlayWidth, perf::kOverlayHeight, 1u};
    vkCmdCopyBufferToImage(command_buffer, overlay_staging, overlay_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1u,
                           &copy);
    transition(command_buffer, overlay_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
               VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    // The game frame was blitted into the same image just before; order the
    // two writes.
    transition(command_buffer, destination, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    VkImageBlit blit{};
    blit.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 0u, 1u};
    blit.srcOffsets[1] = {static_cast<std::int32_t>(perf::kOverlayWidth),
                          static_cast<std::int32_t>(perf::kOverlayHeight), 1};
    blit.dstSubresource = blit.srcSubresource;
    blit.dstOffsets[0] = {static_cast<std::int32_t>(inset), static_cast<std::int32_t>(inset), 0};
    blit.dstOffsets[1] = {static_cast<std::int32_t>(inset + width), static_cast<std::int32_t>(inset + height), 1};
    vkCmdBlitImage(command_buffer, overlay_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, destination,
                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1u, &blit, VK_FILTER_NEAREST);
}

void VulkanRenderer::Impl::update_display_info() {
    float refresh = 0.0f;
    if (const SDL_DisplayID display = SDL_GetDisplayForWindow(window); display != 0u) {
        if (const SDL_DisplayMode *mode = SDL_GetCurrentDisplayMode(display); mode != nullptr)
            refresh = mode->refresh_rate;
    }
    perf::set_display_info(present_mode_name(present_mode), swapchain_extent.width, swapchain_extent.height, refresh);
}

VkPresentModeKHR VulkanRenderer::Impl::wanted_present_mode() const {
    VkPresentModeKHR wanted = VK_PRESENT_MODE_FIFO_KHR;
    if (requested_present == settings::PresentMode::Mailbox) wanted = VK_PRESENT_MODE_MAILBOX_KHR;
    if (requested_present == settings::PresentMode::Immediate) wanted = VK_PRESENT_MODE_IMMEDIATE_KHR;
    // FIFO is the one mode every surface supports.
    return std::find(present_modes.begin(), present_modes.end(), wanted) != present_modes.end()
               ? wanted
               : VK_PRESENT_MODE_FIFO_KHR;
}

// Builds the swapchain for the window's current size, replacing the old one.
bool VulkanRenderer::Impl::create_swapchain(std::string &error) {
    VkSurfaceCapabilitiesKHR capabilities{};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &capabilities);
    VkExtent2D extent = capabilities.currentExtent;
    if (extent.width == 0xFFFFFFFFu) {
        int width = 0;
        int height = 0;
        SDL_GetWindowSizeInPixels(window, &width, &height);
        extent.width = std::clamp(static_cast<std::uint32_t>(std::max(width, 0)), capabilities.minImageExtent.width,
                                  capabilities.maxImageExtent.width);
        extent.height = std::clamp(static_cast<std::uint32_t>(std::max(height, 0)),
                                   capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
    }
    if (extent.width == 0u || extent.height == 0u) extent = target_extent;
    swapchain_extent = extent;
    present_mode = wanted_present_mode();

    std::uint32_t image_count =
        std::max(capabilities.minImageCount, present_mode == VK_PRESENT_MODE_MAILBOX_KHR ? 3u : 2u);
    if (capabilities.maxImageCount != 0u) image_count = std::min(image_count, capabilities.maxImageCount);
    // Transfer source only where offered: it is just for window captures.
    swapchain_usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                      (capabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT);

    const VkSwapchainKHR old_swapchain = swapchain;
    VkSwapchainCreateInfoKHR swapchain_info{VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
    swapchain_info.surface = surface;
    swapchain_info.minImageCount = image_count;
    swapchain_info.imageFormat = swapchain_format;
    swapchain_info.imageColorSpace = surface_format.colorSpace;
    swapchain_info.imageExtent = swapchain_extent;
    swapchain_info.imageArrayLayers = 1u;
    swapchain_info.imageUsage = swapchain_usage;
    swapchain_info.preTransform = capabilities.currentTransform;
    swapchain_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchain_info.presentMode = present_mode;
    swapchain_info.clipped = VK_TRUE;
    swapchain_info.oldSwapchain = old_swapchain;
    VkSwapchainKHR created{};
    const VkResult result = vkCreateSwapchainKHR(device, &swapchain_info, nullptr, &created);
    if (old_swapchain != VK_NULL_HANDLE) vkDestroySwapchainKHR(device, old_swapchain, nullptr);
    swapchain = VK_NULL_HANDLE;
    if (!check(result, "vkCreateSwapchainKHR", error)) return false;
    swapchain = created;
    swapchain_min_images = image_count;

    std::uint32_t count = 0u;
    vkGetSwapchainImagesKHR(device, swapchain, &count, nullptr);
    swapchain_images.resize(count);
    vkGetSwapchainImagesKHR(device, swapchain, &count, swapchain_images.data());
    for (VkImage image : swapchain_images) {
        VkImageViewCreateInfo view_info{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
        view_info.image = image;
        view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        view_info.format = swapchain_format;
        view_info.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 1u, 0u, 1u};
        VkImageView view{};
        if (!check(vkCreateImageView(device, &view_info, nullptr, &view), "vkCreateImageView", error)) return false;
        swapchain_views.push_back(view);
    }
    if (ui_render_pass != VK_NULL_HANDLE && !create_ui_framebuffers(error)) return false;
    swapchain_dirty = false;
    update_display_info();
    return true;
}

void VulkanRenderer::Impl::destroy_swapchain_views() {
    for (VkFramebuffer framebuffer : ui_framebuffers) vkDestroyFramebuffer(device, framebuffer, nullptr);
    ui_framebuffers.clear();
    for (VkImageView view : swapchain_views) vkDestroyImageView(device, view, nullptr);
    swapchain_views.clear();
}

void VulkanRenderer::Impl::recreate_swapchain() {
    // A minimised window has no area to present to; keep the old swapchain
    // and try again once it has one.
    int width = 0;
    int height = 0;
    SDL_GetWindowSizeInPixels(window, &width, &height);
    if (width <= 0 || height <= 0) return;
    vkDeviceWaitIdle(device);
    destroy_swapchain_views();
    const std::uint32_t previous_min_images = swapchain_min_images;
    std::string error;
    if (!create_swapchain(error)) {
        std::cout << "[render] cannot recreate the swapchain: " << error << "\n";
        return;
    }
    if (ui_ready && swapchain_min_images != previous_min_images) ImGui_ImplVulkan_SetMinImageCount(swapchain_min_images);
}

bool VulkanRenderer::Impl::create_ui_framebuffers(std::string &error) {
    for (VkImageView view : swapchain_views) {
        VkFramebufferCreateInfo info{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
        info.renderPass = ui_render_pass;
        info.attachmentCount = 1u;
        info.pAttachments = &view;
        info.width = swapchain_extent.width;
        info.height = swapchain_extent.height;
        info.layers = 1u;
        VkFramebuffer framebuffer{};
        if (!check(vkCreateFramebuffer(device, &info, nullptr, &framebuffer), "vkCreateFramebuffer", error))
            return false;
        ui_framebuffers.push_back(framebuffer);
    }
    return true;
}

// Scales the game frame onto the swapchain image, which is in TRANSFER_DST
// layout: stretched over the whole window, or at the PSP's aspect ratio with
// black bars. Fill's target already has the window's shape.
void VulkanRenderer::Impl::record_game_blit(VkImage source, VkImage destination) {
    const auto width = static_cast<std::int32_t>(swapchain_extent.width);
    const auto height = static_cast<std::int32_t>(swapchain_extent.height);
    VkOffset3D low{0, 0, 0};
    VkOffset3D high{width, height, 1};
    if (aspect == settings::Aspect::Original) {
        const double scale = std::min(static_cast<double>(width) / kPspWidth, static_cast<double>(height) / kPspHeight);
        const auto shown_width = std::clamp(static_cast<std::int32_t>(std::lround(kPspWidth * scale)), 1, width);
        const auto shown_height = std::clamp(static_cast<std::int32_t>(std::lround(kPspHeight * scale)), 1, height);
        low = {(width - shown_width) / 2, (height - shown_height) / 2, 0};
        high = {low.x + shown_width, low.y + shown_height, 1};
        if (shown_width < width || shown_height < height) {
            const VkClearColorValue black{{0.0f, 0.0f, 0.0f, 1.0f}};
            const VkImageSubresourceRange range{VK_IMAGE_ASPECT_COLOR_BIT, 0u, 1u, 0u, 1u};
            vkCmdClearColorImage(command_buffer, destination, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &black, 1u, &range);
            // Order the clear before the blit into the same image.
            transition(command_buffer, destination, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        }
    }
    VkImageBlit blit{};
    blit.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 0u, 1u};
    blit.srcOffsets[1] = {static_cast<std::int32_t>(target_extent.width),
                          static_cast<std::int32_t>(target_extent.height), 1};
    blit.dstSubresource = blit.srcSubresource;
    blit.dstOffsets[0] = low;
    blit.dstOffsets[1] = high;
    vkCmdBlitImage(command_buffer, source, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, destination,
                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1u, &blit, sharp_screen ? VK_FILTER_NEAREST : VK_FILTER_LINEAR);
}

// Finishes the frame being recorded: the game frame (or a plain background
// when `source` is null), the performance overlay on game frames, the
// interface, then submit and present.
void VulkanRenderer::Impl::submit_and_present(VkImage source, bool game_frame) {
    if (swapchain_dirty || swapchain == VK_NULL_HANDLE) recreate_swapchain();
    ImDrawData *ui = ui_ready ? ui_draw_data : nullptr;
    ui_draw_data = nullptr;
    const bool draw_ui = ui != nullptr && ui->CmdListsCount > 0;
    // A game flip before anything was drawn has nothing to show.
    const bool has_content = source != VK_NULL_HANDLE || !game_frame || draw_ui;

    // The GPU time covers the frame's own rendering, not the copy to the
    // window, which may wait for the presentation engine to release an image.
    end_gpu_segment(command_buffer);
    gpu_timer_pending = gpu_timer_used;

    std::uint32_t image_index = 0u;
    VkResult acquired = VK_ERROR_OUT_OF_DATE_KHR;
    if (has_content && swapchain != VK_NULL_HANDLE) {
        const perf::Clock::time_point acquire_start = perf::Clock::now();
        acquired = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, image_available, VK_NULL_HANDLE, &image_index);
        perf::add_wait_time(perf::Clock::now() - acquire_start, perf::Stall::Acquire);
        if (acquired == VK_ERROR_OUT_OF_DATE_KHR) swapchain_dirty = true;
    }
    const bool can_present = acquired == VK_SUCCESS || acquired == VK_SUBOPTIMAL_KHR;
    bool capture = false;
    if (can_present) {
        VkImage target = swapchain_images[image_index];
        transition(command_buffer, target, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        if (source != VK_NULL_HANDLE) {
            transition(command_buffer, source, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                       VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
            record_game_blit(source, target);
            transition(command_buffer, source, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                       VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        } else {
            // Behind the setup screens: the dark brown of the project's logo.
            const VkClearColorValue background{{0.075f, 0.055f, 0.045f, 1.0f}};
            const VkImageSubresourceRange range{VK_IMAGE_ASPECT_COLOR_BIT, 0u, 1u, 0u, 1u};
            vkCmdClearColorImage(command_buffer, target, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &background, 1u,
                                 &range);
        }
        if (game_frame && overlay_visible) {
            const perf::Clock::time_point overlay_start = perf::Clock::now();
            record_overlay(target);
            perf::add_overlay_time(perf::Clock::now() - overlay_start);
        }
        VkImageLayout layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        if (draw_ui && image_index < ui_framebuffers.size()) {
            transition(command_buffer, target, layout, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
            layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            VkRenderPassBeginInfo pass{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
            pass.renderPass = ui_render_pass;
            pass.framebuffer = ui_framebuffers[image_index];
            pass.renderArea = {{0, 0}, swapchain_extent};
            vkCmdBeginRenderPass(command_buffer, &pass, VK_SUBPASS_CONTENTS_INLINE);
            ImGui_ImplVulkan_RenderDrawData(ui, command_buffer);
            vkCmdEndRenderPass(command_buffer);
        }
        if (!capture_path.empty() && (swapchain_usage & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) != 0u) {
            const VkDeviceSize bytes = static_cast<VkDeviceSize>(swapchain_extent.width) * swapchain_extent.height * 4u;
            VkBufferCreateInfo buffer_info{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
            buffer_info.size = bytes;
            buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            if (vkCreateBuffer(device, &buffer_info, nullptr, &capture_buffer) == VK_SUCCESS) {
                VkMemoryRequirements requirements{};
                vkGetBufferMemoryRequirements(device, capture_buffer, &requirements);
                VkMemoryAllocateInfo allocate{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
                allocate.allocationSize = requirements.size;
                allocate.memoryTypeIndex =
                    find_memory_type(requirements.memoryTypeBits,
                                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
                vkAllocateMemory(device, &allocate, nullptr, &capture_memory);
                vkBindBufferMemory(device, capture_buffer, capture_memory, 0u);
                transition(command_buffer, target, layout, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
                layout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                VkBufferImageCopy copy{};
                copy.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 0u, 1u};
                copy.imageExtent = {swapchain_extent.width, swapchain_extent.height, 1u};
                vkCmdCopyImageToBuffer(command_buffer, target, layout, capture_buffer, 1u, &copy);
                capture_extent = swapchain_extent;
                capture = true;
            }
        }
        transition(command_buffer, target, layout, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    }
    vkEndCommandBuffer(command_buffer);

    const VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo submit{VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submit.commandBufferCount = 1u;
    submit.pCommandBuffers = &command_buffer;
    if (can_present) {
        submit.waitSemaphoreCount = 1u;
        submit.pWaitSemaphores = &image_available;
        submit.pWaitDstStageMask = &wait_stage;
        submit.signalSemaphoreCount = 1u;
        submit.pSignalSemaphores = &render_finished;
    }
    // MoltenVK waits for the next drawable here rather than in the acquire.
    const perf::Clock::time_point submit_start = perf::Clock::now();
    vkQueueSubmit(queue, 1u, &submit, frame_fence);
    perf::add_wait_time(perf::Clock::now() - submit_start, perf::Stall::Submit);

    if (can_present) {
        VkPresentInfoKHR present{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
        present.waitSemaphoreCount = 1u;
        present.pWaitSemaphores = &render_finished;
        present.swapchainCount = 1u;
        present.pSwapchains = &swapchain;
        present.pImageIndices = &image_index;
        const perf::Clock::time_point present_start = perf::Clock::now();
        const VkResult presented = vkQueuePresentKHR(queue, &present);
        perf::add_wait_time(perf::Clock::now() - present_start, perf::Stall::Present);
        if (presented == VK_ERROR_OUT_OF_DATE_KHR || presented == VK_SUBOPTIMAL_KHR) swapchain_dirty = true;
    }
    recording = false;
    if (capture) write_capture();
}

// Writes the image copied by submit_and_present once its frame has finished.
void VulkanRenderer::Impl::write_capture() {
    vkWaitForFences(device, 1u, &frame_fence, VK_TRUE, UINT64_MAX);
    void *mapped = nullptr;
    vkMapMemory(device, capture_memory, 0u, VK_WHOLE_SIZE, 0u, &mapped);
    const bool bgra = swapchain_format == VK_FORMAT_B8G8R8A8_UNORM || swapchain_format == VK_FORMAT_B8G8R8A8_SRGB;
    if (write_bmp(capture_path, static_cast<const std::uint8_t *>(mapped), capture_extent.width,
                  capture_extent.height, bgra))
        std::cout << "[render] window " << capture_extent.width << "x" << capture_extent.height << " -> "
                  << capture_path << std::endl;
    else
        std::cout << "[render] cannot write " << capture_path << std::endl;
    vkUnmapMemory(device, capture_memory);
    vkDestroyBuffer(device, capture_buffer, nullptr);
    vkFreeMemory(device, capture_memory, nullptr);
    capture_buffer = VK_NULL_HANDLE;
    capture_memory = VK_NULL_HANDLE;
    capture_path.clear();
}

void VulkanRenderer::Impl::destroy_target(Target &target) {
    for (VkDescriptorSet &descriptor : target.copy_descriptors)
        if (descriptor != VK_NULL_HANDLE) vkFreeDescriptorSets(device, descriptor_pool, 1u, &descriptor);
    vkDestroyImageView(device, target.copy_opaque_view, nullptr);
    vkDestroyImageView(device, target.copy_view, nullptr);
    vkDestroyImage(device, target.copy, nullptr);
    vkFreeMemory(device, target.copy_memory, nullptr);
    vkDestroyFramebuffer(device, target.framebuffer, nullptr);
    vkDestroyImageView(device, target.depth_view, nullptr);
    vkDestroyImage(device, target.depth, nullptr);
    vkFreeMemory(device, target.depth_memory, nullptr);
    vkDestroyImageView(device, target.color_view, nullptr);
    vkDestroyImage(device, target.color, nullptr);
    vkFreeMemory(device, target.color_memory, nullptr);
    target = Target{};
}

// Records commands into a one-time buffer and waits for them to finish.
void VulkanRenderer::Impl::run_commands(const std::function<void(VkCommandBuffer)> &record) {
    VkCommandBufferAllocateInfo command_info{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    command_info.commandPool = command_pool;
    command_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    command_info.commandBufferCount = 1u;
    VkCommandBuffer commands{};
    vkAllocateCommandBuffers(device, &command_info, &commands);
    VkCommandBufferBeginInfo begin{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(commands, &begin);
    record(commands);
    vkEndCommandBuffer(commands);
    VkSubmitInfo submit{VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submit.commandBufferCount = 1u;
    submit.pCommandBuffers = &commands;
    vkQueueSubmit(queue, 1u, &submit, VK_NULL_HANDLE);
    vkQueueWaitIdle(queue);
    vkFreeCommandBuffers(device, command_pool, 1u, &commands);
}

VulkanRenderer::Impl::Target *VulkanRenderer::Impl::target_for(std::uint32_t address, std::string &error) {
    const auto found = targets.find(address);
    if (found != targets.end()) return &found->second;

    Target target{};
    if (!create_image(target_extent.width, target_extent.height, VK_FORMAT_R8G8B8A8_UNORM,
                      VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                          VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                      target.color,
                      target.color_memory, target.color_view, VK_IMAGE_ASPECT_COLOR_BIT, error))
        return nullptr;
    if (!create_image(target_extent.width, target_extent.height, VK_FORMAT_D32_SFLOAT,
                      VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, target.depth, target.depth_memory,
                      target.depth_view, VK_IMAGE_ASPECT_DEPTH_BIT, error))
        return nullptr;
    const std::array<VkImageView, 2> views{target.color_view, target.depth_view};
    VkFramebufferCreateInfo info{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
    info.renderPass = render_pass;
    info.attachmentCount = static_cast<std::uint32_t>(views.size());
    info.pAttachments = views.data();
    info.width = target_extent.width;
    info.height = target_extent.height;
    info.layers = 1u;
    if (!check(vkCreateFramebuffer(device, &info, nullptr, &target.framebuffer), "vkCreateFramebuffer", error))
        return nullptr;
    static const bool trace = std::getenv("MHP3RD_TRACE_FB_TEXTURES") != nullptr;
    if (trace) std::cout << "[fbtex] frame " << frames << " new render target 0x" << std::hex << address << std::dec << "\n";
    return &targets.emplace(address, target).first->second;
}

void VulkanRenderer::Impl::destroy_upload() {
    if (upload_mapped != nullptr) vkUnmapMemory(device, upload_staging_memory);
    vkDestroyBuffer(device, upload_staging, nullptr);
    vkFreeMemory(device, upload_staging_memory, nullptr);
    vkDestroyImageView(device, upload_view, nullptr);
    vkDestroyImage(device, upload_image, nullptr);
    vkFreeMemory(device, upload_memory, nullptr);
    upload_mapped = nullptr;
    upload_staging = VK_NULL_HANDLE;
    upload_staging_memory = VK_NULL_HANDLE;
    upload_view = VK_NULL_HANDLE;
    upload_image = VK_NULL_HANDLE;
    upload_memory = VK_NULL_HANDLE;
    upload_extent = {};
}

bool VulkanRenderer::Impl::create_upload(std::uint32_t width, std::uint32_t height, std::string &error) {
    destroy_upload();
    if (!create_image(width, height, VK_FORMAT_R8G8B8A8_UNORM,
                      VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, upload_image, upload_memory,
                      upload_view, VK_IMAGE_ASPECT_COLOR_BIT, error))
        return false;
    const VkDeviceSize bytes = static_cast<VkDeviceSize>(width) * height * 4u;
    VkBufferCreateInfo buffer_info{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    buffer_info.size = bytes;
    buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    if (!check(vkCreateBuffer(device, &buffer_info, nullptr, &upload_staging), "vkCreateBuffer", error)) return false;
    VkMemoryRequirements requirements{};
    vkGetBufferMemoryRequirements(device, upload_staging, &requirements);
    VkMemoryAllocateInfo allocate{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    allocate.allocationSize = requirements.size;
    allocate.memoryTypeIndex = find_memory_type(
        requirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    if (!check(vkAllocateMemory(device, &allocate, nullptr, &upload_staging_memory), "vkAllocateMemory", error))
        return false;
    vkBindBufferMemory(device, upload_staging, upload_staging_memory, 0u);
    if (!check(vkMapMemory(device, upload_staging_memory, 0u, bytes, 0u, &upload_mapped), "vkMapMemory", error))
        return false;
    upload_extent = {width, height};
    return true;
}

void VulkanRenderer::Impl::begin_pass(std::uint32_t address) {
    std::string error;
    Target *target = target_for(address, error);
    if (target == nullptr) return;
    if (!target->initialized) {
        // Attachments are loaded, not cleared, so a new target starts undefined:
        // move it into the layouts the render pass expects once.
        transition(command_buffer, target->color, VK_IMAGE_LAYOUT_UNDEFINED,
                   VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        transition(command_buffer, target->depth, VK_IMAGE_LAYOUT_UNDEFINED,
                   VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT);
        target->initialized = true;
    }
    VkRenderPassBeginInfo pass{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
    pass.renderPass = render_pass;
    pass.framebuffer = target->framebuffer;
    pass.renderArea = {{0, 0}, target_extent};
    vkCmdBeginRenderPass(command_buffer, &pass, VK_SUBPASS_CONTENTS_INLINE);
    forget_bindings();
    current_target = address;
    last_drawn_target = address;
    pass_active = true;
}

void VulkanRenderer::Impl::end_pass() {
    if (!pass_active) return;
    vkCmdEndRenderPass(command_buffer);
    pass_active = false;
}

VulkanRenderer::Impl::Texture VulkanRenderer::Impl::create_texture(std::uint32_t width, std::uint32_t height,
                                                                   const std::uint32_t *pixels) {
    Texture texture{};
    std::string error;
    if (!create_image(width, height, VK_FORMAT_R8G8B8A8_UNORM,
                      VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, texture.image, texture.memory,
                      texture.view, VK_IMAGE_ASPECT_COLOR_BIT, error))
        return texture;

    const VkDeviceSize bytes = static_cast<VkDeviceSize>(width) * height * 4u;
    VkBuffer staging{};
    VkDeviceMemory staging_memory{};
    VkBufferCreateInfo buffer_info{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    buffer_info.size = bytes;
    buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    vkCreateBuffer(device, &buffer_info, nullptr, &staging);
    VkMemoryRequirements requirements{};
    vkGetBufferMemoryRequirements(device, staging, &requirements);
    VkMemoryAllocateInfo allocate{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    allocate.allocationSize = requirements.size;
    allocate.memoryTypeIndex = find_memory_type(
        requirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    vkAllocateMemory(device, &allocate, nullptr, &staging_memory);
    vkBindBufferMemory(device, staging, staging_memory, 0u);
    void *mapped = nullptr;
    vkMapMemory(device, staging_memory, 0u, bytes, 0u, &mapped);
    std::memcpy(mapped, pixels, static_cast<std::size_t>(bytes));
    vkUnmapMemory(device, staging_memory);

    VkCommandBufferAllocateInfo command_info{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    command_info.commandPool = command_pool;
    command_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    command_info.commandBufferCount = 1u;
    VkCommandBuffer commands{};
    vkAllocateCommandBuffers(device, &command_info, &commands);
    VkCommandBufferBeginInfo begin{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(commands, &begin);
    transition(commands, texture.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    VkBufferImageCopy copy{};
    copy.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 0u, 1u};
    copy.imageExtent = {width, height, 1u};
    vkCmdCopyBufferToImage(commands, staging, texture.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1u, &copy);
    transition(commands, texture.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
               VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    vkEndCommandBuffer(commands);
    VkSubmitInfo submit{VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submit.commandBufferCount = 1u;
    submit.pCommandBuffers = &commands;
    // Uploads are synchronous: the GPU finishes everything queued before this
    // returns, which counts as waiting rather than rendering.
    const perf::Clock::time_point wait_start = perf::Clock::now();
    vkQueueSubmit(queue, 1u, &submit, VK_NULL_HANDLE);
    vkQueueWaitIdle(queue);
    perf::add_wait_time(perf::Clock::now() - wait_start, perf::Stall::Upload);
    vkFreeCommandBuffers(device, command_pool, 1u, &commands);
    vkDestroyBuffer(device, staging, nullptr);
    vkFreeMemory(device, staging_memory, nullptr);

    VkDescriptorSetAllocateInfo descriptor_info{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    descriptor_info.descriptorPool = descriptor_pool;
    descriptor_info.descriptorSetCount = 1u;
    descriptor_info.pSetLayouts = &descriptor_layout;
    vkAllocateDescriptorSets(device, &descriptor_info, &texture.descriptor);
    VkDescriptorImageInfo image_info{texture_sampler(), texture.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
    VkWriteDescriptorSet write{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
    write.dstSet = texture.descriptor;
    write.descriptorCount = 1u;
    write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write.pImageInfo = &image_info;
    vkUpdateDescriptorSets(device, 1u, &write, 0u, nullptr);
    texture.last_used = ++texture_clock;
    return texture;
}

void VulkanRenderer::Impl::destroy_texture(Texture &texture) {
    if (texture.descriptor != VK_NULL_HANDLE) vkFreeDescriptorSets(device, descriptor_pool, 1u, &texture.descriptor);
    if (texture.view != VK_NULL_HANDLE) vkDestroyImageView(device, texture.view, nullptr);
    if (texture.image != VK_NULL_HANDLE) vkDestroyImage(device, texture.image, nullptr);
    if (texture.memory != VK_NULL_HANDLE) vkFreeMemory(device, texture.memory, nullptr);
    texture = Texture{};
}

namespace {

// The largest V a draw reads from a 512-tall texture, the way texture packs
// track it: through-mode draws give theirs, and anything else counts as the
// whole texture.
std::uint16_t drawn_max_v(const DrawCall &call) {
    if (!call.through) return 512u;
    float max_v = 0.0f;
    for (const Vertex &vertex : call.vertices) max_v = std::max(max_v, vertex.texcoord[1]);
    return static_cast<std::uint16_t>(std::clamp(max_v, 0.0f, 512.0f));
}

// A texture's seen height after a draw that read down to `drawn`.
std::uint16_t update_max_seen_v(std::uint16_t seen, std::uint16_t drawn, bool through) {
    if (!through) return 512u;
    if (seen == 0u) return drawn > 0u ? std::max<std::uint16_t>(272u, drawn) : 0u;
    return drawn > seen ? 512u : seen;
}

} // namespace

VulkanRenderer::Impl::Texture &VulkanRenderer::Impl::texture_for(const GuestMemory &memory, const DrawCall &call) {
    const TextureState &state = call.texture;
    const TextureKeyInput input{state.address,
                                state.buffer_width,
                                static_cast<std::uint32_t>(state.width) << 16u | state.height,
                                static_cast<std::uint32_t>(state.format),
                                state.clut_address,
                                state.clut_format,
                                state.swizzled};
    static const bool no_lookup_env = std::getenv("MHP3RD_NO_LOOKUP_CACHE") != nullptr;
    const bool no_lookup_cache = no_lookup_env || perf::alternate_off(perf::NewPath::Lookup);
    ListTexture *memo = nullptr;
    if (!no_lookup_cache && last_texture != nullptr && input == last_texture_input) {
        memo = last_texture;
    } else {
        auto [entry, inserted] = list_texture_keys.try_emplace(input);
        if (inserted) entry->second.key = texture_key(memory, state);
        memo = &entry->second;
        last_texture_input = input;
        last_texture = memo;
    }
    // A cached texture drawn again. A 512-tall one drawn further down than
    // before is hashed again over the rows now in use, and may find another
    // image in the texture pack.
    const auto use = [&](Texture &texture) -> Texture & {
        texture.last_used = ++texture_clock;
        if (pack && state.height == 512u && texture.max_seen_v < 512u) {
            const std::uint16_t seen = update_max_seen_v(texture.max_seen_v, drawn_max_v(call), call.through);
            if (seen != texture.max_seen_v) {
                texture.max_seen_v = seen;
                texture.replacement = pack->find(memory, state, seen);
            }
        }
        return texture;
    };
    if (!no_lookup_cache && memo->texture != nullptr && memo->erased == textures_erased) return use(*memo->texture);
    const std::uint64_t key = memo->key;
    const auto found = textures.find(key);
    if (found != textures.end()) {
        memo->texture = &found->second;
        memo->erased = textures_erased;
        return use(found->second);
    }
    std::vector<std::uint32_t> pixels;
    if (!decode_texture(memory, state, pixels) || pixels.empty()) {
        // MHP3RD_TRACE_WHITE_TEXTURES: each texture that could not be decoded
        // and is drawn white instead, once.
        static const bool trace_white = std::getenv("MHP3RD_TRACE_WHITE_TEXTURES") != nullptr;
        static std::set<std::uint64_t> reported;
        if (trace_white && reported.insert(key).second) {
            std::cerr << "[white-texture] addr=0x" << std::hex << state.address << " buffer_width=" << std::dec
                      << state.buffer_width << " size=" << state.width << "x" << state.height
                      << " format=" << static_cast<int>(state.format) << " swizzled=" << state.swizzled << " clut=0x"
                      << std::hex << state.clut_address << " clut_format=" << state.clut_format << std::dec
                      << " in_memory=" << memory.contains(state.address, 1u) << "\n";
        }
        return white_texture;
    }
    const std::uint16_t max_seen_v =
        state.height == 512u ? update_max_seen_v(0u, drawn_max_v(call), call.through) : 0u;
    if (dumper) {
        static const TexturePackOptions kDumpOptions = [] {
            TexturePackOptions options;
            options.ignore_address = true;
            return options;
        }();
        dumper->dump(memory, state, max_seen_v, pack ? pack->options() : kDumpOptions, pixels.data());
    }

    if (textures.size() >= kMaxCachedTextures) {
        auto oldest = textures.begin();
        for (auto it = textures.begin(); it != textures.end(); ++it) {
            if (it->second.last_used < oldest->second.last_used) oldest = it;
        }
        const perf::Clock::time_point wait_start = perf::Clock::now();
        vkQueueWaitIdle(queue);
        perf::add_wait_time(perf::Clock::now() - wait_start, perf::Stall::Evict);
        destroy_texture(oldest->second);
        textures.erase(oldest);
        ++textures_erased;
    }
    Texture texture = create_texture(state.width, state.height, pixels.data());
    if (texture.descriptor == VK_NULL_HANDLE) return white_texture;
    texture.max_seen_v = max_seen_v;
    // The pack's hash reads the whole texture, so it is taken here, once per
    // upload, and never on the per-draw path above.
    if (pack) texture.replacement = pack->find(memory, state, max_seen_v);
    Texture &cached = textures.emplace(key, std::move(texture)).first->second;
    memo->texture = &cached;
    memo->erased = textures_erased;
    return cached;
}

VkDescriptorSet VulkanRenderer::Impl::texture_descriptor(const GuestMemory &memory, const DrawCall &call) {
    Texture &texture = texture_for(memory, call);
    if (texture.replacement && pack) {
        // Until the image is decoded and on the GPU, the original is drawn.
        if (const VkDescriptorSet replaced = replacements.descriptor(*texture.replacement, *pack, frames)) {
            ++replaced_draws;
            return replaced;
        }
    }
    return texture.descriptor;
}

void VulkanRenderer::Impl::apply_texture_pack() {
    const bool wanted = pack_wanted && !pack_held;
    if (pack_applied == wanted && !pack_reload) return;
    pack_applied = wanted;
    pack_reload = false;
    // Every cached texture is dropped, so the next draws decode the originals
    // again and, with the pack on, look them up in it. Off draws exactly what
    // no pack draws.
    vkDeviceWaitIdle(device);
    replacements.clear();
    for (auto &[key, texture] : textures) destroy_texture(texture);
    textures.clear();
    ++textures_erased;
    list_texture_keys.clear();
    last_texture = nullptr;
    pack.reset();
    pack_status.clear();
    if (!wanted) {
        pack_status = pack_held ? "Updating" : "Off";
        return;
    }
    // MHP3RD_TEXTURE_PACK may name the folder, or the player may use a pack
    // where it is; otherwise the pack lives in textures/<disc id>/ in the
    // per-user data directory (texture_pack_import.hpp).
    pack_location = texture_pack_location(VulkanRenderer::textures_root(), install::kDiscId,
                                          settings::current().texture_pack_folder);
    const std::filesystem::path &folder = pack_location.folder;
    std::string error;
    pack = TexturePack::open(folder, install::kDiscId, error);
    if (!pack) {
        std::error_code ec;
        if (std::filesystem::is_directory(folder, ec)) pack_status = "Not loaded: " + error;
        else if (pack_location.source == TexturePackLocation::Source::Installed) pack_status = "Not installed";
        else pack_status = "Folder missing: " + install::path_to_utf8(folder);
        std::cout << "[texpack] " << error << "\n";
        return;
    }
    pack_status = std::to_string(pack->entry_count()) + " textures";
    std::cout << "[texpack] " << pack->entry_count() << " textures from " << folder.string() << "\n";
}

namespace {

std::uint32_t texture_bits_per_pixel(TextureFormat format) {
    switch (format) {
    case TextureFormat::Rgba8888:
    case TextureFormat::Clut32: return 32u;
    case TextureFormat::Clut8: return 8u;
    case TextureFormat::Clut4:
    case TextureFormat::Dxt1: return 4u;
    case TextureFormat::Dxt3:
    case TextureFormat::Dxt5: return 8u;
    default: return 16u;
    }
}

std::uint32_t framebuffer_bytes_per_pixel(std::uint32_t format) { return format == 3u ? 4u : 2u; }

} // namespace

// MHP3RD_TRACE_FB_TEXTURES: every distinct texture whose memory overlaps a
// guest framebuffer the renderer has drawn to, once, with where it is drawn.
void VulkanRenderer::Impl::trace_framebuffer_texture(const DrawCall &call) {
    const TextureState &texture = call.texture;
    const std::uint64_t texture_bytes = static_cast<std::uint64_t>(std::max<std::uint32_t>(
                                            texture.buffer_width, texture.width)) *
                                        texture.height * texture_bits_per_pixel(texture.format) / 8u;
    // Textures filled by a DMA copy out of VRAM, and large textures in
    // general: a screen-sized background the game copied with the CPU shows up
    // as one of these, next to [vram] lines for the reads.
    static std::map<std::array<std::uint32_t, 4>, std::uint32_t> seen_textures;
    const std::array<std::uint32_t, 4> texture_id{texture.address, static_cast<std::uint32_t>(texture.format),
                                                  static_cast<std::uint32_t>(texture.width) << 16u | texture.height,
                                                  texture.buffer_width};
    std::uint32_t copy_source = 0u;
    std::uint32_t copy_destination = 0u;
    const bool copied = find_vram_copy(texture.address, copy_source, copy_destination);
    if ((copied || (texture.width >= 256u && texture.height >= 128u)) && seen_textures.size() < 400u &&
        seen_textures[texture_id]++ == 0u) {
        std::cout << "[fbtex] frame " << frames << " large or copied texture 0x" << std::hex << texture.address
                  << std::dec << " fmt=" << static_cast<int>(texture.format) << " " << texture.width << "x"
                  << texture.height << " bufw=" << texture.buffer_width << " swizzled=" << (texture.swizzled ? 1 : 0)
                  << " drawn to 0x" << std::hex << call.target.color_address << std::dec
                  << (call.through ? " through" : " transform");
        if (copied)
            std::cout << " copied from VRAM 0x" << std::hex << copy_source << " to 0x" << copy_destination << std::dec;
        std::cout << "\n";
    }

    static std::map<std::array<std::uint32_t, 7>, std::uint32_t> seen;
    for (const auto &[address, target] : targets) {
        const std::uint64_t target_bytes =
            static_cast<std::uint64_t>(target.stride) * kPspHeight * framebuffer_bytes_per_pixel(target.format);
        const std::uint64_t start = texture.address;
        if (start + texture_bytes <= address || start >= address + target_bytes) continue;
        const std::array<std::uint32_t, 7> key{texture.address, static_cast<std::uint32_t>(texture.format),
                                               texture.width,   texture.height,
                                               texture.buffer_width, address, call.target.color_address};
        std::uint32_t &count = seen[key];
        if (count++ != 0u || seen.size() > 400u) continue;
        const Vertex &first = call.vertices.front();
        const Vertex &last = call.vertices.back();
        std::cout << "[fbtex] frame " << frames << " texture 0x" << std::hex << texture.address << std::dec
                  << " fmt=" << static_cast<int>(texture.format) << " " << texture.width << "x" << texture.height
                  << " bufw=" << texture.buffer_width << " swizzled=" << (texture.swizzled ? 1 : 0)
                  << " filter=" << texture.min_filter << "/" << texture.mag_filter << " wrap=" << texture.wrap_s
                  << "/" << texture.wrap_t << " in framebuffer 0x" << std::hex << address << std::dec
                  << " (stride=" << target.stride << " fmt=" << target.format
                  << " last drawn frame " << target.last_drawn_frame << ", offset "
                  << (texture.address - address) << ") drawn to 0x" << std::hex << call.target.color_address
                  << std::dec << " fmt=" << call.target.color_format << (call.through ? " through" : " transform")
                  << " prim=" << static_cast<int>(call.primitive) << " verts=" << call.vertices.size()
                  << " pos=(" << first.position[0] << "," << first.position[1] << ")-(" << last.position[0] << ","
                  << last.position[1] << ") uv=(" << first.texcoord[0] << "," << first.texcoord[1] << ")-("
                  << last.texcoord[0] << "," << last.texcoord[1] << ") uvscale=(" << texture.scale_u << ","
                  << texture.scale_v << "," << texture.offset_u << "," << texture.offset_v << ") blend="
                  << (call.blend.enabled ? 1 : 0) << " tfx=" << texture.function << "\n";
    }
}

bool VulkanRenderer::Impl::create_writeback(std::string &error) {
    if (!create_image(kPspWidth, kPspHeight, VK_FORMAT_R8G8B8A8_UNORM,
                      VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, writeback_image,
                      writeback_memory, writeback_view, VK_IMAGE_ASPECT_COLOR_BIT, error))
        return false;
    const VkDeviceSize bytes = static_cast<VkDeviceSize>(kPspWidth) * kPspHeight * 4u;
    VkBufferCreateInfo buffer_info{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    buffer_info.size = bytes;
    buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    if (!check(vkCreateBuffer(device, &buffer_info, nullptr, &writeback_buffer), "vkCreateBuffer", error))
        return false;
    VkMemoryRequirements requirements{};
    vkGetBufferMemoryRequirements(device, writeback_buffer, &requirements);
    VkMemoryAllocateInfo allocate{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    allocate.allocationSize = requirements.size;
    allocate.memoryTypeIndex = find_memory_type(
        requirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    if (!check(vkAllocateMemory(device, &allocate, nullptr, &writeback_buffer_memory), "vkAllocateMemory", error))
        return false;
    vkBindBufferMemory(device, writeback_buffer, writeback_buffer_memory, 0u);
    return check(vkMapMemory(device, writeback_buffer_memory, 0u, bytes, 0u, &writeback_mapped), "vkMapMemory",
                 error);
}

void VulkanRenderer::Impl::destroy_writeback() {
    if (writeback_mapped != nullptr) vkUnmapMemory(device, writeback_buffer_memory);
    vkDestroyBuffer(device, writeback_buffer, nullptr);
    vkFreeMemory(device, writeback_buffer_memory, nullptr);
    vkDestroyImageView(device, writeback_view, nullptr);
    vkDestroyImage(device, writeback_image, nullptr);
    vkFreeMemory(device, writeback_memory, nullptr);
    writeback_mapped = nullptr;
    writeback_buffer = VK_NULL_HANDLE;
    writeback_buffer_memory = VK_NULL_HANDLE;
    writeback_view = VK_NULL_HANDLE;
    writeback_image = VK_NULL_HANDLE;
    writeback_memory = VK_NULL_HANDLE;
    writeback_in_flight = false;
    writeback_has_pixels = false;
}

// Records the copy of the displayed target that write_back_frame() stores in
// guest memory. Only framebuffers in VRAM that the GE drew are written back;
// a frame the CPU wrote itself is in memory already.
void VulkanRenderer::Impl::record_writeback(std::uint32_t address) {
    const auto found = targets.find(address);
    if (found == targets.end() || (GuestMemory::canonical(address) & 0x1F000000u) != 0x04000000u) return;
    Target &target = found->second;
    if (!target.initialized || target.guest_words.empty() || target.stride < kPspWidth) return;
    if (writeback_image == VK_NULL_HANDLE) {
        std::string error;
        if (!create_writeback(error)) {
            std::cout << "[render] cannot write frames back to guest memory: " << error << "\n";
            destroy_writeback();
            return;
        }
    }
    transition(command_buffer, target.color, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
               VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    transition(command_buffer, writeback_image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    VkImageBlit blit{};
    blit.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 0u, 1u};
    blit.srcOffsets[1] = {static_cast<std::int32_t>(target_extent.width),
                          static_cast<std::int32_t>(target_extent.height), 1};
    blit.dstSubresource = blit.srcSubresource;
    blit.dstOffsets[1] = {static_cast<std::int32_t>(kPspWidth), static_cast<std::int32_t>(kPspHeight), 1};
    vkCmdBlitImage(command_buffer, target.color, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, writeback_image,
                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1u, &blit, VK_FILTER_LINEAR);
    transition(command_buffer, writeback_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
               VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    VkBufferImageCopy copy{};
    copy.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 0u, 1u};
    copy.imageExtent = {kPspWidth, kPspHeight, 1u};
    vkCmdCopyImageToBuffer(command_buffer, writeback_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, writeback_buffer,
                           1u, &copy);
    transition(command_buffer, target.color, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
               VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    writeback_recorded = {address, target.stride, target.format};
    writeback_in_flight = true;
}

// Reads one guest word from every 256 bytes of the buffer a target stands for.
void VulkanRenderer::Impl::snapshot_guest_words(const GuestMemory &memory, std::uint32_t address, Target &target) {
    const std::uint32_t bytes = target.stride * kPspHeight * framebuffer_bytes_per_pixel(target.format);
    target.guest_words.clear();
    if (!memory.contains(address, bytes)) return;
    target.guest_words.reserve(bytes / 256u + 1u);
    for (std::uint32_t offset = 0; offset + 4u <= bytes; offset += 256u)
        target.guest_words.push_back(memory.load32(address + offset));
}

// The render target a texture lies in, if the texture reads it the way it was
// drawn: the same row length, a direct colour format of the same pixel size,
// and guest memory under the texture unchanged since the target was last drawn
// to. Anything else (a palette or swizzled view of a framebuffer, or a texture
// the game has since put where a framebuffer was) is decoded from guest memory.
VulkanRenderer::Impl::FramebufferTexture VulkanRenderer::Impl::find_framebuffer_texture(
    const GuestMemory &memory, const TextureState &texture) {
    if (texture.swizzled || static_cast<std::uint32_t>(texture.format) > 3u || texture.width == 0u ||
        texture.height == 0u)
        return {};
    const std::uint32_t texture_address = GuestMemory::canonical(texture.address);
    Target *best = nullptr;
    std::uint32_t best_base = 0u;
    for (auto &[address, target] : targets) {
        if (!target.initialized || target.guest_words.empty()) continue;
        const std::uint32_t bytes_per_pixel = framebuffer_bytes_per_pixel(target.format);
        if (texture_bits_per_pixel(texture.format) != bytes_per_pixel * 8u) continue;
        const std::uint32_t base = GuestMemory::canonical(address);
        const std::uint32_t bytes = target.stride * kPspHeight * bytes_per_pixel;
        if (texture_address < base || texture_address - base >= bytes) continue;
        if (texture.buffer_width != target.stride || (texture_address - base) % bytes_per_pixel != 0u) continue;
        if (best == nullptr || target.draw_serial > best->draw_serial) {
            best = &target;
            best_base = base;
        }
    }
    if (best == nullptr) return {};

    const std::uint32_t bytes_per_pixel = framebuffer_bytes_per_pixel(best->format);
    const std::uint64_t texture_end = static_cast<std::uint64_t>(texture_address) +
                                      static_cast<std::uint64_t>(texture.buffer_width) * texture.height *
                                          bytes_per_pixel;
    for (std::size_t i = 0; i < best->guest_words.size(); ++i) {
        const std::uint32_t word_address = best_base + static_cast<std::uint32_t>(i) * 256u;
        if (word_address < texture_address || word_address >= texture_end) continue;
        if (!memory.contains(word_address, 4u) || memory.load32(word_address) != best->guest_words[i]) return {};
    }
    const std::uint32_t pixel = (texture_address - best_base) / bytes_per_pixel;
    FramebufferTexture found{best, pixel % best->stride, pixel / best->stride};
    if (found.x >= kPspWidth || found.y >= kPspHeight) return {};
    return found;
}

// A sampled copy of the target, brought up to date with its latest draw. The
// copy is recorded between render passes, so the pass in progress ends here.
VkDescriptorSet VulkanRenderer::Impl::framebuffer_descriptor(Target &target, bool opaque) {
    if (target.copy == VK_NULL_HANDLE) {
        std::string error;
        if (!create_image(target_extent.width, target_extent.height, VK_FORMAT_R8G8B8A8_UNORM,
                          VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, target.copy,
                          target.copy_memory, target.copy_view, VK_IMAGE_ASPECT_COLOR_BIT, error)) {
            std::cout << "[render] cannot sample a render target: " << error << "\n";
            return VK_NULL_HANDLE;
        }
        // A 5650 texture has no alpha: the GE reads it as opaque, whatever the
        // target's alpha channel holds.
        VkImageViewCreateInfo view_info{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
        view_info.image = target.copy;
        view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        view_info.format = VK_FORMAT_R8G8B8A8_UNORM;
        view_info.components.a = VK_COMPONENT_SWIZZLE_ONE;
        view_info.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 1u, 0u, 1u};
        if (vkCreateImageView(device, &view_info, nullptr, &target.copy_opaque_view) != VK_SUCCESS)
            return VK_NULL_HANDLE;
        for (std::size_t i = 0; i < target.copy_descriptors.size(); ++i) {
            VkDescriptorSetAllocateInfo descriptor_info{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
            descriptor_info.descriptorPool = descriptor_pool;
            descriptor_info.descriptorSetCount = 1u;
            descriptor_info.pSetLayouts = &descriptor_layout;
            if (vkAllocateDescriptorSets(device, &descriptor_info, &target.copy_descriptors[i]) != VK_SUCCESS) {
                target.copy_descriptors[i] = VK_NULL_HANDLE;
                return VK_NULL_HANDLE;
            }
            VkDescriptorImageInfo image_info{framebuffer_sampler(), i == 0u ? target.copy_view : target.copy_opaque_view,
                                             VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
            VkWriteDescriptorSet write{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
            write.dstSet = target.copy_descriptors[i];
            write.descriptorCount = 1u;
            write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            write.pImageInfo = &image_info;
            vkUpdateDescriptorSets(device, 1u, &write, 0u, nullptr);
        }
        target.copy_valid = false;
    }
    if (!target.copy_valid || target.copy_serial != target.draw_serial) {
        end_pass();
        transition(command_buffer, target.color, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                   VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
        transition(command_buffer, target.copy,
                   target.copy_valid ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_UNDEFINED,
                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        VkImageCopy region{};
        region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 0u, 1u};
        region.dstSubresource = region.srcSubresource;
        region.extent = {target_extent.width, target_extent.height, 1u};
        vkCmdCopyImage(command_buffer, target.color, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, target.copy,
                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1u, &region);
        transition(command_buffer, target.copy, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                   VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        transition(command_buffer, target.color, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                   VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        target.copy_serial = target.draw_serial;
        target.copy_valid = true;
    }
    return target.copy_descriptors[opaque ? 1u : 0u];
}

VkPipeline VulkanRenderer::Impl::pipeline_for(const PipelineKey &key) {
    static const bool no_lookup_env = std::getenv("MHP3RD_NO_LOOKUP_CACHE") != nullptr;
    const bool no_lookup_cache = no_lookup_env || perf::alternate_off(perf::NewPath::Lookup);
    if (!no_lookup_cache && last_pipeline != VK_NULL_HANDLE && key == last_pipeline_key) return last_pipeline;
    const auto found = pipelines.find(key);
    if (found != pipelines.end()) {
        last_pipeline_key = key;
        last_pipeline = found->second;
        return found->second;
    }

    std::array<VkPipelineShaderStageCreateInfo, 2> stages{};
    stages[0] = {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
    stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    stages[0].module = vertex_shader;
    stages[0].pName = "main";
    stages[1] = stages[0];
    stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    stages[1].module = fragment_shader;

    VkVertexInputBindingDescription binding{0u, sizeof(GpuVertex), VK_VERTEX_INPUT_RATE_VERTEX};
    std::array<VkVertexInputAttributeDescription, 4> attributes{
        VkVertexInputAttributeDescription{0u, 0u, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(GpuVertex, x)},
        VkVertexInputAttributeDescription{1u, 0u, VK_FORMAT_R32G32_SFLOAT, offsetof(GpuVertex, u)},
        VkVertexInputAttributeDescription{2u, 0u, VK_FORMAT_R8G8B8A8_UNORM, offsetof(GpuVertex, color)},
        VkVertexInputAttributeDescription{3u, 0u, VK_FORMAT_R32G32B32_SFLOAT, offsetof(GpuVertex, nx)},
    };
    VkPipelineVertexInputStateCreateInfo vertex_input{VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
    vertex_input.vertexBindingDescriptionCount = 1u;
    vertex_input.pVertexBindingDescriptions = &binding;
    vertex_input.vertexAttributeDescriptionCount = static_cast<std::uint32_t>(attributes.size());
    vertex_input.pVertexAttributeDescriptions = attributes.data();

    VkPipelineInputAssemblyStateCreateInfo assembly{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
    assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    // The GE viewport and scissor change per draw, so they are dynamic state.
    VkPipelineViewportStateCreateInfo viewport_state{VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
    viewport_state.viewportCount = 1u;
    viewport_state.scissorCount = 1u;
    const std::array<VkDynamicState, 3> dynamic_states{VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR,
                                                       VK_DYNAMIC_STATE_BLEND_CONSTANTS};
    VkPipelineDynamicStateCreateInfo dynamic_state{VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
    dynamic_state.dynamicStateCount = static_cast<std::uint32_t>(dynamic_states.size());
    dynamic_state.pDynamicStates = dynamic_states.data();

    VkPipelineRasterizationStateCreateInfo raster{VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
    raster.polygonMode = VK_POLYGON_MODE_FILL;
    raster.cullMode = key.cull ? VK_CULL_MODE_BACK_BIT : VK_CULL_MODE_NONE;
    raster.frontFace = key.cull_clockwise ? VK_FRONT_FACE_CLOCKWISE : VK_FRONT_FACE_COUNTER_CLOCKWISE;
    raster.lineWidth = 1.0f;

    VkPipelineMultisampleStateCreateInfo multisample{VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
    multisample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineDepthStencilStateCreateInfo depth{VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
    depth.depthTestEnable = key.depth_test ? VK_TRUE : VK_FALSE;
    depth.depthWriteEnable = key.depth_write ? VK_TRUE : VK_FALSE;
    depth.depthCompareOp = to_compare_op(key.depth_function);

    VkPipelineColorBlendAttachmentState blend_attachment{};
    blend_attachment.blendEnable = key.blend ? VK_TRUE : VK_FALSE;
    blend_attachment.srcColorBlendFactor = to_blend_factor(key.source_factor, true);
    blend_attachment.dstColorBlendFactor = to_blend_factor(key.destination_factor, false);
    blend_attachment.colorBlendOp = to_blend_op(key.equation);
    blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;
    blend_attachment.colorWriteMask = key.color_mask;
    VkPipelineColorBlendStateCreateInfo blend{VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
    blend.attachmentCount = 1u;
    blend.pAttachments = &blend_attachment;

    VkGraphicsPipelineCreateInfo info{VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
    info.stageCount = static_cast<std::uint32_t>(stages.size());
    info.pStages = stages.data();
    info.pVertexInputState = &vertex_input;
    info.pInputAssemblyState = &assembly;
    info.pViewportState = &viewport_state;
    info.pRasterizationState = &raster;
    info.pMultisampleState = &multisample;
    info.pDepthStencilState = &depth;
    info.pColorBlendState = &blend;
    info.pDynamicState = &dynamic_state;
    info.layout = pipeline_layout;
    info.renderPass = render_pass;
    VkPipeline pipeline{};
    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1u, &info, nullptr, &pipeline) != VK_SUCCESS)
        return VK_NULL_HANDLE;
    pipelines.emplace(key, pipeline);
    last_pipeline_key = key;
    last_pipeline = pipeline;
    return pipeline;
}

bool VulkanRenderer::pump_events() {
    if (!impl_ || impl_->window == nullptr) return false;
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        // Closing the window is the only way out. Esc used to quit as well, but
        // Steam's desktop controller layout on a Steam Deck sends Esc from the B
        // button, which is also the game's confirm button, so confirming a menu
        // closed the game. Esc is reserved for the in-game menu instead.
        if (event.type == SDL_EVENT_QUIT || event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) impl_->quit = true;
        // Hot-plug is handled before the text-input branch below, which skips
        // every other event while the on-screen keyboard is up.
        if (event.type == SDL_EVENT_GAMEPAD_ADDED) impl_->open_gamepad(event.gdevice.which);
        if (event.type == SDL_EVENT_GAMEPAD_REMOVED) impl_->close_gamepad(event.gdevice.which);
        // F3 toggles the performance overlay. No pad combination: L3+R3 is
        // reserved for the in-game menu.
        if (event.type == SDL_EVENT_WINDOW_DISPLAY_CHANGED) impl_->update_display_info();
        if (event.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED) impl_->swapchain_dirty = true;
        // The mouse, while captured for the game. Releases always count.
        if (event.type == SDL_EVENT_MOUSE_MOTION && impl_->mouse_captured &&
            (!impl_->scripted_input || event.motion.which == kScriptedMouse)) {
            impl_->mouse_motion.x += event.motion.xrel;
            impl_->mouse_motion.y += event.motion.yrel;
        }
        if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN && impl_->mouse_captured && event.button.button < 32u &&
            (!impl_->scripted_input || event.button.which == kScriptedMouse))
            impl_->mouse_buttons |= 1u << event.button.button;
        if (event.type == SDL_EVENT_MOUSE_BUTTON_UP && event.button.button < 32u)
            impl_->mouse_buttons &= ~(1u << event.button.button);
        if (impl_->event_hook && impl_->event_hook(event)) continue;
        if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_F3 && !event.key.repeat && impl_->overlay_ready)
            impl_->overlay_visible = !impl_->overlay_visible;
    }
    // Only sample while the window has focus: a key still down when focus is
    // lost stays down in SDL's snapshot, which the guest sees as a held
    // direction it can never release.
    const bool focused = (SDL_GetWindowFlags(impl_->window) & SDL_WINDOW_INPUT_FOCUS) != 0u;
    impl_->update_pointer(focused);
    // While a menu or the on-screen keyboard is open, nothing reaches the game.
    if (!impl_->game_input) {
        impl_->pad = PadState{};
        return !impl_->quit;
    }

    // Keyboard and mouse buttons to the PSP pad, through the player's
    // bindings. Bits follow SceCtrlButtons; the stick is centred at 0x80.
    const settings::Settings &player = settings::current();
    const bool *keys = SDL_GetKeyboardState(nullptr);
    const input::PadInput typed = input::read(player.bindings, [&](input::Binding binding) {
        if (const int button = input::mouse_button_of(binding))
            return impl_->mouse_captured && (impl_->mouse_buttons & (1u << button)) != 0u;
        const int position = input::key_position(binding);
        return position >= 0 && ((focused && position < SDL_SCANCODE_COUNT && keys[position]) ||
                                 impl_->scripted_keys[static_cast<std::size_t>(position)]);
    });
    PadState pad{};
    pad.buttons = typed.buttons;
    int analog_x = typed.stick_x;
    int analog_y = typed.stick_y;
    // Camera keys push the second stick fully, as a right stick would: in
    // the D-pad mode they press the D-pad instead, and with it off, nothing.
    if (player.right_stick == settings::RightStick::Camera) {
        pad.right_x = static_cast<std::uint8_t>(0x80 + typed.camera_x);
        pad.right_y = static_cast<std::uint8_t>(0x80 + typed.camera_y);
    } else if (player.right_stick == settings::RightStick::DPad) {
        if (typed.camera_x < 0) pad.buttons |= 0x0080u;
        if (typed.camera_x > 0) pad.buttons |= 0x0020u;
        if (typed.camera_y < 0) pad.buttons |= 0x0010u;
        if (typed.camera_y > 0) pad.buttons |= 0x0040u;
    }

    // The gamepad adds to the same bits and offsets, so both sources are live.
    if (impl_->gamepad != nullptr) read_gamepad(impl_->gamepad, pad, analog_x, analog_y);

    pad.analog_x = static_cast<std::uint8_t>(std::clamp(0x80 + analog_x, 0, 255));
    pad.analog_y = static_cast<std::uint8_t>(std::clamp(0x80 + analog_y, 0, 255));

    // Unattended runs (overlay bootstrapping) press confirm periodically so the
    // game walks through title screens and dialogs on its own.
    static const std::uint64_t auto_confirm = [] {
        const char *text = std::getenv("MHP3RD_AUTO_CONFIRM");
        return text != nullptr ? std::strtoull(text, nullptr, 10) : 0ull;
    }();
    if (auto_confirm != 0u) {
        const std::uint64_t phase = impl_->frames % auto_confirm;
        if (phase < auto_confirm / 8u) pad.buttons |= 0x2000u;  // circle
    }
    // Buttons held when the game got its input back stay hidden from it until
    // they are released.
    if (impl_->suppress_held) {
        impl_->suppressed_buttons = pad.buttons;
        impl_->suppress_held = false;
    }
    impl_->suppressed_buttons &= pad.buttons;
    pad.buttons &= ~impl_->suppressed_buttons;
    if (pad_tuning().trace && (pad.buttons != impl_->pad.buttons || pad.analog_x != impl_->pad.analog_x ||
                               pad.analog_y != impl_->pad.analog_y || pad.right_x != impl_->pad.right_x ||
                               pad.right_y != impl_->pad.right_y)) {
        std::cout << "[pad] buttons 0x" << std::hex << pad.buttons << std::dec << " analog "
                  << static_cast<int>(pad.analog_x) << "," << static_cast<int>(pad.analog_y) << " right "
                  << static_cast<int>(pad.right_x) << "," << static_cast<int>(pad.right_y) << std::endl;
    }
    impl_->pad = pad;
    return !impl_->quit;
}

PadState VulkanRenderer::pad() const noexcept { return impl_ ? impl_->pad : PadState{}; }

MouseMotion VulkanRenderer::take_mouse_motion() noexcept {
    return impl_ ? std::exchange(impl_->mouse_motion, MouseMotion{}) : MouseMotion{};
}

bool VulkanRenderer::mouse_captured() const noexcept { return impl_ && impl_->mouse_captured; }

void VulkanRenderer::set_pointer_free(bool free) {
    if (impl_) impl_->pointer_free = free;
}

void VulkanRenderer::set_scripted_key(int position, bool down) {
    if (impl_ && position > 0 && position < static_cast<int>(input::kKeyPositions))
        impl_->scripted_keys[static_cast<std::size_t>(position)] = down;
}

void VulkanRenderer::set_scripted_input(bool scripted) {
    if (impl_) impl_->scripted_input = scripted;
}

void VulkanRenderer::set_event_hook(std::function<bool(const SDL_Event &)> hook) {
    if (impl_) impl_->event_hook = std::move(hook);
}

void VulkanRenderer::set_game_input(bool enabled) {
    if (!impl_) return;
    if (enabled && !impl_->game_input) impl_->suppress_held = true;
    impl_->game_input = enabled;
}

void VulkanRenderer::request_quit() noexcept {
    if (impl_) impl_->quit = true;
}

void VulkanRenderer::hold_frame(bool hold) {
    if (!impl_) return;
    Impl &impl = *impl_;
    if (!impl.ready || hold == impl.holding) return;
    if (!hold) {
        impl.holding = false;
        vkDeviceWaitIdle(impl.device);
        impl.destroy_target(impl.held);
        impl.held = {};
        return;
    }
    const auto shown = impl.targets.find(impl.presented_target);
    if (shown == impl.targets.end() || !shown->second.initialized) return;
    std::string error;
    if (!impl.create_image(impl.target_extent.width, impl.target_extent.height, VK_FORMAT_R8G8B8A8_UNORM,
                           VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                               VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                           impl.held.color, impl.held.color_memory, impl.held.color_view, VK_IMAGE_ASPECT_COLOR_BIT,
                           error)) {
        std::cerr << "Renderer: cannot hold the frame (" << error << ")\n";
        impl.destroy_target(impl.held);
        impl.held = {};
        return;
    }
    // The shown target and the copy both rest in the layout presenting
    // expects of a game frame.
    const VkImage source = shown->second.color;
    const VkImage copy_to = impl.held.color;
    const VkExtent2D extent = impl.target_extent;
    impl.run_commands([&](VkCommandBuffer commands) {
        impl.transition(commands, source, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
        impl.transition(commands, copy_to, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        VkImageCopy region{};
        region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 0u, 1u};
        region.dstSubresource = region.srcSubresource;
        region.extent = {extent.width, extent.height, 1u};
        vkCmdCopyImage(commands, source, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, copy_to,
                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1u, &region);
        impl.transition(commands, source, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        impl.transition(commands, copy_to, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    });
    impl.holding = true;
}

SDL_Window *VulkanRenderer::window() const noexcept { return impl_ ? impl_->window : nullptr; }
std::string VulkanRenderer::device_name() const { return impl_ ? impl_->device_name : std::string{}; }
SDL_Gamepad *VulkanRenderer::gamepad() const noexcept { return impl_ ? impl_->gamepad : nullptr; }

VkExtent2D VulkanRenderer::Impl::wanted_target_extent() const {
    const bool known = swapchain_extent.width != 0u && swapchain_extent.height != 0u;
    const double window_width = known ? swapchain_extent.width : static_cast<double>(target_extent.width);
    const double window_height = known ? swapchain_extent.height : static_cast<double>(target_extent.height);
    if (aspect != settings::Aspect::Fill) {
        std::uint32_t scale = requested_scale;
        if (scale == 0u) {
            // The smallest multiple that covers the picture on the window.
            const double x = window_width / kPspWidth;
            const double y = window_height / kPspHeight;
            const double cover = aspect == settings::Aspect::Original ? std::min(x, y) : std::max(x, y);
            scale = static_cast<std::uint32_t>(std::clamp(std::ceil(cover - 0.01), 1.0,
                                                          static_cast<double>(kMaxAutoLines / kPspHeight)));
        }
        return {kPspWidth * scale, kPspHeight * scale};
    }
    // Fill: the window's shape, 272 lines per step or the window's own lines.
    const double shape = std::clamp(window_width / std::max(window_height, 1.0), 0.25, 8.0);
    double lines = requested_scale != 0u ? static_cast<double>(kPspHeight * requested_scale)
                                         : std::clamp(window_height, static_cast<double>(kPspHeight),
                                                      static_cast<double>(kMaxAutoLines));
    lines = std::min(lines, kMaxTargetWidth / shape);
    const auto height = static_cast<std::uint32_t>(std::max(1.0, std::round(lines)));
    const auto width = static_cast<std::uint32_t>(
        std::clamp(std::round(lines * shape), 1.0, static_cast<double>(kMaxTargetWidth)));
    return {width, height};
}

bool VulkanRenderer::Impl::interface_fit(float &fit_x, float &fit_y) const {
    if (aspect != settings::Aspect::Fill) return false;
    const float x = static_cast<float>(target_extent.width) / static_cast<float>(kPspWidth);
    const float y = static_cast<float>(target_extent.height) / static_cast<float>(kPspHeight);
    const float square = std::min(x, y);
    fit_x = square / x;
    fit_y = square / y;
    return fit_x < 0.999f || fit_y < 0.999f;
}

void VulkanRenderer::Impl::follow_window() {
    if (!ready || recording) return;
    const VkExtent2D wanted = wanted_target_extent();
    if (wanted.width == target_extent.width && wanted.height == target_extent.height) {
        pending_frames = 0u;
        resize_now = false;
        return;
    }
    // The keyboard's held frame has the old size; a new window size waits
    // for it, a changed setting does not.
    if (holding && !resize_now) return;
    if (!resize_now) {
        if (wanted.width != pending_extent.width || wanted.height != pending_extent.height) {
            pending_extent = wanted;
            pending_frames = 0u;
        }
        if (++pending_frames < kSettleFrames) return;
    }
    pending_frames = 0u;
    resize_now = false;
    resize_targets(wanted);
}

void VulkanRenderer::Impl::resize_targets(VkExtent2D extent) {
    if (!ready || recording ||
        (extent.width == target_extent.width && extent.height == target_extent.height))
        return;
    // Every target is rebuilt at the new size with its current picture scaled
    // into it, so the paused frame behind the menu, and render-to-texture
    // targets the game reads back, stay intact.
    vkDeviceWaitIdle(device);
    // A held frame has the old size; let the game's own frames show again.
    if (holding) {
        holding = false;
        destroy_target(held);
        held = {};
    }
    std::map<std::uint32_t, Target> old_targets = std::move(targets);
    targets.clear();
    const VkExtent2D old_extent = target_extent;
    target_extent = extent;
    std::vector<std::pair<Target *, const Target *>> copies;
    for (const auto &[address, old_target] : old_targets) {
        std::string error;
        Target *target = target_for(address, error);
        if (target == nullptr) {
            std::cout << "[render] cannot resize a render target: " << error << "\n";
            continue;
        }
        target->stride = old_target.stride;
        target->format = old_target.format;
        target->last_drawn_frame = old_target.last_drawn_frame;
        target->draw_serial = old_target.draw_serial;
        target->guest_words = old_target.guest_words;
        if (old_target.initialized) copies.emplace_back(target, &old_target);
    }
    run_commands([&](VkCommandBuffer commands) {
        for (const auto &[target, old_target] : copies) {
            transition(commands, old_target->color, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                       VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
            transition(commands, target->color, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
            VkImageBlit blit{};
            blit.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 0u, 1u};
            blit.srcOffsets[1] = {static_cast<std::int32_t>(old_extent.width),
                                  static_cast<std::int32_t>(old_extent.height), 1};
            blit.dstSubresource = blit.srcSubresource;
            blit.dstOffsets[1] = {static_cast<std::int32_t>(extent.width), static_cast<std::int32_t>(extent.height), 1};
            vkCmdBlitImage(commands, old_target->color, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, target->color,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1u, &blit, VK_FILTER_LINEAR);
            transition(commands, target->color, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                       VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
            transition(commands, target->depth, VK_IMAGE_LAYOUT_UNDEFINED,
                       VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT);
            target->initialized = true;
        }
    });
    for (auto &[address, old_target] : old_targets) destroy_target(old_target);
    std::cout << "[render] internal resolution " << extent.width << "x" << extent.height << "\n";
}

void VulkanRenderer::set_internal_scale(std::uint32_t scale) {
    if (!impl_) return;
    impl_->requested_scale = std::min(scale, settings::kMaxInternalScale);
    impl_->resize_now = true;
    impl_->follow_window();
}

void VulkanRenderer::set_window_scale(std::uint32_t scale) {
    if (!impl_ || impl_->window == nullptr) return;
    scale = std::clamp<std::uint32_t>(scale, 1u, settings::kMaxWindowScale);
    if ((SDL_GetWindowFlags(impl_->window) & SDL_WINDOW_FULLSCREEN) != 0u) return;
    SDL_SetWindowSize(impl_->window, static_cast<int>(kPspWidth * scale), static_cast<int>(kPspHeight * scale));
    impl_->swapchain_dirty = true;
}

void VulkanRenderer::set_fullscreen(bool fullscreen) {
    if (!impl_ || impl_->window == nullptr) return;
    SDL_SetWindowFullscreen(impl_->window, fullscreen);
    impl_->swapchain_dirty = true;
}

void VulkanRenderer::set_present_mode(settings::PresentMode mode) {
    if (!impl_) return;
    impl_->requested_present = mode;
    impl_->swapchain_dirty = true;
}

bool VulkanRenderer::supports_present_mode(settings::PresentMode mode) const {
    if (!impl_) return false;
    VkPresentModeKHR wanted = VK_PRESENT_MODE_FIFO_KHR;
    if (mode == settings::PresentMode::Mailbox) wanted = VK_PRESENT_MODE_MAILBOX_KHR;
    if (mode == settings::PresentMode::Immediate) wanted = VK_PRESENT_MODE_IMMEDIATE_KHR;
    const auto &modes = impl_->present_modes;
    return wanted == VK_PRESENT_MODE_FIFO_KHR || std::find(modes.begin(), modes.end(), wanted) != modes.end();
}

void VulkanRenderer::set_aspect(settings::Aspect aspect) {
    if (!impl_) return;
    impl_->aspect = aspect;
    impl_->resize_now = true;
    impl_->follow_window();
}

float VulkanRenderer::game_aspect() const noexcept {
    if (!impl_ || impl_->aspect != settings::Aspect::Fill || impl_->target_extent.height == 0u)
        return static_cast<float>(kPspWidth) / static_cast<float>(kPspHeight);
    return static_cast<float>(impl_->target_extent.width) / static_cast<float>(impl_->target_extent.height);
}

std::array<std::uint32_t, 2> VulkanRenderer::target_size() const noexcept {
    if (!impl_) return {kPspWidth, kPspHeight};
    return {impl_->target_extent.width, impl_->target_extent.height};
}

void VulkanRenderer::set_sharp_screen(bool sharp) {
    if (impl_) impl_->sharp_screen = sharp;
}

void VulkanRenderer::set_texture_pack(bool enabled) {
    // Applied at the next begin_frame(), where no frame is being recorded.
    if (impl_) impl_->pack_wanted = enabled;
}

std::string VulkanRenderer::texture_pack_status() const {
    if (!impl_) return {};
    const Impl &impl = *impl_;
    if (impl.pack_held) return "Updating";
    if (impl.pack_wanted != impl.pack_applied || impl.pack_reload) return impl.pack_wanted ? "Loading" : "Off";
    if (!impl.pack) return impl.pack_status;
    return impl.pack_status + ", " + std::to_string(impl.replacements.resident_count()) + " on the GPU (" +
           std::to_string(impl.replacements.resident_bytes() >> 20u) + " MB)";
}

void VulkanRenderer::reload_texture_pack() {
    if (impl_) impl_->pack_reload = true;
}

void VulkanRenderer::hold_texture_pack(bool hold) {
    if (impl_) impl_->pack_held = hold;
}

bool VulkanRenderer::texture_pack_held() const {
    return impl_ && impl_->pack_held && !impl_->pack_applied && !impl_->pack;
}

std::filesystem::path VulkanRenderer::textures_root() { return install::user_data_directory() / "textures"; }

std::string VulkanRenderer::texture_pack_folder() const {
    const std::string in_place = settings::current().texture_pack_folder;
    return install::path_to_utf8(texture_pack_location(textures_root(), install::kDiscId, in_place).folder);
}

void VulkanRenderer::set_sharp_textures(bool sharp) {
    Impl &impl = *impl_;
    if (!impl.ready || impl.recording || impl.sharp_textures == sharp) return;
    impl.sharp_textures = sharp;
    // The sampler is part of each texture's descriptor; rewrite them all.
    vkDeviceWaitIdle(impl.device);
    const auto rewrite = [&](Impl::Texture &texture) {
        if (texture.descriptor == VK_NULL_HANDLE) return;
        VkDescriptorImageInfo image_info{impl.texture_sampler(), texture.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
        VkWriteDescriptorSet write{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
        write.dstSet = texture.descriptor;
        write.descriptorCount = 1u;
        write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        write.pImageInfo = &image_info;
        vkUpdateDescriptorSets(impl.device, 1u, &write, 0u, nullptr);
    };
    rewrite(impl.white_texture);
    for (auto &[key, texture] : impl.textures) rewrite(texture);
    impl.replacements.set_sharp(sharp);
    for (auto &[address, target] : impl.targets) {
        for (std::size_t i = 0; i < target.copy_descriptors.size(); ++i) {
            if (target.copy_descriptors[i] == VK_NULL_HANDLE) continue;
            VkDescriptorImageInfo image_info{impl.framebuffer_sampler(), i == 0u ? target.copy_view : target.copy_opaque_view,
                                             VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
            VkWriteDescriptorSet write{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
            write.dstSet = target.copy_descriptors[i];
            write.descriptorCount = 1u;
            write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            write.pImageInfo = &image_info;
            vkUpdateDescriptorSets(impl.device, 1u, &write, 0u, nullptr);
        }
    }
}

void VulkanRenderer::set_perf_overlay(bool visible) {
    if (impl_) impl_->overlay_visible = impl_->overlay_ready && visible;
}

void VulkanRenderer::capture_window(const std::string &path) {
    if (!impl_ || !impl_->ready) return;
    if ((impl_->swapchain_usage & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) == 0u) {
        std::cout << "[render] this swapchain cannot be read back; no window capture\n";
        return;
    }
    impl_->capture_path = path;
}

bool VulkanRenderer::initialize_ui(std::string &error) {
    Impl &impl = *impl_;
    if (!impl.ready) {
        error = "no renderer";
        return false;
    }
    if (impl.ui_ready) return true;
    // Loads what the game frame and the overlay left in the swapchain image
    // and draws over it; submit_and_present moves it on to presentation.
    VkAttachmentDescription attachment{};
    attachment.format = impl.swapchain_format;
    attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    VkAttachmentReference reference{0u, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1u;
    subpass.pColorAttachments = &reference;
    VkRenderPassCreateInfo pass_info{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
    pass_info.attachmentCount = 1u;
    pass_info.pAttachments = &attachment;
    pass_info.subpassCount = 1u;
    pass_info.pSubpasses = &subpass;
    if (!check(vkCreateRenderPass(impl.device, &pass_info, nullptr, &impl.ui_render_pass), "vkCreateRenderPass",
               error))
        return false;
    if (!impl.create_ui_framebuffers(error)) return false;

    ImGui_ImplVulkan_InitInfo info{};
    info.ApiVersion = VK_API_VERSION_1_1;
    info.Instance = impl.instance;
    info.PhysicalDevice = impl.physical_device;
    info.Device = impl.device;
    info.QueueFamily = impl.queue_family;
    info.Queue = impl.queue;
    info.DescriptorPoolSize = IMGUI_IMPL_VULKAN_MINIMUM_SAMPLED_IMAGE_POOL_SIZE;
    info.MinImageCount = impl.swapchain_min_images;
    info.ImageCount = std::max<std::uint32_t>(static_cast<std::uint32_t>(impl.swapchain_images.size()),
                                              impl.swapchain_min_images);
    info.PipelineInfoMain.RenderPass = impl.ui_render_pass;
    info.PipelineInfoMain.Subpass = 0u;
    info.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    if (!ImGui_ImplVulkan_Init(&info)) {
        error = "ImGui_ImplVulkan_Init failed";
        return false;
    }
    impl.ui_ready = true;
    return true;
}

void VulkanRenderer::shutdown_ui() {
    Impl &impl = *impl_;
    if (!impl.ui_ready) return;
    vkDeviceWaitIdle(impl.device);
    ImGui_ImplVulkan_Shutdown();
    impl.ui_ready = false;
    for (VkFramebuffer framebuffer : impl.ui_framebuffers) vkDestroyFramebuffer(impl.device, framebuffer, nullptr);
    impl.ui_framebuffers.clear();
    vkDestroyRenderPass(impl.device, impl.ui_render_pass, nullptr);
    impl.ui_render_pass = VK_NULL_HANDLE;
}

void VulkanRenderer::begin_ui_frame() {
    if (impl_ && impl_->ui_ready) ImGui_ImplVulkan_NewFrame();
}

void VulkanRenderer::set_ui_draw_data(ImDrawData *draw_data) {
    if (impl_) impl_->ui_draw_data = draw_data;
}

void VulkanRenderer::present_ui(bool show_game) {
    Impl &impl = *impl_;
    if (!impl.ready) return;
    if (!impl.recording) begin_frame();
    impl.end_pass();
    VkImage source = VK_NULL_HANDLE;
    if (show_game) {
        if (const auto shown = impl.targets.find(impl.presented_target); shown != impl.targets.end())
            source = shown->second.color;
    }
    impl.submit_and_present(source, false);
    impl.follow_window();
}

void VulkanRenderer::begin_frame() {
    Impl &impl = *impl_;
    if (!impl.ready || impl.recording) return;
    const perf::Clock::time_point wait_start = perf::Clock::now();
    vkWaitForFences(impl.device, 1u, &impl.frame_fence, VK_TRUE, UINT64_MAX);
    perf::add_wait_time(perf::Clock::now() - wait_start, perf::Stall::Fence);
    vkResetFences(impl.device, 1u, &impl.frame_fence);
    impl.collect_gpu_time();
    impl.apply_texture_pack();
    impl.replacements.begin_frame(impl.frames);
    if (texture_pack_trace() && impl.pack && impl.frames % 60u == 0u) {
        std::cout << "[texpack] frame " << impl.frames << ": " << impl.replaced_draws << " draws replaced in 60 frames, "
                  << impl.replacements.resident_count() << " images on the GPU ("
                  << (impl.replacements.resident_bytes() >> 20u) << " MB)\n";
        impl.replaced_draws = 0u;
    }
    if (impl.writeback_in_flight) {
        const perf::Clock::time_point copy_start = perf::Clock::now();
        impl.writeback_pixels.resize(static_cast<std::size_t>(kPspWidth) * kPspHeight);
        std::memcpy(impl.writeback_pixels.data(), impl.writeback_mapped, impl.writeback_pixels.size() * 4u);
        impl.writeback_ready = impl.writeback_recorded;
        impl.writeback_has_pixels = true;
        impl.writeback_in_flight = false;
        perf::note_stall(perf::Stall::Copy, perf::Clock::now() - copy_start);
    }
    vkResetCommandBuffer(impl.command_buffer, 0u);
    VkCommandBufferBeginInfo begin{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(impl.command_buffer, &begin);
    if (impl.gpu_timer != VK_NULL_HANDLE) {
        vkCmdResetQueryPool(impl.command_buffer, impl.gpu_timer, 0u, 2u * kGpuTimerSegments);
        impl.gpu_timer_used = 0u;
        impl.gpu_timer_open = false;
        impl.begin_gpu_segment(impl.command_buffer);
    }
    impl.vertex_offset = 0u;
    impl.environment_version = 0u;
    impl.object_valid = false;
    impl.forget_bindings();
    impl.pass_active = false;
    impl.recording = true;
}

void VulkanRenderer::upload_frame(std::uint32_t display_address, const std::uint8_t *pixels, std::uint32_t width,
                                  std::uint32_t height, std::uint32_t stride) {
    Impl &impl = *impl_;
    if (!impl.ready || pixels == nullptr || width == 0u || height == 0u || stride < width) return;
    if (!impl.recording) begin_frame();
    impl.end_pass();
    std::string error;
    if (impl.upload_extent.width != width || impl.upload_extent.height != height) {
        // Nothing recorded so far this frame uses the old image yet.
        const perf::Clock::time_point idle_start = perf::Clock::now();
        vkDeviceWaitIdle(impl.device);
        perf::note_stall(perf::Stall::Idle, perf::Clock::now() - idle_start);
        if (!impl.create_upload(width, height, error)) {
            std::cerr << "Renderer: cannot upload frames (" << error << ")\n";
            impl.destroy_upload();
            return;
        }
    }
    Impl::Target *target = impl.target_for(display_address, error);
    if (target == nullptr) return;

    // The staging buffer is free: the frame fence was waited on in begin_frame.
    auto *staging = static_cast<std::uint8_t *>(impl.upload_mapped);
    for (std::uint32_t row = 0; row < height; ++row)
        std::memcpy(staging + static_cast<std::size_t>(row) * width * 4u,
                    pixels + static_cast<std::size_t>(row) * stride * 4u, static_cast<std::size_t>(width) * 4u);
    impl.transition(impl.command_buffer, impl.upload_image, VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    VkBufferImageCopy copy{};
    copy.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 0u, 1u};
    copy.imageExtent = {width, height, 1u};
    vkCmdCopyBufferToImage(impl.command_buffer, impl.upload_staging, impl.upload_image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1u, &copy);
    impl.transition(impl.command_buffer, impl.upload_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    // A new target has no contents to keep; an old one rests in the layout
    // the render pass expects.
    impl.transition(impl.command_buffer, target->color,
                    target->initialized ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    if (!target->initialized) {
        impl.transition(impl.command_buffer, target->depth, VK_IMAGE_LAYOUT_UNDEFINED,
                        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT);
        target->initialized = true;
    }
    VkImageBlit blit{};
    blit.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 0u, 1u};
    blit.srcOffsets[1] = {static_cast<std::int32_t>(width), static_cast<std::int32_t>(height), 1};
    blit.dstSubresource = blit.srcSubresource;
    blit.dstOffsets[1] = {static_cast<std::int32_t>(impl.target_extent.width),
                          static_cast<std::int32_t>(impl.target_extent.height), 1};
    // A movie has the PSP's shape: under Fill it keeps it, with black beside
    // (or above and below) it.
    float fit_x = 1.0f, fit_y = 1.0f;
    if (impl.interface_fit(fit_x, fit_y)) {
        const auto inset_x = static_cast<std::int32_t>(std::lround(0.5f * (1.0f - fit_x) * impl.target_extent.width));
        const auto inset_y = static_cast<std::int32_t>(std::lround(0.5f * (1.0f - fit_y) * impl.target_extent.height));
        blit.dstOffsets[0] = {inset_x, inset_y, 0};
        blit.dstOffsets[1] = {static_cast<std::int32_t>(impl.target_extent.width) - inset_x,
                              static_cast<std::int32_t>(impl.target_extent.height) - inset_y, 1};
        const VkClearColorValue black{{0.0f, 0.0f, 0.0f, 1.0f}};
        const VkImageSubresourceRange range{VK_IMAGE_ASPECT_COLOR_BIT, 0u, 1u, 0u, 1u};
        vkCmdClearColorImage(impl.command_buffer, target->color, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &black, 1u,
                             &range);
        impl.transition(impl.command_buffer, target->color, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    }
    vkCmdBlitImage(impl.command_buffer, impl.upload_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, target->color,
                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1u, &blit, VK_FILTER_LINEAR);
    impl.transition(impl.command_buffer, target->color, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    impl.last_drawn_target = display_address;
    // The frame came from guest memory, which texturing reads correctly anyway.
    ++target->draw_serial;
    target->guest_words.clear();
}

void VulkanRenderer::begin_display_list() {
    if (!impl_) return;
    impl_->list_texture_keys.clear();
    impl_->last_texture = nullptr;
}

void VulkanRenderer::submit(const DrawCall &call, const GuestMemory &memory) {
    Impl &impl = *impl_;
    if (!impl.ready) return;
    if (!impl.recording) begin_frame();
    if (call.vertices.empty()) return;

    // Everything becomes a triangle list; sprites expand to two triangles.
    impl.scratch.clear();
    // A vertex format without a colour field leaves every vertex white, and the
    // GE supplies the colour from the material registers instead. That is where
    // the marker over an NPC's head and the shadow blobs under characters get
    // both their colour and the alpha that makes them faint.
    //
    // Only unlit draws, though. Lighting on means the material colour is one term
    // of a sum the lights complete, which the vertex shader evaluates; handing
    // it over as the finished colour turned every character a flat muddy brown.
    // MHP3RD_NO_LIGHTING leaves lit geometry with the old white stand-in and
    // turns fog off too, which is how everything was drawn before either
    // existed; MHP3RD_NO_FOG turns off fog alone.
    static const bool no_material_color = std::getenv("MHP3RD_NO_MATERIAL_COLOR") != nullptr;
    static const bool no_lighting = std::getenv("MHP3RD_NO_LIGHTING") != nullptr;
    static const bool no_fog = no_lighting || std::getenv("MHP3RD_NO_FOG") != nullptr;
    const bool lit = call.lighting_enabled && !no_lighting && !call.through && !call.clear_mode;
    const bool use_material_color = !no_material_color && !call.has_vertex_color && !call.lighting_enabled;
    const auto to_gpu = [&](const Vertex &vertex) {
        GpuVertex out{};
        out.x = vertex.position[0];
        out.y = vertex.position[1];
        out.z = vertex.position[2];
        out.u = vertex.texcoord[0];
        out.v = vertex.texcoord[1];
        out.color = use_material_color ? call.material_color : vertex.color;
        out.nx = vertex.normal[0];
        out.ny = vertex.normal[1];
        out.nz = vertex.normal[2];
        return out;
    };
    const auto push_vertex = [&](const Vertex &vertex) { impl.scratch.push_back(to_gpu(vertex)); };
    const auto vertex_at = [&](std::size_t index) -> const Vertex & {
        if (!call.indices.empty()) {
            const std::size_t mapped = call.indices[index];
            return call.vertices[std::min(mapped, call.vertices.size() - 1u)];
        }
        return call.vertices[std::min(index, call.vertices.size() - 1u)];
    };
    const std::size_t count = call.indices.empty() ? call.vertices.size() : call.indices.size();

    static const bool trace = std::getenv("MHP3RD_TRACE_GE") != nullptr;
    static const bool trace3d = std::getenv("MHP3RD_TRACE_3D") != nullptr;
    // MHP3RD_TRACE_SPRITES=N: every through-mode sprite of frame N, with the
    // texture state it samples, to find the tiles a 2D screen is built from.
    static const std::uint64_t trace_sprites_frame = [] {
        const char *text = std::getenv("MHP3RD_TRACE_SPRITES");
        return text != nullptr ? std::strtoull(text, nullptr, 10) : ~0ull;
    }();

    // Transformed triangles, strips and fans go straight into the vertex
    // buffer: each decoded vertex once, converted as it is written, and a
    // 16-bit index list in the order the expansion below writes vertices in,
    // so the GPU draws the same triangles from the same vertex data without
    // the copies and the repeated strip vertices. MHP3RD_NO_DIRECT_VERTICES
    // expands every draw as before; the traces that read expanded vertices
    // keep the expansion too.
    static const bool legacy_vertices = std::getenv("MHP3RD_NO_DIRECT_VERTICES") != nullptr;
    static const bool check_direct = std::getenv("MHP3RD_CHECK_DIRECT_VERTICES") != nullptr;
    const bool direct = !legacy_vertices && !perf::alternate_off(perf::NewPath::Direct) && !call.through &&
                        (call.primitive == PrimitiveType::Triangles ||
                         call.primitive == PrimitiveType::TriangleStrip ||
                         call.primitive == PrimitiveType::TriangleFan) &&
                        !trace && !trace3d && impl.frames != trace_sprites_frame;

    const auto expand = [&]() -> bool {
        switch (call.primitive) {
        case PrimitiveType::Triangles:
            for (std::size_t i = 0; i + 2u < count; i += 3u) {
                push_vertex(vertex_at(i));
                push_vertex(vertex_at(i + 1u));
                push_vertex(vertex_at(i + 2u));
            }
            break;
        case PrimitiveType::TriangleStrip:
            for (std::size_t i = 0; i + 2u < count; ++i) {
                const bool odd = (i & 1u) != 0u;
                push_vertex(vertex_at(i));
                push_vertex(vertex_at(odd ? i + 2u : i + 1u));
                push_vertex(vertex_at(odd ? i + 1u : i + 2u));
            }
            break;
        case PrimitiveType::TriangleFan:
            for (std::size_t i = 1u; i + 1u < count; ++i) {
                push_vertex(vertex_at(0));
                push_vertex(vertex_at(i));
                push_vertex(vertex_at(i + 1u));
            }
            break;
        case PrimitiveType::Sprites:
            // Vertex pairs describe the opposite corners of a rectangle.
            for (std::size_t i = 0; i + 1u < count; i += 2u) {
                Vertex a = vertex_at(i);
                const Vertex &b = vertex_at(i + 1u);
                // The GE flat-shades sprites from the second vertex: its depth (and
                // colour) cover the whole rectangle. sceGuClear relies on this, as
                // its first vertex carries z = 0 and only the second the clear depth.
                a.position[2] = b.position[2];
                a.color = b.color;
                a.normal = b.normal;
                Vertex top_right = b;
                top_right.position[1] = a.position[1];
                top_right.texcoord[1] = a.texcoord[1];
                Vertex bottom_left = a;
                bottom_left.position[1] = b.position[1];
                bottom_left.texcoord[1] = b.texcoord[1];
                push_vertex(a);
                push_vertex(top_right);
                push_vertex(b);
                push_vertex(a);
                push_vertex(b);
                push_vertex(bottom_left);
            }
            break;
        default:
            return false;  // points and lines are not drawn yet
        }
        return true;
    };
    if (direct) {
        triangle_indices(call.primitive, count, call.indices, call.vertices.size(), impl.direct_indices);
        if (impl.direct_indices.empty()) return;
        if (check_direct) {
            // Every index must name a vertex equal, byte for byte, to the one
            // the expansion puts in its place.
            if (!expand()) return;
            ++impl.direct_checked;
            bool same = impl.scratch.size() == impl.direct_indices.size();
            for (std::size_t i = 0; same && i < impl.scratch.size(); ++i) {
                const GpuVertex converted = to_gpu(call.vertices[impl.direct_indices[i]]);
                same = std::memcmp(&converted, &impl.scratch[i], sizeof(GpuVertex)) == 0;
            }
            if (!same && ++impl.direct_mismatched <= 20u)
                std::cout << "[direct-check] draw " << impl.draws << " prim=" << static_cast<int>(call.primitive)
                          << " count=" << count << " vertices=" << call.vertices.size() << " expanded "
                          << impl.scratch.size() << " indices " << impl.direct_indices.size() << " differ\n";
        }
    } else {
        if (!expand()) return;
        if (impl.scratch.empty()) return;
    }

    // Through-mode vertices carry the texel range they may sample in the
    // otherwise unused normal and w; see clamp_through_quad().
    // MHP3RD_NO_SPRITE_CLAMP lets them sample anywhere, as before.
    if (call.through) {
        for (GpuVertex &vertex : impl.scratch) set_uv_rect(vertex, -kNoClamp, -kNoClamp, kNoClamp, kNoClamp);
        static const bool no_sprite_clamp = std::getenv("MHP3RD_NO_SPRITE_CLAMP") != nullptr;
        const bool quads = call.primitive == PrimitiveType::Sprites ||
                           ((call.primitive == PrimitiveType::TriangleStrip ||
                             call.primitive == PrimitiveType::TriangleFan) &&
                            count == 4u);
        if (!no_sprite_clamp && call.texture.enabled && !call.clear_mode && quads) {
            for (std::size_t first = 0; first + 6u <= impl.scratch.size(); first += 6u)
                clamp_through_quad(impl.scratch, first, static_cast<float>(call.texture.width),
                                   static_cast<float>(call.texture.height));
        }
    }

    if (trace && impl.draws < 400u) {
        const GpuVertex &first = impl.scratch.front();
        const GpuVertex &second = impl.scratch[std::min<std::size_t>(1u, impl.scratch.size() - 1u)];
        std::cout << "[ge] prim=" << static_cast<int>(call.primitive) << (call.through ? " through" : " transform")
                  << " verts=" << impl.scratch.size() << " p0=(" << first.x << "," << first.y << "," << first.z
                  << ") p1=(" << second.x << "," << second.y << ") uv0=(" << first.u << "," << first.v << ") uv1=("
                  << second.u << "," << second.v << ") tex=" << (call.texture.enabled ? 1 : 0)
                  << " fmt=" << static_cast<int>(call.texture.format) << " size=" << call.texture.width << "x"
                  << call.texture.height << " bufw=" << call.texture.buffer_width
                  << " swizzled=" << (call.texture.swizzled ? 1 : 0) << " addr=0x" << std::hex
                  << call.texture.address << " target=0x" << call.target.color_address << std::dec
                  << " stride=" << call.target.color_stride << " blend=" << (call.blend.enabled ? 1 : 0) << "\n";
    }


    if (impl.frames == trace_sprites_frame && call.primitive != PrimitiveType::Sprites) {
        float x0 = 1e9f, y0 = 1e9f, x1 = -1e9f, y1 = -1e9f, u0 = 1e9f, v0 = 1e9f, u1 = -1e9f, v1 = -1e9f;
        for (std::size_t i = 0; i < count; ++i) {
            const Vertex &v = vertex_at(i);
            x0 = std::min(x0, v.position[0]); x1 = std::max(x1, v.position[0]);
            y0 = std::min(y0, v.position[1]); y1 = std::max(y1, v.position[1]);
            u0 = std::min(u0, v.texcoord[0]); u1 = std::max(u1, v.texcoord[0]);
            v0 = std::min(v0, v.texcoord[1]); v1 = std::max(v1, v.texcoord[1]);
        }
        std::cout << "[sprite] other prim=" << static_cast<int>(call.primitive) << (call.through ? " through" : " transform")
                  << " n=" << count << " pos=(" << x0 << "," << y0 << ")-(" << x1 << "," << y1 << ") uv=(" << u0
                  << "," << v0 << ")-(" << u1 << "," << v1 << ") tex=" << (call.texture.enabled ? 1 : 0) << " 0x"
                  << std::hex << call.texture.address << std::dec << " " << call.texture.width << "x"
                  << call.texture.height << " fmt=" << static_cast<int>(call.texture.format)
                  << " filter=" << call.texture.min_filter << "/" << call.texture.mag_filter << "\n";
    }
    if (call.through && call.primitive == PrimitiveType::Sprites && impl.frames == trace_sprites_frame) {
        for (std::size_t i = 0; i + 1u < count; i += 2u) {
            const Vertex &a = vertex_at(i);
            const Vertex &b = vertex_at(i + 1u);
            std::cout << "[sprite] pos=(" << a.position[0] << "," << a.position[1] << ")-(" << b.position[0] << ","
                      << b.position[1] << ") uv=(" << a.texcoord[0] << "," << a.texcoord[1] << ")-("
                      << b.texcoord[0] << "," << b.texcoord[1] << ") tex=" << (call.texture.enabled ? 1 : 0)
                      << " 0x" << std::hex << call.texture.address << std::dec << " " << call.texture.width << "x"
                      << call.texture.height << " fmt=" << static_cast<int>(call.texture.format)
                      << " filter=" << call.texture.min_filter << "/" << call.texture.mag_filter
                      << " wrap=" << call.texture.wrap_s << "/" << call.texture.wrap_t
                      << " blend=" << (call.blend.enabled ? 1 : 0) << " color=0x" << std::hex << b.color << std::dec
                      << "\n";
        }
    }

    // Deep dump of the first transformed draws: matrices, raw positions and the
    // same positions after a CPU-side transform, so a geometry that never shows
    // up can be traced to the stage that loses it.
    static const bool trace_camera = std::getenv("MHP3RD_TRACE_CAMERA") != nullptr;
    // The camera hunt reads the same measurement without printing it.
    static const bool watch_camera = trace_camera || std::getenv("MHP3RD_FIND_CAMERA") != nullptr;
    static std::uint32_t traced_3d = 0u;
    static std::uint32_t traced_clears = 0u;
    if (trace3d && call.clear_mode && traced_clears < 4u) {
        ++traced_clears;
        const GpuVertex &first = impl.scratch.front();
        const GpuVertex &last = impl.scratch.back();
        std::cout << "[3d] clear#" << traced_clears << " flags=" << call.clear_flags
                  << " (colour=" << ((call.clear_flags & 1u) != 0u) << " alpha=" << ((call.clear_flags & 2u) != 0u)
                  << " depth=" << ((call.clear_flags & 4u) != 0u) << ") through=" << (call.through ? 1 : 0)
                  << " verts=" << impl.scratch.size() << " z=" << first.z << ".." << last.z << "\n";
    }
    if (trace3d && !call.through && traced_3d < 12u) {
        ++traced_3d;
        const auto wvp = multiply(call.projection, multiply(call.view, call.world));
        const auto dump = [](const char *name, const std::array<float, 16> &m) {
            std::cout << "  " << name << " =";
            for (std::uint32_t row = 0; row < 4u; ++row) {
                std::cout << " [";
                for (std::uint32_t col = 0; col < 4u; ++col)
                    std::cout << (col ? " " : "") << m[col * 4u + row];
                std::cout << "]";
            }
            std::cout << "\n";
        };
        std::cout << "[3d] draw#" << traced_3d << " prim=" << static_cast<int>(call.primitive)
                  << " vtype=0x" << std::hex << call.vertex_type << std::dec << " verts=" << impl.scratch.size()
                  << " clear=" << (call.clear_mode ? 1 : 0) << "/" << call.clear_flags
                  << " ztest=" << (call.depth.test_enabled ? 1 : 0) << " zfunc=" << call.depth.function
                  << " zwrite=" << (call.depth.write_enabled ? 1 : 0) << " zrange=[" << call.depth.range_near << ","
                  << call.depth.range_far << "]"
                  << " cull=" << (call.culling_enabled ? 1 : 0) << " cw=" << (call.cull_clockwise ? 1 : 0)
                  << " blend=" << (call.blend.enabled ? 1 : 0) << " tex=" << (call.texture.enabled ? 1 : 0) << "\n";
        std::cout << "  viewport scale=(" << call.viewport.x_scale << "," << call.viewport.y_scale << ","
                  << call.viewport.z_scale << ") offset=(" << call.viewport.x_offset << "," << call.viewport.y_offset
                  << "," << call.viewport.z_offset << ") region=(" << call.viewport.offset_x << ","
                  << call.viewport.offset_y << ") scissor=(" << call.viewport.scissor_x1 << ","
                  << call.viewport.scissor_y1 << ")-(" << call.viewport.scissor_x2 << "," << call.viewport.scissor_y2
                  << ")\n";
        dump("world", call.world);
        dump("view ", call.view);
        dump("proj ", call.projection);
        dump("wvp  ", wvp);
        for (std::size_t i = 0; i < std::min<std::size_t>(3u, impl.scratch.size()); ++i) {
            const GpuVertex &v = impl.scratch[i];
            float clip[4]{};
            for (std::uint32_t row = 0; row < 4u; ++row)
                clip[row] = wvp[0 * 4u + row] * v.x + wvp[1 * 4u + row] * v.y + wvp[2 * 4u + row] * v.z +
                            wvp[3 * 4u + row];
            std::cout << "  v" << i << " obj=(" << v.x << "," << v.y << "," << v.z << ") clip=(" << clip[0] << ","
                      << clip[1] << "," << clip[2] << "," << clip[3] << ")";
            if (clip[3] != 0.0f)
                std::cout << " ndc=(" << clip[0] / clip[3] << "," << clip[1] / clip[3] << "," << clip[2] / clip[3]
                          << ")";
            std::cout << "\n";
        }
    }

    if (call.through) {
        ++impl.frame_through_draws;
    } else {
        ++impl.frame_transformed_draws;
        ++impl.frame_transformed_targets[call.target.color_address];
        if (watch_camera) {
            const auto same = std::find_if(impl.frame_views.begin(), impl.frame_views.end(),
                                           [&](const auto &entry) { return entry.first == call.view; });
            const auto vertices =
                static_cast<std::uint32_t>(direct ? impl.direct_indices.size() : impl.scratch.size());
            if (same == impl.frame_views.end())
                impl.frame_views.emplace_back(call.view, vertices);
            else
                same->second += vertices;
        }
        if (trace3d) {
            // Where does this frame's transformed geometry actually land? A
            // bounding box in normalised device coordinates separates "clipped
            // away" from "drawn but invisible".
            const auto wvp = multiply(call.projection, multiply(call.view, call.world));
            for (const GpuVertex &v : impl.scratch) {
                float clip[4]{};
                for (std::uint32_t row = 0; row < 4u; ++row)
                    clip[row] = wvp[0 * 4u + row] * v.x + wvp[1 * 4u + row] * v.y + wvp[2 * 4u + row] * v.z +
                                wvp[3 * 4u + row];
                if (clip[3] <= 0.0f) {
                    ++impl.frame_behind_camera;
                    continue;
                }
                const float ndc[3]{clip[0] / clip[3], clip[1] / clip[3], clip[2] / clip[3]};
                for (std::uint32_t axis = 0; axis < 3u; ++axis) {
                    impl.frame_ndc_min[axis] = std::min(impl.frame_ndc_min[axis], ndc[axis]);
                    impl.frame_ndc_max[axis] = std::max(impl.frame_ndc_max[axis], ndc[axis]);
                }
                if (ndc[0] >= -1.0f && ndc[0] <= 1.0f && ndc[1] >= -1.0f && ndc[1] <= 1.0f && ndc[2] >= -1.0f &&
                    ndc[2] <= 1.0f)
                    ++impl.frame_onscreen_vertices;
                ++impl.frame_transformed_vertices;
            }
        }
    }

    // Lighting and fog read the environment block; lit draws also read an
    // object block. Each is written into the vertex buffer only when it
    // changed, and the vertices follow whatever was written.
    const bool fogged = call.fog.enabled && !no_fog && !call.through && !call.clear_mode;
    const auto view_world = multiply(call.view, call.world);
    if ((lit || fogged) && impl.environment_version != call.environment_version) {
        const LightingState &state = call.lighting;
        EnvironmentBlock environment{};
        environment.ambient = unpack_color(state.ambient_color, static_cast<float>(state.ambient_alpha) / 255.0f);
        environment.fog = {call.fog.end, call.fog.scale, 0.0f, 0.0f};
        environment.fog_color = unpack_color(call.fog.color);
        for (std::size_t i = 0; i < state.lights.size(); ++i) {
            const LightState &light = state.lights[i];
            environment.light_position[i] = {light.position[0], light.position[1], light.position[2],
                                             light.enabled ? 1.0f : 0.0f};
            environment.light_direction[i] = {light.direction[0], light.direction[1], light.direction[2],
                                              static_cast<float>(light.type)};
            environment.light_attenuation[i] = {light.attenuation[0], light.attenuation[1], light.attenuation[2],
                                                static_cast<float>(light.kind)};
            environment.light_spot[i] = {light.spot_exponent, light.spot_cutoff, 0.0f, 0.0f};
            environment.light_ambient[i] = unpack_color(light.ambient);
            environment.light_diffuse[i] = unpack_color(light.diffuse);
            environment.light_specular[i] = unpack_color(light.specular);
        }
        if (!impl.write_uniform(&environment, sizeof(environment), impl.environment_offset)) return;
        impl.environment_version = call.environment_version;
    }
    if (lit) {
        const LightingState &state = call.lighting;
        if (impl.material_version != call.material_version) {
            impl.material[0] = unpack_color(state.material_emissive, state.specular_power);
            impl.material[1] =
                unpack_color(call.material_color, static_cast<float>(call.material_color >> 24u) / 255.0f);
            impl.material[2] = unpack_color(state.material_diffuse, static_cast<float>(state.mode));
            impl.material[3] = unpack_color(state.material_specular, state.reverse_normals ? 1.0f : 0.0f);
            impl.material_version = call.material_version;
        }
        ObjectBlock object{};
        object.world = call.world;
        object.flags = {1.0f, call.has_vertex_color ? 1.0f : 0.0f, 0.0f, static_cast<float>(state.material_update)};
        object.emissive = impl.material[0];
        object.material_ambient = impl.material[1];
        object.material_diffuse = impl.material[2];
        object.material_specular = impl.material[3];
        if (!impl.object_valid || std::memcmp(&object, &impl.last_object, sizeof(object)) != 0) {
            if (!impl.write_uniform(&object, sizeof(object), impl.object_offset)) return;
            impl.last_object = object;
            impl.object_valid = true;
        }
    }
    // A texture in a framebuffer the renderer drew is read from that render
    // target: the pixels never reach guest memory, which holds whatever was
    // there before. MHP3RD_NO_FB_TEXTURES decodes guest memory as before.
    static const bool no_fb_textures = std::getenv("MHP3RD_NO_FB_TEXTURES") != nullptr;
    const Impl::FramebufferTexture framebuffer_source =
        call.texture.enabled && !call.clear_mode && !no_fb_textures
            ? impl.find_framebuffer_texture(memory, call.texture)
            : Impl::FramebufferTexture{};

    // Under Fill the game's 480x272 screen is spread over a target of the
    // window's shape, which suits the 3D view the game now draws at that
    // shape, but would stretch the 2D interface. Its draws into the shown
    // framebuffer are pulled in about the screen's centre to square pixels
    // again. Draws that cover the screen's width (fades, backdrops) and
    // draws that sample a rendered picture (blur, the quest-reward
    // background) belong with the 3D view and stay spread.
    float fit_x = 1.0f, fit_y = 1.0f;
    bool fitted = false;
    if (call.through && !call.clear_mode && framebuffer_source.target == nullptr &&
        impl.shows(call.target.color_address) && impl.interface_fit(fit_x, fit_y)) {
        float left = impl.scratch.front().x, right = left;
        for (const GpuVertex &vertex : impl.scratch) {
            left = std::min(left, vertex.x);
            right = std::max(right, vertex.x);
        }
        if (right - left < kScreenWideDraw) {
            fitted = true;
            const float centre_x = 0.5f * static_cast<float>(kPspWidth);
            const float centre_y = 0.5f * static_cast<float>(kPspHeight);
            for (GpuVertex &vertex : impl.scratch) {
                vertex.x = centre_x + (vertex.x - centre_x) * fit_x;
                vertex.y = centre_y + (vertex.y - centre_y) * fit_y;
            }
        }
    }

    VkDeviceSize vertex_start = impl.vertex_offset;
    VkDeviceSize index_start = 0u;
    VkDeviceSize draw_end = 0u;
    std::uint32_t draw_count = 0u;
    if (direct) {
        // Floats want 4-byte alignment and index buffer offsets a multiple of
        // the index size; a vertex run starts on 16 bytes.
        vertex_start = (impl.vertex_offset + 15u) & ~VkDeviceSize{15u};
        index_start = (vertex_start + call.vertices.size() * sizeof(GpuVertex) + 3u) & ~VkDeviceSize{3u};
        draw_end = index_start + impl.direct_indices.size() * sizeof(std::uint16_t);
        if (draw_end > kVertexBufferBytes) return;
        auto *out = static_cast<std::uint8_t *>(impl.vertex_mapped) + vertex_start;
        for (const Vertex &vertex : call.vertices) {
            const GpuVertex converted = to_gpu(vertex);
            std::memcpy(out, &converted, sizeof(GpuVertex));
            out += sizeof(GpuVertex);
        }
        std::memcpy(static_cast<std::uint8_t *>(impl.vertex_mapped) + index_start, impl.direct_indices.data(),
                    impl.direct_indices.size() * sizeof(std::uint16_t));
        draw_count = static_cast<std::uint32_t>(impl.direct_indices.size());
    } else {
        const VkDeviceSize bytes = impl.scratch.size() * sizeof(GpuVertex);
        if (impl.vertex_offset + bytes > kVertexBufferBytes) return;
        std::memcpy(static_cast<std::uint8_t *>(impl.vertex_mapped) + impl.vertex_offset, impl.scratch.data(),
                    static_cast<std::size_t>(bytes));
        draw_end = impl.vertex_offset + bytes;
        draw_count = static_cast<std::uint32_t>(impl.scratch.size());
    }

    PipelineKey key{};
    if (call.clear_mode) {
        // sceGuClear draws a screen-sized sprite with CLEARMODE on: blending,
        // the depth test and the texture are bypassed and bits 8..10 say which
        // buffers it is allowed to write. Treating it as an ordinary draw left
        // the depth buffer at whatever the allocation happened to contain.
        key.blend = false;
        // Vulkan ties depth writes to the depth test: with depthTestEnable false
        // the attachment is never updated, so the test has to stay on and always
        // pass for the clear to reach the depth buffer at all.
        key.depth_test = true;
        key.depth_function = 1u;  // always
        key.depth_write = (call.clear_flags & 4u) != 0u;
        key.cull = false;
        key.color_mask = ((call.clear_flags & 1u) != 0u ? (VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                                          VK_COLOR_COMPONENT_B_BIT)
                                                       : 0u) |
                         ((call.clear_flags & 2u) != 0u ? VK_COLOR_COMPONENT_A_BIT : 0u);
    } else {
        key.blend = call.blend.enabled;
        key.source_factor = resolve_fixed_factor(call.blend.source_factor, call.blend.fixed_source);
        key.destination_factor = resolve_fixed_factor(call.blend.destination_factor, call.blend.fixed_destination);
        // Both sides fixed is common for fog and steam, and one constant cannot
        // serve two colours. When the pair adds up to white the destination is
        // exactly the complement of the source, which Vulkan does express.
        if (key.source_factor == kFactorFixed && key.destination_factor == kFactorFixed &&
            ((call.blend.fixed_source + call.blend.fixed_destination) & 0x00FFFFFFu) == 0x00FFFFFFu)
            key.destination_factor = kFactorInverseConstant;
        key.equation = call.blend.equation;
        key.depth_test = call.depth.test_enabled && !call.through;
        key.depth_write = call.depth.write_enabled;
        key.depth_function = call.depth.function;
        key.cull = call.culling_enabled && !call.through && call.primitive != PrimitiveType::Sprites;
        key.cull_clockwise = call.cull_clockwise;
    }
    // Escape hatch for bisecting "nothing is visible" reports.
    static const bool no_cull = std::getenv("MHP3RD_NO_CULL") != nullptr;
    static const bool no_depth = std::getenv("MHP3RD_NO_DEPTH") != nullptr;
    if (no_cull) key.cull = false;
    if (no_depth && !call.clear_mode) key.depth_test = false;
    VkPipeline pipeline = impl.pipeline_for(key);
    if (pipeline == VK_NULL_HANDLE) return;

    PushConstants push{};
    push.transform = multiply(call.projection, view_world);
    push.viewport = {static_cast<float>(kPspWidth), static_cast<float>(kPspHeight), call.through ? 1.0f : 0.0f,
                     (fogged ? kPushFog : 0.0f) + (lit ? kPushLighting : 0.0f)};
    push.view_z = {view_world[2], view_world[6], view_world[10], view_world[14]};
    push.texture_params = {call.texture.enabled ? 1.0f : 0.0f, static_cast<float>(call.texture.function),
                           static_cast<float>(call.alpha_test.enabled ? call.alpha_test.reference : 0u),
                           static_cast<float>(call.alpha_test.enabled ? call.alpha_test.function : 0u)};
    // Through-mode texture coordinates are in texels, transformed ones in [0,1].
    push.uv_transform = call.through && call.texture.width != 0u
                            ? std::array<float, 4>{1.0f / static_cast<float>(call.texture.width),
                                                   1.0f / static_cast<float>(call.texture.height), 0.0f, 0.0f}
                            : std::array<float, 4>{call.texture.scale_u, call.texture.scale_v, call.texture.offset_u,
                                                   call.texture.offset_v};

    if (call.clear_mode) push.texture_params = {0.0f, 0.0f, 0.0f, 0.0f};

    static const bool trace_fb = std::getenv("MHP3RD_TRACE_FB_TEXTURES") != nullptr;
    if (trace_fb && call.texture.enabled && !call.clear_mode) impl.trace_framebuffer_texture(call);

    VkDescriptorSet texture_descriptor = impl.white_texture.descriptor;
    if (call.texture.enabled && !call.clear_mode) {
        const Impl::FramebufferTexture &source = framebuffer_source;
        const VkDescriptorSet copy =
            source.target != nullptr
                ? impl.framebuffer_descriptor(*source.target, call.texture.format == TextureFormat::Rgba5650)
                : VK_NULL_HANDLE;
        if (copy != VK_NULL_HANDLE) {
            // Texture coordinates address the game's texture, whose first texel
            // is pixel (x, y) of a target that holds 480x272 guest pixels at
            // any internal scale.
            texture_descriptor = copy;
            const float width = static_cast<float>(call.texture.width);
            const float height = static_cast<float>(call.texture.height);
            const std::array<float, 4> uv = push.uv_transform;
            push.uv_transform = {uv[0] * width / static_cast<float>(kPspWidth),
                                 uv[1] * height / static_cast<float>(kPspHeight),
                                 (uv[2] * width + static_cast<float>(source.x)) / static_cast<float>(kPspWidth),
                                 (uv[3] * height + static_cast<float>(source.y)) / static_cast<float>(kPspHeight)};
        } else {
            texture_descriptor = impl.texture_descriptor(memory, call);
        }
    }

    if (!impl.pass_active || impl.current_target != call.target.color_address) {
        impl.end_pass();
        impl.begin_pass(call.target.color_address);
        if (!impl.pass_active) return;
    }
    {
        Impl::Target &drawn = impl.targets.at(call.target.color_address);
        const bool layout_changed =
            drawn.stride != call.target.color_stride || drawn.format != call.target.color_format;
        drawn.stride = call.target.color_stride;
        drawn.format = call.target.color_format;
        if (layout_changed || drawn.guest_words.empty() || drawn.last_drawn_frame != impl.frames)
            impl.snapshot_guest_words(memory, call.target.color_address, drawn);
        drawn.last_drawn_frame = impl.frames;
        drawn.draw_serial = ++impl.target_draw_counter;
    }
    // The GE viewport maps normalised device coordinates onto the screen as
    //   screen = ndc * scale + offset - region_offset
    // with a negative y scale (PSP device y points up, the screen down) and a z
    // scale/offset that drives the 16-bit depth buffer. Folding all of that into
    // the Vulkan viewport reproduces the PSP's screen space exactly, including a
    // reversed depth range, and keeps triangle winding as the GE sees it. Through
    // draws bypass the transform, so they keep the plain full-target viewport.
    // A target holds the game's 480x272 screen whatever its size, so a PSP
    // pixel is scale_x by scale_y of its pixels: equal at a multiple of
    // 480x272, different under Fill.
    const float scale_x = static_cast<float>(impl.target_extent.width) / static_cast<float>(kPspWidth);
    const float scale_y = static_cast<float>(impl.target_extent.height) / static_cast<float>(kPspHeight);
    VkViewport vk_viewport{0.0f, 0.0f, static_cast<float>(impl.target_extent.width),
                           static_cast<float>(impl.target_extent.height), 0.0f, 1.0f};
    if (!call.through && call.viewport.x_scale != 0.0f && call.viewport.y_scale != 0.0f) {
        const ViewportState &vp = call.viewport;
        vk_viewport.x = (vp.x_offset - vp.offset_x - vp.x_scale) * scale_x;
        vk_viewport.y = (vp.y_offset - vp.offset_y - vp.y_scale) * scale_y;
        vk_viewport.width = 2.0f * vp.x_scale * scale_x;
        // The GE's y scale is negative (device y points up, the screen down), so
        // this is a flipping viewport. That keeps framebuffer space identical to
        // the PSP's screen space, which is what the cull winding is defined in.
        vk_viewport.height = 2.0f * vp.y_scale * scale_y;
        if (vp.z_scale != 0.0f) {
            // The shader hands over device z in [0, 1]; this undoes that halving
            // and applies the GE's z scale/offset, reproducing a reversed range
            // when the guest set one with sceGuDepthRange(65535, 0).
            vk_viewport.minDepth = std::clamp((vp.z_offset - vp.z_scale) / 65535.0f, 0.0f, 1.0f);
            vk_viewport.maxDepth = std::clamp((vp.z_offset + vp.z_scale) / 65535.0f, 0.0f, 1.0f);
        }
    }
    vkCmdSetViewport(impl.command_buffer, 0u, 1u, &vk_viewport);

    // Whichever side asked for the constant decides its colour; the source wins
    // when both do, because the destination is then its complement.
    const std::uint32_t constant_color =
        key.source_factor == kFactorFixed ? call.blend.fixed_source : call.blend.fixed_destination;
    const std::array<float, 4> blend_constants{static_cast<float>(constant_color & 0xFFu) / 255.0f,
                                               static_cast<float>((constant_color >> 8u) & 0xFFu) / 255.0f,
                                               static_cast<float>((constant_color >> 16u) & 0xFFu) / 255.0f, 1.0f};
    vkCmdSetBlendConstants(impl.command_buffer, blend_constants.data());

    const auto clamp_axis = [](std::uint32_t value, std::uint32_t limit) { return std::min(value, limit); };
    const std::uint32_t sx1 = clamp_axis(call.viewport.scissor_x1, kPspWidth - 1u);
    const std::uint32_t sy1 = clamp_axis(call.viewport.scissor_y1, kPspHeight - 1u);
    const std::uint32_t sx2 = clamp_axis(std::max(call.viewport.scissor_x2, sx1), kPspWidth - 1u);
    const std::uint32_t sy2 = clamp_axis(std::max(call.viewport.scissor_y2, sy1), kPspHeight - 1u);
    // The scissor's edges, in PSP pixels; a fitted draw's scissor is fitted
    // with it, so a list the interface clips still clips at its own edges.
    float edges[4]{static_cast<float>(sx1), static_cast<float>(sy1), static_cast<float>(sx2 + 1u),
                   static_cast<float>(sy2 + 1u)};
    if (fitted) {
        const float centre_x = 0.5f * static_cast<float>(kPspWidth);
        const float centre_y = 0.5f * static_cast<float>(kPspHeight);
        edges[0] = centre_x + (edges[0] - centre_x) * fit_x;
        edges[2] = centre_x + (edges[2] - centre_x) * fit_x;
        edges[1] = centre_y + (edges[1] - centre_y) * fit_y;
        edges[3] = centre_y + (edges[3] - centre_y) * fit_y;
    }
    const auto to_pixels = [](float edge, float scale, std::uint32_t size) {
        return static_cast<std::int32_t>(
            std::clamp(std::lround(edge * scale), 0l, static_cast<long>(size)));
    };
    const std::int32_t left = to_pixels(edges[0], scale_x, impl.target_extent.width);
    const std::int32_t top = to_pixels(edges[1], scale_y, impl.target_extent.height);
    const std::int32_t right = std::max(left, to_pixels(edges[2], scale_x, impl.target_extent.width));
    const std::int32_t bottom = std::max(top, to_pixels(edges[3], scale_y, impl.target_extent.height));
    VkRect2D vk_scissor{};
    vk_scissor.offset = {left, top};
    vk_scissor.extent = {static_cast<std::uint32_t>(right - left), static_cast<std::uint32_t>(bottom - top)};
    vkCmdSetScissor(impl.command_buffer, 0u, 1u, &vk_scissor);

    if (pipeline != impl.bound_pipeline) {
        vkCmdBindPipeline(impl.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
        impl.bound_pipeline = pipeline;
    }
    vkCmdBindDescriptorSets(impl.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, impl.pipeline_layout, 0u, 1u,
                            &texture_descriptor, 0u, nullptr);
    // Every pipeline shares one layout, so set 1 stays bound across pipeline
    // and texture changes; it is bound again only when a block moved.
    const std::array<std::uint32_t, 2> lighting_offsets{impl.environment_offset, impl.object_offset};
    if (!impl.lighting_bound || lighting_offsets != impl.bound_lighting_offsets) {
        vkCmdBindDescriptorSets(impl.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, impl.pipeline_layout, 1u, 1u,
                                &impl.lighting_descriptor, static_cast<std::uint32_t>(lighting_offsets.size()),
                                lighting_offsets.data());
        impl.bound_lighting_offsets = lighting_offsets;
        impl.lighting_bound = true;
    }
    vkCmdPushConstants(impl.command_buffer, impl.pipeline_layout,
                       VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0u, sizeof(push), &push);
    vkCmdBindVertexBuffers(impl.command_buffer, 0u, 1u, &impl.vertex_buffer, &vertex_start);
    if (direct) {
        vkCmdBindIndexBuffer(impl.command_buffer, impl.vertex_buffer, index_start, VK_INDEX_TYPE_UINT16);
        vkCmdDrawIndexed(impl.command_buffer, draw_count, 1u, 0u, 0, 0u);
    } else {
        vkCmdDraw(impl.command_buffer, draw_count, 1u, 0u, 0u);
    }
    impl.vertex_offset = draw_end;
    ++impl.draws;
}

void VulkanRenderer::write_back_frame(GuestMemory &memory) {
    Impl &impl = *impl_;
    if (!impl.ready || !impl.writeback_has_pixels) return;
    impl.writeback_has_pixels = false;
    const perf::Clock::time_point store_start = perf::Clock::now();
    impl.store_frame(memory, impl.writeback_ready, impl.writeback_pixels.data());
    perf::note_stall(perf::Stall::Store, perf::Clock::now() - store_start);
}

void VulkanRenderer::read_back_framebuffer(std::uint32_t source, GuestMemory &memory) {
    Impl &impl = *impl_;
    static const bool disabled = std::getenv("MHP3RD_NO_FB_TEXTURES") != nullptr;
    if (!impl.ready || disabled) return;
    const std::uint32_t wanted = GuestMemory::canonical(source);
    std::uint32_t found = 0u;
    bool any = false;
    for (const auto &[address, target] : impl.targets) {
        if (!target.initialized || target.guest_words.empty()) continue;
        const std::uint32_t base = GuestMemory::canonical(address);
        const std::uint32_t bytes = target.stride * kPspHeight * framebuffer_bytes_per_pixel(target.format);
        if (wanted >= base && wanted - base < bytes) {
            found = address;
            any = true;
            break;
        }
    }
    if (!any) return;
    if (!impl.recording) begin_frame();
    impl.end_pass();
    impl.record_writeback(found);
    if (!impl.writeback_in_flight) return;
    // Run everything recorded so far and carry on recording afterwards.
    impl.end_gpu_segment(impl.command_buffer);
    vkEndCommandBuffer(impl.command_buffer);
    VkSubmitInfo submit{VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submit.commandBufferCount = 1u;
    submit.pCommandBuffers = &impl.command_buffer;
    const perf::Clock::time_point wait_start = perf::Clock::now();
    vkQueueSubmit(impl.queue, 1u, &submit, impl.frame_fence);
    vkWaitForFences(impl.device, 1u, &impl.frame_fence, VK_TRUE, UINT64_MAX);
    perf::add_wait_time(perf::Clock::now() - wait_start, perf::Stall::Readback);
    vkResetFences(impl.device, 1u, &impl.frame_fence);
    vkResetCommandBuffer(impl.command_buffer, 0u);
    VkCommandBufferBeginInfo begin{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(impl.command_buffer, &begin);
    // The queries written so far stay valid: they were reset at the start of
    // the frame, and the next pair follows them.
    impl.begin_gpu_segment(impl.command_buffer);
    impl.vertex_offset = 0u;
    impl.environment_version = 0u;
    impl.object_valid = false;
    impl.forget_bindings();
    impl.writeback_in_flight = false;
    const perf::Clock::time_point copy_start = perf::Clock::now();
    std::vector<std::uint32_t> pixels(static_cast<std::size_t>(kPspWidth) * kPspHeight);
    std::memcpy(pixels.data(), impl.writeback_mapped, pixels.size() * 4u);
    const perf::Clock::time_point store_start = perf::Clock::now();
    perf::note_stall(perf::Stall::Copy, store_start - copy_start);
    impl.store_frame(memory, impl.writeback_recorded, pixels.data());
    perf::note_stall(perf::Stall::Store, perf::Clock::now() - store_start);
    // A write-back still waiting from an earlier frame is older than this one.
    if (impl.writeback_ready.address == found) impl.writeback_has_pixels = false;
    static const bool trace = std::getenv("MHP3RD_TRACE_FB_TEXTURES") != nullptr;
    if (trace)
        std::cout << "[fbtex] frame " << impl.frames << " read framebuffer 0x" << std::hex << found << std::dec
                  << " back for a block transfer\n";
}

void VulkanRenderer::Impl::store_frame(GuestMemory &memory, const WritebackFrame &frame,
                                       const std::uint32_t *pixels) {
    Impl &impl = *this;
    const std::uint32_t bytes_per_pixel = framebuffer_bytes_per_pixel(frame.format);
    const std::size_t bytes = static_cast<std::size_t>(frame.stride) * kPspHeight * bytes_per_pixel;
    std::uint8_t *out = memory.raw_pointer(frame.address, bytes);
    if (out == nullptr) return;
    for (std::uint32_t y = 0; y < kPspHeight; ++y) {
        const std::uint32_t *row = pixels + static_cast<std::size_t>(y) * kPspWidth;
        std::uint8_t *line = out + static_cast<std::size_t>(y) * frame.stride * bytes_per_pixel;
        if (frame.format == 3u) {
            std::memcpy(line, row, static_cast<std::size_t>(kPspWidth) * 4u);
            continue;
        }
        for (std::uint32_t x = 0; x < kPspWidth; ++x) {
            // Red in the low bits, as texture_decode's expand_* read them.
            const std::uint32_t pixel = row[x];
            const std::uint32_t r = pixel & 0xFFu;
            const std::uint32_t g = (pixel >> 8u) & 0xFFu;
            const std::uint32_t b = (pixel >> 16u) & 0xFFu;
            const std::uint32_t a = pixel >> 24u;
            std::uint32_t value = 0u;
            if (frame.format == 0u)
                value = (r >> 3u) | (g >> 2u) << 5u | (b >> 3u) << 11u;
            else if (frame.format == 1u)
                value = (r >> 3u) | (g >> 3u) << 5u | (b >> 3u) << 10u | (a >> 7u) << 15u;
            else
                value = (r >> 4u) | (g >> 4u) << 4u | (b >> 4u) << 8u | (a >> 4u) << 12u;
            line[x * 2u] = static_cast<std::uint8_t>(value & 0xFFu);
            line[x * 2u + 1u] = static_cast<std::uint8_t>(value >> 8u);
        }
    }
    // The bytes now differ from the snapshot taken when the target was drawn,
    // and are exactly what it shows: take the snapshot again.
    const auto target = impl.targets.find(frame.address);
    if (target != impl.targets.end()) impl.snapshot_guest_words(memory, frame.address, target->second);
}

void VulkanRenderer::present(std::uint32_t display_address) {
    Impl &impl = *impl_;
    if (!impl.ready) return;
    if (!impl.recording) begin_frame();
    impl.end_pass();

    static const bool trace3d = std::getenv("MHP3RD_TRACE_3D") != nullptr;
    static const bool trace_camera = std::getenv("MHP3RD_TRACE_CAMERA") != nullptr;
    // The camera hunt reads the same measurement without printing it.
    static const bool watch_camera = trace_camera || std::getenv("MHP3RD_FIND_CAMERA") != nullptr;
    if (trace3d && impl.frame_transformed_draws != 0u) {
        std::cout << "[3d] frame " << impl.frames << " through=" << impl.frame_through_draws
                  << " transformed=" << impl.frame_transformed_draws << " showing=0x" << std::hex << display_address
                  << " transformed targets:";
        for (const auto &[address, count] : impl.frame_transformed_targets)
            std::cout << " 0x" << address << "x" << std::dec << count << std::hex;
        std::cout << std::dec << "\n";
        std::cout << "     verts=" << impl.frame_transformed_vertices << " onscreen=" << impl.frame_onscreen_vertices
                  << " behind=" << impl.frame_behind_camera << " ndc x[" << impl.frame_ndc_min[0] << ","
                  << impl.frame_ndc_max[0] << "] y[" << impl.frame_ndc_min[1] << "," << impl.frame_ndc_max[1]
                  << "] z[" << impl.frame_ndc_min[2] << "," << impl.frame_ndc_max[2] << "]\n";
    }
    if (watch_camera && !impl.frame_views.empty()) {
        // The busiest view matrix of the frame is the scene the player looks
        // at; the others belong to reflections and shadow passes.
        const auto scene = std::max_element(impl.frame_views.begin(), impl.frame_views.end(),
                                            [](const auto &a, const auto &b) { return a.second < b.second; });
        const std::array<float, 16> &view = scene->first;
        // A view matrix holds the camera's own axes as the rows of its
        // rotation part, and the array is column major, so row 2 — the
        // direction the camera looks along — is elements 2, 6 and 10.
        const float forward_x = view[2];
        const float forward_y = view[6];
        const float forward_z = view[10];
        constexpr float kDegrees = 57.29577951308232f;
        const float yaw = std::atan2(forward_x, forward_z) * kDegrees;
        const float pitch = std::asin(std::clamp(forward_y, -1.0f, 1.0f)) * kDegrees;
        // The camera's world position is the rotation applied backwards to the
        // translation, which tells a turn in place from the hunter walking.
        const float tx = view[12];
        const float ty = view[13];
        const float tz = view[14];
        const float px = -(view[0] * tx + view[1] * ty + view[2] * tz);
        const float py = -(view[4] * tx + view[5] * ty + view[6] * tz);
        const float pz = -(view[8] * tx + view[9] * ty + view[10] * tz);
        float turn = yaw - impl.traced_yaw;
        while (turn > 180.0f) turn -= 360.0f;
        while (turn < -180.0f) turn += 360.0f;
        impl.traced_yaw = yaw;
        impl.reading = CameraReading{true, yaw, pitch, turn, {px, py, pz}, view};
        if (trace_camera) {
        const std::ios::fmtflags flags = std::cout.flags();
        const std::streamsize precision = std::cout.precision();
        std::cout << std::fixed << std::setprecision(4) << "[camera] frame " << impl.frames << " stick="
                  << static_cast<int>(impl.pad.right_x) - 0x80 << "," << static_cast<int>(impl.pad.right_y) - 0x80
                  << " yaw=" << yaw << " pitch=" << pitch << " turn=" << turn << " pos=" << std::setprecision(1) << px
                  << "," << py << "," << pz << " views=" << impl.frame_views.size() << "\n";
        std::cout.flags(flags);
        // Precision is not one of a stream's flags, so restoring the flags
        // leaves it wherever the line left it -- here at one significant digit
        // for the position -- and every number printed afterwards by anything
        // else comes out rounded to one digit for the rest of the run.
        std::cout.precision(precision);
        }
    }
    impl.frame_views.clear();
    impl.frame_through_draws = 0u;
    impl.frame_transformed_draws = 0u;
    impl.frame_transformed_vertices = 0u;
    impl.frame_onscreen_vertices = 0u;
    impl.frame_behind_camera = 0u;
    impl.frame_ndc_min = {1e30f, 1e30f, 1e30f};
    impl.frame_ndc_max = {-1e30f, -1e30f, -1e30f};
    impl.frame_transformed_targets.clear();

    static const bool check_direct = std::getenv("MHP3RD_CHECK_DIRECT_VERTICES") != nullptr;
    if (check_direct && impl.frames % 300u == 0u)
        std::cout << "[direct-check] " << impl.direct_checked << " draws compared, " << impl.direct_mismatched
                  << " differed" << std::endl;

    static const bool no_writeback = std::getenv("MHP3RD_NO_FB_TEXTURES") != nullptr;
    if (!no_writeback) impl.record_writeback(display_address);

    // Show the target the guest flipped to; fall back to whatever was drawn last.
    auto displayed = impl.targets.find(display_address);
    if (displayed == impl.targets.end()) displayed = impl.targets.find(impl.last_drawn_target);
    VkImage source = displayed != impl.targets.end() ? displayed->second.color : VK_NULL_HANDLE;
    impl.presented_target = displayed != impl.targets.end() ? displayed->first : 0u;
    if (impl.holding) source = impl.held.color;
    if (display_address != impl.display_addresses[0]) {
        impl.display_addresses[1] = impl.display_addresses[0];
        impl.display_addresses[0] = display_address;
    }
    impl.submit_and_present(source, true);
    ++impl.frames;
    impl.follow_window();
}

bool VulkanRenderer::capture_frame(const std::string &path) {
    Impl &impl = *impl_;
    if (!impl.ready) return false;
    vkDeviceWaitIdle(impl.device);

    auto shown = impl.targets.find(impl.presented_target);
    if (shown == impl.targets.end()) return false;
    const VkImage captured = shown->second.color;
    const std::uint32_t width = impl.target_extent.width;
    const std::uint32_t height = impl.target_extent.height;
    const VkDeviceSize bytes = static_cast<VkDeviceSize>(width) * height * 4u;
    VkBuffer staging{};
    VkDeviceMemory staging_memory{};
    VkBufferCreateInfo buffer_info{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    buffer_info.size = bytes;
    buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    if (vkCreateBuffer(impl.device, &buffer_info, nullptr, &staging) != VK_SUCCESS) return false;
    VkMemoryRequirements requirements{};
    vkGetBufferMemoryRequirements(impl.device, staging, &requirements);
    VkMemoryAllocateInfo allocate{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    allocate.allocationSize = requirements.size;
    allocate.memoryTypeIndex = impl.find_memory_type(
        requirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    vkAllocateMemory(impl.device, &allocate, nullptr, &staging_memory);
    vkBindBufferMemory(impl.device, staging, staging_memory, 0u);

    VkCommandBufferAllocateInfo command_info{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    command_info.commandPool = impl.command_pool;
    command_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    command_info.commandBufferCount = 1u;
    VkCommandBuffer commands{};
    vkAllocateCommandBuffers(impl.device, &command_info, &commands);
    VkCommandBufferBeginInfo begin{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(commands, &begin);
    impl.transition(commands, captured, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    VkBufferImageCopy copy{};
    copy.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 0u, 1u};
    copy.imageExtent = {width, height, 1u};
    vkCmdCopyImageToBuffer(commands, captured, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, staging, 1u, &copy);
    impl.transition(commands, captured, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    vkEndCommandBuffer(commands);
    VkSubmitInfo submit{VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submit.commandBufferCount = 1u;
    submit.pCommandBuffers = &commands;
    vkQueueSubmit(impl.queue, 1u, &submit, VK_NULL_HANDLE);
    vkQueueWaitIdle(impl.queue);
    vkFreeCommandBuffers(impl.device, impl.command_pool, 1u, &commands);

    void *mapped = nullptr;
    vkMapMemory(impl.device, staging_memory, 0u, bytes, 0u, &mapped);
    std::vector<std::uint8_t> pixels(static_cast<const std::uint8_t *>(mapped),
                                     static_cast<const std::uint8_t *>(mapped) + bytes);
    vkUnmapMemory(impl.device, staging_memory);
    vkDestroyBuffer(impl.device, staging, nullptr);
    vkFreeMemory(impl.device, staging_memory, nullptr);

    // The capture is the game's own target, before the window blit; draw the
    // overlay over it the same way, so captures show what the window shows.
    if (impl.overlay_visible) {
        const std::uint32_t scale = perf::overlay_scale(height);
        const std::uint32_t inset = 4u * scale;
        if (inset + perf::kOverlayWidth * scale <= width && inset + perf::kOverlayHeight * scale <= height) {
            for (std::uint32_t y = 0; y < perf::kOverlayHeight * scale; ++y) {
                std::uint8_t *row = pixels.data() + static_cast<std::size_t>(inset + y) * width * 4u;
                for (std::uint32_t x = 0; x < perf::kOverlayWidth * scale; ++x) {
                    const std::uint32_t pixel = impl.overlay_pixels[(y / scale) * perf::kOverlayWidth + x / scale];
                    std::memcpy(row + (inset + x) * 4u, &pixel, 4u);
                }
            }
        }
    }
    return write_bmp(path, pixels.data(), width, height, false);
}

void VulkanRenderer::shutdown() {
    if (impl_ && impl_->gamepad != nullptr) {
        SDL_CloseGamepad(impl_->gamepad);
        impl_->gamepad = nullptr;
    }
    if (!impl_ || impl_->device == VK_NULL_HANDLE) {
        if (impl_ && impl_->window != nullptr) {
            SDL_DestroyWindow(impl_->window);
            impl_->window = nullptr;
        }
        return;
    }
    Impl &impl = *impl_;
    vkDeviceWaitIdle(impl.device);
    // The interface may still be up when the game quits from its menu.
    if (impl.ui_ready && ImGui::GetCurrentContext() != nullptr) ImGui_ImplVulkan_Shutdown();
    impl.ui_ready = false;
    impl.destroy_swapchain_views();
    if (impl.ui_render_pass != VK_NULL_HANDLE) vkDestroyRenderPass(impl.device, impl.ui_render_pass, nullptr);
    for (auto &[key, texture] : impl.textures) impl.destroy_texture(texture);
    impl.textures.clear();
    impl.replacements.shutdown();
    impl.pack.reset();
    impl.dumper.reset();
    impl.destroy_texture(impl.white_texture);
    if (impl.overlay_mapped != nullptr) vkUnmapMemory(impl.device, impl.overlay_staging_memory);
    vkDestroyBuffer(impl.device, impl.overlay_staging, nullptr);
    vkFreeMemory(impl.device, impl.overlay_staging_memory, nullptr);
    vkDestroyImageView(impl.device, impl.overlay_view, nullptr);
    vkDestroyImage(impl.device, impl.overlay_image, nullptr);
    vkFreeMemory(impl.device, impl.overlay_memory, nullptr);
    impl.destroy_upload();
    impl.destroy_writeback();
    for (auto &[key, pipeline] : impl.pipelines) vkDestroyPipeline(impl.device, pipeline, nullptr);
    impl.pipelines.clear();
    impl.last_pipeline = VK_NULL_HANDLE;
    if (impl.vertex_mapped != nullptr) vkUnmapMemory(impl.device, impl.vertex_memory);
    vkDestroyBuffer(impl.device, impl.vertex_buffer, nullptr);
    vkFreeMemory(impl.device, impl.vertex_memory, nullptr);
    vkDestroySampler(impl.device, impl.sampler, nullptr);
    vkDestroySampler(impl.device, impl.sharp_sampler, nullptr);
    vkDestroySampler(impl.device, impl.clamp_sampler, nullptr);
    vkDestroySampler(impl.device, impl.clamp_sharp_sampler, nullptr);
    vkDestroyDescriptorPool(impl.device, impl.descriptor_pool, nullptr);
    vkDestroyDescriptorSetLayout(impl.device, impl.descriptor_layout, nullptr);
    vkDestroyDescriptorSetLayout(impl.device, impl.lighting_layout, nullptr);
    vkDestroyPipelineLayout(impl.device, impl.pipeline_layout, nullptr);
    vkDestroyShaderModule(impl.device, impl.vertex_shader, nullptr);
    vkDestroyShaderModule(impl.device, impl.fragment_shader, nullptr);
    for (auto &[address, target] : impl.targets) impl.destroy_target(target);
    impl.targets.clear();
    impl.destroy_target(impl.held);
    impl.held = {};
    impl.holding = false;
    vkDestroyRenderPass(impl.device, impl.render_pass, nullptr);
    vkDestroySemaphore(impl.device, impl.image_available, nullptr);
    vkDestroySemaphore(impl.device, impl.render_finished, nullptr);
    vkDestroyFence(impl.device, impl.frame_fence, nullptr);
    if (impl.gpu_timer != VK_NULL_HANDLE) vkDestroyQueryPool(impl.device, impl.gpu_timer, nullptr);
    vkDestroyCommandPool(impl.device, impl.command_pool, nullptr);
    vkDestroySwapchainKHR(impl.device, impl.swapchain, nullptr);
    vkDestroyDevice(impl.device, nullptr);
    if (impl.surface != VK_NULL_HANDLE) vkDestroySurfaceKHR(impl.instance, impl.surface, nullptr);
    vkDestroyInstance(impl.instance, nullptr);
    if (impl.window != nullptr) SDL_DestroyWindow(impl.window);
    impl = Impl{};
}

} // namespace mhp3rd::gpu
