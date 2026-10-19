#pragma once

// Calls a list of the game's functions from a host wrapper, one after the
// other, as the function the wrapper replaces would have: the wrapper hands
// over the list and returns, each function runs as ordinary guest code and
// returns to a stub, and after the last one the replaced function returns to
// its caller with the value the wrapper chose. Used to draw with the game's
// own windows, text and sounds (game/item_sets.hpp).
//
// Each call gets up to eight arguments in a0..a3 and t0..t3 and a ninth word
// at 0(sp), as the game passes them. The calls run in a frame of their own
// below the caller's stack pointer; the callee-saved registers are the
// callees' to keep, so the caller sees what it would have after the replaced
// function. Only one list runs at a time.

#include <array>
#include <cstdint>
#include <vector>

namespace psprecomp {
class Runtime;
struct AllegrexContext;
} // namespace psprecomp

namespace mhfu::game::guest_calls {

// Where the calls return, in the window the host reserves (kernel/kernel.hpp).
inline constexpr std::uint32_t kReturnStub = 0x08800010u;
// The calls' frame: room for the ninth argument and the callees' own use of
// their caller's argument area.
inline constexpr std::uint32_t kFrame = 32u;

struct Call {
    std::uint32_t function{};
    std::array<std::uint32_t, 8> args{}; // a0..a3, t0..t3
    std::uint32_t stack{};               // the word at 0(sp)
};

// Puts the return stub in place; doing it again changes nothing.
void install(psprecomp::Runtime &runtime);

// From a wrapper entered at the start of the function it replaces: runs
// `calls` in order, then returns to ctx.gpr[31] with v0 = `result`. Returns
// at once, with `result`, when the list is empty or another one is running.
void run(psprecomp::Runtime &runtime, psprecomp::AllegrexContext &ctx, std::vector<Call> calls, std::uint32_t result);

// A list is running: its functions have been entered and not all returned.
[[nodiscard]] bool running();

} // namespace mhfu::game::guest_calls
