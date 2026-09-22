#pragma once

#include <cstdint>
#include <optional>
#include <span>

namespace psprecomp {
class Runtime;
struct AllegrexContext;
class GuestMemory;
}

// Drives the game's own camera from camera::take() (camera_input.hpp).
//
// NPJB-40001 only. The game's camera update calls a rotation helper once per
// update; the port wraps that helper and, for that one caller, adjusts the
// camera's angle and eye offset before the game runs its collision checks. The
// game keeps every other decision: presets, recentring, cutscenes and every
// camera mode that has no driver here yet, which then keeps the stock stick.
namespace mhp3rd::camera {

using RotationFunction = void (*)(psprecomp::Runtime &, psprecomp::AllegrexContext &);

// One instruction or constant of the game that the driver depends on.
struct CodeWord {
    std::uint32_t address;
    std::uint32_t word;
    const char *what;
};
// Everything the driver relies on in the game's code, checked before it is
// ever installed. Exposed for tests, which build a stand-in for that code.
[[nodiscard]] std::span<const CodeWord> game_code_signature();

// Checks the game's code against game_code_signature() and remembers the
// generated function behind the rotation helper. Nothing is hooked yet: that
// waits until the player turns Analog camera on, so a player who never does
// pays nothing at all. Returns false, having said why, if the code differs.
bool prepare_game_camera(psprecomp::Runtime &runtime, RotationFunction original);

// Once per presented game frame, at the game's flip (never per interpolated
// present): installs the hook the first time the option is on, and notices
// when the camera update has stopped running.
void game_camera_frame(psprecomp::Runtime &runtime);

// The port is driving the game's camera right now, so the game must not also
// act on the second stick: its own turn and vertical presets would fight ours.
[[nodiscard]] bool game_camera_driving();

// A bow or a bowgun is aiming under the driver: the stick goes to the game,
// stretched to full length, so the game's aim code steps at any push and the
// driver can size each step.
[[nodiscard]] bool game_camera_aim_boost();

// A mouse has no stick for the game's aim code to read. While a bow or a
// bowgun aims and the mouse has moved, the direction, at full length, the
// second stick should show the game this sample so its aim steps the mouse's
// way; the driver then sizes those steps from the mouse's degrees.
struct StickDirection {
    float x{};
    float y{};
};
[[nodiscard]] std::optional<StickDirection> game_camera_mouse_aim();

// Where the port does not drive the camera (Analog camera off, the village,
// a camera mode without a driver) the game's own turn is one speed, on or
// off, and the mouse can only switch it: -1 or +1 while the mouse moves left
// or right fast enough this frame, 0 otherwise. Vertical motion has nowhere
// to go there: the game's second stick up and down are recentring commands.
[[nodiscard]] int game_camera_mouse_stock_turn();

// Full-deflection speed for the current camera: Aim speed while a bow or a
// bowgun aims, Camera speed otherwise.
[[nodiscard]] float game_camera_degrees_per_second();

} // namespace mhp3rd::camera
