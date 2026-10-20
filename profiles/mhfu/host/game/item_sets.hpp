#pragma once

// Item sets: the item box menu in the village and the Guild Hall gets a
// seventh entry, Item Sets, after Combine Items. Like the game's Change Equipment
// it opens a small menu, Load Item Set and Register Pouch, and then the
// hunter's 24 sets, eight to a page (L and R turn it), beside the chosen
// one's items, all drawn with the
// game's own windows, text, button icons and sounds (game/item_sets_menu.hpp
// has what the buttons do). Loading a set puts the pouch into the box and
// takes the set's items out of it into the same slots, as the later games
// do. On by default (Mods page, `game.item_sets`; `MHFU_ITEM_SETS=0` turns it
// off). `MHFU_TRACE_ITEM_SETS=1` prints each step.
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
// (game/empty_item_slot.hpp; 0x08856884 gives a slot). The box's screens work
// on a copy of it at GameSys + 0x43C8: 0x088D8900 makes the copy, and
// 0x088D893C puts it back into the pouch after each change (Put In Item's
// Sort among them), so a set is loaded into both. The hunter's name is
// UTF-16 at GameSys + 0x4A0, eight characters at most; the sets are kept by
// that name.
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
// (0x09ADCF68: sltiu 6); the box's update 0x09AE38F8, which also draws it,
// then runs the row's screen through another (0x09AE3A20: sltiu 6). A row of
// 6 starts nothing and the update draws nothing for it, so the box waits
// there.
//
// The changes: the record's row count becomes 7 and both id lists get the
// id kTextId after Combine Items, the rows after it moving into the padding.
// The box still numbers Item Sets 6, so its own rows keep their numbers:
// the box's cursor call (from 0x09ADCE0C) moves over the rows as shown, and
// 0x09AE66F0's call of 0x0882F580 (returning to 0x09AE6784; the row from
// 0x09AE6768) highlights the row where it is shown. The text call answers
// kTextId with "Item Sets", kept at kLabel in the window the host reserves
// below the load image (kernel/kernel.hpp), and the screens' rows with
// theirs. The box's frame,
// 0x09ADCCD0(box), reads the buttons and then runs the update; on a row with
// no screen of its own, its back goes straight to the menu (0x09ADD008).
// So while the box waits on row 6 the frame is the screens': they take the
// buttons, run the update, and draw after it through game/guest_calls.hpp.
// The update also shows the message of the box's rows, Put In Item's for a
// row without one, while bit 4 of the word at ui + 1512 is set (0x09AE3958),
// which the frame sets as soon as a row is chosen (0x09ADCF5C), and in that
// frame no menu is drawn for a row with no screen. So the screens take the
// frame Item Sets is chosen in too (the menu's choose, 0x4000, with nothing
// the menu acts on before it, at 0x09ADCE14), and clear the bit each frame.
// Leaving the screens puts the phase back to 0, and the box shows its menu
// again, on Item Sets.
//
// The screens use the game's own parts, as Put In Item and Change Equipment
// do:
// - 0x0884FB6C: the buttons, (*0x08A62D94 & 0xF0) | *0x08A62D8C, or none
//   while *(*0x08A62D80 + 116) is set. The box chooses with 0x4000 and goes
//   back with 0x2000.
// - 0x09AE66F0(box, 1): the box menu, dimmed behind a screen.
// - 0x0882FA24(record, template, title, dx, dy) builds a window from a
//   20-byte template (u16 x, y; u8 width in characters, line height - 2, 0,
//   frame; u16 character width, rows; the rows' text ids; the colour), and
//   0x0882F580(ui, record, title, row or -1, 255, 1) draws it, with the row
//   highlighted and the rows' words from 0x09A7B890. ui is *0x089CBD0C.
//   0x0882FB38(ui, record, page, pages, 0, 0) adds the arrows and the page
//   number to its title, as Equipment Set's and the pouch's windows have:
//   the number ends 21 pixels in from the right, 7 pixels a character
//   (0x09AEB194). The list's title is printed between them, centred.
// - 0x088198E4(ui, slots, first, 0, 0x10, 0): the pouch's window, "Items",
//   eight of 24 (id, count) slots from `first`, with the page arrows, as Put
//   In Item shows the pouch (0x088197A0 passes it a copy).
// - 0x0882FC50(ui, record, text, 255): a message window. Put In Item's is the
//   record at 0x09B0B128, its Yes/No at (320, 240): entries 8 and 9 of the
//   game's text table 0. 0x088920A8(font, x, y, format, ...) prints, with
//   the game's markup (~B01 the choose button, ~B02 △, as the hints under
//   Equipment Set); font is *0x08A632C8.
// - 0x08883988(*0x08A62EAC, bank, sound, 0, 0, 0, 0, 0, 0) plays a sound.
//   Bank 0, the menus': 17 a page, 19 choose, 20 back, 21 refused, 23 the
//   cursor. Bank 1: 136 items put in or taken out (Take Out Item, 0x09ADDD14),
//   137 an equipment set changed (0x09AE0DC0).

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
inline constexpr std::uint32_t kBoxPouch = 0x43C8u; // the box screens' copy
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
inline constexpr std::uint8_t kRow = 6u;      // Item Sets, as the box numbers its rows
inline constexpr std::uint8_t kShownRow = 3u; // where it is shown: after Combine Items
inline constexpr std::uint32_t kMenuDrawReturn = 0x09AE6784u; // 0x09AE66F0's call of 0x0882F580
inline constexpr std::uint32_t kText = 0x09A7B890u;
inline constexpr std::uint32_t kCursor = 0x0881C2E4u;
inline constexpr std::uint32_t kCursorReturn = 0x09ADCE14u;
inline constexpr std::uint32_t kUpdate = 0x09AE38F8u;
inline constexpr std::uint32_t kBoxFrame = 0x09ADCCD0u; // the box's buttons, then its update
inline constexpr std::uint32_t kBoxFlags = 48u;
inline constexpr std::uint32_t kMenuShown = 96u; // 0 while START has hidden the menu
inline constexpr std::uint16_t kStart = 0x0008u;inline constexpr std::uint32_t kChosenRow = 54u;
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

// The box keeps numbering its rows as the game does, Item Sets 6, and shows
// Item Sets fourth: where a row is shown, the row shown there, and the row
// up (0x10) or down (0x40) moves to, wrapping as the game's cursor does.
[[nodiscard]] std::uint8_t shown_row(std::uint8_t row);
[[nodiscard]] std::uint8_t game_row(std::uint8_t shown);
[[nodiscard]] std::uint8_t move_shown(std::uint8_t row, std::uint16_t buttons);

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
// 24 sets, shown eight to a page as the list's rows.
inline constexpr std::size_t kSets = 24u;
inline constexpr std::size_t kSetsPerPage = 8u;
inline constexpr std::size_t kSetPages = kSets / kSetsPerPage;
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
// holds, and into the box screens' copy of the pouch. All or nothing: when
// the box cannot take the pouch, nothing moves.
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

// The screens' parts.
inline constexpr std::uint32_t kButtons = 0x08A62D8Cu;
inline constexpr std::uint32_t kHeldDpad = 0x08A62D94u;
inline constexpr std::uint32_t kInputOwner = 0x08A62D80u;
inline constexpr std::uint32_t kInputBlocked = 116u;
inline constexpr std::uint32_t kUiPointer = 0x089CBD0Cu;
inline constexpr std::uint32_t kUiFlags = 1512u;
inline constexpr std::uint32_t kUiMessage = 4u; // the box's update shows its row's message
inline constexpr std::uint32_t kFontPointer = 0x08A632C8u;
inline constexpr std::uint32_t kSoundPointer = 0x08A62EACu;
inline constexpr std::uint32_t kBoxMenu = 0x09AE66F0u;
inline constexpr std::uint32_t kWindowBuild = 0x0882FA24u;
inline constexpr std::uint32_t kWindowDraw = 0x0882F580u;
inline constexpr std::uint32_t kPager = 0x0882FB38u;
inline constexpr std::uint32_t kFontSize = 0x08890DE8u;   // (font, width, height)
inline constexpr std::uint32_t kFontColour = 0x08890E38u; // (font, colour): 5 the titles' yellow
inline constexpr std::uint32_t kItemWindow = 0x088198E4u;
inline constexpr std::uint32_t kMessageDraw = 0x0882FC50u;
inline constexpr std::uint32_t kMessageRecord = 0x09B0B128u;
inline constexpr std::uint32_t kPrint = 0x088920A8u;
inline constexpr std::uint32_t kPrintFormat = 0x08938598u; // "%s"
inline constexpr std::uint32_t kSound = 0x08883988u;
inline constexpr std::uint32_t kYesText = 8u; // "[Yes] · No", table 0
inline constexpr std::uint32_t kNoText = 9u;  // "Yes · [No]"

// What the screens keep in guest memory, in the window the host reserves:
// the windows' templates and record, the rows' ids and words, the set's
// slots for the pouch's window, and the words of the message.
inline constexpr std::uint32_t kScratch = 0x08802000u;
inline constexpr std::uint32_t kChooseTemplate = kScratch;
inline constexpr std::uint32_t kListTemplate = kScratch + 0x20u;
inline constexpr std::uint32_t kWindowRecord = kScratch + 0x40u;
inline constexpr std::uint32_t kChooseIds = kScratch + 0x80u;
inline constexpr std::uint32_t kListIds = kScratch + 0x90u;
inline constexpr std::uint32_t kSetSlots = kScratch + 0xC0u;
inline constexpr std::uint32_t kRowText = kScratch + 0x200u; // kRowTextSize bytes a row
inline constexpr std::uint32_t kRowTextSize = 32u;
inline constexpr std::uint32_t kTitleText = kScratch + 0x400u; // the window's own title: none
inline constexpr std::uint32_t kListTitle = kScratch + 0x420u; // drawn centred over it
inline constexpr std::uint32_t kMessageText = kScratch + 0x440u;
inline constexpr std::uint32_t kMessageTextSize = 256u;
inline constexpr std::uint32_t kHintText = kScratch + 0x540u;
inline constexpr std::uint32_t kHintTextSize = 64u;
inline constexpr std::uint32_t kYesNoText = kScratch + 0x580u;
inline constexpr std::uint32_t kScratchEnd = kScratch + 0x5A0u;
// The rows' text ids after kTextId: the menu's two, then the list's eight,
// the sets of the page shown.
inline constexpr std::uint16_t kChooseId = kTextId + 1u;
inline constexpr std::uint16_t kSetId = kChooseId + 2u;
inline constexpr std::uint16_t kLastId = static_cast<std::uint16_t>(kSetId + kSetsPerPage - 1u);

// At the flip: puts the hooks in place once lobby_task is installed. Runs
// every frame; doing it again is a no-op.
void frame(psprecomp::Runtime &runtime);

// The Item Sets screens are showing.
[[nodiscard]] bool screen_open();

// The sets, for the hunter at the box.
[[nodiscard]] Store &store();

} // namespace mhfu::game::item_sets
