#pragma once

namespace mhp3rd {

// Applies changed network settings (hle_adhoc.cpp). Turning ad hoc play off
// takes the game off line at once, as a normal disconnect; a new server or
// nickname is used the next time the game goes on line.
void adhoc_apply_settings();

// True while the game is in an ad hoc group or joining one: pausing it then
// would stop it answering the other players.
[[nodiscard]] bool adhoc_session_active();

} // namespace mhp3rd
