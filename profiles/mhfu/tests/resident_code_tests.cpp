// Units of the executable the game rewrites, and dropping their compiled code.
// No game data.
#include "resident_code.hpp"

#include "psprecomp/runtime.hpp"

#include <cstdint>
#include <iostream>
#include <vector>

namespace {
int failures = 0;

void check(bool condition, const char *message) {
    if (!condition) {
        ++failures;
        std::cerr << "FAIL: " << message << '\n';
    }
}

using mhfu::resident_code::changed_units;
using mhfu::resident_code::Range;

constexpr std::uint32_t kBase = 0x08804000u;
constexpr std::uint32_t kSpan = mhfu::resident_code::kUnitSpan;

void finds_units() {
    std::vector<std::uint8_t> loaded(3u * kSpan, 0u);
    std::vector<std::uint8_t> now = loaded;
    check(changed_units(loaded, now, kBase, kBase, kSpan).empty(), "unchanged code drops nothing");

    now[kSpan + 8u] = 1u;
    now[kSpan + 0x100u] = 1u;
    check(changed_units(loaded, now, kBase, kBase, kSpan) == std::vector<Range>{{kBase + kSpan, kBase + 2u * kSpan}},
        "two changed words in one unit drop that unit once");

    now[3u * kSpan - 1u] = 1u;
    check(changed_units(loaded, now, kBase, kBase, kSpan) ==
            std::vector<Range>{{kBase + kSpan, kBase + 2u * kSpan}, {kBase + 2u * kSpan, kBase + 3u * kSpan}},
        "the last byte of the last unit counts");

    std::vector<std::uint8_t> first = loaded;
    first[0] = 1u;
    check(changed_units(loaded, first, kBase, kBase, kSpan) == std::vector<Range>{{kBase, kBase + kSpan}},
        "the first word counts");
}

void follows_unit_base() {
    // Text that starts inside a unit: units are counted from the load base.
    const std::uint32_t start = kBase + 0x3FF0u;
    std::vector<std::uint8_t> loaded(0x20u, 0u);
    std::vector<std::uint8_t> now = loaded;
    now[0x0Cu] = 1u; // 0x08807FFC, the last word of unit 0
    now[0x10u] = 1u; // 0x08808000, the first word of unit 1
    check(changed_units(loaded, now, start, kBase, kSpan) ==
            std::vector<Range>{{kBase, kBase + kSpan}, {kBase + kSpan, kBase + 2u * kSpan}},
        "words either side of a unit boundary fall in their own units");
    check(changed_units(loaded, now, kBase - 4u, kBase, kSpan).empty(), "text before the unit base is refused");
    check(changed_units(loaded, now, start, kBase, 0u).empty(), "a zero span is refused");
}

void step(psprecomp::Runtime &, psprecomp::AllegrexContext &cpu) {
    cpu.pc += 4u;
}

void drops_once() {
    psprecomp::Runtime runtime(32u * 1024u * 1024u);
    runtime.register_function(kBase, step, "recomp_unit_zero");
    runtime.register_function(kBase + kSpan, step, "recomp_unit_one");
    runtime.register_function(kBase + kSpan + 0x40u, step, "recomp_unit_one");
    runtime.register_function(kBase + 2u * kSpan, step, "recomp_unit_two");
    mhfu::resident_code::remember(runtime.memory(), kBase, kBase + 3u * kSpan);
    check(mhfu::resident_code::drop_changed_units(runtime, kBase) == 0u, "nothing is dropped before a change");

    runtime.memory().store32(kBase + kSpan + 0x80u, 0x0A28D644u);
    check(mhfu::resident_code::drop_changed_units(runtime, kBase) == 1u, "a changed unit is dropped");
    check(!runtime.has_function(kBase + kSpan) && !runtime.has_function(kBase + kSpan + 0x40u),
        "every function of the changed unit is gone");
    check(runtime.has_function(kBase) && runtime.has_function(kBase + 2u * kSpan), "the units around it stay");

    runtime.memory().store32(kBase + kSpan + 0x84u, 0u);
    check(mhfu::resident_code::drop_changed_units(runtime, kBase) == 0u, "a unit is dropped once");
}
} // namespace

int main() {
    finds_units();
    follows_unit_base();
    drops_once();
    if (failures != 0) return 1;
    std::cout << "resident code tests passed\n";
    return 0;
}
