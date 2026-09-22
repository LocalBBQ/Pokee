#pragma once

namespace psprecomp {
class Runtime;
struct AllegrexContext;
}

namespace mhp3rd::input {

// NPJB-40001 only; install after generated functions and profile HLE bindings.
using CameraRotationFunction = void (*)(psprecomp::Runtime &, psprecomp::AllegrexContext &);
void install_analog_camera(psprecomp::Runtime &runtime, CameraRotationFunction original);

// Sample the host right stick once per presented frame. Dead zone and axis
// inversion have already been applied by the input layer.
void analog_camera_frame(float x, float y);

// Suppress the game's digital right-stick commands, on both axes, only while
// its ordinary follow-camera calculation is active. Menus and special cameras
// retain input.
[[nodiscard]] bool analog_camera_driving();

} // namespace mhp3rd::input
