#include "debug/game_state.hpp"

#include "debug/debug_tools.hpp"
#include "debug/guest_ram.hpp"

#include <algorithm>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <set>

namespace mhp3rd::debug {
namespace p3rd {
namespace {

constexpr std::uint32_t kTableEnd = 0xFFFFFFFFu;
// More entries than any of the game's tables has; a larger count means the
// bytes are not a table.
constexpr std::uint32_t kMostEntries = 4096u;
constexpr std::size_t kLongestText = 256u;

// The text block is not word aligned throughout, so read bytes.
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

std::uint32_t item_slot(std::uint32_t slot) { return kItemBox + slot * 4u; }
std::uint32_t equipment_slot(std::uint32_t slot) { return kEquipmentBox + slot * kEquipmentRecord; }

} // namespace

bool character_loaded(const Ram &ram) { return !hunter_name(ram).empty(); }

std::string hunter_name(const Ram &ram) {
    std::string name;
    if (!ram.contains(kHunterName, kHunterNameUnits * 2u)) return name;
    for (std::size_t i = 0; i < kHunterNameUnits; ++i) {
        std::uint32_t c = ram.load16(kHunterName + static_cast<std::uint32_t>(i * 2u));
        if (c == 0u) break;
        // The game keeps ASCII names as their fullwidth forms.
        if (c >= 0xFF01u && c <= 0xFF5Eu) c -= 0xFEE0u;
        if (c == 0x3000u) c = ' ';
        if (c < 0x80u) {
            name += static_cast<char>(c);
        } else if (c < 0x800u) {
            name += static_cast<char>(0xC0u | (c >> 6u));
            name += static_cast<char>(0x80u | (c & 0x3Fu));
        } else {
            name += static_cast<char>(0xE0u | (c >> 12u));
            name += static_cast<char>(0x80u | ((c >> 6u) & 0x3Fu));
            name += static_cast<char>(0x80u | (c & 0x3Fu));
        }
    }
    return name;
}

namespace {
// Where table `index` of the text block starts and how many entries it has,
// or nothing when it does not look like a table.
std::optional<std::pair<std::uint32_t, std::uint32_t>> find_table(const Ram &ram, std::uint32_t block, int index) {
    const std::uint32_t entry = block + static_cast<std::uint32_t>(index) * 4u;
    if (index < 0 || !ram.contains(entry, 4u)) return std::nullopt;
    const std::uint32_t table = block + read32(ram, entry);
    if (!ram.contains(table, 8u)) return std::nullopt;
    // The first offset points just past the list and its end marker.
    const std::uint32_t first = read32(ram, table);
    if (first < 8u || first % 4u != 0u || first / 4u - 1u > kMostEntries) return std::nullopt;
    const std::uint32_t count = first / 4u - 1u;
    if (!ram.contains(table, first) || read32(ram, table + count * 4u) != kTableEnd) return std::nullopt;
    return std::make_pair(table, count);
}
} // namespace

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

ItemInfo item_info(const Ram &ram, std::uint16_t id) {
    ItemInfo info;
    const std::uint32_t at = kItemData + static_cast<std::uint32_t>(id) * kItemRecord;
    if (!ram.contains(at, kItemRecord)) return info;
    info.category = ram.load8(at + 4u);
    info.rarity = ram.load8(at + 5u);
    info.carry = ram.load8(at + 6u);
    info.use = ram.load8(at + 7u);
    info.buy = read32(ram, at + 12u);
    info.sell = read32(ram, at + 16u);
    return info;
}

ItemGroup item_group(const ItemInfo &info) {
    switch (info.category) {
    case 1: return ItemGroup::Ammo;
    case 3: return ItemGroup::Decoration;
    case 0:
        if (info.use != 0u) return ItemGroup::Consumable;
        // Materials stack to 99 in the pouch and are not used from it.
        return info.carry == kMostPerStack ? ItemGroup::Material : ItemGroup::Other;
    default: return ItemGroup::Other;
    }
}

const char *group_name(ItemGroup group) {
    switch (group) {
    case ItemGroup::Material: return "Material";
    case ItemGroup::Consumable: return "Consumable";
    case ItemGroup::Ammo: return "Ammo";
    case ItemGroup::Decoration: return "Decoration";
    default: return "Other";
    }
}

std::vector<Item> item_list(const Ram &ram) {
    std::vector<Item> items;
    const std::vector<std::string> names = text_table(ram, kTextBlock, kItemNameTable);
    // Id 0 is the empty slot.
    for (std::size_t id = 1; id < names.size(); ++id) {
        if (names[id].empty()) continue;
        const auto item_id = static_cast<std::uint16_t>(id);
        items.push_back({item_id, names[id], item_group(item_info(ram, item_id))});
    }
    return items;
}

std::vector<ItemStack> item_box(const Ram &ram) {
    std::vector<ItemStack> box(kItemBoxSlots);
    for (std::uint32_t i = 0; i < kItemBoxSlots; ++i)
        box[i] = {ram.load16(item_slot(i)), ram.load16(item_slot(i) + 2u)};
    return box;
}

std::uint32_t box_count(const Ram &ram, std::uint16_t id) {
    std::uint32_t total = 0u;
    for (const ItemStack &s : item_box(ram))
        if (s.id == id) total += s.count;
    return total;
}

std::uint32_t free_item_slots(const Ram &ram) {
    std::uint32_t free = 0u;
    for (const ItemStack &s : item_box(ram))
        if (s.id == 0u) ++free;
    return free;
}

std::uint32_t give_item(Ram &ram, std::uint16_t id, std::uint32_t count) {
    if (id == 0u || count == 0u) return 0u;
    std::uint32_t left = count;
    // Top up the item's own stacks first.
    for (std::uint32_t i = 0; i < kItemBoxSlots && left > 0u; ++i) {
        if (ram.load16(item_slot(i)) != id) continue;
        const std::uint16_t have = ram.load16(item_slot(i) + 2u);
        if (have >= kMostPerStack) continue;
        const std::uint32_t add = std::min<std::uint32_t>(left, kMostPerStack - have);
        ram.store16(item_slot(i) + 2u, static_cast<std::uint16_t>(have + add));
        left -= add;
    }
    for (std::uint32_t i = 0; i < kItemBoxSlots && left > 0u; ++i) {
        if (ram.load16(item_slot(i)) != 0u) continue;
        const std::uint32_t add = std::min<std::uint32_t>(left, kMostPerStack);
        ram.store16(item_slot(i), id);
        ram.store16(item_slot(i) + 2u, static_cast<std::uint16_t>(add));
        left -= add;
    }
    return count - left;
}

std::uint32_t remove_item(Ram &ram, std::uint16_t id) {
    if (id == 0u) return 0u;
    std::uint32_t removed = 0u;
    for (std::uint32_t i = 0; i < kItemBoxSlots; ++i) {
        if (ram.load16(item_slot(i)) != id) continue;
        removed += ram.load16(item_slot(i) + 2u);
        ram.store16(item_slot(i), 0u);
        ram.store16(item_slot(i) + 2u, 0u);
    }
    return removed;
}

std::uint32_t fill_materials(Ram &ram, std::uint16_t count) {
    std::set<std::uint16_t> held;
    for (const ItemStack &s : item_box(ram))
        if (s.id != 0u) held.insert(s.id);
    std::uint32_t kinds = 0u;
    for (const Item &item : item_list(ram)) {
        if (item.group != ItemGroup::Material || held.count(item.id) != 0u) continue;
        if (give_item(ram, item.id, count) == 0u) break;  // the box is full
        ++kinds;
    }
    return kinds;
}

const std::vector<EquipmentKind> &equipment_kinds() {
    // Kinds and tables as traced: the equipment box sorted by the game lists
    // the weapon kinds in the order below, and each kind's ids fit only its
    // own name table.
    static const std::vector<EquipmentKind> kinds{
        {4, 29, "Head"},          {0, 31, "Chest"},         {1, 33, "Arms"},       {2, 35, "Waist"},
        {3, 37, "Legs"},          {5, 5, "Great Sword"},    {12, 17, "Long Sword"}, {6, 7, "Sword and Shield"},
        {16, 25, "Dual Blades"},  {7, 9, "Hammer"},         {17, 27, "Hunting Horn"}, {8, 11, "Lance"},
        {14, 21, "Gunlance"},     {13, 19, "Switch Axe"},   {11, 15, "Light Bowgun"}, {9, 13, "Heavy Bowgun"},
        {15, 23, "Bow"},
    };
    return kinds;
}

const EquipmentKind *equipment_kind(std::uint8_t kind) {
    for (const EquipmentKind &k : equipment_kinds())
        if (k.kind == kind) return &k;
    return nullptr;
}

std::vector<std::optional<Equipment>> equipment_box(const Ram &ram) {
    std::vector<std::optional<Equipment>> box(kEquipmentBoxSlots);
    for (std::uint32_t i = 0; i < kEquipmentBoxSlots; ++i) {
        const std::uint32_t at = equipment_slot(i);
        if (ram.load8(at) == 0u) continue;
        box[i] = Equipment{ram.load8(at + 1u), ram.load16(at + 2u), ram.load16(at + 4u)};
    }
    return box;
}

std::uint32_t free_equipment_slots(const Ram &ram) {
    std::uint32_t free = 0u;
    for (std::uint32_t i = 0; i < kEquipmentBoxSlots; ++i)
        if (ram.load8(equipment_slot(i)) == 0u) ++free;
    return free;
}

std::optional<std::uint32_t> give_equipment(Ram &ram, std::uint8_t kind, std::uint16_t id) {
    if (equipment_kind(kind) == nullptr || id == 0u) return std::nullopt;
    for (std::uint32_t i = 0; i < kEquipmentBoxSlots; ++i) {
        const std::uint32_t at = equipment_slot(i);
        if (ram.load8(at) != 0u) continue;
        for (std::uint32_t b = 4u; b < kEquipmentRecord; b += 2u) ram.store16(at + b, 0u);
        ram.store16(at + 2u, id);
        ram.store8(at + 1u, kind);
        ram.store8(at, 1u);
        return i;
    }
    return std::nullopt;
}

std::vector<std::string> equipment_names(const Ram &ram, std::uint8_t kind) {
    const EquipmentKind *k = equipment_kind(kind);
    if (k == nullptr) return {};
    return text_table(ram, kTextBlock, k->name_table);
}

bool on_quest(const Ram &ram) {
    // The overlay header: "MWo3", its load address at +8, its name at +32.
    if (!ram.contains(kTaskSlot, 64u) || ram.load32(kTaskSlot) != 0x336F574Du || ram.load32(kTaskSlot + 8u) != kTaskSlot)
        return false;
    return read_text(ram, kTaskSlot + 32u) == "game_task.ovl";
}

std::vector<Monster> monsters(const Ram &ram) {
    std::vector<Monster> out;
    if (!on_quest(ram)) return out;
    for (std::uint32_t i = 0; i < kMonsterSlots; ++i) {
        const std::uint32_t at = ram.load32(kMonsterTable + i * 4u);
        if (at < 0x08800000u || !ram.contains(at, kMonsterMostHealth + 2u)) continue;
        Monster m{at, ram.load8(at + kMonsterKind), static_cast<std::int16_t>(ram.load16(at + kMonsterHealth)),
                  static_cast<std::int16_t>(ram.load16(at + kMonsterMostHealth))};
        if (m.most <= 0) continue;  // a companion, or not spawned yet
        out.push_back(m);
    }
    return out;
}

std::string monster_name(const Ram &ram, std::uint8_t kind) {
    if (kind >= kMonsterKinds) return "Monster " + std::to_string(kind);
    const std::string name = text_entry(ram, kTextBlock, kMonsterNameTable, kMonsterNameFirst + kind);
    return !name.empty() ? name : "Monster " + std::to_string(kind);
}

std::uint32_t money(const Ram &ram) { return ram.load32(kMoney); }
void set_money(Ram &ram, std::uint32_t zenny) { ram.store32(kMoney, std::min(zenny, kMostMoney)); }

} // namespace p3rd

namespace {
std::uint32_t number(const std::string &text) {
    return static_cast<std::uint32_t>(std::strtoll(text.c_str(), nullptr, 0));
}
} // namespace

bool game_command(Ram &ram, const std::string &command, const std::vector<std::string> &args,
                  std::vector<std::string> &out) {
    using namespace p3rd;
    const auto arg = [&](std::size_t i, std::uint32_t fallback = 0u) {
        return i < args.size() ? number(args[i]) : fallback;
    };
    if (command == "state") {
        out.push_back("character: " + (character_loaded(ram) ? hunter_name(ram) : std::string("(none)")) +
                      ", zenny " + std::to_string(money(ram)) + ", free item slots " +
                      std::to_string(free_item_slots(ram)) + ", free equipment slots " +
                      std::to_string(free_equipment_slots(ram)) + ", items named " +
                      std::to_string(item_list(ram).size()));
    } else if (command == "money") {
        set_money(ram, arg(0));
        out.push_back("zenny now " + std::to_string(money(ram)));
    } else if (command == "give") {
        const auto id = static_cast<std::uint16_t>(arg(0));
        const std::uint32_t given = give_item(ram, id, arg(1, 1u));
        out.push_back("gave " + std::to_string(given) + " of item " + std::to_string(id) + "; box holds " +
                      std::to_string(box_count(ram, id)));
    } else if (command == "remove") {
        const auto id = static_cast<std::uint16_t>(arg(0));
        out.push_back("removed " + std::to_string(remove_item(ram, id)) + " of item " + std::to_string(id));
    } else if (command == "fillmats") {
        out.push_back("added " + std::to_string(fill_materials(ram, static_cast<std::uint16_t>(arg(0, 99u)))) +
                      " kinds of material");
    } else if (command == "giveequip") {
        const auto kind = static_cast<std::uint8_t>(arg(0));
        const auto id = static_cast<std::uint16_t>(arg(1));
        const std::optional<std::uint32_t> slot = give_equipment(ram, kind, id);
        const std::vector<std::string> names = equipment_names(ram, kind);
        out.push_back(slot ? "equipment " + std::to_string(kind) + ":" + std::to_string(id) + " (" +
                                 (id < names.size() ? names[id] : std::string("?")) + ") in slot " +
                                 std::to_string(*slot)
                           : std::string("no equipment given"));
    } else if (command == "item") {
        const auto id = static_cast<std::uint16_t>(arg(0));
        const std::vector<std::string> names = text_table(ram, kTextBlock, kItemNameTable);
        const ItemInfo info = item_info(ram, id);
        out.push_back("item " + std::to_string(id) + " " + (id < names.size() ? names[id] : std::string("?")) +
                      ": " + group_name(item_group(info)) + ", rarity " + std::to_string(info.rarity) +
                      ", carry " + std::to_string(info.carry) + ", in box " + std::to_string(box_count(ram, id)));
    } else if (command == "table") {
        const std::vector<std::string> t = text_table(ram, kTextBlock, static_cast<int>(arg(0)));
        out.push_back("table " + std::to_string(arg(0)) + ": " + std::to_string(t.size()) + " entries");
        for (std::size_t i = arg(1); i < t.size() && i < arg(1) + arg(2, 8u); ++i)
            out.push_back("  " + std::to_string(i) + " " + t[i]);
    } else if (command == "quest") {
        if (!on_quest(ram)) out.push_back("not on a quest");
        for (const std::string &line : quest_lines(ram)) out.push_back(line);
    } else if (command == "monsterhp") {
        // monsterhp N: every monster's health to N (at least 1).
        const auto hp = static_cast<std::uint16_t>(std::max<std::uint32_t>(arg(0, 1u), 1u));
        std::size_t changed = 0u;
        for (const Monster &m : monsters(ram)) {
            ram.store16(m.address + kMonsterHealth, hp);
            ++changed;
        }
        out.push_back("set " + std::to_string(changed) + " monsters to " + std::to_string(hp) + " health");
    } else {
        return false;
    }
    return true;
}

void held_cheats_frame(Ram &ram, const HeldCheats &cheats) {
    using namespace p3rd;
    static std::optional<std::uint32_t> frozen_time;
    if (!on_quest(ram)) {
        frozen_time.reset();
        return;
    }
    if (cheats.health) {
        const std::uint16_t most = ram.load16(kMostHealth);
        if (most > 0u) {
            ram.store16(kHealth, most);
            ram.store16(kRecoverableHealth, most);
        }
    }
    if (cheats.stamina) {
        const float most = static_cast<float>(ram.load16(kMostStamina));
        std::uint32_t bits = 0u;
        static_assert(sizeof(bits) == sizeof(most));
        std::memcpy(&bits, &most, sizeof(bits));
        if (most > 0.0f) ram.store32(kStamina, bits);
    }
    if (cheats.timer) {
        if (!frozen_time) frozen_time = ram.load32(kQuestTimeLeft);
        ram.store32(kQuestTimeLeft, *frozen_time);
    } else {
        frozen_time.reset();
    }
    if (cheats.one_hit)
        for (const Monster &m : monsters(ram))
            if (m.health > 1) ram.store16(m.address + kMonsterHealth, 1u);
}

std::vector<std::string> quest_lines(const Ram &ram) {
    using namespace p3rd;
    std::vector<std::string> lines;
    if (!on_quest(ram)) return lines;
    const std::uint32_t left = ram.load32(kQuestTimeLeft) / 30u;
    char clock[32];
    std::snprintf(clock, sizeof(clock), "%u:%02u left of %u min", left / 60u, left % 60u,
                  ram.load32(kQuestTimeLimit) / 1800u);
    lines.push_back(std::string("Time ") + clock);
    lines.push_back("Hunter health " + std::to_string(static_cast<std::int16_t>(ram.load16(kHealth))) + "/" +
                    std::to_string(ram.load16(kMostHealth)));
    for (const Monster &m : monsters(ram))
        lines.push_back(monster_name(ram, m.kind) + " " + std::to_string(m.health) + "/" + std::to_string(m.most));
    return lines;
}

} // namespace mhp3rd::debug
