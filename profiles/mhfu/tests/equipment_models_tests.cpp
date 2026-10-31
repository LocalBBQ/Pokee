// Layered armor and the armor tables it reads (game/layered_armor.hpp,
// game/equipment_models.hpp), on a buffer standing for guest memory. No game
// data: the tables and names here are made up.
#include "game/equipment_models.hpp"
#include "game/game_data.hpp"
#include "game/guest_ram.hpp"
#include "game/layered_armor.hpp"

#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

namespace {
using namespace mhfu::game;
int failures{};

void check(bool condition, const char *message) {
    if (!condition) {
        ++failures;
        std::cerr << "FAIL: " << message << '\n';
    }
}

constexpr std::uint32_t kBase = 0x08800000u;
constexpr std::size_t kSize = 0x02000000u; // up to 0x0A800000

void write_table(Ram &ram, std::uint32_t at, const std::vector<std::string> &strings) {
    const auto count = static_cast<std::uint32_t>(strings.size());
    std::uint32_t text = (count + 1u) * 4u;
    for (std::uint32_t i = 0; i < count; ++i) {
        ram.store32(at + i * 4u, text);
        for (std::size_t c = 0; c <= strings[i].size(); ++c)
            ram.store8(at + text + static_cast<std::uint32_t>(c),
                c < strings[i].size() ? static_cast<std::uint8_t>(strings[i][c]) : 0u);
        text += static_cast<std::uint32_t>(strings[i].size() + 1u);
    }
    ram.store32(at + count * 4u, 0xFFFFFFFFu);
}

void write_text(Ram &ram) {
    const auto table = [&ram](int index, std::uint32_t at, const std::vector<std::string> &names) {
        ram.store32(kTextBlock + static_cast<std::uint32_t>(index + 2) * 4u, at - kTextBlock);
        write_table(ram, at, names);
    };
    table(9, kTextBlock + 0x1000u, {"No Equipment", "Plain Mail", "Test Suit", "Plain Mail S"});
    table(7, kTextBlock + 0x2000u, {"No Equipment", "Cap"});
    table(11, kTextBlock + 0x3000u, {"No Equipment"});
}

// An armor record: male and female model, who can wear it.
void write_armor(
    Ram &ram, std::uint32_t table, std::uint16_t id, std::uint16_t male, std::uint16_t female, std::uint8_t who) {
    const std::uint32_t at = table + id * kArmorRecord;
    ram.store16(at, male);
    ram.store16(at + 2u, female);
    ram.store8(at + 4u, who);
}

void write_tables(Ram &ram) {
    write_armor(ram, kChestData, 1, 3, 4, 0x0F);
    write_armor(ram, kChestData, 2, 5, 0, 0x05); // for men only
    write_armor(ram, kChestData, 3, 3, 4, 0x0F); // the same look as 1
    write_armor(ram, kHeadData, 1, 7, 7, 0x0F);
}

// Portable 3rd's character block and equipment box. Layered armor must not
// read them.
constexpr std::uint32_t kOldCharacter = 0x09F4FCACu;
constexpr std::uint32_t kOldCharacterSex = 0x1Bu;
constexpr std::uint32_t kOldEquipmentBox = 0x09F4FE14u;

// Layered armor (game/layered_armor.hpp): the model it gives the game's lookup
// for the hunter played here, and what it leaves to the game.
void test_layered_armor() {
    namespace layered = mhfu::game::layered;
    BufferRam ram(kBase, kSize);
    write_text(ram);
    write_tables(ram);
    // The block with the hunter records, and two hunters: this one (female)
    // and another player (male).
    const std::uint32_t block = 0x09000000u;
    ram.store32(layered::kGameBlockPointer, block);
    const std::uint32_t own = block + layered::kHunterRecords;
    const std::uint32_t other = own + layered::kHunterRecordBytes;
    ram.store8(own + layered::kRecordSex, 1u);
    ram.store8(other + layered::kRecordSex, 0u);
    ram.store16(own + layered::kRecordArmorId[0], 1u); // chest she wears
    ram.store16(own + layered::kRecordArmorId[1], 1u); // arms she wears
    ram.store16(own + layered::kRecordArmorId[4], 1u); // head she wears
    write_armor(ram, kArmsData, 1, 8, 9, 0x0Fu);
    write_armor(ram, kHeadData, 1, 11, 12, 0x0Fu);

    layered::Pieces pieces{layered::kReal, layered::kReal, layered::kReal, layered::kReal, layered::kReal};
    check(!layered::replacement_model(ram, block, 0, 3, pieces), "a real part is left to the game");
    pieces[0] = 1; // chest: Plain Mail, models 3 and 4. The lookup's part 3 draws it.
    check(layered::replacement_model(ram, block, 0, 3, pieces) == std::optional<std::uint16_t>{4},
        "the chosen chest's model for her");
    check(layered::replacement_model(ram, block, 1, 3, pieces) == std::optional<std::uint16_t>{3},
        "a quest hunter is not record 0, so that record is drawn in the choice for its sex");
    check(!layered::replacement_model(ram, block, layered::kHunters, 3, pieces),
        "a record past the four hunters is left to the game");
    check(!layered::replacement_model(ram, block + 4u, 0, 3, pieces), "a block not the game's is left alone");
    check(!layered::replacement_model(ram, block, 0, 1, pieces), "the lookup's part 1 is not armor");
    pieces[1] = 1;
    check(
        layered::replacement_model(ram, block, 0, 4, pieces) == std::optional<std::uint16_t>{9}, "part 4 is the arms");
    pieces[4] = 1;
    check(
        layered::replacement_model(ram, block, 0, 2, pieces) == std::optional<std::uint16_t>{12}, "part 2 is the head");
    check(layered::replacement_model_by_sex(ram, 2, 1, pieces) == std::optional<std::uint16_t>{12},
        "the lobby lookup draws the chosen head for her sex");
    check(layered::replacement_model_by_sex(ram, 2, 0, pieces) == std::optional<std::uint16_t>{11},
        "and the male model when that call says male");
    check(layered::sex_lookup_returns_model(0) && layered::sex_lookup_returns_model(2) &&
            layered::sex_lookup_returns_model(3) && layered::sex_lookup_returns_model(4) &&
            layered::sex_lookup_returns_model(5),
        "an equipment change gets a model for every armor part, even when t0 is 0");
    check(!layered::sex_lookup_returns_model(1), "that lookup's part 1 is the id, and stays the game's");
    check(layered::equipment_resolve(0x09A7E4ACu), "the equipment change is the lobby's model resolve");
    check(!layered::equipment_resolve(0x08866EA8u), "the file load is not the lobby call");
    check(layered::file_load_resolve(0x08866EA8u), "the file load's return is the resident resolve");
    check(!layered::file_load_resolve(0x09A7E4ACu), "the lobby call is not the file load");
    check(layered::substitute_resolved_model(0x09A7E4ACu), "the equipment change returns the chosen model");
    check(!layered::substitute_resolved_model(0x08866EA8u) && !layered::substitute_resolved_model(0x08866EFCu),
        "the resident file load keeps the model the game computed");
    check(layered::equipment_sex_lookup(0x09A7E360u), "the equipment change is the sex lookup that returns a model");
    check(!layered::equipment_sex_lookup(0x09A85840u) && !layered::equipment_sex_lookup(0x09AA6680u),
        "the other sex lookups keep the game's model");
    check(layered::equipment_part_model(0x09A7DF24u), "the equipment change loads the model this lookup returns");
    check(!layered::equipment_part_model(0x088DBCCCu), "the character load is not that lookup");
    check(layered::equipment_part_present(0x09A7DF48u), "the equipment change uses this answer to pick the helm files");
    check(!layered::equipment_part_present(0x088DBCCCu), "the character load does not use that answer");
    check(layered::equipment_model_read(0x09A7E548u), "the equipment change is the file read that takes a model");
    check(!layered::equipment_model_read(0x088DBCCCu), "the character load keeps the model its own lookup returned");
    check(layered::sex_lookup_id(ram, 3, 0, pieces, 0) == std::optional<std::uint16_t>{1},
        "the equipment lookup is given the chosen chest id");
    check(!layered::sex_lookup_id(ram, 1, 0, pieces, 0), "that lookup's part 1 keeps the game's id");
    pieces[0] = layered::kReal;
    check(!layered::sex_lookup_id(ram, 3, 0, pieces, 0), "a real part keeps the game's id");
    pieces[0] = 4;
    check(!layered::sex_lookup_id(ram, 3, 0, pieces, 0), "a piece this hunter does not own keeps the game's id");
    pieces[0] = 1;
    check(!layered::replacement_model_by_sex(ram, 1, 1, pieces), "that lookup's part 1 is not armor");
    check(layered::armor_present(ram, block, 0, 2, pieces) == std::optional<bool>{true},
        "a chosen helm is armor, so the loader reads the helm files");
    pieces[4] = 0;
    check(layered::armor_present(ram, block, 0, 2, pieces) == std::optional<bool>{false},
        "nothing on the head is no armor, so the hair shows");
    pieces[4] = layered::kReal;
    check(!layered::armor_present(ram, block, 0, 2, pieces), "a real head is the game's own answer");
    check(!layered::replacement_model_by_sex(ram, 2, 1, pieces), "a real head is left to the lobby lookup");
    write_armor(ram, kHeadData, 2, 13, 0, 0x05);
    pieces[4] = 2;
    check(!layered::armor_present(ram, block, 0, 2, pieces), "a helm she cannot wear is the game's answer");
    check(!layered::replacement_model_by_sex(ram, 2, 1, pieces), "and the lobby lookup leaves it to the game");
    pieces[4] = 1;
    ram.store8(own + layered::kRecordSex, 0u);
    check(layered::replacement_model(ram, block, 0, 3, pieces) == std::optional<std::uint16_t>{3},
        "the record's sex picks the model");
    ram.store8(own + layered::kRecordSex, 1u);
    pieces[0] = 2; // for men only
    check(!layered::replacement_model(ram, block, 0, 3, pieces), "a piece she cannot wear shows the real one");
    pieces[0] = 0;
    check(layered::replacement_model(ram, block, 0, 3, pieces) == std::optional<std::uint16_t>{0},
        "nothing: the model of an empty part");

    // Offers: the chest she wears, and one kept in the equipment box. A wearable
    // chest she does not own is not offered, and the old box address is not read.
    const std::uint32_t box = block + layered::kEquipmentBoxOffset;
    ram.store8(box, layered::kEquipmentSlotUsed);
    ram.store8(box + 1u, layered::kBoxKind[0]);
    ram.store16(box + 2u, 3u);
    ram.store8(box + kEquipmentRecord, 0u);
    ram.store8(box + kEquipmentRecord + 1u, layered::kBoxKind[0]);
    ram.store16(box + kEquipmentRecord + 2u, 3u);
    ram.store8(kOldEquipmentBox, 1u);
    ram.store8(kOldEquipmentBox + 1u, 0u);
    ram.store16(kOldEquipmentBox + 2u, 4u);
    write_armor(ram, kChestData, 4, 6, 6, 0x0Fu);
    const std::vector<layered::Offer> owned = layered::offers(ram, 0);
    check(owned.size() == 2u && owned[0].id == 1 && owned[0].worn && owned[0].name == "Plain Mail",
        "the worn chest is listed");
    check(owned.size() == 2u && owned[1].id == 3 && owned[1].owned && !owned[1].worn && owned[1].name == "Plain Mail S",
        "the equipment box chest is listed");
    check(!layered::owns_piece(ram, 0, 4), "a wearable chest she does not own is refused");
    pieces[0] = 4;
    check(!layered::replacement_model(ram, block, 0, 3, pieces), "an unowned chest is left to the game");
    check(layered::chosen_model(ram, 3, 1, pieces) == std::optional<std::uint16_t>{6},
        "changing equipment still draws that chest when the box check misses");
    check(layered::replacement_model_by_sex(ram, 3, 1, pieces, 4) == std::optional<std::uint16_t>{6},
        "a chest that just left the worn slot is still the chosen one");
    pieces[0] = 3;
    check(layered::replacement_model(ram, block, 0, 3, pieces) == std::optional<std::uint16_t>{4},
        "a chest in the equipment box is drawn");
    pieces[0] = 1;
    check(layered::choice_name(&ram, 4, 1) == "Cap", "a choice named by the game");
    check(layered::choice_name(nullptr, 4, 1) == "Piece 1", "a choice by id without the game");
    check(layered::choice_name(&ram, 4, layered::kReal) == "Real equipment", "the real piece");
    ram.store32(kTextBlock + (9u + 2u) * 4u, 0u);
    const std::vector<layered::Offer> by_id = layered::offers(ram, 0);
    check(by_id.size() == 2u && by_id[0].id == 1 && by_id[0].name == "Piece 1" && by_id[1].id == 3,
        "without the name table, owned pieces are listed by id");
    // The old character block says male. She is female, and the chest she just
    // bought is in a slot the game counts as used without the byte being 1.
    ram.store8(kOldCharacter + kOldCharacterSex, 0u);
    write_armor(ram, kChestData, 5, 0, 8, 0x02);
    const std::uint32_t bought = box + 2u * kEquipmentRecord;
    ram.store8(bought, 2u);
    ram.store8(bought + 1u, layered::kBoxKind[0]);
    ram.store16(bought + 2u, 5u);
    const std::vector<layered::Offer> bought_list = layered::offers(ram, 0);
    bool listed = false;
    for (const layered::Offer &offer : bought_list) listed = listed || offer.id == 5u;
    check(listed, "a chest she bought is listed for her, not for the old character block");
    check(bought_list.size() == 3u && bought_list[0].id == 1u && bought_list[1].id == 3u && bought_list[2].id == 5u,
        "owned chests are listed in id order, under the game's names");
    const std::uint32_t again = box + 3u * kEquipmentRecord;
    ram.store8(again, 1u);
    ram.store8(again + 1u, layered::kBoxKind[0]);
    ram.store16(again + 2u, 3u);
    bool second = false;
    for (const layered::Offer &offer : layered::offers(ram, 0))
        if (offer.id == 3u) second = offer.copies == 2u;
    check(second, "a second copy of a chest is counted on its row");
    check(layered::owned_count(ram, 0) == 4u, "the owned count is the worn piece plus every box copy");
    // The box numbers armor legs 0, head 1, chest 2, arms 3, waist 4. A legs
    // record (kind 0) holding a chest-wearable id is not a chest, and the same
    // id stored as a chest (kind 2) is.
    check(layered::kBoxKind[0] == 2u && layered::kBoxKind[1] == 3u && layered::kBoxKind[2] == 4u &&
            layered::kBoxKind[3] == 0u && layered::kBoxKind[4] == 1u,
        "the box kinds are the game's: legs 0, head 1, chest 2, arms 3, waist 4");
    write_armor(ram, kChestData, 6, 9, 9, 0x0Fu);
    const std::uint32_t legs = box + 4u * kEquipmentRecord;
    ram.store8(legs, 1u);
    ram.store8(legs + 1u, 0u);
    ram.store16(legs + 2u, 6u);
    bool leaked = false;
    for (const layered::Offer &offer : layered::offers(ram, 0)) leaked = leaked || offer.id == 6u;
    check(!leaked && !layered::owns_piece(ram, 0, 6), "a legs record is not offered as a chest");
    check(layered::owned_count(ram, 0) == 4u, "nor counted as one");
    ram.store8(legs + 1u, layered::kBoxKind[0]);
    check(layered::owns_piece(ram, 0, 6), "the same id kept as a chest is owned");
    ram.store8(legs, 0u);

    // The hunter's load. The step runs for that load only; an address outside
    // guest memory is not it. Done is state 5, and a restart clears the state
    // and the word the game clears with it.
    const std::uint32_t hunter = 0x09100000u;
    ram.store32(hunter + layered::kLoadState, 5u);
    ram.store32(hunter + layered::kLoadCleared, 9u);
    check(layered::own_hunter(ram, hunter) && layered::load_done(ram, hunter), "the hunter played here, loaded");
    layered::restart_load(ram, hunter);
    check(ram.load32(hunter + layered::kLoadState) == 0u && ram.load32(hunter + layered::kLoadCleared) == 0u,
        "a restarted load begins again");
    check(!layered::load_done(ram, hunter), "and is running");
    check(!layered::own_hunter(ram, 0u), "no object is not the hunter's load");
    // Asking again releases the part files and the flags that make an area
    // and a quest skip the pump. The hunter's own load stays finished:
    // setting it back to 0 drops the face.
    const std::uint32_t files = 0x09200000u;
    const std::uint32_t model = 0x09400000u;
    const std::uint32_t archive = 0x09300000u;
    ram.store32(hunter + layered::kLoadState, 5u);
    ram.store32(hunter + layered::kTaskState, 4u);
    ram.store32(layered::kHunterObjectPointer, hunter);
    check(layered::loaded_hunter(ram) == std::optional<std::uint32_t>{hunter}, "the loaded hunter");
    check(!layered::request_model_load(ram), "no file object, nothing is released");
    ram.store32(layered::kFileObjectPointer, files);
    ram.store32(layered::kModelObjectPointer, model);
    ram.store32(layered::kArchivePointer, archive);
    for (std::uint32_t part = 0; part < layered::kFileParts; ++part)
        ram.store32(files + layered::kFilePartState + part * 4u, 8u);
    ram.store32(files + layered::kFilePump, 4u);
    ram.store16(model + layered::kModelLoaded, 0xFFFFu);
    ram.store8(archive + layered::kArchiveNeedLoad, 1u);
    check(layered::request_model_load(ram), "a loaded hunter can be asked to load again");
    bool files_released = ram.load32(files + layered::kFilePump) == 0u;
    for (std::uint32_t part = 0; part < layered::kFileParts; ++part)
        files_released = files_released && ram.load32(files + layered::kFilePartState + part * 4u) == 0u;
    check(files_released && (ram.load16(model + layered::kModelLoaded) & layered::kModelLoadedBit) == 0u &&
            ram.load8(archive + layered::kArchiveNeedLoad) == 0u && ram.load32(hunter + layered::kTaskState) == 4u &&
            ram.load32(hunter + layered::kLoadState) == 5u,
        "the part files are read again and the hunter's load is left finished");
    ram.store32(hunter + layered::kLoadState, 0u);
    ram.store32(files + layered::kFilePartState, 8u);
    check(layered::arm_model_load(ram, hunter) == false, "a load already running is left to finish");
    check(ram.load32(files + layered::kFilePartState) == 8u, "and its files stay as they are");
    ram.store32(hunter + layered::kLoadState, 5u);
    ram.store32(hunter + layered::kTaskState, 10u);
    check(!layered::loaded_hunter(ram), "a task index the dispatcher ignores is not the hunter");

    // The menu used to wait on the Portable 3rd character address, so a loaded
    // hunter kept every part on Real. The record is enough.
    BufferRam bare(kBase, kSize);
    write_tables(bare);
    check(!layered::played_sex(bare), "no hunter before the block exists");
    bare.store32(layered::kGameBlockPointer, block);
    bare.store8(block + layered::kHunterRecords + layered::kRecordSex, 1u);
    check(layered::played_sex(bare) == std::optional<std::uint8_t>{1u}, "her sex comes from the record");
    check(layered::offers(bare, 0).empty(), "a record that owns no armor lists nothing");
    bare.store16(block + layered::kHunterRecords + layered::kRecordArmorId[0], 1u);
    const std::vector<layered::Offer> from_record = layered::offers(bare, 0);
    check(from_record.size() == 1u && from_record[0].id == 1 && from_record[0].worn && from_record[0].name == "Piece 1",
        "the worn piece is listed from the hunter record without the old character block");

    // Changing equipment reloads the meshes itself. The first set is the hunter
    // appearing, not a change; a later set is, once. The flip records that
    // change and does not release the part files for it.
    for (std::uint8_t part = 0; part < layered::kParts; ++part)
        bare.store16(
            block + layered::kHunterRecords + layered::kRecordArmorId[part], static_cast<std::uint16_t>(part + 1u));
    const std::optional<layered::WornIds> first = layered::worn_ids(bare);
    check(first && (*first)[0] == 1u && (*first)[4] == 5u, "the five worn ids are what a change is compared with");
    layered::WornSet worn;
    check(!layered::note_worn(worn, first), "the hunter appearing is not a change of equipment");
    check(!layered::note_worn(worn, first), "the same armor is not another change");
    if (first) {
        layered::WornIds changed = *first;
        changed[0] = 9u;
        check(layered::note_worn(worn, changed), "a different set is one change of equipment");
        check(!layered::note_worn(worn, changed), "that change is only asked once");
        check(!layered::note_worn(worn, std::nullopt), "no hunter does not forget the armor");
        check(!layered::note_worn(worn, changed), "and the same armor afterwards is not a new change");
    }
}

} // namespace

int main() {
    test_layered_armor();
    if (failures == 0) std::cout << "equipment models tests passed\n";
    return failures == 0 ? 0 : 1;
}
