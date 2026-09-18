#include "ui/layer.hpp"

#include "ui/input_script.hpp"
#include "ui/widgets.hpp"

#include "gpu/vulkan_renderer.hpp"
#include "install/user_data.hpp"
#include "settings/settings.hpp"

#include "backends/imgui_impl_sdl3.h"
#include "imgui.h"

#include <SDL3/SDL.h>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <thread>
#include <utility>

namespace mhp3rd::ui {
namespace {

// How long an Esc waits for a gamepad press that would mark it as sent by
// Steam's controller layout rather than by a keyboard.
constexpr auto kEscapeWindow = std::chrono::milliseconds(100);

// Interface frames are presented with the game's present mode; without vsync
// they would spin, so they are held to about 120 per second.
constexpr auto kMinFrameTime = std::chrono::microseconds(8'333);

// Text faces with Latin and Cyrillic, then a Japanese face merged in for file
// names. The first one found is used.
const char *const kTextFonts[] = {
    "/System/Library/Fonts/SFNS.ttf",
    "/System/Library/Fonts/Helvetica.ttc",
    "/usr/share/fonts/truetype/noto/NotoSans-Regular.ttf",
    "/usr/share/fonts/noto/NotoSans-Regular.ttf",
    "/usr/share/fonts/google-noto/NotoSans-Regular.ttf",
    "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
    "/usr/share/fonts/TTF/DejaVuSans.ttf",
    "/usr/share/fonts/dejavu/DejaVuSans.ttf",
    "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
    "C:/Windows/Fonts/segoeui.ttf",
    "C:/Windows/Fonts/arial.ttf",
};
const char *const kJapaneseFonts[] = {
    "/System/Library/Fonts/ヒラギノ角ゴシック W4.ttc",
    "/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc",
    "/usr/share/fonts/noto-cjk/NotoSansCJK-Regular.ttc",
    "/usr/share/fonts/google-noto-cjk/NotoSansCJK-Regular.ttc",
    "C:/Windows/Fonts/meiryo.ttc",
    "C:/Windows/Fonts/msgothic.ttc",
};

bool exists(const char *path) {
    std::error_code ec;
    return std::filesystem::is_regular_file(install::path_from_utf8(path), ec);
}

void load_fonts() {
    ImGuiIO &io = ImGui::GetIO();
    const char *text_font = std::getenv("MHP3RD_UI_FONT");
    if (text_font != nullptr && !exists(text_font)) {
        std::cout << "[ui] MHP3RD_UI_FONT " << text_font << " not found\n";
        text_font = nullptr;
    }
    for (const char *candidate : kTextFonts) {
        if (text_font != nullptr) break;
        if (exists(candidate)) text_font = candidate;
    }
    ImFont *font = text_font != nullptr ? io.Fonts->AddFontFromFileTTF(text_font) : nullptr;
    if (font == nullptr) {
        io.Fonts->AddFontDefaultVector();
        std::cout << "[ui] no system font found; using Dear ImGui's own\n";
        return;
    }
    for (const char *candidate : kJapaneseFonts) {
        if (!exists(candidate)) continue;
        ImFontConfig merge;
        merge.MergeMode = true;
        io.Fonts->AddFontFromFileTTF(candidate, 0.0f, &merge);
        break;
    }
}

bool face_button_held() {
    int count = 0;
    SDL_JoystickID *ids = SDL_GetGamepads(&count);
    bool held = false;
    for (int i = 0; ids != nullptr && i < count && !held; ++i) {
        SDL_Gamepad *pad = SDL_GetGamepadFromID(ids[i]);
        if (pad == nullptr) continue;
        for (SDL_GamepadButton button : {SDL_GAMEPAD_BUTTON_SOUTH, SDL_GAMEPAD_BUTTON_EAST, SDL_GAMEPAD_BUTTON_WEST,
                                         SDL_GAMEPAD_BUTTON_NORTH, SDL_GAMEPAD_BUTTON_START})
            held = held || SDL_GetGamepadButton(pad, button);
    }
    SDL_free(ids);
    return held;
}

} // namespace

Layer &Layer::get() {
    static Layer layer;
    return layer;
}

bool Layer::attach(gpu::VulkanRenderer &renderer) {
    if (renderer_ != nullptr) return true;
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.LogFilename = nullptr;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_NavEnableGamepad;
    // Keep the focused row highlighted: on a gamepad there is no pointer.
    io.ConfigNavCursorVisibleAlways = true;
    io.ConfigNavEscapeClearFocusItem = false;
    if (!ImGui_ImplSDL3_InitForVulkan(renderer.window())) {
        std::cout << "[ui] ImGui_ImplSDL3_InitForVulkan failed; no menu\n";
        ImGui::DestroyContext();
        return false;
    }
    // Any connected pad drives the interface, not only the one the game reads.
    ImGui_ImplSDL3_SetGamepadMode(ImGui_ImplSDL3_GamepadMode_AutoAll);
    load_fonts();
    std::string error;
    if (!renderer.initialize_ui(error)) {
        std::cout << "[ui] cannot draw the interface (" << error << "); no menu\n";
        ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();
        return false;
    }
    renderer.set_event_hook([this](const SDL_Event &event) { return handle_event(event); });
    renderer_ = &renderer;
    if (renderer.gamepad() != nullptr) device_ = InputDevice::Gamepad;
    script::attach();
    return true;
}

void Layer::set_interactive(bool interactive) {
    if (interactive == interactive_) return;
    interactive_ = interactive;
    ImGuiIO &io = ImGui::GetIO();
    // Nothing typed while the game ran is replayed into a screen, and nothing
    // held when a screen closes stays held for the next one.
    io.ClearEventsQueue();
    io.ClearInputKeys();
    menu_toggle_ = false;
    back_ = false;
    escape_pending_.reset();
    gamepad_armed_ = false;
}

bool Layer::confirm_south() const { return settings::current().confirm_south; }

bool Layer::handle_event(const SDL_Event &event) {
    const Clock::time_point now = Clock::now();
    switch (event.type) {
    case SDL_EVENT_QUIT:
    case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
        window_closed_ = true;
        break;
    case SDL_EVENT_KEY_DOWN:
    case SDL_EVENT_KEY_UP:
        if (event.key.key == SDLK_ESCAPE) {
            // The game's own text entry takes Esc as "cancel".
            if (!interactive_ && renderer_->text_input_active()) return false;
            if (event.type == SDL_EVENT_KEY_DOWN && !event.key.repeat) escape_pending_ = now;
            return true;
        }
        if (event.type == SDL_EVENT_KEY_DOWN) device_ = InputDevice::Keyboard;
        break;
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
    case SDL_EVENT_MOUSE_WHEEL:
        device_ = InputDevice::Keyboard;
        break;
    case SDL_EVENT_GAMEPAD_BUTTON_DOWN: {
        device_ = InputDevice::Gamepad;
        last_pad_button_ = now;
        // A Steam Esc that came first is dropped here.
        if (escape_pending_ && now - *escape_pending_ < kEscapeWindow) escape_pending_.reset();
        const auto button = static_cast<SDL_GamepadButton>(event.gbutton.button);
        if (button == SDL_GAMEPAD_BUTTON_LEFT_STICK || button == SDL_GAMEPAD_BUTTON_RIGHT_STICK) {
            SDL_Gamepad *pad = SDL_GetGamepadFromID(event.gbutton.which);
            const SDL_GamepadButton other = button == SDL_GAMEPAD_BUTTON_LEFT_STICK ? SDL_GAMEPAD_BUTTON_RIGHT_STICK
                                                                                     : SDL_GAMEPAD_BUTTON_LEFT_STICK;
            if (pad != nullptr && SDL_GetGamepadButton(pad, other)) menu_toggle_ = true;
        }
        break;
    }
    case SDL_EVENT_GAMEPAD_AXIS_MOTION:
        if (std::abs(static_cast<int>(event.gaxis.value)) > 16000) device_ = InputDevice::Gamepad;
        break;
    case SDL_EVENT_DROP_FILE:
        if (event.drop.data != nullptr) dropped_ = install::path_from_utf8(event.drop.data);
        return true;
    default: break;
    }
    if (!interactive_) return false;
    ImGui_ImplSDL3_ProcessEvent(&event);
    return true;
}

void Layer::resolve_escape() {
    if (!escape_pending_) return;
    const Clock::time_point now = Clock::now();
    if (now - *escape_pending_ < kEscapeWindow) return;
    const bool from_pad = *escape_pending_ - last_pad_button_ < kEscapeWindow;
    escape_pending_.reset();
    if (from_pad) return;
    if (interactive_) back_ = true;
    else menu_toggle_ = true;
}

bool Layer::take_menu_toggle() {
    resolve_escape();
    return std::exchange(menu_toggle_, false);
}

bool Layer::take_back() {
    resolve_escape();
    return std::exchange(back_, false);
}

std::optional<std::filesystem::path> Layer::take_dropped_file() { return std::exchange(dropped_, std::nullopt); }

void Layer::apply_theme() {
    const ImGuiIO &io = ImGui::GetIO();
    // About 27 px on a Steam Deck's 800 lines, 18 px in the default 544-line
    // window, growing with larger windows.
    const float size = std::clamp(std::round(io.DisplaySize.y * 0.034f), 16.0f, 72.0f);
    if (size == font_size_) return;
    font_size_ = size;
    ImGui::GetStyle() = make_style(scale(), font_size_);
}

void Layer::begin_frame() {
    renderer_->begin_ui_frame();
    ImGui_ImplSDL3_NewFrame();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigNavSwapGamepadButtons = !confirm_south();
    if (!gamepad_armed_) gamepad_armed_ = !face_button_held();
    if (gamepad_armed_) io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    else io.ConfigFlags &= ~ImGuiConfigFlags_NavEnableGamepad;
    apply_theme();
    ImGui::NewFrame();
    description_.clear();
}

void Layer::end_frame() {
    ImGui::Render();
    renderer_->set_ui_draw_data(ImGui::GetDrawData());
}

bool Layer::run(const std::function<bool()> &frame, bool show_game) {
    for (;;) {
        const Clock::time_point start = Clock::now();
        script::tick();
        if (!renderer_->pump_events()) window_closed_ = true;
        if (window_closed_) return false;
        begin_frame();
        const bool keep_going = frame();
        end_frame();
        renderer_->present_ui(show_game);
        if (!keep_going) return true;
        const Clock::duration spent = Clock::now() - start;
        if (spent < kMinFrameTime) std::this_thread::sleep_for(kMinFrameTime - spent);
    }
}

} // namespace mhp3rd::ui
