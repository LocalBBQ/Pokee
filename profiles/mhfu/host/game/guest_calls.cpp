#include "game/guest_calls.hpp"

#include "psprecomp/runtime.hpp"

#include <utility>

namespace mhfu::game::guest_calls {
namespace {

struct State {
    std::vector<Call> calls;
    std::size_t next{};
    std::uint32_t return_address{};
    std::uint32_t result{};
    bool active{};
};

State &state() {
    static State s;
    return s;
}

void enter(psprecomp::Runtime &runtime, psprecomp::AllegrexContext &ctx, const Call &call) {
    for (std::uint32_t i = 0; i < 4u; ++i) ctx.set_gpr(4u + i, call.args[i]);
    for (std::uint32_t i = 0; i < 4u; ++i) ctx.set_gpr(8u + i, call.args[4u + i]);
    runtime.memory().store32(ctx.gpr[29], call.stack);
    ctx.set_gpr(31, kReturnStub);
    ctx.pc = call.function;
}

void finish(psprecomp::AllegrexContext &ctx) {
    State &s = state();
    s.active = false;
    s.calls.clear();
    ctx.set_gpr(29, ctx.gpr[29] + kFrame);
    ctx.set_gpr(2, s.result);
    ctx.set_gpr(31, s.return_address);
    ctx.pc = s.return_address;
}

void returned(psprecomp::Runtime &runtime, psprecomp::AllegrexContext &ctx) {
    State &s = state();
    if (!s.active) {
        runtime.stop("a guest call returned with none running");
        return;
    }
    if (s.next < s.calls.size()) {
        enter(runtime, ctx, s.calls[s.next++]);
        return;
    }
    finish(ctx);
}

} // namespace

void install(psprecomp::Runtime &runtime) {
    if (runtime.function_at(kReturnStub) != &returned)
        runtime.register_function(kReturnStub, &returned, "mhfu_guest_calls_return");
}

void run(psprecomp::Runtime &runtime, psprecomp::AllegrexContext &ctx, std::vector<Call> calls, std::uint32_t result) {
    State &s = state();
    if (calls.empty() || s.active) {
        ctx.set_gpr(2, result);
        ctx.pc = ctx.gpr[31];
        return;
    }
    install(runtime);
    s.calls = std::move(calls);
    s.next = 1u;
    s.return_address = ctx.gpr[31];
    s.result = result;
    s.active = true;
    ctx.set_gpr(29, ctx.gpr[29] - kFrame);
    enter(runtime, ctx, s.calls.front());
}

bool running() {
    return state().active;
}

} // namespace mhfu::game::guest_calls
