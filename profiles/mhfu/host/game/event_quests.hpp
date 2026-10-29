#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace psprecomp {
class GuestMemory;
class Runtime;
struct AllegrexContext;
} // namespace psprecomp

// Built-in event quests: the Guild Hall lists quests that ship with Pokee's
// data directory after the ones in the save, and starts them as its own.
//
// USA Freedom Unite keeps downloaded quests in the save. The save object
// (load32(0x09A44DA0)) has the decrypted save at +2380, and quest slot k of
// six is at that buffer + 0x142300 + k * 0x6810: the quest file ("2NDG", its
// info block at the offset its first word gives), and its name ("m60010.mib")
// at +0x6800. A table of six 40-byte rows at the save object + 2480 holds
// each slot's title; a zero first byte is an empty slot. At start
// 0x088C0300 decodes the slots in place and writes the titles. Traced from
// guest RAM with the Guild Hall's event list open.
//
// lobby_task (at 0x09A5F300) reads them through two functions. The getter,
// 0x09A88470(lobby, k), returns slot k's info block, or 0 for an empty slot
// or a quest whose info flags have 0x01000000, and stores the slot's address
// at lobby + 120, from where the quest is later taken. The lookup,
// 0x09A88500(lobby, id), tries slots 0 to 5 for the quest with that id (the
// halfword at info + 24). The event list is built by a loop over the six slots
// (0x09A87250) that writes each id to the list at lobby + 2048, keeping its
// write position in s2 and the count in s4, and stores the count at
// lobby + 2246.
//
// Both functions are replaced here by host functions that do what the game's
// do and go on past the six slots: the getter, on the list loop's call for
// the last slot, adds the built-in quests' ids to the list through the loop's
// own s2 and s4, so the count it stores includes them; the lookup finds them
// after the save's. The save is never written to. Calls to the two functions
// from inside their own generated unit are local jumps unless lobby_task's
// library dispatches them (config/overlay_dispatch.txt), so nothing is
// replaced with a library built without that.
//
// The Training School (arcade_task, also at 0x09A5F300) lists the save's
// challenges, the quests with that flag, the other way round: its loop
// (0x09A63C80) asks 0x09A6E2B8(arcade, k) for slot k's info block, which is
// 0 unless the flag is set, and appends the quest's id to arcade + 376
// (halfwords), the info block to + 420 and the slot to + 476 (words), one
// more each time, counting at the halfword + 416. The arrays hold 14 before
// + 532. A chosen challenge's slot is copied, 0x6800 bytes, into the quest
// buffer. The loop's call crosses generated units, so the getter's
// replacement is reached without a dispatch list; on its call for the last
// slot it appends the built-in challenges to the three arrays, leaving room
// for that slot's own.
//
// The quests are the files m<id>.mib in event_quests/ of the data directory,
// decoded. A quest whose id the save already holds is the save's.
namespace mhfu::game::event_quests {

inline constexpr std::uint32_t kSaveObject = 0x09A44DA0u;
inline constexpr std::uint32_t kSaveBuffer = 2380u;
inline constexpr std::uint32_t kTitles = 2480u;
inline constexpr std::uint32_t kTitleSize = 40u;
inline constexpr std::uint32_t kSlots = 0x142300u;
inline constexpr std::uint32_t kSlotSize = 0x6810u;
inline constexpr std::uint32_t kSlotCount = 6u;
inline constexpr std::uint32_t kQuestBytes = 0x6800u; // a slot's file; its name follows
inline constexpr std::uint32_t kHiddenFlag = 0x01000000u;
inline constexpr std::uint32_t kInfoId = 24u;
// The offset of the quest's text: seven language tables (English first), each
// a list of offsets to its strings, the title first. FUComplete's quests point
// all seven at their English one.
inline constexpr std::uint32_t kInfoText = 20u;

inline constexpr std::uint32_t kLobbySlot = 0x09A5F300u;
inline constexpr std::uint64_t kLobbyHash = 0xF431CAA134157E88ull;
inline constexpr std::uint32_t kGetter = 0x09A88470u;
inline constexpr std::uint32_t kLookup = 0x09A88500u;
inline constexpr std::uint32_t kListLoopReturn = 0x09A87268u; // after the list loop's call of the getter
inline constexpr std::uint32_t kChosenSlot = 120u;
inline constexpr std::uint32_t kList = 2048u;
inline constexpr std::uint32_t kListCount = 2246u;
inline constexpr std::uint32_t kListRoom = (kListCount - kList) / 2u;

inline constexpr std::uint32_t kChallengeGetter = 0x09A6E2B8u;
inline constexpr std::uint32_t kChallengeLoopReturn = 0x09A63C94u;
inline constexpr std::uint32_t kChallengeIds = 376u;
inline constexpr std::uint32_t kChallengeCount = 416u;
inline constexpr std::uint32_t kChallengeInfos = 420u;
inline constexpr std::uint32_t kChallengeSlots = 476u;
inline constexpr std::uint32_t kChallengeRoom = 14u;

struct Quest {
    std::uint16_t id{};
    std::vector<std::uint8_t> bytes; // the decoded file
    std::uint32_t flags{};           // its info block's first word
};

// A decoded quest file: "2NDG" after its info block's offset, the info block
// inside it, and no larger than a slot's file. Nothing otherwise.
[[nodiscard]] std::optional<Quest> parse(std::span<const std::uint8_t> bytes);

// The quest's English title, or nothing when its text does not lie inside it.
[[nodiscard]] std::string title(std::span<const std::uint8_t> bytes);

// The quests in `directory`: m<id>.mib files that parse, by id, without a
// second quest of the same id.
[[nodiscard]] std::vector<Quest> load(const std::filesystem::path &directory);

// The built-in quests as guest memory holds them: where each slot starts.
struct Bank {
    std::vector<std::uint32_t> slots;
};

// Writes `quests` into the block at `base`, one slot each, `stride` apart
// (kSlotSize, with the name after the file, or less), and returns their slots.
Bank place(psprecomp::GuestMemory &memory, std::uint32_t base, std::uint32_t stride, std::span<const Quest> quests);

// The game's getter for save slot k, as 0x09A88470 does it.
[[nodiscard]] std::uint32_t save_slot_info(psprecomp::GuestMemory &memory, std::uint32_t lobby, std::uint32_t k);
// A built-in slot's info block, storing the slot at lobby + 120 as the getter
// does; 0 for a hidden quest.
[[nodiscard]] std::uint32_t bank_slot_info(psprecomp::GuestMemory &memory, std::uint32_t lobby, std::uint32_t slot);
// The lookup: the save's slots first, then the bank's.
[[nodiscard]] std::uint32_t find(
    psprecomp::GuestMemory &memory, const Bank &bank, std::uint32_t lobby, std::uint16_t id);
// The list loop's call for the last slot: adds the bank's ids the list does
// not hold yet after its `count` entries at `write` (the loop's s2 + 2048),
// leaving room for the last slot's own. Returns how many it added.
std::uint32_t extend_list(psprecomp::GuestMemory &memory, const Bank &bank, std::uint32_t lobby,
    std::uint32_t write, std::uint32_t count, std::uint32_t last_slot_info);

// The Training School's getter for save slot k, as 0x09A6E2B8 does it: the
// info block of a challenge only.
[[nodiscard]] std::uint32_t save_challenge_info(psprecomp::GuestMemory &memory, std::uint32_t k);
// The loop's call for the last slot: appends the bank's challenges the list
// does not hold yet to the arcade's arrays, leaving room for the last slot's
// own (`last_slot_info`, 0 for none). Returns how many it added.
std::uint32_t extend_challenges(
    psprecomp::GuestMemory &memory, const Bank &bank, std::uint32_t arcade, std::uint32_t last_slot_info);
// Whether arcade_task's list loop and getter are those this was read from.
[[nodiscard]] bool training_school_matches(const psprecomp::GuestMemory &memory);

// Once a frame: loads the quests and places them the first time lobby_task
// or arcade_task is in, and keeps the replacements registered while it is.
using BlockAllocator = std::uint32_t (*)(std::uint32_t size);
void frame(psprecomp::Runtime &runtime, BlockAllocator allocate);
// Whether frame() has read event_quests/ this run, so a quest added now is
// listed only from the next start.
[[nodiscard]] bool loaded();

} // namespace mhfu::game::event_quests
