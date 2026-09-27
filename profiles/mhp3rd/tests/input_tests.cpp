// Bindings and control presets, without SDL or game data.
#include "input/bindings.hpp"
#include "input/presets.hpp"

#include <cmath>
#include <cstdint>
#include <iostream>
#include <set>

namespace {
using namespace mhp3rd::input;
int failures{};

void check(bool condition, const char *message) {
    if (!condition) {
        ++failures;
        std::cerr << "FAIL: " << message << '\n';
    }
}

Slots slots_of(const Bindings &b, Action action) { return b[static_cast<std::size_t>(action)]; }
const Bindings &default_keys() { return layout(Preset::Default).keys; }
const Bindings &default_pad() { return layout(Preset::Default).pad; }

void test_names() {
    check(name(key(26)) == "W" && from_name("w") == key(26), "letters round-trip, ignoring case");
    check(name(key(225)) == "Left Shift" && from_name("left shift") == key(225), "modifiers have their names");
    check(name(key(40)) == "Enter" && from_name("Return") == key(40), "Enter also answers to SDL's Return");
    check(name(mouse_button(1)) == "Mouse Left" && from_name("Mouse Right") == mouse_button(3),
          "mouse buttons are named by their place");
    check(name(key(300)) == "Key 300" && from_name("Key 300") == key(300), "unnamed positions keep their number");
    check(from_name("Nonsense") == kNone && from_name("") == kNone, "unknown names are refused");
    for (std::uint16_t position = 1; position < kKeyPositions; ++position)
        if (from_name(name(key(position))) != key(position)) {
            check(false, "every key position round-trips through its name");
            break;
        }
    check(name(pad(PadInput::South)) == "Pad South" && from_name("pad south") == pad(PadInput::South),
          "pad buttons are named by their place");
    check(name(pad(PadInput::LeftTrigger)) == "Pad LT" && from_name("Pad RT") == pad(PadInput::RightTrigger),
          "and so are the triggers");
    check(from_name("Pad Up") == pad(PadInput::DpadUp) && from_name("Up") == key(82),
          "a pad's D-pad and the arrow keys are told apart");
    check(from_name("Pad Nonsense") == kNone, "unknown pad names are refused");
    for (int i = 0; i < static_cast<int>(PadInput::Count); ++i) {
        if (i >= static_cast<int>(PadInput::ButtonCount) && i < static_cast<int>(PadInput::LeftTrigger)) continue;
        const Binding b = pad(static_cast<PadInput>(i));
        if (from_name(name(b)) != b) {
            check(false, "every pad input round-trips through its name");
            break;
        }
    }
    check(label(pad(PadInput::LeftShoulder), PadStyle::PlayStation) == "L1" &&
              label(pad(PadInput::LeftShoulder), PadStyle::Xbox) == "LB" &&
              label(pad(PadInput::East), PadStyle::Nintendo) == "A",
          "the menu labels a pad's buttons as the pad does");
    check(label(chord(pad(PadInput::LeftShoulder), pad(PadInput::East)), PadStyle::PlayStation) == "L1 + ○",
          "and a chord with a plus");
    check(chord_pressed(pad(PadInput::East), pad(PadInput::LeftShoulder)) ==
              chord(pad(PadInput::LeftShoulder), pad(PadInput::East)) &&
              chord_pressed(key(9), key(225)) == chord(key(225), key(9)) &&
              chord_pressed(key(9), key(10)) == chord(key(9), key(10)),
          "a shoulder or Shift pressed with another is the modifier, whichever came first");
    check(!is_pad(key(26)) && !is_pad(mouse_button(1)) && is_pad(pad(PadInput::North)) &&
              key_position(pad(PadInput::North)) < 0 && mouse_button_of(pad(PadInput::North)) == 0,
          "the three kinds of input do not overlap");
}

void test_settings_spelling() {
    Slots slots{};
    check(parse("Mouse Right / F", slots) && slots[0] == single(mouse_button(3)) && slots[1] == single(key(9)),
          "two bindings separated by a slash");
    check(format(slots) == "Mouse Right / F", "and written back the same way");
    check(parse("/ / F", slots) && slots[0] == single(key(56)) && slots[1] == single(key(9)),
          "the slash key itself is a name");
    check(parse("F / /", slots) && slots[1] == single(key(56)), "also as the second binding");
    check(parse("", slots) && slots[0].empty() && slots[1].empty(), "empty means unbound");
    check(format(slots).empty(), "and unbound is written empty");
    slots = {single(key(4)), {}};
    check(!parse("A / B / C / D / E", slots) && slots[0] == single(key(4)),
          "more bindings than slots are refused, leaving the old ones");
    check(!parse("Q / Nonsense", slots) && slots[0] == single(key(4)), "an unknown name refuses the whole value");
    check(parse("Left Shift + F / Pad LB + Pad East", slots) && slots[0] == chord(key(225), key(9)) &&
              slots[1] == chord(pad(PadInput::LeftShoulder), pad(PadInput::East)),
          "chords are joined with a plus");
    check(format(slots) == "Left Shift + F / Pad LB + Pad East", "and written back the same way");
    check(!parse("A + B + C", slots) && !parse("F + F", slots), "three inputs, or one twice, are no chord");
    check(parse("Keypad Plus + =", slots) && slots[0] == chord(key(87), key(46)), "keys named like the separators");
    for (const Layout *l : {&layout(Preset::Default), &layout(Preset::Modern), &layout(Preset::LeftHanded),
                            &layout(Preset::Classic)})
        for (const Bindings *table : {&l->keys, &l->pad})
            for (std::size_t i = 0; i < kActions; ++i) {
                Slots back{};
                check(parse(format((*table)[i]), back) && back == (*table)[i], "every preset round-trips");
            }
}

void test_presets() {
    for (std::size_t p = 0; p < kPresets; ++p) {
        const Layout &l = layout(static_cast<Preset>(p));
        for (const Bindings *table : {&l.keys, &l.pad}) {
            // Frame step is read only in the photo mode, when the game has
            // no input, so it may share one with a game action.
            bool unique = true;
            for (std::size_t a = 0; a < kActions; ++a)
                for (std::size_t b = a + 1u; b < kActions; ++b) {
                    const Context ca = context_of(static_cast<Action>(a));
                    const Context cb = context_of(static_cast<Action>(b));
                    if (ca != cb && ca != Context::Anywhere && cb != Context::Anywhere) continue;
                    for (const Chord &x : (*table)[a])
                        for (const Chord &y : (*table)[b])
                            if (!x.empty() && x == y) unique = false;
                }
            for (const Slots &slots : *table)
                if (!slots[0].empty() && slots[0] == slots[1]) unique = false;
            check(unique, "no input does two things at once in a shipped preset");
            for (std::size_t i = 0; i < kActions; ++i)
                check(conflicts(*table, static_cast<Action>(i)).empty(), "and no preset has a conflict");
        }
        check(preset_from_id(info(static_cast<Preset>(p)).id) == static_cast<Preset>(p), "presets by their ids");
        check(matching_preset(l) == static_cast<Preset>(p), "each preset matches itself only");
    }
    const Bindings &d = default_keys();
    check(slots_of(d, Action::StickUp)[0] == single(from_name("W")) &&
              slots_of(d, Action::StickLeft)[0] == single(from_name("A")) &&
              slots_of(d, Action::StickDown)[0] == single(from_name("S")) &&
              slots_of(d, Action::StickRight)[0] == single(from_name("D")),
          "the default moves on W A S D");
    check(slots_of(d, Action::Triangle)[0] == single(mouse_button(1)), "the left mouse button attacks");
    check(slots_of(d, Action::TriangleCircle)[0].empty(), "the default has no key for △ + ○, as before");
    for (std::size_t p = 0; p < kPresets; ++p) {
        const Layout &l = layout(static_cast<Preset>(p));
        check(slots_of(l.keys, Action::FastForward)[0] == single(from_name("`")),
              "every keyboard preset fast-forwards on the key under Esc");
        check(slots_of(l.pad, Action::FastForward)[0].empty() && slots_of(l.pad, Action::FastForward)[1].empty(),
              "and no gamepad preset binds fast-forward yet");
        check(slots_of(l.keys, Action::HideHud)[0] == single(from_name("F7")),
              "every keyboard preset hides the HUD on F7");
        check(slots_of(l.pad, Action::HideHud)[0].empty() && slots_of(l.pad, Action::HideHud)[1].empty(),
              "and no gamepad preset binds hiding the HUD");
    }
    for (std::size_t p = 0; p < kPresets; ++p) {
        const Layout &l = layout(static_cast<Preset>(p));
        check(slots_of(l.keys, Action::Screenshot)[0] == single(from_name("F12")) &&
                  slots_of(l.keys, Action::Screenshot)[1] == single(from_name("PrintScreen")),
              "every keyboard preset takes a screenshot on F12 or Print Screen");
        check(slots_of(l.keys, Action::FrameStep)[0] == single(from_name(".")), "and steps a frame on .");
        const bool mirrored = l.swap_sticks;
        check(slots_of(l.pad, Action::Screenshot)[0] ==
                  (mirrored ? chord(pad(PadInput::LeftStick), pad(PadInput::West))
                            : chord(pad(PadInput::RightStick), pad(PadInput::DpadLeft))),
              "gamepads take a screenshot with the camera stick's button and D-pad left, mirrored left-handed");
        check(slots_of(l.pad, Action::FrameStep)[0] == single(pad(mirrored ? PadInput::East : PadInput::DpadRight)),
              "and step a frame on D-pad right, mirrored left-handed");
        // Neither pad chord is one the port already reads by itself: L3 + R3
        // opens the menu and Back + R3 turns the free camera on and off.
        for (const Chord &c : slots_of(l.pad, Action::Screenshot)) {
            const auto has = [&](PadInput input) { return c.modifier == pad(input) || c.main == pad(input); };
            check(!(has(PadInput::LeftStick) && has(PadInput::RightStick)), "the screenshot chord is not the menu's");
            check(!(has(PadInput::Back) && has(PadInput::RightStick)), "nor the free camera's");
        }
    }
    {
        // Frame step and a game action on one input do not clash; a
        // screenshot, taken during play as well, does.
        Bindings b{};
        b[static_cast<std::size_t>(Action::Right)] = {single(pad(PadInput::DpadRight)), {}};
        b[static_cast<std::size_t>(Action::FrameStep)] = {single(pad(PadInput::DpadRight)), {}};
        check(conflicts(b, Action::FrameStep).empty() && conflicts(b, Action::Right).empty(),
              "frame step shares an input with the game without a conflict");
        b[static_cast<std::size_t>(Action::Screenshot)] = {single(pad(PadInput::DpadRight)), {}};
        check(conflicts(b, Action::Screenshot).size() == 2u, "a screenshot on it clashes with both");
        const PadState held = read(b, [](Binding x) { return x == pad(PadInput::DpadRight); });
        check(held.frame_step && held.screenshot && (held.buttons & 0x0020u) != 0u, "each is read as held");
    }
    const Bindings &c = layout(Preset::Classic).keys;
    check(slots_of(c, Action::StickUp)[0] == single(from_name("I")) &&
              slots_of(c, Action::Circle)[0] == single(from_name("X")) &&
              slots_of(c, Action::Cross)[0] == single(from_name("Z")) &&
              slots_of(c, Action::R)[0] == single(from_name("W")) &&
              slots_of(c, Action::Select)[0] == single(from_name("Right Shift")) &&
              slots_of(c, Action::Select)[1] == single(from_name("Backspace")),
          "the classic layout is the one earlier versions had");
    // The pad as it was before presets: fixed buttons, the triggers L and R.
    const Bindings &p = default_pad();
    check(slots_of(p, Action::Cross)[0] == single(pad(PadInput::South)) &&
              slots_of(p, Action::Circle)[0] == single(pad(PadInput::East)) &&
              slots_of(p, Action::L)[1] == single(pad(PadInput::LeftTrigger)) &&
              slots_of(p, Action::R)[1] == single(pad(PadInput::RightTrigger)) &&
              slots_of(p, Action::Select)[0] == single(pad(PadInput::Back)),
          "the default pad is the fixed mapping of earlier versions");
    check(layout(Preset::LeftHanded).swap_sticks && !layout(Preset::Default).swap_sticks,
          "the left-handed preset moves with the right stick");
}

void test_earlier_versions() {
    const Layout standard = layout_from_earlier(default_keys(), "standard");
    check(standard == layout(Preset::Default), "the default keys and the standard triggers are Default");
    check(layout_from_earlier(layout(Preset::Classic).keys, "standard") == layout(Preset::Classic),
          "the classic keys are the Classic preset");
    const Layout bows = layout_from_earlier(default_keys(), "bows");
    check(slots_of(bows.pad, Action::R)[1] == single(pad(PadInput::LeftTrigger)) &&
              slots_of(bows.pad, Action::Triangle)[1] == single(pad(PadInput::RightTrigger)) &&
              slots_of(bows.pad, Action::L)[1].empty() && slots_of(bows.pad, Action::R)[0] == single(pad(PadInput::RightShoulder)),
          "bows: R on LT, △ on RT, the shoulders unchanged");
    const Layout bowguns = layout_from_earlier(default_keys(), "bowguns");
    check(slots_of(bowguns.pad, Action::Circle)[1] == single(pad(PadInput::RightTrigger)) &&
              slots_of(bowguns.pad, Action::Triangle)[1].empty(),
          "bowguns: ○ on RT");
    check(!matching_preset(bows) && !matching_preset(bowguns), "the shooting profiles are no shipped preset");
    for (std::size_t i = 0; i < kActions; ++i)
        check(conflicts(bows.pad, static_cast<Action>(i)).empty(), "and have no conflicts");
}

void test_names_of_presets() {
    std::vector<UserPreset> presets;
    check(unique_preset_name(presets, "Custom") == "Custom", "a free name is kept");
    presets.push_back({"Custom", {}});
    check(unique_preset_name(presets, "custom") == "custom 2", "a taken one, ignoring case, is numbered");
    check(unique_preset_name(presets, "Default") == "Default 2", "shipped names are taken too");
    check(clean_preset_name("  My = keys#\n ") == "My  keys", "spaces trimmed, the file's characters dropped");
    check(clean_preset_name(std::string(40, 'x')).size() == kMaxPresetName, "names are cut to their length");
    check(unique_preset_name(presets, "   ") == "Custom 2", "an empty name becomes Custom");
    PresetChoice choice{Preset::Modern, {}};
    check(format(choice) == "modern" && parse_choice("modern") == choice, "a shipped preset by its id");
    choice = {std::nullopt, "My keys"};
    check(format(choice) == "user:My keys" && parse_choice("user:My keys") == choice, "the player's by name");
    check(!parse_choice("nonsense") && !parse_choice("user:"), "anything else is refused");
}

void test_editing() {
    Bindings b = default_keys();
    check(count(slots_of(b, Action::Circle)) == 2u && count(slots_of(b, Action::TriangleCircle)) == 0u,
          "count() counts the chords an action has");
    check(add(b, Action::Cross, single(key(9))), "a new key is added");  // F, which ○ has
    check(slots_of(b, Action::Cross)[0] == single(key(44)) && slots_of(b, Action::Cross)[1] == single(key(9)),
          "after the ones the action has");
    check(slots_of(b, Action::Circle)[1] == single(key(9)), "and the control that had it keeps it");
    const std::vector<Conflict> found = conflicts(b, Action::Cross);
    check(found.size() == 1u && found[0].kind == Conflict::Kind::Same && found[0].other == Action::Circle &&
              found[0].theirs == single(key(9)),
          "which is shown as a conflict");
    check(!add(b, Action::Cross, single(key(44))), "adding a key the action has changes nothing");
    check(count(slots_of(b, Action::Cross)) == 2u, "so it is not bound twice, nor removed");
    check(add(b, Action::Cross, single(key(10))) && add(b, Action::Cross, single(key(11))),
          "up to kSlots chords");
    check(!add(b, Action::Cross, single(key(12))) && count(slots_of(b, Action::Cross)) == kSlots,
          "and never silently one more");
    check(!add(b, Action::Cross, Chord{}) && !add(b, Action::Cross, chord(key(4), key(4))),
          "nothing pressed, or one input twice, is no chord");

    // Replacing: in place, and never leaving the action with a chord twice.
    check(replace(b, Action::Cross, 1u, single(key(29))) && slots_of(b, Action::Cross)[1] == single(key(29)),
          "a chord is replaced in its slot");
    check(conflicts(b, Action::Cross).empty(), "and the conflict it had is gone");
    check(replace(b, Action::Cross, 0u, single(key(11))), "replacing with a chord the action has elsewhere");
    check(slots_of(b, Action::Cross)[0] == single(key(11)) && slots_of(b, Action::Cross)[1] == single(key(29)) &&
              slots_of(b, Action::Cross)[2] == single(key(10)) && slots_of(b, Action::Cross)[3].empty(),
          "moves it there");
    check(!replace(b, Action::Cross, 3u, single(key(14))), "an empty slot is not replaced; that is adding");

    // Clearing, including the last chord an action has.
    check(clear(b, Action::Cross, 0u), "a chord is cleared");
    check(slots_of(b, Action::Cross)[0] == single(key(29)) && slots_of(b, Action::Cross)[1] == single(key(10)) &&
              slots_of(b, Action::Cross)[2].empty(),
          "and the ones after it move up");
    check(!clear(b, Action::Cross, 2u), "an empty slot has nothing to clear");
    check(clear(b, Action::Cross, 1u) && clear(b, Action::Cross, 0u) && count(slots_of(b, Action::Cross)) == 0u,
          "an action can be left with nothing");
    check(read(b, [](Binding held) { return held == key(44) || held == key(29); }).buttons == 0u,
          "and then nothing presses it");
    check(format(slots_of(b, Action::Cross)).empty(), "which settings.ini keeps as empty");

    // Chords, and the conflict of a modifier that does something alone.
    check(add(b, Action::TriangleCircle, chord(key(20), mouse_button(1))), "a chord is added");  // Q + Mouse Left
    const std::vector<Conflict> modifier = conflicts(b, Action::TriangleCircle);
    check(modifier.size() == 1u && modifier[0].kind == Conflict::Kind::Modifier && modifier[0].other == Action::L &&
              modifier[0].theirs == single(key(20)),
          "a chord's modifier that does something alone is a conflict");
    check(conflicts(b, Action::Triangle).empty(), "a chord's main input done alone is not");
    check(remove(b, modifier[0].other, modifier[0].theirs) && conflicts(b, Action::TriangleCircle).empty(),
          "removing the other action's input fixes it");
    check(!remove(b, Action::L, single(key(20))), "and it is gone from there");
    check(remove(b, Action::TriangleCircle, chord(key(20), mouse_button(1))) &&
              count(slots_of(b, Action::TriangleCircle)) == 0u,
          "a chord is removed like a single input");
}

void test_slots_in_settings() {
    Slots slots{};
    check(parse("Left Shift + F / Mouse Right / G / Pad LB + Pad South", slots) && count(slots) == 4u &&
              slots[3] == chord(pad(PadInput::LeftShoulder), pad(PadInput::South)),
          "four chords are read");
    check(format(slots) == "Left Shift + F / Mouse Right / G / Pad LB + Pad South", "and written back");
    const Slots before = slots;
    check(!parse("A / B / C / D / E", slots) && slots == before, "a fifth is refused and nothing changes");
    check(parse("Mouse Right / F", slots) && count(slots) == 2u && slots[2].empty(),
          "two, as earlier versions wrote them, still read");
    check(parse("", slots) && count(slots) == 0u, "and none");
}

void test_groups() {
    std::size_t in_group[kActionGroups]{};
    for (std::size_t i = 0; i < kActions; ++i) ++in_group[static_cast<std::size_t>(group_of(static_cast<Action>(i)))];
    for (std::size_t g = 0; g < kActionGroups; ++g) check(in_group[g] != 0u, "every group has an action");
    check(group_of(Action::StickUp) == ActionGroup::Movement && group_of(Action::Triangle) == ActionGroup::Attacks &&
              group_of(Action::Square) == ActionGroup::Items && group_of(Action::CameraLeft) == ActionGroup::Camera &&
              group_of(Action::Start) == ActionGroup::System && group_of(Action::FastForward) == ActionGroup::Port,
          "actions are grouped by what they do");
}

void test_read() {
    const Bindings &d = default_keys();
    std::set<Binding> held;
    const auto read_held = [&](const Bindings &b) { return read(b, [&](Binding x) { return held.count(x) != 0u; }); };
    check(read_held(d).buttons == 0u && read_held(d).stick_x == 0 && read_held(d).camera_x == 0,
          "nothing held, nothing pressed");
    held = {from_name("W"), from_name("D")};
    PadState state = read_held(d);
    check(state.stick_x == 127 && state.stick_y == -127, "W and D push the stick up and right");
    held = {from_name("A"), from_name("D")};
    check(read_held(d).stick_x == 0, "opposite keys cancel");
    held = {mouse_button(1), mouse_button(3), from_name("Left Shift"), from_name("Q")};
    check(read_held(d).buttons == (0x1000u | 0x2000u | 0x0200u | 0x0100u),
          "mouse buttons and keys press their buttons");
    held = {from_name("F")};
    check(read_held(d).buttons == 0x2000u, "either binding of a control presses it");
    held = {from_name("J"), from_name("K")};
    state = read_held(d);
    check(state.camera_x == -127 && state.camera_y == 127 && state.buttons == 0u, "camera keys push the second stick");
    held = {from_name("Up"), from_name("Right"), from_name("Enter"), from_name("Backspace")};
    check(read_held(d).buttons == (0x0010u | 0x0020u | 0x0008u | 0x0001u), "D-pad, START and SELECT");

    // △ + ○ presses both in one frame.
    held = {mouse_button(4)};
    check(read_held(layout(Preset::Modern).keys).buttons == 0x3000u, "△ + ○ is both buttons at once");

    // Chords: Left Shift + F does △ + ○, and F alone stays ○.
    Bindings b = d;
    add(b, Action::TriangleCircle, chord(key(225), key(9)));
    held = {key(9)};
    check(read_held(b).buttons == 0x2000u, "the main input alone does its own action");
    held = {key(225)};
    check(read_held(b).buttons == 0x0200u, "the modifier alone does its own");
    held = {key(225), key(9)};
    check(read_held(b).buttons == (0x3000u | 0x0200u),
          "together they do the chord, the modifier's own action, and not the main input's");
    held = {key(225), mouse_button(3)};
    check(read_held(b).buttons == (0x2000u | 0x0200u), "the main input's other binding still works");

    // The pad of the Default preset.
    const Bindings &p = default_pad();
    held = {pad(PadInput::South), pad(PadInput::LeftTrigger), pad(PadInput::DpadUp)};
    check(read_held(p).buttons == (0x4000u | 0x0100u | 0x0010u), "pad buttons and triggers press their buttons");
}

void test_mouse_turn() {
    MouseTurn t = mouse_turn(10.0f, -20.0f, 0.1f, false, false, 1.0f);
    check(std::fabs(t.yaw - 1.0f) < 1e-5f && std::fabs(t.pitch + 2.0f) < 1e-5f,
          "right turns right and forward looks up, by the sensitivity");
    t = mouse_turn(10.0f, -20.0f, 0.1f, true, true, 0.5f);
    check(std::fabs(t.yaw + 0.5f) < 1e-5f && std::fabs(t.pitch - 1.0f) < 1e-5f, "inversion and the aim's share apply");
}

} // namespace

int main() {
    test_names();
    test_settings_spelling();
    test_presets();
    test_earlier_versions();
    test_names_of_presets();
    test_editing();
    test_slots_in_settings();
    test_groups();
    test_read();
    test_mouse_turn();
    std::cout << (failures ? "FAIL" : "PASS") << ": input bindings (" << failures << " failures)\n";
    return failures ? 1 : 0;
}
