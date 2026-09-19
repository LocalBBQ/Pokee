// The in-game menu: Esc, or L3+R3 on a gamepad. It pauses the game and holds
// every player setting; each change applies at once and is saved to
// settings.ini straight away.

#include "ui/ui.hpp"

#include "ui/font_menu.hpp"
#include "ui/input_script.hpp"
#include "ui/layer.hpp"
#include "ui/widgets.hpp"

#include "audio/audio_sink.hpp"
#include "gpu/vulkan_renderer.hpp"
#include "install/game_identity.hpp"
#include "install/installer.hpp"
#include "install/user_data.hpp"
#include "perf/frame_stats.hpp"
#include "settings/settings.hpp"
#include "yakumo_version.hpp"

#include "imgui.h"

#include <SDL3/SDL.h>

#include <algorithm>
#include <array>
#include <cctype>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

namespace mhp3rd::ui {
namespace {

using Clock = std::chrono::steady_clock;

// Seconds the "how to open the menu" hint stays up at start, until the menu
// has been opened once.
constexpr double kHintSeconds = 12.0;
// Resolutions the menu offers. MHP3RD_INTERNAL_SCALE goes up to 8, but a
// setting that runs out of video memory would fail on every start.
constexpr int kMenuMaxInternalScale = 6;

std::string size_text(std::uint32_t scale) {
    return std::to_string(480u * scale) + "×" + std::to_string(272u * scale);
}

// Row options for the setting stored under `key`: when an environment
// variable decides it for this run, the row is shown but locked.
RowOptions options_for(const char *key, std::string description) {
    RowOptions options;
    options.description = std::move(description);
    if (const char *variable = settings::overridden_by(key)) {
        options.disabled = true;
        options.note = std::string("Set by ") + variable;
    }
    return options;
}

float font_gap() { return Layer::get().font_size() * 0.5f; }

int cycle(int value, int delta, int count) { return ((value + delta) % count + count) % count; }

// Percent-encodes a path for a file:// URL.
std::string file_url(const std::string &path) {
    std::string url = "file://";
    for (const unsigned char c : path) {
        if (std::isalnum(c) != 0 || c == '/' || c == '-' || c == '_' || c == '.' || c == '~') {
            url += static_cast<char>(c);
        } else {
            char escaped[4];
            std::snprintf(escaped, sizeof(escaped), "%%%02X", c);
            url += escaped;
        }
    }
    return url;
}

float gain(const settings::Settings &s) { return s.mute ? 0.0f : static_cast<float>(s.volume) / 100.0f; }

class Menu {
public:
    // One frame; false once the menu closes.
    bool frame();
    [[nodiscard]] bool quit() const noexcept { return quit_; }

private:
    enum class Confirm { None, Quit, Setup };

    void video();
    void audio();
    void controls();
    void system();
    bool confirm_dialog();
    void name_row();

    gpu::VulkanRenderer &renderer() { return Layer::get().renderer(); }

    int tab_{};
    bool first_frame_{true};
    bool close_{};
    bool quit_{};
    bool was_editing_{};
    bool back_{};  // the back button was pressed this frame
    Confirm confirm_{Confirm::None};
    bool confirm_opened_{};  // the confirmation was on screen last frame
};

bool Menu::frame() {
    Layer &layer = Layer::get();
    if (layer.take_menu_toggle()) return false;
    const bool back = layer.take_back();
    // The pad's back button closes the menu too, once nothing is being edited.
    const ImGuiKey cancel = layer.confirm_south() ? ImGuiKey_GamepadFaceRight : ImGuiKey_GamepadFaceDown;
    const bool pad_back = ImGui::IsKeyPressed(cancel, false);
    const bool start = ImGui::IsKeyPressed(ImGuiKey_GamepadStart, false);
    // Back closes the font list before it closes the menu.
    const bool font_list_was_open = tab_ == 0 && font_list_open();
    back_ = back || pad_back;

    begin_panel("##menu", "Yakumo", "Paused", true);
    static const char *const kTabs[] = {"Video", "Audio", "Controls", "System"};
    const bool switched = tab_bar(kTabs, 4, tab_) || first_frame_;
    first_frame_ = false;
    begin_content();
    if (switched) {
        focus_next_row();
        ImGui::SetScrollY(0.0f);
    }
    switch (tab_) {
    case 0: video(); break;
    case 1: audio(); break;
    case 2: controls(); break;
    default: system(); break;
    }
    begin_footer();
    if (tab_ == 3)
        hints({{Control::Confirm, "Select"}, {Control::Back, "Back"}, {Control::Tabs, "Section"},
               {Control::Menu, "Resume"}});
    else
        hints({{Control::Confirm, "Select"},
               {Control::Change, "Change"},
               {Control::Back, "Back"},
               {Control::Tabs, "Section"},
               {Control::Menu, "Resume"}});
    end_panel();

    if (confirm_ != Confirm::None) {
        if ((back || pad_back) && confirm_opened_) confirm_ = Confirm::None;
    } else if (!font_list_was_open && (((back || pad_back) && !was_editing_) || start)) {
        close_ = true;
    }
    if (!confirm_dialog()) return false;
    was_editing_ = ImGui::IsAnyItemActive();
    return !close_;
}

void Menu::video() {
    if (font_list(back_)) return;
    settings::Settings &s = settings::current();
    section("Picture");
    {
        RowOptions o = options_for("video.internal_scale",
                                   "The game is drawn at a multiple of the PSP's 480×272. Higher is sharper and "
                                   "needs more from the GPU.");
        const std::string value = "×" + std::to_string(s.internal_scale) + "   " + size_text(s.internal_scale);
        if (const int delta = choice_row("Resolution", value, o)) {
            const int limit = std::max<int>(kMenuMaxInternalScale, static_cast<int>(s.internal_scale));
            s.internal_scale = static_cast<std::uint32_t>(cycle(static_cast<int>(s.internal_scale) - 1, delta, limit) + 1);
            renderer().set_internal_scale(s.internal_scale);
            settings::save();
        }
    }
    {
        const int delta =
            choice_row("Display", s.fullscreen ? "Fullscreen" : "Window",
                       options_for("video.fullscreen", "Fill the screen, or play in a window you can resize."));
        if (delta != 0) {
            s.fullscreen = !s.fullscreen;
            renderer().set_fullscreen(s.fullscreen);
            settings::save();
        }
    }
    {
        RowOptions o = options_for("video.window_scale", "The window's size, in multiples of the PSP's screen.");
        if (s.fullscreen) {
            o.disabled = true;
            o.note = "Fullscreen";
        }
        const std::string value = "×" + std::to_string(s.window_scale) + "   " + size_text(s.window_scale);
        if (const int delta = choice_row("Window size", value, o)) {
            s.window_scale = static_cast<std::uint32_t>(
                cycle(static_cast<int>(s.window_scale) - 1, delta, static_cast<int>(settings::kMaxWindowScale)) + 1);
            renderer().set_window_scale(s.window_scale);
            settings::save();
        }
    }
    if (choice_row("Aspect ratio", s.keep_aspect ? "Original" : "Stretch",
                   options_for("video.keep_aspect", "Original keeps the PSP's shape with black bars at the sides or "
                                                    "top; Stretch fills the window."))) {
        s.keep_aspect = !s.keep_aspect;
        renderer().set_keep_aspect(s.keep_aspect);
        settings::save();
    }
    if (choice_row("Scaling filter", s.sharp_screen ? "Sharp" : "Smooth",
                   options_for("video.sharp_screen", "How the finished picture is scaled to the window: smooth "
                                                     "(bilinear) or sharp (nearest pixel)."))) {
        s.sharp_screen = !s.sharp_screen;
        renderer().set_sharp_screen(s.sharp_screen);
        settings::save();
    }
    if (choice_row("Texture filter", s.sharp_textures ? "Sharp" : "Smooth",
                   options_for("video.sharp_textures", "How the game's textures are sampled: smooth (bilinear) or "
                                                       "sharp (nearest texel)."))) {
        s.sharp_textures = !s.sharp_textures;
        renderer().set_sharp_textures(s.sharp_textures);
        settings::save();
    }
    section("Timing");
    {
        struct Mode {
            settings::PresentMode mode;
            const char *name;
        };
        std::vector<Mode> modes{{settings::PresentMode::Fifo, "On"}};
        if (renderer().supports_present_mode(settings::PresentMode::Mailbox))
            modes.push_back({settings::PresentMode::Mailbox, "Off (mailbox)"});
        if (renderer().supports_present_mode(settings::PresentMode::Immediate))
            modes.push_back({settings::PresentMode::Immediate, "Off (immediate)"});
        int current = 0;
        for (std::size_t i = 0; i < modes.size(); ++i)
            if (modes[i].mode == s.present_mode) current = static_cast<int>(i);
        if (const int delta = choice_row(
                "Vsync", modes[static_cast<std::size_t>(current)].name,
                options_for("video.present_mode", "On waits for the display's refresh and never tears. Off shows "
                                                  "each frame at once; the game's speed is the same either way."))) {
            s.present_mode = modes[static_cast<std::size_t>(cycle(current, delta, static_cast<int>(modes.size())))].mode;
            renderer().set_present_mode(s.present_mode);
            settings::save();
        }
    }
    if (choice_row("Game speed", s.unthrottled ? "Unlimited" : "Normal",
                   options_for("video.unthrottled", "Normal holds the game to real time. Unlimited lets it run as "
                                                    "fast as frames can be drawn, which also speeds up the game."))) {
        s.unthrottled = !s.unthrottled;
        settings::save();
    }
    {
        static const char *const kPerf[] = {"Off", "Overlay", "Overlay and log", "Log only"};
        const int current = static_cast<int>(s.perf);
        if (const int delta = choice_row(
                "Performance", kPerf[current],
                options_for("video.performance", "Frame times and speed in the top-left corner, and a [perf] line "
                                                 "per second on the console. F3 shows or hides the overlay."))) {
            s.perf = static_cast<settings::PerfDisplay>(cycle(current, delta, 4));
            renderer().set_perf_overlay(perf::options().overlay);
            settings::save();
        }
    }
    font_rows();
    ImGui::Dummy({0.0f, font_gap()});
    if (button_row("Restore video defaults", {false, {}, "Every setting on this page back to how Yakumo ships."})) {
        const settings::Settings &d = settings::defaults();
        const auto restore = [&](const char *key, auto &value, const auto &fallback) {
            if (settings::overridden_by(key) == nullptr) value = fallback;
        };
        restore("video.internal_scale", s.internal_scale, d.internal_scale);
        restore("video.fullscreen", s.fullscreen, d.fullscreen);
        restore("video.window_scale", s.window_scale, d.window_scale);
        restore("video.keep_aspect", s.keep_aspect, d.keep_aspect);
        restore("video.sharp_screen", s.sharp_screen, d.sharp_screen);
        restore("video.sharp_textures", s.sharp_textures, d.sharp_textures);
        restore("video.present_mode", s.present_mode, d.present_mode);
        restore("video.unthrottled", s.unthrottled, d.unthrottled);
        restore("video.performance", s.perf, d.perf);
        renderer().set_internal_scale(s.internal_scale);
        renderer().set_fullscreen(s.fullscreen);
        renderer().set_window_scale(s.window_scale);
        renderer().set_keep_aspect(s.keep_aspect);
        renderer().set_sharp_screen(s.sharp_screen);
        renderer().set_sharp_textures(s.sharp_textures);
        renderer().set_present_mode(s.present_mode);
        renderer().set_perf_overlay(perf::options().overlay);
        settings::save();
    }
}

void Menu::audio() {
    settings::Settings &s = settings::current();
    audio::AudioSink &sink = audio::AudioSink::instance();
    const bool device = sink.has_device();
    const auto locked = [&](const char *key, const char *description) {
        RowOptions options = options_for(key, description);
        if (!device) {
            options.disabled = true;
            options.note = std::getenv("MHP3RD_NO_AUDIO") != nullptr ? "Off: MHP3RD_NO_AUDIO" : "No audio device";
        }
        return options;
    };
    section("Output");
    int volume = static_cast<int>(s.volume);
    if (slider_row("Volume", volume, 0, 100, 5, "%d%%",
                   locked("audio.volume", "Loudness of everything the game plays."))) {
        s.volume = static_cast<std::uint32_t>(volume);
        sink.set_volume(gain(s));
        settings::save();
    }
    if (toggle_row("Mute", s.mute, locked("audio.mute", "Silence the game without losing the volume setting."))) {
        s.mute = !s.mute;
        sink.set_volume(gain(s));
        settings::save();
    }
    info_row("Device", device ? "44100 Hz stereo, paused while this menu is open" : "None");
    ImGui::Dummy({0.0f, font_gap()});
    if (button_row("Restore audio defaults", {!device, {}, "Full volume, not muted."})) {
        s.volume = settings::defaults().volume;
        s.mute = settings::defaults().mute;
        sink.set_volume(gain(s));
        settings::save();
    }
}

void Menu::name_row() {
    settings::Settings &s = settings::current();
    RowOptions o = options_for("input.name", "The name given when the game asks for one and typing is off. Letters, "
                                             "digits and punctuation from a keyboard; on a Steam Deck, Steam+X opens "
                                             "the on-screen keyboard.");
    static char buffer[32] = {};
    static bool loaded = false;
    if (!loaded || !ImGui::IsAnyItemActive()) {
        std::snprintf(buffer, sizeof(buffer), "%s", s.name.c_str());
        loaded = true;
    }
    const float row_height = std::round(Layer::get().font_size() * 1.9f);
    const ImVec2 start = ImGui::GetCursorScreenPos();
    const float width = ImGui::GetContentRegionAvail().x;
    ImDrawList *draw = ImGui::GetWindowDrawList();
    // Highlighted like the other rows; the field reports its focus only after
    // it is drawn, so last frame's state is used.
    static bool focused = false;
    if (focused)
        draw->AddRectFilled(start, {start.x + width, start.y + row_height}, colors::kRowFocus,
                            std::round(6.0f * Layer::get().scale()));
    draw->AddText({start.x + std::round(16.0f * Layer::get().scale()),
                   start.y + (row_height - Layer::get().font_size()) * 0.5f},
                  o.disabled ? colors::kTextDisabled : colors::kText, "Hunter name");
    const float field_width = std::min(width * 0.45f, Layer::get().font_size() * 12.0f);
    ImGui::SetCursorScreenPos({start.x + width - field_width - std::round(16.0f * Layer::get().scale()),
                               start.y + (row_height - ImGui::GetFrameHeight()) * 0.5f});
    ImGui::SetNextItemWidth(field_width);
    if (o.disabled) ImGui::BeginDisabled();
    const auto printable = [](ImGuiInputTextCallbackData *data) {
        // The game stores the name as UTF-16 converted byte by byte.
        return data->EventChar >= 0x20 && data->EventChar < 0x7F ? 0 : 1;
    };
    ImGui::InputText("##name", buffer, 17, ImGuiInputTextFlags_CallbackCharFilter, printable);
    focused = ImGui::IsItemFocused();
    if (ImGui::IsItemFocused() || ImGui::IsItemHovered()) {
        std::string description = o.description;
        if (!o.note.empty()) description += "\n" + o.note;
        Layer::get().set_description(description);
    }
    if (ImGui::IsItemDeactivatedAfterEdit() && buffer[0] != '\0') {
        s.name = buffer;
        settings::save();
    }
    if (o.disabled) ImGui::EndDisabled();
    ImGui::SetCursorScreenPos({start.x, start.y + row_height});
    ImGui::Dummy({0.0f, 0.0f});
}

void Menu::controls() {
    settings::Settings &s = settings::current();
    section("Gamepad");
    {
        SDL_Gamepad *pad = renderer().gamepad();
        const char *name = pad != nullptr ? SDL_GetGamepadName(pad) : nullptr;
        info_row("Connected", pad == nullptr ? "No gamepad; the keyboard drives the game"
                                             : (name != nullptr ? name : "Gamepad"));
    }
    if (choice_row("Confirm button", s.confirm_south ? "Bottom (Western)" : "Right, ○ (Japanese)",
                   options_for("input.confirm", "Which face button confirms, in the game and in this menu. The "
                                                "game's prompts show ○ to confirm and × to go back."))) {
        s.confirm_south = !s.confirm_south;
        settings::save();
    }
    int dead_zone = static_cast<int>(std::lround(s.dead_zone * 100.0f));
    if (slider_row("Stick dead zone", dead_zone, 0, 50, 1, "%d%%",
                   options_for("input.dead_zone", "How far the left stick moves before the hunter does. Raise it if "
                                                  "the hunter drifts."))) {
        s.dead_zone = static_cast<float>(dead_zone) / 100.0f;
        settings::save();
    }
    int trigger = static_cast<int>(std::lround(s.trigger * 100.0f));
    if (slider_row("Trigger point", trigger, 5, 100, 5, "%d%%",
                   options_for("input.trigger", "How far LT/RT (L2/R2) travel before they press L/R."))) {
        s.trigger = static_cast<float>(trigger) / 100.0f;
        settings::save();
    }
    {
        static const char *const kModes[] = {"Camera", "D-pad", "Off"};
        const int current = static_cast<int>(s.right_stick);
        if (const int delta = choice_row(
                "Right stick", kModes[current],
                options_for("input.right_stick", "Camera uses the HD release's own right-stick camera. D-pad "
                                                 "presses the D-pad instead, like the PSP's camera controls."))) {
            s.right_stick = static_cast<settings::RightStick>(cycle(current, delta, 3));
            settings::save();
        }
    }
    const bool camera = s.right_stick == settings::RightStick::Camera;
    {
        RowOptions o = options_for("input.invert_camera_x", "Turn the camera the other way left and right.");
        if (!camera && !o.disabled) {
            o.disabled = true;
            o.note = "Right stick is not the camera";
        }
        if (toggle_row("Invert camera horizontally", s.invert_camera_x, o)) {
            s.invert_camera_x = !s.invert_camera_x;
            settings::save();
        }
        o = options_for("input.invert_camera_y", "Turn the camera the other way up and down.");
        if (!camera && !o.disabled) {
            o.disabled = true;
            o.note = "Right stick is not the camera";
        }
        if (toggle_row("Invert camera vertically", s.invert_camera_y, o)) {
            s.invert_camera_y = !s.invert_camera_y;
            settings::save();
        }
    }
    {
        RowOptions o = options_for("input.right_stick_zone", "How far the right stick moves before it presses the "
                                                             "D-pad.");
        if (s.right_stick != settings::RightStick::DPad && !o.disabled) {
            o.disabled = true;
            o.note = "Right stick is not the D-pad";
        }
        int zone = static_cast<int>(std::lround(s.right_stick_zone * 100.0f));
        if (slider_row("Right stick D-pad point", zone, 10, 100, 5, "%d%%", o)) {
            s.right_stick_zone = static_cast<float>(zone) / 100.0f;
            settings::save();
        }
    }

    section("Hunter name");
    if (choice_row("When the game asks for a name", s.type_name ? "Type it" : "Use the name below",
                   options_for("input.type_name", "Type it: the game waits while you type in the window; Enter "
                                                  "confirms, Esc cancels. Otherwise the name below is given at once."))) {
        s.type_name = !s.type_name;
        settings::save();
    }
    name_row();

    section("Keyboard");
    static const std::array<std::pair<const char *, const char *>, 10> kKeys{{
        {"Arrow keys", "D-pad"},
        {"I  J  K  L", "Analog stick"},
        {"X", "○  (confirm)"},
        {"Z", "×  (back)"},
        {"A", "□"},
        {"S", "△"},
        {"Q  /  W", "L  /  R"},
        {"Enter", "START"},
        {"Right Shift, Backspace", "SELECT"},
        {"Esc", "This menu"},
    }};
    for (const auto &[key, button] : kKeys) info_row(key, button);
    ImGui::Dummy({0.0f, font_gap()});
    if (button_row("Restore control defaults", {false, {}, "Every gamepad and name setting back to how Yakumo ships."})) {
        const settings::Settings &d = settings::defaults();
        const auto restore = [&](const char *key, auto &value, const auto &fallback) {
            if (settings::overridden_by(key) == nullptr) value = fallback;
        };
        restore("input.confirm", s.confirm_south, d.confirm_south);
        restore("input.dead_zone", s.dead_zone, d.dead_zone);
        restore("input.trigger", s.trigger, d.trigger);
        restore("input.right_stick", s.right_stick, d.right_stick);
        restore("input.right_stick_zone", s.right_stick_zone, d.right_stick_zone);
        restore("input.invert_camera_x", s.invert_camera_x, d.invert_camera_x);
        restore("input.invert_camera_y", s.invert_camera_y, d.invert_camera_y);
        restore("input.type_name", s.type_name, d.type_name);
        restore("input.name", s.name, d.name);
        settings::save();
    }
}

void Menu::system() {
    std::string data_dir;
    try {
        data_dir = install::path_to_utf8(install::user_data_directory());
    } catch (const std::exception &e) {
        data_dir = e.what();
    }
    section("Game");
    if (button_row("Resume", {false, {}, "Back to the game."})) close_ = true;
    if (button_row("Open the data folder", {false, {}, "Show Yakumo's data folder in the file manager."})) {
        if (!SDL_OpenURL(file_url(data_dir).c_str()))
            std::cout << "[menu] cannot open " << data_dir << ": " << SDL_GetError() << "\n";
    }
    if (button_row("Set up game data again…",
                   {false, {}, "Choose the disc image again, for example after moving it. The game closes first."}))
        confirm_ = Confirm::Setup;
    if (button_row("Quit game", {false, {}, "Close Yakumo. Progress since your last save is lost."},
                   colors::kDanger))
        confirm_ = Confirm::Quit;

    section("About");
    info_row("Yakumo", std::string(kYakumoVersion));
    info_row("Game", std::string(install::kGameTitle) + " (" + install::kDiscIdDisplay + ")");
    info_row("Data folder", data_dir);
    info_row("Graphics", "Vulkan on " + renderer().device_name());
    info_row("Interface", std::string("Dear ImGui ") + IMGUI_VERSION);
}

// The quit and setup confirmations. False when the menu should close.
bool Menu::confirm_dialog() {
    if (confirm_ != Confirm::None && !ImGui::IsPopupOpen("##confirm")) ImGui::OpenPopup("##confirm");
    const ImGuiIO &io = ImGui::GetIO();
    const float font = Layer::get().font_size();
    ImGui::SetNextWindowPos({io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f}, ImGuiCond_Always, {0.5f, 0.5f});
    ImGui::SetNextWindowSize({std::min(io.DisplaySize.x * 0.9f, font * 26.0f), 0.0f}, ImGuiCond_Always);
    bool keep_open = true;
    confirm_opened_ = false;
    if (ImGui::BeginPopupModal("##confirm", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings)) {
        if (confirm_ == Confirm::None) {
            ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
            return true;
        }
        confirm_opened_ = true;
        const bool quit = confirm_ == Confirm::Quit;
        heading(quit ? "Quit the game?" : "Set up game data again?");
        paragraph(quit ? "Progress since your last save is lost."
                       : "Yakumo closes the game and opens the setup, where you choose the disc image again. "
                         "Progress since your last save is lost.",
                  colors::kTextDim);
        ImGui::Dummy({0.0f, font * 0.6f});
        const float gap = font * 0.6f;
        const float width = (ImGui::GetContentRegionAvail().x - gap) * 0.5f;
        if (big_button(quit ? "Quit" : "Close and set up", width, true)) {
            if (!quit) install::request_setup_on_exit();
            quit_ = true;
            keep_open = false;
        }
        ImGui::SameLine(0.0f, gap);
        if (big_button("Cancel", width)) confirm_ = Confirm::None;
        // Cancel is the safe default.
        ImGui::SetItemDefaultFocus();
        ImGui::Dummy({0.0f, font * 0.2f});
        ImGui::Dummy({0.0f, 0.0f});
        hints({{Control::Confirm, "Select"}, {Control::Back, "Cancel"}});
        if (confirm_ == Confirm::None || !keep_open) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
    return keep_open;
}

// The hint shown over the game until the menu has been opened once.
void draw_hint() {
    static const Clock::time_point first_frame = Clock::now();
    const double seconds = std::chrono::duration<double>(Clock::now() - first_frame).count();
    if (seconds > kHintSeconds) return;
    Layer &layer = Layer::get();
    layer.begin_frame();
    const ImGuiIO &io = ImGui::GetIO();
    const float font = layer.font_size();
    const float alpha = static_cast<float>(std::clamp(kHintSeconds - seconds, 0.0, 1.0));
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {font * 0.8f, font * 0.5f});
    ImGui::SetNextWindowPos({io.DisplaySize.x * 0.5f, io.DisplaySize.y - font}, ImGuiCond_Always, {0.5f, 1.0f});
    ImGui::Begin("##hint", nullptr,
                 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_AlwaysAutoResize |
                     ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav);
    hints({{Control::Menu, "Settings and pause"}});
    ImGui::End();
    ImGui::PopStyleVar(2);
    layer.end_frame();
}

} // namespace

bool attach(gpu::VulkanRenderer &renderer) { return Layer::get().attach(renderer); }

void draw_over_game() {
    Layer &layer = Layer::get();
    if (!layer.attached()) return;
    script::tick();
    if (!settings::current().menu_hint_seen) draw_hint();
}

bool menu_requested() {
    Layer &layer = Layer::get();
    return layer.attached() && layer.take_menu_toggle();
}

bool run_menu() {
    Layer &layer = Layer::get();
    settings::Settings &s = settings::current();
    if (!s.menu_hint_seen) {
        s.menu_hint_seen = true;
        settings::save();
    }
    std::cout << "[menu] opened; the game is paused" << std::endl;
    const Clock::time_point opened = Clock::now();
    layer.renderer().set_game_input(false);
    layer.set_interactive(true);
    Menu menu;
    const bool window_open = layer.run([&] { return menu.frame(); }, true);
    layer.set_interactive(false);
    layer.renderer().set_game_input(true);
    const double seconds = std::chrono::duration<double>(Clock::now() - opened).count();
    char text[64];
    std::snprintf(text, sizeof(text), "%.1f", seconds);
    std::cout << "[menu] closed after " << text << " s" << (menu.quit() ? "; quitting" : "") << std::endl;
    return window_open && !menu.quit();
}

} // namespace mhp3rd::ui
