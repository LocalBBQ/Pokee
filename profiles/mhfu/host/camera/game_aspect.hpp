#pragma once

#include "camera/game_camera.hpp"

#include <span>

namespace psprecomp {
class Runtime;
}

// Gives the game's 3D view the shape of the picture it is drawn into.
//
// USA Freedom Unite (ULUS-10391). The projection's parameters sit at the start
// of the object whose address is stored at 0x089CBD04: near plane, far plane,
// aspect ratio and vertical field of view at +0x0, +0x4, +0x8 and +0xC. The
// set-up plants 480/272 with a lui/ori pair, and the camera's update builds
// the projection again only when the field of view differs from the one kept
// at +0x10. Those immediates are baked into the recompiled code, so this
// writes the live camera and leaves the instructions as they are.
namespace mhfu::camera {

// Everything the aspect ratio relies on in the game's code, checked before
// anything is written. Exposed for tests.
[[nodiscard]] std::span<const CodeWord> game_aspect_signature();

// Checks the game's code against game_aspect_signature(). Returns false,
// having said why, if it differs; nothing is ever written then.
bool prepare_game_aspect(psprecomp::Runtime &runtime);

// Once per game frame, at the flip: gives the game `aspect` (width over
// height of the picture) when it differs from what the game has, or puts back
// the game's own 480/272 when the port had changed it. At exactly 480/272 and
// with nothing changed before, it writes nothing at all.
void game_aspect_frame(psprecomp::Runtime &runtime, float aspect);

} // namespace mhfu::camera
