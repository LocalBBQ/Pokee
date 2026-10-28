#include "resident_code.hpp"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <set>
#include <string_view>

namespace mhfu::resident_code {
namespace {

struct State {
    std::uint32_t start{};
    std::vector<std::uint8_t> loaded;
    std::set<std::uint32_t> dropped;
};

State &state() {
    static State s;
    return s;
}

bool enabled() {
    static const bool on = [] {
        const char *value = std::getenv("MHFU_DROP_CHANGED_CODE");
        return value == nullptr || std::string_view(value) != "0";
    }();
    return on;
}

} // namespace

std::vector<Range> changed_units(std::span<const std::uint8_t> loaded, std::span<const std::uint8_t> now,
    std::uint32_t start, std::uint32_t unit_base, std::uint32_t span) {
    std::vector<Range> units;
    if (span == 0u || start < unit_base) return units;
    const std::size_t size = std::min(loaded.size(), now.size()) & ~std::size_t{3u};
    for (std::size_t offset = 0; offset < size; offset += 4u) {
        if (std::memcmp(loaded.data() + offset, now.data() + offset, 4u) == 0) continue;
        const std::uint32_t address = start + static_cast<std::uint32_t>(offset);
        const std::uint32_t unit = unit_base + (address - unit_base) / span * span;
        if (units.empty() || units.back().start != unit) units.push_back({unit, unit + span});
        // Nothing more in this unit can add to it.
        offset = unit + span - start - 4u;
    }
    return units;
}

void remember(const psprecomp::GuestMemory &memory, std::uint32_t start, std::uint32_t end) {
    State &s = state();
    s.start = start;
    s.loaded.resize(end > start ? end - start : 0u);
    for (std::size_t i = 0; i < s.loaded.size(); ++i) s.loaded[i] = memory.load8(start + static_cast<std::uint32_t>(i));
    s.dropped.clear();
}

std::size_t drop_changed_units(psprecomp::Runtime &runtime, std::uint32_t unit_base) {
    State &s = state();
    if (s.loaded.empty() || !enabled()) return 0u;
    const auto size = static_cast<std::uint32_t>(s.loaded.size());
    std::vector<std::uint8_t> copy;
    std::span<const std::uint8_t> now;
    if (const std::uint8_t *raw = runtime.memory().raw_pointer(s.start, size); raw != nullptr) {
        now = {raw, size};
    } else {
        copy.resize(size);
        for (std::uint32_t i = 0; i < size; ++i) copy[i] = runtime.memory().load8(s.start + i);
        now = copy;
    }
    std::size_t count = 0u;
    for (const Range &unit : changed_units(s.loaded, now, s.start, unit_base, kUnitSpan)) {
        if (!s.dropped.insert(unit.start).second) continue;
        runtime.unregister_functions(unit.start, unit.end);
        ++count;
        std::cout << "[code] the game changed its code in 0x" << std::hex << unit.start << "-0x" << unit.end << std::dec
                  << "; that part runs in the interpreter" << std::endl;
    }
    return count;
}

} // namespace mhfu::resident_code
