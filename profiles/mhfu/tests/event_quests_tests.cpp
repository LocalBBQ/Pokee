// Checks for the built-in event quests (game/event_quests.hpp) that need no
// game data: quest files, and the getter, lookup and list extension on a
// guest memory laid out as the save's slots and the Guild Hall's list.
#include "game/event_quests.hpp"

#include "psprecomp/guest_memory.hpp"

#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <vector>

using namespace mhfu::game::event_quests;

namespace {

int failures = 0;

void check(bool condition, const char *what) {
    std::printf("%s %s\n", condition ? "ok  " : "FAIL", what);
    std::fflush(stdout);
    if (!condition) ++failures;
}

// A quest file: the info block at 0x4C, as the game's have it.
std::vector<std::uint8_t> quest_file(std::uint16_t id, std::uint32_t flags = 0u, std::size_t size = 0x200u) {
    std::vector<std::uint8_t> bytes(size, 0u);
    const std::uint32_t info = 0x4Cu;
    std::memcpy(bytes.data(), &info, 4u);
    std::memcpy(bytes.data() + 4u, "2NDG", 4u);
    std::memcpy(bytes.data() + info, &flags, 4u);
    std::memcpy(bytes.data() + info + kInfoId, &id, 2u);
    return bytes;
}

constexpr std::uint32_t kSaveObjectAt = 0x09A06BD0u;
constexpr std::uint32_t kBufferAt = 0x0976C740u;
constexpr std::uint32_t kLobbyAt = 0x09B00000u;
constexpr std::uint32_t kBankAt = 0x09C00000u;

// The save's slot k holding a quest, with a title so it counts as in use.
void fill_slot(psprecomp::GuestMemory &memory, std::uint32_t k, const std::vector<std::uint8_t> &file) {
    memory.copy_in(kBufferAt + kSlots + k * kSlotSize, file);
    memory.store8(kSaveObjectAt + kTitles + k * kTitleSize, 'Q');
}

void lay_out(psprecomp::GuestMemory &memory) {
    memory.store32(kSaveObject, kSaveObjectAt);
    memory.store32(kSaveObjectAt + kSaveBuffer, kBufferAt);
}

void test_parse() {
    check(parse(quest_file(60023)).has_value() && parse(quest_file(60023))->id == 60023u, "a quest file parses");
    auto bad = quest_file(60023);
    bad[4] = 'X';
    check(!parse(bad).has_value(), "without 2NDG it is not a quest");
    check(!parse(quest_file(60023, 0u, kQuestBytes + 4u)).has_value(), "a file larger than a slot's is refused");
    auto pointing_out = quest_file(60023);
    const std::uint32_t far = 0x1F0u;
    std::memcpy(pointing_out.data(), &far, 4u);
    check(!parse(pointing_out).has_value(), "an info block past the file is refused");
}

void test_load() {
    const auto directory = std::filesystem::temp_directory_path() / "mhfu_event_quests_tests";
    std::filesystem::remove_all(directory);
    std::filesystem::create_directories(directory);
    const auto write = [&](const char *name, const std::vector<std::uint8_t> &bytes) {
        std::ofstream(directory / name, std::ios::binary)
            .write(reinterpret_cast<const char *>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    };
    write("m60024.mib", quest_file(60024));
    write("m60023.mib", quest_file(60023));
    write("m60025.mib", quest_file(60099));
    write("notes.txt", quest_file(60026));
    const std::vector<Quest> quests = load(directory);
    check(quests.size() == 2u && quests[0].id == 60023u && quests[1].id == 60024u,
        "quest files load by id; a file named for another id and other files are left out");
    std::filesystem::remove_all(directory);
}

void test_getter_and_lookup() {
    psprecomp::GuestMemory memory;
    lay_out(memory);
    fill_slot(memory, 0u, quest_file(60010));
    fill_slot(memory, 2u, quest_file(60031, kHiddenFlag));
    const std::uint32_t slot0 = kBufferAt + kSlots;
    check(save_slot_info(memory, kLobbyAt, 0u) == slot0 + 0x4Cu, "the getter returns a slot's info block");
    check(memory.load32(kLobbyAt + kChosenSlot) == slot0, "and keeps the slot where the game takes it from");
    check(save_slot_info(memory, kLobbyAt, 1u) == 0u, "an empty slot has none");
    check(save_slot_info(memory, kLobbyAt, 2u) == 0u, "nor a hidden quest");

    const Quest built_in{60023u, quest_file(60023)};
    const Quest duplicate{60010u, quest_file(60010)};
    const std::vector<Quest> quests{built_in, duplicate};
    const Bank bank = place(memory, kBankAt, kSlotSize, quests);
    check(bank.slots.size() == 2u && bank.slots[1] == kBankAt + kSlotSize, "the bank takes a slot each");
    char name[11] = {};
    for (std::uint32_t i = 0; i < 10u; ++i) name[i] = static_cast<char>(memory.load8(kBankAt + kQuestBytes + i));
    check(std::strcmp(name, "m60023.mib") == 0, "with the name after the file, as a save slot has it");

    check(find(memory, bank, kLobbyAt, 60023u) == kBankAt + 0x4Cu, "the lookup finds a built-in quest");
    check(memory.load32(kLobbyAt + kChosenSlot) == kBankAt, "and keeps its slot");
    check(find(memory, bank, kLobbyAt, 60010u) == slot0 + 0x4Cu, "the save's quest of an id comes first");
    check(find(memory, bank, kLobbyAt, 60050u) == 0u, "an unknown id is not found");
    check(find(memory, Bank{}, kLobbyAt, 60023u) == 0u, "without a bank, only the save's");
}

void test_list() {
    psprecomp::GuestMemory memory;
    lay_out(memory);
    const std::vector<Quest> quests{{60010u, quest_file(60010)}, {60023u, quest_file(60023)},
        {60024u, quest_file(60024, kHiddenFlag)}, {60025u, quest_file(60025)}};
    const Bank bank = place(memory, kBankAt, 0x200u, quests);
    // The loop has listed the save's 60010 and is asking for slot 5, which holds 60025.
    memory.store16(kLobbyAt + kList, 60010u);
    const std::uint32_t last = kBufferAt; // any block with id 60025 at +24
    memory.store16(last + kInfoId, 60025u);
    const std::uint32_t added = extend_list(memory, bank, kLobbyAt, kLobbyAt + 2u, 1u, last);
    check(added == 1u && memory.load16(kLobbyAt + kList + 2u) == 60023u,
        "the list gains the built-in quests the save does not hold, leaving out hidden ones and slot 5's");

    psprecomp::GuestMemory full;
    lay_out(full);
    const Bank many = place(full, kBankAt, 0x200u, quests);
    check(extend_list(full, many, kLobbyAt, kLobbyAt + 2u * (kListRoom - 1u), kListRoom - 1u, 0u) == 1u,
        "the list stops at its room");
    check(extend_list(full, many, kLobbyAt, kLobbyAt + 2u * (kListRoom - 1u), kListRoom - 1u, last) == 0u,
        "keeping one entry for slot 5's quest");
}

} // namespace

int main() {
    test_parse();
    test_load();
    test_getter_and_lookup();
    test_list();
    std::printf("%d failure(s)\n", failures);
    return failures == 0 ? 0 : 1;
}
