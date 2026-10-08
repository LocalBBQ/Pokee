#include "game/layered_armor.hpp"

#include "game/equipment_models.hpp"
#include "game/game_data.hpp"
#include "game/guest_ram.hpp"

#include <set>

namespace mhfu::game::layered {

const char *part_label(std::uint8_t part) {
    static constexpr const char *kLabels[kParts] = {"Chest", "Arms", "Waist", "Legs", "Head"};
    return part < kParts ? kLabels[part] : "?";
}

// The lookup's part, then the kind it draws. Part 1 is not an armor piece.
std::optional<std::uint8_t> armor_kind(std::uint32_t part) {
    switch (part) {
    case 0: return 0u;
    case 2: return 1u;
    case 3: return 2u;
    case 4: return 3u;
    case 5: return 4u;
    default: return std::nullopt;
    }
}

// Where a record keeps that kind's armor id.
constexpr std::uint32_t kArmorId[kParts] = {0x3Eu, 0x4Au, 0x56u, 0x62u, 0x6Eu};
// The lookup refuses an id at or past this (slti at the id, 0x5DD).
constexpr std::uint16_t kMostArmorIds = 0x5DDu;

std::optional<Sex> record_sex(const Ram &ram) {
    if (!ram.contains(kGameBlockPointer, 4u)) return std::nullopt;
    const std::uint32_t record = ram.load32(kGameBlockPointer) + kHunterRecords;
    if (!ram.contains(record + kRecordSex, 1u)) return std::nullopt;
    return ram.load8(record + kRecordSex) != 0u ? Sex::Female : Sex::Male;
}

std::optional<std::uint8_t> played_sex(const Ram &ram) {
    const std::optional<Sex> sex = record_sex(ram);
    if (!sex) return std::nullopt;
    return *sex == Sex::Female ? 1u : 0u;
}

std::optional<std::uint16_t> played_armor(const Ram &ram, std::uint8_t part) {
    if (part >= kParts || !ram.contains(kGameBlockPointer, 4u)) return std::nullopt;
    const std::uint32_t at = ram.load32(kGameBlockPointer) + kHunterRecords + kArmorId[part];
    if (!ram.contains(at, 2u)) return std::nullopt;
    return ram.load16(at);
}

std::optional<std::uint16_t> replacement_model(const Ram &ram, std::uint32_t block, std::uint32_t index,
                                               std::uint32_t part, const Pieces &pieces) {
    const std::optional<std::uint8_t> kind = armor_kind(part);
    if (index != kOwnRecord || !kind) return std::nullopt;
    const std::int32_t choice = pieces[*kind];
    if (choice < 0) return std::nullopt;
    // Only the block the game reaches through its pointer, as traced.
    if (!ram.contains(kGameBlockPointer, 4u) || ram.load32(kGameBlockPointer) != block) return std::nullopt;
    const std::uint32_t record = block + kHunterRecords + index * kHunterRecordBytes;
    if (!ram.contains(record, kHunterRecordBytes)) return std::nullopt;
    const Sex sex = ram.load8(record + kRecordSex) != 0u ? Sex::Female : Sex::Male;
    const auto id = static_cast<std::uint16_t>(choice);
    // Nothing: the model the game gives an empty part, that of piece 0.
    if (id != 0u && !wearable_by(ram, *kind, id, sex)) return std::nullopt;
    return armor_model(ram, *kind, id, sex);
}

std::vector<Offer> offers(const Ram &ram, std::uint8_t part, bool all) {
    std::vector<Offer> list;
    const std::optional<Look> look = hunter_look(ram);
    const std::optional<Sex> sex = look ? std::optional<Sex>{look->sex} : record_sex(ram);
    if (part >= kParts || !sex) return list;
    const std::vector<std::string> names = equipment_names(ram, part);
    const std::optional<Piece> worn = worn_armor(ram, part);
    std::optional<std::uint16_t> worn_id = worn ? std::optional<std::uint16_t>{worn->id} : std::nullopt;
    if (!worn_id && ram.contains(kGameBlockPointer, 4u)) {
        const std::uint32_t at = ram.load32(kGameBlockPointer) + kHunterRecords + kArmorId[part];
        if (ram.contains(at, 2u)) worn_id = ram.load16(at);
    }
    // The name tables and the equipment box are still the Portable 3rd
    // addresses, so on this executable the names come back empty. The pieces
    // are then listed from the armor tables, by id.
    if (names.empty()) {
        if (!all) {
            if (worn_id && *worn_id != 0u && wearable_by(ram, part, *worn_id, *sex))
                list.push_back({*worn_id, "Piece " + std::to_string(*worn_id), true, true});
            return list;
        }
        for (std::uint16_t id = 1; id < kMostArmorIds; ++id) {
            if (!ram.contains(armor_data(part) + static_cast<std::uint32_t>(id) * kArmorRecord, kArmorRecord)) break;
            if (!wearable_by(ram, part, id, *sex)) continue;
            list.push_back({id, "Piece " + std::to_string(id), worn_id && *worn_id == id, worn_id && *worn_id == id});
        }
        return list;
    }
    std::set<std::uint16_t> owned;
    if (worn_id && *worn_id != 0u) owned.insert(*worn_id);
    for (std::uint32_t slot = 0; slot < kEquipmentBoxSlots; ++slot) {
        const std::uint32_t at = kEquipmentBox + slot * kEquipmentRecord;
        if (!ram.contains(at, kEquipmentRecord)) break;
        if (ram.load8(at) != 0u && ram.load8(at + 1u) == part) owned.insert(ram.load16(at + 2u));
    }
    for (std::size_t id = 1; id < names.size(); ++id) {
        const auto piece = static_cast<std::uint16_t>(id);
        const bool has = owned.contains(piece);
        if ((!all && !has) || names[id].empty() || !wearable_by(ram, part, piece, *sex)) continue;
        list.push_back({piece, names[id], worn_id && *worn_id == piece, has});
    }
    return list;
}

std::string choice_name(const Ram *ram, std::uint8_t part, std::int32_t choice) {
    if (choice < 0) return "Real equipment";
    if (choice == 0) return "Nothing";
    const auto id = static_cast<std::uint16_t>(choice);
    if (ram != nullptr) {
        const std::string name = equipment_name(*ram, part, id);
        if (!name.empty()) return name;
    }
    return "Piece " + std::to_string(id);
}

bool own_hunter(const Ram &ram, std::uint32_t object) {
    return object != 0u && ram.contains(object + kLoadState, 8u);
}

bool load_done(const Ram &ram, std::uint32_t object) { return ram.load32(object + kLoadState) == kLoadDone; }

void restart_load(Ram &ram, std::uint32_t object) {
    ram.store32(object + kLoadCleared, 0u);
    ram.store32(object + kLoadState, 0u);
}

} // namespace mhfu::game::layered
