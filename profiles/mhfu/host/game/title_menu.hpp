#pragma once

#include <cstdint>

namespace psprecomp {
class GuestMemory;
class Runtime;
} // namespace psprecomp

// The title menu without DATA INSTALL while the install is served
// (game/install_data.hpp): the game finds it complete at every start, so the
// entry has nothing to do.
//
// demo_task (at 0x09A5F300) runs the title menu in 0x09A61568(task), called
// through its function table. task + 28 is the menu's step; at step 2 it
// reads the pad (the buttons pressed this frame in the word at 0x08A62D94):
// up (0x10) takes the cursor down by one, below 0 to 5, and otherwise down
// (0x40) up by one, from 6 to 0. The cursor is the word at + 1176 of the
// object load32(0x089CC558). Confirm later starts the entry under it:
// NEW GAME, CONTINUE, GALLERY, OPTIONS, DOWNLOAD and DATA INSTALL, 0 to 5.
// Then, at every step, the same call draws the six labels, the game's texts
// 615 to 620 of text table 0 (the halfwords at 0x089ACC0C), 32 pixels apart.
//
// The update is wrapped. Before the game's own runs, the cursor is set so
// that its move skips the last entry: on DOWNLOAD going down it is put on
// DATA INSTALL, which the game then wraps to NEW GAME, and on NEW GAME going
// up likewise, which the game takes to DOWNLOAD. A cursor found on the entry
// goes to NEW GAME. While demo_task is in, text 620 points at its own end, so
// its label is drawn empty; it is put back when demo_task goes. Read from
// demo_task's code; the code and the labels are checked before anything is
// wrapped or changed.
namespace mhfu::game::title_menu {

inline constexpr std::uint32_t kMenuUpdate = 0x09A61568u;
inline constexpr std::uint32_t kStepOffset = 28u;
inline constexpr std::uint32_t kInputStep = 2u;
inline constexpr std::uint32_t kPressed = 0x08A62D94u;
inline constexpr std::uint32_t kUp = 0x10u;
inline constexpr std::uint32_t kDown = 0x40u;
inline constexpr std::uint32_t kCursorObject = 0x089CC558u;
inline constexpr std::uint32_t kCursorOffset = 1176u;
inline constexpr std::uint32_t kEntries = 6u;
inline constexpr std::uint32_t kHiddenEntry = 5u; // DATA INSTALL
inline constexpr std::uint32_t kLabels = 0x089ACC0Cu;
inline constexpr std::uint16_t kFirstLabel = 615u;
inline constexpr std::uint32_t kTextBlock = 0x089CC560u; // text table k at + load32(+ (k + 2) * 4)

// The cursor to give the game's step 2 so that, with `pressed`, it moves
// past the hidden entry, as the five entries before it would wrap.
[[nodiscard]] std::uint32_t cursor_before_input(std::uint32_t cursor, std::uint32_t pressed) noexcept;

// Points the hidden entry's label at the end of its text. False, changing
// nothing, when the text is empty already or not where the table says.
struct HiddenLabel {
    std::uint32_t entry{};  // where the label's offset is kept
    std::uint32_t offset{}; // the game's
    std::uint32_t hidden_offset{};
};
[[nodiscard]] bool hide_label(psprecomp::GuestMemory &memory, HiddenLabel &hidden);
// Puts the game's offset back, unless the table has been loaded again since.
void restore_label(psprecomp::GuestMemory &memory, const HiddenLabel &hidden);

// Whether demo_task's menu code and the labels are those this was read from.
[[nodiscard]] bool code_matches(const psprecomp::GuestMemory &memory);

// Once a frame: keeps the update wrapped and the label hidden while
// demo_task is in and the install is served.
void frame(psprecomp::Runtime &runtime);

} // namespace mhfu::game::title_menu
