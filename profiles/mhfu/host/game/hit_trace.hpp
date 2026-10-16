#pragma once

// MHFU_TRACE_HITS: what the game's code looks like around a hit on a monster.
//
// The health routine damage numbers watch (game/damage_numbers.hpp,
// 0x09AC7A08) receives the monster and the amount; by then the point of
// contact, the part that was struck, its hitzone and the critical roll are
// gone. They live in the caller, the attack-against-hitbox resolution. This
// trace is how that caller is found: at each call of the health routine it
// prints the registers, the chain of return addresses and the functions they
// lie in, the words on the stack, and the words behind every argument that
// points into RAM; at the return, the health before and after.
//
// Reading the output: `ra@start` names the caller and where its function
// begins; the stack words marked `*` look like code addresses and are the
// older return addresses. Words that change from one part of the monster to
// another, with the same weapon, are the part or its hitzone; words that
// change with the weapon alone are the attack's own.
//
// Pure functions over a GuestMemory and a register file, so the tests run
// them on a buffer. Nothing here changes guest memory.

#include "psprecomp/allegrex_context.hpp"
#include "psprecomp/guest_memory.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace mhfu::game::hit_trace {

// MHFU_TRACE_HITS is set. Read once.
[[nodiscard]] bool enabled() noexcept;

// The emulated clock the ad hoc trace stamps its lines with (t=…ms), so a
// health change can be matched to the packet that arrived before it. The
// host sets it; until then the time is 0.
using GuestClock = std::uint64_t (*)();
void set_clock(GuestClock clock) noexcept;
[[nodiscard]] std::uint64_t now_ms() noexcept;
// How many hits are traced before the trace falls silent:
// MHFU_TRACE_HITS_LIMIT, kDefaultLimit when unset or not a number.
inline constexpr std::uint32_t kDefaultLimit = 200u;
[[nodiscard]] std::uint32_t parse_limit(const char *text) noexcept;
[[nodiscard]] std::uint32_t limit() noexcept;

// User RAM, where the game's objects and code are.
inline constexpr std::uint32_t kRamStart = 0x08800000u;
inline constexpr std::uint32_t kRamEnd = 0x0A000000u;
[[nodiscard]] bool in_ram(std::uint32_t address) noexcept;
// The game's code: the resident executable and the overlay slots. A stack
// word in one of these ranges is probably a return address.
[[nodiscard]] bool looks_like_code(std::uint32_t word) noexcept;

// A function's frame, found from a return address into it: the nearest
// `addiu sp, sp, -N` before it, and where after that it saves ra (~0u when it
// does not).
struct Frame {
    std::uint32_t start{};
    std::uint32_t size{};
    std::uint32_t ra_offset{~0u};
};
[[nodiscard]] std::optional<Frame> frame_of(const psprecomp::GuestMemory &memory, std::uint32_t ra);

// The guest call chain from a return address and stack pointer, as
// " ra@start" per level, innermost first, up to `levels` of them.
[[nodiscard]] std::string call_chain(
    const psprecomp::GuestMemory &memory, std::uint32_t ra, std::uint32_t sp, int levels = 10);

// `count` words from `address` as hex, code-looking ones marked with `*`;
// empty when the range is not mapped.
[[nodiscard]] std::string words_line(
    const psprecomp::GuestMemory &memory, const char *label, std::uint32_t address, std::uint32_t count);

// The same words read as floats, for records that hold positions.
[[nodiscard]] std::string floats_line(
    const psprecomp::GuestMemory &memory, const char *label, std::uint32_t address, std::uint32_t count);

// Every player on the quest entity list (camera/lock_on.hpp) and its position
// at +0x200, to tell a position in a hit's record from the hunter's own.
// Empty outside a quest.
[[nodiscard]] std::string players_line(const psprecomp::GuestMemory &memory);

// The registers at the call: the arguments, the saved registers, ra, sp,
// the float arguments and the monster's health.
[[nodiscard]] std::string call_line(
    const psprecomp::AllegrexContext &ctx, std::uint32_t hit, std::uint32_t depth, std::optional<int> health);
[[nodiscard]] std::string return_line(
    const psprecomp::AllegrexContext &ctx, std::uint32_t hit, std::uint32_t depth, int before, int after);

// Everything printed for one call, one line each, without the newlines.
[[nodiscard]] std::vector<std::string> describe_call(const psprecomp::GuestMemory &memory,
    const psprecomp::AllegrexContext &ctx, std::uint32_t hit, std::uint32_t depth, std::optional<int> health);

// The routine is also called every frame by the monster's own update, with
// nothing to take off (seen on Akantor: 200 such calls in the first seconds,
// none a hit). So a call is only remembered here, by its depth, and printed
// at its return when the health changed. A caller whose calls never change
// it is named once. `log_return` returns the hit's number, 0 when nothing
// was printed; printed hits count against the limit.
void note_call(const psprecomp::GuestMemory &memory, const psprecomp::AllegrexContext &ctx, std::uint32_t depth,
    std::optional<int> health);
std::uint32_t log_return(const psprecomp::AllegrexContext &ctx, std::uint32_t depth, int before, int after);

// Whether the hit goes through that routine at all. Each monster the routine
// was called with is read again once a frame; a drop that no call this frame
// accounted for is reported, because the hit was then written elsewhere.
using HealthOf = std::optional<int> (*)(const psprecomp::GuestMemory &, std::uint32_t object);
void frame(const psprecomp::GuestMemory &memory, HealthOf health_of);

// For tests: the count, the callers and the monsters start again.
void reset() noexcept;

} // namespace mhfu::game::hit_trace
