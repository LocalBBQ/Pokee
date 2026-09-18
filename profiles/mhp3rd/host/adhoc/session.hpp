#pragma once

namespace mhp3rd {

// Applies changed network settings (hle_adhoc.cpp). Turning ad hoc play off
// takes the game off line at once, as a normal disconnect; a new server or
// nickname is used the next time the game goes on line.
void adhoc_apply_settings();

} // namespace mhp3rd
