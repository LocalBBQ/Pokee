#pragma once

// Item sets: the item box menu in the village and the Guild Hall gets a
// seventh entry, Item Sets, after Sell Item. It opens a
// screen of eight sets per hunter: register the pouch as a set, and load a
// set, which puts the pouch into the box and takes the set's items out of it
// into the same slots, as the later games do. On by default (Mods page,
// `game.item_sets`; `MHFU_ITEM_SETS=0` turns it off). `MHFU_TRACE_ITEM_SETS=1`
// prints each step.
//
// Read off the USA executable (ULUS-10391) and its lobby_task overlay:
//
// The box. GameSys (the word at 0x089CC558) + 0x3428 holds 1000 slots of a
// u16 item id and a u16 count, straight after the equipment box; lobby_task
// copies it with memcpy(dst, GameSys + 0x3428, 4000) (0x09AA6A34). Only the
// first 100 * (8 + pages) slots are in use: 0x08850EC4 counts the pages
// (0x08850E58), one for each of items 829 and 830 the hunter has obtained.
// Obtained items are bits of GameSys + 0x6AC54 + (id >> 5) * 4, bit id & 31,
// for ids below 1261 (0x08857C98 tests one, 0x08857CF0 sets one).
//
// Putting an item in (0x08853ABC(GameSys, id, count, slot or 0xFFFF)) tops up
// the stacks of that item first and then fills empty slots, in slot order.
// A stack holds 99, or 255 when byte 3 of the item's 24-byte record in the
// table at 0x0899D2B8 is 255. Every change marks the item obtained
// (0x0885ACA0). The same byte 3 is what the pouch holds of the item
// (0x08856920: room is byte 3 minus the count).
//
// The pouch is 24 slots of the same shape at GameSys + 0x6AEA6
// (game/empty_item_slot.hpp). The hunter's name is UTF-16 at GameSys + 0x4A0,
// eight characters at most; the sets are kept by that name.
//
// The menu. lobby_task opens the box menu each frame in 0x09AE66F0: it copies
// the menu record at 0x09B0B0A8 with 0x0882FA24 and passes it to 0x0882F580.
// The record's u16 at +10 is the number of rows (6) and the word at +12 the
// list of their text ids, ended by 0: 294 to 299 at 0x09B05158, two padding
// halfwords, then the Small Item Box's 300 to 305 at 0x09B05168, which
// 0x09AE66F0 puts in the copy instead when the box record's byte +52 is set.
// The menu draws a row's text with 0x09A7B890(text, id) (from 0x0882F880).
//
// The box record is a lobby_task object: byte +54 is the chosen row and
// +56 the phase, 0 while the menu shows. 0x0881C2E4(?, &row, buttons, rows)
// moves the row, called with 6 rows from 0x09ADCE0C. Confirming a row
// raises the phase to 1 (0x09ADCEB4) and starts rows 0 to 5 through a table
// (0x09ADCF68: sltiu 6); the box's update 0x09AE38F8 then runs the row's
// screen through another (0x09AE3A20: sltiu 6). A row of 6 starts nothing
// and the update does nothing for it, so the box waits there.
//
// The changes: the record's row count becomes 7 and both id lists get the
// id kText in their first padding halfword; the cursor call is answered with
// 7 rows; the text call answers kText with "Item Sets", kept at kLabel in
// the window the host reserves below the load image (kernel/kernel.hpp).
// While the box waits on row 6, the update is not run and the screen is
// asked for; once the screen closes, the phase goes back to 0 and the box
// shows its menu again, on Item Sets.

#include "psprecomp/guest_memory.hpp"

#include <array>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace psprecomp {
class Runtime;
}

namespace mhfu::game {
class Ram;
}

namespace mhfu::game::item_sets {

// GameSys and what it holds.
inline constexpr std::uint32_t kGameSysPointer = 0x089CC558u;
inline constexpr std::uint32_t kBox = 0x3428u;
inline constexpr std::uint32_t kBoxSlots = 1000u;
inline constexpr std::uint32_t kPouch = 0x6AEA6u;
inline constexpr std::uint32_t kPouchSlots = 24u;
inline constexpr std::uint32_t kName = 0x4A0u;
inline constexpr std::uint32_t kNameChars = 8u;
inline constexpr std::uint32_t kObtained = 0x6AC54u;
inline constexpr std::uint16_t kFlagIds = 1261u;
inline constexpr std::uint16_t kPageItems[] = {829u, 830u};
inline constexpr std::uint32_t kBasePages = 8u;
inline constexpr std::uint32_t kPageSlots = 100u;
// The item table.
inline constexpr std::uint32_t kItemTable = 0x0899D2B8u;
inline constexpr std::uint32_t kItemRecord = 24u;
inline constexpr std::uint32_t kCarryByte = 3u;
inline constexpr std::uint16_t kStack = 99u;
inline constexpr std::uint16_t kLargeStack = 255u;

// The menu and the box record.
inline constexpr std::uint32_t kMenuRecord = 0x09B0B0A8u;
inline constexpr std::uint32_t kBoxList = 0x09B05158u;
inline constexpr std::uint32_t kSmallBoxList = 0x09B05168u;
inline constexpr std::uint16_t kGameRows = 6u;
inline constexpr std::uint16_t kRows = 7u;
inline constexpr std::uint8_t kRow = 6u; // Item Sets, counted from 0
inline constexpr std::uint32_t kText = 0x09A7B890u;
inline constexpr std::uint32_t kCursor = 0x0881C2E4u;
inline constexpr std::uint32_t kCursorReturn = 0x09ADCE14u;
inline constexpr std::uint32_t kUpdate = 0x09AE38F8u;
inline constexpr std::uint32_t kChosenRow = 54u;
inline constexpr std::uint32_t kPhase = 56u;
// A text id no list of the game's uses, and where its words are kept.
inline constexpr std::uint16_t kTextId = 0x7FF0u;
inline constexpr std::uint32_t kLabel = 0x08803F20u;
inline constexpr char kLabelText[] = "Item Sets";

struct CodeWord {
    std::uint32_t address;
    std::uint32_t word;
};

// The executable's words the hooks were read off, and lobby_task's. A
// mismatch means another executable or overlay, and nothing is changed.
[[nodiscard]] std::span<const CodeWord> code_signature();
[[nodiscard]] std::span<const CodeWord> overlay_signature();
[[nodiscard]] bool matches(const Ram &ram, std::span<const CodeWord> words);

// Box and pouch --------------------------------------------------------------

struct Slot {
    std::uint16_t item{};
    std::uint16_t count{};
    friend bool operator==(const Slot &, const Slot &) = default;
};
using Pouch = std::array<Slot, kPouchSlots>;

// GameSys, or 0 when its block cannot be read up to the pouch.
[[nodiscard]] std::uint32_t game_sys(const Ram &ram);
// The slots the box has now (800 to 1000).
[[nodiscard]] std::uint32_t box_capacity(const Ram &ram, std::uint32_t sys);
[[nodiscard]] bool obtained(const Ram &ram, std::uint32_t sys, std::uint16_t item);
// What one pouch slot and one box stack hold of an item.
[[nodiscard]] std::uint16_t carry_limit(const Ram &ram, std::uint16_t item);
[[nodiscard]] std::uint16_t stack_limit(const Ram &ram, std::uint16_t item);
[[nodiscard]] Pouch read_pouch(const Ram &ram, std::uint32_t sys);
// The hunter's name, UTF-8; empty when there is none.
[[nodiscard]] std::string hunter_name(const Ram &ram, std::uint32_t sys);
// How many of an item the box holds.
[[nodiscard]] std::uint32_t box_count(const Ram &ram, std::uint32_t sys, std::uint16_t item);

// A set ------------------------------------------------------------------------

struct ItemSet {
    std::string name;
    Pouch items{};
    [[nodiscard]] bool empty() const;
};
inline constexpr std::size_t kSets = 8u;
using Sets = std::array<ItemSet, kSets>;

// An item the box could not give in full when a set was loaded.
struct Short {
    std::uint16_t item{};
    std::uint16_t wanted{};
    std::uint16_t taken{};
};
struct Loaded {
    bool done{};          // false: nothing was changed
    bool box_full{};      // the pouch did not fit in the box
    std::vector<Short> short_items;
};

// Puts the pouch into the box and then fills each pouch slot with the set's
// item in that slot, as many as the set says, the box has and the pouch
// holds. All or nothing: when the box cannot take the pouch, nothing moves.
Loaded load(Ram &ram, std::uint32_t sys, const ItemSet &set);

// The sets of every hunter, by name, in a text file.
class Store {
public:
    explicit Store(std::filesystem::path path) : path_(std::move(path)) {}
    // The hunter's sets; empty ones (named "Item Set N") for a new hunter.
    [[nodiscard]] Sets sets(const std::string &hunter);
    void set(const std::string &hunter, std::size_t index, const ItemSet &set);
    // False when the file could not be written.
    [[nodiscard]] bool save();
    [[nodiscard]] const std::filesystem::path &path() const { return path_; }

private:
    void load_file();
    std::filesystem::path path_;
    bool loaded_{};
    std::vector<std::pair<std::string, Sets>> hunters_;
};
[[nodiscard]] std::string default_name(std::size_t index);

// Hooks ----------------------------------------------------------------------

// At the flip: puts the hooks in place once lobby_task is installed. Runs
// every frame; doing it again is a no-op.
void frame(psprecomp::Runtime &runtime);

// The box waiting on Item Sets, for the screen: true once each time it is
// chosen.
[[nodiscard]] bool take_screen_request();
// The screen closed: the box shows its menu again.
void screen_closed();
[[nodiscard]] bool screen_open();

// What the screen reads and changes, for the hunter at the box.
[[nodiscard]] Store &store();

} // namespace mhfu::game::item_sets
