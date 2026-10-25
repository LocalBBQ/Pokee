// Key quests: a quest pack's key names, a pack found in memory, tagged and put
// back, and the switch around it. The packs are built here in the traced
// layout; no game data.
#include "game/key_quests.hpp"

#include "settings/settings.hpp"

#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <map>
#include <string>
#include <vector>

namespace {
using namespace mhfu::game::key_quests;

int failures{};

void check(bool condition, const char *message) {
    if (!condition) {
        ++failures;
        std::cerr << "FAIL: " << message << '\n';
    }
}

void set_environment(const char *name, const std::string &value) {
#if defined(_WIN32)
    _putenv_s(name, value.c_str());
#else
    // _putenv_s removes a variable set to "", so do the same here.
    if (value.empty())
        unsetenv(name);
    else
        setenv(name, value.c_str(), 1);
#endif
}

constexpr std::uint32_t kMemory = 32u * 1024u * 1024u;
constexpr std::uint32_t kPackAt = 0x09700000u;
constexpr std::uint32_t kBlockAt = 0x09F00000u;

struct Quest {
    std::uint8_t flags{};
    std::string name; // English; the other four languages get a prefix
};

void put32(std::vector<std::uint8_t> &bytes, std::uint32_t at, std::uint32_t value) {
    if (bytes.size() < at + 4u) bytes.resize(at + 4u);
    for (std::uint32_t i = 0; i < 4u; ++i) bytes[at + i] = static_cast<std::uint8_t>(value >> (8u * i));
}

std::uint32_t append(std::vector<std::uint8_t> &bytes, const std::string &text) {
    const auto at = static_cast<std::uint32_t>(bytes.size());
    bytes.insert(bytes.end(), text.begin(), text.end());
    bytes.push_back(0u);
    while (bytes.size() % 4u != 0u) bytes.push_back(0u);
    return at;
}

// A pack in the traced layout: the list of records, then per quest its
// record, five string tables (English shared by three of the seven language
// words, as on the disc) and their strings.
std::vector<std::uint8_t> build_pack(const std::vector<Quest> &quests) {
    std::vector<std::uint8_t> bytes(quests.size() * 4u, 0u);
    const char *prefixes[] = {"", "FR ", "DE ", "ES ", "IT "};
    for (std::size_t q = 0; q < quests.size(); ++q) {
        const auto record = static_cast<std::uint32_t>(bytes.size());
        put32(bytes, static_cast<std::uint32_t>(q * 4u), record);
        bytes.resize(record + 0x34u, 0u);
        bytes[record + kFlags] = quests[q].flags;
        put32(bytes, record + 0x18u, 10301u + static_cast<std::uint32_t>(q));
        std::uint32_t tables[5]{};
        for (int l = 0; l < 5; ++l) {
            const std::uint32_t name = append(bytes, prefixes[l] + quests[q].name);
            const std::uint32_t goal = append(bytes, "Hunt it");
            tables[l] = static_cast<std::uint32_t>(bytes.size());
            put32(bytes, tables[l], name);
            put32(bytes, tables[l] + 4u, goal);
        }
        const auto languages = static_cast<std::uint32_t>(bytes.size());
        const std::uint32_t slots[kLanguages] = {tables[0], tables[0], tables[1], tables[2], tables[3], tables[4],
            tables[0]};
        for (std::uint32_t l = 0; l < kLanguages; ++l) put32(bytes, languages + l * 4u, slots[l]);
        put32(bytes, record + kLanguagesWord, languages);
    }
    return bytes;
}

const std::vector<Quest> kQuests = {
    {0x00, "Gathering - Volcanic Zone"},
    {0x02, "The Ruler of the Snow"},
    {0x04, "Battle of the Blos"},
    {0x04, "Commander in the Flames"},
};

// What the game reads as a quest's name in one language: the pack's start
// plus the offsets.
std::string name_in_memory(const psprecomp::GuestMemory &memory, std::uint32_t base, std::uint32_t quest,
    std::uint32_t language) {
    const std::uint32_t record = base + memory.load32(base + quest * 4u);
    const std::uint32_t languages = base + memory.load32(record + kLanguagesWord);
    const std::uint32_t strings = base + memory.load32(languages + language * 4u);
    const std::uint32_t name = base + memory.load32(strings);
    std::string text;
    for (std::uint32_t at = name; memory.contains(at) && memory.load8(at) != 0u && text.size() < 80u; ++at)
        text += static_cast<char>(memory.load8(at));
    return text;
}

void place(psprecomp::GuestMemory &memory, std::uint32_t at, const std::vector<std::uint8_t> &pack) {
    memory.copy_in(at, pack);
}

void test_key_names() {
    const std::vector<std::uint8_t> pack = build_pack(kQuests);
    const auto names = key_names(pack);
    check(names.has_value(), "a built pack reads");
    if (!names) return;
    check(names->size() == 10u, "two key quests, five distinct language tables each");
    check((*names)[0].text == "Battle of the Blos" && (*names)[4].text == "IT Battle of the Blos" &&
            (*names)[5].text == "Commander in the Flames",
        "the names in order, every language");
    for (const Name &name : *names) {
        check(name.name < pack.size() && name.word + 4u <= pack.size(), "offsets inside the pack");
    }
    const auto none = key_names(build_pack({{0x02, "The Ruler of the Snow"}, {0x00, "Gathering"}}));
    check(none && none->empty(), "the Urgent Quest's bit is not a key quest's");
}

void test_key_names_refuses() {
    const std::vector<std::uint8_t> pack = build_pack(kQuests);
    check(!key_names({}), "nothing is not a pack");
    std::vector<std::uint8_t> bad = pack;
    put32(bad, 0u, 0u);
    check(!key_names(bad), "a list of no quests");
    bad = pack;
    put32(bad, 0u, 6u);
    check(!key_names(bad), "a list that is not whole words");
    bad = pack;
    put32(bad, 4u, static_cast<std::uint32_t>(pack.size()));
    check(!key_names(bad), "a record past the end");
    bad = pack;
    const std::uint32_t first = static_cast<std::uint32_t>(kQuests.size() * 4u); // the first record
    put32(bad, first + kLanguagesWord, static_cast<std::uint32_t>(pack.size() - 8u));
    check(!key_names(bad), "language words past the end");
    bad = std::vector<std::uint8_t>(pack.begin(), pack.end() - 4);
    check(!key_names(bad), "a cut pack");
    std::vector<std::uint8_t> many(4u * (kMaxQuests + 1u) + 4u, 0u);
    put32(many, 0u, 4u * (kMaxQuests + 1u));
    check(!key_names(many), "more quests than any list has");
    // A key quest's name with no end.
    bad = build_pack({{0x04, "Battle of the Blos"}});
    const auto names = key_names(bad);
    if (names && !names->empty()) {
        bad.push_back('x');
        bad.push_back('x');
        put32(bad, (*names)[0].word, static_cast<std::uint32_t>(bad.size() - 2u));
        check(!key_names(bad), "a name that does not end");
    }
}

void test_find_pack() {
    const std::vector<std::uint8_t> pack = build_pack(kQuests);
    std::vector<std::uint8_t> ram(0x40000u, 0xEEu);
    const std::uint32_t base = 0x08800000u;
    check(!find_pack(ram, base, pack), "no pack in memory");
    // Same start, different end: a pack the game has changed, or another.
    std::copy(pack.begin(), pack.end(), ram.begin() + 0x1000);
    ram[0x1000 + pack.size() - 1u] ^= 0xFFu;
    // Unaligned: not where the game puts one.
    std::copy(pack.begin(), pack.end(), ram.begin() + 0x10002);
    check(!find_pack(ram, base, pack), "an unaligned or changed copy is not the pack");
    std::copy(pack.begin(), pack.end(), ram.begin() + 0x20000);
    check(find_pack(ram, base, pack) == std::optional<std::uint32_t>{base + 0x20000u}, "the exact copy is found");
    check(!find_pack(ram, base, pack, base + 0x20004u), "and nothing after it");
    check(!find_pack(std::span<const std::uint8_t>(ram).first(16), base, pack), "a pack larger than memory");
    check(!find_pack(ram, base, {}), "nothing to look for");
}

void test_tag_untag() {
    psprecomp::GuestMemory memory(kMemory);
    const std::vector<std::uint8_t> pack = build_pack(kQuests);
    place(memory, kPackAt, pack);
    const auto names = key_names(pack);
    if (!names) return;
    const std::vector<Patch> patches = tag(memory, kPackAt, *names, kBlockAt, kSlot);
    check(patches.size() == names->size(), "every name tagged");
    check(name_in_memory(memory, kPackAt, 2u, 0u) == "[Key] Battle of the Blos", "English reads [Key]");
    check(name_in_memory(memory, kPackAt, 2u, 6u) == "[Key] Battle of the Blos", "in every English word");
    check(name_in_memory(memory, kPackAt, 3u, 3u) == "[Key] DE Commander in the Flames", "and the other languages");
    check(name_in_memory(memory, kPackAt, 1u, 0u) == "The Ruler of the Snow", "the Urgent Quest is left");
    check(name_in_memory(memory, kPackAt, 0u, 0u) == "Gathering - Volcanic Zone", "and so are the others");

    // The game wrote over one word since: it stays the game's.
    memory.store32(patches[0].address, 0x1234u);
    check(untag(memory, patches) == patches.size() - 1u, "every word that still holds ours is put back");
    check(memory.load32(patches[0].address) == 0x1234u, "the game's own write is left");
    check(name_in_memory(memory, kPackAt, 3u, 0u) == "Commander in the Flames", "the names are the game's again");

    // A slot too small takes what fits; a word that is not the name's offset is left.
    place(memory, kPackAt, pack);
    const std::uint32_t small = static_cast<std::uint32_t>(std::string(kTag).size() + (*names)[0].text.size() + 1u);
    check(tag(memory, kPackAt, *names, kBlockAt, small).size() == 1u, "only the names that fit");
    place(memory, kPackAt, pack);
    memory.store32(kPackAt + (*names)[0].word, 0u);
    check(tag(memory, kPackAt, *names, kBlockAt, kSlot).size() == names->size() - 1u,
        "a word the game changed is not tagged");
}

std::map<std::uint32_t, std::vector<std::uint8_t>> &disc() {
    static std::map<std::uint32_t, std::vector<std::uint8_t>> packs;
    return packs;
}

std::vector<std::uint8_t> disc_bytes(std::uint32_t entry) {
    const auto found = disc().find(entry);
    return found != disc().end() ? found->second : std::vector<std::uint8_t>{};
}

int allocations{};
std::uint32_t allocate_block(std::uint32_t size) {
    ++allocations;
    return size == kBlockSize ? kBlockAt : 0u;
}

std::uint32_t no_block(std::uint32_t) {
    ++allocations;
    return 0u;
}

void test_switch() {
    auto &settings = mhfu::settings::current();
    reset();
    disc().clear();
    const std::vector<std::uint8_t> pack = build_pack(kQuests);
    disc()[5345u] = pack;
    disc()[5346u] = std::vector<std::uint8_t>(64u, 0u); // not a pack as traced
    psprecomp::GuestMemory memory(kMemory);
    place(memory, kPackAt, pack);

    check(!settings.key_quests, "off by default");
    file_loaded(5345u);
    frame(memory, &allocate_block, &disc_bytes);
    check(allocations == 0 && name_in_memory(memory, kPackAt, 2u, 0u) == "Battle of the Blos",
        "off, nothing runs and nothing is tagged");

    settings.key_quests = true;
    frame(memory, &allocate_block, &disc_bytes);
    check(allocations == 1, "turned on, one block for the names");
    check(name_in_memory(memory, kPackAt, 2u, 0u) == "[Key] Battle of the Blos",
        "a pack already in memory is tagged when the switch goes on");
    frame(memory, &allocate_block, &disc_bytes);
    check(allocations == 1, "the block is kept");

    // The game loads the pack again elsewhere.
    const std::uint32_t again = kPackAt + 0x40000u;
    place(memory, again, pack);
    file_loaded(5345u);
    file_loaded(1234u); // not a pack
    frame(memory, &allocate_block, &disc_bytes);
    check(name_in_memory(memory, again, 3u, 0u) == "[Key] Commander in the Flames", "a pack read again is tagged");

    // Read, but not in memory: given up after a few frames.
    file_loaded(5347u);
    disc()[5347u] = build_pack({{0x04, "Not loaded"}});
    for (int i = 0; i < kAttempts + 1; ++i) frame(memory, &allocate_block, &disc_bytes);

    settings.key_quests = false;
    frame(memory, &allocate_block, &disc_bytes);
    check(name_in_memory(memory, again, 3u, 0u) == "Commander in the Flames", "turned off, the names are put back");
    file_loaded(5345u);
    frame(memory, &allocate_block, &disc_bytes);
    check(name_in_memory(memory, again, 3u, 0u) == "Commander in the Flames", "and stay the game's while off");

    settings.key_quests = true;
    frame(memory, &allocate_block, &disc_bytes);
    check(name_in_memory(memory, again, 3u, 0u) == "[Key] Commander in the Flames", "and on again, tagged again");
    settings.key_quests = false;
    frame(memory, &allocate_block, &disc_bytes);
}

void test_no_block() {
    auto &settings = mhfu::settings::current();
    reset();
    disc().clear();
    disc()[5345u] = build_pack(kQuests);
    psprecomp::GuestMemory memory(kMemory);
    place(memory, kPackAt, disc()[5345u]);
    allocations = 0;
    settings.key_quests = true;
    frame(memory, &no_block, &disc_bytes);
    frame(memory, &no_block, &disc_bytes);
    check(allocations == 1, "without a block it asks once");
    check(name_in_memory(memory, kPackAt, 2u, 0u) == "Battle of the Blos", "and tags nothing");
    settings.key_quests = false;
    frame(memory, &no_block, &disc_bytes);
}

} // namespace

int main() {
    const std::filesystem::path data = std::filesystem::temp_directory_path() / "pokee_key_quests_tests";
    std::filesystem::create_directories(data);
    set_environment("MHFU_DATA_DIR", data.string());
    set_environment("MHFU_KEY_QUESTS", "");
    set_environment("MHFU_TRACE_KEY_QUESTS", "1");
    test_key_names();
    test_key_names_refuses();
    test_find_pack();
    test_tag_untag();
    test_switch();
    test_no_block();
    std::error_code ec;
    std::filesystem::remove_all(data, ec);
    if (failures != 0) {
        std::cerr << failures << " failure(s)\n";
        return 1;
    }
    std::cout << "key quests tests passed\n";
    return 0;
}
