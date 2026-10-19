// Wrappers that share a guest function: two of them checked every frame, in
// either order, run once each around the game's function, and a function
// registered again starts a new chain. No game data.
#include "game/hooks.hpp"

#include "psprecomp/runtime.hpp"

#include <cstdint>
#include <iostream>

namespace {
using mhfu::game::hooks::wrap;
using Function = psprecomp::Runtime::RecompiledFunction;

int failures{};

void check(bool condition, const char *message) {
    if (!condition) {
        ++failures;
        std::cerr << "FAIL: " << message << '\n';
    }
}

constexpr std::uint32_t kBox = 0x09ADCCD0u;
constexpr std::uint32_t kEmpty = 0x09ADD000u;
// More calls than this in one call of the box means the wrappers call each
// other; the guard stops the test there instead of overflowing the stack.
constexpr int kRunaway = 8;

int game_calls{}, reloaded_calls{}, sets_calls{}, hall_calls{}, depth{};
Function sets_next{}, hall_next{};

void game_box(psprecomp::Runtime &, psprecomp::AllegrexContext &) {
    ++game_calls;
}

// A body of its own, so the linker does not fold it into game_box.
void game_box_reloaded(psprecomp::Runtime &, psprecomp::AllegrexContext &) {
    ++game_calls;
    ++reloaded_calls;
}

void sets_box(psprecomp::Runtime &runtime, psprecomp::AllegrexContext &ctx) {
    ++sets_calls;
    if (++depth > kRunaway) return;
    sets_next(runtime, ctx);
}

void hall_box(psprecomp::Runtime &runtime, psprecomp::AllegrexContext &ctx) {
    ++hall_calls;
    if (++depth > kRunaway) return;
    hall_next(runtime, ctx);
}

void call_box(psprecomp::Runtime &runtime) {
    game_calls = reloaded_calls = sets_calls = hall_calls = depth = 0;
    psprecomp::AllegrexContext ctx{};
    runtime.function_at(kBox)(runtime, ctx);
}

// Both mods' frames, in the order present_frame runs them or the other.
void frames(psprecomp::Runtime &runtime, bool hall_first, int count) {
    for (int i = 0; i < count; ++i) {
        if (hall_first) check(wrap(runtime, kBox, &hall_box, hall_next, "hall"), "the hall's wrap finds the box");
        check(wrap(runtime, kBox, &sets_box, sets_next, "sets"), "item sets' wrap finds the box");
        if (!hall_first) check(wrap(runtime, kBox, &hall_box, hall_next, "hall"), "the hall's wrap finds the box");
    }
}

void test_two_wrappers(bool hall_first) {
    psprecomp::Runtime runtime(32u * 1024u * 1024u);
    runtime.register_function(kBox, &game_box, "game_box");
    frames(runtime, hall_first, 5);
    call_box(runtime);
    check(game_calls == 1 && sets_calls == 1 && hall_calls == 1,
        "after five frames each wrapper and the game's box run once, not each other without end");
    check(runtime.function_at(kBox) == (hall_first ? &sets_box : &hall_box), "the later wrapper stays outermost");
}

void test_reinstalled_function() {
    psprecomp::Runtime runtime(32u * 1024u * 1024u);
    runtime.register_function(kBox, &game_box, "game_box");
    frames(runtime, true, 2);
    // lobby_task installed again: the box is the game's function again.
    runtime.register_function(kBox, &game_box_reloaded, "game_box");
    frames(runtime, true, 3);
    check(hall_next == &game_box_reloaded, "the first wrapper wraps the function registered again");
    check(sets_next == &hall_box, "the second wrapper wraps the first again");
    call_box(runtime);
    check(game_calls == 1 && reloaded_calls == 1 && sets_calls == 1 && hall_calls == 1,
        "after the reload each runs once, around the function registered again");
}

void test_nothing_registered() {
    psprecomp::Runtime runtime(32u * 1024u * 1024u);
    Function next = &game_box;
    check(!wrap(runtime, kEmpty, &hall_box, next, "hall"), "nothing to wrap where nothing is registered");
    check(next == &game_box, "next is left as it was");
    check(runtime.function_at(kEmpty) == nullptr, "and nothing is registered there");
}

} // namespace

int main() {
    test_two_wrappers(true);
    test_two_wrappers(false);
    test_reinstalled_function();
    test_nothing_registered();
    if (failures != 0) {
        std::cerr << failures << " failure(s)\n";
        return 1;
    }
    std::cout << "hooks tests passed\n";
    return 0;
}
