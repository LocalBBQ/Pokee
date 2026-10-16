#include "input/button_face.hpp"

namespace mhfu::input {

Binding pressed_for(const Bindings &bindings, Action action, bool pad, bool confirm_south) {
    const Slots &slots = bindings[static_cast<std::size_t>(action)];
    Binding chosen = kNone;
    for (const Chord &c : slots)
        if (c.size() == 1u) {
            chosen = c.inputs[0];
            break;
        }
    if (chosen == kNone)
        for (const Chord &c : slots)
            if (!c.empty()) {
                chosen = c.inputs[0];
                break;
            }
    if (!pad || !confirm_south) return chosen;
    if (chosen == input::pad(PadInput::South)) return input::pad(PadInput::East);
    if (chosen == input::pad(PadInput::East)) return input::pad(PadInput::South);
    return chosen;
}

namespace {

struct Face {
    ButtonFace::Shape shape;
    const char *text;
    std::uint8_t red, green, blue;
};

// South, east, west and north, as each pad shows them.
constexpr Face kXbox[] = {
    {ButtonFace::Shape::Letter, "A", 96, 176, 72},
    {ButtonFace::Shape::Letter, "B", 214, 64, 58},
    {ButtonFace::Shape::Letter, "X", 58, 116, 214},
    {ButtonFace::Shape::Letter, "Y", 226, 178, 44},
};
constexpr Face kPlayStation[] = {
    {ButtonFace::Shape::Cross, "", 124, 178, 232},
    {ButtonFace::Shape::Circle, "", 255, 102, 102},
    {ButtonFace::Shape::Square, "", 232, 150, 206},
    {ButtonFace::Shape::Triangle, "", 64, 200, 160},
};
// By place: Nintendo's A is on the right, where Xbox has B.
constexpr Face kNintendo[] = {
    {ButtonFace::Shape::Letter, "B", 70, 70, 76},
    {ButtonFace::Shape::Letter, "A", 70, 70, 76},
    {ButtonFace::Shape::Letter, "Y", 70, 70, 76},
    {ButtonFace::Shape::Letter, "X", 70, 70, 76},
};

ButtonFace from(const Face &face) {
    return ButtonFace{face.shape, face.text, face.red, face.green, face.blue};
}

} // namespace

ButtonFace button_face(Binding pressed, PadStyle style) {
    const int input = pad_input_of(pressed);
    if (input >= static_cast<int>(PadInput::South) && input <= static_cast<int>(PadInput::North)) {
        const auto n = static_cast<std::size_t>(input);
        switch (style) {
        case PadStyle::Xbox:
            return from(kXbox[n]);
        case PadStyle::Nintendo:
            return from(kNintendo[n]);
        case PadStyle::PlayStation:
        case PadStyle::Generic:
            return from(kPlayStation[n]);
        }
    }
    return ButtonFace{ButtonFace::Shape::Label, pressed == kNone ? std::string() : label(pressed, style), 70, 70, 76};
}

} // namespace mhfu::input
