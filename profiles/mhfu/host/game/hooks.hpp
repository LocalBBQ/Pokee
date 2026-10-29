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
// registered at the address. `chained` registers it as register_chained does.
bool wrap(psprecomp::Runtime &runtime, std::uint32_t address, psprecomp::Runtime::RecompiledFunction wrapper,
    psprecomp::Runtime::RecompiledFunction &next, const std::string &name, bool chained = false);

// A host wrapper generated code calls straight into. The runtime chains a
// call only into functions registered under a generated unit's name; any
// other sends the calling guest function back to the dispatcher at that call,
// to go on later (src/runtime.cpp). A wrapper around that caller then sees it
// return before it has finished. Registered first under `name`, which takes
// the wrapper's 16 KiB bucket off the direct path so a call looks the wrapper
// up, and then under the unit name, which lets that lookup chain into it.
// Only for a wrapper that switches no thread: imports and HLE calls must not
// run inline.
inline void register_chained(psprecomp::Runtime &runtime, std::uint32_t address,
    psprecomp::Runtime::RecompiledFunction wrapper, const std::string &name) {
    runtime.register_function(address, wrapper, name);
    runtime.register_function(address, wrapper, "recomp_unit_" + name);
}

} // namespace mhfu::game::hooks
