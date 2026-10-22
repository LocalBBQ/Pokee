#pragma once

// True element: the equipment screens show a weapon's element and status as
// the game counts them, without the trailing zero, as FUComplete's True
// Ele/Status does. Fire shown as 250 shows 25. Part of True Attack Value
// (game/true_attack.hpp), off by default. Only the text changes; damage never
// reads it.
//
// Read from USA Freedom Unite (ULUS-10391). A weapon's element and status are
// kept as the game counts them, in 10-byte records at 0x08979878 (u16 weapon,
// then two pairs of u16 element bits and s16 value), and 0x088DA740 applies
// the hunter's element skills to a value. Nothing multiplies it for the
// screens: the text that prints it ends in a literal zero. In the English
// table (DATA.BIN entry 19) those formats are ":~C%02d%3d0" and
// "(Total Val):~C%02d%2d0"; the other languages (entries 20 to 23) end the
// same way, and no other text or the executable has a "%<width>d0". The
// screens print with sprintf (0x0880E8F0, format in a1), e.g. from
// 0x08823784. It is wrapped rather than the formatter it calls (0x0880F68C),
// because the generated code reaches that one with a jump inside its own
// unit, which a registered wrapper never sees; sprintf is only entered from
// other units.
//
// While the switch is on, the wrapper changes a format that ends in
// "%<width>d0" to "%<width+1>d" before sprintf reads it, so the number fills
// the same width without the zero, and leaves it changed: sprintf can stop at
// a kernel call and finish later, after the wrapper has returned. Turned off,
// the game's bytes go back at the next frame. Text the game loads again is
// changed again at its next print. Every other format is the game's.
// MHFU_TRACE_ELEMENT=1 prints each format it changes, its caller, and when
// they are put back.

#include "psprecomp/guest_memory.hpp"

#include <array>
#include <cstdint>
#include <optional>
#include <span>

namespace psprecomp {
class Runtime;
}

namespace mhfu::game::true_element {

inline constexpr std::uint32_t kSprintf = 0x0880E8F0u;
inline constexpr std::uint32_t kLongestFormat = 512u;

struct CodeWord {
    std::uint32_t address;
    std::uint32_t word;
};

// What the addresses above were read off. A mismatch means another
// executable, and nothing is wrapped.
[[nodiscard]] std::span<const CodeWord> code_signature();
[[nodiscard]] bool code_matches(const psprecomp::GuestMemory &memory);

// Where the "%<width>d0" that ends the format at `format` starts, and the
// four bytes that stand for it without the zero. Nothing for any other
// format, a width of 9, or one that leaves memory or runs past
// kLongestFormat.
struct Rewrite {
    std::uint32_t address;
    std::array<std::uint8_t, 4> bytes;
};
[[nodiscard]] std::optional<Rewrite> rewrite_for(const psprecomp::GuestMemory &memory, std::uint32_t format);

// At the flip: puts the wrapper in place once the switch is on and the
// executable's code is installed; it stays for the run. Runs every frame;
// wrapping again is a no-op.
void frame(psprecomp::Runtime &runtime);

} // namespace mhfu::game::true_element
