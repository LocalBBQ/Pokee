#pragma once

// The Event quests page (game/event_quests.hpp), opened from the Mods page:
// the switch, the quests in event_quests/ of the data directory, and
// importing quest files from a folder with the file browser (or by dropping
// the folder on the window).
namespace mhfu::ui {

// The row on the Mods page that opens the page.
void event_quests_row();

// Draws the page, or the screen it opened, while it is open; false when it is
// closed and the Mods page is to be drawn instead. `back`: the back button
// was pressed this frame.
bool event_quests_screen(bool back);

// Whether the page is open, so back closes it before the menu.
[[nodiscard]] bool event_quests_screen_open();

} // namespace mhfu::ui
