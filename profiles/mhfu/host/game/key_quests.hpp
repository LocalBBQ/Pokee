#pragma once

// Key quests: the quests that count towards a level's Urgent Quest are named
// "[Key] " and their name, in the quest lists and wherever else the game
// shows a quest's name. Off by default (`game.key_quests`,
// `MHFU_KEY_QUESTS=1` turns it on).
//
// The executable (ULUS-10391) reads the quests it lists from quest packs,
// DATA.BIN entries 5342 to 5352, one or two lists of a level each (5345 holds
// the Village Chief's 4★ quests, ids 10301 to 10319, and a 4★ list of ids 1101
// to 1126). The game loads a pack unchanged into a buffer of its own; RAM
// captures with the Village Chief's 4★ list open had entry 5345 at 0x0977D710,
// byte for byte, in a 0x20000-byte buffer. A pack, all offsets from its start:
//
//   +0          one word per quest: the offset of its record. The first word
//               is also the size of this list.
//   record +3   flags. Bit 0x04 is set on 3 to 6 quests of every Village
//               Chief, Nekoht and Guild list from 3★, and on no training,
//               treasure or event quest; those are the key quests (checked
//               against the Village Chief's 4★ keys in the game). Bit 0x02 is
//               on the list's Urgent Quest.
//   record +0x14 the offset of 7 words, one per language, each the offset of
//               that language's strings. English is in three of them.
//   strings +0  the offset of the quest's name, then of its goal, failure
//               conditions, description, monsters and client.
//
// The list reads a name through those offsets each time it draws, so pointing
// a key quest's name offsets at a tagged copy renames it on the next draw;
// tried live, the list showed the new names after a scroll.
//
// When the file I/O has served a whole pack (file_loaded), the next frames
// look for that pack in guest RAM, byte for byte as the game got it, and point
// the names of its key quests at "[Key] " copies in a block of guest memory of
// our own. Each word is checked before it is written; turning the switch off
// puts every word back that still holds our offset. Turning it on looks once
// for packs already loaded. Off from the start, nothing runs.

#include "psprecomp/guest_memory.hpp"

#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace mhfu::game::key_quests {

inline constexpr std::uint32_t kFirstPack = 5342u;
inline constexpr std::uint32_t kLastPack = 5352u;
inline constexpr std::uint32_t kFlags = 3u;
inline constexpr std::uint8_t kKeyBit = 0x04u;
inline constexpr std::uint32_t kLanguagesWord = 0x14u;
inline constexpr std::uint32_t kLanguages = 7u;
inline constexpr std::uint32_t kMaxQuests = 256u;
inline constexpr char kTag[] = "[Key] ";
// Each pack's tagged names get a slot this size in the block.
inline constexpr std::uint32_t kSlot = 2048u;
inline constexpr std::uint32_t kBlockSize = (kLastPack - kFirstPack + 1u) * kSlot;
// Frames to look for a pack after it was read before giving up.
inline constexpr int kAttempts = 3;

[[nodiscard]] constexpr bool is_pack(std::uint32_t entry) noexcept {
    return entry >= kFirstPack && entry <= kLastPack;
}

// A key quest's name in one language's strings.
struct Name {
    std::uint32_t word{}; // offset of the word that holds the name's offset
    std::uint32_t name{}; // the name's offset
    std::string text;     // the name, as stored (UTF-8)
};

// The names of a pack's key quests, one per distinct language table. None
// when the bytes are not a pack as traced.
[[nodiscard]] std::optional<std::vector<Name>> key_names(std::span<const std::uint8_t> pack);

// Where in `ram` (guest address `ram_base`) an exact copy of `pack` starts,
// looking from guest address `from`. None when there is none.
[[nodiscard]] std::optional<std::uint32_t> find_pack(std::span<const std::uint8_t> ram, std::uint32_t ram_base,
    std::span<const std::uint8_t> pack, std::uint32_t from = 0u);

// A word changed in a loaded pack.
struct Patch {
    std::uint32_t address{};
    std::uint32_t original{};
    std::uint32_t replacement{};
};

// Writes "[Key] " and each name into `slot` (guest address, `capacity`
// bytes) and points the names of the pack loaded at `base` there. A word that
// no longer holds the name's offset, or a name that does not fit, is left.
std::vector<Patch> tag(psprecomp::GuestMemory &memory, std::uint32_t base, const std::vector<Name> &names,
    std::uint32_t slot, std::uint32_t capacity);

// Puts back each word that still holds our offset. Returns how many.
std::size_t untag(psprecomp::GuestMemory &memory, const std::vector<Patch> &patches);

// From the file I/O: the game has read the whole of `entry`.
void file_loaded(std::uint32_t entry);

using Allocate = std::uint32_t (*)(std::uint32_t size);
using EntryBytes = std::vector<std::uint8_t> (*)(std::uint32_t entry);

// At the flip: follows the switch, and tags the packs read since.
void frame(psprecomp::GuestMemory &memory, Allocate allocate, EntryBytes bytes);

// Forgets everything, as at start; for tests.
void reset();

} // namespace mhfu::game::key_quests
