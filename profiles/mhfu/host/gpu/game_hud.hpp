#pragma once

#include <cstdint>
#include <span>
#include <string>

namespace psprecomp {
class GuestMemory;
class Runtime;
}

// Hiding the game's HUD (issue #184): the health and stamina bars, the clock,
// sharpness, the item bar, the map, name tags and the other labels that
// follow characters, and the prompts in the corners. Menus, dialogs, fades,
// cutscenes and loading screens stay.
//
// How the game's 2D draws were told apart, read from USA Freedom Unite
// (ULUS-10391) and from the lobby_task and game_task overlays, both loaded
// at 0x09A5F300:
//
// - Every 2D element is a packet of GE commands the game files into its
//   display list's ordering table with 0x088596C8 (a0 the list object, a1
//   the packet, a2 its length in words, a3 the bucket). Buckets are drawn in
//   order. The HUD shares its buckets with menus and dialogs, so the bucket
//   alone cannot tell them apart.
// - The code that asks for the packet can. In the village and the Guild Hall,
//   lobby_task draws its text layer from 0x09AE38F8, and its task callback
//   0x09A6C248 is on the stack for the village interface (it calls through
//   the mode at object+44). In a quest, the HUD is a task callback, not the
//   frame function: traced with MHFU_TRACE_HUD in
//   the training quest, the health bar, the map, the item bar and their text
//   are filed while 0x09A60298 is on the stack (through 0x09A62740 and
//   0x09A627B8). 0x09A617A8 and 0x09A62400 file the same sprites for the
//   quest's other HUD states. Name tags in a quest queue their own text from
//   0x09B62488. In the village the same names are drawn with the rest of that
//   interface, so the name size leaves them as the game draws them.
//   Menus are other callbacks and stay. A packet is HUD when one
//   of those is on the guest's call stack when it is filed; the stack is
//   read the way the game's code builds it (each function's addiu sp, sp, -N
//   and sw ra). Each function's first two words are checked, because the two
//   overlays replace each other in the same slot.
// - Most text is not drawn where it is asked for. 0x08A632C8 points at the
//   text context, which keeps six queues, and the game draws every queue at
//   the end of the frame, far from the code that asked. The four functions
//   that append to a queue are watched instead, and the entries added until
//   the next watched call or packet belong to the code on the stack then.
//   When a queue is drawn, the entry each glyph comes from is read off the
//   drawing function's stack frame, and so is the list entry of each button
//   icon in the text.
// - The GE draws a list while the game fills the next elsewhere, so a HUD
//   packet's range is kept until another packet is filed over it.
//
// The renderer then leaves out the draws of HUD packets, and only those,
// while the HUD is hidden. The packets' other commands still run, so the GE
// state they set is the same either way.
namespace mhfu::gpu::hud {

struct CodeWord {
    std::uint32_t address;
    std::uint32_t word;
};

// One function the HUD is drawn from, and the two words that must still be
// there (its addiu sp and its sw ra) before a call stack counts.
struct HudFunction {
    std::uint32_t address;
    std::uint32_t first;
    std::uint32_t saved_ra;
};

// The resident code this was read from. Exposed for tests.
[[nodiscard]] std::span<const CodeWord> code_signature();
// The overlay functions. A match also requires the words above.
[[nodiscard]] std::span<const HudFunction> hud_functions();
[[nodiscard]] bool code_matches(const psprecomp::Runtime &runtime);
[[nodiscard]] bool hud_function_matches(const psprecomp::GuestMemory &memory, std::uint32_t address);
// The name over another hunter in a quest: game_task 0x09B62488. The village
// and the gathering hall draw those names with the rest of that interface, so
// they are not names for this size. The words still have to match before a
// call stack counts; this is the address only.
[[nodiscard]] bool name_label(std::uint32_t address);
[[nodiscard]] bool name_tag_only(std::uint32_t address);

// Called at every flip, where guest functions may be wrapped: puts the
// wrappers in place once the HUD is first hidden, a name or the map is scaled
// off the game's size, or MHFU_TRACE_HUD asks.
void frame(psprecomp::Runtime &runtime, std::uint64_t presented_frames);

// The player's switch (the Hide HUD bind and the menu).
void set_hidden(bool hidden);
void toggle();
[[nodiscard]] bool hidden();
// Whether the renderer leaves the HUD out now.
[[nodiscard]] bool active();
// False when the game's code is not the code the classification was traced
// on: the switch then does nothing.
[[nodiscard]] bool available();

// A short note after the switch changes: its text, and how long it has left.
[[nodiscard]] double note_seconds_left();
[[nodiscard]] std::string note_text();

// True when a through-mode draw belongs to the HUD and the HUD is hidden.
// `command_address` is its PRIM command, `call_return` where the list's
// innermost CALL returns to.
[[nodiscard]] bool hides(std::uint32_t command_address, std::uint32_t call_return);

// A through-mode draw of a name over another hunter, once the 2D draws are
// being watched. `has_origin` is the point the game placed the text (the
// entry's x and y); a plate filed on its own has none, and shrinks about its
// own centre. `tag_only` limits a name to a name-sized draw. The quest name is
// the whole draw, so it is not set. Empty when this draw is not a name.
struct NameDraw {
    bool is_name{};
    bool has_origin{};
    bool tag_only{};
    float origin_x{};
    float origin_y{};
};
[[nodiscard]] NameDraw name_at(std::uint32_t command_address, std::uint32_t call_return);

} // namespace mhfu::gpu::hud
