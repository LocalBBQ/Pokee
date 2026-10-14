#include "game/layered_armor.hpp"

#include "game/equipment_models.hpp"
#include "game/game_data.hpp"
#include "game/guest_ram.hpp"

#include <algorithm>

namespace mhfu::game::layered {

const char *part_label(std::uint8_t part) {
    static constexpr const char *kLabels[kParts] = {"Chest", "Arms", "Waist", "Legs", "Head"};
    return part < kParts ? kLabels[part] : "?";
}

// The lookup's part, then the kind it draws. Part 0 is the legs, part 2 the
// head, part 3 the chest, part 4 the arms and part 5 the waist: each part's
// file base is that slot. Part 1 is not an armor piece.
std::optional<std::uint8_t> armor_kind(std::uint32_t part) {
    switch (part) {
    case 0: return 3u;
    case 2: return 4u;
    case 3: return 0u;
    case 4: return 1u;
    case 5: return 2u;
    default: return std::nullopt;
    }
}

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
    const std::uint32_t at = ram.load32(kGameBlockPointer) + kHunterRecords + kRecordArmorId[part];
    if (!ram.contains(at, 2u)) return std::nullopt;
    return ram.load16(at);
}

std::optional<WornIds> worn_ids(const Ram &ram) {
    WornIds ids{};
    for (std::uint8_t part = 0; part < kParts; ++part) {
        const std::optional<std::uint16_t> id = played_armor(ram, part);
        if (!id) return std::nullopt;
        ids[part] = *id;
    }
    return ids;
}

bool note_worn(WornSet &stored, const std::optional<WornIds> &now) {
    if (!now) return false;
    const bool changed = stored.known && stored.ids != *now;
    stored.known = true;
    stored.ids = *now;
    return changed;
}

bool owns_piece(const Ram &ram, std::uint8_t part, std::uint16_t id) {
    if (part >= kParts || id == 0u) return false;
    if (const std::optional<std::uint16_t> worn = played_armor(ram, part); worn && *worn == id) return true;
    if (!ram.contains(kGameBlockPointer, 4u)) return false;
    const std::uint32_t block = ram.load32(kGameBlockPointer);
    if (block == 0u) return false;
    const std::uint32_t box = block + kEquipmentBoxOffset;
    if (!ram.contains(box, kEquipmentBoxSlots * kEquipmentRecord)) return false;
    for (std::uint32_t slot = 0; slot < kEquipmentBoxSlots; ++slot) {
        const std::uint32_t at = box + slot * kEquipmentRecord;
        // 0x0885077C keeps a slot when this byte is anything but 0. A purchase writes 1.
        if (ram.load8(at) == 0u || ram.load8(at + 1u) != kBoxKind[part]) continue;
        if (ram.load16(at + 2u) == id) return true;
    }
    return false;
}

std::optional<std::uint16_t> replacement_model(const Ram &ram, std::uint32_t block, std::uint32_t index,
                                               std::uint32_t part, const Pieces &pieces) {
    const std::optional<std::uint8_t> kind = armor_kind(part);
    // A quest does not keep the hunter played here in record 0. The lookup is
    // what loads each hunter this machine draws, so a chosen piece applies to
    // every record. Nothing here is written back, so other players still
    // receive the real armor.
    if (index >= kHunters || !kind) return std::nullopt;
    const std::int32_t choice = pieces[*kind];
    if (choice < 0) return std::nullopt;
    // Only the block the game reaches through its pointer, as traced.
    if (!ram.contains(kGameBlockPointer, 4u) || ram.load32(kGameBlockPointer) != block) return std::nullopt;
    const std::uint32_t record = block + kHunterRecords + index * kHunterRecordBytes;
    if (!ram.contains(record, kHunterRecordBytes)) return std::nullopt;
    const Sex sex = ram.load8(record + kRecordSex) != 0u ? Sex::Female : Sex::Male;
    const auto id = static_cast<std::uint16_t>(choice);
    // Nothing: the model the game gives an empty part, that of piece 0.
    // A piece the hunter does not wear or keep is left to the real armor.
    if (id != 0u && (!wearable_by(ram, *kind, id, sex) || !owns_piece(ram, *kind, id))) return std::nullopt;
    return armor_model(ram, *kind, id, sex);
}

bool sex_lookup_returns_model(std::uint32_t part) { return armor_kind(part).has_value(); }

bool equipment_resolve(std::uint32_t return_address) {
    // lobby_task stores this return address and then calls 0x088D6388.
    // 0x09A7E4AC is the equipment change. The other seven are the lobby's
    // remaining calls of the same function. 0x08866EA8 and 0x08866EFC are
    // the resident file load, and they are not listed.
    switch (return_address) {
    case 0x09A7DBB4u:
    case 0x09A7DB68u:
    case 0x09A7DCD0u:
    case 0x09A7DD18u:
    case 0x09A7DE54u:
    case 0x09A7DE9Cu:
    case 0x09A7E45Cu:
    case 0x09A7E4ACu:
        return true;
    default:
        return false;
    }
}

bool file_load_resolve(std::uint32_t return_address) {
    // 0x08866E10 calls 0x088D6388 and comes back here. Two sites: the armor
    // file table and the other model table.
    return return_address == 0x08866EA8u || return_address == 0x08866EFCu;
}

bool equipment_sex_lookup(std::uint32_t return_address) { return return_address == 0x09A7E360u; }

bool equipment_part_model(std::uint32_t return_address) { return return_address == 0x09A7DF24u; }

bool equipment_part_present(std::uint32_t return_address) { return return_address == 0x09A7DF48u; }

bool equipment_model_read(std::uint32_t return_address) {
    // lobby_task's equipment change. The character load returns to 0x088DBCCC
    // and is not this call.
    return return_address == 0x09A7E548u;
}

bool substitute_resolved_model(std::uint32_t return_address) { return equipment_resolve(return_address); }

std::optional<std::uint16_t> sex_lookup_id(const Ram &ram, std::uint32_t part, std::uint32_t sex, const Pieces &pieces,
                                           std::uint16_t held) {
    const std::optional<std::uint8_t> kind = armor_kind(part);
    if (!kind) return std::nullopt;
    const std::int32_t choice = pieces[*kind];
    if (choice < 0) return std::nullopt;
    const auto id = static_cast<std::uint16_t>(choice);
    if (id == 0u) return 0u;
    const auto fits = [&](std::uint32_t which) {
        const Sex who = (which & 255u) != 0u ? Sex::Female : Sex::Male;
        return wearable_by(ram, *kind, id, who) && (owns_piece(ram, *kind, id) || held == id);
    };
    if (fits(sex)) return id;
    if (const std::optional<std::uint8_t> played = played_sex(ram); played && fits(*played)) return id;
    return std::nullopt;
}

std::optional<std::uint16_t> chosen_model(const Ram &ram, std::uint32_t part, std::uint32_t sex,
                                               const Pieces &pieces) {
    const std::optional<std::uint8_t> kind = armor_kind(part);
    if (!kind) return std::nullopt;
    const std::int32_t choice = pieces[*kind];
    if (choice < 0) return std::nullopt;
    const Sex which = (sex & 255u) != 0u ? Sex::Female : Sex::Male;
    const auto id = static_cast<std::uint16_t>(choice);
    if (id != 0u && !wearable_by(ram, *kind, id, which)) return std::nullopt;
    return armor_model(ram, *kind, id, which);
}

std::optional<std::uint16_t> replacement_model_by_sex(const Ram &ram, std::uint32_t part, std::uint32_t sex,
                                                      const Pieces &pieces, std::uint16_t held) {
    const std::optional<std::uint8_t> kind = armor_kind(part);
    if (!kind) return std::nullopt;
    const std::int32_t choice = pieces[*kind];
    if (choice < 0) return std::nullopt;
    const Sex which = (sex & 255u) != 0u ? Sex::Female : Sex::Male;
    const auto id = static_cast<std::uint16_t>(choice);
    // A piece that was worn until this change is on its way into the box.
    // The equipment reload runs before that slot is written, and would
    // otherwise draw the piece just put on.
    if (id != 0u && (!wearable_by(ram, *kind, id, which) || (!owns_piece(ram, *kind, id) && held != id)))
        return std::nullopt;
    return armor_model(ram, *kind, id, which);
}

std::optional<bool> armor_present(const Ram &ram, std::uint32_t block, std::uint32_t index, std::uint32_t part,
                                   const Pieces &pieces) {
    const std::optional<std::uint8_t> kind = armor_kind(part);
    if (index >= kHunters || !kind) return std::nullopt;
    const std::int32_t choice = pieces[*kind];
    // Nothing: the file loader should take the bare part, not an armor file.
    if (choice == 0) return false;
    if (choice < 0) return std::nullopt;
    const std::uint32_t record = block + kHunterRecords + index * kHunterRecordBytes;
    if (!ram.contains(record + kRecordSex, 1u)) return std::nullopt;
    const Sex sex = ram.load8(record + kRecordSex) != 0u ? Sex::Female : Sex::Male;
    const auto id = static_cast<std::uint16_t>(choice);
    if (!wearable_by(ram, *kind, id, sex) || !owns_piece(ram, *kind, id)) return std::nullopt;
    // A chosen piece is armor. An empty real head would make the loader add the
    // helm's model to the hair files, and the helm would not show.
    return true;
}

// One owned id. slot is the highest box slot that holds it, or -1 when the
// hunter only wears it. The menu lists these by id, the armor table's order.
struct Held {
    std::uint16_t id{};
    std::int32_t slot{-1};
    std::uint16_t copies{};
    bool worn{};
};

std::vector<Held> held_pieces(const Ram &ram, std::uint8_t part) {
    std::vector<Held> list;
    // The hunter played here, from the record.
    const std::optional<Sex> sex = record_sex(ram);
    if (part >= kParts || !sex) return list;
    const auto find = [&](std::uint16_t id) -> Held * {
        for (Held &have : list)
            if (have.id == id) return &have;
        return nullptr;
    };
    const auto add = [&](std::uint16_t id, std::int32_t slot, bool wearing) {
        if (id == 0u || !wearable_by(ram, part, id, *sex)) return;
        if (Held *have = find(id)) {
            ++have->copies;
            if (slot > have->slot) have->slot = slot;
            have->worn = have->worn || wearing;
            return;
        }
        list.push_back(Held{id, slot, 1u, wearing});
    };
    if (const std::optional<std::uint16_t> worn = played_armor(ram, part); worn) add(*worn, -1, true);
    if (ram.contains(kGameBlockPointer, 4u)) {
        const std::uint32_t block = ram.load32(kGameBlockPointer);
        const std::uint32_t box = block + kEquipmentBoxOffset;
        if (block != 0u && ram.contains(box, kEquipmentBoxSlots * kEquipmentRecord)) {
            for (std::uint32_t slot = 0; slot < kEquipmentBoxSlots; ++slot) {
                const std::uint32_t at = box + slot * kEquipmentRecord;
                // 0x0885077C keeps a slot when this byte is anything but 0. A purchase writes 1.
                if (ram.load8(at) == 0u || ram.load8(at + 1u) != kBoxKind[part]) continue;
                add(ram.load16(at + 2u), static_cast<std::int32_t>(slot), false);
            }
        }
    }
    std::sort(list.begin(), list.end(), [](const Held &a, const Held &b) { return a.id < b.id; });
    return list;
}

std::uint32_t owned_count(const Ram &ram, std::uint8_t part) {
    std::uint32_t count = 0u;
    for (const Held &have : held_pieces(ram, part)) count += have.copies;
    return count;
}

std::vector<Offer> offers(const Ram &ram, std::uint8_t part) {
    std::vector<Offer> list;
    const std::vector<Held> held = held_pieces(ram, part);
    const std::vector<std::string> names = equipment_names(ram, part);
    const auto label = [&](std::uint16_t id) {
        if (id < names.size() && !names[id].empty()) return names[id];
        return "Piece " + std::to_string(id);
    };
    list.reserve(held.size());
    for (const Held &have : held) list.push_back({have.id, label(have.id), have.worn, true, have.copies});
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

std::optional<std::uint32_t> loaded_hunter(const Ram &ram) {
    if (!ram.contains(kHunterObjectPointer, 4u)) return std::nullopt;
    const std::uint32_t object = ram.load32(kHunterObjectPointer);
    // The dispatcher ignores a task index of 10 or more, so a word that large
    // is not this hunter.
    if (!own_hunter(ram, object) || !ram.contains(object + kTaskState, 4u) || ram.load32(object + kTaskState) >= 10u)
        return std::nullopt;
    if (!load_done(ram, object)) return std::nullopt;
    return object;
}

bool arm_model_load(Ram &ram, std::uint32_t object) {
    if (!own_hunter(ram, object) || !ram.contains(object + kTaskState, 4u) || ram.load32(object + kTaskState) >= 10u)
        return false;
    // A load still running is the game's, including the face. Starting it
    // again at state 0 calls 0x088DDBD8 and drops the face, and the part
    // files stay finished, so the new look never arrives.
    if (!load_done(ram, object)) return false;
    if (!ram.contains(kFileObjectPointer, 4u) || !ram.contains(kModelObjectPointer, 4u) ||
        !ram.contains(kArchivePointer, 4u))
        return false;
    const std::uint32_t files = ram.load32(kFileObjectPointer);
    const std::uint32_t model = ram.load32(kModelObjectPointer);
    const std::uint32_t archive = ram.load32(kArchivePointer);
    if (files == 0u || model == 0u || archive == 0u) return false;
    if (!ram.contains(files + kFilePartState, kFileParts * 4u) || !ram.contains(files + kFilePump, 4u) ||
        !ram.contains(model + kModelLoaded, 2u) || !ram.contains(archive + kArchiveNeedLoad, 1u))
        return false;
    for (std::uint32_t part = 0; part < kFileParts; ++part) ram.store32(files + kFilePartState + part * 4u, 0u);
    ram.store32(files + kFilePump, 0u);
    ram.store16(model + kModelLoaded, static_cast<std::uint16_t>(ram.load16(model + kModelLoaded) & ~kModelLoadedBit));
    ram.store8(archive + kArchiveNeedLoad, 0u);
    return true;
}

bool request_model_load(Ram &ram) {
    const std::optional<std::uint32_t> object = loaded_hunter(ram);
    if (!object) return false;
    return arm_model_load(ram, *object);
}

} // namespace mhfu::game::layered
