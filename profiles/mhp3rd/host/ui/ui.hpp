#pragma once

// The port's own interface, drawn with Dear ImGui over the game's window.
// Only builds with the renderer have it.
namespace mhp3rd::gpu {
class VulkanRenderer;
}

namespace mhp3rd::ui {

// Puts the interface on the renderer's window. False when it cannot start;
// the game then runs without a menu.
bool attach(gpu::VulkanRenderer &renderer);

// Before each game frame is presented: draws what the interface shows over
// the running game (the hint that says how to open the menu).
void draw_over_game();

// After a game frame's window events: whether the player asked for the menu
// (Esc, or L3+R3 on a gamepad).
[[nodiscard]] bool menu_requested();

// Runs the menu over the last game frame until the player closes it. The
// caller pauses the game around it. False: the player chose to quit.
bool run_menu();

} // namespace mhp3rd::ui
