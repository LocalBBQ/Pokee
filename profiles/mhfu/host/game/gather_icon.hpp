#pragma once

// A gather mark drawn by the port over a node the game would gather (ULUS-10391).
// Off, nothing here runs. On, the mark sits above the node while the hunter is
// inside the radius the game's own search uses, and it is left out of the
// game's frames.
//
// The search is game_task 0x09A6F490, and it is read only while that overlay
// is loaded (the "MWo3" header and the name game_task.ovl at 0x09A5F300).
// It asks 0x08869570 for the loaded area's list, then walks 24-byte records
// until x is -1:
//   +0 x, +4 y, +8 z, +12 radius, all floats
//   +16 the gather id it returns, +20 the kind the action switches on
// The hunter is inside when node.y - 200 <= hunter.y < node.y + 100 and the
// horizontal distance is within the radius. The first such record is the one
// ○ gathers. The list does not carry uses left, so a spent node still matches
// while it remains on the list.
//
// 0x08869570 reads the object at 0x08A62D9C. Its word at +96 is the area
// table and the word at +80 is added to that area's pointer. The area index
// is 0x088C722C: the byte at GameSys+0x6AF10 selects an 8-byte entry of the
// table at 0x09CE4808 (a count, then halfword stage ids), and the stage id is
// the halfword at the followed hunter +0x29A. The followed hunter is the word
// at (the pointer at 0x089CBD04) + 2692, the same object the follow camera
// reads. The view a world draw uploads has no translation, so the mark is
// placed from the follow camera's eye and look-at.

#include "psprecomp/guest_memory.hpp"

#include <array>
#include <cstdint>
#include <optional>
#include <span>

namespace mhfu::game {

struct GatherNode {
    std::uint32_t address{};
    float x{};
    float y{};
    float z{};
    std::uint16_t id{};
    // The halfword at +20. The action switches on 0 (○: plants and the like),
    // 3 (□: mining) and 4 (□: a bug net).
    std::uint16_t kind{};
};

// True for the kinds the game gathers with □: mining (3) and a bug net (4).
[[nodiscard]] inline bool gather_uses_square(std::uint16_t kind) noexcept {
    return kind == 3u || kind == 4u;
}

// Above the node's origin, where the mark floats.
inline constexpr float kGatherLift = 100.0f;
// The usual mark, in the picture's pixels (the 272-tall frame): the disc's
// radius and the ? drawn in it. Size is a percent of these; thickness widens
// the ring and the ? (gather_mark_shape).
inline constexpr float kGatherDisc = 8.0f;
inline constexpr float kGatherGlyph = 9.0f;

// The mark as drawn, in those same pixels. `ring` is the outline's width and
// `weight` is how far the ? is stroked past the font; weight is 0 at thickness 1.
struct GatherShape {
    float radius{};
    float ring{};
    float glyph{};
    float weight{};
};

// `size_percent` and `thickness` are clamped to the settings ranges. Thickness
// 1 is a one-pixel ring and the font's own ?, scaled with the size. Each step
// adds a pixel of ring and half a pixel of ? stroke, both scaled the same way.
[[nodiscard]] GatherShape gather_mark_shape(std::uint32_t size_percent, std::uint32_t thickness) noexcept;

struct GatherWord {
    std::uint32_t address{};
    std::uint32_t instruction{};
};

// The instructions that establish the search, the list and the hunter.
[[nodiscard]] std::span<const GatherWord> gather_checked_words();

// True when every one of those words is mapped and matches. An unmapped word
// (the quest overlay is not loaded) is a quiet false. A mapped word that
// differs is logged once.
[[nodiscard]] bool gather_code_matches(const psprecomp::GuestMemory &memory);

// The first node on `list` the hunter at (x, y, z) would gather. Nothing at
// the sentinel, past 64 records, or when none is in range.
[[nodiscard]] std::optional<GatherNode> gather_from_list(const psprecomp::GuestMemory &memory, std::uint32_t list,
    float hunter_x, float hunter_y, float hunter_z);

// The area list for this stage id, or nothing when the pointers are not there.
[[nodiscard]] std::optional<std::uint32_t> gather_list_for(const psprecomp::GuestMemory &memory, std::uint16_t stage);

// The node ○ would gather now, when the instructions match. Nothing in the
// village, with no hunter, or with no node in range.
[[nodiscard]] std::optional<GatherNode> gather_node(const psprecomp::GuestMemory &memory);

// The projection and viewport of one perspective draw. The view matrix is not
// kept: the uploaded view has no translation.
struct GatherPicture {
    std::array<float, 16> projection{};
    float x_scale{};
    float y_scale{};
    float x_offset{};
    float y_offset{};
    float origin_x{};
    float origin_y{};
};

struct GatherPose {
    float eye_x{};
    float eye_y{};
    float eye_z{};
    float look_x{};
    float look_y{};
    float look_z{};
};

// The perspective draw with the most vertices since the mark last asked.
void gather_note_draw(const GatherPicture &picture, std::uint32_t vertices);

// Where the mark goes on the picture, 0..1 from the top left, while Gather
// icon is on. Nothing when it is off, the code differs, there is no node, or
// the node is behind the camera or off the picture. Takes the draw noted
// since the last call.
[[nodiscard]] std::optional<std::array<float, 2>> gather_mark(
    const psprecomp::GuestMemory &memory, const std::optional<GatherPose> &pose);

// The pulse a hold sends, in pad reads the game takes: the gather button down
// for kGatherPulseDown reads, then up for kGatherPulseUp. Each part is long
// enough that a read at the game's own rate sees it.
inline constexpr unsigned kGatherPulseDown = 3u;
inline constexpr unsigned kGatherPulseUp = 3u;

// Holding ○ or □ on a gather point, or ○ on a monster that can be carved.
// Plants and carving use ○ (B on an Xbox pad); mining and a bug net use □
// (X on an Xbox pad). The game starts one when that button goes down, so a
// bit that stays down is one gather or one carve. The hold presses the button
// again and again, the way a player mashes it: the game starts the next
// gather on the first press after the hunter is free and ignores the ones
// during the animation. The hunter's action is not read. Following it broke
// whenever the gather went through an action the hold did not expect, such as
// stopping out of a run, and then the hold never pressed again.
// A hold that begins with the button already down starts on the up part, so
// the game sees a new press. Letting go stops the pulse at once. While the
// hold is going, the game is shown the pulse only: movement and every other
// button are left off, so they cannot walk the hunter off or start another
// action. Another button can take the gather button out of the resolved pad
// for a few reads. The hold stays until nothing is held, and those reads
// still show only the pulse. Away from a node and from a body, or with the
// setting off, the buttons pass through. L+○ or L+□ before a hold has started
// is the item bar.
class GatherRepeat {
public:
    // `ready` is the gather search, or the carve search, accepting `button`.
    // `button` is ○ or □ for this hold.
    [[nodiscard]] std::uint32_t apply(std::uint32_t buttons, bool ready, std::uint32_t button);
    void reset() { *this = GatherRepeat{}; }
    // The hold owns the pad, even on a read where the gather button is missing.
    [[nodiscard]] bool underway() const { return holding_ && latched_; }
    // The ○ or □ bit this hold is pulsing. Meaningful while underway.
    [[nodiscard]] std::uint32_t button() const { return button_; }
    // True after apply when this hold owns the pad, so the movement stick
    // is reported centred for that read.
    [[nodiscard]] bool centres_stick() const { return centres_; }

private:
    bool holding_{};
    // The gather button was down on the last read, whether or not the hold had it.
    // Tied to input_button_: a □ hold must not treat a held ○ as already down.
    bool input_down_{};
    bool centres_{};
    // The gather button was down for this hold. Stays set while some other
    // button is held, so a resolved pad that dropped it does not read as a release.
    bool latched_{};
    std::uint32_t button_{0x2000u}; // ○ until a hold picks □
    std::uint32_t input_button_{0x2000u};
    unsigned step_{};
};

// A monster body the game would carve with ○. 0x0886CEA8 walks the table whose
// pointer is the word at 0x09A4F1A0: records start 4 bytes in, 32 bytes each,
// at most 20. +0 x, +4 y, +8 z and +12 radius are floats. The halfword at +16
// is 0xFFFF for an empty slot. The halfword at +24 is the stage and must
// match the hunter at +0x29A. The height window is the gather one: at or
// above the body minus 200, and below it plus 100. The horizontal distance
// must be inside the radius. 0x0886CF78 returns the halfword at +20 of the
// first such body; 0xFFFF is not a carve, and the circle handler
// (game_task 0x09AA73A4) does not look at a later body. It also skips the
// carve when bit 8 of the word at hunter +1164 is set. Nothing in the
// village, with no hunter, or with no body in reach.
[[nodiscard]] bool carve_in_reach(const psprecomp::GuestMemory &memory);

// The buttons the game should see for this pad read. Off, or idle with the
// gather button up, `buttons` comes back unchanged and the gather list is not
// read. While the hold owns the pad, every other button is cleared.
// gather_hold_centres_stick is then true: the movement stick for that read is
// centred, so a push does not walk the gather or the carve off.
[[nodiscard]] std::uint32_t gather_hold(std::uint32_t buttons, const psprecomp::GuestMemory &memory);
[[nodiscard]] bool gather_hold_centres_stick();

} // namespace mhfu::game
