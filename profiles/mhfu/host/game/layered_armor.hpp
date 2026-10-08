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
//   +0x3E, +0x4A, +0x56, +0x62 and +0x6E, but the part argument is not the
//   kind byte: 0 chest, 2 arms, 3 waist, 4 legs, 5 head. Part 1 is not armor.
// - 0x0885187C (a0 the block, a1 the record, a2 the part) returns that part's
//   model: the male or female half of the armor table at the record's id. It
//   only reads. The callers in the executable are what draws a hunter, in the
//   village and on a quest. Defense, skills and the Equipment screen read the
//   tables by id elsewhere, and nothing here writes guest memory, so the save
//   and what the game sends to other players keep the real pieces.
//
// So layered armor replaces that one function while it is on: for record 0
// and a part with a chosen piece it returns the chosen piece's model for the
// record's sex, exactly as the game would for that piece; anything else runs
// the game's own code. It also wraps the hunter's load step so that a changed
// look shows at the game's next load (below). Off from the start, nothing is
// registered and the game runs as built.

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
// The game's model lookup that layered armor stands in for.
inline constexpr std::uint32_t kModelOfPart = 0x0885187Cu;

// The armor parts in the kind byte's order: chest, arms, waist, legs, head.
// The lookup's part argument uses a different numbering; armor_kind maps it.
inline constexpr std::uint8_t kParts = 5u;
[[nodiscard]] const char *part_label(std::uint8_t part);
// The kind (0 to 4) the lookup's part argument draws, or nothing for a part
// that is not armor (part 1, and anything past the head).
[[nodiscard]] std::optional<std::uint8_t> armor_kind(std::uint32_t part);
// The hunter played here, from its record: 0 male, 1 female. Nothing on the
// title screen, where the block pointer is clear. This is what the model
// lookup uses; the character block in game_data.hpp is still the Portable
// 3rd address, so the menu must not wait on it.
[[nodiscard]] std::optional<std::uint8_t> played_sex(const Ram &ram);
// The armor id that record wears in a part (kind order), or nothing when no
// hunter is in play.
[[nodiscard]] std::optional<std::uint16_t> played_armor(const Ram &ram, std::uint8_t part);

// The choice for each part: kReal (the piece worn), 0 (nothing: the bare part,
// or the inner wear where the game shows it) or an armor piece's id.
inline constexpr std::int32_t kReal = -1;
using Pieces = std::array<std::int32_t, kParts>;

// The model number the game is to draw for `part` of hunter record `index`,
// looked up in block `block` (a0 of the game's lookup), when layered armor
// changes it. Nothing leaves the part to the game: another hunter's record, a
// part left real, a piece this hunter's sex cannot wear, a block or table not
// where they were traced.
[[nodiscard]] std::optional<std::uint16_t> replacement_model(const Ram &ram, std::uint32_t block, std::uint32_t index,
                                                             std::uint32_t part, const Pieces &pieces);

// A piece the choice screen offers.
struct Offer {
    std::uint16_t id{};
    std::string name;
    bool worn{};   // the loaded hunter wears it in that part
    bool owned{};  // worn, or in the equipment box
};
// The pieces of a part the loaded hunter can be shown in: those wearable by
// its sex, owned ones only unless `all`, with the game's names, in id order.
// Empty while no character is loaded.
[[nodiscard]] std::vector<Offer> offers(const Ram &ram, std::uint8_t part, bool all);
// The name of a choice for a part, for the menu: "Real equipment", "Nothing",
// the piece's name from the game, or its id while the names cannot be read.
[[nodiscard]] std::string choice_name(const Ram *ram, std::uint8_t part, std::int32_t choice);

// Showing a change. 0x088C0E58 is the hunter's model load, stepped from
// 0x088BCBFC while the game loads something (an area, a quest, a change of
// equipment). The object's +0x998 is the state: 0 starts it, each step adds
// one, and the function reports done only when the state is 5. The game's own
// request to load again (0x088BCBB4) stores 0 at +0x99C and +0x998. So when
// the look has changed since this hunter last loaded, the next step the game
// itself makes, once the load is done, starts it over the same way.
inline constexpr std::uint32_t kLoadStep = 0x088C0E58u;
inline constexpr std::uint32_t kLoadState = 0x998u;
inline constexpr std::uint32_t kLoadCleared = 0x99Cu;  // cleared with the state
inline constexpr std::uint32_t kLoadDone = 5u;

// Whether `object` is the hunter load this step runs for. The step is called
// for that load only; an address that cannot hold the state is not it.
[[nodiscard]] bool own_hunter(const Ram &ram, std::uint32_t object);
// Whether the hunter's models are loaded (no load running).
[[nodiscard]] bool load_done(const Ram &ram, std::uint32_t object);
// Starts the hunter's load over, as the game's own request does once a load
// is done.
void restart_load(Ram &ram, std::uint32_t object);

// Called at every flip, between two frames: puts the replacement in place
// when the setting is on, and gives the game its own lookup back when it is
// turned off. Off from the start, it never registers anything.
void frame(psprecomp::Runtime &runtime);

// Whether the replacement is in place, for the menu.
[[nodiscard]] bool installed();

} // namespace layered
} // namespace mhfu::game
