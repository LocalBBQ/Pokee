#pragma once

namespace mhfu::perf {

// MHFU_TRACE_STALL_STACKS (Linux): when the game thread has been inside one
// HLE call for more than 45 ms, a watchdog thread interrupts it and prints
// where it is, as a [stall-stack] line and one address per frame, again every
// 15 ms up to four times a call. It answers what the thread waits on in a stall
// that the stall trace can only place in a call (`slowest call`). Addresses
// in Pokee resolve with `addr2line -f -C -e Pokee`. Elsewhere it does nothing.
[[nodiscard]] bool stall_stacks_enabled();
// Around each HLE call, on the game thread. `name` must outlive the program.
void stall_call_begin(const char *name);
void stall_call_end();

} // namespace mhfu::perf
