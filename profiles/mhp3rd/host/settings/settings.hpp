#pragma once

#include <cstdint>
#include <string>

// The player's settings: what the in-game menu changes and settings.ini in the
// per-user data directory keeps.
//
// A value comes from its environment variable when that is set, otherwise from
// settings.ini, otherwise from the default below. A variable decides the value
// for the whole run: the menu shows it but cannot change it, and it is never
// written to settings.ini, so unsetting the variable brings the player's own
// choice back.
//
// Only the main thread reads or writes these.
namespace mhp3rd::settings {

enum class PresentMode { Fifo, Mailbox, Immediate };
enum class PerfDisplay { Off, Overlay, OverlayAndLog, Log };
enum class RightStick { Camera, DPad, Off };

struct Settings {
    // Video
    std::uint32_t internal_scale{2u};  // render resolution, multiples of 480x272
    std::uint32_t window_scale{2u};    // windowed size, multiples of 480x272
    bool fullscreen{};
    PresentMode present_mode{PresentMode::Fifo};
    bool keep_aspect{true};            // letterbox rather than stretch to the window
    bool sharp_screen{};               // nearest instead of linear scaling to the window
    bool sharp_textures{};             // nearest instead of linear texture sampling
    bool unthrottled{};                // let emulated time run ahead of real time
    PerfDisplay perf{PerfDisplay::Off};

    // Audio
    std::uint32_t volume{100u};        // percent
    bool mute{};

    // Controls
    bool confirm_south{};              // confirm (circle) on the south face button
    float dead_zone{0.15f};
    float trigger{0.25f};
    RightStick right_stick{RightStick::Camera};
    float right_stick_zone{0.5f};
    bool invert_camera_x{};
    bool invert_camera_y{};
    bool type_name{};                  // type the name when the game asks for one
    std::string name{"Hunter"};        // otherwise answer with this

    // Interface
    bool menu_hint_seen{};             // the "Esc / L3+R3 opens the menu" hint was shown
    std::string last_folder;           // where the setup's file browser was last used
};

inline constexpr std::uint32_t kMaxInternalScale = 8u;
inline constexpr std::uint32_t kMaxWindowScale = 4u;

// Loads the settings on first use.
[[nodiscard]] Settings &current();
// The defaults, for "Restore defaults".
[[nodiscard]] const Settings &defaults();
// Writes current() to settings.ini, leaving values set by environment
// variables at what the file had. Failures are reported on the console.
void save();

// The environment variable that decides the setting stored under `key`
// (for example "video.internal_scale") for this run, or null.
[[nodiscard]] const char *overridden_by(const char *key);

} // namespace mhp3rd::settings
