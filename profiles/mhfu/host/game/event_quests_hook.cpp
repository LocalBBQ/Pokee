#include "game/event_quests.hpp"

#include "game/hooks.hpp"
#include "install/user_data.hpp"
#include "overlays.hpp"
#include "settings/settings.hpp"

#include "platform/utf8_path.hpp"
#include "psprecomp/guest_memory.hpp"
#include "psprecomp/runtime.hpp"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <string>

// The replacements and their place in the frame (event_quests.hpp); what they
// do with guest memory is in event_quests.cpp, which needs no game.
namespace mhfu::game::event_quests {
namespace {

struct State {
    bool loaded{};
    std::vector<Quest> quests;
    bool placed{};
    Bank bank;
    bool reported{};
    bool trace{};
};

State &state() {
    static State s;
    return s;
}

void return_value(psprecomp::AllegrexContext &ctx, std::uint32_t value) {
    ctx.gpr[2] = value;
    ctx.pc = ctx.gpr[31];
}

void getter(psprecomp::Runtime &runtime, psprecomp::AllegrexContext &ctx) {
    State &s = state();
    psprecomp::GuestMemory &memory = runtime.memory();
    const std::uint32_t lobby = ctx.gpr[4];
    const std::uint32_t k = ctx.gpr[5] & 0xFFu;
    const std::uint32_t info = save_slot_info(memory, lobby, k);
    // The list loop asks for the last slot: the built-in ids go in before it,
    // through the loop's write position (s2) and count (s4).
    if (k == kSlotCount - 1u && ctx.gpr[31] == kListLoopReturn && settings::current().event_quests) {
        const std::uint32_t added = extend_list(memory, s.bank, lobby, ctx.gpr[18], ctx.gpr[20], info);
        ctx.gpr[18] += 2u * added;
        ctx.gpr[20] += added;
        if (s.trace)
            std::cerr << "[event quests] list: " << ctx.gpr[20] + (info != 0u ? 1u : 0u) << " with " << added
                      << " built in\n";
    }
    return_value(ctx, info);
}

void lookup(psprecomp::Runtime &runtime, psprecomp::AllegrexContext &ctx) {
    State &s = state();
    static const Bank none;
    const std::uint16_t id = static_cast<std::uint16_t>(ctx.gpr[5]);
    const std::uint32_t info =
        find(runtime.memory(), settings::current().event_quests ? s.bank : none, ctx.gpr[4], id);
    if (s.trace) std::cerr << "[event quests] lookup " << id << " -> " << std::hex << info << std::dec << "\n";
    return_value(ctx, info);
}

} // namespace

void frame(psprecomp::Runtime &runtime, BlockAllocator allocate) {
    State &s = state();
    if (!installed_overlay_dispatches(kLobbySlot, kLobbyHash, kGetter) ||
        !installed_overlay_dispatches(kLobbySlot, kLobbyHash, kLookup))
        return;
    if (!s.loaded) {
        s.loaded = true;
        static const bool trace = [] {
            const char *text = std::getenv("MHFU_TRACE_EVENT_QUESTS");
            return text != nullptr && *text != '\0' && std::string(text) != "0";
        }();
        s.trace = trace;
        const std::filesystem::path directory = install::user_data_directory() / "event_quests";
        s.quests = load(directory);
        std::cout << "[event quests] " << s.quests.size() << " built-in quest(s) in " << path_to_utf8(directory)
                  << "\n";
    }
    if (s.quests.empty()) return;
    psprecomp::GuestMemory &memory = runtime.memory();
    if (!s.placed && allocate != nullptr) {
        s.placed = true;
        std::size_t largest = 0u;
        for (const Quest &quest : s.quests) largest = std::max(largest, quest.bytes.size());
        // A slot each, as the save has them, where there is room; otherwise
        // packed, with a whole slot's bytes after the last for code that
        // reads that far.
        const auto count = static_cast<std::uint32_t>(s.quests.size());
        std::uint32_t stride = kSlotSize;
        std::uint32_t base = allocate(count * kSlotSize);
        if (base == 0u) {
            stride = static_cast<std::uint32_t>((largest + 15u) & ~std::size_t{15u});
            base = allocate((count - 1u) * stride + kSlotSize);
        }
        if (base == 0u) {
            std::cerr << "[event quests] no guest memory for the built-in quests; left out\n";
            s.quests.clear();
            return;
        }
        s.bank = place(memory, base, stride, s.quests);
        std::cout << "[event quests] placed at " << std::hex << base << std::dec << ", " << stride << " bytes apart\n";
    }
    // lobby_task registers its functions again each time it is installed.
    if (runtime.function_at(kGetter) != &getter)
        hooks::register_chained(runtime, kGetter, &getter, "mhfu_event_quests_getter");
    if (runtime.function_at(kLookup) != &lookup)
        hooks::register_chained(runtime, kLookup, &lookup, "mhfu_event_quests_lookup");
    if (!s.reported) {
        s.reported = true;
        std::cout << "[event quests] Guild Hall event list extended" << std::endl;
    }
}

bool loaded() {
    return state().loaded;
}

} // namespace mhfu::game::event_quests
