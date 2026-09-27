#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <string_view>
#include <vector>

// Bindings for the game (#94, #165): which keys, mouse buttons and gamepad
// buttons press each PSP control, alone or held together with another one.
//
// A binding is one input: a key, by its position on the keyboard (a USB HID
// usage, the numbers SDL's scancodes use), a mouse button, or a gamepad
// button or trigger, by its place on the pad. Positions rather than
// characters or labels, so W A S D stay where they are on any keyboard
// layout and the pad's bottom button is the bottom button on any pad. A chord
// is one input, or a modifier held with a main input, such as LB + B. Nothing
// here needs SDL, so settings.ini is read the same way in every build.
namespace mhp3rd::input {

enum class Action : std::uint8_t {
    StickUp,
    StickLeft,
    StickDown,
    StickRight,
    Triangle,
    Circle,
    Cross,
    Square,
    L,
    R,
    Start,
    Select,
    Up,
    Left,
    Down,
    Right,
    // The HD release's second stick, pushed fully that way while held.
    CameraUp,
    CameraLeft,
    CameraDown,
    CameraRight,
    // △ and ○ in the same frame: the game's combined attacks, which two
    // fingers or two keys do not always manage together.
    TriangleCircle,
    // Not a PSP control: runs the game faster than real time while held, or
    // turns that on and off (kernel/fast_forward.hpp). Single player only.
    FastForward,
    Count
};
inline constexpr std::size_t kActions = static_cast<std::size_t>(Action::Count);
// Every action takes up to two chords on each device.
inline constexpr std::size_t kSlots = 2u;

// 0: none. 1-511: a key position. kMouse + 1..5: a mouse button, numbered as
// SDL numbers them (left, middle, right, back, forward). kPad + 1 + n: the
// gamepad input n below.
using Binding = std::uint16_t;
inline constexpr Binding kNone = 0u;
inline constexpr Binding kMouse = 0x200u;
inline constexpr Binding kPad = 0x300u;
inline constexpr std::uint16_t kKeyPositions = 512u;

// Gamepad inputs by place. The buttons have the values of SDL3's
// SDL_GamepadButton (the renderer checks that); the triggers, which SDL
// reports as axes, count as held past the trigger point.
enum class PadInput : std::uint8_t {
    South,
    East,
    West,
    North,
    Back,
    Guide,
    Start,
    LeftStick,
    RightStick,
    LeftShoulder,
    RightShoulder,
    DpadUp,
    DpadDown,
    DpadLeft,
    DpadRight,
    Misc1,
    RightPaddle1,
    LeftPaddle1,
    RightPaddle2,
    LeftPaddle2,
    Touchpad,
    Misc2,
    Misc3,
    Misc4,
    Misc5,
    Misc6,
    ButtonCount,
    LeftTrigger = 32,
    RightTrigger,
    Count
};

[[nodiscard]] constexpr Binding key(std::uint16_t position) { return position; }
[[nodiscard]] constexpr Binding mouse_button(int button) { return static_cast<Binding>(kMouse + button); }
[[nodiscard]] constexpr Binding pad(PadInput input) { return static_cast<Binding>(kPad + 1u + static_cast<unsigned>(input)); }
// The key position, or -1 for anything else.
[[nodiscard]] constexpr int key_position(Binding binding) {
    return binding != kNone && binding < kKeyPositions ? binding : -1;
}
// The mouse button, 1-5, or 0 for anything else.
[[nodiscard]] constexpr int mouse_button_of(Binding binding) {
    return binding > kMouse && binding <= kMouse + 5u ? binding - kMouse : 0;
}
// The gamepad input, or -1 for anything else.
[[nodiscard]] constexpr int pad_input_of(Binding binding) {
    return binding > kPad && binding <= kPad + static_cast<unsigned>(PadInput::Count) ? binding - kPad - 1 : -1;
}
[[nodiscard]] constexpr bool is_pad(Binding binding) { return pad_input_of(binding) >= 0; }

// One input, or `modifier` held together with `main`. An empty chord has no
// main input.
struct Chord {
    Binding modifier{kNone};
    Binding main{kNone};
    [[nodiscard]] constexpr bool empty() const { return main == kNone; }
    [[nodiscard]] constexpr bool combined() const { return modifier != kNone && main != kNone; }
    friend constexpr bool operator==(const Chord &, const Chord &) = default;
};
[[nodiscard]] constexpr Chord single(Binding binding) { return {kNone, binding}; }
[[nodiscard]] constexpr Chord chord(Binding modifier, Binding main) { return {modifier, main}; }
// Inputs usually held as a modifier: Shift, Ctrl, Alt and GUI; a pad's
// shoulders, triggers, Back and stick buttons.
[[nodiscard]] bool modifier_like(Binding binding);
// The chord of two inputs pressed in this order: the first is the modifier,
// unless only the second is modifier_like (two pressed in the same instant
// arrive in no particular order).
[[nodiscard]] Chord chord_pressed(Binding first, Binding second);

using Slots = std::array<Chord, kSlots>;
using Bindings = std::array<Slots, kActions>;

struct ActionInfo {
    const char *key;    // in settings.ini, after "input.bind." or "input.pad."
    const char *label;  // in the menu
};
[[nodiscard]] const ActionInfo &info(Action action);
// The SceCtrlButtons an action presses; 0 for the sticks.
[[nodiscard]] std::uint32_t buttons_of(Action action);

// "W", "Left Shift", "Mouse Left", "Pad South"; "Key 123" for a position
// without a name. These are the names settings.ini keeps.
[[nodiscard]] std::string name(Binding binding);
// The reverse of name(), ignoring case. kNone if the name is not known.
[[nodiscard]] Binding from_name(std::string_view text);

// How the menu names a gamepad's inputs: by place, or as the pad in use
// labels them.
enum class PadStyle { Generic, Xbox, PlayStation, Nintendo };
// name() for the menu: a pad input as `style` labels it ("LB", "L1", "L"), a
// key or a mouse button as name() has it.
[[nodiscard]] std::string label(Binding binding, PadStyle style);
[[nodiscard]] std::string label(const Chord &chord, PadStyle style);

// Slots as settings.ini keeps them: chords separated by " / ", the two inputs
// of a chord by " + ", empty for none. "Left Shift + F / Mouse Right".
[[nodiscard]] std::string format(const Slots &slots);
[[nodiscard]] std::string format(const Chord &chord);
// False, leaving `slots` alone, if a name is not known.
bool parse(std::string_view text, Slots &slots);

// The menu's way of changing a binding: pressing a chord the action has
// already removes it; anything else is added, replacing the second slot when
// both are full. Other actions keep what they have: a chord bound twice is
// shown as a conflict, not taken away.
void assign(Bindings &bindings, Action action, Chord chord);

// What clashes with a chord of an action.
struct Conflict {
    enum class Kind : std::uint8_t {
        Same,      // another action has the same chord: one press does both
        Modifier,  // the chord's modifier does another action on its own while held
    };
    Kind kind{};
    Action other{};
    Chord chord;  // the chord of `action` that clashes
};
[[nodiscard]] std::vector<Conflict> conflicts(const Bindings &bindings, Action action);

// What the held inputs press, in PSP terms. A chord presses its action while
// both its inputs are held, and its main input then does nothing else on its
// own: with LB + B bound to one action and B to another, holding LB and
// pressing B does only the first. The modifier keeps doing what it does
// alone.
struct PadState {
    std::uint32_t buttons{};  // SceCtrlButtons
    int stick_x{};            // -127..127 from the centre, each axis
    int stick_y{};
    int camera_x{};           // the second stick
    int camera_y{};
    bool fast_forward{};      // the fast-forward bind is held
};
[[nodiscard]] PadState read(const Bindings &bindings, const std::function<bool(Binding)> &held);

// The mouse's motion as degrees for the camera: positive yaw turns right,
// positive pitch looks down, as camera_input expects. `counts` are relative
// motion; `scale` is what the current camera makes of it (1, or Aim speed's
// share of Camera speed while a bow or a bowgun aims).
struct MouseTurn {
    float yaw{};
    float pitch{};
};
[[nodiscard]] MouseTurn mouse_turn(float counts_x, float counts_y, float degrees_per_count, bool invert_x,
                                   bool invert_y, float scale);

} // namespace mhp3rd::input
