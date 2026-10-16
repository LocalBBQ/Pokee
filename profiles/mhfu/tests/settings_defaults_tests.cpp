// Which defaults each platform starts from; no settings.ini is read.
#include "gpu/hud_scale.hpp"
#include "settings/settings.hpp"

#include <iostream>
#include <optional>
#include <string>
#include <vector>

namespace {
using namespace mhfu::settings;

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
    using namespace mhfu;
    Settings fresh = from_entries({});
    check(fresh.control_preset.shipped == input::Preset::Default && fresh.user_presets.empty() &&
            fresh.controls == input::layout(input::Preset::Default),
        "a new player starts on Default");

    // An earlier version's file: bindings as they were, no preset.
    Entries earlier{
        {"input.bind.circle", "Mouse Right / F"}, {"input.bind.cross", "Space"}, {"input.trigger_profile", "standard"}};
    check(
        from_entries(earlier).control_preset.shipped == input::Preset::Default, "earlier default bindings are Default");
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

// A settings.ini written by the version before bindings could be cleared:
// two chords at most, user presets without the preset they were made from.
// Everything reads the same, and what is cleared stays cleared.
void test_controls_from_the_previous_version() {
    using namespace mhfu;
    const auto action = [](input::Action a) { return static_cast<std::size_t>(a); };
    Entries previous = to_entries(from_entries({}));
    previous["input.preset"] = "user:Mine";
    previous["input.user_preset.1.name"] = "Mine";
    previous["input.user_preset.1.move_stick"] = "right";
    for (std::size_t i = 0; i < input::kActions; ++i) {
        const char *key = input::info(static_cast<input::Action>(i)).key;
        previous[std::string("input.user_preset.1.bind.") + key] =
            input::format(input::layout(input::Preset::Modern).keys[i]);
        previous[std::string("input.user_preset.1.pad.") + key] =
            input::format(input::layout(input::Preset::Modern).pad[i]);
    }
    previous["input.user_preset.1.pad.circle"] = "Pad East / Pad LB + Pad South";
    previous["input.user_preset.1.bind.square"] = "";
    // The layout in use, which that version wrote as the preset is.
    for (auto &[key, value] : Entries(previous)) {
        const std::string prefix = "input.user_preset.1.";
        if (key.starts_with(prefix + "bind.") || key.starts_with(prefix + "pad."))
            previous["input." + key.substr(prefix.size())] = value;
    }
    previous["input.move_stick"] = "right";
    Settings read = from_entries(previous);
    check(read.control_preset.user == "Mine" && read.user_presets.size() == 1u &&
            read.user_presets[0].base == input::Preset::Default && read.controls.swap_sticks,
        "a preset of the previous version is read, made from Default");
    const input::Slots &circle = read.controls.pad[action(input::Action::Circle)];
    check(circle[0] == input::single(input::pad(input::PadInput::East)) &&
            circle[1] == input::chord(input::pad(input::PadInput::LeftShoulder), input::pad(input::PadInput::South)) &&
            input::count(circle) == 2u,
        "with its chords as they were");
    check(input::count(read.controls.keys[action(input::Action::Square)]) == 0u, "and an unbound action unbound");
    check(settings::base_preset(read) == input::Preset::Default, "reset goes back to Default");

    // Cleared bindings, and more than two, survive a save.
    input::clear(read.controls.pad, input::Action::Cross, 0u);
    input::add(read.controls.keys, input::Action::Circle, input::single(input::from_name("G")));
    input::add(read.controls.keys, input::Action::Circle, input::single(input::from_name("H")));
    controls_edited(read);
    Settings again = from_entries(to_entries(read));
    check(input::count(again.controls.pad[action(input::Action::Cross)]) == 0u &&
            input::count(again.user_presets[0].layout.pad[action(input::Action::Cross)]) == 0u,
        "a cleared binding stays cleared");
    check(input::count(again.controls.keys[action(input::Action::Circle)]) == 4u, "four bindings are kept");

    // Editing a shipped preset makes one of the player's based on it.
    choose_preset(again, {input::Preset::LeftHanded, {}});
    check(settings::base_preset(again) == input::Preset::LeftHanded, "a shipped preset is its own base");
    prepare_controls_edit(again);
    check(!again.control_preset.shipped && settings::base_preset(again) == input::Preset::LeftHanded,
        "the preset made from it remembers it");
    Settings reread = from_entries(to_entries(again));
    check(settings::base_preset(reread) == input::Preset::LeftHanded, "through settings.ini");

    // Bindings changed by hand behind Modern's back are based on Modern.
    Entries by_hand = to_entries(from_entries({}));
    by_hand["input.preset"] = "modern";
    for (std::size_t i = 0; i < input::kActions; ++i) {
        const char *key = input::info(static_cast<input::Action>(i)).key;
        by_hand[std::string("input.bind.") + key] = input::format(input::layout(input::Preset::Modern).keys[i]);
        by_hand[std::string("input.pad.") + key] = input::format(input::layout(input::Preset::Modern).pad[i]);
    }
    by_hand["input.pad.square"] = "";
    Settings behind = from_entries(by_hand);
    check(!behind.control_preset.shipped && settings::base_preset(behind) == input::Preset::Modern,
        "a changed shipped preset becomes Custom, based on it");
}

// A settings.ini written by main before any chord and combination (#198)
// and before lock-on (#163): no keys for the item bar's actions, lock-on,
// the chord window or combinations.
// Every binding reads as it was; the new actions take the preset's inputs
// where they clash with nothing.
Entries as_main_wrote(Entries entries) {
    for (auto it = entries.begin(); it != entries.end();) {
        const std::string &key = it->first;
        const bool added = key.find("item_left") != std::string::npos || key.find("item_right") != std::string::npos ||
            key.find("lock_on") != std::string::npos || key == "input.chord_window" ||
            key.find("combo.") != std::string::npos;
        it = added ? entries.erase(it) : std::next(it);
    }
    return entries;
}

void test_controls_from_main() {
    using namespace mhfu;
    const auto action = [](input::Action a) { return static_cast<std::size_t>(a); };
    for (std::size_t p = 0; p < input::kPresets; ++p) {
        const auto preset = static_cast<input::Preset>(p);
        Settings chosen = from_entries({});
        choose_preset(chosen, {preset, {}});
        const Entries main_file = as_main_wrote(to_entries(chosen));
        check(main_file.count("input.pad.item_left") == 0u && main_file.count("input.bind.circle") == 1u,
            "the file is as main wrote it");
        const Settings read = from_entries(main_file);
        check(read.control_preset.shipped == preset && read.controls == input::layout(preset) &&
                read.user_presets.empty() && read.chord_window == input::kDefaultChordWindowMs,
            "a shipped preset from main is still that preset, whole");
    }

    // The player's preset from main, with a paddle on Hide HUD and LB + X
    // on SELECT: those stay, and Item left, which main did not have, does not
    // take the paddle.
    Settings mine = from_entries({});
    prepare_controls_edit(mine);
    input::Layout &layout = mine.controls;
    layout.pad[action(input::Action::HideHud)] = {input::single(input::pad(input::PadInput::LeftPaddle1))};
    layout.pad[action(input::Action::Select)] = {
        input::chord(input::pad(input::PadInput::LeftShoulder), input::pad(input::PadInput::West))};
    // Main had no item actions and no lock-on.
    for (const input::Action a : {input::Action::ItemLeft, input::Action::ItemRight, input::Action::LockOn})
        layout.keys[action(a)] = layout.pad[action(a)] = {};
    controls_edited(mine);
    const Entries main_file = as_main_wrote(to_entries(mine));
    const Settings read = from_entries(main_file);
    check(
        !read.control_preset.shipped && read.user_presets.size() == 1u && read.user_presets[0].layout == read.controls,
        "the player's preset from main is read and in use");
    bool same = true;
    for (std::size_t i = 0; i < input::kActions; ++i) {
        const auto a = static_cast<input::Action>(i);
        if (a == input::Action::ItemLeft || a == input::Action::ItemRight || a == input::Action::LockOn) continue;
        same = same && read.controls.keys[i] == mine.controls.keys[i] && read.controls.pad[i] == mine.controls.pad[i];
    }
    check(same, "every binding main wrote reads unchanged");
    check(input::count(read.controls.pad[action(input::Action::ItemLeft)]) == 0u &&
            read.controls.pad[action(input::Action::ItemRight)][0] ==
                input::single(input::pad(input::PadInput::RightPaddle1)),
        "Item left does not take the paddle Hide HUD has; Item right takes the free one");
    check(read.controls.keys[action(input::Action::ItemLeft)][0] == input::single(input::mouse_button(4)),
        "and the keyboard's item keys are added where free");
    check(
        read.controls.pad[action(input::Action::LockOn)][0] == input::single(input::pad(input::PadInput::RightStick)) &&
            read.controls.keys[action(input::Action::LockOn)][0] == input::single(input::mouse_button(2)),
        "lock-on takes R3 and the middle mouse button, which the old file left free");
    for (std::size_t i = 0; i < input::kActions; ++i)
        for (const bool pad : {false, true})
            check(input::conflicts(input::table(read.controls, pad), i).size() ==
                        input::conflicts(input::table(mine.controls, pad), i).size() ||
                    i == action(input::Action::ItemLeft) || i == action(input::Action::ItemRight) ||
                    i == action(input::Action::LockOn),
                "and no new conflict");

    // A file with only a few keys: the rest keeps the preset's, and nothing
    // is taken away for a clash with the chords it adds.
    const Settings partial = from_entries({{"input.preset", "default"}, {"input.pad.item_right", "Pad LB + Pad East"}});
    check(partial.controls.pad[action(input::Action::Circle)] ==
            input::layout(input::Preset::Default).pad[action(input::Action::Circle)],
        "a key the file does not have keeps the preset's binding");

    // What this version writes reads back: combinations, the chord window,
    // and an item action cleared on purpose.
    Settings next = read;
    next.chord_window = 0u;
    next.controls.combos.push_back({0x4000u | 0x2000u, {input::single(input::from_name("V"))},
        {input::chord(input::pad(input::PadInput::North), input::pad(input::PadInput::East))}});
    next.controls.pad[action(input::Action::ItemRight)] = {};
    controls_edited(next);
    const Entries written = to_entries(next);
    check(written.at("input.combo.1.buttons") == "Circle + Cross" &&
            written.at("input.combo.1.pad") == "Pad North + Pad East",
        "a combination is written with its buttons and chords");
    const Settings again = from_entries(written);
    check(again.controls == next.controls && again.user_presets[0].layout.combos == next.controls.combos &&
            again.chord_window == 0u,
        "and read back, in the preset too");
    check(input::count(again.controls.pad[action(input::Action::ItemRight)]) == 0u,
        "a cleared item action stays cleared");
    Entries broken = written;
    broken["input.combo.1.buttons"] = "Nonsense";
    check(from_entries(broken).controls.combos.empty(), "a combination with no buttons it knows is dropped");
}

} // namespace

// Layered armor: off by default with every part real, and the choices kept
// by part: "real" or a piece id.
void test_layered_armor() {
    const Settings fresh = from_entries({});
    check(!fresh.layered_armor && !fresh.layered_all, "layered armor starts off, listing owned pieces");
    for (const std::int32_t choice : fresh.layered_pieces) check(choice == kLayeredReal, "every part starts real");
    Entries entries = to_entries(fresh);
    check(entries["look.layered_head"] == "real", "a real part is written as real");
    entries["look.layered_armor"] = "1";
    entries["look.layered_chest"] = "12";
    entries["look.layered_head"] = "0";
    entries["look.layered_legs"] = "70000"; // not a piece id: stays real
    const Settings read = from_entries(entries);
    check(read.layered_armor, "layered armor read on");
    check(read.layered_pieces[0] == 12, "the chest's piece read");
    check(read.layered_pieces[4] == 0, "nothing for the head read");
    check(read.layered_pieces[3] == kLayeredReal, "a number past the ids is not a piece");
    const Settings again = from_entries(to_entries(read));
    check(again.layered_pieces == read.layered_pieces && again.layered_armor, "the choices survive a round trip");
}

// Sharp text is off by default. v0.6.5 and v0.6.6 wrote text.crisp=1 into
// every settings.ini they saved, so a file without settings.version gets the
// new default; from version 2 on, the file's value is the player's.
void test_crisp_text() {
    check(!defaults_for(Platform::Desktop).crisp_text && !defaults_for(Platform::Android).crisp_text,
        "Sharp text is off by default on every platform");
    Entries earlier = to_entries(from_entries({}));
    earlier.erase("settings.version");
    earlier["text.crisp"] = "1";
    check(!from_entries(earlier).crisp_text, "text.crisp=1 from an earlier version is its default, not a choice");
    Settings chosen = from_entries({});
    chosen.crisp_text = true;
    const Entries written = to_entries(chosen);
    check(written.count("settings.version") == 1u && written.at("settings.version") == "3",
        "every file written says its version");
    check(from_entries(written).crisp_text, "Sharp text turned on now stays on");
    Entries off = written;
    off["text.crisp"] = "0";
    check(!from_entries(off).crisp_text, "and off stays off");
}

// Lock-on style and damage numbers. Comparable weapon numbers used to be a
// setting; a file that still has the key drops it.
void test_lock_on_and_damage_numbers() {
    const Settings fresh = from_entries({});
    check(to_entries(from_entries({{"game.comparable_weapons", "1"}})).count("game.comparable_weapons") == 0u,
        "a saved comparable-weapons switch is not kept");
    check(fresh.lock_on_style == LockOnStyle::Follow, "lock-on follows the monster until the style says otherwise");
    check(fresh.hunt_portrait_size == 100u, "the monster icon starts at its usual size");
    Entries portrait = to_entries(fresh);
    portrait["video.hunt_portrait"] = "150";
    check(from_entries(to_entries(from_entries(portrait))).hunt_portrait_size == 150u,
        "the portrait size survives a round trip");
    portrait["video.hunt_portrait"] = "10";
    check(from_entries(portrait).hunt_portrait_size == kMinHuntPortraitSize, "a size under the limit clamps up");
    portrait["video.hunt_portrait"] = "400";
    check(from_entries(portrait).hunt_portrait_size == kMaxHuntPortraitSize, "a size past the limit clamps down");
    Entries style = to_entries(fresh);
    style["input.lock_on_style"] = "focus";
    check(from_entries(style).lock_on_style == LockOnStyle::Focus, "the file selects focus");
    check(from_entries(to_entries(from_entries(style))).lock_on_style == LockOnStyle::Focus,
        "and focus survives a round trip");
    style["input.lock_on_style"] = "nope";
    check(from_entries(style).lock_on_style == LockOnStyle::Follow, "an unknown style keeps follow");
    check(fresh.gather_icon && fresh.gather_hold, "the gather mark and holding ○ to gather start on");
    Entries gather = to_entries(fresh);
    gather["game.gather_icon"] = "0";
    check(!from_entries(gather).gather_icon, "the file turns the gather mark off");
    check(!from_entries(to_entries(from_entries(gather))).gather_icon, "and off survives a round trip");
    gather["game.gather_hold"] = "0";
    check(!from_entries(gather).gather_hold, "the file turns holding ○ to gather off");
    check(!from_entries(to_entries(from_entries(gather))).gather_hold, "and that off survives a round trip");
    check(fresh.empty_item_slot, "the item bar starts with an empty slot");
    Entries empty_slot = to_entries(fresh);
    empty_slot["game.empty_item_slot"] = "0";
    check(!from_entries(empty_slot).empty_item_slot, "the file turns the empty item slot off");
    check(!from_entries(to_entries(from_entries(empty_slot))).empty_item_slot, "and off survives a round trip");
    Entries earlier_slot = to_entries(fresh);
    earlier_slot["settings.version"] = "2";
    earlier_slot["game.empty_item_slot"] = "0";
    check(from_entries(earlier_slot).empty_item_slot,
        "game.empty_item_slot=0 from before version 3 is its default, not a choice");
    check(fresh.gather_icon_size == 100u && fresh.gather_icon_thickness == 1u,
        "the gather mark starts at its usual size and the thinnest ring");
    Entries mark = to_entries(fresh);
    mark["game.gather_icon_size"] = "180";
    mark["game.gather_icon_thickness"] = "2";
    const Settings sized = from_entries(to_entries(from_entries(mark)));
    check(sized.gather_icon_size == 180u && sized.gather_icon_thickness == 2u,
        "the gather mark's size and thickness come from the file and survive a round trip");
    mark["game.gather_icon_size"] = std::to_string(kMaxGatherIconSize + 1u);
    mark["game.gather_icon_thickness"] = "9";
    const Settings huge = from_entries(mark);
    check(huge.gather_icon_size == kMaxGatherIconSize && huge.gather_icon_thickness == kMaxGatherIconThickness,
        "a gather mark past either limit clamps to it");
    mark["game.gather_icon_size"] = "1";
    mark["game.gather_icon_thickness"] = "0";
    const Settings tiny = from_entries(mark);
    check(tiny.gather_icon_size == kMinGatherIconSize && tiny.gather_icon_thickness == 1u,
        "a gather mark under either limit clamps up");
    mark["game.gather_icon_size"] = std::to_string(kMinGatherIconSize);
    mark["game.gather_icon_thickness"] = std::to_string(kMaxGatherIconThickness);
    const Settings ends = from_entries(mark);
    check(ends.gather_icon_size == kMinGatherIconSize && ends.gather_icon_thickness == kMaxGatherIconThickness,
        "the ends of the gather mark's ranges are allowed");
    check(!fresh.damage_numbers, "damage numbers start off");
    Entries hits = to_entries(fresh);
    hits["game.damage_numbers"] = "1";
    check(from_entries(hits).damage_numbers, "the file turns damage numbers on");
    check(from_entries(to_entries(from_entries(hits))).damage_numbers, "and they survive a round trip");
    check(!fresh.sharpness_gauge, "the sharpness gauge starts off, so the game's bar stays whole");
    Entries gauge = to_entries(fresh);
    gauge["game.sharpness_gauge"] = "1";
    check(from_entries(to_entries(from_entries(gauge))).sharpness_gauge, "the file turns the sharpness gauge on");
    check(fresh.sharpness_effect == SharpnessEffect::Glow, "the colour on the blade pulses by default");
    Entries effect = to_entries(fresh);
    effect["game.sharpness_effect"] = "solid";
    check(from_entries(to_entries(from_entries(effect))).sharpness_effect == SharpnessEffect::Solid,
        "the file sets the sharpness effect to solid");
    check(fresh.damage_number_size == 100u && fresh.damage_number_thickness == 1u,
        "damage numbers start at the interface's size and the thinnest stroke");
    Entries look = to_entries(fresh);
    look["game.damage_number_size"] = "250";
    look["game.damage_number_thickness"] = "2";
    const Settings bigger = from_entries(to_entries(from_entries(look)));
    check(bigger.damage_number_size == 250u && bigger.damage_number_thickness == 2u,
        "their size and thickness come from the file and survive a round trip");
    look["game.damage_number_thickness"] = "5";
    check(from_entries(look).damage_number_thickness == kMaxDamageNumberThickness,
        "a thickness past the readable limit clamps to it");
    look["game.damage_number_size"] = std::to_string(kMaxDamageNumberSize + 1u);
    look["game.damage_number_thickness"] = "0";
    const Settings outside = from_entries(look);
    check(outside.damage_number_size == kMaxDamageNumberSize && outside.damage_number_thickness == 1u,
        "a size past the limit clamps to it, and a thickness of 0 clamps to the thinnest");
    look["game.damage_number_size"] = std::to_string(kMinDamageNumberSize);
    look["game.damage_number_thickness"] = std::to_string(kMaxDamageNumberThickness);
    const Settings edges = from_entries(look);
    check(
        edges.damage_number_size == kMinDamageNumberSize && edges.damage_number_thickness == kMaxDamageNumberThickness,
        "the ends of both ranges are allowed");
}

// The Video page's Interface sizes: the area map, the clock with health and
// the weapon pieces, and the item bar, each saved on its own.
void test_interface_sizes() {
    using mhfu::gpu::hud_scale::Frame;
    using mhfu::gpu::hud_scale::Scales;
    const Settings fresh = defaults();
    check(fresh.hud.map == 100u && fresh.hud.health == 100u && fresh.hud.item_bar == 100u && fresh.hud.names == 100u,
        "every interface piece starts at the game's size");
    const Settings chosen = from_entries({{"video.hud.map", "50"}, {"video.hud.health", "150"},
        {"video.hud.item_bar", "70"}, {"video.hud.names", "60"}});
    check(chosen.hud.map == 50u && chosen.hud.health == 150u && chosen.hud.item_bar == 70u && chosen.hud.names == 60u,
        "each piece reads its own size");
    const Entries written = to_entries(chosen);
    check(written.at("video.hud.map") == "50" && written.at("video.hud.health") == "150" &&
            written.at("video.hud.item_bar") == "70" && written.at("video.hud.names") == "60",
        "each piece writes its own size");
    check(from_entries({{"video.hud.map", "40"}}).hud.map == 50u, "below 50% stays at 50%");
    check(from_entries({{"video.hud.item_bar", "900"}}).hud.item_bar == 150u, "above 150% stays at 150%");
    check(from_entries({{"video.hud.names", "10"}}).hud.names == 50u, "a name below 50% stays at 50%");
    check(from_entries({{"video.hud.names", "200"}}).hud.names == 150u, "a name above 150% stays at 150%");

    // One size used to cover all three.
    const Settings older = from_entries({{"video.map_item_scale", "80"}});
    check(older.hud.map == 80u && older.hud.health == 80u && older.hud.item_bar == 80u && older.hud.names == 100u,
        "the earlier single size carries over to the map, status and items, and leaves names at the game's size");
    check(from_entries({{"video.map_item_scale", "80"}, {"video.hud.map", "60"}}).hud.health == 100u,
        "the earlier size is ignored once a separate size is saved");
    check(to_entries(older).count("video.map_item_scale") == 0u, "the earlier key is not written again");

    struct Vertex {
        float x;
        float y;
    };
    // The area map tile, (316, 32)-(468, 184), grows from its top-right.
    const Scales map_only{2.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f};
    std::vector<Vertex> map(6, Vertex{316.0f, 32.0f});
    map[2] = {468.0f, 184.0f};
    Frame frame;
    mhfu::gpu::hud_scale::scale_quads(map, map_only, frame);
    check(map[0].x == 164.0f && map[0].y == 32.0f && map[2].x == 468.0f && map[2].y == 336.0f,
        "the map grows left and down from its top-right corner");
    // The small map, (425, 78)-(465, 118), shrinks toward the slot's top-right,
    // and the player's arrow drawn after it stays on it.
    const Scales map_half{0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f};
    Frame quest;
    std::vector<Vertex> small(6, Vertex{425.0f, 78.0f});
    small[2] = {465.0f, 118.0f};
    std::vector<Vertex> quest_clock(6, Vertex{8.0f, 8.0f});
    quest_clock[2] = {60.0f, 56.0f};
    small.insert(small.end(), quest_clock.begin(), quest_clock.end());
    mhfu::gpu::hud_scale::scale_quads(small, map_half, quest, mhfu::gpu::hud_scale::kAreaMapTexture);
    check(small[0].x == 446.5f && small[0].y == 55.0f && small[2].x == 466.5f && small[2].y == 75.0f,
        "the small map shrinks toward the slot's top-right");
    std::vector<Vertex> arrow(6, Vertex{438.0f, 95.0f});
    arrow[2] = {450.0f, 107.0f};
    mhfu::gpu::hud_scale::scale_quads(arrow, map_half, quest);
    check(arrow[0].x == 453.0f && arrow[0].y == 63.5f && arrow[2].x == 459.0f && arrow[2].y == 69.5f,
        "the player's arrow moves with the small map");
    // Pointing up, the arrow's box is 10 by 12, the same size as a letter, so
    // it was left at the game's size until the hunter turned. A letter of that
    // size, from the font, still stays put.
    std::vector<Vertex> upright(6, Vertex{440.0f, 90.0f});
    upright[2] = {450.0f, 102.0f};
    mhfu::gpu::hud_scale::scale_quads(upright, map_half, quest, mhfu::gpu::hud_scale::kMapIconTexture);
    check(upright[0].x == 454.0f && upright[0].y == 61.0f && upright[2].x == 459.0f && upright[2].y == 67.0f,
        "the arrow pointing up stays at the map's size");
    // Pointing sideways the box is 16 by 8, the size of the notice's border.
    std::vector<Vertex> sideways(6, Vertex{430.0f, 96.0f});
    sideways[2] = {446.0f, 104.0f};
    mhfu::gpu::hud_scale::scale_quads(sideways, map_half, quest, mhfu::gpu::hud_scale::kMapIconTexture);
    check(sideways[0].x == 449.0f && sideways[0].y == 64.0f && sideways[2].x == 457.0f && sideways[2].y == 68.0f,
        "the arrow pointing sideways stays at the map's size");
    Frame letter_up;
    std::vector<Vertex> letter_map(6, Vertex{425.0f, 78.0f});
    letter_map[2] = {465.0f, 118.0f};
    std::vector<Vertex> letter_clock(6, Vertex{8.0f, 8.0f});
    letter_clock[2] = {60.0f, 56.0f};
    letter_map.insert(letter_map.end(), letter_clock.begin(), letter_clock.end());
    mhfu::gpu::hud_scale::scale_quads(letter_map, map_half, letter_up, mhfu::gpu::hud_scale::kAreaMapTexture);
    std::vector<Vertex> upright_letter(6, Vertex{440.0f, 90.0f});
    upright_letter[2] = {450.0f, 102.0f};
    mhfu::gpu::hud_scale::scale_quads(upright_letter, map_half, letter_up, 0x040CC000u);
    check(upright_letter[0].x == 440.0f && upright_letter[0].y == 90.0f && upright_letter[2].x == 450.0f &&
            upright_letter[2].y == 102.0f,
        "a letter the size of the upright arrow stays where the game drew it");

    std::vector<Vertex> health(6, Vertex{56.0f, 12.0f});
    health[2] = {320.0f, 20.0f};
    mhfu::gpu::hud_scale::scale_quads(health, map_only, frame);
    check(health[0].x == 56.0f && health[2].x == 320.0f && health[2].y == 20.0f,
        "the map's size leaves the health bar alone");
    // Health shrinks toward the clock's corner at (8, 8).
    const Scales group_half{1.0f, 0.5f, 1.0f, 0.5f, 0.5f, 0.5f};
    mhfu::gpu::hud_scale::scale_quads(health, group_half, frame);
    check(health[0].x == 32.0f && health[0].y == 10.0f && health[2].x == 164.0f && health[2].y == 14.0f,
        "health shrinks toward the clock's corner");
    // Raised maximum health draws the bar out to x 399. It still shrinks with
    // the clock; left at the game's position it is drawn over the clock.
    Frame raised;
    std::vector<Vertex> long_health(6, Vertex{56.0f, 12.0f});
    long_health[2] = {399.0f, 20.0f};
    mhfu::gpu::hud_scale::scale_quads(long_health, group_half, raised);
    check(long_health[0].x == 32.0f && long_health[0].y == 10.0f && long_health[2].x == 203.5f &&
            long_health[2].y == 14.0f,
        "a raised health bar shrinks toward the clock's corner");
    // At 150 either bar reaches x 452, and its 24 by 12 end cap x 476. Both
    // stay with the clock instead of springing back to the game's size.
    Frame full;
    std::vector<Vertex> full_bars(6, Vertex{56.0f, 12.0f});
    full_bars[2] = {452.0f, 20.0f};
    const auto add_quad = [&](float x0, float y0, float x1, float y1) {
        std::vector<Vertex> quad(6, Vertex{x0, y0});
        quad[2] = {x1, y1};
        full_bars.insert(full_bars.end(), quad.begin(), quad.end());
    };
    add_quad(452.0f, 9.0f, 476.0f, 21.0f);  // health's end cap
    add_quad(56.0f, 21.0f, 452.0f, 29.0f);  // stamina at 150
    add_quad(452.0f, 21.0f, 476.0f, 33.0f); // stamina's end cap
    add_quad(56.0f, 24.0f, 452.0f, 26.0f);  // stamina's two-pixel fill
    mhfu::gpu::hud_scale::scale_quads(full_bars, group_half, full);
    check(full_bars[2].x == 230.0f && full_bars[2].y == 14.0f, "health at 150 shrinks toward the clock's corner");
    check(full_bars[6].x == 230.0f && full_bars[6].y == 8.5f && full_bars[8].x == 242.0f && full_bars[8].y == 14.5f,
        "health's end cap stays on the shrunk bar");
    check(full_bars[12].x == 32.0f && full_bars[12].y == 14.5f && full_bars[14].x == 230.0f && full_bars[14].y == 18.5f,
        "stamina at 150 shrinks toward the clock's corner");
    check(full_bars[20].x == 242.0f && full_bars[20].y == 20.5f, "stamina's end cap stays on the shrunk bar");
    check(full_bars[24].y == 16.0f && full_bars[26].x == 230.0f, "stamina's fill stays inside the shrunk bar");
    check(mhfu::gpu::hud_scale::piece_of({316.0f, 32.0f, 468.0f, 184.0f}) == mhfu::gpu::hud_scale::Piece::Map,
        "the wider status band does not take the area map");
    check(mhfu::gpu::hud_scale::piece_of({232.0f, 28.0f, 456.0f, 36.0f}) == mhfu::gpu::hud_scale::Piece::None,
        "a strip past the bars' end that is not a bar or a cap is not status");
    check(mhfu::gpu::hud_scale::piece_of({386.0f, 21.0f, 410.0f, 33.0f}) == mhfu::gpu::hud_scale::Piece::Vitals,
        "a cap just past the old status edge, stamina at 125, is status");
    // Panels drawn in the status band while the bars are up, traced with
    // MHFU_TRACE_HUD_SCALE: the item box's top edge, and a message window
    // with its text. Shrunk with the status they were pulled off their panel.
    for (const mhfu::gpu::hud_scale::Rect other : {mhfu::gpu::hud_scale::Rect{216.0f, 28.0f, 232.0f, 36.0f},
             mhfu::gpu::hud_scale::Rect{152.0f, 8.0f, 168.0f, 24.0f},
             mhfu::gpu::hud_scale::Rect{168.0f, 8.0f, 312.0f, 24.0f},
             mhfu::gpu::hud_scale::Rect{168.0f, 24.0f, 312.0f, 32.0f},
             mhfu::gpu::hud_scale::Rect{174.0f, 9.0f, 181.0f, 23.0f}})
        check(mhfu::gpu::hud_scale::piece_of(other) == mhfu::gpu::hud_scale::Piece::None,
            "another panel in the status band is not status");
    // The notice on a cart and on a carve. It starts in that band, then slides
    // down. Traced with MHFU_TRACE_HUD_SCALE: the right border crosses the area
    // map, then sits on the ammo menu's corner, and the last letters follow it.
    Frame cart_banner;
    std::vector<Vertex> banner(6, Vertex{152.0f, 8.0f});
    banner[2] = {328.0f, 24.0f};
    std::vector<Vertex> banner_letter(6, Vertex{195.0f, 9.0f});
    banner_letter[2] = {202.0f, 23.0f};
    std::vector<Vertex> banner_clock(6, Vertex{8.0f, 8.0f});
    banner_clock[2] = {60.0f, 56.0f};
    std::vector<Vertex> banner_row(banner);
    banner_row.insert(banner_row.end(), banner_letter.begin(), banner_letter.end());
    banner_row.insert(banner_row.end(), banner_clock.begin(), banner_clock.end());
    mhfu::gpu::hud_scale::scale_quads(banner_row, group_half, cart_banner);
    check(banner_row[0].x == 152.0f && banner_row[0].y == 8.0f && banner_row[6].x == 195.0f &&
            banner_row[6].y == 9.0f && banner_row[14].x == 34.0f && banner_row[14].y == 32.0f,
        "the notice at the top of the picture stays put while the clock shrinks");
    Frame carving;
    std::vector<Vertex> area(6, Vertex{316.0f, 32.0f});
    area[2] = {468.0f, 184.0f};
    std::vector<Vertex> cap_top(6, Vertex{328.0f, 42.0f});
    cap_top[2] = {344.0f, 50.0f};
    std::vector<Vertex> cap_mid(6, Vertex{328.0f, 50.0f});
    cap_mid[2] = {344.0f, 66.0f};
    std::vector<Vertex> cap_low(6, Vertex{328.0f, 66.0f});
    cap_low[2] = {344.0f, 74.0f};
    std::vector<Vertex> cap_letter(6, Vertex{317.0f, 51.0f});
    cap_letter[2] = {324.0f, 65.0f};
    std::vector<Vertex> carving_row(area);
    carving_row.insert(carving_row.end(), cap_top.begin(), cap_top.end());
    carving_row.insert(carving_row.end(), cap_mid.begin(), cap_mid.end());
    carving_row.insert(carving_row.end(), cap_low.begin(), cap_low.end());
    carving_row.insert(carving_row.end(), cap_letter.begin(), cap_letter.end());
    mhfu::gpu::hud_scale::scale_quads(carving_row, map_half, carving);
    check(carving_row[0].x == 392.0f && carving_row[2].x == 468.0f && carving_row[6].x == 328.0f &&
            carving_row[6].y == 42.0f && carving_row[12].x == 328.0f && carving_row[18].x == 328.0f &&
            carving_row[24].x == 317.0f && carving_row[24].y == 51.0f,
        "the notice's border and its letters stay put while they cross the area map");
    // A long carve or cart line reaches into the map. Traced with
    // MHFU_TRACE_HUD_SCALE: the cap, the rows, and the player's arrow, which
    // is the same size as the 16-tall border and must still follow the map.
    Frame long_notice;
    std::vector<Vertex> long_map(6, Vertex{316.0f, 32.0f});
    long_map[2] = {468.0f, 184.0f};
    std::vector<Vertex> long_cap(6, Vertex{344.0f, 160.0f});
    long_cap[2] = {360.0f, 168.0f};
    std::vector<Vertex> long_strip(6, Vertex{360.0f, 160.0f});
    long_strip[2] = {456.0f, 168.0f};
    std::vector<Vertex> long_row(6, Vertex{350.0f, 46.0f});
    long_row[2] = {466.0f, 61.0f};
    std::vector<Vertex> long_row_b(6, Vertex{350.0f, 95.0f});
    long_row_b[2] = {466.0f, 112.0f};
    std::vector<Vertex> long_strip_b(6, Vertex{376.0f, 168.0f});
    long_strip_b[2] = {456.0f, 176.0f};
    std::vector<Vertex> long_mid(6, Vertex{344.0f, 168.0f});
    long_mid[2] = {360.0f, 184.0f};
    std::vector<Vertex> long_arrow(6, Vertex{427.0f, 151.0f});
    long_arrow[2] = {443.0f, 167.0f};
    std::vector<Vertex> long_all(long_map);
    long_all.insert(long_all.end(), long_cap.begin(), long_cap.end());
    long_all.insert(long_all.end(), long_strip.begin(), long_strip.end());
    long_all.insert(long_all.end(), long_row.begin(), long_row.end());
    long_all.insert(long_all.end(), long_row_b.begin(), long_row_b.end());
    long_all.insert(long_all.end(), long_strip_b.begin(), long_strip_b.end());
    long_all.insert(long_all.end(), long_mid.begin(), long_mid.end());
    long_all.insert(long_all.end(), long_arrow.begin(), long_arrow.end());
    mhfu::gpu::hud_scale::scale_quads(long_all, map_half, long_notice);
    check(long_all[6].x == 344.0f && long_all[6].y == 160.0f && long_all[12].x == 360.0f && long_all[18].x == 350.0f &&
            long_all[18].y == 46.0f && long_all[24].y == 95.0f && long_all[30].x == 376.0f &&
            long_all[36].x == 344.0f && long_all[36].y == 168.0f && long_all[42].x == 447.5f && long_all[42].y == 91.5f,
        "a long notice stays put inside the area map, and the player's arrow still follows the map");
    const Scales bar_notice{1.0f, 1.0f, 0.5f, 1.0f, 1.0f, 0.5f};
    Frame notice_center;
    std::vector<Vertex> edge_top(6, Vertex{312.0f, 120.0f});
    edge_top[2] = {328.0f, 128.0f};
    std::vector<Vertex> edge_mid(6, Vertex{312.0f, 128.0f});
    edge_mid[2] = {328.0f, 144.0f};
    std::vector<Vertex> edge_low(6, Vertex{312.0f, 144.0f});
    edge_low[2] = {328.0f, 152.0f};
    std::vector<Vertex> edge_letter(6, Vertex{303.0f, 129.0f});
    edge_letter[2] = {310.0f, 143.0f};
    std::vector<Vertex> center_row(edge_top);
    center_row.insert(center_row.end(), edge_mid.begin(), edge_mid.end());
    center_row.insert(center_row.end(), edge_low.begin(), edge_low.end());
    center_row.insert(center_row.end(), edge_letter.begin(), edge_letter.end());
    mhfu::gpu::hud_scale::scale_quads(center_row, bar_notice, notice_center);
    check(!notice_center.ammo_menu_now && center_row[0].x == 312.0f && center_row[0].y == 120.0f &&
            center_row[6].x == 312.0f && center_row[12].x == 312.0f && center_row[18].x == 303.0f &&
            center_row[18].y == 129.0f,
        "the notice over the item bar is not the ammo menu");
    // The border's top tile passes through the menu's corner row. The tile 24
    // pixels below it is the rest of the border, so the menu stays shut and
    // the letter beside the border stays where the game drew it.
    Frame notice_passing;
    std::vector<Vertex> pass_top(6, Vertex{312.0f, 128.0f});
    pass_top[2] = {328.0f, 136.0f};
    std::vector<Vertex> pass_low(6, Vertex{312.0f, 152.0f});
    pass_low[2] = {328.0f, 160.0f};
    std::vector<Vertex> pass_letter(6, Vertex{300.0f, 129.0f});
    pass_letter[2] = {307.0f, 143.0f};
    std::vector<Vertex> pass_row(pass_top);
    pass_row.insert(pass_row.end(), pass_low.begin(), pass_low.end());
    pass_row.insert(pass_row.end(), pass_letter.begin(), pass_letter.end());
    mhfu::gpu::hud_scale::scale_quads(pass_row, bar_notice, notice_passing);
    check(!notice_passing.ammo_menu_now && pass_row[0].x == 312.0f && pass_row[0].y == 128.0f &&
            pass_row[12].x == 300.0f && pass_row[12].y == 129.0f,
        "the notice's border on the ammo menu's corner row does not open the menu");
    Frame notice_gun;
    notice_gun.note({8.0f, 8.0f, 60.0f, 56.0f});
    std::vector<Vertex> gun_bar(6, Vertex{324.0f, 184.0f});
    gun_bar[2] = {468.0f, 200.0f};
    std::vector<Vertex> gun_cap(6, Vertex{312.0f, 128.0f});
    gun_cap[2] = {328.0f, 144.0f};
    std::vector<Vertex> gun_letter(6, Vertex{312.0f, 140.0f});
    gun_letter[2] = {319.0f, 154.0f};
    std::vector<Vertex> gun_count(6, Vertex{346.0f, 186.0f});
    gun_count[2] = {353.0f, 200.0f};
    std::vector<Vertex> gun_row(gun_bar);
    gun_row.insert(gun_row.end(), gun_cap.begin(), gun_cap.end());
    gun_row.insert(gun_row.end(), gun_letter.begin(), gun_letter.end());
    gun_row.insert(gun_row.end(), gun_count.begin(), gun_count.end());
    mhfu::gpu::hud_scale::scale_quads(gun_row, group_half, notice_gun);
    check(gun_row[6].x == 312.0f && gun_row[6].y == 128.0f && gun_row[12].x == 312.0f && gun_row[12].y == 140.0f &&
            gun_row[18].x == 409.0f && gun_row[18].y == 193.0f,
        "the notice above the ammo bar stays put, and the ammo bar's own letter shrinks with it");
    check(mhfu::gpu::hud_scale::piece_of({56.0f, 17.0f, 320.0f, 19.0f}) == mhfu::gpu::hud_scale::Piece::Vitals &&
            mhfu::gpu::hud_scale::piece_of({200.0f, 13.0f, 260.0f, 17.0f}) == mhfu::gpu::hud_scale::Piece::Vitals,
        "a fill, and a segment inside a bar, stay status");
    // Once a monster is found the small map is the window at (371, 59)-(411, 99).
    // It shrinks toward the slot's top-right, and the mark on it stays on it.
    // Status size does not pull it toward the clock.
    Frame found;
    std::vector<Vertex> map_window(6, Vertex{371.0f, 59.0f});
    map_window[2] = {411.0f, 99.0f};
    std::vector<Vertex> map_mark(6, Vertex{385.0f, 74.0f});
    map_mark[2] = {399.0f, 86.0f};
    std::vector<Vertex> found_map(map_window);
    found_map.insert(found_map.end(), map_mark.begin(), map_mark.end());
    std::vector<Vertex> found_clock(6, Vertex{8.0f, 8.0f});
    found_clock[2] = {60.0f, 56.0f};
    found_map.insert(found_map.end(), found_clock.begin(), found_clock.end());
    mhfu::gpu::hud_scale::scale_quads(found_map, map_half, found, mhfu::gpu::hud_scale::kAreaMapTexture);
    check(found_map[0].x == 419.5f && found_map[0].y == 45.5f && found_map[2].x == 439.5f && found_map[2].y == 65.5f,
        "the map window shown once a monster is found shrinks toward the slot's top-right");
    check(found_map[6].x == 426.5f && found_map[6].y == 53.0f && found_map[8].x == 433.5f && found_map[8].y == 59.0f,
        "the mark on that window stays on the map");
    // For a few frames as the monster is found, those texels are 60 by 60
    // from (276, 70), left of the slot. A mark on that window follows it.
    // The 176 by 120 panel at (280, 16) starts just as far left and stays put.
    Frame zoomed;
    std::vector<Vertex> zoom(6, Vertex{276.0f, 70.0f});
    zoom[2] = {336.0f, 130.0f};
    std::vector<Vertex> zoom_mark(6, Vertex{290.0f, 80.0f});
    zoom_mark[2] = {304.0f, 94.0f};
    std::vector<Vertex> zoom_clock(6, Vertex{8.0f, 8.0f});
    zoom_clock[2] = {60.0f, 56.0f};
    zoom.insert(zoom.end(), zoom_mark.begin(), zoom_mark.end());
    zoom.insert(zoom.end(), zoom_clock.begin(), zoom_clock.end());
    mhfu::gpu::hud_scale::scale_quads(zoom, map_half, zoomed, mhfu::gpu::hud_scale::kAreaMapTexture);
    check(zoom[0].x == 372.0f && zoom[0].y == 51.0f && zoom[2].x == 402.0f && zoom[2].y == 81.0f,
        "the 60 by 60 window as a monster is found shrinks toward the slot's top-right");
    check(zoom[6].x == 379.0f && zoom[6].y == 56.0f && zoom[8].x == 386.0f && zoom[8].y == 63.0f,
        "the mark on that window stays on it");
    std::vector<Vertex> beside(6, Vertex{280.0f, 16.0f});
    beside[2] = {456.0f, 136.0f};
    mhfu::gpu::hud_scale::scale_quads(beside, map_half, zoomed, 0x09164420u);
    check(beside[0].x == 280.0f && beside[0].y == 16.0f && beside[2].x == 456.0f && beside[2].y == 136.0f,
        "the panel just left of the slot is not the found-monster window");
    // "Herb obtained." sits in the middle of the picture. Its right end and
    // the item icon overlap the enlarged map and were pulled onto it.
    std::vector<Vertex> obtain_top(6, Vertex{176.0f, 122.0f});
    obtain_top[2] = {286.0f, 126.0f};
    std::vector<Vertex> obtain_bottom(6, Vertex{176.0f, 148.0f});
    obtain_bottom[2] = {286.0f, 152.0f};
    std::vector<Vertex> obtain_cap(6, Vertex{286.0f, 122.0f});
    obtain_cap[2] = {302.0f, 146.0f};
    std::vector<Vertex> obtain_icon(6, Vertex{248.0f, 128.0f});
    obtain_icon[2] = {264.0f, 144.0f};
    std::vector<Vertex> obtain(obtain_top);
    obtain.insert(obtain.end(), obtain_bottom.begin(), obtain_bottom.end());
    obtain.insert(obtain.end(), obtain_cap.begin(), obtain_cap.end());
    obtain.insert(obtain.end(), obtain_icon.begin(), obtain_icon.end());
    mhfu::gpu::hud_scale::scale_quads(obtain, map_half, zoomed, 0x040CC000u);
    check(obtain[0].x == 176.0f && obtain[0].y == 122.0f && obtain[6].x == 176.0f && obtain[6].y == 148.0f &&
            obtain[12].x == 286.0f && obtain[12].y == 122.0f && obtain[18].x == 248.0f && obtain[18].y == 128.0f,
        "the obtain window stays whole while its right end overlaps the enlarged map");
    // The item's name is the size of a hunter's name, and the quest clock is
    // up. Status must not pull that name off the banner.
    Frame obtained;
    std::vector<Vertex> got_edge(6, Vertex{176.0f, 122.0f});
    got_edge[2] = {286.0f, 126.0f};
    std::vector<Vertex> got_name(6, Vertex{188.0f, 128.0f});
    got_name[2] = {195.0f, 142.0f};
    std::vector<Vertex> got_label(6, Vertex{188.0f, 128.0f});
    got_label[2] = {260.0f, 142.0f};
    std::vector<Vertex> got_clock(6, Vertex{8.0f, 8.0f});
    got_clock[2] = {60.0f, 56.0f};
    std::vector<Vertex> got(got_edge);
    got.insert(got.end(), got_name.begin(), got_name.end());
    got.insert(got.end(), got_label.begin(), got_label.end());
    got.insert(got.end(), got_clock.begin(), got_clock.end());
    mhfu::gpu::hud_scale::scale_quads(got, group_half, obtained);
    check(got[0].x == 176.0f && got[0].y == 122.0f && got[6].x == 188.0f && got[6].y == 128.0f && got[12].x == 188.0f &&
            got[12].y == 128.0f && got[20].x == 34.0f && got[20].y == 32.0f,
        "the item banner's name stays on the banner while status shrinks the clock");
    // The same window on the way down, still crossing that enlarged map.
    // Its edges are above the settled row, and the body is the taller panel
    // it starts as.
    Frame descending;
    std::vector<Vertex> descend_map(6, Vertex{276.0f, 70.0f});
    descend_map[2] = {336.0f, 130.0f};
    std::vector<Vertex> descend_clock(6, Vertex{8.0f, 8.0f});
    descend_clock[2] = {60.0f, 56.0f};
    descend_map.insert(descend_map.end(), descend_clock.begin(), descend_clock.end());
    mhfu::gpu::hud_scale::scale_quads(descend_map, map_half, descending, mhfu::gpu::hud_scale::kAreaMapTexture);
    std::vector<Vertex> slide_top(6, Vertex{176.0f, 64.0f});
    slide_top[2] = {286.0f, 68.0f};
    std::vector<Vertex> slide_bottom(6, Vertex{176.0f, 90.0f});
    slide_bottom[2] = {286.0f, 94.0f};
    std::vector<Vertex> slide_cap(6, Vertex{286.0f, 64.0f});
    slide_cap[2] = {302.0f, 88.0f};
    std::vector<Vertex> slide_icon(6, Vertex{248.0f, 70.0f});
    slide_icon[2] = {264.0f, 86.0f};
    std::vector<Vertex> slide_arrow(6, Vertex{290.0f, 80.0f});
    slide_arrow[2] = {304.0f, 94.0f};
    std::vector<Vertex> slide(slide_top);
    slide.insert(slide.end(), slide_bottom.begin(), slide_bottom.end());
    slide.insert(slide.end(), slide_cap.begin(), slide_cap.end());
    slide.insert(slide.end(), slide_icon.begin(), slide_icon.end());
    mhfu::gpu::hud_scale::scale_quads(slide, map_half, descending, 0x040CC000u);
    mhfu::gpu::hud_scale::scale_quads(slide_arrow, map_half, descending, mhfu::gpu::hud_scale::kMapIconTexture);
    check(slide[0].y == 64.0f && slide[6].y == 90.0f && slide[12].x == 286.0f && slide[12].y == 64.0f &&
            slide[18].x == 248.0f && slide[18].y == 70.0f && slide_arrow[0].x == 379.0f && slide_arrow[0].y == 56.0f,
        "the obtain window stays whole while it slides across the enlarged map");
    Frame slide_start;
    std::vector<Vertex> start_map(6, Vertex{316.0f, 32.0f});
    start_map[2] = {468.0f, 184.0f};
    std::vector<Vertex> start_body(6, Vertex{168.0f, 8.0f});
    start_body[2] = {312.0f, 32.0f};
    std::vector<Vertex> start_end(6, Vertex{312.0f, 8.0f});
    start_end[2] = {328.0f, 32.0f};
    std::vector<Vertex> start_mark(6, Vertex{400.0f, 80.0f});
    start_mark[2] = {414.0f, 94.0f};
    std::vector<Vertex> start_panel(start_map);
    start_panel.insert(start_panel.end(), start_body.begin(), start_body.end());
    start_panel.insert(start_panel.end(), start_end.begin(), start_end.end());
    mhfu::gpu::hud_scale::scale_quads(start_panel, map_half, slide_start);
    mhfu::gpu::hud_scale::scale_quads(start_mark, map_half, slide_start, mhfu::gpu::hud_scale::kMapIconTexture);
    check(start_panel[6].x == 168.0f && start_panel[6].y == 8.0f && start_panel[12].x == 312.0f &&
            start_panel[12].y == 8.0f && start_mark[0].x == 434.0f && start_mark[0].y == 56.0f,
        "the obtain window's panel stays whole at the start of the slide");
    // Without the Map item the same texels are drawn at sizes and places the
    // slot test misses. The window still shrinks toward the slot's top-right,
    // and the arrow, drawn after it, follows that corner.
    Frame bare_map;
    std::vector<Vertex> bare(6, Vertex{240.0f, 48.0f});
    bare[2] = {320.0f, 128.0f};
    std::vector<Vertex> bare_clock(6, Vertex{8.0f, 8.0f});
    bare_clock[2] = {60.0f, 56.0f};
    bare.insert(bare.end(), bare_clock.begin(), bare_clock.end());
    mhfu::gpu::hud_scale::scale_quads(bare, map_half, bare_map, mhfu::gpu::hud_scale::kAreaMapTexture);
    check(bare[0].x == 354.0f && bare[0].y == 40.0f && bare[2].x == 394.0f && bare[2].y == 80.0f,
        "a map window outside the slot shrinks toward the slot's top-right");
    std::vector<Vertex> bare_arrow(6, Vertex{260.0f, 70.0f});
    bare_arrow[2] = {278.0f, 88.0f};
    mhfu::gpu::hud_scale::scale_quads(bare_arrow, map_half, bare_map, 0x091874a0u);
    check(
        bare_arrow[0].x == 364.0f && bare_arrow[0].y == 51.0f && bare_arrow[2].x == 373.0f && bare_arrow[2].y == 60.0f,
        "the player's arrow follows a map window outside the slot");
    // The south of an area is drawn below the slot. On the bottom edge of
    // Forest and Hills zone 1 the window sits under y 240, and the map's edge
    // can clip it shorter than the usual 28. Both still shrink from the slot.
    Frame south;
    std::vector<Vertex> south_map(6, Vertex{424.0f, 216.0f});
    south_map[2] = {464.0f, 256.0f};
    std::vector<Vertex> south_clock(6, Vertex{8.0f, 8.0f});
    south_clock[2] = {60.0f, 56.0f};
    south_map.insert(south_map.end(), south_clock.begin(), south_clock.end());
    mhfu::gpu::hud_scale::scale_quads(south_map, map_half, south, mhfu::gpu::hud_scale::kAreaMapTexture);
    check(south_map[0].x == 446.0f && south_map[0].y == 124.0f && south_map[2].x == 466.0f && south_map[2].y == 144.0f,
        "a map window on the bottom edge of an area shrinks toward the slot's top-right");
    Frame south_clip;
    std::vector<Vertex> clipped(6, Vertex{430.0f, 236.0f});
    clipped[2] = {470.0f, 252.0f};
    clipped.insert(clipped.end(), south_clock.begin(), south_clock.end());
    mhfu::gpu::hud_scale::scale_quads(clipped, map_half, south_clip, mhfu::gpu::hud_scale::kAreaMapTexture);
    check(clipped[0].x == 449.0f && clipped[0].y == 134.0f && clipped[2].x == 469.0f && clipped[2].y == 142.0f,
        "a map window clipped by the bottom of the area still shrinks");
    Frame not_bare;
    std::vector<Vertex> foreign_zoom(6, Vertex{240.0f, 48.0f});
    foreign_zoom[2] = {320.0f, 128.0f};
    foreign_zoom.insert(foreign_zoom.end(), bare_clock.begin(), bare_clock.end());
    mhfu::gpu::hud_scale::scale_quads(foreign_zoom, map_half, not_bare, 0x09164420u);
    check(foreign_zoom[0].x == 240.0f && foreign_zoom[0].y == 48.0f && foreign_zoom[2].x == 320.0f &&
            foreign_zoom[2].y == 128.0f,
        "a window of that size from another texture is not the map");
    // Without the Map item the same 40 texels are drawn at 60 by 60. They go
    // back to the slot's placement, (316, 32) plus the texel, and the Map size
    // is applied there. The arrow's vertices are in that enlargement. The camp
    // icon is drawn at its own pixel size, so only its centre comes back.
    struct Marked {
        float x;
        float y;
        float u;
        float v;
    };
    Frame stretched;
    std::vector<Marked> wide_tex(6);
    wide_tex[0] = {276.0f, 70.0f, 99.0f, 47.0f};
    wide_tex[1] = {336.0f, 70.0f, 139.0f, 47.0f};
    wide_tex[2] = {336.0f, 130.0f, 139.0f, 87.0f};
    wide_tex[3] = {276.0f, 70.0f, 99.0f, 47.0f};
    wide_tex[4] = {336.0f, 130.0f, 139.0f, 87.0f};
    wide_tex[5] = {276.0f, 130.0f, 99.0f, 87.0f};
    std::vector<Marked> stretch_clock(6, Marked{8.0f, 8.0f, 0.0f, 0.0f});
    stretch_clock[2] = {60.0f, 56.0f, 0.0f, 0.0f};
    wide_tex.insert(wide_tex.end(), stretch_clock.begin(), stretch_clock.end());
    mhfu::gpu::hud_scale::scale_quads(wide_tex, map_half, stretched, mhfu::gpu::hud_scale::kAreaMapTexture);
    check(wide_tex[0].x == 441.5f && wide_tex[0].y == 55.5f && wide_tex[2].x == 461.5f && wide_tex[2].y == 75.5f,
        "a window drawn without the map item returns to the slot, then takes the map size");
    // The enlargement of the arrow that was (430, 94)-(444, 106) on the slot.
    std::vector<Vertex> stretch_arrow(6, Vertex{298.5f, 92.5f});
    stretch_arrow[2] = {319.5f, 110.5f};
    mhfu::gpu::hud_scale::scale_quads(stretch_arrow, map_half, stretched);
    check(stretch_arrow[0].x == 449.0f && stretch_arrow[0].y == 63.0f && stretch_arrow[2].x == 456.0f &&
            stretch_arrow[2].y == 69.0f,
        "the player's arrow on the enlarged map returns to the slot with the window");
    std::vector<Marked> camp_zoom(6);
    camp_zoom[0] = {313.5f, 97.0f, 48.0f, 244.0f};
    camp_zoom[1] = {325.5f, 97.0f, 60.0f, 244.0f};
    camp_zoom[2] = {325.5f, 109.0f, 60.0f, 256.0f};
    camp_zoom[3] = {313.5f, 97.0f, 48.0f, 244.0f};
    camp_zoom[4] = {325.5f, 109.0f, 60.0f, 256.0f};
    camp_zoom[5] = {313.5f, 109.0f, 48.0f, 256.0f};
    mhfu::gpu::hud_scale::scale_quads(camp_zoom, map_half, stretched, 0x091874a0u);
    check(camp_zoom[0].x == 453.0f && camp_zoom[0].y == 63.5f && camp_zoom[2].x == 459.0f && camp_zoom[2].y == 69.5f,
        "the camp icon keeps its pixel size and returns to the slot with the enlarged map");
    // With the Map item the same texels are already on the slot, one pixel per
    // texel. Recognising the enlargement must leave that window where it is.
    Frame with_map;
    std::vector<Marked> fitted(6);
    fitted[0] = {425.0f, 78.0f, 109.0f, 46.0f};
    fitted[1] = {465.0f, 78.0f, 149.0f, 46.0f};
    fitted[2] = {465.0f, 118.0f, 149.0f, 86.0f};
    fitted[3] = {425.0f, 78.0f, 109.0f, 46.0f};
    fitted[4] = {465.0f, 118.0f, 149.0f, 86.0f};
    fitted[5] = {425.0f, 118.0f, 109.0f, 86.0f};
    std::vector<Marked> fitted_clock(6, Marked{8.0f, 8.0f, 0.0f, 0.0f});
    fitted_clock[2] = {60.0f, 56.0f, 0.0f, 0.0f};
    fitted.insert(fitted.end(), fitted_clock.begin(), fitted_clock.end());
    mhfu::gpu::hud_scale::scale_quads(fitted, map_half, with_map, mhfu::gpu::hud_scale::kAreaMapTexture);
    check(fitted[0].x == 446.5f && fitted[0].y == 55.0f && fitted[2].x == 466.5f && fitted[2].y == 75.0f,
        "a window drawn with the map item stays on the slot's placement");
    Frame status_only;
    std::vector<Vertex> parked(map_window);
    mhfu::gpu::hud_scale::scale_quads(parked, group_half, status_only);
    check(parked[0].x == 371.0f && parked[0].y == 59.0f && parked[2].x == 411.0f && parked[2].y == 99.0f,
        "status size leaves the found-monster map window where the game drew it");
    // The corner window is sometimes 34 by 40, at (432, 76)-(466, 116).
    Frame corner_map;
    std::vector<Vertex> narrow(6, Vertex{432.0f, 76.0f});
    narrow[2] = {466.0f, 116.0f};
    std::vector<Vertex> narrow_clock(6, Vertex{8.0f, 8.0f});
    narrow_clock[2] = {60.0f, 56.0f};
    narrow.insert(narrow.end(), narrow_clock.begin(), narrow_clock.end());
    mhfu::gpu::hud_scale::scale_quads(narrow, map_half, corner_map, mhfu::gpu::hud_scale::kAreaMapTexture);
    check(narrow[0].x == 450.0f && narrow[0].y == 54.0f && narrow[2].x == 467.0f && narrow[2].y == 74.0f,
        "the 34 by 40 map window shrinks toward the slot's top-right");
    // The window slides inside the area-map slot as the hunter moves. With the
    // quest clock on screen, a spot between the traced samples is still the
    // map, and the mark on it follows. The clock's own size is unchanged here.
    Frame walking;
    std::vector<Vertex> walked(6, Vertex{400.0f, 100.0f});
    walked[2] = {440.0f, 140.0f};
    std::vector<Vertex> walked_mark(6, Vertex{412.0f, 112.0f});
    walked_mark[2] = {426.0f, 124.0f};
    std::vector<Vertex> walked_clock(6, Vertex{8.0f, 8.0f});
    walked_clock[2] = {60.0f, 56.0f};
    std::vector<Vertex> walked_map(walked);
    walked_map.insert(walked_map.end(), walked_mark.begin(), walked_mark.end());
    walked_map.insert(walked_map.end(), walked_clock.begin(), walked_clock.end());
    mhfu::gpu::hud_scale::scale_quads(walked_map, map_half, walking, mhfu::gpu::hud_scale::kAreaMapTexture);
    check(
        walked_map[0].x == 434.0f && walked_map[0].y == 66.0f && walked_map[2].x == 454.0f && walked_map[2].y == 86.0f,
        "a map window between the traced spots shrinks toward the slot's top-right");
    check(
        walked_map[6].x == 440.0f && walked_map[6].y == 72.0f && walked_map[8].x == 447.0f && walked_map[8].y == 78.0f,
        "the mark on a moving map window stays on it");
    check(walked_map[12].x == 8.0f && walked_map[12].y == 8.0f, "the quest clock is not moved by the map's size");
    // Each area draws its map at its own size. A 72 by 48 window is neither the
    // 152 tile nor the traced 40-wide window; it is still the map, and the
    // arrow on it follows the slot's top-right.
    Frame area_bounds;
    std::vector<Vertex> wide(6, Vertex{380.0f, 60.0f});
    wide[2] = {452.0f, 108.0f};
    std::vector<Vertex> wide_arrow(6, Vertex{400.0f, 72.0f});
    wide_arrow[2] = {414.0f, 86.0f};
    std::vector<Vertex> wide_clock(6, Vertex{8.0f, 8.0f});
    wide_clock[2] = {60.0f, 56.0f};
    std::vector<Vertex> wide_map(wide);
    wide_map.insert(wide_map.end(), wide_arrow.begin(), wide_arrow.end());
    wide_map.insert(wide_map.end(), wide_clock.begin(), wide_clock.end());
    mhfu::gpu::hud_scale::scale_quads(wide_map, map_half, area_bounds, mhfu::gpu::hud_scale::kAreaMapTexture);
    check(wide_map[0].x == 424.0f && wide_map[0].y == 46.0f && wide_map[2].x == 460.0f && wide_map[2].y == 70.0f &&
            wide_map[6].x == 434.0f && wide_map[6].y == 52.0f && wide_map[12].x == 8.0f,
        "a map of another area's size shrinks toward the slot's top-right, arrow included");
    Frame not_area;
    std::vector<Vertex> foreign(wide);
    foreign.insert(foreign.end(), wide_clock.begin(), wide_clock.end());
    mhfu::gpu::hud_scale::scale_quads(foreign, map_half, not_area, 0x040CC000u);
    check(foreign[0].x == 380.0f && foreign[0].y == 60.0f && foreign[2].x == 452.0f && foreign[2].y == 108.0f,
        "a panel of that size from another texture is not the map");
    // Two quads of one map share the slot's top-right, rather than whichever
    // piece was last or the union of the two.
    Frame pieces;
    std::vector<Vertex> left_piece(6, Vertex{340.0f, 50.0f});
    left_piece[2] = {400.0f, 130.0f};
    std::vector<Vertex> right_piece(6, Vertex{400.0f, 40.0f});
    right_piece[2] = {460.0f, 140.0f};
    std::vector<Vertex> piece_arrow(6, Vertex{410.0f, 80.0f});
    piece_arrow[2] = {424.0f, 94.0f};
    std::vector<Vertex> piece_clock(6, Vertex{8.0f, 8.0f});
    piece_clock[2] = {60.0f, 56.0f};
    std::vector<Vertex> piece_map(left_piece);
    piece_map.insert(piece_map.end(), right_piece.begin(), right_piece.end());
    piece_map.insert(piece_map.end(), piece_arrow.begin(), piece_arrow.end());
    piece_map.insert(piece_map.end(), piece_clock.begin(), piece_clock.end());
    mhfu::gpu::hud_scale::scale_quads(piece_map, map_half, pieces, mhfu::gpu::hud_scale::kAreaMapTexture);
    check(piece_map[0].x == 404.0f && piece_map[0].y == 41.0f && piece_map[6].x == 434.0f && piece_map[6].y == 36.0f &&
            piece_map[12].x == 439.0f && piece_map[12].y == 56.0f,
        "pieces of one map share the slot's top-right, and the arrow follows it");
    // The arrow turns. Near the top of the large map its box leaves the slot,
    // and it used to be drawn at the size the game chose.
    Frame edge_frame;
    std::vector<Vertex> edge_map(6, Vertex{316.0f, 32.0f});
    edge_map[2] = {468.0f, 184.0f};
    std::vector<Vertex> edge_arrow(6, Vertex{430.0f, 24.0f});
    edge_arrow[2] = {444.0f, 40.0f};
    std::vector<Vertex> edge_all(edge_map);
    edge_all.insert(edge_all.end(), edge_arrow.begin(), edge_arrow.end());
    mhfu::gpu::hud_scale::scale_quads(edge_all, map_half, edge_frame);
    check(edge_all[0].x == 392.0f && edge_all[0].y == 32.0f && edge_all[6].x == 449.0f && edge_all[6].y == 28.0f &&
            edge_all[8].x == 456.0f && edge_all[8].y == 36.0f,
        "the player's arrow still follows the map when its box crosses the top edge");
    // The arrow is its own draw, after the map, and again on the next frame
    // before the map is submitted.
    edge_frame.begin();
    std::vector<Vertex> next_arrow(6, Vertex{430.0f, 24.0f});
    next_arrow[2] = {444.0f, 40.0f};
    mhfu::gpu::hud_scale::scale_quads(next_arrow, map_half, edge_frame);
    check(
        next_arrow[0].x == 449.0f && next_arrow[0].y == 28.0f && next_arrow[2].x == 456.0f && next_arrow[2].y == 36.0f,
        "the arrow drawn before the map this frame still follows last frame's bounds");
    // A mark inside the slot but off the window that was drawn still shrinks
    // from the slot's top-right. The window moving off it used to snap the
    // mark back to the size the game drew.
    Frame off_window;
    std::vector<Vertex> off_map(6, Vertex{400.0f, 100.0f});
    off_map[2] = {440.0f, 140.0f};
    std::vector<Vertex> off_mark(6, Vertex{320.0f, 40.0f});
    off_mark[2] = {334.0f, 54.0f};
    std::vector<Vertex> off_clock(6, Vertex{8.0f, 8.0f});
    off_clock[2] = {60.0f, 56.0f};
    std::vector<Vertex> off_all(off_map);
    off_all.insert(off_all.end(), off_mark.begin(), off_mark.end());
    off_all.insert(off_all.end(), off_clock.begin(), off_clock.end());
    mhfu::gpu::hud_scale::scale_quads(off_all, map_half, off_window, mhfu::gpu::hud_scale::kAreaMapTexture);
    check(off_all[6].x == 394.0f && off_all[6].y == 36.0f && off_all[8].x == 401.0f && off_all[8].y == 43.0f &&
            off_all[0].x == 434.0f,
        "a mark in the slot still shrinks when the window has moved off it");
    // The camp icon is about the size of a map window and is drawn on its own,
    // from another texture. It follows the map. It is not itself a window.
    Frame camp_mark;
    std::vector<Vertex> camp_map(6, Vertex{316.0f, 32.0f});
    camp_map[2] = {468.0f, 184.0f};
    mhfu::gpu::hud_scale::scale_quads(camp_map, map_half, camp_mark, mhfu::gpu::hud_scale::kAreaMapTexture);
    std::vector<Vertex> house(6, Vertex{400.0f, 90.0f});
    house[2] = {432.0f, 122.0f};
    mhfu::gpu::hud_scale::scale_quads(house, map_half, camp_mark, 0x091874a0u);
    check(house[0].x == 434.0f && house[0].y == 61.0f && house[2].x == 450.0f && house[2].y == 77.0f,
        "the camp icon on the map shrinks with the map");
    Frame house_alone;
    std::vector<Vertex> lone_house(6, Vertex{400.0f, 90.0f});
    lone_house[2] = {432.0f, 122.0f};
    mhfu::gpu::hud_scale::scale_quads(lone_house, map_half, house_alone, 0x091874a0u);
    check(
        lone_house[0].x == 400.0f && lone_house[0].y == 90.0f && lone_house[2].x == 432.0f && lone_house[2].y == 122.0f,
        "the camp icon is not a map window on its own");
    // The camp icon stays where the game draws it while the window slides.
    // Both windows shrink it from the slot corner, so running does not snap
    // it back to the game's size and does not drag it along with the window.
    const auto camp_at = [&](float x0, float y0, float x1, float y1) {
        Frame frame;
        std::vector<Vertex> window(6, Vertex{x0, y0});
        window[2] = {x1, y1};
        std::vector<Vertex> clock(6, Vertex{8.0f, 8.0f});
        clock[2] = {60.0f, 56.0f};
        window.insert(window.end(), clock.begin(), clock.end());
        mhfu::gpu::hud_scale::scale_quads(window, map_half, frame, mhfu::gpu::hud_scale::kAreaMapTexture);
        std::vector<Vertex> icon(6, Vertex{320.0f, 140.0f});
        icon[2] = {352.0f, 172.0f};
        mhfu::gpu::hud_scale::scale_quads(icon, map_half, frame, 0x091874a0u);
        return icon;
    };
    const std::vector<Vertex> camp_near = camp_at(425.0f, 78.0f, 465.0f, 118.0f);
    const std::vector<Vertex> camp_far = camp_at(371.0f, 59.0f, 411.0f, 99.0f);
    check(camp_near[0].x == 394.0f && camp_near[0].y == 86.0f && camp_near[2].x == 410.0f && camp_near[2].y == 102.0f &&
            camp_far[0].x == camp_near[0].x && camp_far[0].y == camp_near[0].y && camp_far[2].x == camp_near[2].x &&
            camp_far[2].y == camp_near[2].y,
        "the camp icon keeps the map's size from the slot corner as the window slides off it");
    // The same panel in the shop, with no quest clock or health bar. The
    // label drawn inside the map's rectangle stays with it.
    Frame shop_panel;
    std::vector<Vertex> panel(6, Vertex{400.0f, 100.0f});
    panel[2] = {440.0f, 140.0f};
    std::vector<Vertex> shop_label(6, Vertex{350.0f, 80.0f});
    shop_label[2] = {390.0f, 96.0f};
    std::vector<Vertex> shop_row(panel);
    shop_row.insert(shop_row.end(), shop_label.begin(), shop_label.end());
    mhfu::gpu::hud_scale::scale_quads(shop_row, map_half, shop_panel);
    check(shop_row[0].x == 400.0f && shop_row[0].y == 100.0f && shop_row[2].x == 440.0f && shop_row[2].y == 140.0f &&
            shop_row[6].x == 350.0f && shop_row[6].y == 80.0f,
        "a shop panel in the map's slot is left as the game drew it");
    // The same panel while returning to town: the quest clock is up, and the
    // prompt sits in the map's slot, but it is not drawn from the area-map
    // texture. The label beside it stays with the panel.
    Frame town;
    std::vector<Vertex> town_clock(6, Vertex{8.0f, 8.0f});
    town_clock[2] = {60.0f, 56.0f};
    std::vector<Vertex> town_row(panel);
    town_row.insert(town_row.end(), shop_label.begin(), shop_label.end());
    town_row.insert(town_row.end(), town_clock.begin(), town_clock.end());
    mhfu::gpu::hud_scale::scale_quads(town_row, map_half, town, 0x040CC000u);
    check(town_row[0].x == 400.0f && town_row[0].y == 100.0f && town_row[2].x == 440.0f && town_row[2].y == 140.0f &&
            town_row[6].x == 350.0f && town_row[6].y == 80.0f && town_row[12].x == 8.0f,
        "the return-to-town prompt stays put while the quest clock is up");
    Frame camp_icon;
    std::vector<Vertex> icon(6, Vertex{400.0f, 100.0f});
    icon[2] = {424.0f, 124.0f};
    mhfu::gpu::hud_scale::scale_quads(icon, map_half, camp_icon);
    check(icon[0].x == 400.0f && icon[0].y == 100.0f && icon[2].x == 424.0f && icon[2].y == 124.0f,
        "a smaller icon in the map slot is not the moving window");
    Frame stray_mark;
    std::vector<Vertex> lone_mark(6, Vertex{385.0f, 74.0f});
    lone_mark[2] = {399.0f, 86.0f};
    mhfu::gpu::hud_scale::scale_quads(lone_mark, map_half, stray_mark);
    check(lone_mark[0].x == 385.0f && lone_mark[0].y == 74.0f,
        "a mark in that spot without the map window is not scaled");
    // The yellow eye, 12 by 16 at the right end of the name, once a monster
    // is found. It grows from the clock with the name and the bars.
    Frame eye_frame;
    std::vector<Vertex> eye(6, Vertex{104.0f, 56.0f});
    eye[2] = {116.0f, 72.0f};
    mhfu::gpu::hud_scale::scale_quads(eye, group_half, eye_frame);
    check(eye[0].x == 56.0f && eye[0].y == 32.0f && eye[2].x == 62.0f && eye[2].y == 40.0f,
        "the yellow eye shrinks toward the clock with the name and health");
    // Status icons sit on that row, past the plate: poison is about 12 by 14,
    // and a later one steps right of the queued-note mark. The stars over a
    // stunned hunter are larger and are not this row.
    Frame poison_icon;
    std::vector<Vertex> poison(6, Vertex{116.0f, 56.0f});
    poison[2] = {128.0f, 70.0f};
    mhfu::gpu::hud_scale::scale_quads(poison, group_half, poison_icon);
    check(poison[0].x == 62.0f && poison[0].y == 32.0f && poison[2].x == 68.0f && poison[2].y == 39.0f,
        "a status icon beside the name shrinks toward the clock");
    Frame later_icon;
    std::vector<Vertex> stench(6, Vertex{152.0f, 56.0f});
    stench[2] = {164.0f, 72.0f};
    mhfu::gpu::hud_scale::scale_quads(stench, group_half, later_icon);
    check(stench[0].x == 80.0f && stench[0].y == 32.0f && stench[2].x == 86.0f && stench[2].y == 40.0f,
        "a later status icon on the name row shrinks toward the clock");
    check(mhfu::gpu::hud_scale::piece_of({128.0f, 56.0f, 140.0f, 72.0f}) == mhfu::gpu::hud_scale::Piece::LongSword,
        "the queued-note mark stays with the horn");
    Frame below_name;
    std::vector<Vertex> dropped(6, Vertex{116.0f, 90.0f});
    dropped[2] = {128.0f, 104.0f};
    mhfu::gpu::hud_scale::scale_quads(dropped, group_half, below_name);
    check(dropped[0].x == 62.0f && dropped[0].y == 49.0f && dropped[2].x == 68.0f && dropped[2].y == 56.0f,
        "a teammate's buff below the first name row shrinks toward the clock");
    // A cat's badge sits beside the cat's name and can start left of the
    // hunter's own status icons. The yellow eye and a buff on a later row
    // are the same marks.
    Frame cat_badge;
    std::vector<Vertex> badge(6, Vertex{96.0f, 56.0f});
    badge[2] = {112.0f, 72.0f};
    mhfu::gpu::hud_scale::scale_quads(badge, group_half, cat_badge);
    check(badge[0].x == 52.0f && badge[0].y == 32.0f && badge[2].x == 60.0f && badge[2].y == 40.0f,
        "a cat's badge left of the status icons shrinks toward the clock");
    Frame mate_eye;
    std::vector<Vertex> mate(6, Vertex{104.0f, 116.0f});
    mate[2] = {116.0f, 132.0f};
    mhfu::gpu::hud_scale::scale_quads(mate, group_half, mate_eye);
    check(mate[0].x == 56.0f && mate[0].y == 62.0f && mate[2].x == 62.0f && mate[2].y == 70.0f,
        "a teammate's yellow eye shrinks toward the clock");
    Frame mate_buff;
    std::vector<Vertex> mate_icon(6, Vertex{240.0f, 116.0f});
    mate_icon[2] = {252.0f, 130.0f};
    mhfu::gpu::hud_scale::scale_quads(mate_icon, group_half, mate_buff);
    check(mate_icon[0].x == 124.0f && mate_icon[0].y == 62.0f && mate_icon[2].x == 130.0f && mate_icon[2].y == 69.0f,
        "a buff past the first row's icons shrinks toward the clock");
    // The cat's face in the badge column, under the cat's name. It is not a
    // buff (12 by 11, a pixel short), and it follows the clock with the plates
    // beside it, keeping the place the game gave it. The face alone does not
    // open Status, or a shop icon would.
    Frame cat_face;
    cat_face.note({8.0f, 8.0f, 60.0f, 56.0f});
    std::vector<Vertex> face(6, Vertex{8.0f, 76.0f});
    face[2] = {20.0f, 87.0f};
    mhfu::gpu::hud_scale::scale_quads(face, group_half, cat_face);
    check(face[0].x == 8.0f && face[0].y == 42.0f && face[2].x == 14.0f && face[2].y == 47.5f,
        "the cat's face shrinks toward the clock");
    Frame no_clock;
    std::vector<Vertex> lone_face(6, Vertex{8.0f, 76.0f});
    lone_face[2] = {20.0f, 87.0f};
    mhfu::gpu::hud_scale::scale_quads(lone_face, group_half, no_clock);
    check(lone_face[0].x == 8.0f && lone_face[0].y == 76.0f && lone_face[2].x == 20.0f && lone_face[2].y == 87.0f,
        "the cat's face alone does not open Status");
    // The paint on screen is about 10 by 7. Drawn with the cat's name, it
    // stays under that name as both shrink.
    Frame painted;
    painted.note({8.0f, 8.0f, 60.0f, 56.0f});
    std::vector<Vertex> named(6, Vertex{40.0f, 72.0f});
    named[2] = {48.0f, 86.0f};
    std::vector<Vertex> paint(6, Vertex{9.0f, 82.0f});
    paint[2] = {20.0f, 88.0f};
    named.insert(named.end(), paint.begin(), paint.end());
    mhfu::gpu::hud_scale::scale_quads(named, group_half, painted);
    check(named[0].x == 24.0f && named[0].y == 40.0f && named[2].x == 28.0f && named[2].y == 47.0f &&
            named[6].x == 8.5f && named[6].y == 45.0f && named[8].x == 14.0f && named[8].y == 48.0f,
        "the painted face stays under the cat's name");
    const Scales status_75{1.0f, 0.75f, 1.0f, 0.75f, 0.75f, 0.75f};
    Frame status_three_quarters;
    status_three_quarters.note({8.0f, 8.0f, 60.0f, 56.0f});
    std::vector<Vertex> quarter(6, Vertex{9.0f, 82.0f});
    quarter[2] = {20.0f, 89.0f};
    mhfu::gpu::hud_scale::scale_quads(quarter, status_75, status_three_quarters);
    check(quarter[0].x == 8.75f && quarter[0].y == 63.5f && quarter[2].x == 17.0f && quarter[2].y == 68.75f,
        "at 75% the cat's face shrinks toward the clock by the same amount");
    // The face can be a sprite on the map-icon atlas. It is in the badge
    // column, so it is still the party list; a mark from that atlas on the
    // right of the picture is not.
    Frame atlas_face;
    atlas_face.note({8.0f, 8.0f, 60.0f, 56.0f});
    std::vector<Vertex> atlas(6, Vertex{9.0f, 82.0f});
    atlas[2] = {20.0f, 89.0f};
    mhfu::gpu::hud_scale::scale_quads(atlas, status_75, atlas_face, mhfu::gpu::hud_scale::kMapIconTexture);
    check(atlas[0].x == 8.75f && atlas[0].y == 63.5f && atlas[2].x == 17.0f && atlas[2].y == 68.75f,
        "a cat face on the icon atlas follows the clock");
    check(!mhfu::gpu::hud_scale::is_party_column_icon({400.0f, 80.0f, 416.0f, 96.0f}),
        "a map mark on the right is not a party-column icon");
    check(mhfu::gpu::hud_scale::is_party_column_icon({8.0f, 78.0f, 16.0f, 92.0f}),
        "the face's letter-sized cell in the weapon column is a party-column icon");
    check(!mhfu::gpu::hud_scale::is_party_column_icon({32.0f, 80.0f, 40.0f, 94.0f}),
        "a name letter to the right of the weapon column is not a party-column icon");
    check(mhfu::gpu::hud_scale::is_party_column_icon({8.0f, 64.0f, 24.0f, 80.0f}),
        "an icon that crosses the name plate is a party-column icon");
    Frame plate_tile;
    plate_tile.note({8.0f, 8.0f, 60.0f, 56.0f});
    std::vector<Vertex> tile(6, Vertex{8.0f, 64.0f});
    tile[2] = {20.0f, 72.0f};
    mhfu::gpu::hud_scale::scale_quads(tile, group_half, plate_tile);
    check(tile[0].x == 8.0f && tile[0].y == 36.0f && tile[2].x == 14.0f && tile[2].y == 40.0f,
        "a name-plate tile still follows the plate");
    check(!mhfu::gpu::hud_scale::is_party_column_icon({8.0f, 64.0f, 20.0f, 72.0f}),
        "a tile of the name plate is not a party-column icon");
    // The dark badge behind the cat is a tile of that name's plate. It stays
    // there and takes Status, the same way the hunter's badge does.
    check(!mhfu::gpu::hud_scale::is_party_column_icon({4.0f, 78.0f, 20.0f, 94.0f}),
        "the badge behind a later name is not a party-column icon");
    Frame later_badge;
    later_badge.note({8.0f, 8.0f, 60.0f, 56.0f});
    std::vector<Vertex> later(6, Vertex{4.0f, 78.0f});
    later[2] = {20.0f, 94.0f};
    mhfu::gpu::hud_scale::scale_quads(later, group_half, later_badge);
    check(later[0].x == 6.0f && later[0].y == 43.0f && later[2].x == 14.0f && later[2].y == 51.0f,
        "the badge behind a later name scales in place with Status");
    check(!mhfu::gpu::hud_scale::is_party_column_icon({4.0f, 56.0f, 36.0f, 68.0f}),
        "a weapon mark on the name plate is not a party-column icon");
    // A cell larger than the paint is the same icon, and a lower name's
    // face follows the clock from that row.
    Frame cat_cell;
    cat_cell.note({8.0f, 8.0f, 60.0f, 56.0f});
    std::vector<Vertex> cell(6, Vertex{0.0f, 72.0f});
    cell[2] = {32.0f, 96.0f};
    mhfu::gpu::hud_scale::scale_quads(cell, group_half, cat_cell);
    check(cell[0].x == 4.0f && cell[0].y == 40.0f && cell[2].x == 20.0f && cell[2].y == 52.0f,
        "the cat's cell shrinks toward the clock");
    Frame lower_face;
    lower_face.note({8.0f, 8.0f, 60.0f, 56.0f});
    std::vector<Vertex> lower(6, Vertex{8.0f, 108.0f});
    lower[2] = {20.0f, 119.0f};
    mhfu::gpu::hud_scale::scale_quads(lower, group_half, lower_face);
    check(lower[0].x == 8.0f && lower[0].y == 58.0f && lower[2].x == 14.0f && lower[2].y == 63.5f,
        "a cat face under a lower name shrinks toward the clock");
    // The same face drawn outside the through-mode HUD (a small orthographic
    // draw) takes the same place. Nothing moves before the clock is up, and
    // a quad outside the badge column stays.
    std::vector<Vertex> ortho(6, Vertex{8.0f, 76.0f});
    ortho[2] = {20.0f, 87.0f};
    std::vector<Vertex> elsewhere(6, Vertex{200.0f, 76.0f});
    elsewhere[2] = {212.0f, 87.0f};
    ortho.insert(ortho.end(), elsewhere.begin(), elsewhere.end());
    std::vector<Vertex> ortho_early = ortho;
    Frame ortho_frame;
    check(!mhfu::gpu::hud_scale::place_party_icons(ortho_early, group_half, ortho_frame) && ortho_early[0].y == 76.0f,
        "a party-column icon outside the through-mode HUD waits for the clock");
    ortho_frame.note({8.0f, 8.0f, 60.0f, 56.0f});
    check(mhfu::gpu::hud_scale::place_party_icons(ortho, group_half, ortho_frame) && ortho[0].x == 8.0f &&
            ortho[0].y == 42.0f && ortho[2].x == 14.0f && ortho[2].y == 47.5f && ortho[6].x == 200.0f &&
            ortho[6].y == 76.0f,
        "a party-column icon outside the through-mode HUD follows the clock, and nothing else moves");
    // Four hunters. Each weapon badge already sits on its name. None of them
    // is the one cat that accompanies the player.
    Frame four_plates;
    four_plates.note({8.0f, 8.0f, 60.0f, 56.0f});
    std::vector<Vertex> plates;
    for (int row = 1; row < 4; ++row) {
        const float y = 56.0f + 20.0f * static_cast<float>(row);
        std::vector<Vertex> mark(6, Vertex{4.0f, y});
        mark[2] = {36.0f, y + 12.0f};
        plates.insert(plates.end(), mark.begin(), mark.end());
    }
    mhfu::gpu::hud_scale::scale_quads(plates, group_half, four_plates);
    check(plates[0].x == 6.0f && plates[0].y == 42.0f && plates[2].x == 22.0f && plates[2].y == 48.0f &&
            plates[6].x == 6.0f && plates[6].y == 52.0f && plates[8].x == 22.0f && plates[8].y == 58.0f &&
            plates[12].x == 6.0f && plates[12].y == 62.0f && plates[14].x == 22.0f && plates[14].y == 68.0f,
        "each hunter's weapon badge shrinks from the clock like the first");
    check(!mhfu::gpu::hud_scale::is_party_column_icon({4.0f, 76.0f, 36.0f, 88.0f}) &&
            !mhfu::gpu::hud_scale::is_party_column_icon({4.0f, 96.0f, 36.0f, 108.0f}) &&
            !mhfu::gpu::hud_scale::is_party_column_icon({4.0f, 116.0f, 36.0f, 128.0f}),
        "the other hunters' weapon badges are not party-column icons");
    // A teammate's health line, traced at (26, 90)-(98, 92) under the second
    // name, whose plate is (24, 76)-(96, 92). Two pixels tall, it still moves
    // with that plate, so its bottom stays on the plate's bottom.
    const auto status_125 = mhfu::gpu::hud_scale::Scales::from_percent(100u, 125u, 100u);
    Frame health_line;
    health_line.note({8.0f, 8.0f, 60.0f, 56.0f});
    std::vector<Vertex> line(6, Vertex{26.0f, 90.0f});
    line[2] = {98.0f, 92.0f};
    std::vector<Vertex> line_plate(6, Vertex{24.0f, 76.0f});
    line_plate[2] = {96.0f, 92.0f};
    line.insert(line.end(), line_plate.begin(), line_plate.end());
    mhfu::gpu::hud_scale::scale_quads(line, status_125, health_line);
    check(line[0].x == 30.5f && line[0].y == 110.5f && line[2].x == 120.5f && line[2].y == 113.0f &&
            line[6].x == 28.0f && line[6].y == 93.0f && line[8].x == 118.0f && line[8].y == 113.0f,
        "a teammate's health line moves with its name plate at 125%");
    std::vector<Vertex> short_line(6, Vertex{26.0f, 110.0f});
    short_line[2] = {40.0f, 112.0f};
    mhfu::gpu::hud_scale::scale_quads(short_line, group_half, health_line);
    check(short_line[0].x == 17.0f && short_line[0].y == 59.0f && short_line[2].x == 24.0f &&
            short_line[2].y == 60.0f,
        "a low health line on the third name shrinks toward the clock with it");
    check(mhfu::gpu::hud_scale::piece_of({26.0f, 90.0f, 98.0f, 92.0f}) == mhfu::gpu::hud_scale::Piece::Vitals,
        "the health line is part of Status");
    check(mhfu::gpu::hud_scale::piece_of({200.0f, 90.0f, 272.0f, 92.0f}) == mhfu::gpu::hud_scale::Piece::None &&
            mhfu::gpu::hud_scale::piece_of({26.0f, 160.0f, 98.0f, 162.0f}) == mhfu::gpu::hud_scale::Piece::None,
        "a line of that shape away from the name plates is not one");
    // The name over another hunter crosses the party list in the same 8 by
    // 14 letters. Traced at (55, 77), (60, 56), (26, 99) and (49, 104); the
    // list's own letters start at y 58 and 78.
    using mhfu::gpu::hud_scale::piece_of;
    using mhfu::gpu::hud_scale::Piece;
    check(piece_of({26.0f, 58.0f, 34.0f, 72.0f}) == Piece::Vitals && piece_of({42.0f, 78.0f, 50.0f, 92.0f}) == Piece::Vitals,
        "the party list's own letters are Status");
    check(piece_of({55.0f, 77.0f, 63.0f, 91.0f}) == Piece::None && piece_of({26.0f, 99.0f, 34.0f, 113.0f}) == Piece::None &&
            piece_of({49.0f, 104.0f, 57.0f, 118.0f}) == Piece::None && piece_of({60.0f, 63.0f, 68.0f, 77.0f}) == Piece::None,
        "an overhead name's letters off the party rows are not Status");
    check(piece_of({63.0f, 116.0f, 71.0f, 130.0f}) == Piece::None && piece_of({46.0f, 58.0f, 54.0f, 72.0f}) == Piece::None &&
            piece_of({140.0f, 72.0f, 148.0f, 86.0f}) == Piece::None && piece_of({21.0f, 110.0f, 29.0f, 124.0f}) == Piece::None,
        "nor on a row but off the letters' 8-pixel grid, past the Felyne's row, or at the badge column's edge");
    check(piece_of({160.0f, 56.0f, 168.0f, 70.0f}) == Piece::Vitals && piece_of({120.0f, 78.0f, 128.0f, 92.0f}) == Piece::Vitals,
        "a cat's name beside its hunter, past the plate, is still the list");
    check(piece_of({96.0f, 76.0f, 104.0f, 92.0f}) == Piece::Vitals && piece_of({8.0f, 78.0f, 16.0f, 92.0f}) == Piece::Vitals,
        "a plate's 8 by 16 end and the cat's letter-sized cell stay Status");
    Frame overhead;
    overhead.note({8.0f, 8.0f, 60.0f, 56.0f});
    std::vector<Vertex> passing(6, Vertex{55.0f, 77.0f});
    passing[2] = {63.0f, 91.0f};
    mhfu::gpu::hud_scale::scale_quads(passing, status_125, overhead);
    check(passing[0].x == 55.0f && passing[0].y == 77.0f && passing[2].x == 63.0f && passing[2].y == 91.0f,
        "an overhead name crossing the party list stays where the game drew it");
    {
        // A Felyne named "Ken": K at x 26, then e and n 7 apart, each its own
        // draw. Only K is on the grid; the rest follow it. The same letters
        // with no K before them, or starting a pixel off, are not the list.
        const auto letter = [](float x, float y) {
            std::vector<Vertex> q(6, Vertex{x, y});
            q[2] = {x + 7.0f, y + 14.0f};
            return q;
        };
        Frame cat;
        cat.note({8.0f, 8.0f, 60.0f, 56.0f});
        std::vector<Vertex> k = letter(26.0f, 78.0f);
        std::vector<Vertex> e = letter(33.0f, 78.0f);
        std::vector<Vertex> n = letter(40.0f, 78.0f);
        std::vector<mhfu::gpu::hud_scale::Decision> decided;
        mhfu::gpu::hud_scale::scale_quads(k, status_125, cat);
        mhfu::gpu::hud_scale::scale_quads(e, status_125, cat, 0x040CC000u, mhfu::gpu::hud::Owner::Unknown, &decided);
        mhfu::gpu::hud_scale::scale_quads(n, status_125, cat);
        check(k[0].x != 26.0f && e[0].x != 33.0f && n[0].x != 40.0f && e[0].x == k[2].x && n[0].x == e[2].x &&
                e[0].y == k[0].y && n[0].y == k[0].y,
            "a cat's narrow letters follow the first one with the plate");
        check(decided.size() == 1u && decided[0].reason == mhfu::gpu::hud_scale::Reason::PartyName,
            "a letter that continues a name is said to be one");
        std::vector<Vertex> other_row = letter(33.0f, 98.0f);
        std::vector<Vertex> gap = letter(41.0f, 78.0f);
        mhfu::gpu::hud_scale::scale_quads(other_row, status_125, cat);
        mhfu::gpu::hud_scale::scale_quads(gap, status_125, cat);
        check(other_row[0].x == 33.0f && other_row[0].y == 98.0f && gap[0].x == 41.0f,
            "a letter on another row, or not touching the name, stays where the game drew it");
        cat.begin();
        cat.note({8.0f, 8.0f, 60.0f, 56.0f});
        std::vector<Vertex> next_frame = letter(33.0f, 78.0f);
        mhfu::gpu::hud_scale::scale_quads(next_frame, status_125, cat);
        check(next_frame[0].x == 33.0f, "a name's end is forgotten with the frame");
    }

    {
    // A window over the area map, traced at (344, 48)-(456, 160): the side,
    // the body and the corners from the mark atlas all stay as drawn, while
    // the map itself and its marks still take the Map size without it.
    const auto map_60 = mhfu::gpu::hud_scale::Scales::from_percent(60u, 100u, 100u);
    const auto quad = [](float x0, float y0, float x1, float y1) {
        std::vector<Vertex> q(6, Vertex{x0, y0});
        q[2] = {x1, y1};
        return q;
    };
    Frame with_panel;
    with_panel.note({8.0f, 8.0f, 60.0f, 56.0f});
    std::vector<Vertex> map_window = quad(333.0f, 148.0f, 373.0f, 182.0f);
    mhfu::gpu::hud_scale::scale_quads(map_window, map_60, with_panel, mhfu::gpu::hud_scale::kAreaMapTexture);
    check(map_window[0].x != 333.0f, "the map's own window takes the Map size");
    std::vector<Vertex> mark = quad(345.0f, 162.0f, 361.0f, 177.0f);
    Frame mark_frame = with_panel;
    mhfu::gpu::hud_scale::scale_quads(mark, map_60, mark_frame, mhfu::gpu::hud_scale::kMapIconTexture);
    check(mark[0].x != 345.0f, "a mark on the map takes the Map size while no window is over it");
    std::vector<Vertex> panel = quad(344.0f, 48.0f, 360.0f, 160.0f);
    const std::vector<Vertex> body = quad(360.0f, 48.0f, 456.0f, 160.0f);
    const std::vector<Vertex> corner = quad(344.0f, 47.0f, 357.0f, 61.0f);
    panel.insert(panel.end(), body.begin(), body.end());
    mhfu::gpu::hud_scale::scale_quads(panel, map_60, with_panel, 0x09165C20u);
    check(panel[0].x == 344.0f && panel[0].y == 48.0f && panel[8].x == 456.0f && panel[8].y == 160.0f,
        "a window over the map keeps its side and body where the game drew them");
    std::vector<Vertex> panel_corner = corner;
    mhfu::gpu::hud_scale::scale_quads(panel_corner, map_60, with_panel, mhfu::gpu::hud_scale::kMapIconTexture);
    check(panel_corner[0].x == 344.0f && panel_corner[0].y == 47.0f, "the window's corner from the mark atlas stays");
    with_panel.begin();
    std::vector<Vertex> next_corner = corner;
    mhfu::gpu::hud_scale::scale_quads(next_corner, map_60, with_panel, mhfu::gpu::hud_scale::kMapIconTexture);
    check(next_corner[0].x == 344.0f, "and stays in the next frame, drawn before the window's body");
    std::vector<Vertex> still_map = quad(333.0f, 148.0f, 373.0f, 182.0f);
    mhfu::gpu::hud_scale::scale_quads(still_map, map_60, with_panel, mhfu::gpu::hud_scale::kAreaMapTexture);
    check(still_map[0].x != 333.0f, "the map's own window under it still takes the Map size");
    check(!mhfu::gpu::hud_scale::is_map_panel({312.0f, 128.0f, 472.0f, 264.0f}, 0u),
        "the ammo menu reaching below the map is not a window over it");
    }
    // Quest details letters share a name's size and cross the buff band. They
    // stay where the window drew them.
    Frame details;
    details.note({8.0f, 8.0f, 60.0f, 56.0f});
    std::vector<Vertex> detail_letter(6, Vertex{220.0f, 80.0f});
    detail_letter[2] = {228.0f, 94.0f};
    mhfu::gpu::hud_scale::scale_quads(detail_letter, group_half, details);
    check(detail_letter[0].x == 220.0f && detail_letter[0].y == 80.0f && detail_letter[2].x == 228.0f &&
            detail_letter[2].y == 94.0f,
        "a quest-details letter stays in the window");
    std::vector<Vertex> wide_letter(6, Vertex{210.0f, 80.0f});
    wide_letter[2] = {220.0f, 94.0f};
    mhfu::gpu::hud_scale::scale_quads(wide_letter, group_half, details);
    check(wide_letter[0].x == 210.0f && wide_letter[0].y == 80.0f, "a wider quest-details letter is not a buff");
    // A cat's name beside the hunter: letters about 8 by 14, or one wider
    // label. A quest shows up to four hunters, so a letter on a later row
    // is the same piece. The 12-wide mark above is not a letter.
    std::vector<Vertex> name_clock(6, Vertex{8.0f, 8.0f});
    name_clock[2] = {60.0f, 56.0f};
    Frame cat_letter;
    std::vector<Vertex> cat(6, Vertex{160.0f, 56.0f});
    cat[2] = {168.0f, 70.0f};
    std::vector<Vertex> cat_row(name_clock);
    cat_row.insert(cat_row.end(), cat.begin(), cat.end());
    mhfu::gpu::hud_scale::scale_quads(cat_row, group_half, cat_letter);
    check(cat_row[6].x == 84.0f && cat_row[6].y == 32.0f && cat_row[8].x == 88.0f && cat_row[8].y == 39.0f,
        "a cat's name beside the hunter shrinks toward the clock");
    Frame cat_label;
    std::vector<Vertex> cat_name(6, Vertex{132.0f, 56.0f});
    cat_name[2] = {180.0f, 72.0f};
    std::vector<Vertex> label_row(name_clock);
    label_row.insert(label_row.end(), cat_name.begin(), cat_name.end());
    mhfu::gpu::hud_scale::scale_quads(label_row, group_half, cat_label);
    check(label_row[6].x == 70.0f && label_row[6].y == 32.0f && label_row[8].x == 94.0f && label_row[8].y == 40.0f,
        "a cat's name drawn as one label shrinks toward the clock");
    Frame fourth;
    std::vector<Vertex> fourth_name(6, Vertex{20.0f, 116.0f});
    fourth_name[2] = {28.0f, 130.0f};
    std::vector<Vertex> fourth_row(name_clock);
    fourth_row.insert(fourth_row.end(), fourth_name.begin(), fourth_name.end());
    mhfu::gpu::hud_scale::scale_quads(fourth_row, group_half, fourth);
    check(fourth_row[6].x == 14.0f && fourth_row[6].y == 62.0f && fourth_row[8].x == 18.0f && fourth_row[8].y == 69.0f,
        "a name on the fourth hunter's row shrinks toward the clock");
    Frame stars;
    stars.note({8.0f, 8.0f, 60.0f, 56.0f});
    std::vector<Vertex> stun_stars(6, Vertex{180.0f, 120.0f});
    stun_stars[2] = {202.0f, 142.0f};
    mhfu::gpu::hud_scale::scale_quads(stun_stars, group_half, stars);
    check(stun_stars[0].x == 180.0f && stun_stars[0].y == 120.0f,
        "the stars drawn on a stunned hunter stay where the game drew them");

    // The long sword's red gauge is two pixels tall inside the sharpness bar.
    Frame spirit;
    std::vector<Vertex> gauge(6, Vertex{66.0f, 46.0f});
    gauge[2] = {162.0f, 48.0f};
    mhfu::gpu::hud_scale::scale_quads(gauge, group_half, spirit);
    check(gauge[0].x == 37.0f && gauge[0].y == 27.0f && gauge[2].x == 85.0f && gauge[2].y == 28.0f,
        "a full spirit gauge shrinks with the clock and health");
    Frame sliver;
    std::vector<Vertex> bit(6, Vertex{66.0f, 46.0f});
    bit[2] = {69.0f, 48.0f};
    mhfu::gpu::hud_scale::scale_quads(bit, group_half, sliver);
    check(bit[0].x == 37.0f && bit[0].y == 27.0f && bit[2].x == 38.5f && bit[2].y == 28.0f,
        "a barely filled spirit gauge shrinks from the same corner");

    // A bowgun's loaded shots under stamina: the frame, then a round.
    using mhfu::gpu::hud_scale::Piece;
    using mhfu::gpu::hud_scale::piece_of;
    check(piece_of({60.0f, 32.0f, 164.0f, 44.0f}) == Piece::LongSword &&
            piece_of({60.0f, 32.0f, 68.0f, 48.0f}) == Piece::LongSword,
        "the loaded shots are in the weapon slot under stamina");
    check(piece_of({324.0f, 184.0f, 468.0f, 200.0f}) == Piece::Bowgun &&
            piece_of({346.0f, 186.0f, 353.0f, 200.0f}) == Piece::Bowgun,
        "the ammo bar and its text above the item bar are bowgun ammo");
    Frame gunner;
    std::vector<Vertex> shots(6, Vertex{60.0f, 32.0f});
    shots[2] = {164.0f, 44.0f};
    mhfu::gpu::hud_scale::scale_quads(shots, group_half, gunner);
    // Under the stamina bar: (60, 29) follows the clock's corner to (34, 18.5).
    check(shots[0].x == 34.0f && shots[0].y == 20.0f && shots[2].x == 86.0f && shots[2].y == 26.0f,
        "the loaded shots shrink with the clock and health");
    std::vector<Vertex> ammo(6, Vertex{324.0f, 184.0f});
    ammo[2] = {468.0f, 200.0f};
    gunner.note({8.0f, 8.0f, 60.0f, 56.0f});
    mhfu::gpu::hud_scale::scale_quads(ammo, group_half, gunner);
    check(ammo[2].x == 470.0f && ammo[2].y == 200.0f && ammo[0].x == 398.0f && ammo[0].y == 192.0f,
        "the ammo bar shrinks into its corner above the item bar");
    // The item box's colour menu draws a checkbox under the area map, in the
    // strip the ammo bar uses. It must not be taken for that bar: at the
    // player's item-bar size it was sliding down onto the row's label.
    Frame box_menu;
    std::vector<Vertex> checkbox(6, Vertex{328.0f, 172.0f});
    checkbox[2] = {344.0f, 188.0f};
    mhfu::gpu::hud_scale::scale_quads(checkbox, Scales::from_percent(75u, 90u, 90u), box_menu);
    check(checkbox[0].x == 328.0f && checkbox[0].y == 172.0f && checkbox[2].x == 344.0f && checkbox[2].y == 188.0f,
        "a menu checkbox under the map is not scaled with the ammo bar");
    Frame box_with_map;
    std::vector<Vertex> box_map(6, Vertex{316.0f, 32.0f});
    box_map[2] = {468.0f, 184.0f};
    std::vector<Vertex> box_check(checkbox);
    std::vector<Vertex> box_row(box_map);
    box_row.insert(box_row.end(), box_check.begin(), box_check.end());
    mhfu::gpu::hud_scale::scale_quads(box_row, map_half, box_with_map);
    check(box_row[0].x == 392.0f && box_row[6].x == 328.0f && box_row[6].y == 172.0f && box_row[8].y == 188.0f,
        "the checkbox stays put while the area map above it shrinks");

    // A bow's coatings under stamina, in each of their states, move like the
    // loaded rounds: (60, 29) follows the clock's corner to (34, 18.5).
    Frame no_coating;
    std::vector<Vertex> plate(6, Vertex{60.0f, 32.0f});
    plate[2] = {94.0f, 44.0f};
    mhfu::gpu::hud_scale::scale_quads(plate, group_half, no_coating);
    check(plate[0].x == 34.0f && plate[0].y == 20.0f && plate[2].x == 51.0f && plate[2].y == 26.0f,
        "a bow's empty coating plate shrinks with the clock and health");
    Frame coated;
    std::vector<Vertex> coating(6, Vertex{60.0f, 30.0f});
    coating[2] = {78.0f, 46.0f};
    mhfu::gpu::hud_scale::scale_quads(coating, group_half, coated);
    std::vector<Vertex> count(6, Vertex{90.0f, 30.0f});
    count[2] = {100.0f, 50.0f};
    mhfu::gpu::hud_scale::scale_quads(count, group_half, coated);
    std::vector<Vertex> single(6, Vertex{120.0f, 30.0f});
    single[2] = {132.0f, 46.0f};
    mhfu::gpu::hud_scale::scale_quads(single, group_half, coated);
    check(coating[0].x == 34.0f && coating[0].y == 19.0f && coating[2].x == 43.0f && coating[2].y == 27.0f,
        "a bow's large coating icon shrinks with the clock and health");
    check(count[0].x == 49.0f && count[0].y == 19.0f && count[2].x == 54.0f && count[2].y == 29.0f,
        "the coating count moves with the large icon");
    check(single[0].x == 64.0f && single[0].y == 19.0f && single[2].x == 70.0f && single[2].y == 27.0f,
        "a single coating icon shrinks with the clock and health");
    Frame ninth_only;
    std::vector<Vertex> ninth(6, Vertex{232.0f, 30.0f});
    ninth[2] = {244.0f, 46.0f};
    mhfu::gpu::hud_scale::scale_quads(ninth, group_half, ninth_only);
    check(ninth[0].x == 120.0f && ninth[0].y == 19.0f && ninth[2].x == 126.0f && ninth[2].y == 27.0f,
        "the ninth single coating, the last of the row, shrinks with the rest");

    // A gunlance draws no long sharpness bar, only the frame, and its shells
    // sit to the right of it. Both shrink from the clock's corner.
    Frame lance;
    std::vector<Vertex> sharp(6, Vertex{60.0f, 29.0f});
    sharp[2] = {104.0f, 41.0f};
    mhfu::gpu::hud_scale::scale_quads(sharp, group_half, lance);
    check(sharp[0].x == 34.0f && sharp[0].y == 18.5f && sharp[2].x == 56.0f && sharp[2].y == 24.5f,
        "a gunlance's sharpness frame shrinks without the long color bar");
    Frame shells_only;
    std::vector<Vertex> shell(6, Vertex{148.0f, 30.0f});
    shell[2] = {156.0f, 46.0f};
    mhfu::gpu::hud_scale::scale_quads(shell, group_half, shells_only);
    check(shell[0].x == 78.0f && shell[0].y == 19.0f && shell[2].x == 82.0f && shell[2].y == 27.0f,
        "a gunlance shell past the bowgun rounds shrinks with the clock and health");
    Frame shell_frame_only;
    std::vector<Vertex> shell_frame(6, Vertex{116.0f, 32.0f});
    shell_frame[2] = {220.0f, 44.0f};
    mhfu::gpu::hud_scale::scale_quads(shell_frame, group_half, shell_frame_only);
    check(shell_frame[0].x == 62.0f && shell_frame[0].y == 20.0f && shell_frame[2].x == 114.0f &&
            shell_frame[2].y == 26.0f,
        "the gunlance shell frame shrinks with the shells");

    // A hunting horn's staff is on screen with no notes queued. A note above
    // the slot, the note that grows during a recital, and the mark under the
    // staff all shrink from the same corner.
    Frame horn;
    std::vector<Vertex> staff(6, Vertex{118.0f, 30.0f});
    staff[2] = {246.0f, 46.0f};
    mhfu::gpu::hud_scale::scale_quads(staff, group_half, horn);
    check(staff[0].x == 63.0f && staff[0].y == 19.0f && staff[2].x == 127.0f && staff[2].y == 27.0f,
        "a hunting horn's staff shrinks with the clock and health");
    std::vector<Vertex> note(6, Vertex{150.0f, 28.0f});
    note[2] = {162.0f, 44.0f};
    mhfu::gpu::hud_scale::scale_quads(note, group_half, horn);
    check(note[0].x == 79.0f && note[0].y == 18.0f && note[2].x == 85.0f && note[2].y == 26.0f,
        "a queued note above the staff shrinks with it");
    // The last queued note reaches about (234, 50). It still belongs to the horn.
    Frame last_note;
    std::vector<Vertex> far_note(6, Vertex{222.0f, 34.0f});
    far_note[2] = {234.0f, 50.0f};
    mhfu::gpu::hud_scale::scale_quads(far_note, group_half, last_note);
    check(far_note[0].x == 115.0f && far_note[0].y == 21.0f && far_note[2].x == 121.0f && far_note[2].y == 29.0f,
        "the last queued note shrinks with the horn");
    // The equipment-set list's weapon icon is the same size as a note and sits
    // just past the last one. The status size was sliding it into the frame.
    Frame equip_set;
    std::vector<Vertex> weapon_icon(6, Vertex{240.0f, 28.0f});
    weapon_icon[2] = {256.0f, 44.0f};
    mhfu::gpu::hud_scale::scale_quads(weapon_icon, Scales::from_percent(75u, 90u, 90u), equip_set);
    check(weapon_icon[0].x == 240.0f && weapon_icon[0].y == 28.0f && weapon_icon[2].x == 256.0f &&
            weapon_icon[2].y == 44.0f,
        "the equipment-set weapon icon is not scaled with the horn");
    Frame recital;
    std::vector<Vertex> grown(6, Vertex{149.0f, 26.0f});
    grown[2] = {164.0f, 47.0f};
    mhfu::gpu::hud_scale::scale_quads(grown, group_half, recital);
    check(grown[0].x == 78.5f && grown[0].y == 17.0f && grown[2].x == 86.0f && grown[2].y == 27.5f,
        "a note grows and still shrinks with the clock and health while it is recited");
    // The first frame of a new note is larger and higher than a settled one.
    // It has to move with the staff, or it shows at the game's position and
    // jumps onto the sheet a frame later.
    Frame arriving;
    std::vector<Vertex> arrival(6, Vertex{145.0f, 21.0f});
    arrival[2] = {168.0f, 52.0f};
    mhfu::gpu::hud_scale::scale_quads(arrival, group_half, arriving);
    check(arrival[0].x == 76.5f && arrival[0].y == 14.5f && arrival[2].x == 88.0f && arrival[2].y == 30.0f,
        "a note just played shrinks with the staff instead of jumping onto it");
    // The equip confirmation's 24 by 24 icon sits where a note would, and the
    // first line of that box runs through the weapon slot. Neither moves.
    Frame equip;
    std::vector<Vertex> equip_icon(6, Vertex{240.0f, 32.0f});
    equip_icon[2] = {264.0f, 56.0f};
    std::vector<Vertex> equip_word(6, Vertex{65.0f, 33.0f});
    equip_word[2] = {72.0f, 47.0f};
    std::vector<Vertex> equip_row(equip_icon);
    equip_row.insert(equip_row.end(), equip_word.begin(), equip_word.end());
    mhfu::gpu::hud_scale::scale_quads(equip_row, group_half, equip);
    check(equip_row[0].x == 240.0f && equip_row[0].y == 32.0f && equip_row[6].x == 65.0f && equip_row[6].y == 33.0f,
        "the equip confirmation's icon does not scale the text beside it");
    Frame horn_mark;
    std::vector<Vertex> mark(6, Vertex{128.0f, 56.0f});
    mark[2] = {140.0f, 72.0f};
    mhfu::gpu::hud_scale::scale_quads(mark, group_half, horn_mark);
    check(mark[0].x == 68.0f && mark[0].y == 32.0f && mark[2].x == 74.0f && mark[2].y == 40.0f,
        "the mark under the note staff shrinks with the horn");
    Frame lone_count;
    std::vector<Vertex> digit(6, Vertex{90.0f, 30.0f});
    digit[2] = {100.0f, 50.0f};
    mhfu::gpu::hud_scale::scale_quads(digit, group_half, lone_count);
    check(digit[0].x == 90.0f && digit[0].y == 30.0f, "a glyph in that slot alone is not a coating");

    // A clock hand is a turned quad, a four-vertex strip expanded to two
    // triangles, inside the gauge at (8, 8)-(60, 56). It shrinks with it.
    Frame clock;
    clock.note({8.0f, 8.0f, 60.0f, 56.0f});
    std::vector<Vertex> hand{
        {32.0f, 30.0f}, {49.0f, 28.0f}, {34.0f, 44.0f}, {34.0f, 44.0f}, {49.0f, 28.0f}, {51.0f, 42.0f}};
    mhfu::gpu::hud_scale::scale_quads(hand, group_half, clock);
    check(hand[0].x == 20.0f && hand[0].y == 19.0f && hand[1].x == 28.5f && hand[1].y == 18.0f && hand[5].x == 29.5f &&
            hand[5].y == 25.0f,
        "a turned clock hand shrinks with the clock");

    // The player's sizes: the ammo bar takes the item bar's, not health's.
    const Scales players = Scales::from_percent(75u, 65u, 70u);
    check(players.map == 0.75f && players.vitals == 0.65f && players.sharpness == 0.65f &&
            players.long_sword == 0.65f && players.item_bar == 0.70f && players.bowgun == 0.70f,
        "the bowgun's ammo bar and coatings take the item bar's size");
    Frame bar_only;
    bar_only.note({8.0f, 8.0f, 60.0f, 56.0f});
    std::vector<Vertex> bar_ammo(6, Vertex{324.0f, 184.0f});
    bar_ammo[2] = {468.0f, 200.0f};
    mhfu::gpu::hud_scale::scale_quads(bar_ammo, Scales::from_percent(100u, 100u, 50u), bar_only);
    // The item bar's top edge, y 200, goes to 232 at 50%; the ammo bar stays on it.
    check(bar_ammo[0].x == 398.0f && bar_ammo[0].y == 224.0f && bar_ammo[2].x == 470.0f && bar_ammo[2].y == 232.0f,
        "the ammo bar shrinks with the item bar it sits on");

    // Loaded rounds are drawn without their frame.
    Frame loaded;
    std::vector<Vertex> round(6, Vertex{60.0f, 32.0f});
    round[2] = {68.0f, 48.0f};
    mhfu::gpu::hud_scale::scale_quads(round, group_half, loaded);
    check(round[0].x == 34.0f && round[0].y == 20.0f && round[2].x == 38.0f && round[2].y == 28.0f,
        "a loaded round shrinks with the clock and health without its frame");
    // Shop header letters are 7 by 14 in the same band, including the stretch
    // a gunlance shell occupies. They must stay put.
    Frame shop;
    std::vector<Vertex> type_letter(6, Vertex{65.0f, 33.0f});
    type_letter[2] = {72.0f, 47.0f};
    std::vector<Vertex> type_shell(6, Vertex{114.0f, 33.0f});
    type_shell[2] = {121.0f, 47.0f};
    std::vector<Vertex> type_far(6, Vertex{240.0f, 32.0f});
    type_far[2] = {247.0f, 46.0f};
    std::vector<Vertex> header(type_letter);
    header.insert(header.end(), type_shell.begin(), type_shell.end());
    header.insert(header.end(), type_far.begin(), type_far.end());
    mhfu::gpu::hud_scale::scale_quads(header, group_half, shop);
    check(header[0].x == 65.0f && header[0].y == 33.0f && header[6].x == 114.0f && header[6].y == 33.0f &&
            header[12].x == 240.0f && header[12].y == 32.0f,
        "shop header letters in the weapon slot are not scaled");
    // The Abandon Quest window's top-left border tile, 16 by 8, sits in the
    // weapon slot while a bow's coating plate is up. It stays where the window
    // draws it; the plate still shrinks. Traced with MHFU_TRACE_HUD_SCALE.
    check(piece_of({232.0f, 40.0f, 248.0f, 48.0f}) == Piece::None &&
            piece_of({160.0f, 42.0f, 176.0f, 50.0f}) == Piece::None &&
            piece_of({160.0f, 34.0f, 176.0f, 42.0f}) == Piece::None,
        "a window's border tile in the weapon slot is not a weapon piece");
    Frame abandon;
    std::vector<Vertex> coating_plate(6, Vertex{60.0f, 32.0f});
    coating_plate[2] = {94.0f, 44.0f};
    std::vector<Vertex> window_corner(6, Vertex{232.0f, 40.0f});
    window_corner[2] = {248.0f, 48.0f};
    std::vector<Vertex> abandon_row(coating_plate);
    abandon_row.insert(abandon_row.end(), window_corner.begin(), window_corner.end());
    mhfu::gpu::hud_scale::scale_quads(abandon_row, group_half, abandon);
    check(abandon_row[0].x == 34.0f && abandon_row[0].y == 20.0f && abandon_row[6].x == 232.0f &&
            abandon_row[6].y == 40.0f && abandon_row[8].x == 248.0f && abandon_row[8].y == 48.0f,
        "the Abandon Quest window's corner stays put while the coating plate shrinks");
    // The item box's "Please select an item." begins inside the weapon slot,
    // and with a bow out its first letters were moved out of the box with the
    // coating plate. A letter is 7 by 14; the plate still shrinks.
    check(piece_of({229.0f, 36.0f, 236.0f, 50.0f}) == Piece::None &&
            piece_of({60.0f, 32.0f, 68.0f, 48.0f}) == Piece::LongSword,
        "a window's letter in the weapon slot is not a weapon piece, and a loaded round still is");
    Frame item_box;
    std::vector<Vertex> prompt_plate(6, Vertex{60.0f, 32.0f});
    prompt_plate[2] = {94.0f, 44.0f};
    std::vector<Vertex> prompt(6, Vertex{229.0f, 36.0f});
    prompt[2] = {236.0f, 50.0f};
    std::vector<Vertex> prompt_row(prompt_plate);
    prompt_row.insert(prompt_row.end(), prompt.begin(), prompt.end());
    mhfu::gpu::hud_scale::scale_quads(prompt_row, group_half, item_box);
    check(prompt_row[0].x == 34.0f && prompt_row[0].y == 20.0f && prompt_row[6].x == 229.0f &&
            prompt_row[6].y == 36.0f && prompt_row[8].x == 236.0f && prompt_row[8].y == 50.0f,
        "the item box prompt's first letters stay in the box while the coating plate shrinks");

    // The ammo menu shrinks as one panel into the item bar's corner (472, 264).
    const Scales bar_half{1.0f, 1.0f, 0.5f, 1.0f, 1.0f, 1.0f};
    Frame menu;
    menu.note({8.0f, 8.0f, 60.0f, 56.0f});
    std::vector<Vertex> highlight(6, Vertex{314.0f, 153.0f});
    highlight[2] = {470.0f, 173.0f};
    mhfu::gpu::hud_scale::scale_quads(highlight, bar_half, menu);
    check(highlight[0].x == 393.0f && highlight[0].y == 208.5f && highlight[2].x == 471.0f && highlight[2].y == 218.5f,
        "the ammo menu's highlight shrinks into the item bar's corner");
    std::vector<Vertex> title(6, Vertex{349.0f, 140.0f});
    title[2] = {356.0f, 154.0f};
    mhfu::gpu::hud_scale::scale_quads(title, bar_half, menu);
    check(title[0].x == 410.5f && title[0].y == 202.0f, "the ammo menu's title moves with it");
    Frame closed;
    std::vector<Vertex> lone_title(6, Vertex{349.0f, 140.0f});
    lone_title[2] = {356.0f, 154.0f};
    mhfu::gpu::hud_scale::scale_quads(lone_title, bar_half, closed);
    check(lone_title[0].x == 349.0f && lone_title[0].y == 140.0f, "without the ammo menu that corner is left alone");

    // The menu's first sprite, its frame's top-left corner, opens it in the
    // same frame, and the arrow above the frame moves with it. It opens from
    // the ammo bar, drawn the frame before.
    Frame opening;
    opening.note({312.0f, 184.0f, 472.0f, 200.0f});
    opening.begin();
    opening.note({8.0f, 8.0f, 60.0f, 56.0f});
    std::vector<Vertex> corner(6, Vertex{312.0f, 128.0f});
    corner[2] = {328.0f, 136.0f};
    mhfu::gpu::hud_scale::scale_quads(corner, bar_half, opening);
    check(corner[0].x == 392.0f && corner[0].y == 196.0f && corner[2].x == 400.0f && corner[2].y == 200.0f,
        "the ammo menu's top border shrinks in the frame it opens");
    std::vector<Vertex> menu_arrow(6, Vertex{384.0f, 124.0f});
    menu_arrow[2] = {400.0f, 140.0f};
    mhfu::gpu::hud_scale::scale_quads(menu_arrow, bar_half, opening);
    check(menu_arrow[0].x == 428.0f && menu_arrow[0].y == 194.0f, "the arrow above the ammo menu moves with it");
    opening.begin();
    std::vector<Vertex> after(6, Vertex{349.0f, 140.0f});
    after[2] = {356.0f, 154.0f};
    mhfu::gpu::hud_scale::scale_quads(after, bar_half, opening);
    check(after[0].x == 349.0f && after[0].y == 140.0f, "the frame after the ammo menu closes nothing is kept");
    Frame bar_closed;
    std::vector<Vertex> closed_corner(6, Vertex{312.0f, 208.0f});
    closed_corner[2] = {328.0f, 216.0f};
    mhfu::gpu::hud_scale::scale_quads(closed_corner, bar_half, bar_closed);
    check(!bar_closed.ammo_menu_now, "the closed item bar's corner is not the ammo menu");

    // Combine in a quest: the Material B window's 16 by 8 corner sits on
    // the ammo menu's corner, and the item list's lower rows sit inside the
    // menu's rectangle. The menu draws its side or highlight every frame;
    // Combine never does, and there is no ammo bar, so its rows stay as
    // drawn. Its first frame used to open the menu: picking the first
    // material shrank the whole list for a frame.
    Frame combine;
    const auto combine_frame = [&](bool body) {
        combine.begin();
        combine.note({8.0f, 8.0f, 60.0f, 56.0f});
        std::vector<Vertex> tile(6, Vertex{312.0f, 128.0f});
        tile[2] = {328.0f, 136.0f};
        mhfu::gpu::hud_scale::scale_quads(tile, bar_half, combine);
        if (body) {
            std::vector<Vertex> side(6, Vertex{312.0f, 136.0f});
            side[2] = {328.0f, 256.0f};
            mhfu::gpu::hud_scale::scale_quads(side, bar_half, combine);
        }
        std::vector<Vertex> row(6, Vertex{338.0f, 160.0f});
        row[2] = {345.0f, 174.0f};
        mhfu::gpu::hud_scale::scale_quads(row, bar_half, combine);
        return row[0];
    };
    const Vertex first_row = combine_frame(false);
    check(first_row.x == 338.0f && first_row.y == 160.0f && !combine.ammo_menu_now,
        "a lone corner with no ammo bar does not open the ammo menu, even in its first frame");
    const Vertex held_row = combine_frame(false);
    check(held_row.x == 338.0f && held_row.y == 160.0f && !combine.ammo_menu_now,
        "a corner held with no menu body is not the ammo menu, so the Combine list stays as drawn");
    combine_frame(false);
    const Vertex reopened_row = combine_frame(true);
    check(reopened_row.x == 405.0f && reopened_row.y == 212.0f, "the menu's side opens it again under a held corner");
    const Vertex open_row = combine_frame(false);
    check(open_row.x == 405.0f && open_row.y == 212.0f,
        "a corner after a frame with the menu's body keeps the menu open");

    // A shop panel in the item bar's corner. Without the quest clock it is
    // not the bar, so the Items size leaves it where the game drew it.
    Frame shop_bar;
    std::vector<Vertex> shop_strip(6, Vertex{320.0f, 208.0f});
    shop_strip[2] = {448.0f, 240.0f};
    mhfu::gpu::hud_scale::scale_quads(shop_strip, bar_half, shop_bar);
    check(shop_strip[0].x == 320.0f && shop_strip[0].y == 208.0f && shop_strip[2].x == 448.0f &&
            shop_strip[2].y == 240.0f,
        "a shop panel in the item bar's corner stays put");
    Frame quest_bar;
    quest_bar.note({8.0f, 8.0f, 60.0f, 56.0f});
    std::vector<Vertex> quest_strip(6, Vertex{320.0f, 208.0f});
    quest_strip[2] = {448.0f, 240.0f};
    mhfu::gpu::hud_scale::scale_quads(quest_strip, bar_half, quest_bar);
    check(quest_strip[0].x == 396.0f && quest_strip[0].y == 236.0f && quest_strip[2].x == 460.0f &&
            quest_strip[2].y == 252.0f,
        "the quest item bar still shrinks while the clock is up");

    // A teammate's name. Glyphs share the point the game placed the text, so
    // half size closes the gap between letters and leaves that point put.
    // A plate filed on its own has no such point and shrinks about its centre.
    std::vector<Vertex> glyphs = {{100.0f, 80.0f}, {108.0f, 80.0f}, {108.0f, 92.0f}, {100.0f, 80.0f}, {108.0f, 92.0f},
        {100.0f, 92.0f}, {116.0f, 80.0f}, {124.0f, 80.0f}, {124.0f, 92.0f}, {116.0f, 80.0f}, {124.0f, 92.0f},
        {116.0f, 92.0f}};
    mhfu::gpu::hud_scale::scale_name(glyphs, 0.5f, true, 100.0f, 80.0f);
    check(glyphs[0].x == 100.0f && glyphs[0].y == 80.0f && glyphs[6].x == 108.0f && glyphs[8].x == 112.0f &&
            glyphs[8].y == 86.0f,
        "a name shrinks toward the point the game placed it");
    std::vector<Vertex> name_plate(6, Vertex{200.0f, 40.0f});
    name_plate[2] = {232.0f, 56.0f};
    mhfu::gpu::hud_scale::scale_name(name_plate, 0.5f, false, 0.0f, 0.0f);
    check(
        name_plate[0].x == 208.0f && name_plate[0].y == 44.0f && name_plate[2].x == 224.0f && name_plate[2].y == 52.0f,
        "a name plate with no text point shrinks about its own centre");
    std::vector<Vertex> as_drawn = {{10.0f, 10.0f}, {20.0f, 30.0f}};
    mhfu::gpu::hud_scale::scale_name(as_drawn, 1.0f, true, 0.0f, 0.0f);
    check(as_drawn[0].x == 10.0f && as_drawn[1].y == 30.0f, "100% leaves a name where the game drew it");
    check(mhfu::gpu::hud_scale::is_name_tag({100.0f, 80.0f, 108.0f, 94.0f}), "a glyph is a name tag");
    check(mhfu::gpu::hud_scale::is_name_tag({8.0f, 40.0f, 104.0f, 56.0f}), "a name plate is a name tag");
    check(!mhfu::gpu::hud_scale::is_name_tag({40.0f, 20.0f, 280.0f, 140.0f}), "a village window is not a name tag");
    check(!mhfu::gpu::hud_scale::is_name_tag({10.0f, 10.0f, 40.0f, 80.0f}), "a tall panel is not a name tag");
    const Scales name_half = Scales::from_percent(100u, 100u, 100u, 50u);
    check(name_half.names == 0.5f && name_half.map == 1.0f && name_half.any(), "the name size is its own");
    check(!Scales::from_percent(100u, 100u, 100u).any(), "an omitted name size stays at the game's size");
}

// A menu draws panels where the quest HUD's pieces are. The sizes resize
// only the quest HUD's own draws (gpu/game_hud.hpp), whatever the rectangle.
// The monster icon follows the quest clock: up while it is drawn, and for one
// frame after, so a frame that files it late does not blink the icon.
void test_quest_clock() {
    struct Vertex {
        float x;
        float y;
    };
    const auto quad = [](float x0, float y0, float x1, float y1) {
        std::vector<Vertex> v(6, Vertex{x0, y0});
        v[1] = {x1, y0};
        v[2] = {x1, y1};
        v[3] = {x0, y1};
        v[4] = {x1, y1};
        v[5] = {x0, y0};
        return v;
    };
    mhfu::gpu::hud_scale::QuestClock clock;
    check(!clock.up(), "no clock before any draw");
    std::vector<Vertex> row = quad(100.0f, 100.0f, 107.0f, 114.0f);
    const std::vector<Vertex> gauge = quad(8.0f, 8.0f, 60.0f, 56.0f);
    row.insert(row.end(), gauge.begin(), gauge.end());
    clock.note(quad(100.0f, 100.0f, 107.0f, 114.0f));
    check(!clock.up(), "a letter is not the clock");
    clock.note(row);
    check(clock.up(), "the clock as the second quad of a draw puts the quest HUD up");
    clock.begin();
    check(clock.up(), "and it stays up for the next frame");
    clock.begin();
    check(!clock.up(), "two frames without it, as after a quest, take it down");
    clock.note(quad(8.0f, 8.0f, 200.0f, 56.0f));
    check(!clock.up(), "a wide panel in that corner is not the clock");
}

void test_interface_sizes_by_owner() {
    using mhfu::gpu::hud::Owner;
    using mhfu::gpu::hud_scale::Frame;
    using mhfu::gpu::hud_scale::Scales;
    using mhfu::gpu::hud_scale::scale_quads;
    struct Vertex {
        float x;
        float y;
    };
    const auto quad = [](float x0, float y0, float x1, float y1) {
        std::vector<Vertex> q(6, Vertex{x0, y0});
        q[2] = {x1, y1};
        return q;
    };
    const Scales half = Scales::from_percent(50u, 50u, 50u);

    check(!mhfu::gpu::hud_scale::may_resize(Owner::Other), "another owner's draw is never resized");
    check(mhfu::gpu::hud_scale::may_resize(Owner::QuestHud), "the quest HUD's draw may be resized");
    check(mhfu::gpu::hud_scale::may_resize(Owner::Unknown), "without owners the rectangles decide");

    // A village panel in the health bar's exact place keeps the game's size.
    Frame village;
    std::vector<Vertex> panel = quad(56.0f, 12.0f, 320.0f, 20.0f);
    scale_quads(panel, half, village, 0u, Owner::Other);
    check(panel[0].x == 56.0f && panel[0].y == 12.0f && panel[2].x == 320.0f && panel[2].y == 20.0f,
        "a menu shaped like the health bar is not resized");
    std::vector<Vertex> map_panel = quad(316.0f, 32.0f, 468.0f, 184.0f);
    scale_quads(map_panel, half, village, 0u, Owner::Other);
    check(map_panel[0].x == 316.0f && map_panel[2].y == 184.0f, "a menu shaped like the area map is not resized");
    std::vector<Vertex> shop_strip = quad(320.0f, 216.0f, 448.0f, 240.0f);
    scale_quads(shop_strip, half, village, 0u, Owner::Other);
    check(shop_strip[0].x == 320.0f && shop_strip[2].y == 240.0f, "a menu shaped like the item bar is not resized");

    // The same rectangle from the quest HUD still takes its size.
    Frame quest;
    std::vector<Vertex> health = quad(56.0f, 12.0f, 320.0f, 20.0f);
    scale_quads(health, half, quest, 0u, Owner::QuestHud);
    check(health[0].x == 32.0f && health[0].y == 10.0f && health[2].x == 164.0f && health[2].y == 14.0f,
        "the quest HUD's health bar shrinks toward the clock's corner");

    // A menu over the HUD is still seen: the item list holds the item bar at
    // the game's size, though only the bar is the quest HUD's.
    Frame listed;
    std::vector<Vertex> clock = quad(8.0f, 8.0f, 60.0f, 56.0f);
    scale_quads(clock, half, listed, 0u, Owner::QuestHud);
    std::vector<Vertex> list = quad(200.0f, 150.0f, 470.0f, 240.0f);
    scale_quads(list, half, listed, 0u, Owner::Other);
    check(list[0].x == 200.0f && list[2].x == 470.0f, "the item list is not resized");
    std::vector<Vertex> bar = quad(320.0f, 216.0f, 448.0f, 240.0f);
    scale_quads(bar, half, listed, 0u, Owner::QuestHud);
    check(bar[0].x == 320.0f && bar[2].y == 240.0f, "the item bar stays while another owner's list is open");
    Frame unlisted;
    clock = quad(8.0f, 8.0f, 60.0f, 56.0f);
    scale_quads(clock, half, unlisted, 0u, Owner::QuestHud);
    bar = quad(320.0f, 216.0f, 448.0f, 240.0f);
    scale_quads(bar, half, unlisted, 0u, Owner::QuestHud);
    check(bar[0].x == 396.0f && bar[2].x == 460.0f && bar[2].y == 252.0f,
        "the item bar shrinks toward its bottom-right with no list open");

    // Unknown owners: the rectangles alone decide, as before owners were known.
    Frame unwatched;
    panel = quad(56.0f, 12.0f, 320.0f, 20.0f);
    scale_quads(panel, half, unwatched, 0u, Owner::Unknown);
    check(panel[0].x == 32.0f && panel[2].x == 164.0f, "without owners a panel in the bar's place is resized");
}

// What the renderer asks of gpu/hud_scale.hpp around a draw: the Fill fit,
// a name label's draw, and where a moved area map went.
void test_hud_scale_draws() {
    using mhfu::gpu::hud::NameDraw;
    using mhfu::gpu::hud_scale::Frame;
    using mhfu::gpu::hud_scale::InterfaceFit;
    using mhfu::gpu::hud_scale::Rect;
    using mhfu::gpu::hud_scale::Scales;
    struct Vertex {
        float x;
        float y;
    };
    const auto quad = [](float x0, float y0, float x1, float y1) {
        std::vector<Vertex> q(6, Vertex{x0, y0});
        q[2] = {x1, y1};
        return q;
    };
    const auto join = [](std::vector<Vertex> a, const std::vector<Vertex> &b) {
        a.insert(a.end(), b.begin(), b.end());
        return a;
    };

    // The fit pulls toward the centre of the 480 by 272 picture, and back.
    const InterfaceFit none;
    float x = 17.0f;
    float y = 201.0f;
    none.apply(x, y);
    check(!none.any() && x == 17.0f && y == 201.0f, "no fit leaves a point where it is");
    const InterfaceFit half_wide{0.5f, 1.0f};
    x = 0.0f;
    y = 0.0f;
    half_wide.apply(x, y);
    const Rect back = half_wide.to_game({120.0f, 0.0f, 360.0f, 272.0f});
    check(half_wide.any() && x == 120.0f && y == 0.0f && back.x0 == 0.0f && back.x1 == 480.0f && back.y1 == 272.0f,
        "the fit pulls toward the centre, and to_game undoes it");
    const Rect both = mhfu::gpu::hud_scale::united({10.0f, 20.0f, 30.0f, 40.0f}, {35.0f, 5.0f, 25.0f, 15.0f});
    check(both.x0 == 10.0f && both.y0 == 5.0f && both.x1 == 35.0f && both.y1 == 40.0f,
        "united holds both rectangles, either way round");

    // A name over a hunter shrinks toward the point the game placed it, and
    // the bounds it took come back for the scissor.
    const Scales names_half{1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.5f};
    NameDraw named;
    named.is_name = true;
    named.has_origin = true;
    named.origin_x = 108.0f;
    named.origin_y = 114.0f;
    Frame quiet;
    std::vector<Vertex> name = join(quad(100.0f, 100.0f, 108.0f, 114.0f), quad(108.0f, 100.0f, 116.0f, 114.0f));
    const std::optional<Rect> name_bounds =
        mhfu::gpu::hud_scale::scale_name_draw(name, names_half, quiet, named, InterfaceFit{});
    check(name[0].x == 104.0f && name[0].y == 107.0f && name[8].x == 112.0f && name[8].y == 114.0f && name_bounds &&
            name_bounds->x0 == 104.0f && name_bounds->y0 == 107.0f && name_bounds->x1 == 112.0f &&
            name_bounds->y1 == 114.0f,
        "a name shrinks toward its point and reports where it went");
    std::vector<Vertex> same = quad(100.0f, 100.0f, 108.0f, 114.0f);
    check(!mhfu::gpu::hud_scale::scale_name_draw(same, Scales{}, quiet, named, InterfaceFit{}) && same[0].x == 100.0f,
        "at the game's size a name stays put and needs no scissor");

    // A village label's plate larger than a name stays as drawn.
    NameDraw village = named;
    village.tag_only = true;
    std::vector<Vertex> plate = quad(100.0f, 100.0f, 300.0f, 140.0f);
    check(!mhfu::gpu::hud_scale::scale_name_draw(plate, names_half, quiet, village, InterfaceFit{}) &&
            plate[0].x == 100.0f && plate[2].x == 300.0f,
        "a village plate larger than a name stays as drawn");

    // The cat's face filed with its name takes Status from the clock while
    // the name takes the Names size.
    const Scales both_half{1.0f, 0.5f, 1.0f, 0.5f, 0.5f, 0.5f, 0.5f};
    Frame clock_up;
    clock_up.note({8.0f, 8.0f, 60.0f, 56.0f});
    NameDraw cat_name = named;
    cat_name.origin_x = 44.0f;
    cat_name.origin_y = 86.0f;
    std::vector<Vertex> labelled = join(quad(8.0f, 76.0f, 20.0f, 87.0f), quad(40.0f, 72.0f, 48.0f, 86.0f));
    const std::optional<Rect> letters =
        mhfu::gpu::hud_scale::scale_name_draw(labelled, both_half, clock_up, cat_name, InterfaceFit{});
    check(labelled[0].x == 8.0f && labelled[0].y == 42.0f && labelled[2].x == 14.0f && labelled[2].y == 47.5f &&
            labelled[6].x == 42.0f && labelled[6].y == 79.0f && labelled[8].x == 46.0f && labelled[8].y == 86.0f,
        "the face in a name draw takes Status and the letters take Names");
    // A letter of a name over a teammate that crosses the party column, in
    // the face's cell, is still a letter: it takes Names about the name's
    // point and nothing from Status.
    NameDraw crossing = cat_name;
    crossing.glyph = true;
    std::vector<Vertex> letter = quad(8.0f, 76.0f, 20.0f, 87.0f);
    mhfu::gpu::hud_scale::scale_name_draw(letter, both_half, clock_up, crossing, InterfaceFit{});
    check(letter[0].x == 26.0f && letter[0].y == 81.0f && letter[2].x == 32.0f && letter[2].y == 86.5f,
        "a name's letter crossing the party column takes Names, not Status");

    // Between game frames a name's letter is paired with the same letter of
    // the same name in the newer frame, on the nearest point.
    using mhfu::gpu::hud_scale::NamePlace;
    const NamePlace o{100.0f, 60.0f, 8.0f, 0.0f, 0.25f, 0.5f, 0x40CC000u, 6u};
    std::vector<NamePlace> newer = {
        {300.0f, 60.0f, 8.0f, 0.0f, 0.25f, 0.5f, 0x40CC000u, 6u}, // the same letter of another hunter's name
        {104.0f, 62.0f, 8.0f, 0.0f, 0.25f, 0.5f, 0x40CC000u, 6u}, // this name, moved a little
        {104.0f, 62.0f, 16.0f, 0.0f, 0.25f, 0.5f, 0x40CC000u, 6u}, // the same letter later in the name
        {104.0f, 62.0f, 8.0f, 0.0f, 0.50f, 0.5f, 0x40CC000u, 6u}, // another letter in that place
    };
    check(mhfu::gpu::hud_scale::match_name_place(o, newer) == 1,
        "a name's letter pairs with the same letter, place and nearest name");
    newer.erase(newer.begin() + 1);
    check(mhfu::gpu::hud_scale::match_name_place(o, newer) == -1,
        "without that name in the newer frame, nothing far away or different is taken");
    newer.push_back({100.0f, 60.0f, 8.0f, 0.0f, 0.25f, 0.5f, 0x91874A0u, 6u});
    check(mhfu::gpu::hud_scale::match_name_place(o, newer) == -1, "another texture is not the same letter");
    check(letters && letters->x0 == 42.0f && letters->x1 == 46.0f,
        "only the name's letters are reported for the scissor");
    // Under Fill the face is told apart in the game's pixels. At Status 100%
    // it then stays where it landed, while a name would shrink.
    const InterfaceFit fill{0.5f, 0.5f};
    std::vector<Vertex> fitted = quad(124.0f, 106.0f, 130.0f, 111.5f);
    mhfu::gpu::hud_scale::scale_name_draw(fitted, names_half, clock_up, cat_name, fill);
    check(fitted[0].x == 124.0f && fitted[0].y == 106.0f && fitted[2].x == 130.0f && fitted[2].y == 111.5f,
        "a fitted face is still recognised in the game's pixels");

    // Only the map's own quads count toward its moved bounds.
    const std::vector<Vertex> drawn = join(quad(316.0f, 32.0f, 468.0f, 184.0f), quad(60.0f, 44.0f, 166.0f, 50.0f));
    std::vector<Vertex> moved = join(quad(392.0f, 32.0f, 468.0f, 108.0f), quad(34.0f, 26.0f, 87.0f, 29.0f));
    const std::optional<Rect> map_bounds = mhfu::gpu::hud_scale::moved_map_bounds(drawn, moved, 0u);
    check(map_bounds && map_bounds->x0 == 392.0f && map_bounds->y0 == 32.0f && map_bounds->x1 == 468.0f &&
            map_bounds->y1 == 108.0f,
        "the moved map's bounds leave out other moved pieces");
    check(!mhfu::gpu::hud_scale::moved_map_bounds(drawn, drawn, 0u), "an unmoved map needs no scissor");
}

// A bowgun's reticle moves with the aim. Crossing the map's slot it is the
// size of a mark, and it must stay the size the game draws it.
// A paintballed monster draws a ring 43 pixels square on the map, traced at
// (328, 121) in Volcano area 4. With the map at 60% it was taken for a window
// over the map: the ring, the arrow and the camp icon kept the game's size and place
// while the map shrank, off it to the lower left.
void test_paintball_ring_stays_on_map() {
    using mhfu::gpu::hud::Owner;
    using mhfu::gpu::hud_scale::Frame;
    using mhfu::gpu::hud_scale::Scales;
    using mhfu::gpu::hud_scale::scale_quads;
    struct Vertex {
        float x;
        float y;
    };
    const auto quad = [](float x0, float y0, float x1, float y1) {
        std::vector<Vertex> q(6, Vertex{x0, y0});
        q[2] = {x1, y1};
        return q;
    };
    const auto stays = [](const std::vector<Vertex> &q, float x0, float y0) { return q[0].x == x0 && q[0].y == y0; };
    const Scales map_60 = Scales::from_percent(60u, 100u, 100u);
    constexpr std::uint32_t kRing = mhfu::gpu::hud_scale::kMapIconTexture;
    const auto quest = [&](Frame &frame) {
        std::vector<Vertex> clock = quad(8.0f, 8.0f, 60.0f, 56.0f);
        scale_quads(clock, map_60, frame, 0u, Owner::QuestHud);
        std::vector<Vertex> tile = quad(316.0f, 32.0f, 468.0f, 184.0f);
        scale_quads(tile, map_60, frame, mhfu::gpu::hud_scale::kAreaMapTexture, Owner::QuestHud);
    };

    Frame frame;
    quest(frame);
    std::vector<Vertex> ring = quad(328.0f, 121.0f, 371.0f, 164.0f);
    scale_quads(ring, map_60, frame, kRing, Owner::QuestHud);
    check(ring[0].x != 328.0f && ring[2].x - ring[0].x < 43.0f, "a paintball's ring takes the map's size");
    check(!frame.panel_up(), "a paintball's ring is not a window over the map");
    std::vector<Vertex> camp = quad(357.0f, 175.0f, 368.0f, 185.0f);
    scale_quads(camp, map_60, frame, mhfu::gpu::hud_scale::kMapIconTexture, Owner::QuestHud);
    check(!stays(camp, 357.0f, 175.0f), "the camp icon stays on the map beside the ring");
    frame.begin();
    quest(frame);
    std::vector<Vertex> arrow = quad(365.0f, 119.0f, 377.0f, 131.0f);
    scale_quads(arrow, map_60, frame, mhfu::gpu::hud_scale::kMapIconTexture, Owner::QuestHud);
    check(!stays(arrow, 365.0f, 119.0f), "the arrow in the next frame stays on the map too");

    // Boundaries: a square larger than any ring, a ring off the map, and a
    // ring drawn by other code (the shop's 40-square panels) stay as drawn.
    check(!mhfu::gpu::hud_scale::is_map_ring({330.0f, 60.0f, 410.0f, 140.0f}, kRing), "an 80 square is no ring");
    check(!mhfu::gpu::hud_scale::is_map_ring({100.0f, 100.0f, 144.0f, 144.0f}, kRing), "a ring off the map is not");
    check(!mhfu::gpu::hud_scale::is_map_ring({323.0f, 129.0f, 367.0f, 173.0f},
              mhfu::gpu::hud_scale::kAreaMapTexture),
        "the map's own window is not a ring");
    check(!mhfu::gpu::hud_scale::is_map_ring({344.0f, 48.0f, 360.0f, 160.0f}, 0x09165BA0u),
        "a window's side over the map is not a ring");
    Frame shop;
    quest(shop);
    std::vector<Vertex> panel = quad(400.0f, 80.0f, 440.0f, 120.0f);
    scale_quads(panel, map_60, shop, kRing, Owner::Other);
    check(stays(panel, 400.0f, 80.0f), "a square panel filed by other code stays as drawn");
}

void test_aim_reticle_is_not_a_map_mark() {
    using mhfu::gpu::hud::Owner;
    using mhfu::gpu::hud_scale::Frame;
    using mhfu::gpu::hud_scale::Scales;
    using mhfu::gpu::hud_scale::scale_quads;
    struct Vertex {
        float x;
        float y;
    };
    const auto quad = [](float x0, float y0, float x1, float y1) {
        std::vector<Vertex> q(6, Vertex{x0, y0});
        q[2] = {x1, y1};
        return q;
    };
    const Scales map_half = Scales::from_percent(50u, 100u, 100u);
    constexpr std::uint32_t kReticle = 0x09123450u;
    const auto quest = [&](Frame &frame) {
        std::vector<Vertex> clock = quad(8.0f, 8.0f, 60.0f, 56.0f);
        scale_quads(clock, map_half, frame, 0u, Owner::QuestHud);
        std::vector<Vertex> tile = quad(316.0f, 32.0f, 468.0f, 184.0f);
        scale_quads(tile, map_half, frame, 0u, Owner::QuestHud);
    };

    // Not aiming: a sprite of that size in the slot is a mark.
    Frame idle;
    quest(idle);
    std::vector<Vertex> mark = quad(420.0f, 60.0f, 444.0f, 84.0f);
    scale_quads(mark, map_half, idle, kReticle, Owner::QuestHud);
    check(mark[0].x != 420.0f, "a mark in the slot takes the map's size");

    Frame aiming;
    aiming.aiming = true;
    quest(aiming);
    std::vector<Vertex> centre = quad(228.0f, 124.0f, 252.0f, 148.0f);
    scale_quads(centre, map_half, aiming, kReticle, Owner::QuestHud);
    check(centre[0].x == 228.0f && centre[2].y == 148.0f, "the reticle in the middle stays as drawn");
    std::vector<Vertex> right = quad(420.0f, 60.0f, 444.0f, 84.0f);
    scale_quads(right, map_half, aiming, kReticle, Owner::QuestHud);
    check(right[0].x == 420.0f && right[0].y == 60.0f && right[2].x == 444.0f,
        "the reticle aimed into the map's slot stays as drawn");
    std::vector<Vertex> arrow = quad(420.0f, 60.0f, 444.0f, 84.0f);
    scale_quads(arrow, map_half, aiming, mhfu::gpu::hud_scale::kMapIconTexture, Owner::QuestHud);
    check(arrow[0].x != 420.0f, "a mark from the map's atlas still takes the map's size while aiming");
    std::vector<Vertex> other = quad(420.0f, 60.0f, 450.0f, 90.0f);
    scale_quads(other, map_half, aiming, kReticle, Owner::QuestHud);
    check(other[0].x != 420.0f, "a sprite of another size from that texture is still a mark");

    // The aim ends: the reticle is forgotten.
    aiming.aiming = false;
    std::vector<Vertex> after = quad(420.0f, 60.0f, 444.0f, 84.0f);
    scale_quads(after, map_half, aiming, kReticle, Owner::QuestHud);
    check(after[0].x != 420.0f, "after the aim that size is a mark again");

    // The font and the HUD atlas are shared. Traced with
    // MHFU_TRACE_HUD_SCALE=all in a quest: a letter of a name over a hunter
    // and a 16 by 8 tile near the middle were learned as the reticle while
    // aiming, and the party list's letters and the item bar's corners then
    // kept the game's size. A learned reticle only keeps a sprite off the map.
    constexpr std::uint32_t kFont = 0x040CC000u;
    constexpr std::uint32_t kAtlas = 0x09156BA0u;
    const Scales sizes = Scales::from_percent(60u, 75u, 65u);
    Frame shared;
    shared.aiming = true;
    std::vector<Vertex> clock = quad(8.0f, 8.0f, 60.0f, 56.0f);
    scale_quads(clock, sizes, shared, kAtlas, Owner::QuestHud);
    std::vector<Vertex> bar = quad(328.0f, 208.0f, 456.0f, 216.0f);
    scale_quads(bar, sizes, shared, 0x09163C20u, Owner::QuestHud);
    std::vector<Vertex> overhead = quad(205.0f, 128.0f, 213.0f, 142.0f);
    scale_quads(overhead, sizes, shared, kFont, Owner::QuestHud);
    std::vector<Vertex> tile = quad(232.0f, 140.0f, 248.0f, 148.0f);
    scale_quads(tile, sizes, shared, kAtlas, Owner::QuestHud);
    check(overhead[0].x == 205.0f && tile[0].x == 232.0f, "the letter and the tile in the middle stay as drawn");
    std::vector<Vertex> letter = quad(26.0f, 58.0f, 34.0f, 72.0f);
    scale_quads(letter, sizes, shared, kFont, Owner::QuestHud);
    check(letter[0].x != 26.0f, "a party name's letter of the reticle's size still takes Status while aiming");
    std::vector<Vertex> corner = quad(312.0f, 208.0f, 328.0f, 216.0f);
    scale_quads(corner, sizes, shared, kAtlas, Owner::QuestHud);
    check(corner[0].x != 312.0f, "an item bar corner of the reticle's size still takes the item bar's size");
}

// Quest windows on their first frame, before the frame state knows they are
// up. Traced with MHFU_TRACE_HUD_SCALE=all in an ad hoc quest.
void test_quest_windows_keep_their_pieces() {
    using mhfu::gpu::hud::Owner;
    using mhfu::gpu::hud_scale::Frame;
    using mhfu::gpu::hud_scale::Scales;
    using mhfu::gpu::hud_scale::scale_quads;
    struct Vertex {
        float x;
        float y;
    };
    const auto quad = [](float x0, float y0, float x1, float y1) {
        std::vector<Vertex> q(6, Vertex{x0, y0});
        q[2] = {x1, y1};
        return q;
    };
    const auto stays = [](const std::vector<Vertex> &q, float x0, float y0) { return q[0].x == x0 && q[0].y == y0; };
    const Scales sizes = Scales::from_percent(60u, 75u, 65u);
    Frame frame;
    std::vector<Vertex> clock = quad(8.0f, 8.0f, 60.0f, 56.0f);
    scale_quads(clock, sizes, frame, 0x09156BA0u, Owner::QuestHud);
    std::vector<Vertex> tile = quad(316.0f, 32.0f, 468.0f, 184.0f);
    scale_quads(tile, sizes, frame, mhfu::gpu::hud_scale::kAreaMapTexture, Owner::QuestHud);
    std::vector<Vertex> bar = quad(328.0f, 208.0f, 456.0f, 216.0f);
    scale_quads(bar, sizes, frame, 0x09163C20u, Owner::QuestHud);

    // "Lou broke off from the group!": the item bar's frame, in the middle.
    std::vector<Vertex> left = quad(104.0f, 128.0f, 120.0f, 144.0f);
    scale_quads(left, sizes, frame, 0x091643A0u, Owner::QuestHud);
    check(stays(left, 104.0f, 128.0f), "the banner's left side is not a status icon");
    std::vector<Vertex> item_side = quad(312.0f, 216.0f, 328.0f, 224.0f);
    scale_quads(item_side, sizes, frame, 0x091643A0u, Owner::QuestHud);
    check(!stays(item_side, 312.0f, 216.0f), "the same frame in the item bar's place is still the item bar");

    // A list window over the map draws its side before its body.
    std::vector<Vertex> side = quad(344.0f, 48.0f, 360.0f, 160.0f);
    scale_quads(side, sizes, frame, 0x09165BA0u, Owner::QuestHud);
    check(stays(side, 344.0f, 48.0f), "a window's side over the map is not a mark on it");
    std::vector<Vertex> mark = quad(351.0f, 165.0f, 363.0f, 177.0f);
    scale_quads(mark, sizes, frame, mhfu::gpu::hud_scale::kMapIconTexture, Owner::QuestHud);
    check(!stays(mark, 351.0f, 165.0f), "a mark on the map still takes the map's size");

    // Its title strip, a row above where the ammo bar is drawn.
    std::vector<Vertex> strip = quad(312.0f, 176.0f, 456.0f, 192.0f);
    scale_quads(strip, sizes, frame, 0x09165C20u, Owner::QuestHud);
    std::vector<Vertex> edge = quad(312.0f, 192.0f, 456.0f, 200.0f);
    scale_quads(edge, sizes, frame, 0x09166420u, Owner::QuestHud);
    check(stays(strip, 312.0f, 176.0f) && stays(edge, 312.0f, 192.0f), "a window's title strip is not the ammo bar");
    Frame gun;
    std::vector<Vertex> gun_clock = quad(8.0f, 8.0f, 60.0f, 56.0f);
    scale_quads(gun_clock, sizes, gun, 0x09156BA0u, Owner::QuestHud);
    std::vector<Vertex> ammo = quad(312.0f, 184.0f, 472.0f, 200.0f);
    scale_quads(ammo, sizes, gun, 0u, Owner::QuestHud);
    check(!stays(ammo, 312.0f, 184.0f), "the ammo bar where it was traced still takes the item bar's size");
}

// A quest menu's window over the party list: Combine, the kitchen skills,
// the equipment pages. Traced with MHFU_TRACE_CAPTURE and
// MHFU_TRACE_HUD_SCALE=all in a quest.
void test_menu_over_the_party_list() {
    using mhfu::gpu::hud::Owner;
    using mhfu::gpu::hud_scale::Frame;
    using mhfu::gpu::hud_scale::Scales;
    using mhfu::gpu::hud_scale::scale_quads;
    struct Vertex {
        float x;
        float y;
    };
    const auto quad = [](float x0, float y0, float x1, float y1) {
        std::vector<Vertex> q(6, Vertex{x0, y0});
        q[2] = {x1, y1};
        return q;
    };
    const auto stays = [](const std::vector<Vertex> &q, float x0, float y0) { return q[0].x == x0 && q[0].y == y0; };
    constexpr std::uint32_t kAtlas = 0x09156BA0u;
    constexpr std::uint32_t kFont = 0x040CC000u;
    const Scales sizes = Scales::from_percent(60u, 70u, 65u);
    const auto draw = [&](Frame &frame, float x0, float y0, float x1, float y1, std::uint32_t texture) {
        std::vector<Vertex> q = quad(x0, y0, x1, y1);
        scale_quads(q, sizes, frame, texture, Owner::QuestHud);
        return stays(q, x0, y0);
    };

    // No menu: the party list's pieces take Status.
    Frame plain;
    draw(plain, 8.0f, 8.0f, 60.0f, 56.0f, kAtlas);
    check(!draw(plain, 26.0f, 58.0f, 34.0f, 72.0f, kFont), "a name's letter takes Status");
    check(!draw(plain, 96.0f, 56.0f, 104.0f, 72.0f, kAtlas), "the plate's end takes Status");

    // The menu's corner at the left edge: no Status piece is 16 by 8.
    Frame menu;
    draw(menu, 8.0f, 8.0f, 60.0f, 56.0f, kAtlas);
    check(draw(menu, 12.0f, 40.0f, 28.0f, 48.0f, kAtlas), "a menu's corner in the clock's box stays");
    check(draw(menu, 8.0f, 152.0f, 24.0f, 160.0f, kAtlas), "a menu's corner in the badge column stays");
    // The window's own pieces in the party list's band stay as drawn.
    check(draw(menu, 16.0f, 58.0f, 40.0f, 82.0f, 0x091874A0u), "the equipment page's icon stays");
    check(draw(menu, 16.0f, 96.0f, 23.0f, 110.0f, kFont), "a menu letter on a name's row stays");
    check(draw(menu, 114.0f, 96.0f, 121.0f, 110.0f, kFont), "a menu letter past the plates stays");
    check(draw(menu, 65.0f, 128.0f, 79.0f, 142.0f, kFont), "a wide menu letter the size of a buff stays");
    // The party list itself, under the window, still takes Status.
    check(!draw(menu, 24.0f, 56.0f, 96.0f, 72.0f, kAtlas), "the plate under the menu takes Status");
    check(!draw(menu, 96.0f, 56.0f, 104.0f, 72.0f, kAtlas), "the plate's end under the menu takes Status");
    check(!draw(menu, 26.0f, 58.0f, 34.0f, 72.0f, kFont), "a name's letter under the menu takes Status");
    check(!draw(menu, 26.0f, 90.0f, 98.0f, 92.0f, 0u), "a teammate's health under the menu takes Status");

    // The next frame, before the corner is drawn again, the menu is still up.
    menu.begin();
    draw(menu, 8.0f, 8.0f, 60.0f, 56.0f, kAtlas);
    check(draw(menu, 16.0f, 96.0f, 23.0f, 110.0f, kFont), "the menu counts until a frame without it");
    menu.begin();
    menu.begin();
    draw(menu, 8.0f, 8.0f, 60.0f, 56.0f, kAtlas);
    check(!draw(menu, 112.0f, 58.0f, 124.0f, 72.0f, 0x09187000u), "after the menu, a buff takes Status again");
}

int main() {
    test_interface_sizes();
    test_interface_sizes_by_owner();
    test_quest_clock();
    test_aim_reticle_is_not_a_map_mark();
    test_paintball_ring_stays_on_map();
    test_quest_windows_keep_their_pieces();
    test_menu_over_the_party_list();
    test_hud_scale_draws();
    test_crisp_text();
    test_layered_armor();
    test_lock_on_and_damage_numbers();
    test_desktop_is_the_declared_defaults();
    test_android();
    test_this_build();
    test_control_presets();
    test_controls_from_the_previous_version();
    test_controls_from_main();
    if (failures != 0) {
        std::cerr << failures << " failure(s)\n";
        return 1;
    }
    std::cout << "settings defaults tests passed\n";
    return 0;
}
