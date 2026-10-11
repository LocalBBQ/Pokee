#include "game/game_data.hpp"

#include "game/guest_ram.hpp"

namespace mhfu::game {
namespace {

constexpr std::uint32_t kTableEnd = 0xFFFFFFFFu;
// More entries than any of the game's tables has; a larger count means the
// bytes are not a table.
constexpr std::uint32_t kMostEntries = 4096u;
constexpr std::size_t kLongestText = 256u;

// Where table `index` of the text block starts and how many entries it has,
// or nothing when it does not look like a table. 0x0885148C skips the first
// two words of the block.
std::optional<std::pair<std::uint32_t, std::uint32_t>> find_table(const Ram &ram, std::uint32_t block, int index) {
    if (index < 0) return std::nullopt;
    const std::uint32_t entry = block + static_cast<std::uint32_t>(index + 2) * 4u;
    if (!ram.contains(entry, 4u)) return std::nullopt;
    const std::uint32_t relative = read32(ram, entry);
    if (relative < 8u) return std::nullopt;
    const std::uint32_t table = block + relative;
    if (!ram.contains(table, 8u)) return std::nullopt;
    // The first offset points just past the list and its end marker.
    const std::uint32_t first = read32(ram, table);
    if (first < 8u || first % 4u != 0u || first / 4u - 1u > kMostEntries) return std::nullopt;
    const std::uint32_t count = first / 4u - 1u;
    if (!ram.contains(table, first) || read32(ram, table + count * 4u) != kTableEnd) return std::nullopt;
    return std::make_pair(table, count);
}

} // namespace

std::uint32_t read32(const Ram &ram, std::uint32_t address) {
    return static_cast<std::uint32_t>(ram.load8(address)) | (static_cast<std::uint32_t>(ram.load8(address + 1u)) << 8u) |
           (static_cast<std::uint32_t>(ram.load8(address + 2u)) << 16u) |
           (static_cast<std::uint32_t>(ram.load8(address + 3u)) << 24u);
}

std::string read_text(const Ram &ram, std::uint32_t address) {
    std::string text;
    for (std::size_t i = 0; i < kLongestText && ram.contains(address + static_cast<std::uint32_t>(i), 1u); ++i) {
        const char c = static_cast<char>(ram.load8(address + static_cast<std::uint32_t>(i)));
        if (c == '\0') break;
        text += c;
    }
    return text;
}

std::vector<std::string> text_table(const Ram &ram, std::uint32_t block, int index) {
    std::vector<std::string> out;
    const auto table = find_table(ram, block, index);
    if (!table) return out;
    out.reserve(table->second);
    for (std::uint32_t i = 0; i < table->second; ++i)
        out.push_back(read_text(ram, table->first + read32(ram, table->first + i * 4u)));
    return out;
}

std::string text_entry(const Ram &ram, std::uint32_t block, int index, std::uint32_t entry) {
    const auto table = find_table(ram, block, index);
    if (!table || entry >= table->second) return {};
    return read_text(ram, table->first + read32(ram, table->first + entry * 4u));
}

std::uint32_t text_count(const Ram &ram, std::uint32_t block, int index) {
    const auto table = find_table(ram, block, index);
    return table ? table->second : 0u;
}

const std::vector<EquipmentKind> &equipment_kinds() {
    // Armor tables as 0x0885151C and the thunks after it pass them: head 7,
    // chest 9, arms 11, waist 13, legs 15. Weapon classes still have no table
    // of their own in that list (a negative table reads as no names).
    static const std::vector<EquipmentKind> kinds{
        {4, 7, "Head"},           {0, 9, "Chest"},          {1, 11, "Arms"},       {2, 13, "Waist"},
        {3, 15, "Legs"},          {5, -1, "Great Sword"},   {12, -1, "Long Sword"}, {6, -1, "Sword and Shield"},
        {16, -1, "Dual Blades"},  {7, -1, "Hammer"},        {17, -1, "Hunting Horn"}, {8, -1, "Lance"},
        {14, -1, "Gunlance"},     {13, -1, "Switch Axe"},   {11, -1, "Light Bowgun"}, {9, -1, "Heavy Bowgun"},
        {15, -1, "Bow"},
    };
    return kinds;
}

const EquipmentKind *equipment_kind(std::uint8_t kind) {
    for (const EquipmentKind &k : equipment_kinds())
        if (k.kind == kind) return &k;
    return nullptr;
}

bool is_armor(std::uint8_t kind) { return kind <= 4u; }

bool is_weapon(std::uint8_t kind) { return kind >= 5u && equipment_kind(kind) != nullptr; }

std::vector<std::string> equipment_names(const Ram &ram, std::uint8_t kind) {
    const EquipmentKind *k = equipment_kind(kind);
    if (k == nullptr) return {};
    return text_table(ram, kTextBlock, k->name_table);
}

std::string equipment_name(const Ram &ram, std::uint8_t kind, std::uint16_t id) {
    const EquipmentKind *k = equipment_kind(kind);
    if (k == nullptr) return {};
    return text_entry(ram, kTextBlock, k->name_table, id);
}

} // namespace mhfu::game
