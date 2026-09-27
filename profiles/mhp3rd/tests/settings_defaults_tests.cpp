// Which defaults each platform starts from; no settings.ini is read.
#include "settings/settings.hpp"

#include <iostream>
#include <optional>
#include <string>

namespace {
using namespace mhp3rd::settings;

int failures{};

void check(bool condition, const char *message) {
    if (!condition) {
        ++failures;
        std::cerr << "FAIL: " << message << '\n';
    }
}

void test_desktop_is_the_declared_defaults() {
    const Settings desktop = defaults_for(Platform::Desktop);
    const Settings declared{};
    check(desktop.aspect == declared.aspect && desktop.fullscreen == declared.fullscreen &&
              desktop.mouse == declared.mouse && desktop.internal_scale == declared.internal_scale &&
              desktop.right_stick == declared.right_stick && desktop.analog_camera == declared.analog_camera,
          "the desktop defaults are Settings{} as declared");
    check(desktop.aspect == Aspect::Original && !desktop.fullscreen && desktop.mouse,
          "the desktop keeps the original aspect, a window and the mouse");
    check(desktop.fast_loading && !desktop.unthrottled, "loads run fast and the game keeps real time otherwise");
}

void test_android() {
    const Settings android = defaults_for(Platform::Android);
    check(android.aspect == Aspect::Fill, "a phone fills its wide screen");
    check(android.fullscreen, "a phone is full screen");
    check(!android.mouse, "a phone does not capture a mouse");
    check(android.analog_camera && android.right_stick == RightStick::Camera,
          "the analog camera is on, as the touch camera needs");
    check(android.touch_controls, "the touch controls are on");
    check(android.frame_rate == FrameRate::Fps30 && android.frame_rate_auto, "30 fps, lowered when behind");
    check(android.present_mode == PresentMode::Fifo && android.perf == PerfDisplay::Off,
          "vsync on and no performance overlay");
    check(android.fast_loading, "a phone loads fast too");
}

void test_this_build() {
    const Settings expected = defaults_for(kPlatform);
    check(defaults().aspect == expected.aspect && defaults().mouse == expected.mouse &&
              defaults().fullscreen == expected.fullscreen,
          "defaults() is this platform's set");
}

// Controls written by earlier versions, and presets through settings.ini.
void test_control_presets() {
    using namespace mhp3rd;
    Settings fresh = from_entries({});
    check(fresh.control_preset.shipped == input::Preset::Default && fresh.user_presets.empty() &&
              fresh.controls == input::layout(input::Preset::Default),
          "a new player starts on Default");

    // An earlier version's file: bindings as they were, no preset.
    Entries earlier{{"input.bind.circle", "Mouse Right / F"}, {"input.bind.cross", "Space"},
                    {"input.trigger_profile", "standard"}};
    check(from_entries(earlier).control_preset.shipped == input::Preset::Default,
          "earlier default bindings are Default");
    earlier["input.bind.circle"] = "G";
    Settings changed = from_entries(earlier);
    check(!changed.control_preset.shipped && changed.control_preset.user == "Custom" &&
              changed.user_presets.size() == 1u && changed.user_presets[0].name == "Custom" &&
              changed.user_presets[0].layout == changed.controls &&
              changed.controls.keys[static_cast<std::size_t>(input::Action::Circle)][0] ==
                  input::single(input::from_name("G")),
          "changed bindings become the player's preset Custom");
    Entries bows{{"input.trigger_profile", "bows"}};
    Settings shooter = from_entries(bows);
    check(!shooter.control_preset.shipped &&
              shooter.controls == input::layout_from_earlier(input::layout(input::Preset::Default).keys, "bows"),
          "a shooting trigger profile becomes Custom with its triggers");
    Entries classic;
    for (std::size_t i = 0; i < input::kActions; ++i)
        classic[std::string("input.bind.") + input::info(static_cast<input::Action>(i)).key] =
            input::format(input::layout(input::Preset::Classic).keys[i]);
    check(from_entries(classic).control_preset.shipped == input::Preset::Classic,
          "the classic keys are the Classic preset");

    // What is written reads back the same, without the retired key.
    Entries written = to_entries(changed);
    check(written.count("input.trigger_profile") == 0u && written["input.preset"] == "user:Custom",
          "the trigger profile is dropped and the preset kept");
    Settings again = from_entries(written);
    check(again.control_preset == changed.control_preset && again.controls == changed.controls &&
              again.user_presets.size() == 1u && again.user_presets[0].layout == changed.controls,
          "presets round-trip through settings.ini");

    // Presets of the player's, and choosing between them.
    choose_preset(again, {input::Preset::Modern, {}});
    check(again.controls == input::layout(input::Preset::Modern), "choosing a shipped preset puts it in use");
    const std::optional<std::string> made = prepare_controls_edit(again);
    check(made && *made == "Custom 2" && again.control_preset.user == "Custom 2" && again.user_presets.size() == 2u,
          "editing a shipped preset makes a new one of the player's");
    again.controls.swap_sticks = true;
    controls_edited(again);
    check(find_user_preset(again, "Custom 2")->layout.swap_sticks && !prepare_controls_edit(again),
          "which then keeps the edits");
    check(choose_preset(again, {std::nullopt, "Custom"}) && again.controls == changed.controls,
          "choosing the player's preset puts it back");
    check(!choose_preset(again, {std::nullopt, "Missing"}), "a preset that is not there cannot be chosen");
    Settings reread = from_entries(to_entries(again));
    check(reread.user_presets.size() == 2u && reread.user_presets[1].layout.swap_sticks &&
              reread.control_preset.user == "Custom",
          "both presets are kept");

    // The layout in use decides: bindings changed behind a shipped preset's
    // back (by hand, or by an earlier version) become the player's preset.
    Entries edited = to_entries(from_entries({}));
    edited["input.bind.square"] = "R";
    Settings behind = from_entries(edited);
    check(!behind.control_preset.shipped && behind.user_presets.size() == 1u &&
              behind.controls.keys[static_cast<std::size_t>(input::Action::Square)][0] ==
                  input::single(input::from_name("R")),
          "bindings that are no longer the preset are kept as Custom");
}

} // namespace

int main() {
    test_desktop_is_the_declared_defaults();
    test_android();
    test_this_build();
    test_control_presets();
    if (failures != 0) {
        std::cerr << failures << " failure(s)\n";
        return 1;
    }
    std::cout << "settings defaults tests passed\n";
    return 0;
}
