#pragma once

// Code the game rewrites in its own executable. The corpus is generated from
// EBOOT.ELF as it is on the disc, so once the game changes an instruction
// there, the compiled code no longer does what memory says. Freedom Unite
// Complete's loader does this at start: 48 words, three jumps into its own
// code and the constants of two routines. A patch mod can do it too.
//
// Launch keeps .text as it was loaded. At each instruction cache flush the
// units holding a changed word are dropped whole, since code inside a unit
// jumps between its blocks directly: dropping one function would leave the
// rest of its unit running into the old body. The interpreter then runs those
// units from memory. MHFU_DROP_CHANGED_CODE=0 keeps the compiled code.

#include "psprecomp/guest_memory.hpp"
#include "psprecomp/runtime.hpp"

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace mhfu::resident_code {

// generate.sh leaves psp_recomp's unit span at its default.
inline constexpr std::uint32_t kUnitSpan = 0x4000u;

struct Range {
    std::uint32_t start{};
    std::uint32_t end{};
    friend bool operator==(const Range &, const Range &) = default;
};

// The units of `span` bytes counted from `unit_base` that hold a word that
// differs between `loaded` and `now`, both read from `start`, lowest first.
[[nodiscard]] std::vector<Range> changed_units(std::span<const std::uint8_t> loaded, std::span<const std::uint8_t> now,
    std::uint32_t start, std::uint32_t unit_base, std::uint32_t span);

// Keeps [start, end) as it is now.
void remember(const psprecomp::GuestMemory &memory, std::uint32_t start, std::uint32_t end);

// Drops each unit, from `unit_base`, whose code changed since remember(),
// once. Returns how many units this call dropped.
std::size_t drop_changed_units(psprecomp::Runtime &runtime, std::uint32_t unit_base);

} // namespace mhfu::resident_code
