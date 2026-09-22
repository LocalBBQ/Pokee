#pragma once

#include <cstdint>
#include <string>
#include <vector>

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
// What LT/RT (L2/R2) press past the trigger point. Standard makes them L and
// R like the shoulders; the other two move R onto L2 and put a weapon's attack
// on R2 for shooting: triangle for a bow, circle for a bowgun. The buttons
// they copy keep working.
enum class TriggerProfile { Standard, Bows, Bowguns };
// What answers the game when it asks for text such as the hunter's name.
enum class NameEntry { Keyboard, Fixed };

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

    // Text
    std::string font;                  // the game's text font: path, "#face" for a collection; empty: the default
    std::uint32_t font_weight{1u};     // columns the game's glyphs are thickened by, 0 to kMaxFontWeight

    // Audio
    std::uint32_t volume{100u};        // percent
    bool mute{};

    // Controls
    bool confirm_south{};              // confirm (circle) on the south face button
    float dead_zone{0.15f};
    float trigger{0.25f};
    TriggerProfile trigger_profile{TriggerProfile::Standard};
    RightStick right_stick{RightStick::Camera};
    float right_stick_zone{0.5f};
    // Drives the ordinary quest camera's yaw and pitch from how far the stick
    // is pushed, instead of the game's fixed-speed turn and vertical presets.
    // On by default. Off writes nothing at all, so the camera is exactly as
    // the game made it.
    bool analog_camera{true};
    // Degrees per second at full deflection, before the stick's own curve.
    float camera_speed{190.0f};
    // Degrees per second at full deflection while a bow or a bowgun aims.
    float aim_speed{90.0f};
    bool invert_camera_x{};
    bool invert_camera_y{};
    NameEntry name_entry{NameEntry::Keyboard};  // on-screen keyboard, or the name below at once
    std::string name{"Hunter"};        // the fixed name

    // Network (ad hoc play through a PSP ad hoc server)
    bool adhoc{};                      // wireless switch on: the game may go on line
    std::string adhoc_server;          // host or host:port of the server; empty: none
    std::string adhoc_nickname;        // shown to other players; empty: the hunter name
    std::string adhoc_mac;             // this player's virtual MAC, made up on first use
    std::vector<std::string> adhoc_recent;  // sessions joined lately, the latest first
    std::uint32_t adhoc_host_port{27312};   // the built-in server's adhocctl port; the relay is on the next

    // Interface
    bool menu_pause{true};             // opening the menu pauses the game
    bool menu_pause_multiplayer{};     // ...also during ad hoc play, where a paused game stops answering its peers
    bool menu_hint_seen{};             // the "Esc / L3+R3 opens the menu" hint was shown
    std::string last_folder;           // where the setup's file browser was last used

    // Saves
    bool backup_timestamp{true};       // a backup made from the menu goes to a new folder named by its time
};

inline constexpr std::uint32_t kMaxInternalScale = 8u;
inline constexpr std::uint32_t kMaxWindowScale = 4u;
inline constexpr std::uint32_t kMaxFontWeight = 2u;

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
