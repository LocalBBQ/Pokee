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
    std::uint16_t kind{}; // the halfword at +20; the action switches on 0, 3 and 4
};

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

// How long ○ stays up, in pad reads the game takes, after the hunter can
// gather again. Long enough that a read at the game's own rate sees it up
// before the next press.
inline constexpr unsigned kGatherHoldGap = 3u;

// Holding ○ on a gather point. Gather is ○ (B on an Xbox pad), and the game
// starts one when ○ goes down, so a bit that stays down is one gather. While
// the hunter's action (the halfword at +804) is away from the action this
// hold started on, ○ is held up. When that action is back, ○ stays up for
// kGatherHoldGap reads and then goes down. Away from a node, with L down, or
// with the setting off, the buttons pass through. L+○ is the item bar.
class GatherRepeat {
public:
    // `ready` is the gather search accepting ○. `action` is the hunter's
    // current action; nothing means the action cannot be read.
    [[nodiscard]] std::uint32_t apply(std::uint32_t buttons, bool ready, std::optional<std::uint16_t> action);
    void reset() { *this = GatherRepeat{}; }

private:
    enum class Phase : std::uint8_t { Idle, Down, Busy, Gap };
    Phase phase_{Phase::Idle};
    bool input_down_{};
    std::uint16_t free_action_{};
    unsigned gap_{};
};

// The buttons the game should see for this pad read. Off, or with no node,
// `buttons` comes back unchanged and the gather list is not read.
[[nodiscard]] std::uint32_t gather_hold(std::uint32_t buttons, const psprecomp::GuestMemory &memory);

} // namespace mhfu::game
