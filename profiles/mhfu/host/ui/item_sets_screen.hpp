#pragma once

// The Item Sets screen (game/item_sets.hpp), opened from the item box menu's
// Item Sets entry, as the game's Equipment Sets is from Change Equipment: the
// hunter's eight sets, and for the one chosen, Load (the pouch goes into the
// box and the set's items come out), Register Pouch, Rename and Delete Set.
// It is drawn over the running game, which gets no input until it closes.
namespace mhfu::ui {

// Opens the screen. False when there is no interface to draw it in.
bool open_item_sets();
[[nodiscard]] bool item_sets_open();
// Draws one frame inside an interface frame; false once it has closed.
bool item_sets_frame();

} // namespace mhfu::ui
