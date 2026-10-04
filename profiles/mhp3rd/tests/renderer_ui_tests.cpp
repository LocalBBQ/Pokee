// Real Vulkan/SDL/ImGui contracts with public synthetic buffers, no game data.
#include "gpu/vulkan_renderer.hpp"
#include "camera_probe.hpp"
#include "camera/free_camera.hpp"
#include "game/equipment_models.hpp"
#include "game/game_data.hpp"
#include "mods/mhp3rd_mods.hpp"
#include "mods/mhp3rd_data_bin.hpp"
#include "kernel/iso_image.hpp"
#include "ui/mods_screen.hpp"
#include "ui/texture_pack_screen.hpp"
#include "ui/controllers_screen.hpp"
#include "ui/save_screen.hpp"
#include "save_data/savedata_store.hpp"
#include "save_data/save_transfer.hpp"
#include "input/gamepad_devices.hpp"
#include "input/touch_controls.hpp"
#include "input/touch_action.hpp"
#include <algorithm>
#include <cmath>
#include <sstream>
#include "hle/hle_common.hpp"
#include "audio/audio_sink.hpp"
#include "adhoc/client.hpp"
#include "settings/settings.hpp"
#include "ui/layer.hpp"
#include "ui/text_input.hpp"
#include "ui/widgets.hpp"
#include "ui/file_browser.hpp"
#include "install/user_data.hpp"
#include "install/installer.hpp"
#include <thread>
#include <atomic>
#include "ui/ui.hpp"
#include "ui/bindings_editor.hpp"
#include "ui/touch_editor.hpp"
#include "ui/touch_overlay.hpp"
#include "imgui_internal.h"
#include "backends/imgui_impl_sdl3.h"
#include <fstream>
#include "imgui.h"
#include <SDL3/SDL.h>
#include <array>
#include <bit>
#include <filesystem>
#include <iostream>
#include <optional>
#include <vector>

namespace {
using namespace mhp3rd;
int failures{};
void expect(bool ok, const char *what) {
    if (!ok) {
        ++failures;
        std::cerr << "FAIL: " << what << '\n';
    }
}
void render_contracts(gpu::VulkanRenderer &renderer) {
    psprecomp::GuestMemory memory;
    constexpr std::uint32_t framebuffer = 0x04000000;
    const std::array<std::uint8_t, 16> rgba{255, 0, 0, 255, 0, 255, 0, 255, 0, 0, 255, 255, 255, 255, 255, 255};
    renderer.begin_frame();
    renderer.upload_frame(framebuffer, rgba.data(), 2, 2, 2);
    expect(renderer.present(framebuffer), "upload frame presents");
    std::vector<std::uint8_t> pixels;
    std::uint32_t width{}, height{};
    expect(renderer.read_frame(pixels, width, height), "offscreen frame reads back");
    expect(
        width == 480 && height == 272 && pixels.size() == width * height * 4, "native internal scale frame dimensions");
    if (width == 480 && height == 272 && pixels.size() == width * height * 4) {
        const auto at = [&](std::uint32_t x, std::uint32_t y, int channel) {
            return pixels[(y * width + x) * 4 + channel];
        };
        expect(at(40, 40, 0) > 240 && at(40, 40, 1) < 15 && at(40, 40, 2) < 15, "upload top-left red quadrant");
        expect(at(440, 40, 1) > 240 && at(440, 40, 0) < 15, "upload top-right green quadrant");
        expect(at(40, 230, 2) > 240 && at(40, 230, 0) < 15, "upload bottom-left blue quadrant");
    }
    // Through-mode clear sprites fill the guest framebuffer with a known color.
    gpu::DrawCall clear{};
    clear.primitive = gpu::PrimitiveType::Sprites;
    clear.through = true;
    clear.clear_mode = true;
    clear.clear_flags = 7;
    clear.target.color_address = framebuffer;
    clear.target.color_stride = 512;
    clear.target.color_format = 3;
    clear.target.depth_address = 0x04088000;
    clear.target.depth_stride = 512;
    clear.viewport.scissor_x2 = 479;
    clear.viewport.scissor_y2 = 271;
    gpu::Vertex a, b;
    a.position = {0, 0, 0, 1};
    b.position = {480, 272, 65535, 1};
    a.color = b.color = 0xff3366cc;
    clear.has_vertex_color = true;
    clear.vertices = {a, b};
    renderer.begin_frame();
    renderer.begin_display_list();
    renderer.submit(clear, memory);
    expect(renderer.present(framebuffer), "GE clear frame presents");
    expect(renderer.read_frame(pixels, width, height), "GE clear framebuffer reads");
    if (pixels.size() > 4) {
        expect(pixels[0] == 0xcc && pixels[1] == 0x66 && pixels[2] == 0x33 && pixels[3] == 255,
            "GE clear keeps channel order");
    }
    renderer.read_back_framebuffer(framebuffer, memory);
    expect(memory.load32(framebuffer) == 0xff3366cc, "GE block transfer source sees GPU framebuffer contents");
    renderer.present(framebuffer);
    const std::array<std::uint32_t, 4> packed_magenta{0xf81f, 0xfc1f, 0xff0f, 0xffff00ff};
    for (std::uint32_t format = 0; format < packed_magenta.size(); ++format) {
        clear.target.color_format = format;
        for (auto &v : clear.vertices) v.color = 0xffff00ff;
        renderer.begin_frame();
        renderer.submit(clear, memory);
        expect(renderer.present(framebuffer), "each PSP framebuffer format presents");
        renderer.read_back_framebuffer(framebuffer, memory);
        const auto actual = format == 3 ? memory.load32(framebuffer) : memory.load16(framebuffer);
        expect(actual == packed_magenta[format], "framebuffer readback packs 5650/5551/4444/8888 exactly");
        renderer.present(framebuffer);
    }
    clear.target.color_format = 3;
    renderer.set_internal_scale(2);
    renderer.begin_frame();
    renderer.present(framebuffer);
    expect(renderer.target_size() == std::array<std::uint32_t, 2>{960, 544}, "internal scale resizes render targets");
    renderer.set_internal_scale(1);
    renderer.set_sharp_screen(true);
    renderer.set_sharp_textures(true);
    renderer.set_aspect(settings::Aspect::Original);
    expect(renderer.game_aspect() > 1.76f && renderer.game_aspect() < 1.77f, "original guest aspect fixed");
    renderer.set_texture_pack(false);
    expect(renderer.texture_pack_status() == "Off", "disabled texture pack status");
    expect(
        !renderer.device_name().empty() && !renderer.device_summary().empty(), "real Vulkan device identity reported");
}
void free_camera_lifecycle_contracts() {
    psprecomp::Runtime runtime;
    auto &memory = runtime.memory();
    auto &player = settings::current();
    player.free_camera = true;
    player.free_camera_speed = 100.0f;
    camera::FreeCameraRequest request;
    request.toggle = true;
    camera::free_camera_update(runtime, request, 0.1f);
    expect(!camera::free_camera_active(), "free camera rejects absent game camera");
    constexpr std::uint32_t object = 0x08900000u;
    memory.store32(0x08A2F958u, object);
    auto store = [&](std::uint32_t offset, float value) {
        memory.store32(object + offset, std::bit_cast<std::uint32_t>(value));
    };
    store(0, 30.0f);
    store(4, 65000.0f);
    store(8, 480.0f / 272.0f);
    store(12, 0.8722222f);
    const auto original = camera::view_of_pose({{10, 20, 30}, 0, 0});
    for (std::uint32_t i = 0; i < original.size(); ++i) store(0xf50u + i * 4, original[i]);
    camera::free_camera_update(runtime, request, 0.1f);
    expect(camera::free_camera_active() && !camera::free_camera_status().paused,
        "valid public camera object starts free camera unpaused");
    request = {};
    request.input.forward = 1;
    camera::free_camera_update(runtime, request, 0.1f);
    auto hook = camera::free_camera_view_hook(memory);
    expect(bool(hook), "active camera installs view hook");
    gpu::DrawCall moved, other;
    moved.view = original;
    other.view = original;
    other.view[12] += 99;
    if (hook) {
        hook(moved);
        hook(other);
    }
    const auto pose = camera::pose_of_view(moved.view);
    expect(pose && std::abs(pose->eye[2] - 40.0f) < 0.001f,
        "free camera moves matching game view by speed times elapsed seconds");
    expect(other.view[12] == original[12] + 99, "unrelated view remains unchanged");
    camera::free_camera_frame_end(runtime);
    const auto status = camera::free_camera_status();
    expect(status.moved_draws == 1 && status.other_draws == 1,
        "frame end publishes exact moved and untouched draw counts");
    request = {};
    request.pause = true;
    request.speed_steps = 100;
    camera::free_camera_update(runtime, request, 0);
    expect(camera::free_camera_status().paused && camera::free_camera_status().speed == settings::kMaxFreeCameraSpeed,
        "photo pause and maximum speed clamp apply");
    request.speed_steps = -100;
    camera::free_camera_update(runtime, request, 0);
    expect(!camera::free_camera_status().paused && camera::free_camera_status().speed == settings::kMinFreeCameraSpeed,
        "second pause resumes and minimum speed clamps");
    request = {};
    request.reset = true;
    camera::free_camera_update(runtime, request, 0);
    moved.view = original;
    camera::free_camera_view_hook(memory)(moved);
    expect(camera::same_uploaded(moved.view, original), "reset restores original game pose");
    player.free_camera = false;
    camera::free_camera_update(runtime, {}, 0);
    expect(!camera::free_camera_active() && !camera::free_camera_status().paused,
        "disabling free camera leaves active and photo state");
    expect(player.free_camera_speed == settings::kMinFreeCameraSpeed,
        "leaving persists chosen camera speed in sandbox settings");
}

void keyboard_contracts(gpu::VulkanRenderer &renderer) {
    auto &layer = ui::Layer::get();
    expect(layer.attach(renderer), "real ImGui Vulkan layer attaches");
    if (!layer.attached()) return;
    layer.set_interactive(true);
    auto frame = [&]() {
        layer.begin_frame();
        ui::text_input_frame();
        layer.end_frame();
        renderer.present_ui(false);
    };
    auto press = [&](ImGuiKey key) {
        ImGui::GetIO().AddKeyEvent(key, true);
        frame();
        ImGui::GetIO().AddKeyEvent(key, false);
        frame();
    };
    std::optional<std::string> result;
    int callbacks{};
    ui::TextInputRequest request;
    request.title = "Synthetic nickname";
    request.initial = "A!B";
    request.max_length = 4;
    request.allowed = [](char32_t c) { return c >= U'A' && c <= U'Z'; };
    ui::open_text_input(request, [&](auto text) {
        ++callbacks;
        result = std::move(text);
    });
    frame();
    ImGui::GetIO().AddInputCharactersUTF8("C!DE");
    frame();
    press(ImGuiKey_Enter);
    expect(result == "ABCD" && callbacks == 1 && !ui::text_input_open(),
        "keyboard filters disallowed characters and truncates at character limit");
    request.initial = "ABCD";
    request.allowed = nullptr;
    request.max_length = 8;
    ui::open_text_input(request, [&](auto text) {
        ++callbacks;
        result = std::move(text);
    });
    frame();
    press(ImGuiKey_Home);
    press(ImGuiKey_Delete);
    press(ImGuiKey_End);
    press(ImGuiKey_Backspace);
    press(ImGuiKey_LeftArrow);
    ImGui::GetIO().AddInputCharactersUTF8("X");
    frame();
    press(ImGuiKey_Enter);
    expect(result == "BXC" && callbacks == 2, "keyboard home/end/delete/backspace/cursor insertion contract");
    ui::open_text_input(request, [&](auto text) {
        ++callbacks;
        result = std::move(text);
    });
    ui::cancel_text_input();
    ui::cancel_text_input();
    expect(!result && callbacks == 3, "cancel returns null once");
    ui::open_text_input(request, [&](auto text) {
        ++callbacks;
        result = std::move(text);
    });
    ui::open_text_input(request, {});
    expect(!result && callbacks == 4 && ui::text_input_open(), "replacement cancels existing keyboard");
    ui::cancel_text_input();
    expect(ui::printable_ascii(U' ') && ui::printable_ascii(U'~') && !ui::printable_ascii(U'\n') &&
            !ui::printable_ascii(U'\u00e9'),
        "printable ASCII boundaries");
    expect(ui::hunter_name_character(U'A') && ui::hunter_name_character(U'9') && !ui::hunter_name_character(U'%'),
        "hunter name allowed set");
    request.initial = "A\xc3\xa9\xe2\x82\xac\xf0\x9f\x98\x80";
    request.allowed = [](char32_t) { return true; };
    request.max_length = 4;
    ui::open_text_input(request, [&](auto text) { result = std::move(text); });
    frame();
    ImGui::GetIO().AddInputCharactersUTF8("Z");
    frame();
    press(ImGuiKey_Enter);
    expect(result == request.initial,
        "UTF-8 one/two/three/four-byte characters roundtrip and character limit rejects overflow");
    ui::open_text_input(request, [&](auto text) { result = std::move(text); });
    frame();
    press(ImGuiKey_Backspace);
    press(ImGuiKey_Enter);
    expect(result == "A\xc3\xa9\xe2\x82\xac", "backspace removes one Unicode character rather than one byte");

    layer.set_interactive(false);
}

void primitive_contracts(gpu::VulkanRenderer &renderer) {
    const bool trace_fb = std::getenv("MHP3RD_TRACE_FB_TEXTURES") != nullptr;
    const bool trace_3d = std::getenv("MHP3RD_TRACE_3D") != nullptr;
    std::ostringstream trace;
    struct RestoreTrace {
        std::streambuf *previous;
        ~RestoreTrace() {
            if (previous) std::cout.rdbuf(previous);
        }
    } restore{trace_fb || trace_3d ? std::cout.rdbuf(trace.rdbuf()) : nullptr};
    psprecomp::GuestMemory memory;
    gpu::DrawCall clear{};
    clear.primitive = gpu::PrimitiveType::Sprites;
    clear.through = true;
    clear.clear_mode = true;
    clear.clear_flags = 7;
    clear.has_vertex_color = true;
    clear.target.color_address = 0x04000000;
    clear.target.color_stride = 512;
    clear.target.color_format = 3;
    clear.target.depth_address = 0x04088000;
    gpu::Vertex lo, hi;
    lo.position = {0, 0, 0, 1};
    hi.position = {480, 272, 65535, 1};
    lo.color = hi.color = 0xff000000;
    clear.vertices = {lo, hi};
    auto draw = clear;
    draw.clear_mode = false;
    auto pixel = [&]() {
        std::vector<std::uint8_t> bytes;
        std::uint32_t w{}, h{};
        expect(renderer.read_frame(bytes, w, h), "primitive frame readback succeeds");
        if (bytes.size() < (150 * w + 240) * 4 + 4) return std::uint32_t{0};
        const auto at = (150 * w + 240) * 4;
        return static_cast<std::uint32_t>(bytes[at]) | (static_cast<std::uint32_t>(bytes[at + 1]) << 8) |
            (static_cast<std::uint32_t>(bytes[at + 2]) << 16) | (static_cast<std::uint32_t>(bytes[at + 3]) << 24);
    };
    auto vertex = [](float x, float y) {
        gpu::Vertex v;
        v.position = {x, y, 0, 1};
        v.color = 0xff2255cc;
        return v;
    };
    for (auto primitive : {gpu::PrimitiveType::Triangles, gpu::PrimitiveType::TriangleStrip,
             gpu::PrimitiveType::TriangleFan, gpu::PrimitiveType::Sprites}) {
        draw.primitive = primitive;
        if (primitive == gpu::PrimitiveType::Triangles)
            draw.vertices = {vertex(60, 60), vertex(420, 60), vertex(240, 230)};
        else if (primitive == gpu::PrimitiveType::TriangleStrip)
            draw.vertices = {vertex(60, 60), vertex(420, 60), vertex(60, 230), vertex(420, 230)};
        else if (primitive == gpu::PrimitiveType::TriangleFan)
            draw.vertices = {vertex(60, 60), vertex(420, 60), vertex(420, 230), vertex(60, 230)};
        else
            draw.vertices = {vertex(60, 60), vertex(420, 230)};
        renderer.begin_frame();
        renderer.submit(clear, memory);
        renderer.submit(draw, memory);
        renderer.present(0x04000000);
        expect(pixel() == 0xff2255cc, "through primitive expansion renders specified color at interior pixel");
    }
    // Alpha-test equality boundaries are observable as an exact pixel or
    // untouched clear color, independent of the GPU's framebuffer alpha mode.
    for (auto &v : draw.vertices) v.color = 0x802255cc;
    draw.alpha_test.enabled = true;
    draw.alpha_test.reference = 128;
    const std::array<bool, 8> accepted{true, false, true, false, false, true, false, true};
    for (std::uint32_t function = 0; function < accepted.size(); ++function) {
        draw.alpha_test.function = function;
        renderer.begin_frame();
        renderer.submit(clear, memory);
        renderer.submit(draw, memory);
        renderer.present(0x04000000);
        expect((pixel() & 0xffffff) == (accepted[function] ? 0x2255ccu : 0u),
            "alpha comparison accepts or discards equal-reference fragment");
    }
    draw.alpha_test.enabled = false;
    for (auto &v : draw.vertices) v.color = 0xff2255cc;
    draw.blend.enabled = true;
    draw.blend.source_factor = draw.blend.destination_factor = 10;
    draw.blend.fixed_source = draw.blend.fixed_destination = 0xffffff;
    const std::array<std::uint32_t, 5> blended{0x2255cc, 0x2255cc, 0, 0, 0x2255cc};
    for (std::uint32_t equation = 0; equation < blended.size(); ++equation) {
        draw.blend.equation = equation;
        renderer.begin_frame();
        renderer.submit(clear, memory);
        renderer.submit(draw, memory);
        renderer.present(0x04000000);
        expect((pixel() & 0xffffff) == blended[equation],
            "blend add/subtract/reverse/min/max preserve specified operands");
    }
    draw.blend.equation = 0;
    draw.blend.destination_factor = 10;
    draw.blend.fixed_destination = 0;
    for (auto &v : draw.vertices) v.color = 0x80404040;
    const std::array<int, 6> source_blended{0, 64, 32, 32, 64, 0};
    for (std::uint32_t factor = 0; factor < source_blended.size(); ++factor) {
        draw.blend.source_factor = factor;
        renderer.begin_frame();
        renderer.submit(clear, memory);
        renderer.submit(draw, memory);
        renderer.present(0x04000000);
        const auto color = pixel();
        expect(std::abs(static_cast<int>(color & 255) - source_blended[factor]) <= 1 &&
                ((color >> 8) & 255) == (color & 255) && ((color >> 16) & 255) == (color & 255),
            "source blend factor gives documented gray RGB within one UNORM rounding step");
    }
    draw.blend.source_factor = 10;
    draw.blend.fixed_source = 0;
    for (auto &v : clear.vertices) v.color = 0x20404040;
    const std::array<int, 6> destination_blended{16, 48, 32, 32, 8, 56};
    for (std::uint32_t factor = 0; factor < destination_blended.size(); ++factor) {
        draw.blend.destination_factor = factor;
        renderer.begin_frame();
        renderer.submit(clear, memory);
        renderer.submit(draw, memory);
        renderer.present(0x04000000);
        expect(std::abs(static_cast<int>(pixel() & 255) - destination_blended[factor]) <= 1,
            "destination blend factor scales retained framebuffer by documented operand");
    }
    draw.blend.source_factor = draw.blend.destination_factor = 10;
    draw.blend.fixed_source = 0x808080;
    draw.blend.fixed_destination = 0x7f7f7f;
    for (auto &v : clear.vertices) v.color = 0xff202020;
    renderer.begin_frame();
    renderer.submit(clear, memory);
    renderer.submit(draw, memory);
    renderer.present(0x04000000);
    expect(std::abs(static_cast<int>(pixel() & 255) - 48) <= 1,
        "complementary fixed constants independently mix source gray64 and destination gray32");
    for (auto &v : clear.vertices) v.color = 0xff000000;
    for (auto &v : draw.vertices) v.color = 0xff2255cc;
    draw.blend.enabled = false;
    draw.viewport.scissor_x2 = 200;
    renderer.begin_frame();
    renderer.submit(clear, memory);
    renderer.submit(draw, memory);
    renderer.present(0x04000000);
    expect((pixel() & 0xffffff) == 0, "scissor boundary excludes interior sample outside the allowed region");
    draw.viewport.scissor_x2 = 479;
    draw.primitive = gpu::PrimitiveType::Triangles;
    draw.through = false;
    draw.vertices = {vertex(-0.8f, -0.8f), vertex(0.8f, -0.8f), vertex(0, 0.8f)};
    for (auto matrix : {&draw.world, &draw.view, &draw.projection, &draw.texture_matrix}) {
        matrix->fill(0);
        (*matrix)[0] = (*matrix)[5] = (*matrix)[10] = (*matrix)[15] = 1;
    }
    draw.viewport.x_scale = 240;
    draw.viewport.y_scale = -136;
    draw.viewport.x_offset = 240;
    draw.viewport.y_offset = 136;
    renderer.begin_frame();
    renderer.submit(clear, memory);
    renderer.submit(draw, memory);
    renderer.present(0x04000000);
    expect(pixel() == 0xff2255cc, "transformed triangle uses matrices and PSP viewport");
    for (auto &v : draw.vertices) v.position[2] = -1;
    draw.projection[10] = -1.020202f;
    draw.projection[11] = -1;
    draw.projection[14] = -0.2020202f;
    draw.projection[15] = 0;
    draw.primitive_count = 3;
    renderer.set_frame_rate_auto(false);
    renderer.set_frame_rate(settings::FrameRate::Fps60);
    const auto presented_before = renderer.frames_presented();
    const int replay_frames = std::getenv("MHP3RD_CHECK_REPLAY") ? 160 : 16;
    int deferred{};
    for (int frame = 0; frame < replay_frames; ++frame) {
        draw.world[12] = static_cast<float>(frame) * 0.002f;
        draw.lighting_enabled = frame >= 8;
        draw.lighting.material_update = 1;
        draw.lighting.ambient_color = 0x00ffffff;
        draw.lighting.lights[0].enabled = true;
        draw.lighting.lights[0].type = static_cast<std::uint32_t>(frame % 3);
        draw.lighting.lights[0].position = {0, 0, 1};
        draw.lighting.lights[0].direction = {0, 0, -1};
        draw.environment_version = static_cast<std::uint64_t>(frame + 1);
        const auto moment = std::chrono::steady_clock::now();
        renderer.begin_frame();
        renderer.submit(clear, memory);
        renderer.submit(draw, memory);
        // A repeated draw also exercises replay-group batching.
        renderer.submit(draw, memory);
        if (!renderer.present(0x04000000, moment)) ++deferred;
        renderer.present_due();
        renderer.present_until(moment + std::chrono::milliseconds(33));
    }
    expect(deferred == replay_frames, "60 fps guest flips defer rendering to interpolation schedule");
    expect(renderer.frames_presented() == presented_before + replay_frames && renderer.frame_rate_now() >= 59.0,
        "guest frame counter advances once per flip while interpolation retains requested rate");
    expect(pixel() == 0xff2255cc, "interpolation replay preserves interior opaque triangle color");
    renderer.pause_interpolation();
    renderer.set_still(true);
    renderer.begin_frame();
    renderer.submit(clear, memory);
    renderer.submit(draw, memory);
    expect(renderer.present(0x04000000), "photo still presents directly instead of interpolating");
    renderer.set_still(false);
    renderer.set_fast_forward(true);
    renderer.begin_frame();
    renderer.submit(clear, memory);
    renderer.submit(draw, memory);
    renderer.present(0x04000000);
    renderer.set_fast_forward(false);
    renderer.set_frame_rate(settings::FrameRate::Fps30);
    renderer.set_frame_rate_auto(true);
    draw.world[12] = 0;
    draw.lighting_enabled = false;
    for (auto &v : draw.vertices) v.position[2] = 0;
    draw.projection[10] = draw.projection[15] = 1;
    draw.projection[11] = draw.projection[14] = 0;
    draw.depth.test_enabled = true;
    const std::array<bool, 8> depth_accepted{false, true, false, true, true, true, false, false};
    for (std::uint32_t function = 0; function < depth_accepted.size(); ++function) {
        draw.depth.function = function;
        renderer.begin_frame();
        renderer.submit(clear, memory);
        renderer.submit(draw, memory);
        renderer.present(0x04000000);
        expect((pixel() & 0xffffff) == (depth_accepted[function] ? 0x2255ccu : 0u),
            "depth comparison tests exact ordering against the cleared far plane");
    }
    draw.depth.test_enabled = false;

    draw.fog.enabled = true;
    draw.fog.scale = 1;
    draw.fog.color = 0x000000ff;
    for (const float fog_end : {0.0f, 0.5f, 1.0f}) {
        draw.fog.end = fog_end;
        renderer.begin_frame();
        renderer.submit(clear, memory);
        renderer.submit(draw, memory);
        renderer.present(0x04000000);
        const auto color = pixel();
        const int red = static_cast<int>(255 * (1 - fog_end) + 204 * fog_end);
        const int green = static_cast<int>(85 * fog_end);
        const int blue = static_cast<int>(34 * fog_end);
        expect(std::abs(static_cast<int>(color & 255) - red) <= 1 &&
                std::abs(static_cast<int>((color >> 8) & 255) - green) <= 1 &&
                std::abs(static_cast<int>((color >> 16) & 255) - blue) <= 1,
            "view-distance fog independently mixes red fog with triangle color at both boundaries and midpoint");
    }
    draw.fog.enabled = false;
    draw.culling_enabled = true;
    std::array<std::uint32_t, 2> culled{};
    for (int winding = 0; winding < 2; ++winding) {
        draw.cull_clockwise = winding != 0;
        renderer.begin_frame();
        renderer.submit(clear, memory);
        renderer.submit(draw, memory);
        renderer.present(0x04000000);
        culled[winding] = pixel() & 0xffffff;
    }
    expect((culled[0] == 0 && culled[1] == 0x2255cc) || (culled[0] == 0x2255cc && culled[1] == 0),
        "opposite cull winding admits exactly one orientation of the same transformed triangle");
    draw.culling_enabled = false;

    for (const bool degenerate : {true, false}) {
        draw.indices = degenerate ? std::vector<std::uint16_t>{0, 1, 0} : std::vector<std::uint16_t>{2, 0, 1};
        renderer.begin_frame();
        renderer.submit(clear, memory);
        renderer.submit(draw, memory);
        renderer.present(0x04000000);
        expect((pixel() & 0xffffff) == (degenerate ? 0 : 0x2255cc),
            "decoded indexed geometry obeys repeated versus reordered vertex indices");
    }
    draw.indices.clear();

    // Feed the PSP byte layout directly to the shader vertex decoder.
    std::array<std::uint32_t, 12> raw{};
    const std::array<std::array<float, 3>, 3> positions{{{-0.8f, -0.8f, -1}, {0.8f, -0.8f, -1}, {0, 0.8f, -1}}};
    for (std::size_t i = 0; i < positions.size(); ++i) {
        raw[i * 4] = 0xff2255cc;
        for (std::size_t axis = 0; axis < 3; ++axis)
            raw[i * 4 + axis + 1] = std::bit_cast<std::uint32_t>(positions[i][axis]);
    }
    // CHECK_GPU_DECODE also needs independently specified decoded reference vertices.
    draw.vertices.clear();
    for (const auto &position : positions) {
        auto decoded = vertex(position[0], position[1]);
        decoded.position[2] = position[2];
        draw.vertices.push_back(decoded);
    }
    draw.raw_vertices = reinterpret_cast<const std::uint8_t *>(raw.data());
    draw.raw_count = 3;
    draw.raw_stride = 16;
    draw.vertex_type = (7u << 2) | (3u << 7);
    renderer.begin_frame();
    renderer.submit(clear, memory);
    renderer.submit(draw, memory);
    renderer.present(0x04000000);
    expect(pixel() == 0xff2255cc, "raw PSP vertices decode color and float positions on the GPU");
    for (const std::uint32_t index_type : {1u, 2u}) {
        draw.vertex_type = (7u << 2) | (3u << 7) | (index_type << 11);
        for (const bool degenerate : {true, false}) {
            draw.indices = degenerate ? std::vector<std::uint16_t>{0, 1, 0} : std::vector<std::uint16_t>{2, 0, 1};
            renderer.begin_frame();
            renderer.submit(clear, memory);
            renderer.submit(draw, memory);
            renderer.present(0x04000000);
            expect((pixel() & 0xffffff) == (degenerate ? 0 : 0x2255cc),
                "raw indexed geometry obeys normalized PSP byte/word index order and repeated-index degeneracy");
        }
    }
    draw.indices.clear();
    draw.vertex_type = (7u << 2) | (3u << 7);

    draw.projection[10] = -1.020202f;
    draw.projection[11] = -1;
    draw.projection[14] = -0.2020202f;
    draw.projection[15] = 0;
    renderer.set_frame_rate_auto(false);
    renderer.set_frame_rate(settings::FrameRate::Fps60);
    int raw_deferred{};
    for (int frame = 0; frame < 8; ++frame) {
        draw.world[12] = static_cast<float>(frame) * 0.003f;
        draw.lighting_enabled = frame >= 4;
        const auto moment = std::chrono::steady_clock::now();
        renderer.begin_frame();
        renderer.submit(clear, memory);
        renderer.submit(draw, memory);
        if (!renderer.present(0x04000000, moment)) ++raw_deferred;
        renderer.present_until(moment + std::chrono::milliseconds(33));
    }
    expect(raw_deferred == 8 && pixel() == 0xff2255cc,
        "raw vertex replay schedules all frames and preserves decoded lit color");
    std::array<std::uint32_t, 15> weighted{};
    for (std::size_t i = 0; i < positions.size(); ++i) {
        weighted[i * 5] = std::bit_cast<std::uint32_t>(1.0f);
        weighted[i * 5 + 1] = 0xff2255cc;
        for (std::size_t axis = 0; axis < 3; ++axis)
            weighted[i * 5 + axis + 2] = std::bit_cast<std::uint32_t>(positions[i][axis]);
    }
    std::array<float, 96> bones{};
    for (std::size_t bone = 0; bone < 8; ++bone) bones[bone * 12] = bones[bone * 12 + 4] = bones[bone * 12 + 8] = 1;
    draw.raw_vertices = reinterpret_cast<const std::uint8_t *>(weighted.data());
    draw.raw_stride = 20;
    draw.vertex_type |= 3u << 9;
    draw.bone_matrices = bones.data();
    int weighted_deferred{};
    for (int frame = 0; frame < 8; ++frame) {
        bones[9] = static_cast<float>(frame) * 0.004f;
        for (std::size_t i = 0; i < positions.size(); ++i) draw.vertices[i].position[0] = positions[i][0] + bones[9];
        const auto moment = std::chrono::steady_clock::now();
        renderer.begin_frame();
        renderer.submit(clear, memory);
        renderer.submit(draw, memory);
        if (!renderer.present(0x04000000, moment)) ++weighted_deferred;
        renderer.present_until(moment + std::chrono::milliseconds(33));
    }
    expect(weighted_deferred == 8 && pixel() == 0xff2255cc,
        "single-weight raw skinning and animated bone replay preserve lit interior color");
    draw.bone_matrices = nullptr;
    draw.raw_vertices = nullptr;
    draw.raw_count = draw.raw_stride = 0;
    int cooked_deferred{};
    for (int frame = 0; frame < 8; ++frame) {
        for (std::size_t i = 0; i < positions.size(); ++i)
            draw.vertices[i].position[0] = positions[i][0] + static_cast<float>(frame) * 0.005f;
        const auto moment = std::chrono::steady_clock::now();
        renderer.begin_frame();
        renderer.submit(clear, memory);
        renderer.submit(draw, memory);
        if (!renderer.present(0x04000000, moment)) ++cooked_deferred;
        renderer.present_until(moment + std::chrono::milliseconds(33));
    }
    expect(cooked_deferred == 8 && pixel() == 0xff2255cc,
        "CPU-skinned vertex replay interpolates animated decoded positions without changing material color");
    renderer.pause_interpolation();
    renderer.set_frame_rate(settings::FrameRate::Fps30);
    renderer.set_frame_rate_auto(true);
    draw.world[12] = 0;
    draw.lighting_enabled = false;
    draw.raw_vertices = nullptr;
    draw.raw_count = draw.raw_stride = 0;
    draw.through = true;
    draw.primitive = gpu::PrimitiveType::Sprites;
    draw.vertices = {vertex(60, 60), vertex(420, 230)};
    draw.vertices[0].texcoord = {0, 0};
    draw.vertices[1].texcoord = {2, 2};
    draw.texture.enabled = true;
    draw.texture.address = 0x08008000;
    draw.texture.width = draw.texture.height = 2;
    draw.texture.buffer_width = 2;
    draw.texture.format = gpu::TextureFormat::Rgba8888;
    draw.texture.function = 3;
    draw.texture.alpha_from_texture = true;
    for (std::uint32_t i = 0; i < 4; ++i) memory.store32(draw.texture.address + i * 4, 0xffee3311);
    renderer.begin_frame();
    renderer.begin_display_list();
    renderer.submit(clear, memory);
    renderer.submit(draw, memory);
    renderer.submit(draw, memory);
    renderer.present(0x04000000);
    expect(pixel() == 0xffee3311, "textured sprite replaces vertex color and repeated draw reuses cached texture");
    memory.store32(draw.texture.address, 0xff11cc77);
    memory.store32(draw.texture.address + 4, 0xff11cc77);
    memory.store32(draw.texture.address + 8, 0xff11cc77);
    memory.store32(draw.texture.address + 12, 0xff11cc77);
    renderer.begin_frame();
    renderer.begin_display_list();
    renderer.submit(clear, memory);
    renderer.submit(draw, memory);
    renderer.present(0x04000000);
    expect(pixel() == 0xff11cc77, "rewriting texture bytes invalidates cache on next display list");
    draw.texture.clut_address = 0x0800a000;
    draw.texture.clut_format = 3;
    draw.texture.clut_mask = 255;
    draw.texture.clut_load_bytes = draw.texture.clut_max_bytes = 1024;
    memory.store32(draw.texture.clut_address + 4, 0xffff00ff);
    const std::array<std::uint32_t, 4> packed{0xf81f, 0xfc1f, 0xff0f, 0xffff00ff};
    for (std::uint32_t format = 0; format < 8; ++format) {
        draw.texture.format = static_cast<gpu::TextureFormat>(format);
        for (std::uint32_t i = 0; i < 4; ++i) memory.store32(draw.texture.address + i * 4, 0);
        if (format < 3) {
            for (std::uint32_t i = 0; i < 4; ++i)
                memory.store16(draw.texture.address + i * 2, static_cast<std::uint16_t>(packed[format]));
        } else if (format == 3) {
            for (std::uint32_t i = 0; i < 4; ++i) memory.store32(draw.texture.address + i * 4, packed[format]);
        } else if (format == 4) {
            memory.store8(draw.texture.address, 0x11);
            memory.store8(draw.texture.address + 1, 0x11);
        } else if (format == 5) {
            for (std::uint32_t i = 0; i < 4; ++i) memory.store8(draw.texture.address + i, 1);
        } else if (format == 6) {
            for (std::uint32_t i = 0; i < 4; ++i) memory.store16(draw.texture.address + i * 2, 1);
        } else {
            for (std::uint32_t i = 0; i < 4; ++i) memory.store32(draw.texture.address + i * 4, 1);
        }
        renderer.begin_frame();
        renderer.begin_display_list();
        renderer.submit(clear, memory);
        renderer.submit(draw, memory);
        renderer.present(0x04000000);
        expect(pixel() == 0xffff00ff, "real GPU texture path decodes each direct and indexed PSP format");
    }
    draw.texture.width = draw.texture.height = draw.texture.buffer_width = 4;
    for (auto format : {gpu::TextureFormat::Dxt1, gpu::TextureFormat::Dxt3, gpu::TextureFormat::Dxt5}) {
        draw.texture.format = format;
        memory.store32(draw.texture.address, 0); // PSP colour indices precede endpoints.
        memory.store32(draw.texture.address + 4, 0x0000f81f);
        memory.store32(draw.texture.address + 8, format == gpu::TextureFormat::Dxt3 ? 0xffffffff : 0);
        memory.store32(draw.texture.address + 12,
            format == gpu::TextureFormat::Dxt3 ? 0xffffffff : 0x00ff0000); // DXT5 alpha endpoint at byte14.
        renderer.begin_frame();
        renderer.begin_display_list();
        renderer.submit(clear, memory);
        renderer.submit(draw, memory);
        renderer.present(0x04000000);
        expect(pixel() == 0xffff00ff, "real GPU texture path renders PSP-layout DXT1, DXT3 and DXT5 blocks");
    }
    draw.texture.width = draw.texture.height = draw.texture.buffer_width = 2;
    draw.texture.format = gpu::TextureFormat::Rgba8888;
    for (std::uint32_t i = 0; i < 4; ++i) memory.store32(draw.texture.address + i * 4, 0xff11cc77);
    const auto ui_textures = settings::current().ui_textures;
    settings::current().ui_textures = settings::UiTextures::Mmpx;
    renderer.set_internal_scale(2);
    for (int frame = 0; frame < 2; ++frame) {
        renderer.begin_frame();
        renderer.begin_display_list();
        renderer.submit(clear, memory);
        renderer.submit(draw, memory);
        renderer.present(0x04000000);
        expect(renderer.target_size() == std::array<std::uint32_t, 2>{960, 544} && pixel() == 0xff11cc77,
            "MMPX upscaled UI texture and cached copy preserve uniform source pixel");
    }
    settings::current().ui_textures = ui_textures;
    renderer.set_internal_scale(1);
    // Sample an offscreen framebuffer as a texture before its bytes reach RAM.
    clear.vertices[0].color = clear.vertices[1].color = 0xffff00ff;
    draw.target.color_address = 0x04110000;
    draw.target.depth_address = 0x04198000;
    draw.vertices[0].position = {50, 50, 0, 1};
    draw.vertices[1].position = {430, 230, 0, 1};
    draw.vertices[0].texcoord = {0, 0};
    draw.vertices[1].texcoord = {480, 272};
    draw.texture.address = clear.target.color_address;
    draw.texture.buffer_width = 512;
    draw.texture.width = 480;
    draw.texture.height = 272;
    renderer.begin_frame();
    renderer.begin_display_list();
    renderer.submit(clear, memory);
    renderer.submit(draw, memory);
    renderer.present(draw.target.color_address);
    expect(pixel() == 0xffff00ff, "framebuffer texture copies freshly rendered source before guest RAM writeback");
    renderer.read_back_framebuffer(draw.target.color_address, memory);
    expect(memory.load32(draw.target.color_address + (150 * 512 + 240) * 4) == 0xffff00ff,
        "sampled framebuffer readback stores exact rendered color at guest stride");

    if (restore.previous) {
        std::cout.rdbuf(restore.previous);
        restore.previous = nullptr;
        const auto text = trace.str();
        // Keep all existing diagnostics visible to CTest's comparison failure patterns.
        std::cout << text;
        if (trace_fb)
            expect(text.find("[fbtex]") != std::string::npos && text.find("texture 0x4000000") != std::string::npos &&
                    text.find("drawn to 0x4110000") != std::string::npos &&
                    text.find("stride=512") != std::string::npos,
                "framebuffer diagnostic identifies the real synthetic source, destination and stride");
        if (trace_3d)
            expect(text.find("[3d] draw#") != std::string::npos && text.find("world =") != std::string::npos &&
                    text.find("proj  =") != std::string::npos && text.find("clip=(") != std::string::npos &&
                    text.find("scissor=(") != std::string::npos,
                "3D diagnostic reports matrix, clip-space and viewport contracts for actual transformed draws");
    }
}

void screenshot_contracts(gpu::VulkanRenderer &renderer, const std::filesystem::path &sandbox) {
    std::vector<std::uint8_t> pixels;
    std::uint32_t width = 0, height = 0;
    expect(renderer.read_frame(pixels, width, height), "screenshot source framebuffer is available");
    const auto path = sandbox / "public-frame.bmp";
    expect(renderer.capture_frame(path), "GPU framebuffer capture writes an isolated BMP");
    std::ifstream in(path, std::ios::binary);
    const std::vector<std::uint8_t> bmp{std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
    auto u32 = [&](std::size_t at) {
        return static_cast<std::uint32_t>(bmp[at]) | (static_cast<std::uint32_t>(bmp[at + 1]) << 8) |
            (static_cast<std::uint32_t>(bmp[at + 2]) << 16) | (static_cast<std::uint32_t>(bmp[at + 3]) << 24);
    };
    const auto stride = (width * 3 + 3) & ~3u;
    const bool header = bmp.size() == 54 + stride * height && bmp.size() >= 54 && bmp[0] == 'B' && bmp[1] == 'M';
    expect(header, "BMP contains exactly the public 24-bit header and padded image bytes");
    if (header) {
        expect(u32(2) == bmp.size() && u32(10) == 54 && u32(14) == 40 && u32(18) == width && u32(22) == height &&
                bmp[26] == 1 && bmp[28] == 24,
            "BMP header records framebuffer dimensions, size, offset and 24-bit pixels");
        bool equal = true;
        for (std::uint32_t y = 0; y < height; ++y) {
            for (std::uint32_t x = 0; x < width; ++x) {
                const auto src = (static_cast<std::size_t>(y) * width + x) * 4;
                const auto dst = 54 + static_cast<std::size_t>(height - 1 - y) * stride + x * 3;
                equal = equal && bmp[dst] == pixels[src + 2] && bmp[dst + 1] == pixels[src + 1] &&
                    bmp[dst + 2] == pixels[src];
            }
        }
        expect(equal, "every captured BMP pixel matches independent framebuffer readback with bottom-up BGR rows");
    }
    expect(!renderer.capture_frame(sandbox / "missing-directory" / "frame.bmp"),
        "framebuffer capture reports a destination failure without claiming success");
    const auto window_path = sandbox / "public-window.bmp";
    renderer.capture_window(window_path);
    expect(renderer.window_capture_pending(), "window capture queues one actual swapchain readback");
    auto &layer = ui::Layer::get();
    for (int i = 0; i < 3 && renderer.window_capture_pending(); ++i) {
        layer.begin_frame();
        layer.end_frame();
        renderer.present_ui(true);
    }
    expect(!renderer.window_capture_pending() && std::filesystem::is_regular_file(window_path) &&
            std::filesystem::file_size(window_path) > 54,
        "bounded presentation consumes window capture and writes pixel data");
    std::ifstream window_in(window_path, std::ios::binary);
    const std::vector<std::uint8_t> window_bmp{
        std::istreambuf_iterator<char>(window_in), std::istreambuf_iterator<char>()};
    if (window_bmp.size() >= 54) {
        auto window_u32 = [&](std::size_t at) {
            return static_cast<std::uint32_t>(window_bmp[at]) | (static_cast<std::uint32_t>(window_bmp[at + 1]) << 8) |
                (static_cast<std::uint32_t>(window_bmp[at + 2]) << 16) |
                (static_cast<std::uint32_t>(window_bmp[at + 3]) << 24);
        };
        const auto w = window_u32(18), h = window_u32(22);
        const auto row = (w * 3 + 3) & ~3u;
        expect(w > 0 && h > 0 && window_bmp.size() == 54 + static_cast<std::size_t>(row) * h,
            "window BMP dimensions match its exact padded byte count");
        const auto center = 54 + static_cast<std::size_t>(h / 2) * row + (w / 2) * 3;
        expect(center + 2 < window_bmp.size() && window_bmp[center] == 255 && window_bmp[center + 1] == 0 &&
                window_bmp[center + 2] == 255,
            "swapchain BMP channel conversion preserves the rendered magenta pixel");
    }
    renderer.capture_window(sandbox / "missing-directory" / "window.bmp");
    for (int i = 0; i < 3 && renderer.window_capture_pending(); ++i) {
        layer.begin_frame();
        layer.end_frame();
        renderer.present_ui(true);
    }
    expect(!renderer.window_capture_pending() && !std::filesystem::exists(sandbox / "missing-directory" / "window.bmp"),
        "failed window capture is consumed once without creating an invalid destination");
    renderer.set_perf_overlay(true);
    const auto overlay_window = sandbox / "public-overlay-window.bmp";
    renderer.capture_window(overlay_window);
    for (int i = 0; i < 3 && renderer.window_capture_pending(); ++i) {
        layer.begin_frame();
        layer.end_frame();
        expect(renderer.present(0x04110000), "performance overlay is drawn during actual game presentation");
    }
    const auto overlay_frame = sandbox / "public-overlay-frame.bmp";
    expect(renderer.capture_frame(overlay_frame), "frame capture includes the enabled performance overlay");
    for (const auto &overlay_path : {overlay_frame, overlay_window}) {
        std::ifstream overlay_in(overlay_path, std::ios::binary);
        const std::vector<std::uint8_t> overlay{
            std::istreambuf_iterator<char>(overlay_in), std::istreambuf_iterator<char>()};
        expect(overlay.size() >= 54, "performance overlay capture exists for both framebuffer and swapchain");
        if (overlay.size() < 54) continue;
        auto dimension = [&](std::size_t at) {
            return static_cast<std::uint32_t>(overlay[at]) | (static_cast<std::uint32_t>(overlay[at + 1]) << 8) |
                (static_cast<std::uint32_t>(overlay[at + 2]) << 16) |
                (static_cast<std::uint32_t>(overlay[at + 3]) << 24);
        };
        const auto w = dimension(18), h = dimension(22);
        const auto row = (w * 3 + 3) & ~3u;
        const auto inset = 4 * std::max(1u, h / 360);
        const auto corner = 54 + static_cast<std::size_t>(h - 1 - inset) * row + inset * 3;
        const auto outside = 54 + static_cast<std::size_t>(h / 2) * row + (w / 2) * 3;
        expect(corner + 2 < overlay.size() && overlay[corner] == 16 && overlay[corner + 1] == 16 &&
                overlay[corner + 2] == 16,
            "performance overlay paints its documented dark background in captured GPU/frame corner");
        expect(outside + 2 < overlay.size() && overlay[outside] == 255 && overlay[outside + 1] == 0 &&
                overlay[outside + 2] == 255,
            "performance overlay leaves the scene outside its rectangle unchanged");
    }
    renderer.set_perf_overlay(false);
    layer.begin_frame();
    layer.end_frame();
    renderer.present_ui(true);
}

void held_frame_contracts(gpu::VulkanRenderer &renderer, const std::filesystem::path &sandbox) {
    auto window_pixel = [&](const std::filesystem::path &path) {
        renderer.capture_window(path);
        expect(renderer.present(0x04110000), "held-frame contract presents actual guest framebuffer");
        std::ifstream in(path, std::ios::binary);
        const std::vector<std::uint8_t> bmp{std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
        if (bmp.size() < 54) {
            expect(false, "held-frame window capture exists");
            return std::uint32_t{0};
        }
        auto u32 = [&](std::size_t at) {
            return static_cast<std::uint32_t>(bmp[at]) | (static_cast<std::uint32_t>(bmp[at + 1]) << 8) |
                (static_cast<std::uint32_t>(bmp[at + 2]) << 16) | (static_cast<std::uint32_t>(bmp[at + 3]) << 24);
        };
        const auto w = u32(18), h = u32(22), row = (w * 3 + 3) & ~3u;
        const auto at = 54 + static_cast<std::size_t>(h / 2) * row + (w / 2) * 3;
        if (at + 2 >= bmp.size()) {
            expect(false, "held-frame BMP dimensions describe its pixels");
            return std::uint32_t{0};
        }
        return static_cast<std::uint32_t>(bmp[at + 2]) | (static_cast<std::uint32_t>(bmp[at + 1]) << 8) |
            (static_cast<std::uint32_t>(bmp[at]) << 16);
    };
    const std::array<std::uint8_t, 4> green{0, 255, 0, 255}, magenta{255, 0, 255, 255};
    renderer.hold_frame(true);
    renderer.hold_frame(true);
    renderer.upload_frame(0x04110000, green.data(), 1, 1, 1);
    expect(window_pixel(sandbox / "held-magenta.bmp") == 0xff00ff,
        "holding frame preserves original magenta window after guest framebuffer becomes green");
    std::vector<std::uint8_t> pixels;
    std::uint32_t w = 0, h = 0;
    expect(
        renderer.read_frame(pixels, w, h) && pixels.size() >= 4 && pixels[0] == 0 && pixels[1] == 255 && pixels[2] == 0,
        "held presentation does not prevent guest target update/readback");
    renderer.hold_frame(false);
    renderer.hold_frame(false);
    expect(window_pixel(sandbox / "released-green.bmp") == 0x00ff00,
        "releasing held frame displays the current guest framebuffer");
    renderer.upload_frame(0x04110000, magenta.data(), 1, 1, 1);
    expect(renderer.present(0x04110000), "held-frame contract restores original synthetic scene");
    int initial_w = 0, initial_h = 0;
    SDL_GetWindowSize(renderer.window(), &initial_w, &initial_h);
    renderer.set_window_scale(2);
    int large_w = 0, large_h = 0;
    SDL_GetWindowSize(renderer.window(), &large_w, &large_h);
    expect(large_w == 960 && large_h == 544, "window scale applies documented PSP logical dimensions");
    renderer.set_window_scale(1);
    int restored_w = 0, restored_h = 0;
    SDL_GetWindowSize(renderer.window(), &restored_w, &restored_h);
    expect(restored_w == initial_w && restored_h == initial_h, "window scale round trip restores initial logical size");
    renderer.set_fullscreen(false);
    expect((SDL_GetWindowFlags(renderer.window()) & SDL_WINDOW_FULLSCREEN) == 0,
        "windowed display setting preserves normal window state");
    renderer.set_present_mode(settings::PresentMode::Fifo);
    expect(renderer.display_refresh() >= 0 && renderer.draws_submitted() > 0 && !renderer.quit_requested(),
        "live renderer reports nonnegative refresh, actual draws and no quit request");
    renderer.pump_events();
}

class MediaFixture {
public:
    psprecomp::Runtime runtime{64u * 1024u * 1024u};
    MediaFixture() {
        auto &kernel = mhp3rd::kernel();
        kernel = mhp3rd::Kernel{};
        kernel.install(runtime, 0x08801000, 0x08820000);
        runtime.nids().load_csv(PSPRECOMP_TEST_NIDS_CSV);
        HleRegistrar hle(runtime);
        register_media(hle);
        kernel.start_loader_thread(runtime.cpu(), 0x08820000, 0);
    }
    ~MediaFixture() {
        mhp3rd::kernel() = mhp3rd::Kernel{};
        psprecomp::set_runtime_starvation_hook(nullptr, 0);
    }
    std::uint32_t call(
        const std::string &library, const std::string &name, std::initializer_list<std::uint32_t> values) {
        auto &cpu = runtime.cpu();
        for (unsigned i = 0; i < 8; ++i) cpu.set_gpr(i + 4, 0);
        unsigned i = 0;
        for (auto value : values) cpu.set_gpr(i++ + 4, value);
        cpu.set_gpr(31, 0x08822000);
        for (const auto &symbol : runtime.nids().all())
            if (symbol.library == library && symbol.name == name) {
                runtime.invoke_import(library, symbol.nid, cpu);
                expect(!runtime.stopped(), "renderer-backed media import keeps guest running");
                return cpu.gpr[2];
            }
        expect(false, "media import exists in public NID table");
        return 0xffffffff;
    }
};
void media_renderer_contracts(MediaFixture &fixture, gpu::VulkanRenderer &renderer) {
    auto &memory = fixture.runtime.memory();
    constexpr std::uint32_t frame = 0x08840000, list = 0x08810000, vertices = 0x08830000;
    expect(fixture.call("sceDisplay", "sceDisplaySetMode", {0, 480, 272}) == 0, "renderer-backed display mode import");
    for (std::uint32_t i = 0; i < 480 * 272; ++i) memory.store32(frame + i * 4, 0xff443322);
    expect(fixture.call("sceDisplay", "sceDisplaySetFrameBuf", {frame, 480, 3, 1}) == 0,
        "RAM movie frame HLE flip succeeds");
    std::vector<std::uint8_t> pixels;
    std::uint32_t width{}, height{};
    expect(renderer.read_frame(pixels, width, height), "HLE movie frame reaches renderer");
    expect(pixels.size() == 480 * 272 * 4 && pixels[0] == 0x22 && pixels[1] == 0x33 && pixels[2] == 0x44,
        "HLE movie upload preserves guest RGBA bytes");
    const auto command = [](std::uint32_t op, std::uint32_t data = 0) { return (op << 24) | (data & 0xffffff); };
    const std::vector<std::uint32_t> commands{command(0x10, 0x080000),
        command(0x12, (1u << 23) | (3u << 7) | (7u << 2)), command(1, vertices & 0xffffff), command(0x9c, 0),
        command(0x9d, 0x040200), command(0x9e, 0x088000), command(0x9f, 0x040200), command(0xd2, 3),
        command(0xd3, 0x701), command(4, (6u << 16) | 2), command(0x0f), command(0x0c)};
    for (std::size_t i = 0; i < commands.size(); ++i)
        memory.store32(list + static_cast<std::uint32_t>(i) * 4, commands[i]);
    for (std::uint32_t i = 0; i < 2; ++i) {
        memory.store32(vertices + i * 16, 0xff665544);
        memory.store32(vertices + i * 16 + 4, std::bit_cast<std::uint32_t>(i ? 480.f : 0.f));
        memory.store32(vertices + i * 16 + 8, std::bit_cast<std::uint32_t>(i ? 272.f : 0.f));
        memory.store32(vertices + i * 16 + 12, std::bit_cast<std::uint32_t>(i ? 65535.f : 0.f));
    }
    auto id = fixture.call("sceGe_user", "sceGeListEnQueue", {list, 0, 0xffffffff, 0});
    expect(id > 0 && id < 0xffff, "renderer-backed GE list enqueues");
    expect(fixture.call("sceGe_user", "sceGeListSync", {id, 0}) == 0, "GE list sink completes actual GPU commands");
    expect(fixture.call("sceDisplay", "sceDisplaySetFrameBuf", {0x04000000, 512, 3, 1}) == 0,
        "GE framebuffer HLE flip succeeds");
    expect(renderer.read_frame(pixels, width, height), "HLE GE framebuffer reads back");
    expect(pixels.size() == 480 * 272 * 4 && pixels[0] == 0x44 && pixels[1] == 0x55 && pixels[2] == 0x66,
        "HLE GE parsing and Vulkan submission retain vertex color");
}
void audio_device_contracts() {
    auto &sink = audio::AudioSink::instance();
    sink.initialize();
    expect(sink.has_device(), "SDL dummy audio device opens");
    sink.initialize();
    expect(sink.has_device(), "audio initialize is idempotent");
    sink.set_paused(true);
    sink.set_volume(-1);
    sink.set_volume(0.5f);
    sink.set_volume(2);
    const std::array<std::int16_t, 8> pcm{1000, -1000, 2000, -2000, 3000, -3000, 4000, -4000};
    std::uint64_t cursor = 0;
    sink.mix(cursor, pcm.data(), 4, 0x8000, 0x8000);
    const auto first = cursor;
    expect(first >= 4, "dummy playback producer advances cursor");
    sink.mix(cursor, pcm.data(), 4, 0x4000, 0x8000);
    expect(cursor == first + 4, "paused dummy device retains producer timeline");
    sink.set_paused(false);
    SDL_Delay(50);
    expect(sink.has_device(), "resuming dummy playback keeps device open");
    sink.shutdown();
    sink.shutdown();
    expect(!sink.has_device(), "audio shutdown idempotently closes playback");
}
void input_capture_contracts(gpu::VulkanRenderer &renderer) {
    auto &layer = ui::Layer::get();
    layer.set_interactive(true);
    renderer.pump_events();
    auto key = [&](SDL_Scancode scan, bool down) {
        SDL_Event event{};
        event.type = down ? SDL_EVENT_KEY_DOWN : SDL_EVENT_KEY_UP;
        event.key.windowID = SDL_GetWindowID(renderer.window());
        event.key.scancode = scan;
        event.key.key = SDL_GetKeyFromScancode(scan, SDL_KMOD_NONE, false);
        event.key.down = down;
        expect(SDL_PushEvent(&event), "synthetic SDL keyboard event queued");
        renderer.pump_events();
    };
    layer.begin_binding_capture();
    expect(layer.capturing_binding(), "binding capture starts");
    key(SDL_SCANCODE_LCTRL, true);
    key(SDL_SCANCODE_A, true);
    key(SDL_SCANCODE_A, false);
    expect(layer.capturing_binding(), "capture waits for last held input");
    key(SDL_SCANCODE_LCTRL, false);
    auto captured = layer.take_captured_binding();
    expect(captured && captured->inputs[0] == input::key(SDL_SCANCODE_LCTRL) &&
            captured->inputs[1] == input::key(SDL_SCANCODE_A),
        "capture returns complete chord in pressed order");
    expect(!layer.capturing_binding() && !layer.take_captured_binding(), "completed capture consumed exactly once");
    layer.begin_binding_capture();
    key(SDL_SCANCODE_ESCAPE, true);
    key(SDL_SCANCODE_ESCAPE, false);
    captured = layer.take_captured_binding();
    expect(captured && captured->inputs[0] == input::kNone, "escape cancels key capture");
    layer.begin_binding_capture(ui::Layer::Capture::Pad);
    expect(
        layer.capture_seconds_left() <= 6 && layer.capture_seconds_left() > 0 && layer.capture_cancel_progress() == 0,
        "new pad capture countdown and hold progress");
    key(SDL_SCANCODE_ESCAPE, true);
    key(SDL_SCANCODE_ESCAPE, false);
    captured = layer.take_captured_binding();
    expect(captured && captured->inputs[0] == 0, "escape cancels pad capture");
    layer.set_interactive(false);
}
void widget_and_browser_contracts(gpu::VulkanRenderer &renderer, const std::filesystem::path &sandbox) {
    auto &layer = ui::Layer::get();
    layer.set_interactive(true);
    int selected = 0, value = 5;
    bool activated = false;
    ImVec2 click{};
    auto frame = [&](bool disabled) {
        layer.begin_frame();
        ui::begin_panel("##contract", "Public widget contracts", "", true);
        const char *labels[] = {"One", "Two", "Three"};
        ui::tab_bar(labels, 3, selected);
        ui::begin_content();
        activated = ui::button_row("Activate", {disabled, "", "Contract action"});
        const auto min = ImGui::GetItemRectMin(), max = ImGui::GetItemRectMax();
        click = {(min.x + max.x) * 0.5f, (min.y + max.y) * 0.5f};
        ui::slider_row("Bounded", value, 0, 10, 1, "%d", {true});
        ui::begin_footer();
        ui::hints({{ui::Control::Confirm, "Choose"}});
        ui::end_panel();
        layer.end_frame();
        renderer.present_ui(false);
    };
    frame(false);
    frame(false);
    ImGui::GetIO().AddMousePosEvent(click.x, click.y);
    ImGui::GetIO().AddMouseButtonEvent(0, true);
    frame(false);
    ImGui::GetIO().AddMouseButtonEvent(0, false);
    frame(false);
    expect(activated, "button activates on completed pointer click");
    ImGui::GetIO().AddMouseButtonEvent(0, true);
    frame(true);
    ImGui::GetIO().AddMouseButtonEvent(0, false);
    frame(true);
    expect(!activated && value == 5, "disabled action and slider ignore input");
    ImGui::GetIO().AddKeyEvent(ImGuiKey_W, true);
    frame(false);
    expect(selected == 1, "tabs advance with W");
    ImGui::GetIO().AddKeyEvent(ImGuiKey_W, false);
    frame(false);
    ImGui::GetIO().AddKeyEvent(ImGuiKey_Q, true);
    frame(false);
    expect(selected == 0, "tabs retreat with Q");
    ImGui::GetIO().AddKeyEvent(ImGuiKey_Q, false);
    frame(false);
    const auto folder = sandbox / "browser";
    std::filesystem::create_directories(folder / "Child");
    std::ofstream(folder / "visible.ISO") << "synthetic disc label";
    std::ofstream(folder / "hidden.txt") << "other";
    std::ofstream(folder / ".dot.iso") << "hidden";
    ui::FileBrowser browser(folder);
    expect(browser.folder() == folder, "browser starts in specified directory");
    auto browse = [&](bool back) {
        layer.begin_frame();
        ui::begin_panel("##browser", "Public file browser", "", false);
        ui::begin_content();
        auto result = browser.frame(back);
        ui::begin_footer();
        ui::hints({{ui::Control::Confirm, "Choose"}});
        ui::end_panel();
        layer.end_frame();
        renderer.present_ui(false);
        return result;
    };
    expect(browse(false) == ui::FileBrowser::Result::Browsing, "browser waits for selection");
    expect(browse(true) == ui::FileBrowser::Result::Browsing && browser.folder() == sandbox,
        "browser back navigates to parent rather than cancels");
    ui::FileBrowser root(folder.root_path());
    layer.begin_frame();
    ui::begin_panel("##root", "Root", "", false);
    ui::begin_content();
    expect(root.frame(true) == ui::FileBrowser::Result::Cancelled, "browser back at filesystem root cancels");
    ui::begin_footer();
    ui::hints({{ui::Control::Confirm, "Choose"}});
    ui::end_panel();
    layer.end_frame();
    renderer.present_ui(false);
    expect(ui::human_size(0) == "0 bytes" && ui::human_size(1000) == "1 KB" && ui::human_size(1000000) == "1 MB",
        "file sizes use displayed decimal units at boundaries");
    layer.set_interactive(false);
}
void file_browser_boundary_contracts(gpu::VulkanRenderer &renderer, const std::filesystem::path &sandbox) {
    const auto base = sandbox / "browser-boundaries";
    std::filesystem::create_directories(base / "Child");
    std::ofstream(base / "visible.ISO") << "PUBLIC";
    std::ofstream(base / "other.txt") << "OTHER";
    auto &layer = ui::Layer::get();
    layer.set_interactive(true);
    auto frame = [&](ui::FileBrowser &browser, const char *focus = nullptr, int index = -1, bool back = false) {
        layer.begin_frame();
        ui::begin_panel("##browser-cases", "Public browser boundaries", "", false);
        ui::begin_content();
        if (focus) {
            ImGuiWindow *target = ImGui::GetCurrentWindow();
            if (index >= 0 || std::string_view(focus) == "##parent" || std::string_view(focus) == "##choose") {
                for (auto *window : ImGui::GetCurrentContext()->Windows)
                    if (std::string_view(window->Name).find("##browser-cases") != std::string_view::npos &&
                        std::string_view(window->Name).find("/entries") != std::string_view::npos)
                        target = window;
            }
            const auto id = index < 0 ? target->GetID(focus) : ImHashStr(focus, 0, target->GetID(index));
            ImGui::FocusWindow(target);
            ImGui::SetFocusID(id, target);
            ImGui::SetNavCursorVisible(true);
        }
        const auto result = browser.frame(back);
        ui::begin_footer();
        ui::hints({{ui::Control::Confirm, "Choose"}});
        ui::end_panel();
        layer.end_frame();
        renderer.present_ui(false);
        return result;
    };
    auto pick = [&](ui::FileBrowser &browser, const char *id, int index = -1) {
        frame(browser, id, index);
        frame(browser, id, index);
        ImGui::GetIO().AddKeyEvent(ImGuiKey_Space, true);
        const auto pressed = frame(browser, id, index);
        ImGui::GetIO().AddKeyEvent(ImGuiKey_Space, false);
        const auto released = frame(browser);
        return pressed == ui::FileBrowser::Result::Chosen ? pressed : released;
    };
    ui::FileBrowser file(base);
    frame(file);
    expect(pick(file, "##entry", 1) == ui::FileBrowser::Result::Chosen && file.chosen() == base / "visible.ISO",
        "browser case-insensitive ISO filter selects exact public file through actual row");
    ui::FileBrowser::Options folders;
    folders.choose_folder = "Choose this public directory";
    folders.choose_on_open = [](const auto &path) { return path.filename() == "Child"; };
    ui::FileBrowser directory(base, folders);
    frame(directory);
    expect(pick(directory, "##entry", 0) == ui::FileBrowser::Result::Chosen && directory.chosen() == base / "Child" &&
            directory.folder() == base,
        "choosable directory returns its path without entering it");
    ui::FileBrowser choose_here(base, folders);
    frame(choose_here);
    expect(pick(choose_here, "##choose") == ui::FileBrowser::Result::Chosen && choose_here.chosen() == base,
        "choose-current-folder row returns exact current directory");
    ui::FileBrowser removed(base);
    frame(removed);
    std::filesystem::remove(base / "Child");
    pick(removed, "##entry", 0);
    expect(removed.folder() == base / "Child" && removed.chosen().empty(),
        "directory removed after listing reports navigation failure without choosing nonexistent data");
    expect(frame(removed) == ui::FileBrowser::Result::Browsing &&
            frame(removed, nullptr, -1, true) == ui::FileBrowser::Result::Browsing && removed.folder() == base,
        "browser error state remains usable and Back recovers to existing parent");
    expect(ui::human_size(1'000'000'000) == "1.0 GB", "file size GB threshold uses documented display unit");
    layer.set_interactive(false);
}

template <class Fn> auto with_escape(gpu::VulkanRenderer &renderer, Fn work) {
    const auto window = SDL_GetWindowID(renderer.window());
    std::jthread input([window] {
        SDL_Delay(180);
        SDL_Event event{};
        event.type = SDL_EVENT_KEY_DOWN;
        event.key.windowID = window;
        event.key.key = SDLK_ESCAPE;
        event.key.scancode = SDL_SCANCODE_ESCAPE;
        event.key.down = true;
        SDL_PushEvent(&event);
        SDL_Delay(120);
        event.type = SDL_EVENT_KEY_UP;
        event.key.down = false;
        SDL_PushEvent(&event);
    });
    return work();
}

void save_screen_contracts(gpu::VulkanRenderer &renderer, const std::filesystem::path &sandbox) {
    namespace sd = savedata;
    const auto previous = sd::memory_stick();
    const auto target = sandbox / "PublicMemoryStick";
    const auto source = sandbox / "PublicSaveSource";
    sd::set_memory_stick(target);
    sd::Block key{};
    for (std::size_t i = 0; i < key.size(); ++i) key[i] = static_cast<std::uint8_t>(i + 1);
    sd::remember_game_key("ULJM05800", key);
    sd::SaveFiles files{"ULJM05800", "", "MHP3RD.BIN", key};
    sd::SaveContents content;
    content.data.assign(2048, 0x5a);
    content.title = "Public synthetic save";
    std::string error;
    expect(sd::write_save(source, files, content, error), "synthetic encrypted public save writes");
    auto &layer = ui::Layer::get();
    layer.set_interactive(true);
    auto frame = [&](const char *focused = nullptr, bool back = false) {
        layer.begin_frame();
        ui::begin_panel("##save-contract", "Public saves", "", false);
        ui::begin_content();
        if (focused) {
            auto *window = ImGui::GetCurrentWindow();
            const auto focus = window->GetID(focused);
            ImGui::FocusWindow(window);
            ImGui::SetFocusID(focus, window);
            ImGui::SetNavCursorVisible(true);
        }
        if (ui::save_screen_open())
            ui::save_screen(back);
        else
            ui::save_rows();
        ui::begin_footer();
        ui::hints({{ui::Control::Confirm, "Choose"}, {ui::Control::Back, "Back"}});
        ui::end_panel();
        layer.end_frame();
        renderer.present_ui(false);
    };
    auto activate = [&](const char *label) {
        frame(label);
        frame(label);
        ImGui::GetIO().AddKeyEvent(ImGuiKey_Space, true);
        frame(label);
        ImGui::GetIO().AddKeyEvent(ImGuiKey_Space, false);
        frame();
    };
    auto drop = [&](const std::filesystem::path &path) {
        const std::string chosen = path.string();
        SDL_Event event{};
        event.type = SDL_EVENT_DROP_FILE;
        event.drop.windowID = SDL_GetWindowID(renderer.window());
        event.drop.data = chosen.c_str();
        expect(SDL_PushEvent(&event), "public save folder drop queues");
        renderer.pump_events();
        frame();
        frame();
    };
    frame();
    activate("Import save…");
    expect(ui::save_screen_open(), "save import opens folder browser");
    drop(source);
    activate("Import this save");
    const auto imported = sd::load_save(target, files);
    expect(imported.status == sd::LoadStatus::Ok && imported.contents.data == content.data,
        "real UI import preserves exact decrypted synthetic payload");
    activate("Later");
    expect(!ui::save_screen_open() && !ui::take_restart_request(), "Later closes without restart request");
    content.data.assign(2048, 0xa5);
    expect(sd::write_save(source, files, content, error), "replacement public save writes");
    activate("Import save…");
    drop(source);
    activate("Back up now");
    activate("Replace and import");
    expect(sd::load_save(target, files).contents.data == content.data,
        "replacement imports second exact synthetic payload");
    bool backup{};
    for (const auto &entry : std::filesystem::recursive_directory_iterator(sd::savedata_root(target) / ".backup")) {
        if (entry.path().filename() == "MHP3RD.BIN") backup = true;
    }
    expect(backup, "replacement keeps prior encrypted public save in backup");
    activate("Restart now");
    expect(ui::take_restart_request() && !ui::take_restart_request(), "restart request is delivered exactly once");
    const auto destination = sandbox / "PublicSaveExport";
    std::filesystem::create_directories(destination);
    activate("Export save…");
    drop(destination);
    bool exported{};
    for (const auto &entry : std::filesystem::recursive_directory_iterator(destination)) {
        if (entry.path().filename() == "MHP3RD.BIN") exported = true;
    }
    expect(exported, "real export UI creates portable encrypted save");
    frame(nullptr, true);
    expect(!ui::save_screen_open(), "Back closes save export result");
    activate("Back up saves…");
    activate("Back up to the backups folder");
    activate("Done");
    expect(!ui::save_screen_open(), "backup UI completes and returns to menu");
    const bool timestamp = settings::current().backup_timestamp;
    settings::current().backup_timestamp = false;
    const auto backup_target = sandbox / "PublicExplicitBackup";
    std::filesystem::create_directories(backup_target);
    activate("Back up saves…");
    activate("Back up to another folder…");
    drop(backup_target);
    activate("Done");
    expect(std::filesystem::is_regular_file(backup_target / "ULJM05800" / "MHP3RD.BIN"),
        "explicit backup contains encrypted save without timestamp");
    activate("Back up saves…");
    activate("Back up to another folder…");
    drop(backup_target);
    activate("Cancel");
    expect(ui::save_screen_open(), "conflicting backup cancellation returns to backup options");
    activate("Back up to another folder…");
    drop(backup_target);
    activate("Replace the backup");
    activate("Done");
    expect(!ui::save_screen_open(), "confirmed conflicting backup replacement completes");
    settings::current().backup_timestamp = timestamp;

    const auto invalid = sandbox / "PublicInvalidSave";
    std::filesystem::create_directories(invalid);
    std::ofstream(invalid / "PARAM.SFO") << "public invalid metadata";
    activate("Import save…");
    drop(invalid);
    activate("Cancel");
    expect(!ui::save_screen_open() && sd::load_save(target, files).contents.data == content.data,
        "invalid save review cancellation preserves exact current payload");
    ui::request_backup_reminder("Public deterministic reminder contract");
    expect(ui::backup_reminder_due(), "explicit reminder becomes due after presented frames");
    expect(with_escape(renderer, [&] { return ui::run_backup_reminder(); }),
        "bounded Escape closes actual backup reminder without closing SDL window");
    expect(!ui::backup_reminder_due(), "closed requested reminder is consumed once");
    sd::set_memory_stick(previous);
    layer.set_interactive(false);
}

void texture_pack_screen_contracts(gpu::VulkanRenderer &renderer, const std::filesystem::path &sandbox) {
    const auto pack = sandbox / "PublicTextures";
    std::filesystem::create_directories(pack);
    std::ofstream(pack / "textures.ini")
        << "[games]\nNPJB40001 = true\n[options]\nhash = xxh64\n[hashes]\n000000000000000000000001 = red.png\n";
    // Independently generated PNG chunks for a single public red RGBA pixel.
    const std::array<std::uint8_t, 70> png{0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d, 0x49,
        0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x08, 0x06, 0x00, 0x00, 0x00, 0x1f, 0x15,
        0xc4, 0x89, 0x00, 0x00, 0x00, 0x0d, 0x49, 0x44, 0x41, 0x54, 0x78, 0x9c, 0x63, 0xf8, 0xcf, 0xc0, 0xf0, 0x1f,
        0x00, 0x05, 0x00, 0x01, 0xff, 0x89, 0x99, 0x3d, 0x1d, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae,
        0x42, 0x60, 0x82};
    {
        std::ofstream out(pack / "red.png", std::ios::binary);
        out.write(reinterpret_cast<const char *>(png.data()), png.size());
    }
    auto &layer = ui::Layer::get();
    layer.set_interactive(true);
    auto frame = [&](bool back = false, const char *focused_row = nullptr) {
        layer.begin_frame();
        ui::begin_panel("##texture-import-contract", "Public texture import", "", false);
        ui::begin_content();
        ImGuiID focus{};
        if (focused_row) {
            auto *window = ImGui::GetCurrentWindow();
            focus = window->GetID(focused_row);
            ImGui::FocusWindow(window);
            ImGui::SetFocusID(focus, window);
            ImGui::SetNavCursorVisible(true);
        }
        ui::texture_pack_import_tick();
        if (ui::texture_pack_screen_open())
            ui::texture_pack_screen(back);
        else
            ui::texture_pack_rows();
        const bool found = focus && GImGui->NavId == focus && GImGui->NavIdIsAlive;
        ui::begin_footer();
        ui::hints({{ui::Control::Confirm, "Choose"}, {ui::Control::Back, "Back"}});
        ui::end_panel();
        layer.end_frame();
        renderer.present_ui(false);
        return found;
    };
    auto press = [&](ImGuiKey key) {
        ImGui::GetIO().AddKeyEvent(key, true);
        frame();
        ImGui::GetIO().AddKeyEvent(key, false);
        frame();
    };
    frame();
    frame(false, "Import texture pack…");
    press(ImGuiKey_Space);
    expect(ui::texture_pack_screen_open(), "texture import row opens actual folder browser");
    const std::string path = pack.string();
    SDL_Event drop{};
    drop.type = SDL_EVENT_DROP_FILE;
    drop.drop.windowID = SDL_GetWindowID(renderer.window());
    drop.drop.data = path.c_str();
    expect(SDL_PushEvent(&drop), "public texture pack drop queues");
    renderer.pump_events();
    frame();
    bool review{};
    for (int settle = 0; settle < 100 && !review; ++settle) {
        SDL_Delay(5);
        review = frame(false, "Copy into Yakumo's data folder");
    }
    expect(review, "asynchronous texture check reaches valid-copy review");
    press(ImGuiKey_Space);
    for (int settle = 0; settle < 100 && ui::texture_pack_import_busy(); ++settle) {
        SDL_Delay(5);
        frame();
    }
    frame();
    expect(!ui::texture_pack_import_busy() &&
            std::filesystem::is_regular_file(sandbox / "textures" / "NPJB40001" / "red.png"),
        "confirmed texture import installs the public PNG through real copy worker");
    expect(settings::current().texture_pack && settings::current().texture_pack_folder.empty(),
        "successful copy enables installed texture pack");
    frame(true);
    expect(!ui::texture_pack_screen_open(), "back closes texture import result");
    auto choose = [&](const std::filesystem::path &source, const char *action) {
        frame(false, "Import texture pack…");
        press(ImGuiKey_Space);
        const std::string chosen = source.string();
        drop.drop.data = chosen.c_str();
        expect(SDL_PushEvent(&drop), "additional texture source drop queues");
        renderer.pump_events();
        frame();
        bool ready{};
        for (int settle = 0; settle < 100 && !ready; ++settle) {
            SDL_Delay(5);
            ready = frame(false, action);
        }
        expect(ready, "texture review offers the requested public action");
        frame(false, action); // Apply explicit focus after review's default-focus request.
    };
    choose(pack, "Use it where it is");
    press(ImGuiKey_Space);
    frame();
    expect(settings::current().texture_pack_folder == pack.string(),
        "in-place import selects source without changing installed copy");
    expect(std::filesystem::is_regular_file(sandbox / "textures" / "NPJB40001" / "red.png"),
        "in-place import preserves installed PNG");
    frame(true);

    choose(pack, "Copy and replace");
    press(ImGuiKey_Space);
    for (int settle = 0; settle < 100 && ui::texture_pack_import_busy(); ++settle) {
        SDL_Delay(5);
        frame();
    }
    frame();
    expect(!ui::texture_pack_import_busy() && settings::current().texture_pack_folder.empty(),
        "replacement restores installed source after copying");
    bool backed_up{};
    for (const auto &entry : std::filesystem::recursive_directory_iterator(sandbox / "textures" / ".backup")) {
        if (entry.path().filename() == "red.png") backed_up = true;
    }
    expect(backed_up, "replacement retains old public PNG in backup");
    frame(true);

    const auto invalid = sandbox / "InvalidTextures";
    std::filesystem::create_directories(invalid);
    std::ofstream(invalid / "textures.ini") << "[options]\nhash = unsupported\n";
    choose(invalid, "Choose another folder");
    press(ImGuiKey_Space);
    expect(ui::texture_pack_screen_open() && !ui::texture_pack_import_busy(),
        "rejected pack returns to browser without starting copy");
    // Back ascends folders first; finish with the review's explicit Cancel row.
    const std::string rejected = invalid.string();
    drop.drop.data = rejected.c_str();
    expect(SDL_PushEvent(&drop), "rejected pack can be selected again");
    renderer.pump_events();
    frame();
    bool can_cancel{};
    for (int settle = 0; settle < 100 && !can_cancel; ++settle) {
        SDL_Delay(5);
        can_cancel = frame(false, "Cancel");
    }
    expect(can_cancel, "invalid texture review provides Cancel");
    frame(false, "Cancel");
    press(ImGuiKey_Space);
    expect(!ui::texture_pack_screen_open(), "cancel closes rejected texture review");
    expect(settings::current().texture_pack_folder.empty() && settings::current().texture_pack,
        "invalid import leaves successful installed selection intact");
    settings::current().texture_pack = false;
    renderer.set_texture_pack(false);
    layer.set_interactive(false);
}

void mods_screen_contracts(
    gpu::VulkanRenderer &renderer, const std::filesystem::path &sandbox, psprecomp::Runtime &runtime) {
    // Construct a tiny ISO9660 image and archive entirely from public bytes.
    // It contains one eight-byte entry and no game executable or assets.
    constexpr std::size_t block = 2048;
    std::vector<std::uint8_t> image(25 * block);
    auto store = [&](std::size_t offset, std::uint32_t value) {
        for (int byte = 0; byte < 4; ++byte) image[offset + byte] = value >> (byte * 8);
    };
    auto record = [&](std::size_t offset, const std::string &name, std::uint32_t lba, std::uint32_t size,
                      bool directory) {
        image[offset] = static_cast<std::uint8_t>((33 + name.size() + 1) & ~1u);
        store(offset + 2, lba);
        store(offset + 10, size);
        image[offset + 25] = directory ? 2 : 0;
        image[offset + 32] = static_cast<std::uint8_t>(name.size());
        std::copy(name.begin(), name.end(), image.begin() + offset + 33);
    };
    image[16 * block] = 1;
    std::copy_n("CD001", 5, image.begin() + 16 * block + 1);
    image[16 * block + 6] = 1;
    record(16 * block + 156, std::string(1, '\0'), 20, block, true);
    record(20 * block, "PSP_GAME", 21, block, true);
    record(21 * block, "USRDIR", 22, block, true);
    record(22 * block, "DATA.BIN;1", 23, block * 2, false);
    mods::p3rd::Directory directory;
    directory.directory_blocks = 1;
    directory.blocks = {1, 2};
    directory.sizes = {{0, 8}};
    directory.trailer.resize(block - 16);
    mods::p3rd::encrypt(directory.trailer, 0, 16);
    const auto header = directory.encode();
    expect(header.size() == block, "synthetic archive directory fills exactly one block");
    std::copy(header.begin(), header.end(), image.begin() + 23 * block);
    std::vector<std::uint8_t> entry(block);
    std::copy_n("PUBLIC!!", 8, entry.begin());
    mods::p3rd::encrypt(entry, 1, 0);
    std::copy(entry.begin(), entry.end(), image.begin() + 24 * block);
    const auto iso_path = sandbox / "public-mod-fixture.iso";
    {
        std::ofstream out(iso_path, std::ios::binary);
        out.write(reinterpret_cast<const char *>(image.data()), image.size());
    }
    const auto folder = sandbox / "mods" / "Public";
    std::filesystem::create_directories(folder);
    std::ofstream(folder / "mod.ini")
        << "[MOD INFO]\nName=Public contract\nAuthor=Test fixture\nDescription=Synthetic eight-byte replacement\nType=File\nVersion=HD\nFiles=replacement.bin\nTarget=0000\n";
    std::ofstream(folder / "replacement.bin", std::ios::binary) << "CHANGED!";
    IsoImage disc(iso_path);
    mods::attach_disc(&disc);
    auto *session = mods::session();
    expect(session && session->library().mods().size() == 1, "public synthetic archive creates a real mod session");
    if (!session || session->library().mods().empty()) {
        mods::attach_disc(nullptr);
        return;
    }
    const auto id = session->library().mods().front().id;
    expect(!session->library().enabled(id), "new public mod starts disabled");
    auto &layer = ui::Layer::get();
    layer.set_interactive(true);
    ImVec2 click{};
    auto frame = [&](bool back = false, bool scroll = false, const char *focused_row = nullptr) {
        layer.begin_frame();
        ui::begin_panel("##mods-contract", "Mod contract", "", false);
        ui::begin_content();
        if (focused_row) {
            auto *window = ImGui::GetCurrentWindow();
            ImGui::FocusWindow(window);
            ImGui::SetFocusID(window->GetID(focused_row), window);
            ImGui::SetNavCursorVisible(true);
        }
        ui::mods_page(back);
        const auto min = ImGui::GetItemRectMin(), max = ImGui::GetItemRectMax();
        click = {(min.x + max.x) * .5f, (min.y + max.y) * .5f};
        if (scroll) ImGui::SetScrollHereY(1);
        ui::begin_footer();
        ui::hints({{ui::Control::Confirm, "Choose"}, {ui::Control::Back, "Back"}});
        ui::end_panel();
        if (ui::text_input_open()) ui::text_input_frame();
        layer.end_frame();
        renderer.present_ui(false);
    };
    frame(false, true);
    frame(false, true);
    frame();
    ImGui::GetIO().AddMousePosEvent(click.x, click.y);
    ImGui::GetIO().AddMouseButtonEvent(0, true);
    frame();
    ImGui::GetIO().AddMouseButtonEvent(0, false);
    frame();
    expect(ui::mods_screen_open(), "selecting installed mod opens real details screen");
    frame();
    frame();
    ImGui::GetIO().AddKeyEvent(ImGuiKey_RightArrow, true);
    frame();
    ImGui::GetIO().AddKeyEvent(ImGuiKey_RightArrow, false);
    frame();
    expect(session->library().enabled(id) && mods::serving(),
        "details toggle activates public replacement through real session");
    std::array<std::uint8_t, 8> bytes{};
    expect(mods::read_data_bin(block, bytes) == bytes.size(), "active UI mod serves replacement bytes");
    mods::p3rd::decrypt(bytes, 1, 0);
    expect(std::string(bytes.begin(), bytes.end()) == "CHANGED!", "replacement selected in UI reaches archive reads");
    ImGui::GetIO().AddKeyEvent(ImGuiKey_LeftArrow, true);
    frame();
    ImGui::GetIO().AddKeyEvent(ImGuiKey_LeftArrow, false);
    frame();
    expect(!session->library().enabled(id) && !mods::serving(), "details toggle restores original archive");
    frame(true);
    expect(!ui::mods_screen_open() && !ui::take_mods_restart_request(),
        "back closes mod details without requesting restart");
    const auto incoming = sandbox / "Incoming";
    std::filesystem::create_directories(incoming);
    std::ofstream(incoming / "mod.ini")
        << "[MOD INFO]\nName=Imported public contract\nType=File\nVersion=HD\nFiles=replacement.bin\nTarget=0000\n";
    std::ofstream(incoming / "replacement.bin", std::ios::binary) << "IMPORT!!";
    auto press = [&](ImGuiKey key) {
        ImGui::GetIO().AddKeyEvent(key, true);
        frame();
        ImGui::GetIO().AddKeyEvent(key, false);
        frame();
    };
    frame();
    frame();
    frame(false, false, "Import mod…");
    press(ImGuiKey_Space);
    for (int settle = 0; settle < 4 && !ui::mods_screen_open(); ++settle) frame();
    expect(ui::mods_screen_open(), "import row opens mod folder browser");
    const std::string dropped_path = incoming.string();
    SDL_Event drop{};
    drop.type = SDL_EVENT_DROP_FILE;
    drop.drop.windowID = SDL_GetWindowID(renderer.window());
    drop.drop.data = dropped_path.c_str();
    expect(SDL_PushEvent(&drop), "synthetic mod folder drop queues");
    renderer.pump_events();
    frame();
    frame();
    frame();
    frame(false, false, "Import this mod");
    press(ImGuiKey_Space);
    for (int settle = 0; settle < 5 && session->library().mods().size() != 2; ++settle) frame();
    frame();
    expect(session->library().mods().size() == 2 &&
            std::filesystem::is_regular_file(sandbox / "mods" / "Incoming" / "replacement.bin"),
        "review confirmation imports public mod and refreshes real library");
    expect(std::all_of(session->library().mods().begin(), session->library().mods().end(),
               [&](const auto &mod) { return !session->library().enabled(mod.id); }),
        "imported mods stay disabled");
    frame(true);
    expect(!ui::mods_screen_open(), "back closes import result");
    frame(false, false, "Import mod…");
    frame(false, false, "Import mod…");
    press(ImGuiKey_Space);
    std::ofstream(incoming / "replacement.bin", std::ios::binary) << "SECOND!!";
    expect(SDL_PushEvent(&drop), "replacement mod drop queues");
    renderer.pump_events();
    frame();
    frame();
    frame(false, false, "Import this mod");
    frame(false, false, "Import this mod");
    press(ImGuiKey_Space);
    frame();
    bool mod_backup{};
    for (const auto &backup : std::filesystem::recursive_directory_iterator(sandbox / "mods" / ".backup")) {
        if (backup.path().filename() == "replacement.bin") mod_backup = true;
    }
    expect(mod_backup && session->library().mods().size() == 2,
        "reimport replaces existing mod while preserving old public bytes in backup");
    frame(true);
    for (const auto &mod : session->library().mods()) session->library().set_enabled(mod.id, true);
    session->commit();
    frame();
    expect(!session->wanted().conflicts.empty(), "two enabled public file mods expose actual same-file conflict");
    frame(false, false, "Use mods");
    press(ImGuiKey_Space);
    expect(!session->library().master() && !mods::serving(),
        "master off restores unmodified archive despite enabled mods");
    frame(false, false, "Use mods");
    press(ImGuiKey_Space);
    expect(session->library().master(), "master toggle restores enabled mod policy");
    for (const auto &mod : session->library().mods()) session->library().set_enabled(mod.id, false);
    session->commit();
    const auto gear_folder = sandbox / "mods" / "ZEquipment";
    std::filesystem::create_directories(gear_folder);
    std::ofstream(gear_folder / "mod.ini") << "[MOD INFO]\nName=Public equipment\nType=EquipHEAD\nFiles=helmet.bin\n";
    std::ofstream(gear_folder / "helmet.bin", std::ios::binary) << "PUBLIC!!";
    session->rescan();
    const auto gear = std::find_if(session->library().mods().begin(), session->library().mods().end(),
        [](const auto &mod) { return mod.name == "Public equipment"; });
    expect(gear != session->library().mods().end() && gear->slots.size() == 1,
        "public equipment mod exposes one real armor slot");
    if (gear != session->library().mods().end()) {
        const auto gear_id = gear->id;
        for (int move = 0; move < 3; ++move) session->library().move(gear_id, -1);
        session->commit();
        frame(false, true);
        frame(false, true);
        frame();
        ImGui::GetIO().AddMousePosEvent(click.x, click.y);
        ImGui::GetIO().AddMouseButtonEvent(0, true);
        frame();
        ImGui::GetIO().AddMouseButtonEvent(0, false);
        frame();
        expect(ui::mods_screen_open(), "equipment row opens details");
        frame(false, false, "Replaces (Head armour)");
        frame(false, false, "Replaces (Head armour)");
        press(ImGuiKey_Space);
        expect(ui::text_input_open(), "equipment replacement opens filtered hex editor");
        ImGui::GetIO().AddInputCharactersUTF8("0000");
        press(ImGuiKey_Enter);
        frame();
        const auto chosen = session->library().choice(gear_id);
        expect(chosen.slots.size() == 1 && chosen.slots[0] == mods::FileId{0},
            "equipment hex editor stores exact public file id");
        frame(false, false, "Use my current armor");
        press(ImGuiKey_Space);
        expect(session->library().choice(gear_id).slots[0] == mods::FileId{0},
            "absent game hunter leaves configured armor slot unchanged");
        frame(false, false, "No armor");
        press(ImGuiKey_Space);
        auto &ram = runtime.memory();
        ram.store16(game::kCharacter, 0xff34); // Public one-letter hunter.
        ram.store8(game::kCharacter + game::kCharacterSex, 0);
        ram.store8(game::kCharacter + game::kCharacterInnerWear, 0);
        const auto worn = game::kCharacter + game::kCharacterArmor + 4 * game::kEquipmentRecord;
        ram.store8(worn, 1);
        ram.store8(worn + 1, 4);
        ram.store16(worn + 2, 1);
        ram.store16(game::kArmorFileBase + 8, 10);
        ram.store16(game::kArmorFileBase + 10, 20);
        ram.store16(game::kHeadData + game::kArmorRecord, 3);
        ram.store8(game::kHeadData + game::kArmorRecord + 4, 0x0f);
        frame(false, false, "Use my current armor");
        press(ImGuiKey_Space);
        expect(session->library().choice(gear_id).slots[0] == mods::FileId{13} && session->library().enabled(gear_id),
            "loaded synthetic hunter selects exact worn head model and enables mod");
        frame(false, false, "No armor");
        press(ImGuiKey_Space);
        expect(session->library().choice(gear_id).slots[0] == mods::FileId{10},
            "No armor selects synthetic bare head base model");
        ram.store16(game::kCharacter, 0);
        frame(true);
        expect(!ui::mods_screen_open(), "Back closes equipment details");
    }
    layer.set_interactive(false);
    mods::attach_disc(nullptr);
}

void camera_probe_contracts(gpu::VulkanRenderer &renderer, const std::filesystem::path &sandbox) {
    psprecomp::Runtime runtime(32u * 1024u * 1024u);
    auto &memory = runtime.memory();
    gpu::DrawCall scene{};
    scene.primitive = gpu::PrimitiveType::Triangles;
    scene.has_vertex_color = true;
    scene.target.color_address = 0x04000000;
    scene.target.color_stride = 512;
    scene.target.color_format = 3;
    scene.viewport.x_scale = 240;
    scene.viewport.y_scale = -136;
    scene.viewport.x_offset = 240;
    scene.viewport.y_offset = 136;
    for (auto matrix : {&scene.world, &scene.view, &scene.projection, &scene.texture_matrix}) {
        matrix->fill(0);
        (*matrix)[0] = (*matrix)[5] = (*matrix)[10] = (*matrix)[15] = 1;
    }
    for (auto position : {std::array<float, 4>{-.2f, -.2f, 0, 1}, {.2f, -.2f, 0, 1}, {0, .2f, 0, 1}}) {
        gpu::Vertex vertex{};
        vertex.position = position;
        vertex.color = 0xffffffff;
        scene.vertices.push_back(vertex);
    }
    std::ostringstream detector_trace;
    struct RestoreOutput {
        std::streambuf *previous;
        ~RestoreOutput() { std::cout.rdbuf(previous); }
    } restore{std::cout.rdbuf(detector_trace.rdbuf())};
    float previous{};
    const std::array<float, 9> yaws{0, 5, 10, 18, 26, 34, 42, 50, 58};
    for (std::size_t frame = 0; frame < yaws.size(); ++frame) {
        const float yaw = yaws[frame];
        const float radians = yaw * 0.017453292519943295f;
        scene.view[0] = scene.view[10] = std::cos(radians);
        scene.view[2] = std::sin(radians);
        scene.view[8] = -std::sin(radians);
        memory.store32(0x08000020, std::bit_cast<std::uint32_t>(yaw));
        memory.store32(0x08000024, std::bit_cast<std::uint32_t>(yaw - previous));
        memory.store32(0x08000028, std::bit_cast<std::uint32_t>(static_cast<float>(frame)));
        memory.store32(0x08000040, 30000 + static_cast<int>(yaw * 100));
        memory.store32(0x08000080, std::bit_cast<std::uint32_t>(scene.view[2]));
        memory.store16(0x08000180, static_cast<std::uint16_t>(30000 + frame * 1150));
        memory.store16(
            0x08000182, static_cast<std::uint16_t>(frame < 3 ? 100 + frame * 1150 : 2400 + (frame - 2) * 333));

        renderer.begin_frame();
        renderer.submit(scene, memory);
        expect(renderer.present(0x04000000), "synthetic camera scene presents");
        const auto measured = renderer.camera();
        expect(measured.valid && std::fabs(measured.yaw - yaw) < .01f &&
                std::fabs(measured.turn - (yaw - previous)) < .01f,
            "camera measurement recovers known view yaw and per-frame turn");
        probe::camera_frame(runtime, 0);
        previous = yaw;
    }
    expect(memory.load32(0x08000120) == std::bit_cast<std::uint32_t>(1.25f) &&
            memory.load32(0x08000124) == std::bit_cast<std::uint32_t>(-2.5f),
        "diagnostic float poke parses and writes multiple values");
    expect(memory.load32(0x08000140) == std::bit_cast<std::uint32_t>(6.5f) &&
            memory.load32(0x08000144) == std::bit_cast<std::uint32_t>(6.5f),
        "diagnostic float finder writes explicitly selected matching copies");
    expect(memory.load32(0x08000160) == static_cast<std::uint32_t>(-7) &&
            memory.load32(0x08000164) == static_cast<std::uint32_t>(-7),
        "diagnostic integer finder writes signed replacement to selected copies");
    expect(detector_trace.str().find("[find-step] 2 fields moved by exactly 1150") != std::string::npos &&
            detector_trace.str().find("[find-step] 1 left") != std::string::npos,
        "step detector retains wrapped fixed-step angle and rejects inconsistent field");
    std::ifstream output(sandbox / "camera-candidates.txt");
    const std::string text{std::istreambuf_iterator<char>(output), {}};
    expect(text.find("yaw float angle 0x8000020") != std::string::npos,
        "camera detector retains a float angle that tracks measured turn");
    expect(text.find("yaw float rate 0x8000024") != std::string::npos,
        "camera detector retains a rate proportional to measured turn");
    expect(text.find("yaw float angle 0x8000028") == std::string::npos,
        "camera detector rejects a counter that stops tracking changed turn rate");
}

void bindings_editor_contracts(gpu::VulkanRenderer &renderer) {
    auto &player = settings::current();
    const auto saved = player;
    player.controls = input::layout(input::Preset::Default);
    auto &layer = ui::Layer::get();
    layer.set_interactive(true);
    std::string notice;
    constexpr auto action = input::Action::StickUp;
    constexpr auto index = static_cast<int>(action);
    auto frame = [&](const char *kind = nullptr, int slot = 0) {
        layer.begin_frame();
        ui::begin_panel("##binding-edit-contract", "Public bindings", "", false);
        ui::begin_content();
        if (kind) {
            auto *window = ImGui::GetCurrentWindow();
            const std::string target(kind);
            const bool combo =
                target == "new" || target == "button" || target == "done" || target == "remove" || target == "name";
            const bool picker = target == "button" || target == "done";
            const bool conflict = target == "keep" || target == "fix";
            ImGui::PushID("bindings");
            if (combo) ImGui::PushID("combos");
            if (target != "new") ImGui::PushID(combo ? static_cast<int>(input::kActions) : index);
            if (picker) ImGui::PushID("picker");
            if (conflict) ImGui::PushID(0);
            if (target == "button") ImGui::PushID(slot);
            if (target == "chip") {
                ImGui::PushID("keys");
                ImGui::PushID(slot);
            }
            const auto id = window->GetID(kind);
            if (target == "chip") {
                ImGui::PopID();
                ImGui::PopID();
            }
            if (target == "button") ImGui::PopID();
            if (picker) ImGui::PopID();
            if (conflict) ImGui::PopID();
            if (target != "new") ImGui::PopID();
            if (combo) ImGui::PopID();
            ImGui::PopID();
            if (combo) ImGui::SetScrollY(window->ScrollMax.y);
            ImGui::FocusWindow(window);
            ImGui::SetFocusID(id, window);
            ImGui::SetNavCursorVisible(true);
        }
        ui::bindings_editor(notice);
        ui::begin_footer();
        ui::hints({{ui::Control::Confirm, "Rebind"}, {ui::Control::Back, "Back"}});
        ui::end_panel();
        ui::bindings_capture_prompt();
        layer.end_frame();
        renderer.present_ui(false);
    };
    auto activate = [&](const char *kind, int slot, ImGuiKey key) {
        frame(kind, slot);
        frame(kind, slot);
        ImGui::GetIO().AddKeyEvent(key, true);
        frame(kind, slot);
        ImGui::GetIO().AddKeyEvent(key, false);
        frame();
    };
    auto captured_key = [&](SDL_Scancode scan) {
        SDL_Event event{};
        event.key.windowID = SDL_GetWindowID(renderer.window());
        event.key.scancode = scan;
        event.key.key = SDL_GetKeyFromScancode(scan, SDL_KMOD_NONE, false);
        event.type = SDL_EVENT_KEY_DOWN;
        event.key.down = true;
        expect(SDL_PushEvent(&event), "editor binding key-down queues");
        renderer.pump_events();
        frame();
        event.type = SDL_EVENT_KEY_UP;
        event.key.down = false;
        expect(SDL_PushEvent(&event), "editor binding key-up queues");
        renderer.pump_events();
        frame();
    };
    frame();
    activate("chip", 0, ImGuiKey_Delete);
    expect(input::count(player.controls.keys[index]) == 0 && !notice.empty(),
        "Delete clears focused movement binding and creates editable preset notice");
    activate("chip", 0, ImGuiKey_Space);
    expect(layer.capturing_binding(), "Add chip starts actual keyboard capture");
    captured_key(SDL_SCANCODE_F10);
    expect(!layer.capturing_binding() && player.controls.keys[index][0].inputs[0] == input::key(SDL_SCANCODE_F10),
        "editor capture stores exact F10 physical input");
    activate("chip", 1, ImGuiKey_Space);
    expect(layer.capturing_binding(), "second Add chip starts capture");
    captured_key(SDL_SCANCODE_F10);
    expect(input::count(player.controls.keys[index]) == 1, "duplicate captured binding does not add another slot");
    activate("chip", 0, ImGuiKey_Space);
    captured_key(SDL_SCANCODE_ESCAPE);
    expect(!layer.capturing_binding() && player.controls.keys[index][0].inputs[0] == input::key(SDL_SCANCODE_F10),
        "Escape cancels rebind and preserves existing physical input");
    frame("reset");
    expect(ui::bindings_focus_resettable(), "edited action exposes resettable focus");
    activate("reset", 0, ImGuiKey_Space);
    expect(player.controls.keys[index] == input::layout(input::Preset::Default).keys[index],
        "reset chip restores shipped movement bindings exactly");
    expect(!ui::bindings_summary(action).empty(), "restored action has meaningful binding summary");
    activate("new", 0, ImGuiKey_Space);
    expect(player.controls.combos.size() == 1 && player.controls.combos[0].buttons == 0,
        "New combination creates exactly one empty custom action");
    activate("button", 0x1000, ImGuiKey_Space);
    activate("button", 0x2000, ImGuiKey_Space);
    expect(player.controls.combos.size() == 1 && player.controls.combos[0].buttons == 0x3000,
        "combination picker selects exact Triangle plus Circle PSP bits");
    activate("done", 0, ImGuiKey_Space);
    activate("name", 0, ImGuiKey_Space);
    activate("button", 0x2000, ImGuiKey_Space);
    expect(player.controls.combos.size() == 1 && player.controls.combos[0].buttons == 0x1000,
        "reopened combination picker toggles one PSP bit independently");
    activate("done", 0, ImGuiKey_Space);
    activate("remove", 0, ImGuiKey_Space);
    expect(player.controls.combos.empty(), "remove chip deletes custom combination and its bindings");
    player.controls.keys = {};
    player.controls.pad = {};
    const auto other = static_cast<int>(input::Action::StickDown);
    player.controls.keys[index][0].inputs[0] = input::key(SDL_SCANCODE_F9);
    player.controls.keys[other][0] = player.controls.keys[index][0];
    frame();
    expect(ui::bindings_conflicts() > 0, "identical physical inputs expose conflict warnings");
    activate("fix", 0, ImGuiKey_Space);
    expect(input::count(player.controls.keys[other]) == 0 && input::count(player.controls.keys[index]) == 1,
        "Fix removes conflicting action binding while preserving focused action");
    player.controls.keys[index][0].inputs[0] = input::key(SDL_SCANCODE_F8);
    player.controls.keys[other][0] = player.controls.keys[index][0];
    frame();
    const auto before_keep = ui::bindings_conflicts();
    activate("keep", 0, ImGuiKey_Space);
    expect(input::count(player.controls.keys[other]) == 1 && input::count(player.controls.keys[index]) == 1 &&
            ui::bindings_conflicts() < before_keep,
        "Keep both preserves both bindings and acknowledges conflict warning");
    player = saved;
    settings::save();
    layer.set_interactive(false);
}

void touch_editor_contracts(gpu::VulkanRenderer &renderer) {
    auto &player = settings::current();
    const auto saved_layout = player.touch_action;
    const auto saved_opacity = player.touch_opacity;
    const auto saved_size = player.touch_size;
    const auto saved_haptics = player.touch_haptics;
    player.touch_action = input::touch::default_action_layout();
    player.touch_opacity = 0.5f;
    player.touch_size = 1;
    auto &layer = ui::Layer::get();
    layer.set_interactive(true);
    ui::open_touch_editor();
    auto frame = [&](const char *focused = nullptr) {
        layer.begin_frame();
        if (focused) {
            if (auto *window = ImGui::FindWindowByName("##touch_editor")) {
                ImGui::FocusWindow(window);
                ImGui::SetFocusID(window->GetID(focused), window);
                ImGui::SetNavCursorVisible(true);
            }
        }
        ui::touch_editor_frame(false);
        layer.end_frame();
        renderer.present_ui(true);
    };
    auto activate = [&](const char *label, ImGuiKey key = ImGuiKey_Space) {
        frame(label);
        frame(label);
        ImGui::GetIO().AddKeyEvent(key, true);
        frame(label);
        ImGui::GetIO().AddKeyEvent(key, false);
        frame();
    };
    frame();
    activate("Opacity", ImGuiKey_RightArrow);
    expect(std::abs(player.touch_opacity - 0.55f) < 0.001f, "touch editor opacity advances exactly five percent");
    activate("Size of all", ImGuiKey_RightArrow);
    expect(std::abs(player.touch_size - 1.05f) < 0.001f, "touch editor global size advances exactly five percent");
    const auto before_haptic = player.touch_haptics;
    activate("Haptic feedback");
    expect(player.touch_haptics != before_haptic, "touch editor haptic option toggles");
    activate("Panel", ImGuiKey_RightArrow);
    activate("Panel", ImGuiKey_LeftArrow);
    const auto &controls = renderer.action_touch_controls();
    const auto at = controls.placed(input::touch::Element::Attack).centre;
    const auto before = player.touch_action.at(input::touch::Element::Attack);
    ImGui::GetIO().AddMousePosEvent(at.x, at.y);
    frame();
    ImGui::GetIO().AddMouseButtonEvent(0, true);
    frame();
    ImGui::GetIO().AddMousePosEvent(at.x - 20, at.y - 15);
    frame();
    ImGui::GetIO().AddMouseButtonEvent(0, false);
    frame();
    const auto moved = player.touch_action.at(input::touch::Element::Attack);
    expect(moved.x != before.x && moved.y != before.y,
        "mouse drag moves selected attack placement in safe-area coordinates");
    activate("Size", ImGuiKey_RightArrow);
    expect(player.touch_action.at(input::touch::Element::Attack).size > moved.size,
        "selected touch element size increases");
    activate("Presses", ImGuiKey_RightArrow);
    expect(player.touch_action.at(input::touch::Element::Attack).buttons != moved.buttons,
        "selected touch element rebinds to a different PSP chord");
    activate("Shown");
    expect(!player.touch_action.at(input::touch::Element::Attack).shown, "selected touch element can be hidden");
    activate("Reset this element");
    expect(player.touch_action.at(input::touch::Element::Attack) ==
            input::touch::default_action_layout().at(input::touch::Element::Attack),
        "element reset restores complete default placement and binding");
    activate("Choose another");
    activate("Reset the whole layout");
    expect(player.touch_action == input::touch::default_action_layout(), "whole layout reset restores every element");
    activate("Done");
    expect(!ui::touch_editor_open(), "Done closes touch editor");
    player.touch_action = saved_layout;
    player.touch_opacity = saved_opacity;
    player.touch_size = saved_size;
    player.touch_haptics = saved_haptics;
    settings::save();
    layer.set_interactive(false);
}

void touch_event_contracts(gpu::VulkanRenderer &renderer) {
    auto &player = settings::current();
    const auto enabled = player.touch_controls;
    const auto original = player.touch_layout;
    player.touch_controls = true;
    player.touch_layout = settings::TouchLayout::Psp;
    renderer.set_game_input(true);
    int width{}, height{};
    expect(SDL_GetWindowSize(renderer.window(), &width, &height), "touch fixture reads real SDL window dimensions");
    auto finger = [&](SDL_EventType type, std::uint64_t id, input::touch::Point at) {
        SDL_Event event{};
        event.type = type;
        event.tfinger.windowID = SDL_GetWindowID(renderer.window());
        event.tfinger.touchID = 1;
        event.tfinger.fingerID = id;
        event.tfinger.timestamp = SDL_GetTicksNS();
        event.tfinger.x = at.x / static_cast<float>(width);
        event.tfinger.y = at.y / static_cast<float>(height);
        event.tfinger.pressure = 1;
        expect(SDL_PushEvent(&event), "synthetic normalized SDL finger event queues");
        renderer.pump_events();
    };
    auto draw_overlay = [&] {
        auto &layer = ui::Layer::get();
        layer.begin_frame();
        if (player.touch_layout == settings::TouchLayout::Psp)
            ui::draw_touch_controls(renderer.touch_controls(), 0.6f);
        else
            ui::draw_action_controls(renderer.action_touch_controls(), 0.6f, SDL_GetTicks());
        layer.end_frame();
        expect(ImGui::GetDrawData() && ImGui::GetDrawData()->TotalVtxCount > 0,
            "held touch controls generate real ImGui overlay geometry");
        renderer.present_ui(true);
    };
    const auto psp = renderer.touch_controls().layout();
    const auto cross = psp.controls[static_cast<std::size_t>(input::touch::Control::Cross)].centre;
    finger(SDL_EVENT_FINGER_DOWN, 1, cross);
    expect(renderer.touch_controls_visible() && renderer.touch_controls().held(input::touch::Control::Cross),
        "SDL touch-down shows overlay and holds actual PSP cross control");
    draw_overlay();
    finger(SDL_EVENT_FINGER_CANCELED, 1, cross);
    expect(!renderer.touch_controls().held(input::touch::Control::Cross), "finger cancellation releases PSP control");
    const input::touch::Point camera{width * 0.60f, height * 0.5f};
    finger(SDL_EVENT_FINGER_DOWN, 2, camera);
    finger(SDL_EVENT_FINGER_MOTION, 2, {camera.x + width * 0.04f, camera.y + height * 0.02f});
    const auto motion = renderer.take_touch_motion();
    expect(motion.x > 0 && motion.y > 0, "SDL finger motion accumulates normalized camera drag");
    expect(renderer.take_touch_motion().x == 0, "touch drag is consumed once");
    finger(SDL_EVENT_FINGER_UP, 2, camera);
    const auto menu = psp.controls[static_cast<std::size_t>(input::touch::Control::Menu)].centre;
    finger(SDL_EVENT_FINGER_DOWN, 3, menu);
    finger(SDL_EVENT_FINGER_UP, 3, menu);
    expect(renderer.take_touch_menu() && !renderer.take_touch_menu(), "touch menu tap is delivered exactly once");
    player.touch_layout = settings::TouchLayout::Action;
    const auto attack = renderer.action_touch_controls().placed(input::touch::Element::Attack).centre;
    finger(SDL_EVENT_FINGER_DOWN, 4, attack);
    expect(renderer.action_touch_controls().held(input::touch::Element::Attack),
        "action layout processes real SDL attack finger");
    draw_overlay();
    finger(SDL_EVENT_FINGER_UP, 4, attack);
    expect(!renderer.action_touch_controls().held(input::touch::Element::Attack), "action attack releases on up");
    finger(SDL_EVENT_FINGER_DOWN, 5, attack);
    renderer.set_game_input(false);
    expect(!renderer.touch_controls_visible() && !renderer.action_touch_controls().held(input::touch::Element::Attack),
        "losing game input hides overlay and releases action fingers");
    renderer.set_game_input(true);
    finger(SDL_EVENT_FINGER_DOWN, 6, attack);
    SDL_Event keyboard{};
    keyboard.type = SDL_EVENT_KEY_DOWN;
    keyboard.key.windowID = SDL_GetWindowID(renderer.window());
    keyboard.key.key = SDLK_K;
    keyboard.key.scancode = SDL_SCANCODE_K;
    keyboard.key.down = true;
    expect(SDL_PushEvent(&keyboard), "keyboard takeover event queues");
    renderer.pump_events();
    expect(!renderer.touch_controls_visible() && !renderer.action_touch_controls().held(input::touch::Element::Attack),
        "keyboard takeover releases touch controls");
    keyboard.type = SDL_EVENT_KEY_UP;
    keyboard.key.down = false;
    SDL_PushEvent(&keyboard);
    renderer.pump_events();
    player.touch_controls = enabled;
    player.touch_layout = original;
}

void scripted_mouse_contracts(gpu::VulkanRenderer &renderer) {
    const auto saved = settings::current();
    auto &player = settings::current();
    player.mouse = true;
    player.controls.keys = {};
    player.controls.pad = {};
    player.controls.keys[static_cast<int>(input::Action::Triangle)][0].inputs[0] = input::mouse_button(1);
    ui::Layer::get().set_interactive(false);
    renderer.set_scripted_input(true);
    renderer.set_pointer_free(false);
    renderer.set_game_input(true);
    renderer.pump_events();
    expect(renderer.mouse_captured(), "scripted game captures logical mouse without taking the physical pointer");
    static_cast<void>(renderer.take_mouse_motion());
    auto motion = [&](std::uint32_t device, float x, float y) {
        SDL_Event event{};
        event.type = SDL_EVENT_MOUSE_MOTION;
        event.motion.windowID = SDL_GetWindowID(renderer.window());
        event.motion.which = device;
        event.motion.xrel = x;
        event.motion.yrel = y;
        expect(SDL_PushEvent(&event), "mouse motion event queues");
        renderer.pump_events();
    };
    motion(0, 90, 80);
    const auto ignored = renderer.take_mouse_motion();
    expect(ignored.x == 0 && ignored.y == 0, "scripted run rejects unrelated physical-device mouse motion");
    motion(gpu::kScriptedMouse, 7, -3);
    const auto moved = renderer.take_mouse_motion();
    expect(moved.x == 7 && moved.y == -3, "scripted captured mouse retains exact relative counts");
    const auto consumed = renderer.take_mouse_motion();
    expect(consumed.x == 0 && consumed.y == 0, "mouse motion is consumed once");
    auto button = [&](std::uint32_t device, bool down) {
        SDL_Event event{};
        event.type = down ? SDL_EVENT_MOUSE_BUTTON_DOWN : SDL_EVENT_MOUSE_BUTTON_UP;
        event.button.windowID = SDL_GetWindowID(renderer.window());
        event.button.which = device;
        event.button.button = 1;
        event.button.down = down;
        expect(SDL_PushEvent(&event), "mouse button event queues");
        renderer.pump_events();
        renderer.sample_pad();
    };
    button(0, true);
    expect((renderer.pad().buttons & 0x1000) == 0, "unrelated device cannot press scripted PSP mouse binding");
    button(gpu::kScriptedMouse, true);
    expect((renderer.pad().buttons & 0x1000) != 0, "scripted mouse button drives exact PSP Triangle binding");
    button(0, false);
    expect((renderer.pad().buttons & 0x1000) == 0, "mouse release clears held state across device identity");
    button(gpu::kScriptedMouse, true);
    motion(gpu::kScriptedMouse, 2, 4);
    renderer.set_pointer_free(true);
    renderer.pump_events();
    renderer.sample_pad();
    const auto freed = renderer.take_mouse_motion();
    expect(!renderer.mouse_captured() && (renderer.pad().buttons & 0x1000) == 0 && freed.x == 0 && freed.y == 0,
        "freeing pointer drops held buttons and accumulated motion without a camera jump");
    motion(gpu::kScriptedMouse, 50, 60);
    const auto ungrabbed = renderer.take_mouse_motion();
    expect(ungrabbed.x == 0 && ungrabbed.y == 0, "uncaptured scripted mouse motion does not reach game");
    renderer.set_game_input(false);
    renderer.set_scripted_input(false);
    renderer.set_pointer_free(false);
    player = saved;
    settings::save();
}

void virtual_gamepad_contracts(gpu::VulkanRenderer &renderer) {
    SDL_VirtualJoystickDesc desc{};
    SDL_INIT_INTERFACE(&desc);
    desc.type = SDL_JOYSTICK_TYPE_GAMEPAD;
    desc.naxes = SDL_GAMEPAD_AXIS_COUNT;
    desc.nbuttons = SDL_GAMEPAD_BUTTON_COUNT;
    desc.axis_mask = (1u << SDL_GAMEPAD_AXIS_COUNT) - 1;
    desc.button_mask = (1u << SDL_GAMEPAD_BUTTON_COUNT) - 1;
    desc.name = "Yakumo synthetic test gamepad";
    const auto id = SDL_AttachVirtualJoystick(&desc);
    expect(id != 0, "synthetic SDL gamepad attaches");
    if (!id) return;
    auto *pad = SDL_OpenGamepad(id);
    expect(pad != nullptr, "virtual joystick is recognized as a gamepad");
    if (!pad) {
        SDL_DetachVirtualJoystick(id);
        return;
    }
    auto *joystick = SDL_GetGamepadJoystick(pad);
    ImGui_ImplSDL3_SetGamepadMode(ImGui_ImplSDL3_GamepadMode_Manual, &pad, 1);
    auto &layer = ui::Layer::get();
    layer.set_interactive(true);
    auto frame = [&] {
        SDL_UpdateJoysticks();
        renderer.pump_events();
        layer.begin_frame();
        ui::text_input_frame();
        layer.end_frame();
        renderer.present_ui(false);
    };
    auto press = [&](SDL_GamepadButton button) {
        expect(SDL_SetJoystickVirtualButton(joystick, button, true), "virtual gamepad button presses");
        frame();
        expect(SDL_GetGamepadButton(pad, button), "SDL gamepad observes virtual pressed state");
        expect(SDL_SetJoystickVirtualButton(joystick, button, false), "virtual gamepad button releases");
        frame();
    };
    frame();
    frame();
    std::optional<std::string> result;
    ui::TextInputRequest request;
    request.title = "Gamepad keyboard contract";
    request.max_length = 16;
    ui::open_text_input(request, [&](auto text) { result = std::move(text); });
    frame();
    frame();
    expect(layer.gamepad_armed(), "released virtual gamepad arms keyboard input");
    const auto confirm = layer.confirm_south() ? SDL_GAMEPAD_BUTTON_SOUTH : SDL_GAMEPAD_BUTTON_EAST;
    const auto back = layer.confirm_south() ? SDL_GAMEPAD_BUTTON_EAST : SDL_GAMEPAD_BUTTON_SOUTH;
    press(confirm);                 // q
    press(SDL_GAMEPAD_BUTTON_WEST); // Shift once
    press(confirm);                 // Q, resets shift
    press(SDL_GAMEPAD_BUTTON_WEST);
    press(SDL_GAMEPAD_BUTTON_WEST);  // Caps lock
    press(confirm);                  // Q
    press(SDL_GAMEPAD_BUTTON_WEST);  // Shift off
    press(back);                     // delete last Q
    press(SDL_GAMEPAD_BUTTON_NORTH); // space
    press(SDL_GAMEPAD_BUTTON_BACK);  // symbol page
    press(confirm);                  // !
    press(SDL_GAMEPAD_BUTTON_DPAD_DOWN);
    press(SDL_GAMEPAD_BUTTON_DPAD_UP);
    press(SDL_GAMEPAD_BUTTON_LEFT_SHOULDER);
    press(SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER);
    press(SDL_GAMEPAD_BUTTON_START);
    if (result != "qQ !" || ui::text_input_open())
        std::cerr << "virtual result=" << result.value_or("<empty>") << " open=" << ui::text_input_open() << "\n";
    expect(result == "qQ !" && !ui::text_input_open(),
        "gamepad keyboard shift/caps/delete/space/symbols/cursor/accept contract");
    layer.begin_binding_capture(ui::Layer::Capture::Pad);
    press(SDL_GAMEPAD_BUTTON_SOUTH);
    auto captured = layer.take_captured_binding();
    expect(captured && captured->inputs[0] == input::pad(input::PadInput::South),
        "real SDL pad capture records released button");
    auto controller_frame = [&](const char *focused = nullptr, bool back = false) {
        SDL_UpdateJoysticks();
        renderer.pump_events();
        layer.begin_frame();
        ui::begin_panel("##controller-contract", "Public controller", "", false);
        ui::begin_content();
        if (focused) {
            auto *window = ImGui::GetCurrentWindow();
            const auto focus = window->GetID(focused);
            ImGui::FocusWindow(window);
            ImGui::SetFocusID(focus, window);
            ImGui::SetNavCursorVisible(true);
        }
        if (ui::controllers_screen_open())
            ui::controllers_screen(back);
        else
            ui::controllers_rows();
        ui::begin_footer();
        ui::hints({{ui::Control::Confirm, "Choose"}, {ui::Control::Back, "Back"}});
        ui::end_panel();
        layer.end_frame();
        renderer.present_ui(false);
    };
    auto activate = [&](const char *label) {
        controller_frame(label);
        controller_frame(label);
        ImGui::GetIO().AddKeyEvent(ImGuiKey_Space, true);
        controller_frame(label);
        ImGui::GetIO().AddKeyEvent(ImGuiKey_Space, false);
        controller_frame();
    };
    controller_frame();
    activate("Connected controllers");
    expect(ui::controllers_screen_open(), "controller row opens actual connected device screen");
    expect(SDL_SetJoystickVirtualAxis(joystick, SDL_GAMEPAD_AXIS_LEFTX, 24000), "live controller axis changes");
    expect(SDL_SetJoystickVirtualButton(joystick, SDL_GAMEPAD_BUTTON_SOUTH, true), "live controller button changes");
    controller_frame();
    expect(input::devices::snapshot(id).axes[SDL_GAMEPAD_AXIS_LEFTX] == 24000,
        "controller live snapshot reads actual virtual SDL axis");
    SDL_SetJoystickVirtualAxis(joystick, SDL_GAMEPAD_AXIS_LEFTX, 0);
    SDL_SetJoystickVirtualButton(joystick, SDL_GAMEPAD_BUTTON_SOUTH, false);
    controller_frame();
    activate("Set up this controller again");
    controller_frame();
    SDL_SetJoystickVirtualButton(joystick, SDL_GAMEPAD_BUTTON_SOUTH, true);
    controller_frame();
    SDL_SetJoystickVirtualButton(joystick, SDL_GAMEPAD_BUTTON_SOUTH, false);
    controller_frame();
    // Reusing the same physical button is rejected for the next face control.
    SDL_SetJoystickVirtualButton(joystick, SDL_GAMEPAD_BUTTON_SOUTH, true);
    controller_frame();
    SDL_SetJoystickVirtualButton(joystick, SDL_GAMEPAD_BUTTON_SOUTH, false);
    controller_frame();
    activate("Back one step");
    SDL_SetJoystickVirtualButton(joystick, SDL_GAMEPAD_BUTTON_SOUTH, true);
    controller_frame();
    SDL_SetJoystickVirtualButton(joystick, SDL_GAMEPAD_BUTTON_SOUTH, false);
    controller_frame();
    // One real answer, then deliberately skip controls absent from this fixture.
    for (int step = 1; step < 20; ++step) activate("Skip this one");
    activate("Save and use this layout");
    const auto info = input::devices::info(id);
    expect(info && info->saved, "controller wizard saves actual first button answer for virtual device");
    const auto file = input::devices::mappings_file();
    std::ifstream mappings(file);
    const std::string saved{std::istreambuf_iterator<char>(mappings), std::istreambuf_iterator<char>()};
    expect(saved.find("a:b0") != std::string::npos && saved.find("Yakumo synthetic test gamepad") != std::string::npos,
        "saved SDL mapping contains exact virtual device name and recorded bottom button");
    activate("Set up this controller again");
    controller_frame(nullptr, true); // Back cancels the wizard, leaving its list open.
    expect(ui::controllers_screen_open() && input::devices::info(id)->saved,
        "cancelled setup keeps controller screen and previous mapping");
    activate("Remove my layout");
    expect(input::devices::info(id) && !input::devices::info(id)->saved,
        "removing virtual controller layout clears saved mapping");
    std::ifstream removed(file);
    const std::string remaining{std::istreambuf_iterator<char>(removed), std::istreambuf_iterator<char>()};
    expect(remaining.find("Yakumo synthetic test gamepad") == std::string::npos,
        "removed mapping no longer appears in sandbox database");
    controller_frame(nullptr, true);
    expect(!ui::controllers_screen_open(), "back closes controller screen after successful wizard");
    const auto saved_settings = settings::current();
    auto &player = settings::current();
    player.controls.keys = {};
    player.controls.pad = {};
    player.controls.swap_sticks = false;
    player.confirm_south = false;
    player.dead_zone = 0.2f;
    player.trigger = 0.5f;
    player.right_stick = settings::RightStick::Camera;
    player.invert_camera_x = player.invert_camera_y = false;
    player.controls.pad[static_cast<int>(input::Action::Triangle)][0].inputs[0] = input::pad(input::PadInput::South);
    player.controls.pad[static_cast<int>(input::Action::Cross)][0].inputs[0] = input::pad(input::PadInput::LeftTrigger);
    layer.set_interactive(false);
    renderer.set_game_input(true);
    auto sample = [&] {
        SDL_UpdateJoysticks();
        renderer.pump_events();
        renderer.sample_pad();
        return renderer.pad();
    };
    for (int axis = 0; axis < SDL_GAMEPAD_AXIS_COUNT; ++axis) SDL_SetJoystickVirtualAxis(joystick, axis, 0);
    for (int button = 0; button < SDL_GAMEPAD_BUTTON_COUNT; ++button)
        SDL_SetJoystickVirtualButton(joystick, button, false);
    sample();
    SDL_SetJoystickVirtualButton(joystick, SDL_GAMEPAD_BUTTON_SOUTH, true);
    expect((sample().buttons & 0x1000) != 0, "physical virtual South button drives exact custom PSP Triangle bit");
    player.confirm_south = true;
    expect((sample().buttons & 0x1000) == 0, "confirm swap changes physical custom face-button source");
    SDL_SetJoystickVirtualButton(joystick, SDL_GAMEPAD_BUTTON_SOUTH, false);
    SDL_SetJoystickVirtualButton(joystick, SDL_GAMEPAD_BUTTON_EAST, true);
    expect((sample().buttons & 0x1000) != 0, "swapped virtual East button drives same PSP Triangle binding");
    SDL_SetJoystickVirtualButton(joystick, SDL_GAMEPAD_BUTTON_EAST, false);
    SDL_SetJoystickVirtualAxis(joystick, SDL_GAMEPAD_AXIS_LEFT_TRIGGER, -24000);
    expect((sample().buttons & 0x4000) == 0, "trigger below configured threshold leaves PSP Cross released");
    SDL_SetJoystickVirtualAxis(joystick, SDL_GAMEPAD_AXIS_LEFT_TRIGGER, 32767);
    expect((sample().buttons & 0x4000) != 0, "trigger above configured threshold presses exact PSP Cross bit");
    SDL_SetJoystickVirtualAxis(joystick, SDL_GAMEPAD_AXIS_LEFT_TRIGGER, -32768);
    SDL_SetJoystickVirtualAxis(joystick, SDL_GAMEPAD_AXIS_LEFTX, 1000);
    expect(sample().analog_x == 128, "movement dead zone leaves centered PSP analog value");
    SDL_SetJoystickVirtualAxis(joystick, SDL_GAMEPAD_AXIS_LEFTX, 32767);
    expect(sample().analog_x == 255, "full virtual movement axis reaches PSP analog endpoint");
    SDL_SetJoystickVirtualAxis(joystick, SDL_GAMEPAD_AXIS_LEFTX, 0);
    SDL_SetJoystickVirtualAxis(joystick, SDL_GAMEPAD_AXIS_RIGHTX, 32767);
    expect(sample().right_x == 255, "full virtual camera axis reaches HD analog endpoint");
    player.invert_camera_x = true;
    expect(sample().right_x == 1, "camera inversion reverses normalized virtual axis endpoint");
    player.right_stick = settings::RightStick::DPad;
    expect((sample().buttons & 0x20) != 0 && sample().right_x == 128,
        "D-pad camera mode emits PSP Right without also turning HD analog camera");
    player.right_stick = settings::RightStick::Off;
    expect((sample().buttons & 0xf0) == 0 && sample().right_x == 128,
        "disabled camera mode leaves both PSP directions and HD camera centered");
    player.controls.swap_sticks = true;
    expect(sample().analog_x == 255, "swapped sticks move PSP analog from virtual right axis");
    SDL_SetJoystickVirtualAxis(joystick, SDL_GAMEPAD_AXIS_RIGHTX, 0);
    sample();
    SDL_SetJoystickVirtualAxis(joystick, SDL_GAMEPAD_AXIS_LEFT_TRIGGER, -32768);
    SDL_SetJoystickVirtualAxis(joystick, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER, -32768);
    player.free_camera = true;
    renderer.set_scripted_key(SDL_SCANCODE_F6, true);
    sample();
    expect(renderer.take_free_camera_controls().toggle, "scripted F6 requests one free-camera toggle");
    sample();
    expect(!renderer.take_free_camera_controls().toggle, "held free-camera toggle does not repeat");
    renderer.set_scripted_key(SDL_SCANCODE_F6, false);
    renderer.set_free_camera(true);
    for (const auto key : {SDL_SCANCODE_D, SDL_SCANCODE_W, SDL_SCANCODE_E, SDL_SCANCODE_LSHIFT, SDL_SCANCODE_P,
             SDL_SCANCODE_R, SDL_SCANCODE_EQUALS})
        renderer.set_scripted_key(key, true);
    sample();
    const auto flying = renderer.take_free_camera_controls();
    expect(flying.right == 1 && flying.forward == 1 && flying.up == 1 && flying.fast && !flying.slow && flying.pause &&
            flying.reset && flying.speed_steps == 1,
        "flying free camera receives held motion plus independent pause/reset/speed press edges");
    sample();
    const auto held = renderer.take_free_camera_controls();
    expect(held.right == 1 && held.forward == 1 && held.up == 1 && !held.pause && !held.reset && held.speed_steps == 0,
        "taking camera controls consumes press edges while preserving held movement");
    for (const auto key : {SDL_SCANCODE_A, SDL_SCANCODE_S, SDL_SCANCODE_Q, SDL_SCANCODE_LCTRL})
        renderer.set_scripted_key(key, true);
    sample();
    const auto opposing = renderer.take_free_camera_controls();
    expect(opposing.right == 0 && opposing.forward == 0 && opposing.up == 0 && opposing.fast && opposing.slow,
        "opposing camera motion keys cancel while fast and slow modifiers stay independent");
    for (const auto key : {SDL_SCANCODE_D, SDL_SCANCODE_W, SDL_SCANCODE_E, SDL_SCANCODE_LSHIFT, SDL_SCANCODE_P,
             SDL_SCANCODE_R, SDL_SCANCODE_EQUALS, SDL_SCANCODE_A, SDL_SCANCODE_S, SDL_SCANCODE_Q, SDL_SCANCODE_LCTRL})
        renderer.set_scripted_key(key, false);
    SDL_SetJoystickVirtualAxis(joystick, SDL_GAMEPAD_AXIS_LEFTX, 32767);
    SDL_SetJoystickVirtualAxis(joystick, SDL_GAMEPAD_AXIS_RIGHTX, 32767);
    SDL_SetJoystickVirtualButton(joystick, SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER, true);
    sample();
    const auto pad_flying = renderer.take_free_camera_controls();
    expect(pad_flying.right == 1 && pad_flying.up == 1 && pad_flying.look_x == -1,
        "virtual gamepad supplies shaped free-camera movement, shoulder elevation and inverted look");
    SDL_SetJoystickVirtualAxis(joystick, SDL_GAMEPAD_AXIS_LEFTX, 0);
    SDL_SetJoystickVirtualAxis(joystick, SDL_GAMEPAD_AXIS_RIGHTX, 0);
    SDL_SetJoystickVirtualButton(joystick, SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER, false);
    player.free_camera = false;
    renderer.set_free_camera(false);
    sample();
    const auto off = renderer.take_free_camera_controls();
    expect(!off.toggle && off.right == 0 && off.look_x == 0 && !off.fast,
        "disabled free camera clears all held and pressed control state");
    player.lock_on = true;
    player.controls.keys[static_cast<int>(input::Action::Screenshot)][0].inputs[0] = input::key(SDL_SCANCODE_F12);
    player.controls.keys[static_cast<int>(input::Action::HideHud)][0].inputs[0] = input::key(SDL_SCANCODE_F11);
    player.controls.keys[static_cast<int>(input::Action::LockOn)][0].inputs[0] = input::key(SDL_SCANCODE_F7);
    static_cast<void>(renderer.take_screenshot_request());
    static_cast<void>(renderer.take_hide_hud_toggle());
    static_cast<void>(renderer.take_lock_on_press());
    renderer.set_scripted_key(SDL_SCANCODE_F12, true);
    renderer.set_scripted_key(SDL_SCANCODE_F11, true);
    sample();
    expect(renderer.take_screenshot_request() && renderer.take_hide_hud_toggle(),
        "custom screenshot and HUD bindings each produce one host request");
    sample();
    expect(!renderer.take_screenshot_request() && !renderer.take_hide_hud_toggle(),
        "holding host request keys does not repeat consumed presses");
    renderer.set_scripted_key(SDL_SCANCODE_F12, false);
    renderer.set_scripted_key(SDL_SCANCODE_F11, false);
    renderer.set_scripted_key(SDL_SCANCODE_F7, true);
    sample();
    expect(!renderer.take_lock_on_press(), "lock-on tap is not emitted before physical release");
    renderer.set_scripted_key(SDL_SCANCODE_F7, false);
    sample();
    expect(renderer.take_lock_on_press() && !renderer.take_lock_on_press(),
        "released lock-on binding produces a single consumable tap");
    player = saved_settings;
    settings::save();
    renderer.set_game_input(false);
    ImGui_ImplSDL3_SetGamepadMode(ImGui_ImplSDL3_GamepadMode_AutoAll);
    SDL_CloseGamepad(pad);
    expect(SDL_DetachVirtualJoystick(id), "virtual test device detaches");
    renderer.pump_events();
    layer.set_interactive(false);
}

void focused_widget_contracts(gpu::VulkanRenderer &renderer) {
    auto &layer = ui::Layer::get();
    layer.set_interactive(true);
    int value = 9, mode = 0, delta{};
    bool on{};
    auto frame = [&](bool disabled = false) {
        layer.begin_frame();
        ui::begin_panel("##focused-contract", "Focused contracts", "", false);
        ui::begin_content();
        ui::focus_next_row();
        if (mode == 0)
            ui::slider_row("Bounded slider", value, 0, 10, 3, "%d", {disabled, "", "Slider contract", true});
        else if (mode == 1) {
            if (ui::toggle_row("Toggle", on, {disabled})) on = !on;
        } else
            delta = ui::choice_row("Choice", "Public choice", {disabled});
        ui::info_row("Information", "Public fixture");
        ui::begin_footer();
        ui::hints({{ui::Control::Confirm, "Choose"}, {ui::Control::Back, "Back"}});
        ui::end_panel();
        layer.end_frame();
        renderer.present_ui(false);
    };
    auto press = [&](ImGuiKey key, bool disabled = false) {
        ImGui::GetIO().AddKeyEvent(key, true);
        frame(disabled);
        ImGui::GetIO().AddKeyEvent(key, false);
        frame(disabled);
    };
    frame();
    frame();
    press(ImGuiKey_RightArrow);
    expect(value == 10, "slider step clamps at upper bound");
    press(ImGuiKey_LeftArrow);
    expect(value == 7, "slider keyboard uses configured step");
    value = 1;
    press(ImGuiKey_LeftArrow);
    expect(value == 0, "slider step clamps at lower bound");
    press(ImGuiKey_RightArrow, true);
    expect(value == 0, "disabled focused slider ignores keyboard changes");
    mode = 1;
    frame();
    press(ImGuiKey_RightArrow);
    expect(on, "right enables focused toggle");
    press(ImGuiKey_RightArrow);
    expect(on, "right on enabled toggle is idempotent");
    press(ImGuiKey_LeftArrow);
    expect(!on, "left disables focused toggle");
    press(ImGuiKey_RightArrow, true);
    expect(!on, "disabled toggle ignores keyboard changes");
    mode = 2;
    frame();
    ImGui::GetIO().AddKeyEvent(ImGuiKey_RightArrow, true);
    frame();
    expect(delta == 1, "choice keyboard right returns next delta");
    ImGui::GetIO().AddKeyEvent(ImGuiKey_RightArrow, false);
    frame();
    ImGui::GetIO().AddKeyEvent(ImGuiKey_LeftArrow, true);
    frame();
    expect(delta == -1, "choice keyboard left returns previous delta");
    ImGui::GetIO().AddKeyEvent(ImGuiKey_LeftArrow, false);
    frame();
    layer.set_interactive(false);
}

// One finite key gesture terminates each modal. The enclosing CTest timeout
// bounds the real UI event loop if a regression ignores this gesture.
void setup_screen_contracts(gpu::VulkanRenderer &renderer, const std::filesystem::path &sandbox) {
    auto &layer = ui::Layer::get();
    layer.set_interactive(true);
    auto setup = ui::make_setup_screens();
    expect(setup != nullptr, "setup uses the actual in-window installer interface");
    expect(!with_escape(renderer, [&] { return setup->introduce(sandbox); }), "welcome back cancels installation");
    expect(!with_escape(renderer, [&] { return setup->offer_retry("Synthetic invalid image"); }),
        "invalid-image retry screen permits quitting");
    expect(!with_escape(renderer, [&] { return setup->offer_retry("Not enough free space for the synthetic image"); }),
        "insufficient-space retry screen permits quitting");
    expect(!with_escape(renderer, [&] { return setup->choose_storage(sandbox / "synthetic.iso", {1024}, sandbox); }),
        "storage selection cancels without choosing copy or in-place");
    expect(
        !with_escape(renderer, [&] { return setup->choose_storage(sandbox / "synthetic.iso", {1ULL << 60}, sandbox); }),
        "storage selection handles insufficient capacity and cancels");
    const auto previous_folder = settings::current().last_folder;
    settings::current().last_folder = "/";
    expect(!with_escape(renderer, [&] { return setup->choose_image(); }),
        "image browser at filesystem root cancels without selecting a disc");
    settings::current().last_folder = previous_folder;
    std::atomic<int> stages{};
    setup->run_task("Synthetic preparation", [&] {
        setup->progress("Checking", 0, 0);
        ++stages;
        SDL_Delay(70);
        setup->progress("Copying", 500'000, 2'000'000);
        ++stages;
        SDL_Delay(70);
        setup->progress("Finishing", 1, 2);
        ++stages;
        SDL_Delay(70);
    });
    expect(stages == 3, "progress work completes all stages exactly once");
    bool rethrown{};
    try {
        setup->run_task("Synthetic failing task", [] {
            SDL_Delay(70);
            throw install::InstallError("synthetic worker failure");
        });
    } catch (const install::InstallError &error) {
        rethrown = std::string(error.what()) == "synthetic worker failure";
    }
    expect(rethrown, "worker exception reaches installer caller unchanged");
    bool cancelled{};
    with_escape(renderer, [&] {
        try {
            setup->run_task("Synthetic cancellable task", [&] {
                // Two seconds maximum even if cancellation regresses.
                for (int step = 0; step < 40; ++step) {
                    setup->progress("Bounded synthetic work", step, 40);
                    SDL_Delay(50);
                }
            });
        } catch (const install::InstallCancelled &) {
            cancelled = true;
        }
        return 0;
    });
    expect(cancelled, "back requests cancellation and progress throws InstallCancelled");
    expect(with_escape(renderer, [&] { return ui::show_problem("Synthetic problem", "Public fixture", false); }) ==
            ui::ProblemAnswer::Quit,
        "plain problem back chooses quit");
    expect(with_escape(renderer, [&] { return ui::show_problem("Synthetic problem", "Public fixture", true); }) ==
            ui::ProblemAnswer::Quit,
        "setup problem back chooses quit without requesting setup");
    expect(with_escape(
               renderer, [&] { return ui::ask_choice("Synthetic choice", "Public fixture", "First", "Second"); }) ==
            ui::ChoiceAnswer::Closed,
        "choice back reports closed instead of selecting an option");
    int wrapper_calls{};
    expect(ui::run_with_progress("Public work",
               [&](const ui::ReportProgress &progress) {
                   progress("Public stage", 1, 2);
                   ++wrapper_calls;
                   SDL_Delay(70);
               }) &&
            wrapper_calls == 1,
        "public progress wrapper invokes worker exactly once");
    bool wrapper_error{};
    try {
        ui::run_with_progress(
            "Public failure", [](const ui::ReportProgress &) { throw install::InstallError("public worker error"); });
    } catch (const install::InstallError &error) {
        wrapper_error = std::string(error.what()) == "public worker error";
    }
    expect(wrapper_error, "public progress wrapper preserves failure and restores interaction mode");
    renderer.pump_events();
    layer.set_interactive(false);
}

void menu_contracts(gpu::VulkanRenderer &renderer) {
    const auto volume = settings::current().volume;
    ui::open_menu_over_game();
    expect(ui::menu_over_game(), "menu opens over game");
    auto frame = [&]() {
        ui::draw_over_game();
        renderer.present_ui(true);
    };
    frame();
    frame();
    for (int tab = 0; tab < 6; ++tab) {
        ImGui::GetIO().AddKeyEvent(ImGuiKey_W, true);
        frame();
        ImGui::GetIO().AddKeyEvent(ImGuiKey_W, false);
        frame();
        expect(ui::menu_over_game() && !ui::take_quit_request(),
            "visiting each page retains menu without requesting quit");
    }
    expect(settings::current().volume == volume, "rendering all settings pages leaves volume unchanged");
    auto change_video = [&](const char *label, ImGuiKey key) {
        auto focus = [&]() {
            for (auto *window : ImGui::GetCurrentContext()->Windows) {
                if (std::string(window->Name).find("##menu/content") == std::string::npos) continue;
                ImGui::FocusWindow(window);
                ImGui::SetFocusID(window->GetID(label), window);
                ImGui::SetNavCursorVisible(true);
                ImGui::SetScrollY(window, 0);
            }
        };
        focus();
        frame();
        focus();
        frame();
        focus();
        ImGui::GetIO().AddKeyEvent(key, true);
        frame();
        ImGui::GetIO().AddKeyEvent(key, false);
        frame();
    };
    auto activate_confirmation = [&](const char *label) {
        auto focus_cancel = [&] {
            for (auto *window : ImGui::GetCurrentContext()->Windows) {
                if (std::string(window->Name) != "##confirm") continue;
                ImGui::FocusWindow(window);
                ImGui::SetFocusID(window->GetID(label), window);
                ImGui::SetNavCursorVisible(true);
            }
        };
        focus_cancel();
        frame();
        focus_cancel();
        frame();
        focus_cancel();
        ImGui::GetIO().AddKeyEvent(ImGuiKey_Space, true);
        frame();
        ImGui::GetIO().AddKeyEvent(ImGuiKey_Space, false);
        frame();
    };
    const auto initial_sharp_screen = settings::current().sharp_screen;
    const auto initial_sharp_textures = settings::current().sharp_textures;
    const auto initial_aspect = settings::current().aspect;
    change_video("Scaling filter", ImGuiKey_RightArrow);
    expect(settings::current().sharp_screen != initial_sharp_screen, "video menu changes actual scaling filter");
    change_video("Scaling filter", ImGuiKey_LeftArrow);
    expect(settings::current().sharp_screen == initial_sharp_screen, "scaling filter round trip restores setting");
    change_video("Texture filter", ImGuiKey_RightArrow);
    expect(settings::current().sharp_textures != initial_sharp_textures, "video menu changes actual texture filter");
    change_video("Texture filter", ImGuiKey_LeftArrow);
    expect(settings::current().sharp_textures == initial_sharp_textures, "texture filter round trip restores setting");
    change_video("Aspect ratio", ImGuiKey_RightArrow);
    expect(settings::current().aspect != initial_aspect, "video menu cycles renderer aspect ratio");
    change_video("Aspect ratio", ImGuiKey_LeftArrow);
    expect(settings::current().aspect == initial_aspect, "aspect ratio round trip restores renderer setting");
    auto page = [&] {
        ImGui::GetIO().AddKeyEvent(ImGuiKey_W, true);
        frame();
        ImGui::GetIO().AddKeyEvent(ImGuiKey_W, false);
        frame();
    };
    auto &sink = audio::AudioSink::instance();
    sink.initialize();
    expect(sink.has_device(), "audio menu has an isolated dummy device");
    const auto saved_mute = settings::current().mute;
    settings::current().volume = 50;
    settings::current().mute = false;
    page();
    change_video("Volume", ImGuiKey_RightArrow);
    expect(settings::current().volume == 55, "audio menu volume steps by exactly five percent");
    change_video("Volume", ImGuiKey_LeftArrow);
    expect(settings::current().volume == 50, "audio menu volume reverse step restores gain setting");
    change_video("Mute", ImGuiKey_RightArrow);
    expect(settings::current().mute, "audio menu mute enables zero gain");
    change_video("Mute", ImGuiKey_LeftArrow);
    expect(!settings::current().mute, "audio menu mute restores gain without losing volume");
    change_video("Restore audio defaults", ImGuiKey_Space);
    expect(settings::current().volume == settings::defaults().volume &&
            settings::current().mute == settings::defaults().mute,
        "audio defaults restore shipped gain and mute state");
    settings::current().volume = volume;
    settings::current().mute = saved_mute;
    page();
    const auto initial_preset = settings::current().control_preset;
    const auto initial_controls = settings::current().controls;
    const auto initial_user_presets = settings::current().user_presets;
    change_video("Preset", ImGuiKey_RightArrow);
    expect(settings::current().control_preset != initial_preset, "preset selector applies a different shipped layout");
    change_video("Preset", ImGuiKey_LeftArrow);
    expect(settings::current().control_preset == initial_preset && settings::current().controls == initial_controls,
        "preset selector round trip restores the original bindings");
    change_video("Save as a new preset", ImGuiKey_Space);
    const auto custom_preset = settings::current().control_preset;
    expect(!custom_preset.shipped && settings::current().user_presets.size() == initial_user_presets.size() + 1 &&
            settings::current().controls == initial_controls,
        "saving a custom preset preserves bindings and creates one independent layout");
    change_video("Delete this preset", ImGuiKey_Space);
    activate_confirmation("Cancel");
    expect(settings::current().control_preset == custom_preset &&
            settings::current().user_presets.size() == initial_user_presets.size() + 1,
        "cancelling preset deletion preserves its selection and saved layout");
    change_video("Delete this preset", ImGuiKey_Space);
    activate_confirmation("Delete");
    expect(settings::current().control_preset.shipped == input::Preset::Default &&
            settings::current().user_presets.size() == initial_user_presets.size(),
        "confirmed deletion removes only the chosen custom preset and selects Default");
    settings::current().control_preset = initial_preset;
    settings::current().controls = initial_controls;
    settings::current().user_presets = initial_user_presets;
    const auto initial_chord_window = settings::current().chord_window;
    change_video("Chord window", ImGuiKey_RightArrow);
    expect(settings::current().chord_window == initial_chord_window + 10,
        "combination timing advances by exactly ten milliseconds");
    change_video("Chord window", ImGuiKey_LeftArrow);
    expect(settings::current().chord_window == initial_chord_window, "combination timing round trip restores latency");
    const auto original_confirm = settings::current().confirm_south;
    change_video("Confirm button", ImGuiKey_RightArrow);
    expect(settings::current().confirm_south != original_confirm, "controls menu switches confirm convention");
    change_video("Confirm button", ImGuiKey_LeftArrow);
    expect(settings::current().confirm_south == original_confirm, "confirm convention round trip restores setting");
    auto toggle_setting = [&](const char *label, bool settings::Settings::*field) {
        const bool original = settings::current().*field;
        change_video(label, ImGuiKey_Space);
        expect(settings::current().*field != original,
            (std::string(label) + " control menu toggle changes its own setting").c_str());
        change_video(label, ImGuiKey_Space);
        expect(settings::current().*field == original,
            (std::string(label) + " control menu toggle round trip restores its own setting").c_str());
    };
    toggle_setting("Analog camera", &settings::Settings::analog_camera);
    toggle_setting("Lock-on", &settings::Settings::lock_on);
    toggle_setting("Lock-on marker", &settings::Settings::lock_on_marker);
    toggle_setting("Invert camera horizontally", &settings::Settings::invert_camera_x);
    toggle_setting("Invert camera vertically", &settings::Settings::invert_camera_y);
    toggle_setting("Mouse", &settings::Settings::mouse);
    toggle_setting("Invert mouse horizontally", &settings::Settings::invert_mouse_x);
    toggle_setting("Invert mouse vertically", &settings::Settings::invert_mouse_y);
    toggle_setting("On-screen controls", &settings::Settings::touch_controls);
    toggle_setting("Free camera", &settings::Settings::free_camera);
    auto slider_setting = [&](const char *label, float settings::Settings::*field, float step) {
        const float original = settings::current().*field;
        change_video(label, ImGuiKey_RightArrow);
        expect(std::abs(settings::current().*field - original - step) < 0.0001f,
            (std::string(label) + " menu slider applies its documented increment").c_str());
        change_video(label, ImGuiKey_LeftArrow);
        expect(std::abs(settings::current().*field - original) < 0.0001f,
            (std::string(label) + " menu slider round trip restores its exact setting").c_str());
    };
    slider_setting("Stick dead zone", &settings::Settings::dead_zone, 0.01f);
    slider_setting("Trigger point", &settings::Settings::trigger, 0.05f);
    slider_setting("Camera speed", &settings::Settings::camera_speed, 10);
    slider_setting("Aim speed", &settings::Settings::aim_speed, 5);
    slider_setting("Mouse sensitivity", &settings::Settings::mouse_sensitivity, 0.01f);
    slider_setting("Controls opacity", &settings::Settings::touch_opacity, 0.05f);
    slider_setting("Controls size", &settings::Settings::touch_size, 0.05f);
    slider_setting("Touch camera speed", &settings::Settings::touch_camera_speed, 10);
    const auto original_camera = settings::current().right_stick;
    change_video("Camera stick", ImGuiKey_RightArrow);
    expect(settings::current().right_stick == settings::RightStick::DPad, "camera selector advances to D-pad mode");
    slider_setting("Camera stick D-pad point", &settings::Settings::right_stick_zone, 0.05f);
    change_video("Camera stick", ImGuiKey_LeftArrow);
    expect(settings::current().right_stick == original_camera, "camera selector round trip restores camera mode");
    toggle_setting("D-pad", &settings::Settings::touch_dpad);
    const auto original_layout = settings::current().touch_layout;
    change_video("Layout", ImGuiKey_RightArrow);
    expect(settings::current().touch_layout == settings::TouchLayout::Action, "touch selector activates Action layout");
    toggle_setting("Haptic feedback", &settings::Settings::touch_haptics);
    change_video("Layout", ImGuiKey_LeftArrow);
    expect(settings::current().touch_layout == original_layout, "touch layout round trip restores PSP controls");
    change_video("Free camera", ImGuiKey_Space);
    expect(settings::current().free_camera, "experimental free-camera settings enable dependent rows");
    slider_setting("Free camera speed", &settings::Settings::free_camera_speed, 50);
    toggle_setting("Hide the HUD while flying", &settings::Settings::free_camera_hide_hud);
    change_video("Free camera", ImGuiKey_Space);
    expect(!settings::current().free_camera, "experimental free-camera setting restores disabled mode");

    page();
    const bool original_tracing = adhoc::Client::tracing();
    change_video("Log every call and packet", ImGuiKey_Space);
    expect(adhoc::Client::tracing() != original_tracing, "network menu enables actual diagnostic tracing");
    change_video("Log every call and packet", ImGuiKey_Space);
    expect(adhoc::Client::tracing() == original_tracing, "network menu restores diagnostic tracing");
    const auto log_directory = install::user_data_directory() / "logs";
    std::filesystem::remove_all(log_directory);
    {
        std::ofstream blocked(log_directory);
        blocked << "Not a directory";
    }
    change_video("Save network log", ImGuiKey_Space);
    expect(std::filesystem::is_regular_file(log_directory), "network log failure preserves the blocking file");
    std::filesystem::remove(log_directory);
    adhoc::Client::log("Public synthetic diagnostic marker", false);
    for (int attempt = 0; attempt < 2; ++attempt) change_video("Save network log", ImGuiKey_Space);
    std::size_t logs = 0;
    if (std::filesystem::is_directory(log_directory)) {
        for (const auto &entry : std::filesystem::directory_iterator(log_directory)) {
            std::ifstream input(entry.path());
            const std::string contents{std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
            expect(contents.find("Yakumo ad hoc log") != std::string::npos &&
                    contents.find("state: off") != std::string::npos &&
                    contents.find("Public synthetic diagnostic marker") != std::string::npos,
                "network menu saves real diagnostics and buffered messages");
            ++logs;
        }
    }
    expect(logs == 2, "repeated network log saves retain both snapshots without overwriting");

    page();
    page();
    toggle_setting("Pause the game when the menu opens", &settings::Settings::menu_pause);
    toggle_setting("Pause during multiplayer", &settings::Settings::menu_pause_multiplayer);
    for (const char *action : {"Quit game", "Set up game data again…"}) {
        change_video(action, ImGuiKey_Space);
        frame();
        expect(ImGui::IsPopupOpen("##confirm", ImGuiPopupFlags_AnyPopupId),
            "destructive system action opens a confirmation before changing lifecycle");
        activate_confirmation("Cancel");
        expect(!ImGui::IsPopupOpen("##confirm", ImGuiPopupFlags_AnyPopupId) && ui::menu_over_game() &&
                !ui::take_quit_request(),
            "cancelling quit or setup keeps the menu and game lifecycle intact");
    }

    SDL_Event escape{};
    escape.type = SDL_EVENT_KEY_DOWN;
    escape.key.windowID = SDL_GetWindowID(renderer.window());
    escape.key.key = SDLK_ESCAPE;
    escape.key.scancode = SDL_SCANCODE_ESCAPE;
    escape.key.down = true;
    SDL_PushEvent(&escape);
    renderer.pump_events();
    SDL_Delay(110);
    frame();
    escape.type = SDL_EVENT_KEY_UP;
    escape.key.down = false;
    SDL_PushEvent(&escape);
    renderer.pump_events();
    frame();
    expect(!ui::menu_over_game() && !ui::take_quit_request(), "keyboard escape resumes game and closes menu");
    ui::set_lock_on_marker(std::array<float, 2>{0.5f, 0.5f});
    ui::show_note("Synthetic status note");
    frame();
    ui::set_lock_on_marker(std::nullopt);
    auto &layer = ui::Layer::get();
    layer.set_interactive(true);
    ui::open_touch_editor();
    expect(ui::touch_editor_open(), "touch layout editor opens");
    layer.begin_frame();
    ui::touch_editor_frame(false);
    layer.end_frame();
    renderer.present_ui(true);
    layer.begin_frame();
    ui::touch_editor_frame(true);
    layer.end_frame();
    renderer.present_ui(true);
    expect(!ui::touch_editor_open(), "back closes touch editor");
    layer.set_interactive(false);
}

}
int run_contracts() {
    const auto sandbox = std::filesystem::temp_directory_path() /
        ("yakumo-renderer-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    std::filesystem::create_directories(sandbox);
    install::set_data_directory_override(sandbox);
    SDL_setenv_unsafe("MHP3RD_FIND_CAMERA", "1", 1);
    SDL_setenv_unsafe("MHP3RD_POKE_FLOAT", "0x08000120:1.25,0x08000124:-2.5,broken", 1);
    SDL_setenv_unsafe("MHP3RD_FIND_FLOAT", "6.25", 1);
    SDL_setenv_unsafe("MHP3RD_POKE_FOUND", "6.5", 1);
    SDL_setenv_unsafe("MHP3RD_FIND_INT32", "1150", 1);
    SDL_setenv_unsafe("MHP3RD_POKE_INT32", "-7", 1);
    SDL_setenv_unsafe("MHP3RD_POKE_WHICH", "all", 1);
    SDL_setenv_unsafe("MHP3RD_FIND_STEP", "1150", 1);

    SDL_setenv_unsafe("MHP3RD_FIND_CAMERA_OUT", (sandbox / "camera-candidates.txt").string().c_str(), 1);
    auto &settings = settings::current();
    settings.internal_scale = 1;
    settings.window_scale = 1;
    settings.fullscreen = false;
    settings.frame_rate = settings::FrameRate::Fps30;
    SDL_SetHint(SDL_HINT_AUDIO_DRIVER, "dummy");
    SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");
    MediaFixture fixture;
    auto *selected = active_renderer();
    if (!selected) {
        std::cerr << "Renderer unavailable\n";
        std::filesystem::remove_all(sandbox);
        return 1;
    }
    auto &renderer = *selected;
    if (const char *compat = std::getenv("MHP3RD_GPU_COMPAT"); compat && std::string_view(compat) == "on")
        expect(renderer.gpu_compat_status().rfind("On", 0) == 0,
            "compatibility variant initializes the actual conservative renderer path");
    auto &diagnostic_memory = fixture.runtime.memory();
    diagnostic_memory.store32(0x08000140, std::bit_cast<std::uint32_t>(6.25f));
    diagnostic_memory.store32(0x08000144, std::bit_cast<std::uint32_t>(6.25f));
    diagnostic_memory.store32(0x08000160, 1150);
    diagnostic_memory.store32(0x08000164, 1150);
    media_renderer_contracts(fixture, renderer);
    render_contracts(renderer);
    primitive_contracts(renderer);
    screenshot_contracts(renderer, sandbox);
    held_frame_contracts(renderer, sandbox);
    camera_probe_contracts(renderer, sandbox);
    free_camera_lifecycle_contracts();
    keyboard_contracts(renderer);
    input_capture_contracts(renderer);
    widget_and_browser_contracts(renderer, sandbox);
    file_browser_boundary_contracts(renderer, sandbox);
    focused_widget_contracts(renderer);
    menu_contracts(renderer);
    setup_screen_contracts(renderer, sandbox);
    texture_pack_screen_contracts(renderer, sandbox);
    save_screen_contracts(renderer, sandbox);
    mods_screen_contracts(renderer, sandbox, fixture.runtime);
    bindings_editor_contracts(renderer);
    touch_editor_contracts(renderer);
    touch_event_contracts(renderer);
    scripted_mouse_contracts(renderer);
    virtual_gamepad_contracts(renderer);
    audio_device_contracts();
    renderer.shutdown();
    std::filesystem::remove_all(sandbox);
    std::cout << (failures ? "FAIL" : "PASS") << ": renderer/UI (" << failures << " failures)\n";
    return failures ? 1 : 0;
}

int main() {
    try {
        return run_contracts();
    } catch (const std::exception &error) {
        std::cerr << "FAIL: test fixture exception: " << error.what() << '\n';
        return 1;
    }
}
