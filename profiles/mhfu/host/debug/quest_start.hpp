#pragma once

// Starting a quest from the developer tools: the village and Guild Hall
// quests the game has, read from its own quest lists, and a departure made the
// way the game makes one when the hunter presses the button at the village
// gate or the Hall's.
//
// The accepted quest and the stage it loads live in one quest-state block.
// A word at 0x089CC558 holds that block (0x09999DA0 while the village was
// loaded). The stage number is a u16 at +0x6AF0E and the quest id is a u16 at
// +0x6AF12. In the village those read 139 and 1, and the stage slot held
// stage139: the file is DATA.BIN entry 5542 plus the stage number. Accepting
// copies the quest record's id (u16 at +0x18) into +0x6AF12 when it is below
// 60000 (lobby_task at 0x09A886EC). The executable writes 1 there when there
// is no quest (0x08851030). The quest lists themselves are the u16 table at
// 0x09B04A6A (5342 through 5352); 5353 is the halfword before it.
//
// Leaving the village is the task object, not those two stores. While
// lobby_task is loaded, *(0x09B22E70) is that object (0x09B100C0 on both
// village boots). Lobby 0x09A960B8 stores 36 at +0, a DATA.BIN file id at +4
// (71 is game_task) and 0 in the bytes at +20..+23. The same object is then
// cleared while a word at +756 is 0. That word belongs to
// *(*(0x09A4F204) + 12 + index * 4), and the index is the byte at
// quest_state+40, which was 0. *(0x09A4F204) was 0x09A4F1BC and the object
// was the heap pointer 0x090B3580, whose +756 read 0. Pointing +756 at the
// zeroed stub 0x09B23000 kept 36, 71 and phase 1. The phase-1 countdown at
// task+52 advanced, and 0x09A9F1C0 did not return 0 or 3, so the file-71
// handler at 0x09A96604 (the table at 0x09B0EDD0) never ran. Phase 0 with
// the stub's byte at +1152 set to 32 did run: task+0 became 3, task+8
// became 1 and task+12 a heap pointer, the state byte stayed 32, and
// game_task was still not installed. Bit 0x4000 at that object's +1156,
// which the other dispatcher checks before it calls 0x09A9BCA8, was stored
// and then read back as 0. The constants below are still the Portable 3rd
// departure, and `start` still uses them.
//
// Pure functions over a Ram and over file bytes, so the unit tests run them
// on buffers. Nothing here writes on its own: debug_tools.cpp calls these at
// the flip.

#include <cstdint>
#include <functional>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace mhfu::game {
class Ram;
}

namespace mhfu::debug::quests {

using game::Ram;

// Where a quest is taken: the Yukumo Chief in the village, or the Gathering
// Hall's counter.
enum class Board : std::uint8_t { Village, Hall };

// One quest from the game's quest lists.
struct Quest {
    std::uint16_t id{};
    std::uint8_t stars{};
    std::uint32_t fee{};        // zenny the counter takes when it is accepted
    std::uint32_t reward{};     // zenny for completing it
    std::uint32_t time_limit{}; // frames at 30 a second
    std::string name;
    std::string objective;
    std::string monsters; // the "Main Monster" lines, one per line
    std::string client;
    std::uint16_t map{}; // the place id at +0x1C; gathering quests name it

    [[nodiscard]] Board board() const;
};

// The game's quest lists in DATA.BIN, one per rank: entries 5342 to 5352.
// Two further lists follow. 5353 repeats quests the earlier lists already
// hold, and 5354 holds ids from 20000, which are not started here.
inline constexpr std::uint32_t kFirstQuestList = 5342u;
inline constexpr std::uint32_t kQuestLists = 11u;

// A quest list: 32-bit offsets to the records, ended by 0. The first offset is
// the byte size of that table. Each record has the fee at +4, the reward at
// +8, the time limit in frames at +0x10, the id (u16) at +0x18 and a map id
// (u16) at +0x1C. From +0x34 come the name, the objective, the failure
// conditions, the description, the main monsters and the client, each ended by
// a zero byte; the same six follow again in the other languages and are not
// read. Records that do not look like quests are left out; an empty result
// means the bytes are not a quest list.
[[nodiscard]] std::vector<Quest> parse_quest_list(std::span<const std::uint8_t> file);

// The main monsters on one line: "Jaggi, Gargwa".
[[nodiscard]] std::string monster_list(const Quest &quest);

// Ids below 10000 are the rank lists' own quests. Ids from 10000 to 19999
// sit in those same lists beside them. Ids from 20000 are the list this tool
// does not start.
[[nodiscard]] bool village_quest(std::uint16_t id);
[[nodiscard]] bool hall_quest(std::uint16_t id);

// The village and Hall quests of every level: the village's by star level,
// then the Hall's by star level, each as the lists order them. Event and
// training quests are not in these lists. `read_entry` returns a DATA.BIN
// entry's plain bytes, or nothing.
[[nodiscard]] std::vector<Quest> board_quests(
    const std::function<std::optional<std::vector<std::uint8_t>>(std::uint32_t entry)> &read_entry);

// The game's memory ------------------------------------------------------------

// Portable 3rd's character, quest id and gate. Freedom Unite does not use
// them. Its quest id is the u16 at *(0x089CC558) + 0x6AF12, its stage number
// is the u16 at +0x6AF0E, and the leave-the-village request is the task
// object described above. These constants remain until that request actually
// installs game_task.
inline constexpr std::uint32_t kCharacterPointer = 0x08AB3640u;
inline constexpr std::uint32_t kQuestIdOffset = 0x60472u;
inline constexpr std::uint32_t kGateOffset = 0x60478u;
inline constexpr std::uint32_t kGateFlagOffset = 0x60479u;
inline constexpr std::uint8_t kVillageGate = 2u;
inline constexpr std::uint8_t kHallGate = 3u;

// The map overlay in its slot names where the hunter is: P_v00a00 the village,
// P_v00a01 the Guild Hall (a02 to a04 the other village areas).
inline constexpr std::uint32_t kMapSlot = 0x0A055E80u;

// The next scene: a pointer at 0x0A25DD28 to a word the village and the Hall
// set while they run (0 while the hunter walks around, other values while a
// menu or a dialog is open and when leaving), and
// 30 when the scene that ends is to become a quest. A word at +0x28 is set to
// -1 with it.
inline constexpr std::uint32_t kNextScenePointer = 0x0A25DD28u;
inline constexpr std::uint32_t kSceneQuest = 30u;
inline constexpr std::uint32_t kSceneWalking = 0u;

// The village scene object (pointer at 0x08ABAE14): bit 4 of its flags at
// +0x20 asks it to end, which it does over the next frames, switching to the
// scene set above.
inline constexpr std::uint32_t kScenePointer = 0x08ABAE14u;
inline constexpr std::uint32_t kSceneFlags = 0x20u;
inline constexpr std::uint32_t kSceneEnd = 4u;

// The flag the game copies beside the gate: bit 0 of a word it reaches through
// the pointer at 0x09FC8BE8, plus 0x01500000 - 0x57DC.
inline constexpr std::uint32_t kStatePointer = 0x09FC8BE8u;
inline constexpr std::uint32_t kGateFlagSource = 0x01500000u - 0x57DCu;

// Why a quest cannot be started now, or "" when it can: the hunter must be in
// the village or the Guild Hall (not on a quest, not in the house, on the
// farm or at the hot spring), walking around with no menu or dialog open, and
// not already leaving.
[[nodiscard]] std::string start_blocked(const Ram &ram);

// Starts `quest` the way its own gate does: takes the counter's fee, sets the
// quest's id, and leaves by the village gate for a village quest or by the
// Hall's for a Hall quest, from either place: the hunter comes back where the
// quest belongs. Returns the line to log. Refused (and nothing written) when
// start_blocked says so, when the quest is neither a village nor a Hall quest,
// or when the hunter cannot pay the fee.
std::string start(Ram &ram, const Quest &quest);

} // namespace mhfu::debug::quests
