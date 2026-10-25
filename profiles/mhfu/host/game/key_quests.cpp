#include "game/key_quests.hpp"

#include "settings/settings.hpp"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <map>
#include <set>
#include <string>

namespace mhfu::game::key_quests {
namespace {

struct Loaded {
    std::uint32_t base{};
    std::vector<Patch> patches;
};

struct State {
    bool on{};
    bool checked{};
    bool trace{};
    bool refused{};
    std::uint32_t block{};
    std::map<std::uint32_t, int> pending;          // packs read, and the frames left to find them
    std::map<std::uint32_t, Loaded> loaded;        // packs tagged
    std::map<std::uint32_t, std::vector<std::uint8_t>> bytes; // packs as the game gets them
};

State &state() {
    static State s;
    return s;
}

std::uint32_t word(std::span<const std::uint8_t> bytes, std::uint32_t at) {
    return static_cast<std::uint32_t>(bytes[at]) | static_cast<std::uint32_t>(bytes[at + 1u]) << 8u |
        static_cast<std::uint32_t>(bytes[at + 2u]) << 16u | static_cast<std::uint32_t>(bytes[at + 3u]) << 24u;
}

bool fits(std::span<const std::uint8_t> bytes, std::uint32_t at, std::uint32_t length) {
    return at <= bytes.size() && length <= bytes.size() - at;
}

std::string hex(std::uint32_t value) {
    char text[16];
    std::snprintf(text, sizeof text, "%08X", value);
    return text;
}

// Looks for each pack read since the last frame, in RAM where it lies. A copy
// was 32 MiB of fresh memory each time, about 8,200 page faults: 50 ms of the
// flip on the first frames of a quest. The copy remains for a memory that is
// not one mapped range.
void look(psprecomp::GuestMemory &memory, EntryBytes bytes) {
    State &s = state();
    std::vector<std::uint8_t> copy;
    std::span<const std::uint8_t> ram;
    if (const std::uint8_t *raw = memory.raw_pointer(psprecomp::GuestMemory::kPhysicalBase, memory.size())) {
        ram = {raw, memory.size()};
    } else {
        copy.resize(memory.size());
        memory.copy_out(psprecomp::GuestMemory::kPhysicalBase, copy);
        ram = copy;
    }
    for (auto it = s.pending.begin(); it != s.pending.end();) {
        const std::uint32_t entry = it->first;
        auto cached = s.bytes.find(entry);
        if (cached == s.bytes.end()) cached = s.bytes.emplace(entry, bytes(entry)).first;
        const std::vector<std::uint8_t> &pack = cached->second;
        const std::optional<std::vector<Name>> names = key_names(pack);
        if (!names) {
            if (s.trace) std::cout << "[key quests] entry " << entry << " is not a quest pack as traced\n";
            it = s.pending.erase(it);
            continue;
        }
        const std::optional<std::uint32_t> base = find_pack(ram, psprecomp::GuestMemory::kPhysicalBase, pack);
        if (!base) {
            if (--it->second <= 0) {
                if (s.trace) std::cout << "[key quests] entry " << entry << " not found in memory\n";
                it = s.pending.erase(it);
            } else {
                ++it;
            }
            continue;
        }
        const std::uint32_t slot = s.block + (entry - kFirstPack) * kSlot;
        Loaded loaded{*base, tag(memory, *base, *names, slot, kSlot)};
        if (s.trace)
            std::cout << "[key quests] entry " << entry << " at 0x" << hex(*base) << ": " << loaded.patches.size()
                      << " of " << names->size() << " names tagged\n";
        s.loaded[entry] = std::move(loaded);
        it = s.pending.erase(it);
    }
}

} // namespace

std::optional<std::vector<Name>> key_names(std::span<const std::uint8_t> pack) {
    if (pack.size() < 4u || pack.size() > 0xFFFFFFFFu) return std::nullopt;
    const std::uint32_t list = word(pack, 0u);
    if (list < 4u || list % 4u != 0u || list / 4u > kMaxQuests || !fits(pack, 0u, list)) return std::nullopt;
    std::vector<Name> names;
    for (std::uint32_t q = 0; q < list / 4u; ++q) {
        const std::uint32_t record = word(pack, q * 4u);
        if (!fits(pack, record, kLanguagesWord + 4u)) return std::nullopt;
        const std::uint32_t languages = word(pack, record + kLanguagesWord);
        if (!fits(pack, languages, kLanguages * 4u)) return std::nullopt;
        if ((pack[record + kFlags] & kKeyBit) == 0u) continue;
        std::set<std::uint32_t> seen;
        for (std::uint32_t l = 0; l < kLanguages; ++l) {
            const std::uint32_t strings = word(pack, languages + l * 4u);
            if (!fits(pack, strings, 4u)) return std::nullopt;
            if (!seen.insert(strings).second) continue;
            const std::uint32_t name = word(pack, strings);
            if (name >= pack.size()) return std::nullopt;
            const auto end = std::find(pack.begin() + name, pack.end(), std::uint8_t{0});
            if (end == pack.end()) return std::nullopt;
            names.push_back(Name{strings, name, std::string(pack.begin() + name, end)});
        }
    }
    return names;
}

std::optional<std::uint32_t> find_pack(std::span<const std::uint8_t> ram, std::uint32_t ram_base,
    std::span<const std::uint8_t> pack, std::uint32_t from) {
    if (pack.empty() || pack.size() > ram.size()) return std::nullopt;
    const std::size_t head = std::min<std::size_t>(pack.size(), 256u);
    const std::boyer_moore_horspool_searcher searcher(pack.begin(), pack.begin() + static_cast<std::ptrdiff_t>(head));
    auto at = ram.begin() + (from > ram_base ? std::min<std::size_t>(from - ram_base, ram.size()) : 0u);
    while (true) {
        at = std::search(at, ram.end(), searcher);
        if (at == ram.end()) return std::nullopt;
        // Packs are word aligned, and the rest must match too.
        const auto offset = static_cast<std::size_t>(at - ram.begin());
        if (offset % 4u == 0u && ram.size() - offset >= pack.size() &&
            std::equal(pack.begin(), pack.end(), at))
            return ram_base + static_cast<std::uint32_t>(offset);
        ++at;
    }
}

std::vector<Patch> tag(psprecomp::GuestMemory &memory, std::uint32_t base, const std::vector<Name> &names,
    std::uint32_t slot, std::uint32_t capacity) {
    std::vector<Patch> patches;
    const std::string tag_text = kTag;
    std::uint32_t used = 0;
    for (const Name &name : names) {
        const std::uint32_t address = base + name.word;
        if (!memory.contains(address, 4u) || memory.load32(address) != name.name) continue;
        const std::string text = tag_text + name.text;
        const auto length = static_cast<std::uint32_t>(text.size() + 1u);
        if (length > capacity - used || !memory.contains(slot + used, length)) continue;
        const std::uint32_t at = slot + used;
        for (std::uint32_t i = 0; i < text.size(); ++i) memory.store8(at + i, static_cast<std::uint8_t>(text[i]));
        memory.store8(at + length - 1u, 0u);
        used += length;
        // The game adds the offset to the pack's start, so one outside the
        // pack is as good as one inside.
        const std::uint32_t replacement = at - base;
        memory.store32(address, replacement);
        patches.push_back(Patch{address, name.name, replacement});
    }
    return patches;
}

std::size_t untag(psprecomp::GuestMemory &memory, const std::vector<Patch> &patches) {
    std::size_t restored = 0;
    for (const Patch &patch : patches) {
        if (!memory.contains(patch.address, 4u) || memory.load32(patch.address) != patch.replacement) continue;
        memory.store32(patch.address, patch.original);
        ++restored;
    }
    return restored;
}

void file_loaded(std::uint32_t entry) {
    State &s = state();
    if (!s.on || !is_pack(entry)) return;
    s.pending[entry] = kAttempts;
}

void frame(psprecomp::GuestMemory &memory, Allocate allocate, EntryBytes bytes) {
    State &s = state();
    const bool was_on = s.on;
    s.on = settings::current().key_quests;
    if (!s.on) {
        if (!was_on) return;
        std::size_t restored = 0;
        for (const auto &[entry, loaded] : s.loaded) restored += untag(memory, loaded.patches);
        if (s.trace) std::cout << "[key quests] off: " << restored << " names put back\n";
        s.loaded.clear();
        s.pending.clear();
        return;
    }
    if (!s.checked) {
        s.checked = true;
        const char *text = std::getenv("MHFU_TRACE_KEY_QUESTS");
        s.trace = text != nullptr && *text != '\0' && std::string(text) != "0";
    }
    if (s.refused) return;
    if (s.block == 0u) {
        s.block = allocate(kBlockSize);
        if (s.block == 0u) {
            std::cerr << "[key quests] no guest memory for the names; key quests stay unmarked\n";
            s.refused = true; // reported once
            return;
        }
    }
    // Turned on: the packs the game already holds.
    if (!was_on)
        for (std::uint32_t entry = kFirstPack; entry <= kLastPack; ++entry) s.pending.emplace(entry, 1);
    if (!s.pending.empty()) look(memory, bytes);
}

void reset() {
    state() = State{};
}

} // namespace mhfu::game::key_quests
