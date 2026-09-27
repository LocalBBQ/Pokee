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
    check(!parse("A / B / C", slots) && slots[0] == single(key(4)), "three bindings are refused, leaving the old ones");
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
            std::set<std::pair<Binding, Binding>> seen;
            bool unique = true;
            for (const Slots &slots : *table)
                for (const Chord &c : slots)
                    if (!c.empty() && !seen.insert({c.modifier, c.main}).second) unique = false;
            check(unique, "no input does two things in a shipped preset");
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

void test_assign() {
    Bindings b = default_keys();
    assign(b, Action::Cross, single(key(9)));  // F, which ○ has
    check(slots_of(b, Action::Cross)[0] == single(key(44)) && slots_of(b, Action::Cross)[1] == single(key(9)),
          "a new key fills the free slot");
    check(slots_of(b, Action::Circle)[1] == single(key(9)), "and the control that had it keeps it");
    const std::vector<Conflict> found = conflicts(b, Action::Cross);
    check(found.size() == 1u && found[0].kind == Conflict::Kind::Same && found[0].other == Action::Circle,
          "which is shown as a conflict");
    assign(b, Action::Cross, single(key(10)));
    check(slots_of(b, Action::Cross)[0] == single(key(44)) && slots_of(b, Action::Cross)[1] == single(key(10)),
          "with both slots full the second is replaced");
    check(conflicts(b, Action::Cross).empty(), "and the conflict is gone");
    assign(b, Action::Cross, single(key(44)));
    check(slots_of(b, Action::Cross)[0] == single(key(10)) && slots_of(b, Action::Cross)[1].empty(),
          "pressing a key the control has removes it");
    assign(b, Action::Cross, Chord{});
    check(slots_of(b, Action::Cross)[0] == single(key(10)), "nothing pressed changes nothing");
    assign(b, Action::TriangleCircle, chord(key(20), mouse_button(1)));  // Q + Mouse Left
    const std::vector<Conflict> modifier = conflicts(b, Action::TriangleCircle);
    check(modifier.size() == 1u && modifier[0].kind == Conflict::Kind::Modifier && modifier[0].other == Action::L,
          "a chord's modifier that does something alone is a conflict");
    check(conflicts(b, Action::Triangle).empty(), "a chord's main input done alone is not");
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
    assign(b, Action::TriangleCircle, chord(key(225), key(9)));
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
    test_assign();
    test_read();
    test_mouse_turn();
    std::cout << (failures ? "FAIL" : "PASS") << ": input bindings (" << failures << " failures)\n";
    return failures ? 1 : 0;
}
