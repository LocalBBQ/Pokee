#pragma once

#include "imgui.h"

#include <initializer_list>
#include <string>

// The look of the interface and the controls its screens are built from.
// Everything is sized from the current text size, so it scales with the
// window, and every control works with a gamepad, a keyboard and a mouse.
namespace mhfu::ui {

// Colours from the game's own menus: a grey-green glass over the scene with a
// diamond lattice, a black and orange frame, a solid green bar behind the chosen
// entry and yellow text on it.
namespace colors {
[[nodiscard]] constexpr ImU32 with_alpha(ImU32 color, int alpha) {
    return (color & ~IM_COL32_A_MASK) | (static_cast<ImU32>(alpha) << IM_COL32_A_SHIFT);
}

inline constexpr ImU32 kBackdrop = IM_COL32(6, 10, 6, 150);
inline constexpr ImU32 kPanel = IM_COL32(36, 46, 40, 232);
inline constexpr ImU32 kPopup = IM_COL32(36, 46, 40, 248);
inline constexpr ImU32 kShade = IM_COL32(6, 10, 6, 150);
inline constexpr ImU32 kPanelEdge = IM_COL32(222, 138, 58, 255);
inline constexpr ImU32 kRow = IM_COL32(220, 232, 214, 12);
inline constexpr ImU32 kRowHover = IM_COL32(40, 160, 36, 60);
inline constexpr ImU32 kRowFocus = IM_COL32(40, 160, 36, 150);
inline constexpr ImU32 kAccent = IM_COL32(72, 184, 56, 255);
inline constexpr ImU32 kAccentBright = IM_COL32(248, 236, 72, 255);
inline constexpr ImU32 kEmber = IM_COL32(222, 138, 58, 255);
inline constexpr ImU32 kText = IM_COL32(236, 236, 228, 255);
inline constexpr ImU32 kTextDim = IM_COL32(168, 178, 166, 255);
inline constexpr ImU32 kTextDisabled = IM_COL32(108, 118, 108, 255);
inline constexpr ImU32 kDanger = IM_COL32(232, 96, 72, 255);
inline constexpr ImU32 kGood = IM_COL32(132, 174, 96, 255);
inline constexpr ImU32 kTrack = IM_COL32(220, 232, 214, 36);
// The selection bar, the frame's ink, and the lattice behind the panel.
inline constexpr ImU32 kSelect = IM_COL32(34, 150, 30, 255);
inline constexpr ImU32 kFrameInk = IM_COL32(10, 10, 8, 255);
// The window frame's corner knobs and the crenellated bands between them.
inline constexpr ImU32 kFrameKnob = IM_COL32(177, 118, 67, 255);
inline constexpr ImU32 kBandTealDark = IM_COL32(20, 71, 71, 255);
inline constexpr ImU32 kBandTeal = IM_COL32(27, 93, 95, 255);
inline constexpr ImU32 kBandRed = IM_COL32(151, 81, 76, 255);
inline constexpr ImU32 kBandRedDark = IM_COL32(114, 63, 60, 255);
inline constexpr ImU32 kLatticeDark = IM_COL32(0, 0, 0, 46);
inline constexpr ImU32 kLatticeLight = IM_COL32(170, 190, 170, 20);
} // namespace colors

ImGuiStyle make_style(float scale, float font_size);

// The game's selection behind a row: a solid green bar.
// 235 for the focused row, 80 for one under the mouse.
void draw_selection(ImDrawList *draw, ImVec2 min, ImVec2 max, int alpha);

// The panel every screen sits in, centred in the window: title and optional
// subtitle, then fixed parts such as tabs, then the scrolling content from
// begin_content(), then a footer with the focused row's description and the
// button hints:
//
//   begin_panel(...); tab_bar(...); begin_content(); rows...;
//   begin_footer(); hints(...); end_panel();
void begin_panel(const char *id, const std::string &title, const std::string &subtitle, bool dim_game);
void begin_content();
// A drag with a finger scrolls the current window, as on any touch screen;
// ImGui itself scrolls only with a wheel. Call before ending the window.
void touch_scroll();
void begin_footer();
void end_panel();

// Tabs switched with L1/R1 (Q/W on the keyboard) or the mouse. Returns true
// when `selected` changed.
bool tab_bar(const char *const *labels, int count, int &selected);

struct RowOptions {
    bool disabled{};
    std::string note;        // shown dimmed next to the value, e.g. who decides it
    std::string description; // shown in the footer while the row is focused
    bool warning{};          // the value clashes with something: drawn in the danger colour
};

// A setting with a few values, changed with left/right or by activating it.
// Returns -1, 0 or +1.
int choice_row(const char *label, const std::string &value, const RowOptions &options = {});
// A setting chosen from a list that opens under the row. Up and down, on a
// keyboard or a gamepad, move through the open list, and confirm chooses the
// highlighted entry. `back` closes the list instead of leaving the page;
// dropdown_swallowed_back() is then true for the rest of the frame.
// Call dropdown_new_frame() once at the start of the frame.
void dropdown_new_frame();
void dismiss_dropdowns();
[[nodiscard]] bool dropdown_swallowed_back();
bool dropdown_row(const char *label, const char *const *items, int count, int &selected, const RowOptions &options = {},
    bool back = false);
// A row that opens the rows drawn after it. Confirm or a click toggles `open`.
// The chevron points right while closed and down while open. Returns true when
// it toggles.
bool disclosure_row(const char *label, bool &open, const RowOptions &options = {});
// An on/off setting. Returns true when toggled.
bool toggle_row(const char *label, bool value, const RowOptions &options = {});
// A number between minimum and maximum, changed in steps with left/right or
// dragged with the mouse. Returns true when `value` changed.
bool slider_row(const char *label, int &value, int minimum, int maximum, int step, const char *format,
    const RowOptions &options = {});
// An action. Returns true when activated.
bool button_row(const char *label, const RowOptions &options = {}, ImU32 color = colors::kText);
// A setting chosen on a screen of its own: its value, and a chevron that
// opens that screen. Returns true when activated.
bool value_row(const char *label, const std::string &value, const RowOptions &options = {});
// An entry of a list such as the file browser's: an icon, a name and a
// detail on the right. `id` keeps rows with equal names apart.
enum class ListIcon { None, Folder, ParentFolder, File, Disc, Drive };
bool list_row(
    const char *id, const std::string &name, const std::string &detail, ListIcon icon, bool highlight = false);
// A line of information, focusable so a gamepad can scroll to it.
void info_row(const char *label, const std::string &value);
void section(const char *title);

// Focuses the next row, e.g. the first row after switching tabs.
void focus_next_row();

// A wide button for the setup screens. Returns true when activated.
bool big_button(const char *label, float width, bool primary = false, bool disabled = false);
void progress_bar(float fraction, const std::string &overlay);

// Text wrapped to the available width.
void paragraph(const std::string &text, ImU32 color = colors::kText);
void heading(const std::string &text);

// Button hints for the footer, drawn with the glyphs of the pad in use or the
// keys of the keyboard. Toggle, Shift, Space and Symbols exist on the pad only
// and are skipped for the keyboard.
enum class Control {
    Confirm,
    Back,
    Tabs,
    Change,
    Menu,
    Start,
    Toggle,  // the top face button
    Delete,  // the back face button; Backspace
    Shift,   // the left face button
    Space,   // the top face button
    Symbols, // Select, Share, Create or View, as the pad names it
    Cursor,  // the shoulder buttons; the arrow keys
    Clear,   // the top face button; Delete
    Reset,   // Select, Share, Create or View, as Symbols
};
struct Hint {
    Control control;
    const char *text;
};
void hints(std::initializer_list<Hint> list);

} // namespace mhfu::ui
