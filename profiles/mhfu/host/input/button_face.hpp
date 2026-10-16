#pragma once

// The face of the button that presses one PSP button, as the device in use
// shows it: the letter in the colour of an Xbox pad, the symbol of a
// PlayStation pad, the letter of a Nintendo pad, or the name of a key. Drawn
// beside the gather mark (game/gather_icon.hpp) so the hint matches the
// controller and the player's bindings. Nothing here needs SDL or ImGui.

#include "input/bindings.hpp"

#include <cstdint>
#include <string>

namespace mhfu::input {

// The input the player presses for `action` on one device: the first chord
// of a single input in its slots, else the first input of its first chord.
// On a gamepad the confirm setting swaps the bottom and right buttons for
// every binding, so the pressed input is the swapped one. kNone when the
// action has nothing bound.
[[nodiscard]] Binding pressed_for(const Bindings &bindings, Action action, bool pad, bool confirm_south);

struct ButtonFace {
    // Letter: `text` in a coloured disc. Symbol shapes are drawn rather than
    // typed. Label: `text` (a key or another input's name) in a key cap.
    enum class Shape : std::uint8_t { Letter, Circle, Cross, Square, Triangle, Label };
    Shape shape{Shape::Label};
    std::string text;
    // The disc for Letter, the stroke for the symbols, the cap for Label.
    std::uint8_t red{}, green{}, blue{};
};

// `pressed` as `style` shows it. The four face buttons get their face; every
// other input (and a face button with Generic) is labelled as the bindings
// menu labels it, except that Generic face buttons take the PSP symbols.
[[nodiscard]] ButtonFace button_face(Binding pressed, PadStyle style);

} // namespace mhfu::input
