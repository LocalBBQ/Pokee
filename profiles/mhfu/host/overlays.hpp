#pragma once

#include "mhfu_profile.hpp"

#include "psprecomp/runtime.hpp"

namespace mhfu {

// Loads the overlay libraries and installs the dispatch-miss hook that
// recognises the overlay currently in a slot and registers its recompiled
// corpus. The libraries are read from MHFU_OVERLAY_DIR, or from overlays/
// next to the executable. With MHFU_DUMP_OVERLAYS set, an unknown overlay is
// written out instead so it can be recompiled.
void install_overlay_support(psprecomp::Runtime &runtime);

// Drops the corpus of any slot whose contents no longer match it. The guest
// flushes the instruction cache right after loading an overlay, and a chained
// call into a slot whose header has already changed does the same check first,
// because that jump can happen before the flush. The next lookup then installs
// the image that is in memory now.
void revalidate_overlays(psprecomp::Runtime &runtime);

// The guest loaded code: a slot whose image matched no corpus is searched
// again at the next call into it.
void forget_unmatched_overlays();

// Whether the overlay installed at `slot` is the image with `hash` and its
// library makes every call to `address` through the runtime
// (config/overlay_dispatch.txt), so that a host wrapper there sees them all.
// False for a library built before its list included the address.
[[nodiscard]] bool installed_overlay_dispatches(std::uint32_t slot, std::uint64_t hash, std::uint32_t address);

} // namespace mhfu
