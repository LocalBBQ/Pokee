// Real Vulkan/SDL/ImGui contracts with public synthetic buffers, no game data.
#include "gpu/vulkan_renderer.hpp"
#include "hle/hle_common.hpp"
#include "audio/audio_sink.hpp"
#include "settings/settings.hpp"
#include "ui/layer.hpp"
#include "ui/text_input.hpp"
#include "ui/widgets.hpp"
#include "ui/file_browser.hpp"
#include "install/user_data.hpp"
#include "ui/ui.hpp"
#include "ui/bindings_editor.hpp"
#include "ui/touch_editor.hpp"
#include "ui/touch_overlay.hpp"
#include "imgui_internal.h"
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
int main() {
    const auto sandbox = std::filesystem::temp_directory_path() /
        ("yakumo-renderer-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    std::filesystem::create_directories(sandbox);
    install::set_data_directory_override(sandbox);
    auto &settings = settings::current();
    settings.internal_scale = 1;
    settings.window_scale = 1;
    settings.fullscreen = false;
    settings.frame_rate = settings::FrameRate::Fps30;
    SDL_SetHint(SDL_HINT_AUDIO_DRIVER, "dummy");
    MediaFixture fixture;
    auto *selected = active_renderer();
    if (!selected) {
        std::cerr << "Renderer unavailable\n";
        std::filesystem::remove_all(sandbox);
        return 1;
    }
    auto &renderer = *selected;
    media_renderer_contracts(fixture, renderer);
    render_contracts(renderer);
    primitive_contracts(renderer);
    keyboard_contracts(renderer);
    input_capture_contracts(renderer);
    widget_and_browser_contracts(renderer, sandbox);
    menu_contracts(renderer);
    audio_device_contracts();
    renderer.shutdown();
    std::filesystem::remove_all(sandbox);
    std::cout << (failures ? "FAIL" : "PASS") << ": renderer/UI (" << failures << " failures)\n";
    return failures ? 1 : 0;
}
