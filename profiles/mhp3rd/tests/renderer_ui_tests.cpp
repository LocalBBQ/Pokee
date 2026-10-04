// Real Vulkan/SDL/ImGui contracts with public synthetic buffers, no game data.
#include "gpu/vulkan_renderer.hpp"
#include "camera_probe.hpp"
#include "mods/mhp3rd_mods.hpp"
#include "mods/mhp3rd_data_bin.hpp"
#include "kernel/iso_image.hpp"
#include "ui/mods_screen.hpp"
#include "ui/texture_pack_screen.hpp"
#include <algorithm>
#include <cmath>
#include <sstream>
#include "hle/hle_common.hpp"
#include "audio/audio_sink.hpp"
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
    draw.depth.test_enabled = true;
    for (std::uint32_t function : {0u, 1u}) {
        draw.depth.function = function;
        renderer.begin_frame();
        renderer.submit(clear, memory);
        renderer.submit(draw, memory);
        renderer.present(0x04000000);
        expect(
            (pixel() & 0xffffff) == (function == 1 ? 0x2255ccu : 0u), "depth never/always governs fragment visibility");
    }
    draw.depth.test_enabled = false;

    // Feed the PSP byte layout directly to the shader vertex decoder.
    std::array<std::uint32_t, 12> raw{};
    const std::array<std::array<float, 3>, 3> positions{{{-0.8f, -0.8f, 0}, {0.8f, -0.8f, 0}, {0, 0.8f, 0}}};
    for (std::size_t i = 0; i < positions.size(); ++i) {
        raw[i * 4] = 0xff2255cc;
        for (std::size_t axis = 0; axis < 3; ++axis)
            raw[i * 4 + axis + 1] = std::bit_cast<std::uint32_t>(positions[i][axis]);
    }
    draw.vertices.clear();
    draw.raw_vertices = reinterpret_cast<const std::uint8_t *>(raw.data());
    draw.raw_count = 3;
    draw.raw_stride = 16;
    draw.vertex_type = (7u << 2) | (3u << 7);
    renderer.begin_frame();
    renderer.submit(clear, memory);
    renderer.submit(draw, memory);
    renderer.present(0x04000000);
    expect(pixel() == 0xff2255cc, "raw PSP vertices decode color and float positions on the GPU");
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
    settings::current().texture_pack = false;
    renderer.set_texture_pack(false);
    layer.set_interactive(false);
}

void mods_screen_contracts(gpu::VulkanRenderer &renderer, const std::filesystem::path &sandbox) {
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
    press(SDL_GAMEPAD_BUTTON_DPAD_DOWN);
    press(SDL_GAMEPAD_BUTTON_DPAD_UP);
    press(confirm); // !
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
    auto &diagnostic_memory = fixture.runtime.memory();
    diagnostic_memory.store32(0x08000140, std::bit_cast<std::uint32_t>(6.25f));
    diagnostic_memory.store32(0x08000144, std::bit_cast<std::uint32_t>(6.25f));
    diagnostic_memory.store32(0x08000160, 1150);
    diagnostic_memory.store32(0x08000164, 1150);
    media_renderer_contracts(fixture, renderer);
    render_contracts(renderer);
    primitive_contracts(renderer);
    camera_probe_contracts(renderer, sandbox);
    keyboard_contracts(renderer);
    input_capture_contracts(renderer);
    widget_and_browser_contracts(renderer, sandbox);
    focused_widget_contracts(renderer);
    menu_contracts(renderer);
    setup_screen_contracts(renderer, sandbox);
    texture_pack_screen_contracts(renderer, sandbox);
    mods_screen_contracts(renderer, sandbox);
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
