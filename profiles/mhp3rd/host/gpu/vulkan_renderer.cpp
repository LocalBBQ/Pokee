#include "vulkan_renderer.hpp"

#include "texture_decode.hpp"

#include "perf/frame_stats.hpp"
#include "perf/perf_overlay.hpp"
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
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace mhp3rd::gpu {
namespace {

constexpr std::uint32_t kPspWidth = 480u;
constexpr std::uint32_t kPspHeight = 272u;
constexpr VkDeviceSize kVertexBufferBytes = 16u * 1024u * 1024u;
constexpr std::size_t kMaxCachedTextures = 1024u;

// Compiled SPIR-V, generated from host/gpu/shaders by the build.
#include "ge_shaders.inc"

struct PushConstants {
    std::array<float, 16> transform{};
    std::array<float, 4> viewport{};        // x,y: target size; z: through flag
    std::array<float, 4> texture_params{};  // x: enabled, y: function, z: alpha ref, w: alpha func
    std::array<float, 4> uv_transform{1.0f, 1.0f, 0.0f, 0.0f};
};

struct GpuVertex {
    float x{}, y{}, z{}, w{1.0f};
    float u{}, v{};
    std::uint32_t color{};
    float nx{}, ny{}, nz{};
};

// Per-draw lighting and fog state, laid out as the std140 `Lighting` block in
// ge.vert. It lives in the vertex buffer, next to the vertices it lights, and
// is bound through a dynamic uniform buffer offset.
struct LightingBlock {
    std::array<float, 16> world{};
    std::array<float, 4> view_z{};
    std::array<float, 4> flags{};             // lighting, vertex colour, fog, material update mask
    std::array<float, 4> emissive{};          // w: specular power
    std::array<float, 4> material_ambient{};
    std::array<float, 4> material_diffuse{};  // w: separate specular
    std::array<float, 4> material_specular{}; // w: reverse normals
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

static_assert(sizeof(LightingBlock) == 656u, "LightingBlock must match the std140 layout in ge.vert");

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
    if (axis(SDL_GAMEPAD_AXIS_LEFT_TRIGGER) > tuning.trigger) buttons |= 0x0100u;
    if (axis(SDL_GAMEPAD_AXIS_RIGHT_TRIGGER) > tuning.trigger) buttons |= 0x0200u;

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
    bool keep_aspect{true};
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
    };
    std::map<std::uint32_t, Target> targets;
    std::uint32_t current_target{};
    std::uint32_t last_drawn_target{};
    std::uint32_t presented_target{};
    // Per-frame tally, so "no 3D" can be told from "3D drawn somewhere else".
    std::uint32_t frame_through_draws{};
    std::uint32_t frame_transformed_draws{};
    std::uint32_t frame_transformed_vertices{};
    std::uint32_t frame_onscreen_vertices{};
    std::uint32_t frame_behind_camera{};
    std::array<float, 3> frame_ndc_min{1e30f, 1e30f, 1e30f};
    std::array<float, 3> frame_ndc_max{-1e30f, -1e30f, -1e30f};
    std::map<std::uint32_t, std::uint32_t> frame_transformed_targets;
    bool pass_active{};
    VkRenderPass render_pass{};
    VkExtent2D target_extent{};

    VkShaderModule vertex_shader{};
    VkShaderModule fragment_shader{};
    VkPipelineLayout pipeline_layout{};
    VkDescriptorSetLayout descriptor_layout{};
    VkDescriptorPool descriptor_pool{};
    VkDescriptorSetLayout lighting_layout{};
    VkDescriptorSet lighting_descriptor{};
    VkDeviceSize uniform_alignment{256u};
    VkSampler sampler{};        // linear
    VkSampler sharp_sampler{};  // nearest, for the sharp texture setting
    std::map<PipelineKey, VkPipeline> pipelines;

    VkBuffer vertex_buffer{};
    VkDeviceMemory vertex_memory{};
    void *vertex_mapped{};
    VkDeviceSize vertex_offset{};
    // The lighting block most recently written this frame, reused while the
    // state stays the same; begin_frame() drops it with the vertex buffer.
    LightingBlock last_lighting{};
    VkDeviceSize last_lighting_offset{};
    bool last_lighting_valid{};

    Texture white_texture{};
    std::map<std::uint64_t, Texture> textures;
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
    std::map<TextureKeyInput, std::uint64_t> list_texture_keys;

    std::vector<GpuVertex> scratch;
    PadState pad{};
    SDL_Gamepad *gamepad{};
    SDL_JoystickID gamepad_id{};
    bool text_input_active{};
    bool text_confirmed{};
    bool text_cancelled{};
    std::string text;
    bool recording{};
    bool quit{};
    bool ready{};
    std::uint64_t frames{};
    std::uint64_t draws{};

    // Only the first pad is used; a second one arriving is ignored rather than
    // stealing the stick from whoever is already playing.
    void open_gamepad(SDL_JoystickID id) {
        if (gamepad != nullptr) return;
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
    [[nodiscard]] VkPresentModeKHR wanted_present_mode() const;
    bool create_swapchain(std::string &error);
    void destroy_swapchain_views();
    void recreate_swapchain();
    bool create_ui_framebuffers(std::string &error);
    void record_game_blit(VkImage source, VkImage destination);
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
    Texture &texture_for(const GuestMemory &memory, const TextureState &state);
    Texture create_texture(std::uint32_t width, std::uint32_t height, const std::uint32_t *pixels);
    void destroy_texture(Texture &texture);
};

VulkanRenderer::VulkanRenderer() : impl_(std::make_unique<Impl>()) {}
VulkanRenderer::~VulkanRenderer() { shutdown(); }

bool VulkanRenderer::available() const noexcept { return impl_ && impl_->ready; }
bool VulkanRenderer::quit_requested() const noexcept { return impl_ && impl_->quit; }
std::uint64_t VulkanRenderer::frames_presented() const noexcept { return impl_ ? impl_->frames : 0u; }
std::uint64_t VulkanRenderer::draws_submitted() const noexcept { return impl_ ? impl_->draws : 0u; }

bool VulkanRenderer::initialize(const RendererConfig &config, std::string &error) {
    Impl &impl = *impl_;
    impl.config = config;
    const settings::Settings &player = settings::current();
    const std::uint32_t scale = std::clamp<std::uint32_t>(player.internal_scale, 1u, settings::kMaxInternalScale);
    impl.target_extent = {kPspWidth * scale, kPspHeight * scale};
    impl.requested_present = player.present_mode;
    impl.keep_aspect = player.keep_aspect;
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
    application.pApplicationName = "MHP3rdNative";
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

    // Set 1: the per-draw lighting block, a window into the vertex buffer.
    VkDescriptorSetLayoutBinding lighting_binding{};
    lighting_binding.binding = 0u;
    lighting_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    lighting_binding.descriptorCount = 1u;
    lighting_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    VkDescriptorSetLayoutCreateInfo lighting_layout_info{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    lighting_layout_info.bindingCount = 1u;
    lighting_layout_info.pBindings = &lighting_binding;
    if (!check(vkCreateDescriptorSetLayout(impl.device, &lighting_layout_info, nullptr, &impl.lighting_layout),
               "vkCreateDescriptorSetLayout", error))
        return false;

    const std::array<VkDescriptorPoolSize, 2> pool_sizes{
        VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                             static_cast<std::uint32_t>(kMaxCachedTextures + 1u)},
        VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1u},
    };
    VkDescriptorPoolCreateInfo pool_info{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = static_cast<std::uint32_t>(kMaxCachedTextures + 2u);
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

    VkBufferCreateInfo buffer_info{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    buffer_info.size = kVertexBufferBytes;
    buffer_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
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
        VkDescriptorBufferInfo lighting_buffer{impl.vertex_buffer, 0u, sizeof(LightingBlock)};
        VkWriteDescriptorSet write{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
        write.dstSet = impl.lighting_descriptor;
        write.descriptorCount = 1u;
        write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        write.pBufferInfo = &lighting_buffer;
        vkUpdateDescriptorSets(impl.device, 1u, &write, 0u, nullptr);
    }

    const std::uint32_t white = 0xFFFFFFFFu;
    impl.white_texture = impl.create_texture(1u, 1u, &white);

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
// black bars.
void VulkanRenderer::Impl::record_game_blit(VkImage source, VkImage destination) {
    const auto width = static_cast<std::int32_t>(swapchain_extent.width);
    const auto height = static_cast<std::int32_t>(swapchain_extent.height);
    VkOffset3D low{0, 0, 0};
    VkOffset3D high{width, height, 1};
    if (keep_aspect) {
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

    std::uint32_t image_index = 0u;
    VkResult acquired = VK_ERROR_OUT_OF_DATE_KHR;
    if (has_content && swapchain != VK_NULL_HANDLE) {
        const perf::Clock::time_point acquire_start = perf::Clock::now();
        acquired = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, image_available, VK_NULL_HANDLE, &image_index);
        perf::add_wait_time(perf::Clock::now() - acquire_start);
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
    perf::add_wait_time(perf::Clock::now() - submit_start);

    if (can_present) {
        VkPresentInfoKHR present{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
        present.waitSemaphoreCount = 1u;
        present.pWaitSemaphores = &render_finished;
        present.swapchainCount = 1u;
        present.pSwapchains = &swapchain;
        present.pImageIndices = &image_index;
        const perf::Clock::time_point present_start = perf::Clock::now();
        const VkResult presented = vkQueuePresentKHR(queue, &present);
        perf::add_wait_time(perf::Clock::now() - present_start);
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
    perf::add_wait_time(perf::Clock::now() - wait_start);
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

VulkanRenderer::Impl::Texture &VulkanRenderer::Impl::texture_for(const GuestMemory &memory,
                                                                 const TextureState &state) {
    const TextureKeyInput input{state.address,
                                state.buffer_width,
                                static_cast<std::uint32_t>(state.width) << 16u | state.height,
                                static_cast<std::uint32_t>(state.format),
                                state.clut_address,
                                state.clut_format,
                                state.swizzled};
    auto [memo, inserted] = list_texture_keys.try_emplace(input, 0u);
    if (inserted) memo->second = texture_key(memory, state);
    const std::uint64_t key = memo->second;
    const auto found = textures.find(key);
    if (found != textures.end()) {
        found->second.last_used = ++texture_clock;
        return found->second;
    }
    std::vector<std::uint32_t> pixels;
    if (!decode_texture(memory, state, pixels) || pixels.empty()) return white_texture;

    if (textures.size() >= kMaxCachedTextures) {
        auto oldest = textures.begin();
        for (auto it = textures.begin(); it != textures.end(); ++it) {
            if (it->second.last_used < oldest->second.last_used) oldest = it;
        }
        const perf::Clock::time_point wait_start = perf::Clock::now();
        vkQueueWaitIdle(queue);
        perf::add_wait_time(perf::Clock::now() - wait_start);
        destroy_texture(oldest->second);
        textures.erase(oldest);
    }
    Texture texture = create_texture(state.width, state.height, pixels.data());
    if (texture.descriptor == VK_NULL_HANDLE) return white_texture;
    return textures.emplace(key, texture).first->second;
}

VkPipeline VulkanRenderer::Impl::pipeline_for(const PipelineKey &key) {
    const auto found = pipelines.find(key);
    if (found != pipelines.end()) return found->second;

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
        if (impl_->event_hook && impl_->event_hook(event)) continue;
        if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_F3 && !event.key.repeat && impl_->overlay_ready)
            impl_->overlay_visible = !impl_->overlay_visible;

        if (!impl_->text_input_active) continue;
        if (event.type == SDL_EVENT_TEXT_INPUT) {
            impl_->text += event.text.text;
        } else if (event.type == SDL_EVENT_KEY_DOWN) {
            if (event.key.key == SDLK_RETURN || event.key.key == SDLK_KP_ENTER) impl_->text_confirmed = true;
            else if (event.key.key == SDLK_ESCAPE) impl_->text_cancelled = true;
            else if (event.key.key == SDLK_BACKSPACE && !impl_->text.empty()) impl_->text.pop_back();
        }
    }
    // While typing, the keyboard drives text rather than the pad; while a menu
    // is open, nothing reaches the game.
    if (impl_->text_input_active || !impl_->game_input) {
        impl_->pad = PadState{};
        return !impl_->quit;
    }

    // Keyboard to PSP pad. Bits follow SceCtrlButtons.
    // Only sample while the window has focus: a key still down when focus is
    // lost stays down in SDL's snapshot, which the guest sees as a held
    // direction it can never release.
    const bool focused = (SDL_GetWindowFlags(impl_->window) & SDL_WINDOW_INPUT_FOCUS) != 0u;
    const bool *keys = SDL_GetKeyboardState(nullptr);
    PadState pad{};
    const auto held = [&](SDL_Scancode code, std::uint32_t bit) {
        if (focused && keys[code]) pad.buttons |= bit;
    };
    held(SDL_SCANCODE_UP, 0x0010u);
    held(SDL_SCANCODE_RIGHT, 0x0020u);
    held(SDL_SCANCODE_DOWN, 0x0040u);
    held(SDL_SCANCODE_LEFT, 0x0080u);
    held(SDL_SCANCODE_Q, 0x0100u);      // L
    held(SDL_SCANCODE_W, 0x0200u);      // R
    held(SDL_SCANCODE_S, 0x1000u);      // triangle
    held(SDL_SCANCODE_X, 0x2000u);      // circle (confirm in Japanese titles)
    held(SDL_SCANCODE_Z, 0x4000u);      // cross
    held(SDL_SCANCODE_A, 0x8000u);      // square
    held(SDL_SCANCODE_RETURN, 0x0008u); // start
    held(SDL_SCANCODE_RSHIFT, 0x0001u); // select
    held(SDL_SCANCODE_BACKSPACE, 0x0001u);
    // Analog stick on IJKL, centred at 0x80.
    int analog_x = 0;
    int analog_y = 0;
    if (focused && keys[SDL_SCANCODE_J]) analog_x -= 127;
    if (focused && keys[SDL_SCANCODE_L]) analog_x += 127;
    if (focused && keys[SDL_SCANCODE_I]) analog_y -= 127;
    if (focused && keys[SDL_SCANCODE_K]) analog_y += 127;

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

bool VulkanRenderer::text_input_active() const noexcept { return impl_ && impl_->text_input_active; }

SDL_Window *VulkanRenderer::window() const noexcept { return impl_ ? impl_->window : nullptr; }
std::string VulkanRenderer::device_name() const { return impl_ ? impl_->device_name : std::string{}; }
SDL_Gamepad *VulkanRenderer::gamepad() const noexcept { return impl_ ? impl_->gamepad : nullptr; }

void VulkanRenderer::set_internal_scale(std::uint32_t scale) {
    Impl &impl = *impl_;
    scale = std::clamp<std::uint32_t>(scale, 1u, settings::kMaxInternalScale);
    const VkExtent2D extent{kPspWidth * scale, kPspHeight * scale};
    if (!impl.ready || impl.recording ||
        (extent.width == impl.target_extent.width && extent.height == impl.target_extent.height))
        return;
    // Every target is rebuilt at the new size with its current picture scaled
    // into it, so the paused frame behind the menu, and render-to-texture
    // targets the game reads back, stay intact.
    vkDeviceWaitIdle(impl.device);
    std::map<std::uint32_t, Impl::Target> old_targets = std::move(impl.targets);
    impl.targets.clear();
    const VkExtent2D old_extent = impl.target_extent;
    impl.target_extent = extent;
    std::vector<std::pair<Impl::Target *, const Impl::Target *>> copies;
    for (const auto &[address, old_target] : old_targets) {
        std::string error;
        Impl::Target *target = impl.target_for(address, error);
        if (target == nullptr) {
            std::cout << "[render] cannot resize a render target: " << error << "\n";
            continue;
        }
        if (old_target.initialized) copies.emplace_back(target, &old_target);
    }
    impl.run_commands([&](VkCommandBuffer commands) {
        for (const auto &[target, old_target] : copies) {
            impl.transition(commands, old_target->color, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
            impl.transition(commands, target->color, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
            VkImageBlit blit{};
            blit.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 0u, 1u};
            blit.srcOffsets[1] = {static_cast<std::int32_t>(old_extent.width),
                                  static_cast<std::int32_t>(old_extent.height), 1};
            blit.dstSubresource = blit.srcSubresource;
            blit.dstOffsets[1] = {static_cast<std::int32_t>(extent.width), static_cast<std::int32_t>(extent.height), 1};
            vkCmdBlitImage(commands, old_target->color, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, target->color,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1u, &blit, VK_FILTER_LINEAR);
            impl.transition(commands, target->color, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
            impl.transition(commands, target->depth, VK_IMAGE_LAYOUT_UNDEFINED,
                            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT);
            target->initialized = true;
        }
    });
    for (auto &[address, old_target] : old_targets) impl.destroy_target(old_target);
    std::cout << "[render] internal resolution " << extent.width << "x" << extent.height << "\n";
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

void VulkanRenderer::set_keep_aspect(bool keep_aspect) {
    if (impl_) impl_->keep_aspect = keep_aspect;
}

void VulkanRenderer::set_sharp_screen(bool sharp) {
    if (impl_) impl_->sharp_screen = sharp;
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
}

void VulkanRenderer::begin_text_input(const std::string &initial) {
    if (!impl_ || impl_->window == nullptr) return;
    impl_->text = initial;
    impl_->text_confirmed = false;
    impl_->text_cancelled = false;
    impl_->text_input_active = true;
    SDL_StartTextInput(impl_->window);
    std::cout << "[osk] type the name in the game window, Enter confirms, Esc cancels\n";
}

void VulkanRenderer::end_text_input() {
    if (!impl_ || impl_->window == nullptr) return;
    impl_->text_input_active = false;
    SDL_StopTextInput(impl_->window);
}

std::string VulkanRenderer::text_input() const { return impl_ ? impl_->text : std::string{}; }
bool VulkanRenderer::text_input_confirmed() const noexcept { return impl_ && impl_->text_confirmed; }
bool VulkanRenderer::text_input_cancelled() const noexcept { return impl_ && impl_->text_cancelled; }

void VulkanRenderer::begin_frame() {
    Impl &impl = *impl_;
    if (!impl.ready || impl.recording) return;
    const perf::Clock::time_point wait_start = perf::Clock::now();
    vkWaitForFences(impl.device, 1u, &impl.frame_fence, VK_TRUE, UINT64_MAX);
    perf::add_wait_time(perf::Clock::now() - wait_start);
    vkResetFences(impl.device, 1u, &impl.frame_fence);
    vkResetCommandBuffer(impl.command_buffer, 0u);
    VkCommandBufferBeginInfo begin{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(impl.command_buffer, &begin);
    impl.vertex_offset = 0u;
    impl.last_lighting_valid = false;
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
        vkDeviceWaitIdle(impl.device);
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
    vkCmdBlitImage(impl.command_buffer, impl.upload_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, target->color,
                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1u, &blit, VK_FILTER_LINEAR);
    impl.transition(impl.command_buffer, target->color, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    impl.last_drawn_target = display_address;
}

void VulkanRenderer::begin_display_list() {
    if (impl_) impl_->list_texture_keys.clear();
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
    const auto push_vertex = [&](const Vertex &vertex) {
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
        impl.scratch.push_back(out);
    };
    const auto vertex_at = [&](std::size_t index) -> const Vertex & {
        if (!call.indices.empty()) {
            const std::size_t mapped = call.indices[index];
            return call.vertices[std::min(mapped, call.vertices.size() - 1u)];
        }
        return call.vertices[std::min(index, call.vertices.size() - 1u)];
    };
    const std::size_t count = call.indices.empty() ? call.vertices.size() : call.indices.size();

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
        return;  // points and lines are not drawn yet
    }
    if (impl.scratch.empty()) return;

    static const bool trace = std::getenv("MHP3RD_TRACE_GE") != nullptr;
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


    // Deep dump of the first transformed draws: matrices, raw positions and the
    // same positions after a CPU-side transform, so a geometry that never shows
    // up can be traced to the stage that loses it.
    static const bool trace3d = std::getenv("MHP3RD_TRACE_3D") != nullptr;
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

    // The lighting block goes into the vertex buffer ahead of the vertices,
    // unless the previous draw left an identical one. Unlit, unfogged draws all
    // share a block of zeros apart from the world matrix, which they ignore.
    LightingBlock block{};
    const bool fogged = call.fog.enabled && !no_fog && !call.through && !call.clear_mode;
    if (lit || fogged) {
        const LightingState &state = call.lighting;
        block.world = call.world;
        const auto view_world = multiply(call.view, call.world);
        block.view_z = {view_world[2], view_world[6], view_world[10], view_world[14]};
        block.flags = {lit ? 1.0f : 0.0f, call.has_vertex_color ? 1.0f : 0.0f, fogged ? 1.0f : 0.0f,
                       static_cast<float>(state.material_update)};
        block.emissive = unpack_color(state.material_emissive, state.specular_power);
        block.material_ambient =
            unpack_color(call.material_color, static_cast<float>(call.material_color >> 24u) / 255.0f);
        block.material_diffuse = unpack_color(state.material_diffuse, static_cast<float>(state.mode));
        block.material_specular = unpack_color(state.material_specular, state.reverse_normals ? 1.0f : 0.0f);
        block.ambient = unpack_color(state.ambient_color, static_cast<float>(state.ambient_alpha) / 255.0f);
        block.fog = {call.fog.end, call.fog.scale, 0.0f, 0.0f};
        block.fog_color = unpack_color(call.fog.color);
        for (std::size_t i = 0; i < state.lights.size(); ++i) {
            const LightState &light = state.lights[i];
            block.light_position[i] = {light.position[0], light.position[1], light.position[2],
                                       light.enabled ? 1.0f : 0.0f};
            block.light_direction[i] = {light.direction[0], light.direction[1], light.direction[2],
                                        static_cast<float>(light.type)};
            block.light_attenuation[i] = {light.attenuation[0], light.attenuation[1], light.attenuation[2],
                                          static_cast<float>(light.kind)};
            block.light_spot[i] = {light.spot_exponent, light.spot_cutoff, 0.0f, 0.0f};
            block.light_ambient[i] = unpack_color(light.ambient);
            block.light_diffuse[i] = unpack_color(light.diffuse);
            block.light_specular[i] = unpack_color(light.specular);
        }
        if (!lit) {
            // Fog alone needs only the view-space z row.
            const auto keep_view_z = block.view_z;
            const auto keep_fog = block.fog;
            const auto keep_fog_color = block.fog_color;
            block = LightingBlock{};
            block.view_z = keep_view_z;
            block.flags[2] = 1.0f;
            block.fog = keep_fog;
            block.fog_color = keep_fog_color;
        }
    }
    const bool reuse_lighting =
        impl.last_lighting_valid && std::memcmp(&block, &impl.last_lighting, sizeof(block)) == 0;
    VkDeviceSize lighting_offset = impl.last_lighting_offset;
    VkDeviceSize vertex_start = impl.vertex_offset;
    if (!reuse_lighting) {
        lighting_offset = (impl.vertex_offset + impl.uniform_alignment - 1u) / impl.uniform_alignment *
                          impl.uniform_alignment;
        vertex_start = lighting_offset + sizeof(LightingBlock);
    }
    const VkDeviceSize bytes = impl.scratch.size() * sizeof(GpuVertex);
    if (vertex_start + bytes > kVertexBufferBytes) return;
    if (!reuse_lighting) {
        std::memcpy(static_cast<std::uint8_t *>(impl.vertex_mapped) + lighting_offset, &block, sizeof(block));
        impl.last_lighting = block;
        impl.last_lighting_offset = lighting_offset;
        impl.last_lighting_valid = true;
        impl.vertex_offset = vertex_start;
    }
    std::memcpy(static_cast<std::uint8_t *>(impl.vertex_mapped) + impl.vertex_offset, impl.scratch.data(),
                static_cast<std::size_t>(bytes));

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
    push.transform = multiply(call.projection, multiply(call.view, call.world));
    push.viewport = {static_cast<float>(kPspWidth), static_cast<float>(kPspHeight), call.through ? 1.0f : 0.0f, 0.0f};
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

    const Impl::Texture &texture = (call.texture.enabled && !call.clear_mode)
                                       ? impl.texture_for(memory, call.texture)
                                       : impl.white_texture;

    if (!impl.pass_active || impl.current_target != call.target.color_address) {
        impl.end_pass();
        impl.begin_pass(call.target.color_address);
        if (!impl.pass_active) return;
    }
    // The GE viewport maps normalised device coordinates onto the screen as
    //   screen = ndc * scale + offset - region_offset
    // with a negative y scale (PSP device y points up, the screen down) and a z
    // scale/offset that drives the 16-bit depth buffer. Folding all of that into
    // the Vulkan viewport reproduces the PSP's screen space exactly, including a
    // reversed depth range, and keeps triangle winding as the GE sees it. Through
    // draws bypass the transform, so they keep the plain full-target viewport.
    const float render_scale = static_cast<float>(impl.target_extent.width) / static_cast<float>(kPspWidth);
    VkViewport vk_viewport{0.0f, 0.0f, static_cast<float>(impl.target_extent.width),
                           static_cast<float>(impl.target_extent.height), 0.0f, 1.0f};
    if (!call.through && call.viewport.x_scale != 0.0f && call.viewport.y_scale != 0.0f) {
        const ViewportState &vp = call.viewport;
        vk_viewport.x = (vp.x_offset - vp.offset_x - vp.x_scale) * render_scale;
        vk_viewport.y = (vp.y_offset - vp.offset_y - vp.y_scale) * render_scale;
        vk_viewport.width = 2.0f * vp.x_scale * render_scale;
        // The GE's y scale is negative (device y points up, the screen down), so
        // this is a flipping viewport. That keeps framebuffer space identical to
        // the PSP's screen space, which is what the cull winding is defined in.
        vk_viewport.height = 2.0f * vp.y_scale * render_scale;
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
    VkRect2D vk_scissor{};
    vk_scissor.offset = {static_cast<std::int32_t>(sx1 * static_cast<std::uint32_t>(render_scale)),
                         static_cast<std::int32_t>(sy1 * static_cast<std::uint32_t>(render_scale))};
    vk_scissor.extent = {(sx2 - sx1 + 1u) * static_cast<std::uint32_t>(render_scale),
                         (sy2 - sy1 + 1u) * static_cast<std::uint32_t>(render_scale)};
    vkCmdSetScissor(impl.command_buffer, 0u, 1u, &vk_scissor);

    vkCmdBindPipeline(impl.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    vkCmdBindDescriptorSets(impl.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, impl.pipeline_layout, 0u, 1u,
                            &texture.descriptor, 0u, nullptr);
    const auto dynamic_offset = static_cast<std::uint32_t>(lighting_offset);
    vkCmdBindDescriptorSets(impl.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, impl.pipeline_layout, 1u, 1u,
                            &impl.lighting_descriptor, 1u, &dynamic_offset);
    vkCmdPushConstants(impl.command_buffer, impl.pipeline_layout,
                       VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0u, sizeof(push), &push);
    const VkDeviceSize offset = impl.vertex_offset;
    vkCmdBindVertexBuffers(impl.command_buffer, 0u, 1u, &impl.vertex_buffer, &offset);
    vkCmdDraw(impl.command_buffer, static_cast<std::uint32_t>(impl.scratch.size()), 1u, 0u, 0u);
    impl.vertex_offset += bytes;
    ++impl.draws;
}

void VulkanRenderer::present(std::uint32_t display_address) {
    Impl &impl = *impl_;
    if (!impl.ready) return;
    if (!impl.recording) begin_frame();
    impl.end_pass();

    static const bool trace3d = std::getenv("MHP3RD_TRACE_3D") != nullptr;
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
    impl.frame_through_draws = 0u;
    impl.frame_transformed_draws = 0u;
    impl.frame_transformed_vertices = 0u;
    impl.frame_onscreen_vertices = 0u;
    impl.frame_behind_camera = 0u;
    impl.frame_ndc_min = {1e30f, 1e30f, 1e30f};
    impl.frame_ndc_max = {-1e30f, -1e30f, -1e30f};
    impl.frame_transformed_targets.clear();

    // Show the target the guest flipped to; fall back to whatever was drawn last.
    auto displayed = impl.targets.find(display_address);
    if (displayed == impl.targets.end()) displayed = impl.targets.find(impl.last_drawn_target);
    const VkImage source = displayed != impl.targets.end() ? displayed->second.color : VK_NULL_HANDLE;
    impl.presented_target = displayed != impl.targets.end() ? displayed->first : 0u;
    impl.submit_and_present(source, true);
    ++impl.frames;
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
    impl.destroy_texture(impl.white_texture);
    if (impl.overlay_mapped != nullptr) vkUnmapMemory(impl.device, impl.overlay_staging_memory);
    vkDestroyBuffer(impl.device, impl.overlay_staging, nullptr);
    vkFreeMemory(impl.device, impl.overlay_staging_memory, nullptr);
    vkDestroyImageView(impl.device, impl.overlay_view, nullptr);
    vkDestroyImage(impl.device, impl.overlay_image, nullptr);
    vkFreeMemory(impl.device, impl.overlay_memory, nullptr);
    impl.destroy_upload();
    for (auto &[key, pipeline] : impl.pipelines) vkDestroyPipeline(impl.device, pipeline, nullptr);
    impl.pipelines.clear();
    if (impl.vertex_mapped != nullptr) vkUnmapMemory(impl.device, impl.vertex_memory);
    vkDestroyBuffer(impl.device, impl.vertex_buffer, nullptr);
    vkFreeMemory(impl.device, impl.vertex_memory, nullptr);
    vkDestroySampler(impl.device, impl.sampler, nullptr);
    vkDestroySampler(impl.device, impl.sharp_sampler, nullptr);
    vkDestroyDescriptorPool(impl.device, impl.descriptor_pool, nullptr);
    vkDestroyDescriptorSetLayout(impl.device, impl.descriptor_layout, nullptr);
    vkDestroyDescriptorSetLayout(impl.device, impl.lighting_layout, nullptr);
    vkDestroyPipelineLayout(impl.device, impl.pipeline_layout, nullptr);
    vkDestroyShaderModule(impl.device, impl.vertex_shader, nullptr);
    vkDestroyShaderModule(impl.device, impl.fragment_shader, nullptr);
    for (auto &[address, target] : impl.targets) impl.destroy_target(target);
    impl.targets.clear();
    vkDestroyRenderPass(impl.device, impl.render_pass, nullptr);
    vkDestroySemaphore(impl.device, impl.image_available, nullptr);
    vkDestroySemaphore(impl.device, impl.render_finished, nullptr);
    vkDestroyFence(impl.device, impl.frame_fence, nullptr);
    vkDestroyCommandPool(impl.device, impl.command_pool, nullptr);
    vkDestroySwapchainKHR(impl.device, impl.swapchain, nullptr);
    vkDestroyDevice(impl.device, nullptr);
    if (impl.surface != VK_NULL_HANDLE) vkDestroySurfaceKHR(impl.instance, impl.surface, nullptr);
    vkDestroyInstance(impl.instance, nullptr);
    if (impl.window != nullptr) SDL_DestroyWindow(impl.window);
    impl = Impl{};
}

} // namespace mhp3rd::gpu
