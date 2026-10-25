#include "hle_common.hpp"

#include "perf/frame_stats.hpp"
#include "perf/stall_stacks.hpp"
#include "psprecomp/common.hpp"

#include <cstdlib>
#include <iostream>
#include <set>
#include <string>
#include <unordered_set>

namespace mhfu {

std::string read_cstring(const psprecomp::GuestMemory &memory, std::uint32_t address, std::size_t max_length) {
    std::string text;
    if (address == 0u) return text;
    for (std::size_t i = 0; i < max_length; ++i) {
        const auto c = static_cast<char>(memory.load8(address + static_cast<std::uint32_t>(i)));
        if (c == '\0') break;
        text.push_back(c);
    }
    return text;
}

void write_cstring(psprecomp::GuestMemory &memory, std::uint32_t address, std::string_view text, std::size_t capacity) {
    if (address == 0u || capacity == 0u) return;
    const std::size_t count = std::min(text.size(), capacity - 1u);
    for (std::size_t i = 0; i < count; ++i)
        memory.store8(address + static_cast<std::uint32_t>(i), static_cast<std::uint8_t>(text[i]));
    memory.store8(address + static_cast<std::uint32_t>(count), 0u);
}

void store64(psprecomp::GuestMemory &memory, std::uint32_t address, std::uint64_t value) {
    memory.store32(address, static_cast<std::uint32_t>(value));
    memory.store32(address + 4u, static_cast<std::uint32_t>(value >> 32u));
}

HleRegistrar::HleRegistrar(Runtime &runtime) : runtime_(runtime) {
    for (const auto &symbol : runtime.nids().all()) nids_by_name_[{symbol.library, symbol.name}] = symbol.nid;
}

bool HleRegistrar::try_add(std::string_view library, std::string_view name, HleFunction function) {
    const auto found = nids_by_name_.find({std::string(library), std::string(name)});
    if (found == nids_by_name_.end()) return false;
    if (perf::stall_trace_enabled() || perf::stall_stacks_enabled()) {
        // The call's own host time, for the [slow-frame] line's slowest call,
        // and where it is when it runs long (MHFU_TRACE_STALL_STACKS).
        // A set keeps each name at one address for the whole run.
        static std::set<std::string> names;
        const char *label = names.insert(std::string(library) + "::" + std::string(name)).first->c_str();
        function = [inner = std::move(function), label](Runtime &rt, AllegrexContext &ctx) {
            static const bool stacks = perf::stall_stacks_enabled();
            if (stacks) perf::stall_call_begin(label);
            const perf::Clock::time_point start = perf::Clock::now();
            const perf::Clock::duration accounted = perf::accounted_time();
            inner(rt, ctx);
            perf::note_hle_call(label, perf::Clock::now() - start - (perf::accounted_time() - accounted));
            if (stacks) perf::stall_call_end();
        };
    }
    runtime_.register_hle(std::string(library), found->second, std::move(function));
    bound_.insert({std::string(library), found->second});
    return true;
}

void HleRegistrar::add(std::string_view library, std::string_view name, HleFunction function) {
    if (!try_add(library, name, std::move(function)))
        throw psprecomp::Error("Unknown HLE function " + std::string(library) + "::" + std::string(name) +
            " (missing from configs/nids.csv)");
}

bool HleRegistrar::bound(const std::string &library, std::uint32_t nid) const {
    return bound_.contains({library, nid});
}

bool trace_sync() {
    static const bool enabled = std::getenv("MHFU_TRACE_SYNC") != nullptr;
    return enabled;
}

void log_sync(const std::string &message) {
    static std::uint64_t remaining = [] {
        const char *limit = std::getenv("MHFU_TRACE_SYNC_LIMIT");
        return limit != nullptr ? std::strtoull(limit, nullptr, 10) : 4000ull;
    }();
    if (remaining == 0u) return;
    --remaining;
    const Thread *thread = kernel().current_thread();
    std::cerr << "[sync] " << (thread != nullptr ? thread->name : "interrupt") << " " << message << "\n";
}

void log_once(const std::string &key, const std::string &message) {
    static std::unordered_set<std::string> seen;
    if (seen.insert(key).second) std::cerr << message << "\n";
}

} // namespace mhfu
