#pragma once

// An analog camera for a game that does not have one.
//
// The game turns its camera at one fixed speed: past about two thirds of the
// second stick's travel it adds 1150 sixteen-thousandths of a turn to its own
// yaw every frame -- 6.317139 degrees -- and below that it does nothing at all.
// Measured in #103, read out of the game's code for #106.
//
// That yaw is an ordinary 16-bit field in guest memory, so the port can turn
// the camera itself, by however much the player actually asked for. While this
// is on, the port keeps the second stick centred as far as the game is
// concerned, so the game never turns the camera and the two never fight; the
// port adds the player's own rate to the field instead. Off writes nothing and
// reports the stick unchanged, which is the stock camera exactly rather than a
// restoration of it.
//
// The field has to be found before it can be written, and it moves with the
// game's memory, so it is found by what only it does: of every 16-bit word in
// 64 MiB, one changes by exactly the game's own step from one frame to the
// next while the camera turns. Finding it needs the game to turn the camera,
// so the port hands the stick through until it has, and takes over after that.

#include <cstdint>

namespace psprecomp {
class Runtime;
}

namespace mhp3rd::input {

// Once per presented frame, before the guest's next camera update.
// `turn` is the degrees the camera turned last frame, from the view matrix;
// `deflection` is the player's stick, -1 to 1, after dead zone and inversion.
void analog_camera_frame(psprecomp::Runtime &runtime, float turn, float deflection, float pitch_change, float yaw_degrees, float pitch_now, float stick_y);

// True while the port is driving the camera, so the second stick handed to the
// game is centred and the game leaves the camera alone.
[[nodiscard]] bool analog_camera_driving();

// True while the port is driving the camera up and down, so the game's own
// one-shot vertical commands are not fired from the same stick and the two do
// not fight. Below the dead zone this is false and the game keeps the camera.
[[nodiscard]] bool analog_camera_vertical_driving();

} // namespace mhp3rd::input
