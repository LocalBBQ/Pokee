#include "game/event_quests.hpp"

#include "platform/utf8_path.hpp"
#include "psprecomp/guest_memory.hpp"

#include <algorithm>
#include <charconv>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

namespace mhfu::game::event_quests {
namespace {

constexpr std::uint8_t kMagic[] = {'2', 'N', 'D', 'G'};
constexpr std::uint32_t kMinimumInfo = kInfoId + 2u;

std::uint32_t load32(std::span<const std::uint8_t> bytes, std::size_t offset) {
    std::uint32_t value = 0u;
    std::memcpy(&value, bytes.data() + offset, 4u);
    return value;
}

std::uint16_t load16(std::span<const std::uint8_t> bytes, std::size_t offset) {
    std::uint16_t value = 0u;
    std::memcpy(&value, bytes.data() + offset, 2u);
    return value;
}

// The info block of the quest whose file starts at `slot`, and its flags.
std::uint32_t info_of(const psprecomp::GuestMemory &memory, std::uint32_t slot) {
    return slot + memory.load32(slot);
}

} // namespace

std::optional<Quest> parse(std::span<const std::uint8_t> bytes) {
    if (bytes.size() < 8u || bytes.size() > kQuestBytes) return std::nullopt;
    if (!std::equal(std::begin(kMagic), std::end(kMagic), bytes.begin() + 4)) return std::nullopt;
    const std::uint32_t info = load32(bytes, 0u);
    if (info < 8u || info > bytes.size() - kMinimumInfo || (info & 3u) != 0u) return std::nullopt;
    Quest quest;
    quest.id = load16(bytes, info + kInfoId);
    quest.bytes.assign(bytes.begin(), bytes.end());
    return quest;
}

std::vector<Quest> load(const std::filesystem::path &directory) {
    std::vector<Quest> quests;
    std::error_code ec;
    for (const auto &entry : std::filesystem::directory_iterator(directory, ec)) {
        const std::string name = path_to_utf8(entry.path().filename());
        // m<id>.mib, as the game names a quest file.
        if (name.size() < 6u || name.front() != 'm' || !name.ends_with(".mib")) continue;
        std::uint32_t named{};
        const auto [end, parsed] = std::from_chars(name.data() + 1, name.data() + name.size() - 4u, named);
        if (parsed != std::errc{} || end != name.data() + name.size() - 4u) continue;
        std::ifstream in(entry.path(), std::ios::binary);
        const std::vector<std::uint8_t> bytes((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        std::optional<Quest> quest = parse(bytes);
        if (!quest || quest->id != named) {
            std::cerr << "[event quests] " << name << " is not a quest file of that id; left out\n";
            continue;
        }
        quests.push_back(std::move(*quest));
    }
    std::sort(quests.begin(), quests.end(), [](const Quest &a, const Quest &b) { return a.id < b.id; });
    quests.erase(std::unique(quests.begin(), quests.end(), [](const Quest &a, const Quest &b) { return a.id == b.id; }),
        quests.end());
    return quests;
}

Bank place(psprecomp::GuestMemory &memory, std::uint32_t base, std::uint32_t stride, std::span<const Quest> quests) {
    Bank bank;
    std::uint32_t slot = base;
    for (const Quest &quest : quests) {
        memory.copy_in(slot, quest.bytes);
        if (stride >= kSlotSize) {
            char name[16] = {};
            std::snprintf(name, sizeof name, "m%05u.mib", static_cast<unsigned>(quest.id));
            for (std::uint32_t i = 0; i < 16u; ++i) memory.store8(slot + kQuestBytes + i, static_cast<std::uint8_t>(name[i]));
        }
        bank.slots.push_back(slot);
        slot += stride;
    }
    return bank;
}

std::uint32_t save_slot_info(psprecomp::GuestMemory &memory, std::uint32_t lobby, std::uint32_t k) {
    const std::uint32_t save = memory.load32(kSaveObject);
    if (memory.load8(save + kTitles + k * kTitleSize) == 0u) return 0u;
    const std::uint32_t slot = memory.load32(save + kSaveBuffer) + kSlots + k * kSlotSize;
    if (memory.load32(slot) == 0u) return 0u;
    memory.store32(lobby + kChosenSlot, slot);
    const std::uint32_t info = info_of(memory, slot);
    return (memory.load32(info) & kHiddenFlag) != 0u ? 0u : info;
}

std::uint32_t bank_slot_info(psprecomp::GuestMemory &memory, std::uint32_t lobby, std::uint32_t slot) {
    memory.store32(lobby + kChosenSlot, slot);
    const std::uint32_t info = info_of(memory, slot);
    return (memory.load32(info) & kHiddenFlag) != 0u ? 0u : info;
}

std::uint32_t find(psprecomp::GuestMemory &memory, const Bank &bank, std::uint32_t lobby, std::uint16_t id) {
    for (std::uint32_t k = 0; k < kSlotCount; ++k) {
        const std::uint32_t info = save_slot_info(memory, lobby, k);
        if (info != 0u && memory.load16(info + kInfoId) == id) return info;
    }
    for (const std::uint32_t slot : bank.slots) {
        if (memory.load16(info_of(memory, slot) + kInfoId) != id) continue;
        return bank_slot_info(memory, lobby, slot);
    }
    return 0u;
}

std::uint32_t extend_list(psprecomp::GuestMemory &memory, const Bank &bank, std::uint32_t lobby, std::uint32_t write,
    std::uint32_t count, std::uint32_t last_slot_info) {
    const std::uint16_t last_id = last_slot_info != 0u ? memory.load16(last_slot_info + kInfoId) : 0u;
    const std::uint32_t room = kListRoom - (last_slot_info != 0u ? 1u : 0u);
    std::uint32_t added = 0u;
    for (const std::uint32_t slot : bank.slots) {
        if (count + added >= room) break;
        const std::uint32_t info = info_of(memory, slot);
        if ((memory.load32(info) & kHiddenFlag) != 0u) continue;
        const std::uint16_t id = memory.load16(info + kInfoId);
        if (id == last_id) continue;
        bool listed = false;
        for (std::uint32_t i = 0; i < count + added && !listed; ++i)
            listed = memory.load16(lobby + kList + 2u * i) == id;
        if (listed) continue;
        memory.store16(write + kList + 2u * added, id);
        ++added;
    }
    return added;
}

} // namespace mhfu::game::event_quests
