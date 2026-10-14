#pragma once

// Layered armor: the hunter drawn in other armor than it wears, part by part,
// while the game keeps the real pieces for defense, skills, the save and what
// it sends to other players. The choices live in the port's settings
// (settings.hpp, look.layered_*), never in the game's memory or save.
//
// Read from the USA Freedom Unite executable (ULUS-10391);
// docs/LAYERED_ARMOR.md says how. In short:
//
// - The block at [0x089CC558] holds a 140-byte record per hunter from +0x30.
//   Record 0 is the hunter played here. A record's byte at +0x10 is 0 for a
//   male hunter and anything else for a female one. The armor ids are u16 at
//   +0x56, +0x62, +0x6E, +0x3E and +0x4A (chest, arms, waist, legs, head). The
//   part argument is not that order: 0 legs, 2 head, 3 chest, 4 arms, 5 waist.
//   Part 1 is not armor.
// - 0x0885187C (a0 the block, a1 the record, a2 the part) returns that part's
//   model: the male or female half of the armor table at the record's id. It
//   only reads. The callers in the executable are what draws a hunter, in the
//   village and on a quest. Defense, skills and the Equipment screen read the
//   tables by id elsewhere, and nothing here writes guest memory, so the save
//   and what the game sends to other players keep the real pieces.
//
// So layered armor replaces that one function while it is on: for a hunter
// record and a part with a chosen piece it returns the chosen piece's model
// for the record's sex, exactly as the game would for that piece; anything
// else runs the game's own code. A quest does not keep the hunter played here
// in record 0, so the choice applies to every record this machine draws.
// 0x088516A4 is the same question asked by sex and part, which the guild and
// the lobby use, including the reload when the hunter changes equipment.
// 0x08851C3C tells the file load whether a part has armor; an
// empty real head would otherwise add a chosen helm's model to the hair
// files. Nothing is written back, so other players still receive the real
// armor. It also wraps the hunter's load step so that a changed look shows at
// the game's next load (below). Off from the start, nothing is registered and
// the game runs as built.

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace psprecomp {
class Runtime;
}

namespace mhfu::game {

class Ram;

namespace layered {

// The pointer to the block that holds the hunter records.
inline constexpr std::uint32_t kGameBlockPointer = 0x089CC558u;
inline constexpr std::uint32_t kHunterRecords = 0x30u;
inline constexpr std::uint32_t kHunterRecordBytes = 140u;
inline constexpr std::uint32_t kHunters = 4u;
inline constexpr std::uint32_t kRecordSex = 0x10u;  // u8: 0 male, otherwise female
// The record of the hunter played on this machine.
inline constexpr std::uint32_t kOwnRecord = 0u;

// The equipment box, read from this executable. 0x08901788 walks it to see
// whether the hunter owns enough of one kind. The records start at
// load32(0x089CC558) + 0x548. 0x08850e58 begins at 8 pages of 100 and adds one
// page for each of two checks; both checks come back clear, so the walk is
// 1000 records. A record is 12 bytes, the same shape as a worn piece: byte 0
// is 1 when the slot is used, byte 1 is the kind (kBoxKind), and the id is the
// u16 at +2.
inline constexpr std::uint32_t kEquipmentBoxOffset = 0x548u;
inline constexpr std::uint32_t kEquipmentBoxSlots = 1000u;
inline constexpr std::uint8_t kEquipmentSlotUsed = 1u;
// The game's model lookup that layered armor stands in for.
inline constexpr std::uint32_t kModelOfPart = 0x0885187Cu;
// The same model, asked by sex and part (0x088516A4). a1 is the sex, a2 the
// part and a3 the armor id. Parts 0, 3, 4 and 5 return that id's model and do
// not read t0. Part 2 returns the model when t0 is 1 and the id otherwise.
// Part 1 returns the id. The equipment change in lobby_task (the call that
// returns to 0x09A7E360) passes the result to 0x088DB4C8 as the model to
// read, and it sets t0 from a halfword that is 0 for an ordinary change.
inline constexpr std::uint32_t kModelBySex = 0x088516A4u;
// Whether a part has armor (0x08851C3C). The file load adds the model to the
// armor files only when this is 1; otherwise a head model is added to the hair.
inline constexpr std::uint32_t kHasArmor = 0x08851C3Cu;
// The equipment change resolves the file here (0x088D6388). a1 is the part
// and a2 the model; a nonzero model is returned as the model to read.
// lobby_task calls it with the newly worn piece's model (return 0x09A7E4AC).
// The hunter's own file load also calls it, from 0x08866E10, and that call
// already carries the model from 0x0885187C. Replacing that one loads a model
// the file is not ready for and faults on the way into a quest select.
inline constexpr std::uint32_t kResolveModel = 0x088D6388u;
// True when this return address is a lobby_task call of kResolveModel, the
// equipment change among them.
[[nodiscard]] bool equipment_resolve(std::uint32_t return_address);
// True when this return address is the resident file load (0x08866E10).
// 0x088DB4C8 jumps there. Returning a chosen model from that call faulted
// the reader at 0x249D5510. It keeps the model 0x088D6388 computes.
[[nodiscard]] bool file_load_resolve(std::uint32_t return_address);
// True for the equipment change's call of 0x088516A4 (return 0x09A7E360).
// The other callers of that lookup keep the game's answer.
[[nodiscard]] bool equipment_sex_lookup(std::uint32_t return_address);
// True for the call of 0x08851A68 whose result the equipment change stores
// and then passes to 0x08866E10 as t0 (return 0x09A7DF70). a1 is the part.
// Watched while layered armor was off: that read loaded t0 unchanged.
[[nodiscard]] bool equipment_part_model(std::uint32_t return_address);
// True for the equipment change's call of 0x08851CC8 (return 0x09A7DF48).
// v0 is copied to t3 of 0x08866E10. t3 == 1 selects the armor files. An empty
// head is the only part whose other file (0x089B6644) is not -1, so t3 == 0
// opens the hair and the helm model is never read.
[[nodiscard]] bool equipment_part_present(std::uint32_t return_address);
// True for the equipment change's entry of 0x08866E10 (return 0x09A7E548).
// a3 is the part, a2 the sex and t0 the model. The character load enters the
// same function with return 0x088DBCCC, and t0 is already the model from
// 0x0885187C. Replacing t0 there faulted the reader at 0x24AB8540.
[[nodiscard]] bool equipment_model_read(std::uint32_t return_address);
// True when this call of kResolveModel should return the chosen model.
// The lobby equipment calls do. The resident file load does not.
[[nodiscard]] bool substitute_resolved_model(std::uint32_t return_address);

// The armor parts in the kind byte's order: chest, arms, waist, legs, head.
// The lookup's part argument uses a different numbering; armor_kind maps it.
inline constexpr std::uint8_t kParts = 5u;
// Armor ids in a hunter record, in that kind order.
inline constexpr std::uint32_t kRecordArmorId[kParts] = {0x56u, 0x62u, 0x6Eu, 0x3Eu, 0x4Au};
// The kind byte the equipment box stores for each part, in that kind order.
// The game numbers armor legs 0, head 1, chest 2, arms 3, waist 4, and its
// weapons 5 (blademaster) and 6 (gunner). 0x0885C83C picks the armor table
// by that byte through the jump table at 0x089315AC, and 0x08901788 turns
// its equipment classes into it through the bytes at 0x089B9260. Comparing
// a box slot with the part number itself listed another part's pieces under
// this part's names, and missed the pieces of this part.
inline constexpr std::uint8_t kBoxKind[kParts] = {2u, 3u, 4u, 0u, 1u};
[[nodiscard]] const char *part_label(std::uint8_t part);
// The kind (0 to 4) the lookup's part argument draws, or nothing for a part
// that is not armor (part 1, and anything past the head).
[[nodiscard]] std::optional<std::uint8_t> armor_kind(std::uint32_t part);
// The hunter played here, from its record: 0 male, 1 female. Nothing on the
// title screen, where the block pointer is clear. This is what the model
// lookup uses.
[[nodiscard]] std::optional<std::uint8_t> played_sex(const Ram &ram);
// The armor id that record wears in a part (kind order), or nothing when no
// hunter is in play.
[[nodiscard]] std::optional<std::uint16_t> played_armor(const Ram &ram, std::uint8_t part);
// The five worn ids, in kind order, once a hunter record can be read.
using WornIds = std::array<std::uint16_t, kParts>;
[[nodiscard]] std::optional<WornIds> worn_ids(const Ram &ram);
// The worn set last compared with the one on the hunter.
struct WornSet {
    bool known{};
    WornIds ids{};
};
// True when `now` is a different set from the one stored. The first set, and
// a frame with no hunter, are not a change of equipment. A change is stored,
// so the same set the next time is not another one. The caller records it
// and does not release the part files: the equipment change is already
// reading the model, and releasing the file object on that flip faults in
// 0x0886005C.
bool note_worn(WornSet &stored, const std::optional<WornIds> &now);

// The choice for each part: kReal (the piece worn), 0 (nothing: the bare part,
// or the inner wear where the game shows it) or an armor piece's id.
inline constexpr std::int32_t kReal = -1;
using Pieces = std::array<std::int32_t, kParts>;
// The armor id 0x088516A4 should look up, when the switch is on. Nothing
// leaves the game's id in a3: a part that is not armor, a part left real,
// or a piece this sex cannot wear and this hunter does not own. `held` is
// an id that just left the worn slot. The caller's sex is tried first; the
// hunter's own sex is used when that byte does not name a sex the piece fits.
[[nodiscard]] std::optional<std::uint16_t> sex_lookup_id(const Ram &ram, std::uint32_t part, std::uint32_t sex,
                                                         const Pieces &pieces, std::uint16_t held);

// The model number the game is to draw for `part` of hunter record `index`,
// looked up in block `block` (a0 of the game's lookup), when layered armor
// changes it. Nothing leaves the part to the game: another hunter's record, a
// part left real, a piece this hunter's sex cannot wear, a block or table not
// where they were traced.
[[nodiscard]] std::optional<std::uint16_t> replacement_model(const Ram &ram, std::uint32_t block, std::uint32_t index,
                                                             std::uint32_t part, const Pieces &pieces);
// 0x088516A4 asks for the same model by sex (0 male, otherwise female) and
// part, with no hunter record. The guild and the lobby draw a hunter this way.
// `held` is an id that just left the worn slot and is not in the box yet; it
// still counts as owned, so changing equipment does not drop the chosen piece.
[[nodiscard]] std::optional<std::uint16_t> replacement_model_by_sex(const Ram &ram, std::uint32_t part,
                                                                    std::uint32_t sex, const Pieces &pieces,
                                                                    std::uint16_t held = 0u);
// The chosen piece's model for this sex, without asking the equipment box.
// Changing equipment rewrites that box before the reload asks what to draw,
// so a box check there answers with the piece just put on. A part left real,
// and a piece this sex cannot wear, are left to the game.
[[nodiscard]] std::optional<std::uint16_t> chosen_model(const Ram &ram, std::uint32_t part, std::uint32_t sex,
                                                         const Pieces &pieces);
// True when that call's result is a model the file reader loads. Every armor
// part: the equipment change uses the result as the model whether or not t0
// is 1. Part 1 returns the id, and is left to the game.
[[nodiscard]] bool sex_lookup_returns_model(std::uint32_t part);
// What 0x08851C3C should report for the file load. False is no armor (the bare
// part, the inner wear, or the hair). True is armor, so a chosen helm is read
// from the helm files even when the real head is empty. Nothing leaves the
// question to the game: a real part, or a piece this sex cannot wear.
[[nodiscard]] std::optional<bool> armor_present(const Ram &ram, std::uint32_t block, std::uint32_t index,
                                                 std::uint32_t part, const Pieces &pieces);

// A piece the choice screen offers.
struct Offer {
    std::uint16_t id{};
    std::string name;
    bool worn{};   // the loaded hunter wears it in that part
    bool owned{};  // worn, or in the equipment box
    // Worn once, plus one for each used box slot. A second copy changes this.
    std::uint16_t copies{1};
};
// True when the hunter played here wears this piece, or keeps a used slot of
// it in the equipment box. Id 0 is not a piece. A piece this hunter does not
// own is not drawn as layered armor.
[[nodiscard]] bool owns_piece(const Ram &ram, std::uint8_t part, std::uint16_t id);
// How many of this part the loaded hunter wears or keeps, copies included.
// 0 while no character is loaded.
[[nodiscard]] std::uint32_t owned_count(const Ram &ram, std::uint8_t part);
// The pieces of a part this hunter can be shown in: those wearable by its sex
// that it wears or keeps in the equipment box, with the game's names, in id
// order. Empty while no character is loaded.
[[nodiscard]] std::vector<Offer> offers(const Ram &ram, std::uint8_t part);
// The name of a choice for a part, for the menu: "Real equipment", "Nothing",
// the piece's name from the game, or its id while the names cannot be read.
[[nodiscard]] std::string choice_name(const Ram *ram, std::uint8_t part, std::int32_t choice);

// Showing a change. Watched with MHFU_TRACE_LAYERED_ARMOR=1. Arriving in the
// village, cont_task calls 0x088BC7F0 (from 0x09A5F53C, then 0x09A5F640). That
// dispatches +12 through the table at 0x08932378, and 2 is the model load:
// 0x088BCBFC calls 0x088C0E58 (return 0x088BCC04), which calls the file load
// 0x088DBBBC (return 0x088C1088). The file load asks the model lookup. Once
// lobby_task replaces cont_task, that task stops. Departing on a quest
// installs game_task and does not call 0x088BC7F0 or 0x088C0E58. lobby_task
// and game_task call the file load (0x088DBBBC, and 0x088DBEE0 which steps
// it). Once a part's word at file_object+0x24+part*4 reaches 8, that call
// returns without reading a file, so a later area or quest keeps the meshes.
// 0x088DAF18 zeros those words. The port does not: releasing them from the
// display flip made the equipment box's next read fault at 0x249D5510. That
// box change is the reload. It calls 0x088516A4 and passes the model to
// 0x088DB4C8, which jumps to 0x08866E10 with the model in t0. The port does
// not set +0x998 back to 0: state 0 of 0x088C0E58 calls
// 0x088DDBD8, which drops the face and still does not re-read a finished
// file. The port does not start the load. 0x0890C41C walks the task list by
// a jump inside its own unit, so a wrapper there never runs. Stepping the
// load from the calls that do enter that list (0x0890DEBC, 0x0890DF34)
// cleared the pointer at 0x09C190D0, and embarking stopped at 0x09B62B80.
// Calling the load from the display flip replaces the model root. +0x998
// starts at 0, each step adds one, and the step reports done only on the
// call that reaches 5.
inline constexpr std::uint32_t kHunterTask = 0x088BC7F0u;
inline constexpr std::uint32_t kLoadStep = 0x088C0E58u;
inline constexpr std::uint32_t kLoadState = 0x998u;
inline constexpr std::uint32_t kLoadCleared = 0x99Cu;  // cleared with the state
inline constexpr std::uint32_t kLoadDone = 5u;
// The hunter task cont_task passes to 0x088BC7F0: load32(0x09A40000 + 19872).
inline constexpr std::uint32_t kHunterObjectPointer = 0x09A44DA0u;
inline constexpr std::uint32_t kTaskState = 12u;  // index into the table at 0x08932378
inline constexpr std::uint32_t kTaskLoadModels = 2u;  // 0x088BCBFC, the model load
// The file load's object, stored by 0x088DAF18: lui 0x09A5; sw at -0xDFC.
// A part's progress is the word at +0x24 + part * 4. At 8 the file load
// returns without reading. Six parts, the walk in 0x088DBE80. +0x3C is the
// index 0x088DBEE0 steps; at 4 or more that pump returns without reading.
inline constexpr std::uint32_t kFileObjectPointer = 0x09A4F204u;
inline constexpr std::uint32_t kFilePartState = 0x24u;
inline constexpr std::uint32_t kFileParts = 6u;
inline constexpr std::uint32_t kFilePump = 0x3Cu;
// The model object 0x088C0E58 and the quest pass to 0x088DDF10: lui 0x09A5,
// lw at -0xDF0. Bit 2 of the halfword at +0x54 makes the quest skip the pump.
inline constexpr std::uint32_t kModelObjectPointer = 0x09A4F210u;
inline constexpr std::uint32_t kModelLoaded = 0x54u;
inline constexpr std::uint16_t kModelLoadedBit = 2u;
// lobby_task reads load32(0x08A62D7C) and, when the byte at +0x800A4 is 0,
// calls the same pump. Nonzero leaves the meshes it already has.
inline constexpr std::uint32_t kArchivePointer = 0x08A62D7Cu;
inline constexpr std::uint32_t kArchiveNeedLoad = 0x800A4u;

// Whether `object` is the hunter load this step runs for. The step is called
// for that load only; an address that cannot hold the state is not it.
[[nodiscard]] bool own_hunter(const Ram &ram, std::uint32_t object);
// Whether the hunter's models are loaded (no load running).
[[nodiscard]] bool load_done(const Ram &ram, std::uint32_t object);
// Starts the hunter's load over, as the game's own request does once a load
// is done.
void restart_load(Ram &ram, std::uint32_t object);
// The hunter on screen, when its models are already loaded. Nothing on the
// title screen, or while a load is running.
[[nodiscard]] std::optional<std::uint32_t> loaded_hunter(const Ram &ram);
// Clears the finished-file marks so a later file load can read again. The
// display flip does not call this: the equipment box change reads the file
// itself, and clearing the marks on the flip faulted that read. The hunter's
// task word and +0x998 stay as they are. False when there is no loaded
// hunter, or the file object is not there yet.
bool request_model_load(Ram &ram);
// The same request for a hunter object already in hand. False when `object`
// is not a hunter whose models are loaded, or the files cannot be released.
bool arm_model_load(Ram &ram, std::uint32_t object);

// Called at every flip, between two frames: puts the replacement in place
// when the setting is on, and gives the game its own lookup back when it is
// turned off. Off from the start, it never registers anything.
void frame(psprecomp::Runtime &runtime);

// Whether the replacement is in place, for the menu.
[[nodiscard]] bool installed();

} // namespace layered
} // namespace mhfu::game
