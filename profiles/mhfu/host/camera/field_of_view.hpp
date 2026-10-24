#pragma once

#include "camera/game_camera.hpp"

#include <cstdint>
#include <span>

namespace psprecomp {
class Runtime;
}

// Widens or narrows the game's 3D view (Video, Field of view).
//
// USA Freedom Unite (ULUS-10391). The camera object (game_aspect.hpp) keeps
// the vertical field of view at +0xC and the one a preset asks for at +0xA8.
// Each frame the game copies the active preset's field of view into +0xA8 and
// calls the camera's update (0x08815384), which builds the projection and the
// culling planes from +0xC; a second update (0x08815964) first moves +0xC a
// ninth of the way to +0xA8. A quest's camera has 0.872222 radians (50
// degrees), the village 1.01118 (58).
//
// On a quest (game_task.ovl loaded), the setting scales the field of view
// for those two updates only, by the setting over 50 degrees, and puts the
// game's own values back after each. The village and the Guild Hall keep the
// game's view.
// The game's logic never sees the scaled value, so its presets and its
// smoothing go on as before, and what it culls matches the wider picture.
// Both updates are called from other generated units, so a host wrapper sees
// those calls; a call from inside their own unit is not seen and draws that
// frame with the game's own view.
//
// MHFU_TRACE_FOV=1 prints the camera's field of view, the one its projection
// was built with and its target whenever one changes.
namespace mhfu::camera {

// Everything the field of view relies on in the game's code, checked before
// anything is wrapped. Exposed for tests.
[[nodiscard]] std::span<const CodeWord> field_of_view_signature();

// Checks the game's code against field_of_view_signature(). Returns false,
// having said why, if it differs; the setting then does nothing.
bool prepare_field_of_view(psprecomp::Runtime &runtime);

// True once prepare_field_of_view() accepted the game's code.
[[nodiscard]] bool field_of_view_available();

// Once per game frame, at the flip: takes the setting, in vertical degrees.
// The wrappers go in the first time a quest runs with it away from the
// game's 50, and stay; at 50 or off a quest they only pass through.
void field_of_view_frame(psprecomp::Runtime &runtime, std::uint32_t degrees);

} // namespace mhfu::camera
