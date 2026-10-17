#pragma once

// The sharpness gauge keeps the sharpness the game draws and shortens it as
// sharpness is spent. Off, nothing here runs and that drawing stays whole.
//
// Read from USA Freedom Unite (ULUS-10391). The hunter's sharpness is the
// signed halfword at +4338 (0x10F2) and its most the one at +1548: the routine
// in game_task.ovl around 0x09A67FB8 adds a step to the first, clamps it to
// 0..most, and calls 0x088DAB94 for the colour, which it keeps in the byte at
// +1522. The game applies every rule before that store (how much a weapon's
// move costs, a guard, Razor Sharp, Handicraft's longer bar), so the gauge
// shows what the game counts and models none of it.
//
// 0x088DAB94 finds the colour from the weapon in the hunter's equipment slot
// at +1256 (u8 used, u8 kind, u16 id; kind 5 is a weapon):
//   record = 0x0895A0EC + id * 24      (u16 model at +0, u8 sharpness row at +8)
//   class  = u8 at 0x089785A8 + model
//   table  = u32 at 0x0897935C + class * 4
//   ends   = 7 halfwords at table + row * 14, red to purple, cumulative
// The colour is how many ends lie below the sharpness. Traced with
// MHFU_TRACE_SHARPNESS on a great sword (id 182): ends 50, 80, 210, 330, 390,
// 430, 450 and most 250; 249 points was green and 210 yellow, as the game's
// own byte said.
//
// The game draws that sharpness whole, every frame, from 0x08841100. A long
// sword draws a 96 by 6 bar at (66, 44) (0x088411EC, sprite 419) and a cap at
// its right end, (162, 44). Every blade draws the knife at (60, 32) and the
// colour painted on it at (60, 29). While the gauge is on, the renderer leaves
// the blade and cuts that colour to the fraction of it still left, so a point
// spent takes a point of the colour off its right end, and cuts the long bar
// (moving the cap with it) to the fraction of the whole bar still sharp. The
// colours stay the ones the game drew. A gun has neither. The
// cuts follow the HUD sizes because each is a fraction of the rectangle the
// game drew, and they hide with the HUD because the drawing does.
// MHFU_TRACE_SHARPNESS=1 logs each change of the reading.

#include "game/damage_numbers.hpp"

#include <array>
#include <cstdint>
#include <optional>

namespace psprecomp {
class GuestMemory;
}

namespace mhfu::game {
class Ram;
}

namespace mhfu::game::sharpness {

// The list of hunters and the followed one in it (camera/game_camera.cpp).
inline constexpr std::uint32_t kHunterList = 0x089CBD04u;
inline constexpr std::uint32_t kFollowedHunter = 2692u;
// In the hunter.
inline constexpr std::uint32_t kPoints = 4338u;
inline constexpr std::uint32_t kMostPoints = 1548u;
inline constexpr std::uint32_t kColourByte = 1522u;
inline constexpr std::uint32_t kWeaponSlot = 1256u;
inline constexpr std::uint8_t kWeaponKind = 5u;
// In the executable.
inline constexpr std::uint32_t kWeaponRecords = 0x0895A0ECu;
inline constexpr std::uint32_t kWeaponRecordBytes = 24u;
inline constexpr std::uint32_t kWeaponRow = 8u;
inline constexpr std::uint32_t kModelClasses = 0x089785A8u;
inline constexpr std::uint32_t kClassTables = 0x0897935Cu;
inline constexpr std::uint32_t kClassCount = 11u;
inline constexpr std::uint32_t kRowBytes = 14u;
inline constexpr std::size_t kColours = 7u;

using Ends = std::array<int, kColours>;

struct Reading {
    Sharpness colour{Sharpness::Red};
    float fill{}; // 0..1 of the current colour still left
    int points{}; // the hunter's sharpness
    int low{};    // the colour starts above this
    int high{};   // and ends here, or at the most the hunter can reach
    int most{};
    int game_colour{-1}; // the game's own colour byte, for the trace
    bool operator==(const Reading &) const = default;
};

// The colour index 0..6 for `points`, as 0x088DAB94 counts it: how many ends
// lie below. Past the last end it is purple.
[[nodiscard]] int colour_index(const Ends &ends, int points) noexcept;
// The gauge for `points` of at most `most`. Nothing when the ends are not a
// rising row or `most` is not positive.
[[nodiscard]] std::optional<Reading> gauge_reading(const Ends &ends, int points, int most) noexcept;
// The followed hunter's weapon ends, through the tables above. Nothing when
// the slot does not hold a weapon or a pointer leaves memory.
[[nodiscard]] std::optional<Ends> weapon_ends(const Ram &ram, std::uint32_t hunter);
// The gauge for the hunter at `hunter`.
[[nodiscard]] std::optional<Reading> read_hunter(const Ram &ram, std::uint32_t hunter);
// The followed hunter, or 0 when the list is not in memory yet.
[[nodiscard]] std::uint32_t followed_hunter(const Ram &ram);

// The colour bar the game draws, in its own pixels: 96 wide from x 66.
inline constexpr float kBarLeft = 66.0f;
inline constexpr float kBarWidth = 96.0f;

// The right edge of a bar from `x0` to `x1`, and the texel that sits on it,
// once `remaining` (0..1) of that bar is still sharp. The left edge stays, so
// the colours the game drew keep their widths.
struct Edge {
    float x{};
    float u{};
};
[[nodiscard]] Edge cut_right(float x0, float x1, float u0, float u1, float remaining) noexcept;
// The colour on the knife (sprite 9) is drawn as wide as the knife, handle
// included, but its colour starts where the blade does. In the quest HUD's
// atlas (0x09156BA0) the knife spans texels 108 to 152 and the colour's opaque
// texels run from 117, the blade's first, to 150.
inline constexpr float kBladeTexel = 117.0f;
// cut_right over the blade alone: the left edge is kBladeTexel's place in the
// quad, so with nothing left the colour ends where the blade meets the handle
// instead of at the handle's end. A quad that does not span that texel is cut
// whole, as cut_right does.
[[nodiscard]] Edge cut_blade(float x0, float x1, float u0, float u1, float remaining) noexcept;
// How far the cap at the bar's right end moves when the bar is cut. Negative
// once any of the bar is spent.
[[nodiscard]] float cap_shift(float bar_width, float remaining) noexcept;
// The long sword's red spirit fill, from `x0` to `x1` in the game's pixels, is
// drawn as a share of the whole bar. Once the bar is cut to `remaining`, the
// fill keeps that share of the bar still drawn: this is the fraction of the
// fill's own width to keep, for cut_right. Left alone it ran past the cut end.
[[nodiscard]] float fill_remaining(float x0, float x1, float remaining) noexcept;

// The game pulses the colour's alpha as 64 + 64*sin(...) (0x08841400), so a
// full pulse is 128. With the gauge on, the colour keeps the game's pulse but
// on kFullPulse..kGlowPeak: it never dims below a full pulse, and its crest is
// brighter than the game ever draws the colour.
inline constexpr std::uint32_t kFullPulse = 128u;
inline constexpr std::uint32_t kGlowPeak = 192u;
// The alpha the gauge draws for the game's `game_alpha`.
[[nodiscard]] std::uint32_t glow_alpha(std::uint32_t game_alpha) noexcept;

// After the knife, 0x08841100 paints the colour on it (sprite 9 at (60, 29))
// from a 4-byte entry per colour at kKnifeColours: red, green, blue, and the
// mask the pulse is ANDed with. A zero red byte skips that layer
// (0x08841404). Read from the executable: red FF 00 00 FF, orange and yellow
// all zero, green 5E D3 00 FF, blue 30 68 EE FF, white F0 F0 F0 FF, purple
// F0 00 F0 FF. The knife sprites at kKnifeSprites are 12 for red, 11 for
// orange and 10 for the rest, so yellow's knife is the plain one the colour
// layer tints, and with no layer it shows no colour and no pulse; orange keeps
// its own knife but does not pulse.
inline constexpr std::uint32_t kKnifeSprites = 0x08938978u;
inline constexpr std::uint32_t kKnifeColours = 0x08938980u;
inline constexpr std::uint32_t kKnifeColourBytes = 4u;
// The entries the game leaves empty, as a little-endian word of those bytes
// (mask, blue, green, red from the top). Their strength follows the game's
// own: a full mask, and channels at F0 like white and purple.
struct AddedColour {
    Sharpness colour;
    std::uint32_t entry;
};
inline constexpr std::array<AddedColour, 2> kAddedColours{{
    {Sharpness::Orange, 0xFF0080F0u},
    {Sharpness::Yellow, 0xFF00E0F0u},
}};
// With `fill`, writes kAddedColours over the game's empty entries; without,
// puts the empty entries back. An entry holding anything else (a mod's own
// colour) is left alone either way.
void fill_knife_colours(Ram &ram, bool fill);

// Once a flip, on the game thread. While `enabled`, it also fills in the knife
// colours above; off, it puts the game's back.
void gauge_frame(bool enabled, psprecomp::GuestMemory *memory);

// The fraction of the game's bar still sharp, 0..1, while the gauge is on and
// a blade weapon is out. Negative when the gauge is off or there is no bar.
// The renderer reads this from its own thread.
[[nodiscard]] float bar_remaining() noexcept;
// The fraction of the current colour still left, 0..1, on the same terms. A
// fresh colour is 1, so the knife stays as the game drew it until a point of
// that colour is spent.
[[nodiscard]] float colour_remaining() noexcept;

// For tests: forget the reading.
void reset_for_tests();
void set_reading_for_tests(std::optional<Reading> reading, bool enabled);

} // namespace mhfu::game::sharpness
