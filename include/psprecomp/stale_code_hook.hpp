#pragma once

// Kept out of runtime.hpp on purpose. Generated units include that header, and
// a change there rebuilds every one of them. This hook is only called from the
// runtime and set by a profile.

namespace psprecomp {
class Runtime;

// Return true when the compiled function already registered at `pc` must not
// run because those bytes have been replaced. invoke_chained_call skips the
// body and the caller unwinds to dispatch, which looks the address up again.
// A null hook changes nothing, and this does not force compile-time direct
// chains onto the indirect path.
using RuntimeStaleCodeHook = bool (*)(Runtime &, std::uint32_t pc);
void set_runtime_stale_code_hook(RuntimeStaleCodeHook hook) noexcept;

} // namespace psprecomp
