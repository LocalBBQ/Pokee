#pragma once

// What the Item Sets screens do with the box's buttons, apart from drawing
// them and changing the game (game/item_sets.cpp does both). Modelled on the
// game's Change Equipment: a menu of two rows, Load Item Set and Register
// Pouch; then the sets, eight to a page that L and R turn, with the chosen
// one's items beside them as the pouch shows them (left and right turn
// those), and the game's Yes/No to confirm. △ deletes a set.

#include "game/item_sets.hpp"

#include <cstdint>
#include <string>

namespace mhfu::game::item_sets::menu {

// The box's buttons, as 0x0884FB6C gives them: pressed this frame, and the
// D-pad repeating while held.
inline constexpr std::uint16_t kUp = 0x0010u;
inline constexpr std::uint16_t kRight = 0x0020u;
inline constexpr std::uint16_t kDown = 0x0040u;
inline constexpr std::uint16_t kLeft = 0x0080u;
inline constexpr std::uint16_t kL = 0x0100u;
inline constexpr std::uint16_t kR = 0x0200u;
inline constexpr std::uint16_t kDelete = 0x1000u;  // △
inline constexpr std::uint16_t kCancel = 0x2000u;  // the box's back
inline constexpr std::uint16_t kConfirm = 0x4000u; // the box's choose

// The game's sounds, as the box plays them (0x08883988(player, bank, id,
// ...)); sound_id gives the id and sound_bank the bank: 0 for the menus'
// sounds, 1 for those of the box's items and equipment.
enum class Sound {
    None,
    Page,       // 17
    Confirm,    // 19
    Cancel,     // 20
    Refuse,     // 21
    Cursor,     // 23
    Moved,      // 136 in bank 1: items put in the box or taken out
    Registered, // 137 in bank 1: an equipment set changed
};
[[nodiscard]] std::uint32_t sound_id(Sound sound);
[[nodiscard]] std::uint32_t sound_bank(Sound sound);

enum class Page { Choose, List, Confirm };
enum class Mode { Load, Register };
enum class Ask { Load, Register, Delete };

// Pages of the pouch's window: 24 slots, 8 to a page.
inline constexpr std::uint8_t kPages = 3u;
inline constexpr std::uint8_t kChooseRows = 2u;

struct State {
    Page page{Page::Choose};
    Mode mode{Mode::Load};
    std::uint8_t row{};      // on Choose
    std::uint8_t set{};      // on List and Confirm, 0 to kSets - 1
    std::uint8_t contents{}; // the page of the chosen set's items
    Ask ask{Ask::Load};
    bool yes{};
    // What the last change did, shown in place of the prompt until the next
    // move.
    std::string result;
    bool result_warning{};
};

enum class Act { None, Close, Load, Register, Delete };

struct Step {
    Act act{Act::None};
    std::size_t set{};
    Sound sound{Sound::None};
};

// One frame of the box's buttons. `sets` are the hunter's, `pouch_empty`
// whether there is anything to register.
Step step(State &state, std::uint16_t buttons, const Sets &sets, bool pouch_empty);

// After an Act: what it did, shown on the list.
void finished(State &state, std::string text, bool warning);

// The message window's words for the page.
[[nodiscard]] std::string message(const State &state);
// The button hints under the list, in the game's markup: ~B01 its choose
// button, ~B02 △, as Equipment Set's.
[[nodiscard]] std::string hint(const State &state);
// The rows and titles, as the game's lists show them.
[[nodiscard]] std::string choose_row(std::uint8_t row);
[[nodiscard]] std::string list_title(Mode mode);
[[nodiscard]] std::string set_row(std::uint8_t set);
// The page of sets the list shows, and the row a set is on.
[[nodiscard]] std::uint8_t set_page(const State &state);
[[nodiscard]] std::uint8_t set_row_on_page(const State &state);

} // namespace mhfu::game::item_sets::menu
