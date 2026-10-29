// Checks for the built-in event quests (game/event_quests.hpp) that need no
// game data: quest files, their import from a folder, and the getter, lookup
// and list extension on a guest memory laid out as the save's slots and the
// Guild Hall's list.
#include "game/event_quests.hpp"

#include "game/event_quest_import.hpp"

#include "psprecomp/guest_memory.hpp"

#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>
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

// A quest file with a title: seven language tables, all pointing at one.
std::vector<std::uint8_t> titled_quest(
    std::uint16_t id, const char *name, std::uint32_t flags = 0u, std::size_t size = 0x200u) {
    std::vector<std::uint8_t> bytes = quest_file(id, flags, size);
    const auto put32 = [&](std::size_t at, std::uint32_t value) { std::memcpy(bytes.data() + at, &value, 4u); };
    put32(0x4Cu + kInfoText, 0x100u);
    for (std::uint32_t i = 0; i < 7u; ++i) put32(0x100u + 4u * i, 0x120u);
    put32(0x120u, 0x140u);
    std::memcpy(bytes.data() + 0x140u, name, std::strlen(name) + 1u);
    return bytes;
}

void write_file(const std::filesystem::path &path, const std::vector<std::uint8_t> &bytes) {
    std::ofstream(path, std::ios::binary)
        .write(reinterpret_cast<const char *>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
}

std::vector<std::uint8_t> read_back(const std::filesystem::path &path) {
    std::ifstream in(path, std::ios::binary);
    return {std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
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

void test_title() {
    check(title(titled_quest(60023, "Its name is \"Mega Kut-Ku\"!?")) == "Its name is \"Mega Kut-Ku\"!?",
        "a quest's title is its first English string");
    check(parse(titled_quest(60023, "Title"))->flags == 0u &&
            parse(titled_quest(60023, "Title", kHiddenFlag))->flags == kHiddenFlag,
        "parsing keeps the info block's flags");
    auto outside = titled_quest(60023, "Title");
    const std::uint32_t far = 0x4000u;
    std::memcpy(outside.data() + 0x120u, &far, 4u);
    check(title(outside).empty(), "a title outside the file is none");
    auto unended = titled_quest(60023, "Title");
    std::memset(unended.data() + 0x140u, 'x', unended.size() - 0x140u);
    check(title(unended).empty(), "nor one that does not end inside it");
    check(title(std::vector<std::uint8_t>(2u, 0u)).empty(), "a file too short for an info block has none");
}

void test_import() {
    namespace fs = std::filesystem;
    const fs::path root = fs::temp_directory_path() / "mhfu_event_quest_import_tests";
    fs::remove_all(root);
    const fs::path chosen = root / "download";
    const fs::path inner = chosen / "tables";
    const fs::path quests = root / "data" / "event_quests";
    fs::create_directories(inner);

    check(check_import(chosen, quests).problem == "No quest files here, or in the folders directly inside it.",
        "an empty folder has nothing to import");

    write_file(chosen / "Mega Kut-Ku.bin", titled_quest(60023, "Mega Kut-Ku"));
    write_file(chosen / "jp.mib", quest_file(60001));
    write_file(chosen / "challenge.mib", quest_file(61001, kHiddenFlag));
    write_file(chosen / "hidden.mib", quest_file(60040, kHiddenFlag));
    write_file(chosen / "other.mib", quest_file(1234));
    write_file(chosen / "shown challenge.mib", quest_file(61002));
    write_file(chosen / "encrypted.mib", std::vector<std::uint8_t>(0x6810u, 0xA5u));
    write_file(chosen / "notes.txt", std::vector<std::uint8_t>{'h', 'i'});
    // FUComplete's table: a name, then quests 0x22B0 bytes apart from 0x30 after it.
    std::vector<std::uint8_t> table(0x40u, 0u);
    std::memcpy(table.data() + 0x10u, "EventQstTbl", 12u);
    for (const std::uint16_t id : {60024, 60025, 60023, 60018}) {
        const auto quest = titled_quest(id, "Table quest", 0u, 0x22B0u);
        table.insert(table.end(), quest.begin(), quest.end());
    }
    table.resize(table.size() + 0x22B0u, 0u); // the table's end: no quest
    write_file(inner / "6594", table);
    fs::create_directories(quests);
    write_file(quests / "m60024.mib", titled_quest(60024, "Table quest", 0u, 0x22B0u));
    write_file(quests / "m60025.mib", quest_file(60025));

    const ImportCheck found = check_import(chosen, quests);
    check(found.quests.size() == 3u && found.quests[0].quest.id == 60023u && found.quests[1].quest.id == 60025u &&
            found.quests[2].quest.id == 61001u,
        "loose quest files of any name and a table's quests are found, by id");
    check(!found.quests[0].challenge && found.quests[2].challenge,
        "a challenge is one of 61000 and up with the flag that keeps it out of the event list");
    check(found.quests[0].title == "Mega Kut-Ku" && found.quests[0].from == chosen / "Mega Kut-Ku.bin",
        "the folder's own files come before its folders' for the same id");
    check(!found.quests[0].replaces && found.quests[1].replaces, "a different quest of an id is replaced");
    check(found.present == 1u, "the same quest imported already is left out");
    check(found.japan_only == 2u, "Japan-only quests are left out, loose or in a table");
    check(found.other == 3u && found.unreadable == 1u,
        "other quests (an event id with the flag, a challenge id without) and undecoded .mib files are counted");
    check(found.problem.empty(), "something to import is no problem");

    const ImportResult result = import_quests(found, quests);
    check(result.error.empty() && result.imported == std::vector<std::uint16_t>{60023u, 60025u, 61001u},
        "the import writes each quest");
    check(read_back(quests / "m60023.mib") == titled_quest(60023, "Mega Kut-Ku") &&
            read_back(quests / "m60025.mib") == titled_quest(60025, "Table quest", 0u, 0x22B0u),
        "as m<id>.mib, replacing the old one");
    check(load(quests).size() == 4u && !fs::exists(quests / "m60023.mib.tmp"), "where load() reads them");
    check(check_import(chosen, quests).problem == "The event quests here are imported already.",
        "importing again finds nothing new");

    fs::remove_all(chosen);
    fs::create_directories(chosen);
    write_file(chosen / "jp.mib", quest_file(60001));
    check(check_import(chosen, quests).problem == "No event quests here that can be imported.",
        "only quests that are left out: a problem");
    fs::remove_all(root);
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

void test_challenges() {
    psprecomp::GuestMemory memory;
    lay_out(memory);
    fill_slot(memory, 0u, quest_file(60010));
    fill_slot(memory, 1u, quest_file(61003, kHiddenFlag));
    check(save_challenge_info(memory, 1u) == kBufferAt + kSlots + kSlotSize + 0x4Cu,
        "the Training School's getter returns a challenge's info block");
    check(save_challenge_info(memory, 0u) == 0u && save_challenge_info(memory, 2u) == 0u,
        "and none for an event quest or an empty slot");

    std::vector<Quest> quests{{60023u, quest_file(60023)}};
    for (std::uint16_t id = 61001u; id <= 61014u; ++id) quests.push_back({id, quest_file(id, kHiddenFlag)});
    const Bank bank = place(memory, kBankAt, 0x200u, quests);
    // The loop has listed the save's 61003 and is asking for slot 5, which holds 61005.
    const std::uint32_t arcade = kLobbyAt;
    memory.store16(arcade + kChallengeIds, 61003u);
    memory.store32(arcade + kChallengeInfos, kBufferAt + kSlots + kSlotSize + 0x4Cu);
    memory.store16(arcade + kChallengeCount, 1u);
    const std::uint32_t last = kBufferAt + kSlots + 5u * kSlotSize + 0x4Cu;
    memory.store16(last + kInfoId, 61005u);
    const std::uint32_t added = extend_challenges(memory, bank, arcade, last);
    check(added == 12u && memory.load16(arcade + kChallengeCount) == 13u,
        "the built-in challenges fill the list but for the save's and slot 5's, and room for slot 5's own");
    check(memory.load16(arcade + kChallengeIds + 2u) == 61001u &&
            memory.load16(arcade + kChallengeIds + 4u) == 61002u &&
            memory.load16(arcade + kChallengeIds + 6u) == 61004u,
        "in id order, without the event quest or a challenge listed already");
    check(memory.load32(arcade + kChallengeSlots + 4u) == bank.slots[1] &&
            memory.load32(arcade + kChallengeInfos + 4u) == bank.slots[1] + 0x4Cu,
        "each with its slot and info block, where the game takes the chosen one from");
    check(extend_challenges(memory, bank, arcade, last) == 0u, "a full list gains nothing");
    memory.store16(arcade + kChallengeCount, 0u);
    check(extend_challenges(memory, bank, arcade, 0u) == kChallengeRoom &&
            memory.load16(arcade + kChallengeCount) == kChallengeRoom,
        "without slot 5's, all fourteen fit");
    check(!training_school_matches(memory), "without arcade_task's code nothing is replaced");
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
    test_title();
    test_import();
    test_getter_and_lookup();
    test_list();
    test_challenges();
    std::printf("%d failure(s)\n", failures);
    return failures == 0 ? 0 : 1;
}
