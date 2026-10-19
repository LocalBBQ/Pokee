#pragma once

// Wrappers that share a guest function. Item Sets (game/item_sets.hpp) and
// Change Equipment in the Guild Hall (game/hall_equipment.hpp) both wrap the
// item box's frame, 0x09ADCCD0, and each checks every frame that its wrapper
// is still in place, since lobby_task registers its functions again each time
// it is installed. Compared with its own wrapper alone, each took the other's
// for the game's function and wrapped it again, and from the second frame
// the two called each other without end.
//
// wrap() keeps, per address, the wrappers put over the function found there.
// A wrapper already among them stays where it is, under or over the others;
// a function at the address that is none of them is the game's again, and
// the wrappers start a new chain on it.

#include "psprecomp/runtime.hpp"

#include <cstdint>
#include <string>

namespace mhfu::game::hooks {

// Puts `wrapper` over the function at `address`, which `next` then holds,
// unless the wrapper is already in the chain there. False when nothing is
// registered at the address.
bool wrap(psprecomp::Runtime &runtime, std::uint32_t address, psprecomp::Runtime::RecompiledFunction wrapper,
    psprecomp::Runtime::RecompiledFunction &next, const std::string &name);

} // namespace mhfu::game::hooks
