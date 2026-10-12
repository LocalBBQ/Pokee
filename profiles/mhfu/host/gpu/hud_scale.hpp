#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace mhfu::gpu::hud_scale {

// Quest interface pieces, each at its own size, pinned to the corner it
// already occupies. Traced on USA Freedom Unite (ULUS-10391) with
// MHFU_TRACE_SPRITES: a quest's base camp (presented frames 2280 and 2850)
// and a hunting area (frame 4430, the area map on the right). The Portable
// 3rd HUD classification does not match this executable.
//
// The quest clock is the round gauge at (8, 8)-(60, 56), to the left of the
// bars. The hunter's name is the plate under it at (8, 56)-(104, 72), with
// the glyphs on that plate. Health is the bar at (56, 12)-(320, 20), stamina
// the bar at (56, 21)-(320, 29), each with its fill and its end cap. The
// clock, the name and the bars grow from the clock's corner so they stay
// together.
// The area map is the tile at (316, 32)-(468, 184), on the right of a quest,
// and a mark drawn on it, or the window that slides inside that slot as the
// hunter moves. The window, the arrow and the camp icon grow from the slot's
// top-right, (468, 32). With the Map item a texel of the area-map texture is
// drawn at the slot's top-left plus that texel, so the picture stays put
// while the window slides over it. Without the Map item the same texels are
// drawn larger, 40 texels at 60 pixels from (276, 70). That draw is put back
// on the slot's placement before the Map size, and the arrow and the camp
// icon drawn with it come back with it. The camp icon keeps the pixel size
// the game gave it; the arrow's vertices are in the enlarged picture. Each
// area draws that window at its own size; the bounds are the rectangle drawn,
// not one fixed size. A panel of the same size from any other texture is not
// it, so the shop and the return-to-town prompt stay as drawn even while the
// quest clock is up. A mark in the slot keeps the map's size even when this
// frame's window does not cover it, and the player's arrow still follows the
// map when the turned quad crosses the edge. In camp that tile is absent; the
// supply icons that sit in the same rectangle stay as drawn.
// Sharpness is the colored bar at y 44, (62, 44)-(166, 50), and the frame
// just above it at (60, 29)-(104, 41). It stays on the stamina bar: when
// health changes size, the bar and the sharpness move together, and the
// weapon size then opens or closes the space inside that stack. The long
// sword's spirit gauge is the red bar under stamina; that weapon was not out
// in the camp trace, so the slot under stamina is what moves with it. Bowgun
// ammo and bow coatings stay on the item bar the same way, and the renderer
// gives them the item bar's size. A mark on the
// area map is not one of those icons.
//
// A corner of the screen is not enough. Menus draw their own pieces there,
// and those stay at the size the game chose. A piece is scaled only while
// its own sprites are on screen.

enum class Piece { None, Map, Vitals, ItemBar, Sharpness, LongSword, Bowgun };

[[nodiscard]] inline const char *piece_name(Piece piece) {
    switch (piece) {
    case Piece::None: return "none";
    case Piece::Map: return "map";
    case Piece::Vitals: return "status";
    case Piece::ItemBar: return "items";
    case Piece::Sharpness: return "sharpness";
    case Piece::LongSword: return "weapon";
    case Piece::Bowgun: return "bowgun";
    }
    return "none";
}

struct Rect {
    float x0{};
    float y0{};
    float x1{};
    float y1{};
};

// Where a piece stays put while it shrinks or grows.
struct Anchor {
    float x{};
    float y{};
};

// The area map, on the right of a quest: (316, 32)-(468, 184), 152 by 152.
// The small window samples this texture in VRAM (256 by 256). Traced with
// MHFU_TRACE_SPRITES across a hunt, the monster-found state and a bowgun:
// every window of that map used 0x0419C200, and the interface panels of the
// same size did not.
inline constexpr std::uint32_t kAreaMapTexture = 0x0419C200u;
// The atlas for the marks drawn on that map: the player's arrow, the camp
// icon, a monster dot. Traced with MHFU_TRACE_SPRITES. The notice's letters
// are a different texture, so a mark of a letter's size is still a mark.
inline constexpr std::uint32_t kMapIconTexture = 0x091874A0u;
inline constexpr float kMapLeft = 316.0f;
inline constexpr float kMapTop = 32.0f;
inline constexpr float kMapRight = 468.0f;
inline constexpr float kMapBottom = 184.0f;
// The quest clock, the round gauge to the left of the health bar.
inline constexpr float kClockLeft = 8.0f;
inline constexpr float kClockTop = 8.0f;
inline constexpr float kClockRight = 60.0f;
inline constexpr float kClockBottom = 56.0f;
// The hunter's name under the clock: the plate (8, 56)-(104, 72), the mark
// at its left, and the glyphs on it. Traced in the same camp frame.
inline constexpr float kNameLeft = 4.0f;
inline constexpr float kNameTop = 54.0f;
inline constexpr float kNameRight = 112.0f;
inline constexpr float kNameBottom = 76.0f;
inline constexpr float kItemLeft = 312.0f;
inline constexpr float kItemTop = 200.0f;
inline constexpr float kItemRight = 472.0f;
inline constexpr float kItemBottom = 264.0f;
// Health and stamina, including the end caps. The health bar is
// (56, 12)-(320, 20) at 100, and (56, 12)-(399, 20) when maximum health is
// raised; stamina is (56, 21)-(320, 29) at 100. Both draw 2.64 pixels per
// point, so either one raised to 150 reaches x 452. Each end cap is 24 by 12
// past the bar's end, (320, 9)-(344, 21) and (320, 21)-(344, 33) at 100, so at
// 150 the caps reach x 476. Sharpness begins at y 29 and runs past this, so
// it is not inside.
inline constexpr float kVitalLeft = 52.0f;
inline constexpr float kVitalTop = 6.0f;
inline constexpr float kRaisedVitalRight = 480.0f;
inline constexpr float kVitalBottom = 36.0f;
// The sharpness frame and the colored bar under it.
inline constexpr float kSharpLeft = 60.0f;
inline constexpr float kSharpTop = 29.0f;
// The long sword spirit gauge, in the band under stamina and clear of the
// sharpness color bar's own rules below. A bowgun's loaded shots use the same
// band: nine 8 by 16 rounds at (60, 32)-(132, 48) over a frame at
// (60, 32)-(164, 44), traced in a hunting area with a bowgun out.
inline constexpr float kSpiritLeft = 56.0f;
inline constexpr float kSpiritTop = 30.0f;
// A bow's ninth single coating ends at x 244.
inline constexpr float kSpiritRight = 256.0f;
inline constexpr float kSpiritBottom = 52.0f;
// Bowgun ammo and bow coatings, on the right above the item bar. The ammo
// bar, with the shot's name and count, is (312, 184)-(472, 200), traced in a
// hunting area with a bowgun out. The supply icons in camp sit above this
// band (from y 85).
inline constexpr float kBowgunLeft = 312.0f;
inline constexpr float kBowgunTop = 96.0f;
inline constexpr float kBowgunRight = 472.0f;
inline constexpr float kBowgunBottom = 200.0f;

[[nodiscard]] inline Rect normalized(Rect rect) {
    return {std::min(rect.x0, rect.x1), std::min(rect.y0, rect.y1), std::max(rect.x0, rect.x1),
        std::max(rect.y0, rect.y1)};
}

[[nodiscard]] inline bool contains(Rect outer, Rect inner) {
    const Rect o = normalized(outer);
    const Rect i = normalized(inner);
    return i.x0 >= o.x0 && i.y0 >= o.y0 && i.x1 <= o.x1 && i.y1 <= o.y1;
}

// True when the two rectangles share any area. A mark can poke a pixel or two
// past the map window and still belong on it.
[[nodiscard]] inline bool overlaps(Rect a, Rect b) {
    a = normalized(a);
    b = normalized(b);
    return a.x0 < b.x1 && b.x0 < a.x1 && a.y0 < b.y1 && b.y0 < a.y1;
}

// The large area-map tile, (316, 32)-(468, 184). The small window is
// travelling_map_window(); it counts only while the quest HUD is up and the
// quad samples kAreaMapTexture. The shop and the return-to-town prompt draw
// panels of that same size in this corner, from other textures.
[[nodiscard]] inline bool is_fixed_map_sprite(Rect rect) {
    const Rect r = normalized(rect);
    const float width = r.x1 - r.x0;
    const float height = r.y1 - r.y0;
    return r.x0 >= 300.0f && r.x0 <= 340.0f && r.y0 >= 20.0f && r.y0 <= 48.0f && width >= 140.0f && width <= 164.0f &&
        height >= 140.0f && height <= 164.0f;
}

// For a few frames as a monster is found, the same texels are drawn at 60 by
// 60 from (276, 70). Only a pixel around that quad is accepted. The panel at
// (280, 16), 176 by 120, starts just as far left and is not the map: it uses
// another texture. Traced with MHFU_TRACE_SPRITES.
[[nodiscard]] inline bool is_found_monster_window(Rect rect) {
    const Rect r = normalized(rect);
    const float width = r.x1 - r.x0;
    const float height = r.y1 - r.y0;
    if (width < 59.0f || width > 61.0f || height < 59.0f || height > 61.0f) return false;
    return r.x0 >= 275.0f && r.x0 <= 277.0f && r.y0 >= 69.0f && r.y0 <= 71.0f &&
        overlaps({kMapLeft, kMapTop, kMapRight, kMapBottom}, r);
}

// A map window in the area-map slot, larger than a mark. Traced samples are
// about 34 by 40 and 40 by 40, at (432, 76)-(466, 116), (425, 78)-(465, 118)
// and (371, 59)-(411, 99). Other areas draw other sizes from the same
// texture, so the size is not fixed; the large tile is is_fixed_map_sprite.
// The found-monster zoom is the one window accepted left of that slack.
[[nodiscard]] inline bool is_travelling_map_window(Rect rect) {
    if (is_found_monster_window(rect)) return true;
    const Rect r = normalized(rect);
    const float width = r.x1 - r.x0;
    const float height = r.y1 - r.y0;
    if (width < 28.0f || height < 28.0f || width > 200.0f || height > 200.0f || is_fixed_map_sprite(rect))
        return false;
    return r.x0 >= kMapLeft - 24.0f && r.y0 >= kMapTop - 24.0f && r.x1 <= kMapRight + 24.0f &&
        r.y1 <= kMapBottom + 24.0f && overlaps({kMapLeft, kMapTop, kMapRight, kMapBottom}, r);
}

// With the Map item a texel is drawn at (316, 32) plus that texel, so the
// south of an area is drawn below the slot. The slot test ends at y 208, and
// the old screen test ended at y 240. On the bottom edge of Forest and Hills
// zone 1 the window sits lower than that and was left at the game's size
// until the hunter stepped back up. The picture is 480 by 272; the quad can
// sit a little past it, and the edge of the map can clip the window below
// the usual 28. A panel of the same size from another texture is not this.
[[nodiscard]] inline bool is_area_map_texture_window(Rect rect) {
    const Rect r = normalized(rect);
    const float width = r.x1 - r.x0;
    const float height = r.y1 - r.y0;
    if (width < 16.0f || height < 16.0f || width > 200.0f || height > 200.0f || is_fixed_map_sprite(rect))
        return false;
    return r.x0 >= 160.0f && r.y0 >= -16.0f && r.x1 <= 512.0f && r.y1 <= 304.0f;
}

// The area map: (316, 32)-(468, 184), 152 by 152, or a small window into the
// same map texture. The quest clock is neither size.
[[nodiscard]] inline bool is_map_sprite(Rect rect) {
    return is_fixed_map_sprite(rect) || is_travelling_map_window(rect);
}

// The area map drawn this frame: the large tile, a window in the slot, or
// any other window sampled from the area-map texture.
[[nodiscard]] inline bool is_drawn_map(Rect rect, std::uint32_t texture) {
    return is_map_sprite(rect) || (texture == kAreaMapTexture && is_area_map_texture_window(rect));
}

// The quest clock: (8, 8)-(60, 56), 52 by 48, with a little room for the
// position to sit a pixel or two off.
[[nodiscard]] inline bool is_clock_sprite(Rect rect) {
    const Rect r = normalized(rect);
    const float width = r.x1 - r.x0;
    const float height = r.y1 - r.y0;
    return r.x0 >= 4.0f && r.x0 <= 14.0f && r.y0 >= 4.0f && r.y0 <= 14.0f && width >= 46.0f && width <= 58.0f &&
        height >= 42.0f && height <= 54.0f;
}

// The yellow eye shown once a monster is found: 12 by 16 at (104, 56)-(116, 72),
// on the right end of the name plate. The plate's rectangle stops at x 112, so
// the name's rule leaves the eye where the game drew it. Traced with
// MHFU_TRACE_SPRITES on the frames after the monster is found.
[[nodiscard]] inline bool is_monster_eye(Rect rect) {
    const Rect r = normalized(rect);
    const float width = r.x1 - r.x0;
    const float height = r.y1 - r.y0;
    return width >= 10.0f && width <= 16.0f && height >= 14.0f && height <= 18.0f && r.x0 >= 98.0f && r.x0 <= 114.0f &&
        r.y0 >= 52.0f && r.y0 <= 64.0f;
}

// Poison, paralysis, sleep, stun, snowman, muddy, stench, defense down and
// fatigue, on the name row. The yellow eye is 12 by 16 at (104, 56) and the
// queued-note mark is 12 by 16 at (128, 56); these icons are the same kind of
// mark, about 12 wide and 14 to 16 tall, stepping right from the plate. The
// note mark is recognised first and keeps the horn's size, which is this same
// percent. A bubble or the stars drawn on the hunter are not this row.
[[nodiscard]] inline bool is_status_icon(Rect rect) {
    const Rect r = normalized(rect);
    const float width = r.x1 - r.x0;
    const float height = r.y1 - r.y0;
    return width >= 10.0f && width <= 16.0f && height >= 12.0f && height <= 18.0f && r.x0 >= 112.0f &&
        r.x0 <= 236.0f && r.y0 >= 52.0f && r.y0 <= 64.0f;
}

// The long health and stamina bars, frames and fills. The fills are two pixels
// tall; a menu strip in the same corner is not this wide and this short.
[[nodiscard]] inline bool is_vital_bar(Rect rect) {
    const Rect r = normalized(rect);
    const float width = r.x1 - r.x0;
    const float height = r.y1 - r.y0;
    return width >= 200.0f && width <= 410.0f && height >= 2.0f && height <= 14.0f && r.x0 >= 48.0f && r.x0 <= 72.0f &&
        contains({kVitalLeft, kVitalTop, kRaisedVitalRight, kVitalBottom}, r);
}

// A piece of health or stamina, out to where a raised bar's cap ends. Bars,
// fills and frames start at the bars' left edge; a cap is 24 by 12 with its
// top on the health or stamina row; a segment inside a bar is at most four
// pixels tall. Other panels share this band while the bars are up, traced
// with MHFU_TRACE_HUD_SCALE: the item box's top edge, (216, 28)-(456, 36),
// and a message window at (152, 8)-(328, 32) with 7 by 14 glyphs on it.
// They are none of these.
[[nodiscard]] inline bool is_vital_piece(Rect rect) {
    const Rect r = normalized(rect);
    const float width = r.x1 - r.x0;
    const float height = r.y1 - r.y0;
    if (!contains({kVitalLeft, kVitalTop, kRaisedVitalRight, kVitalBottom}, r) || width < 2.0f || height < 2.0f)
        return false;
    const bool bar = r.x0 >= 48.0f && r.x0 <= 72.0f && height <= 14.0f;
    const bool cap = width >= 20.0f && width <= 28.0f && height >= 10.0f && height <= 14.0f &&
        ((r.y0 >= 6.0f && r.y0 <= 12.0f) || (r.y0 >= 18.0f && r.y0 <= 24.0f));
    const bool segment = height <= 4.0f && r.y0 >= 10.0f && r.y0 <= 28.0f;
    return bar || cap || segment;
}

// The item list ("Please select an item") is a window on the right of the
// quest picture. Its lower edge lies in the item bar's band and reaches
// left of the bar, so a piece of it is not the bar. A full-screen fade is
// not one either.
[[nodiscard]] inline bool is_item_list(Rect rect) {
    const Rect r = normalized(rect);
    const float width = r.x1 - r.x0;
    const float height = r.y1 - r.y0;
    if (width >= 420.0f || height >= 250.0f || width < 24.0f || height < 4.0f) return false;
    if (r.x0 > 300.0f || r.x1 < kItemLeft + 8.0f) return false;
    return r.y1 >= kItemTop && r.y0 <= kItemBottom;
}

// The wide panel pieces the item bar is built from (the 128-wide strips and
// the open bar's centre). A menu prompt in the same corner is not one of these.
[[nodiscard]] inline bool is_bar_panel(Rect rect) {
    const Rect r = normalized(rect);
    const float width = r.x1 - r.x0;
    const float height = r.y1 - r.y0;
    return width >= 100.0f && width <= 140.0f && height >= 6.0f && height <= 56.0f && r.x0 >= kItemLeft &&
        r.x0 <= 340.0f && contains({kItemLeft, kItemTop, kItemRight, kItemBottom}, r);
}

// The frame just over the colored bar: (60, 29)-(104, 41) and (60, 32)-(104, 40).
// A gunlance draws only these two, the first one tinted with the sharpness
// color, and no long color bar. Traced in a hunting area with a gunlance out.
[[nodiscard]] inline bool is_sharpness_frame(Rect rect) {
    const Rect r = normalized(rect);
    const float width = r.x1 - r.x0;
    const float height = r.y1 - r.y0;
    return width >= 36.0f && width <= 52.0f && height >= 6.0f && height <= 16.0f && r.x0 >= 56.0f && r.x0 <= 68.0f &&
        r.y0 >= 26.0f && r.y0 <= 36.0f && r.y1 <= 44.0f;
}

// The colored bar itself, (66, 44)-(162, 50).
[[nodiscard]] inline bool is_sharpness_bar(Rect rect) {
    const Rect r = normalized(rect);
    const float width = r.x1 - r.x0;
    const float height = r.y1 - r.y0;
    return width >= 70.0f && width <= 120.0f && height >= 4.0f && height <= 12.0f && r.x0 >= 58.0f && r.x0 <= 80.0f &&
        r.y0 >= 40.0f && r.y1 <= 54.0f;
}

// The caps at either end of that bar.
[[nodiscard]] inline bool is_sharpness_cap(Rect rect) {
    const Rect r = normalized(rect);
    const float width = r.x1 - r.x0;
    const float height = r.y1 - r.y0;
    return width <= 8.0f && height >= 4.0f && height <= 10.0f && r.y0 >= 40.0f && r.y1 <= 54.0f && r.x0 >= 56.0f &&
        r.x1 <= 172.0f;
}

[[nodiscard]] inline bool is_sharpness(Rect rect) {
    return is_sharpness_frame(rect) || is_sharpness_bar(rect) || is_sharpness_cap(rect);
}

// A wide bar in the spirit-gauge slot. The sharpness bar is not one: it is
// told apart above.
[[nodiscard]] inline bool is_spirit_bar(Rect rect) {
    const Rect r = normalized(rect);
    const float width = r.x1 - r.x0;
    const float height = r.y1 - r.y0;
    return width >= 40.0f && height >= 2.0f && height <= 16.0f && r.x0 >= 48.0f && r.x0 <= 80.0f &&
        contains({kSpiritLeft, kSpiritTop, kSpiritRight, kSpiritBottom}, r) && !is_sharpness(r);
}

// The long sword's red gauge, drawn inside the sharpness bar. It is two
// pixels tall, from (66, 46) toward (162, 48), and only as wide as the gauge
// is full. A sliver is three pixels wide. Traced with the gauge empty, half
// full and full; full flashes lighter but stays the same rectangle. Too short
// to pass the four-pixel piece rule below.
[[nodiscard]] inline bool is_spirit_fill(Rect rect) {
    const Rect r = normalized(rect);
    const float width = r.x1 - r.x0;
    const float height = r.y1 - r.y0;
    return width >= 2.0f && width <= 110.0f && height >= 1.5f && height <= 3.5f && r.x0 >= 62.0f && r.x0 <= 72.0f &&
        r.y0 >= 44.0f && r.y0 <= 48.0f && r.y1 <= 52.0f && r.x1 <= 176.0f;
}

// One of a bowgun's loaded rounds, 8 by 16, in the row (60, 32)-(132, 48).
// Their frame is not drawn in every frame, so the rounds alone also say the
// weapon slot is on screen. A shop header draws 7 by 14 letters in this band
// ("Type" and the words beside it); those are a pixel shorter, so they are
// not rounds.
[[nodiscard]] inline bool is_loaded_round(Rect rect) {
    const Rect r = normalized(rect);
    const float width = r.x1 - r.x0;
    const float height = r.y1 - r.y0;
    return width >= 6.0f && width <= 10.0f && height >= 15.0f && height <= 18.0f && r.x0 >= 56.0f &&
        r.x0 <= 136.0f && r.y0 >= 30.0f && r.y0 <= 34.0f;
}

// A bow's coating display in the same band, traced frame by frame in a
// hunting area with a bow out. Without a coating it is a 34 by 12 plate at
// (60, 32)-(94, 44). With one it is a large 18 by 16 icon at (60, 30)-(78, 46)
// standing for ten uses, with a count beside it. Once a shot is
// fired the rest of the ten are a row of 12 by 16 single coatings, 14 apart,
// up to nine of them from (120, 30) to (244, 46). Any of them says the slot
// is on screen; the count's glyphs are not drawn on their own. Singles are
// also accepted from the left of the slot, where the large icon would be.
[[nodiscard]] inline bool is_bow_coating(Rect rect) {
    const Rect r = normalized(rect);
    const float width = r.x1 - r.x0;
    const float height = r.y1 - r.y0;
    if (r.y0 < 28.0f || r.y0 > 34.0f) return false;
    const bool plate = width >= 30.0f && width <= 38.0f && height >= 10.0f && height <= 14.0f && r.x0 >= 56.0f &&
        r.x0 <= 64.0f;
    const bool large = width >= 16.0f && width <= 20.0f && height >= 14.0f && height <= 18.0f && r.x0 >= 56.0f &&
        r.x0 <= 64.0f;
    const bool single = width >= 10.0f && width <= 14.0f && height >= 14.0f && height <= 18.0f && r.x0 >= 56.0f &&
        r.x0 <= 236.0f;
    return plate || large || single;
}

// A gunlance's shells, in the same band, to the right of its sharpness.
// Five 8 by 16 icons, 8 apart, from (116, 30) to (156, 46). Loaded and spent
// shells are the same size; only the texture changes. A frame at
// (116, 32)-(220, 44) is drawn in some of those frames and not others, so a
// shell alone also says the slot is on screen. Shop header letters are 7 by
// 14 in this same row; a shell is a pixel taller, so those letters are not
// shells. Traced with MHFU_TRACE_SPRITES.
[[nodiscard]] inline bool is_gunlance_shell(Rect rect) {
    const Rect r = normalized(rect);
    const float width = r.x1 - r.x0;
    const float height = r.y1 - r.y0;
    const bool shell = width >= 6.0f && width <= 10.0f && height >= 15.0f && height <= 18.0f && r.x0 >= 110.0f &&
        r.x0 <= 160.0f && r.y0 >= 28.0f && r.y0 <= 34.0f;
    const bool frame = width >= 96.0f && width <= 112.0f && height >= 10.0f && height <= 14.0f && r.x0 >= 110.0f &&
        r.x0 <= 122.0f && r.y0 >= 30.0f && r.y0 <= 36.0f;
    return shell || frame;
}

// A hunting horn's note staff, traced frame by frame in a hunting area. The
// staff (118, 30)-(246, 46) and the mark at its left, (120, 30)-(138, 48),
// are drawn whenever the horn is out. A new note arrives large at the first
// slot, (145, 21)-(168, 52) then (147, 23)-(166, 49), and settles to 12 by
// 16. Queued notes stagger from (150, 28) to (234, 50); while one is recited
// it grows, to (149, 26)-(164, 47), and the row shifts. A 12 by 16 mark at
// (128, 56)-(140, 72) blinks once a note is queued. Any of them says the
// slot is on screen. Note colours are the sprite's colour, not its size.
[[nodiscard]] inline bool is_hunting_horn(Rect rect) {
    const Rect r = normalized(rect);
    const float width = r.x1 - r.x0;
    const float height = r.y1 - r.y0;
    const bool staff = width >= 120.0f && width <= 140.0f && height >= 14.0f && height <= 18.0f && r.x0 >= 112.0f &&
        r.x0 <= 124.0f && r.y0 >= 28.0f && r.y0 <= 34.0f;
    const bool clef = width >= 16.0f && width <= 22.0f && height >= 16.0f && height <= 22.0f && r.x0 >= 116.0f &&
        r.x0 <= 126.0f && r.y0 >= 28.0f && r.y0 <= 34.0f;
    // The arrival is 23 by 31, the widest a note gets, and taller than a
    // settled one. The shop's equip confirmation draws a 24 by 24 icon at
    // (240, 32)-(264, 56), in this same band; it is a pixel wider than a note.
    // A queued note's far corner is about (234, 50), so its left edge stays
    // near x 222. The equipment-set list draws its weapon icon just past that
    // row, about 16 pixels square near (240, 28), and that icon is not a note.
    const bool note = width >= 10.0f && width <= 23.0f && height >= 14.0f && height <= 34.0f && r.x0 >= 140.0f &&
        r.x0 <= 230.0f && r.y0 >= 18.0f && r.y0 <= 38.0f;
    const bool mark = width >= 10.0f && width <= 16.0f && height >= 14.0f && height <= 18.0f && r.x0 >= 124.0f &&
        r.x0 <= 136.0f && r.y0 >= 52.0f && r.y0 <= 60.0f;
    return staff || clef || note || mark;
}

// The ammo selection menu, opened over the item bar with a bowgun out: the
// item bar's frame grown to (312, 128)-(472, 264), an arrow at
// (384, 124)-(400, 140), a title at y 140, two rows of shots at
// (312, 154)-(472, 186), the highlight over the chosen row,
// (306, 153)-(478, 173), and the strip of shot icons under them. Traced
// frame by frame in a hunting area with MHFU_TRACE_SPRITES.
inline constexpr float kAmmoMenuLeft = 300.0f;
inline constexpr float kAmmoMenuTop = 120.0f;
inline constexpr float kAmmoMenuRight = 480.0f;
inline constexpr float kAmmoMenuBottom = 264.0f;

// The notice that slides down the picture while returning to camp and while
// an item is carved. Its right border is a 16-wide cap. A short line ends at
// x 312 or x 328. A long line, traced with MHFU_TRACE_HUD_SCALE, ends further
// right: the cap (344, 160)-(360, 168). The 8-tall cap is that border
// wherever it sits on the right. The 16-tall piece between the caps is only
// known at x 312 and x 328; further right it is recognised because it sits
// on an 8-tall cap. The closed item bar's own corner is the 16 by 8 tile at
// y 208, and the item-bar rule keeps that one.
[[nodiscard]] inline bool is_notice_cap(Rect rect) {
    const Rect r = normalized(rect);
    const float width = r.x1 - r.x0;
    const float height = r.y1 - r.y0;
    const bool tile = width >= 14.0f && width <= 18.0f &&
        ((height >= 6.0f && height <= 10.0f) || (height >= 14.0f && height <= 18.0f));
    if (!tile || r.y0 >= 180.0f) return false;
    const bool at_narrow = r.x0 >= 308.0f && r.x0 <= 316.0f;
    const bool at_wide = r.x0 >= 324.0f && r.x0 <= 332.0f;
    const bool thin_right = height <= 10.0f && r.x0 >= 300.0f && r.x0 <= 460.0f;
    return at_narrow || at_wide || thin_right;
}

// A row of the long notice, the part that reaches into the area map. Traced
// while carving a long name: (360, 160)-(456, 168), (376, 168)-(456, 176),
// and the untextured rows (350, 46)-(466, 61) and (350, 95)-(466, 112).
// A mark on the map is far smaller than these.
[[nodiscard]] inline bool is_long_notice(Rect rect) {
    const Rect r = normalized(rect);
    const float width = r.x1 - r.x0;
    const float height = r.y1 - r.y0;
    return width >= 48.0f && height >= 4.0f && height <= 24.0f && r.x0 >= 300.0f && r.x1 <= 472.0f;
}

// A letter, 7 by 14. The notice's text is these. The player's arrow, turned
// to point up or down, is about 10 by 12, the same box; it is told apart by
// its texture. The ammo bar's own letters sit on the bar, at y 184 and below.
[[nodiscard]] inline bool is_text_glyph(Rect rect) {
    const Rect r = normalized(rect);
    const float width = r.x1 - r.x0;
    const float height = r.y1 - r.y0;
    return width >= 6.0f && width <= 10.0f && height >= 12.0f && height <= 16.0f;
}

// A mark on the area map: the player's arrow, the camp icon, a monster dot.
// The camp icon is about 32 pixels. A window is 28 or wider, but the panels
// that must stay as drawn (the shop, the return-to-town prompt) are 40, so
// 36 keeps the icon and leaves those panels to the window test.
[[nodiscard]] inline bool is_map_mark(Rect rect) {
    const Rect r = normalized(rect);
    const float width = r.x1 - r.x0;
    const float height = r.y1 - r.y0;
    return width >= 4.0f && height >= 4.0f && width <= 36.0f && height <= 36.0f;
}

// The arrow can leave the top of the slot. The band under the map is the ammo
// bar and a menu checkbox, not a mark on the map.
[[nodiscard]] inline bool in_map_slot(Rect rect) {
    const Rect r = normalized(rect);
    if (r.y0 >= kMapBottom - 8.0f) return false;
    return overlaps({kMapLeft - 16.0f, kMapTop - 16.0f, kMapRight + 16.0f, kMapBottom}, r);
}

// The ammo menu's top-left corner, 16 by 8 at y 128. The notice's border is
// the same tile at other rows, so only this row opens the menu. The closed
// item bar draws it at y 208.
[[nodiscard]] inline bool is_ammo_menu_corner(Rect rect) {
    const Rect r = normalized(rect);
    const float width = r.x1 - r.x0;
    const float height = r.y1 - r.y0;
    return width >= 14.0f && width <= 18.0f && height >= 6.0f && height <= 10.0f && r.x0 >= 310.0f && r.x0 <= 314.0f &&
        r.y0 >= 124.0f && r.y0 <= 136.0f;
}

// A sprite only the open menu draws, apart from its corner: the tall left
// side of the frame and the highlight bar. The corner is the first sprite,
// so the frame is recognised before any of it is drawn.
[[nodiscard]] inline bool is_ammo_menu_body(Rect rect) {
    const Rect r = normalized(rect);
    const float width = r.x1 - r.x0;
    const float height = r.y1 - r.y0;
    const bool side = width >= 14.0f && width <= 18.0f && height >= 80.0f && height <= 140.0f && r.x0 >= 310.0f &&
        r.x0 <= 314.0f && r.y1 >= 254.0f && r.y1 <= 258.0f;
    const bool highlight = width >= 150.0f && width <= 162.0f && height >= 18.0f && height <= 22.0f &&
        r.x0 >= 310.0f && r.x0 <= 318.0f && r.y0 >= 136.0f && r.y0 <= 190.0f;
    return side || highlight;
}

[[nodiscard]] inline bool is_ammo_menu_sprite(Rect rect) {
    return is_ammo_menu_corner(rect) || is_ammo_menu_body(rect);
}

// The ammo bar above the item bar: (312, 184)-(472, 200), a wide strip in the
// band the area map does not cover. Traced with a bowgun out. The equipment
// colour menu draws a checkbox in that same band, under the map, about 16
// pixels square; that checkbox is not the bar.
[[nodiscard]] inline bool is_bowgun_icon(Rect rect) {
    const Rect r = normalized(rect);
    const float width = r.x1 - r.x0;
    const float height = r.y1 - r.y0;
    return width >= 100.0f && width <= 170.0f && height >= 8.0f && height <= 24.0f && r.x0 >= 300.0f &&
        r.x0 <= 340.0f && r.y0 >= 176.0f && r.y0 <= 192.0f &&
        contains({kBowgunLeft, kBowgunTop, kBowgunRight, kBowgunBottom}, r);
}

// The notice's border and its letters share the map's rectangle. A mark from
// the map-icon atlas is neither, including the arrow when its box is a
// letter's size.
[[nodiscard]] inline bool is_notice_piece(Rect rect, std::uint32_t texture) {
    if (texture == kMapIconTexture) return false;
    return is_notice_cap(rect) || is_long_notice(rect) || is_text_glyph(rect);
}

[[nodiscard]] inline Piece piece_of(Rect rect, std::uint32_t texture = 0) {
    const Rect r = normalized(rect);
    const float width = r.x1 - r.x0;
    const float height = r.y1 - r.y0;
    if (is_map_sprite(r)) return Piece::Map;
    if (is_sharpness(r)) return Piece::Sharpness;
    // Notes sit above the weapon slot and the queued-note mark sits below it,
    // so the slot's rectangle does not cover them.
    if (is_hunting_horn(r) || is_spirit_fill(r)) return Piece::LongSword;
    if (is_clock_sprite(r)) return Piece::Vitals;
    // The health and stamina fills are two pixels tall. The clock's hand is
    // drawn inside the gauge.
    if (is_vital_piece(r) ||
        (width >= 2.0f && height >= 2.0f && contains({kClockLeft, kClockTop, kClockRight, kClockBottom}, r)))
        return Piece::Vitals;
    if (width < 4.0f || height < 4.0f) return Piece::None;
    // The hunter's name under the clock. It grows from the clock's corner
    // with the bars, so it stays under the clock. The yellow eye is the icon
    // just past the plate's right end, and the status icons continue along
    // that row.
    if (contains({kNameLeft, kNameTop, kNameRight, kNameBottom}, r) || is_monster_eye(r) || is_status_icon(r))
        return Piece::Vitals;
    // A marker drawn on the map, inside the tile. The player's arrow turns, so
    // its box can cross the edge; that one is caught from the map's own
    // bounds in on_map. The notice's right border and its letters cross this
    // rectangle while the window slides; a mark is neither of those.
    if (contains({kMapLeft, kMapTop, kMapRight, kMapBottom}, r))
        return is_notice_piece(r, texture) ? Piece::None : Piece::Map;
    if (contains({kItemLeft, kItemTop, kItemRight, kItemBottom}, r)) return Piece::ItemBar;
    if (contains({kSpiritLeft, kSpiritTop, kSpiritRight, kSpiritBottom}, r) && !is_sharpness(r))
        return Piece::LongSword;
    // The notice's border and the letters above the ammo bar cross this slot
    // too. The ammo bar's own letters are the glyphs at y 184 and below.
    if (contains({kBowgunLeft, kBowgunTop, kBowgunRight, kBowgunBottom}, r))
        return is_notice_cap(r) || (is_text_glyph(r) && r.y1 <= 176.0f) ? Piece::None : Piece::Bowgun;
    return Piece::None;
}

[[nodiscard]] inline Anchor anchor_of(Piece piece) {
    if (piece == Piece::Map) return {kMapRight, kMapTop};
    if (piece == Piece::Vitals) return {kClockLeft, kClockTop};
    if (piece == Piece::ItemBar) return {kItemRight, kItemBottom};
    if (piece == Piece::Sharpness || piece == Piece::LongSword) return {kSharpLeft, kSharpTop};
    if (piece == Piece::Bowgun) return {kBowgunRight, kBowgunBottom};
    return {};
}

// One size per piece. 1 is the size the game draws.
struct Scales {
    float map{1.0f};
    float vitals{1.0f};
    float item_bar{1.0f};
    float sharpness{1.0f};
    float long_sword{1.0f};
    float bowgun{1.0f};
    // Names over other hunters. 1 is the size the game draws. The renderer
    // scales those draws on their own, about the point the game placed the
    // name, so they are not a rectangle in piece_of.
    float names{1.0f};

    [[nodiscard]] static Scales uniform(float scale) {
        return {scale, scale, scale, scale, scale, scale, scale};
    }
    // The three sizes a player sets, in percent. The clock, health and the
    // weapon pieces under stamina are one group, so they stay together. The
    // bowgun's ammo bar and the bow's coatings sit on the item bar, and the
    // ammo bar opens into the ammo menu over it, so they take its size.
    [[nodiscard]] static Scales from_percent(
        std::uint32_t map, std::uint32_t health, std::uint32_t item_bar, std::uint32_t names = 100u) {
        const float group = static_cast<float>(health) * 0.01f;
        const float bar = static_cast<float>(item_bar) * 0.01f;
        return {static_cast<float>(map) * 0.01f, group, bar, group, group, bar, static_cast<float>(names) * 0.01f};
    }
    [[nodiscard]] bool any() const {
        return map != 1.0f || vitals != 1.0f || item_bar != 1.0f || sharpness != 1.0f || long_sword != 1.0f ||
            bowgun != 1.0f || names != 1.0f;
    }

    [[nodiscard]] float of(Piece piece) const {
        switch (piece) {
        case Piece::Map: return map;
        case Piece::Vitals: return vitals;
        case Piece::ItemBar: return item_bar;
        case Piece::Sharpness: return sharpness;
        case Piece::LongSword: return long_sword;
        case Piece::Bowgun: return bowgun;
        case Piece::None: return 1.0f;
        }
        return 1.0f;
    }
};

// `scale` is 1 at the game's size. The anchor point does not move.
inline void scale_point(float &x, float &y, Anchor anchor, float scale) {
    x = anchor.x + (x - anchor.x) * scale;
    y = anchor.y + (y - anchor.y) * scale;
}

// A weapon piece is measured at `origin` and drawn at `attach`. Sharpness and
// the spirit gauge are the stack under stamina, so their origin follows the
// stamina bar when health changes size. Bowgun ammo and coatings are the
// stack above the item bar, so theirs follows that bar. `scale` then opens or
// closes the space inside the stack. When health and the weapon size match,
// the left stack moves as the stamina bar does. The same is true of the
// bowgun stack and the item bar.
struct Placement {
    float origin_x{};
    float origin_y{};
    float attach_x{};
    float attach_y{};
};

[[nodiscard]] inline Placement placement_of(Piece piece, const Scales &scales) {
    if (piece == Piece::Sharpness || piece == Piece::LongSword) {
        const float attach_x = kClockLeft + (kSharpLeft - kClockLeft) * scales.vitals;
        const float attach_y = kClockTop + (kSharpTop - kClockTop) * scales.vitals;
        return {kSharpLeft, kSharpTop, attach_x, attach_y};
    }
    if (piece == Piece::Bowgun) {
        const float attach_x = kItemRight + (kBowgunRight - kItemRight) * scales.item_bar;
        const float attach_y = kItemBottom + (kBowgunBottom - kItemBottom) * scales.item_bar;
        return {kBowgunRight, kBowgunBottom, attach_x, attach_y};
    }
    const Anchor anchor = anchor_of(piece);
    return {anchor.x, anchor.y, anchor.x, anchor.y};
}

inline void place_point(float &x, float &y, Placement place, float scale) {
    x = place.attach_x + (x - place.origin_x) * scale;
    y = place.attach_y + (y - place.origin_y) * scale;
}

// A name over a hunter: a glyph is about 8 by 14, and the plate is about 16
// tall and under 160 wide. The lobby callback that files that name also files
// the village windows. Those are taller or wider, so they are not a name tag.
[[nodiscard]] inline bool is_name_tag(Rect rect) {
    const Rect r = normalized(rect);
    const float width = r.x1 - r.x0;
    const float height = r.y1 - r.y0;
    return width >= 4.0f && width <= 160.0f && height >= 4.0f && height <= 24.0f;
}

// A teammate's name, plate and glyphs together. `scale` is 1 at the size the
// game draws. With an origin, every vertex moves toward the point the game
// placed the text, so the letters stay on that point and the name gets
// shorter. Without one, the draw shrinks about its own centre: a plate filed
// on its own has no text entry.
template <typename Vertex>
void scale_name(std::vector<Vertex> &vertices, float scale, bool has_origin, float origin_x, float origin_y) {
    if (vertices.empty() || scale == 1.0f) return;
    float anchor_x = origin_x;
    float anchor_y = origin_y;
    if (!has_origin) {
        float x0 = vertices.front().x;
        float y0 = vertices.front().y;
        float x1 = x0;
        float y1 = y0;
        for (const Vertex &vertex : vertices) {
            x0 = std::min(x0, vertex.x);
            y0 = std::min(y0, vertex.y);
            x1 = std::max(x1, vertex.x);
            y1 = std::max(y1, vertex.y);
        }
        anchor_x = 0.5f * (x0 + x1);
        anchor_y = 0.5f * (y0 + y1);
    }
    for (Vertex &vertex : vertices) {
        vertex.x = anchor_x + (vertex.x - anchor_x) * scale;
        vertex.y = anchor_y + (vertex.y - anchor_y) * scale;
    }
}

// `scissor` is exclusive on x1 and y1, in the same pixels as the vertices.
// A scaled piece must not stay clipped to the rectangle the game set, or a
// larger map is cut back to the size the game drew.
inline void include_in_scissor(float &x0, float &y0, float &x1, float &y1, Rect scaled) {
    const Rect s = normalized(scaled);
    x0 = std::min(x0, s.x0);
    y0 = std::min(y0, s.y0);
    x1 = std::max(x1, s.x1 + 1.0f);
    y1 = std::max(y1, s.y1 + 1.0f);
}

// What the frame just drawn, and the one before it, actually contained. The
// map and the bar are scaled only while their own sprites are being drawn, so
// a menu that replaces them is left alone. The previous frame covers a draw
// that arrives before the panel in the same frame.
struct Frame {
    bool map_now{};
    bool vital_now{};
    bool bar_now{};
    bool sharp_now{};
    bool spirit_now{};
    bool bowgun_now{};
    bool list_now{};
    bool map_live{};
    bool vital_live{};
    bool bar_live{};
    bool sharp_live{};
    bool spirit_live{};
    bool bowgun_live{};
    bool list_live{};
    bool ammo_menu_now{};
    bool travelling_now{};
    // The notice's 8-tall border tiles seen this frame, and whether two of
    // them are the top and bottom of that border (24 pixels apart). A tile
    // at the ammo menu's corner row is that menu only when it has no partner.
    float notice_edge_x[8]{};
    float notice_edge_y[8]{};
    int notice_edges{};
    float notice_edge_x_live[8]{};
    float notice_edge_y_live[8]{};
    int notice_edges_live{};
    bool notice_pair{};
    bool menu_body{};
    // The slot's top-right. The window slides inside the slot as the hunter
    // runs; the map, the arrow and the camp icon grow from this corner, not
    // from the window. `map_rect` is the sprite drawn this frame, so a prompt
    // that is not a mark in the slot is not carried along with the window.
    Anchor map_corner{kMapRight, kMapTop};
    Rect map_rect{};
    Rect map_rect_live{};
    Rect travelling_rect{};
    // Without the Map item the window is an enlargement of the slot's
    // placement: screen = zoom * (slot + texel) + bias. Marks drawn with that
    // window use the same enlargement. 1 means the Map item's own placement.
    bool map_zoomed{};
    float zoom_x{1.0f};
    float zoom_y{1.0f};
    float bias_x{};
    float bias_y{};

    void begin() {
        map_rect_live = map_now ? map_rect : Rect{};
        map_live = map_now;
        vital_live = vital_now;
        bar_live = bar_now;
        sharp_live = sharp_now;
        spirit_live = spirit_now;
        bowgun_live = bowgun_now;
        list_live = list_now;
        map_now = vital_now = bar_now = sharp_now = spirit_now = bowgun_now = list_now = ammo_menu_now =
            travelling_now = false;
        map_rect = {};
        travelling_rect = {};
        map_zoomed = false;
        zoom_x = zoom_y = 1.0f;
        notice_edges_live = notice_edges;
        for (int i = 0; i < notice_edges; ++i) {
            notice_edge_x_live[i] = notice_edge_x[i];
            notice_edge_y_live[i] = notice_edge_y[i];
        }
        notice_edges = 0;
        notice_pair = false;
        menu_body = false;
    }

    // An 8-tall piece of the notice border. The two of them are 24 pixels
    // apart, with the 16-tall piece between. The ammo menu's corner is one
    // such tile with no partner.
    void remember_notice_edge(Rect rect) {
        const Rect r = normalized(rect);
        const float width = r.x1 - r.x0;
        const float height = r.y1 - r.y0;
        if (width < 14.0f || width > 18.0f || height < 6.0f || height > 10.0f || r.y0 >= 180.0f) return;
        if (r.x0 < 300.0f || r.x0 > 460.0f) return;
        for (int i = 0; i < notice_edges; ++i) {
            const float dx = r.x0 > notice_edge_x[i] ? r.x0 - notice_edge_x[i] : notice_edge_x[i] - r.x0;
            const float dy = r.y0 > notice_edge_y[i] ? r.y0 - notice_edge_y[i] : notice_edge_y[i] - r.y0;
            if (dx <= 4.0f && dy >= 20.0f && dy <= 28.0f) notice_pair = true;
        }
        if (notice_edges < 8) {
            notice_edge_x[notice_edges] = r.x0;
            notice_edge_y[notice_edges] = r.y0;
            ++notice_edges;
        }
    }

    [[nodiscard]] bool notice_partner(float x, float y) const {
        for (int i = 0; i < notice_edges; ++i) {
            const float dx = x > notice_edge_x[i] ? x - notice_edge_x[i] : notice_edge_x[i] - x;
            const float dy = y > notice_edge_y[i] ? y - notice_edge_y[i] : notice_edge_y[i] - y;
            if (dx <= 4.0f && dy >= 20.0f && dy <= 28.0f) return true;
        }
        return false;
    }

    // The notice's border passes through the ammo menu's corner on the way
    // down. That tile opens the menu only when it is on its own. The menu's
    // side and highlight are never part of the notice, so they keep it open.
    void dismiss_false_menu() {
        if (!notice_pair || menu_body) return;
        for (int i = 0; i < notice_edges; ++i) {
            const bool corner_row = notice_edge_y[i] >= 124.0f && notice_edge_y[i] <= 136.0f;
            const bool corner_x = notice_edge_x[i] >= 310.0f && notice_edge_x[i] <= 314.0f;
            if (corner_row && corner_x && !notice_partner(notice_edge_x[i], notice_edge_y[i])) return;
        }
        ammo_menu_now = false;
    }

    // The 16-tall piece of a long notice's border, directly above or below
    // one of its 8-tall caps. A mark of that size has no such cap.
    [[nodiscard]] bool beside_notice_edge(Rect rect) const {
        const Rect r = normalized(rect);
        const float width = r.x1 - r.x0;
        const float height = r.y1 - r.y0;
        if (width < 14.0f || width > 18.0f || height < 14.0f || height > 18.0f) return false;
        auto stacked = [&](float edge_x, float edge_y) {
            const float dx = r.x0 > edge_x ? r.x0 - edge_x : edge_x - r.x0;
            if (dx > 4.0f) return false;
            const float above = r.y0 - edge_y;
            const float below = edge_y - r.y0;
            return (above >= 6.0f && above <= 12.0f) || (below >= 12.0f && below <= 20.0f);
        };
        for (int i = 0; i < notice_edges; ++i)
            if (stacked(notice_edge_x[i], notice_edge_y[i])) return true;
        for (int i = 0; i < notice_edges_live; ++i)
            if (stacked(notice_edge_x_live[i], notice_edge_y_live[i])) return true;
        return false;
    }

    // The map's bounds are every quad of that map drawn this frame. The scale
    // anchor stays the slot corner; the union is only which pixels are the map.
    static void include_rect(Rect &dest, Rect incoming, bool &now) {
        const Rect r = normalized(incoming);
        if (!now)
            dest = r;
        else
            dest = {std::min(dest.x0, r.x0), std::min(dest.y0, r.y0), std::max(dest.x1, r.x1), std::max(dest.y1, r.y1)};
        now = true;
    }

    // The enlarged window, traced at 60 pixels for 40 texels. A window already
    // on the slot (pixel span matching the texels) clears it. The first vertex
    // carries the texel drawn at its own pixel, which fixes the bias.
    template <typename Vertex>
    void note_map_zoom(const std::vector<Vertex> &vertices, std::size_t first) {
        map_zoomed = false;
        if constexpr (requires { vertices[first].u; vertices[first].v; }) {
            float x0 = vertices[first].x;
            float x1 = x0;
            float y0 = vertices[first].y;
            float y1 = y0;
            float u0 = vertices[first].u;
            float u1 = u0;
            float v0 = vertices[first].v;
            float v1 = v0;
            for (std::size_t i = 1; i < 6u; ++i) {
                x0 = std::min(x0, vertices[first + i].x);
                x1 = std::max(x1, vertices[first + i].x);
                y0 = std::min(y0, vertices[first + i].y);
                y1 = std::max(y1, vertices[first + i].y);
                u0 = std::min(u0, vertices[first + i].u);
                u1 = std::max(u1, vertices[first + i].u);
                v0 = std::min(v0, vertices[first + i].v);
                v1 = std::max(v1, vertices[first + i].v);
            }
            const float pw = x1 - x0;
            const float ph = y1 - y0;
            const float tw = u1 - u0;
            const float th = v1 - v0;
            if (pw < 8.0f || ph < 8.0f || tw < 8.0f || th < 8.0f) return;
            const float zx = pw / tw;
            const float zy = ph / th;
            if (zx < 1.15f || zy < 1.15f || zx > 2.5f || zy > 2.5f) return;
            if (zx - zy > 0.08f || zy - zx > 0.08f) return;
            zoom_x = zx;
            zoom_y = zy;
            bias_x = vertices[first].x - zx * (kMapLeft + vertices[first].u);
            bias_y = vertices[first].y - zy * (kMapTop + vertices[first].v);
            map_zoomed = true;
        }
    }

    // Put one quad back on the slot's placement. The map window's own texels
    // say where it belongs. A sprite the game drew at its texel size (the camp
    // icon) keeps that size and only its centre moves. The arrow's vertices
    // are in the enlarged picture, so each one comes back.
    template <typename Vertex>
    void unzoom_quad(std::vector<Vertex> &vertices, std::size_t first, bool window) {
        if (!map_zoomed) return;
        if (window) {
            if constexpr (requires { vertices[first].u; vertices[first].v; }) {
                for (std::size_t i = 0; i < 6u; ++i) {
                    vertices[first + i].x = kMapLeft + vertices[first + i].u;
                    vertices[first + i].y = kMapTop + vertices[first + i].v;
                }
                return;
            }
        }
        const bool texel_sized = [&] {
            if constexpr (requires { vertices[first].u; vertices[first].v; }) {
                float x0 = vertices[first].x;
                float x1 = x0;
                float y0 = vertices[first].y;
                float y1 = y0;
                float u0 = vertices[first].u;
                float u1 = u0;
                float v0 = vertices[first].v;
                float v1 = v0;
                for (std::size_t i = 1; i < 6u; ++i) {
                    x0 = std::min(x0, vertices[first + i].x);
                    x1 = std::max(x1, vertices[first + i].x);
                    y0 = std::min(y0, vertices[first + i].y);
                    y1 = std::max(y1, vertices[first + i].y);
                    u0 = std::min(u0, vertices[first + i].u);
                    u1 = std::max(u1, vertices[first + i].u);
                    v0 = std::min(v0, vertices[first + i].v);
                    v1 = std::max(v1, vertices[first + i].v);
                }
                const float tw = u1 - u0;
                const float th = v1 - v0;
                if (tw < 4.0f || th < 4.0f) return false;
                const float rx = (x1 - x0) / tw;
                const float ry = (y1 - y0) / th;
                return rx > 0.8f && rx < 1.25f && ry > 0.8f && ry < 1.25f;
            }
            return false;
        }();
        if (texel_sized) {
            float x0 = vertices[first].x;
            float x1 = x0;
            float y0 = vertices[first].y;
            float y1 = y0;
            for (std::size_t i = 1; i < 6u; ++i) {
                x0 = std::min(x0, vertices[first + i].x);
                x1 = std::max(x1, vertices[first + i].x);
                y0 = std::min(y0, vertices[first + i].y);
                y1 = std::max(y1, vertices[first + i].y);
            }
            const float dx = ((0.5f * (x0 + x1) - bias_x) / zoom_x) - 0.5f * (x0 + x1);
            const float dy = ((0.5f * (y0 + y1) - bias_y) / zoom_y) - 0.5f * (y0 + y1);
            for (std::size_t i = 0; i < 6u; ++i) {
                vertices[first + i].x += dx;
                vertices[first + i].y += dy;
            }
            return;
        }
        for (std::size_t i = 0; i < 6u; ++i) {
            vertices[first + i].x = (vertices[first + i].x - bias_x) / zoom_x;
            vertices[first + i].y = (vertices[first + i].y - bias_y) / zoom_y;
        }
    }

    void note(Rect rect, std::uint32_t texture = 0) {
        if (is_fixed_map_sprite(rect)) {
            include_rect(map_rect, rect, map_now);
            map_corner = {kMapRight, kMapTop};
        } else if (texture == kAreaMapTexture &&
            (is_travelling_map_window(rect) || is_area_map_texture_window(rect)))
            include_rect(travelling_rect, rect, travelling_now);
        if (is_vital_bar(rect) || is_clock_sprite(rect) || is_monster_eye(rect) || is_status_icon(rect))
            vital_now = true;
        if (is_bar_panel(rect)) bar_now = true;
        if (is_sharpness_bar(rect) || is_sharpness_frame(rect)) sharp_now = true;
        if (is_spirit_bar(rect) || is_spirit_fill(rect) || is_loaded_round(rect) || is_bow_coating(rect) ||
            is_gunlance_shell(rect) || is_hunting_horn(rect))
            spirit_now = true;
        if (is_ammo_menu_body(rect)) menu_body = true;
        if (is_ammo_menu_sprite(rect)) ammo_menu_now = true;
        remember_notice_edge(rect);
        if (is_bowgun_icon(rect)) bowgun_now = true;
        if (is_item_list(rect)) list_now = true;
    }

    // A window between the traced spots is the map only while the quest clock
    // or the health bar is on screen, and only when the quad samples the
    // area-map texture. The shop and the return-to-town prompt have panels of
    // the same size in that corner; those panels use other textures.
    void adopt_travelling_map() {
        if (!travelling_now || map_now || !(vital_now || vital_live)) return;
        map_now = true;
        map_corner = {kMapRight, kMapTop};
        map_rect = travelling_rect;
    }

    // A small mark on the map whose box is not wholly inside the large slot.
    // The player's arrow is a turned quad: near the edge its box leaves the
    // slot and piece_of misses it, so it was drawn at the game's size. A mark
    // in the slot still counts when the window has slid off it. The notice's
    // border is not a mark.
    [[nodiscard]] bool on_map(Rect rect, std::uint32_t texture = 0) const {
        if (!(map_now || map_live) || list_now || list_live) return false;
        if (is_notice_piece(rect, texture)) return false;
        if (!is_map_mark(rect)) return false;
        if (overlaps(map_now ? map_rect : map_rect_live, rect)) return true;
        return in_map_slot(rect);
    }

    [[nodiscard]] bool allows(Piece piece, Rect rect, std::uint32_t texture) const {
        switch (piece) {
        // The item list covers the map's side of the picture. A cell of it
        // that lands in the map's rectangle stays as the game drew it. The
        // area-map tile itself is not one of those cells: it still scales
        // while the list is open, so a list-shaped sprite earlier in the
        // frame cannot pin the map at the size the game drew. A window
        // between the traced spots scales only after adopt_travelling_map()
        // has accepted it. A mark in the slot keeps that size when the
        // window has moved off it; one on a window outside the slot follows
        // the window's rectangle.
        case Piece::Map:
            if (is_fixed_map_sprite(rect)) return true;
            // Only the area-map texture is the window. The camp icon is the
            // same sort of size and sits in the slot, but it is another
            // texture, so it is a mark. Without the Map item the window
            // leaves the slot; that one is the texture too.
            if (texture == kAreaMapTexture &&
                (is_travelling_map_window(rect) || is_area_map_texture_window(rect)))
                return (map_now || map_live) && (vital_now || vital_live);
            if ((map_now || map_live) && !list_now && !list_live && is_map_mark(rect) && in_map_slot(rect) &&
                !is_notice_piece(rect, texture))
                return true;
            return (map_now || map_live) && !list_now && !list_live &&
                overlaps(map_now ? map_rect : map_rect_live, rect);
        case Piece::Vitals: return vital_now || vital_live;
        // The item list shares the bar's corner, and its right column shares
        // the bowgun slot. Neither is scaled while that list is on screen.
        case Piece::ItemBar: return (bar_now || bar_live) && !list_now && !list_live;
        case Piece::Sharpness: return sharp_now || sharp_live;
        case Piece::LongSword: return spirit_now || spirit_live;
        case Piece::Bowgun: return (bowgun_now || bowgun_live) && !list_now && !list_live;
        case Piece::None: return false;
        }
        return false;
    }
};

// One through sprite, or one through quad, is six vertices (two triangles)
// after it is expanded. Groups of six are scaled when they are the map or the
// item bar and that piece is on screen.
template <typename Vertex>
void scale_quads(std::vector<Vertex> &vertices, const Scales &scales, Frame &frame, std::uint32_t texture = 0) {
    if (!scales.any() || vertices.size() < 6u) return;
    auto bounds_at = [&](std::size_t first) {
        float x0 = vertices[first].x;
        float y0 = vertices[first].y;
        float x1 = x0;
        float y1 = y0;
        for (std::size_t i = 1; i < 6u; ++i) {
            x0 = std::min(x0, vertices[first + i].x);
            y0 = std::min(y0, vertices[first + i].y);
            x1 = std::max(x1, vertices[first + i].x);
            y1 = std::max(y1, vertices[first + i].y);
        }
        return Rect{x0, y0, x1, y1};
    };
    for (std::size_t first = 0; first + 6u <= vertices.size(); first += 6u) {
        const Rect bounds = bounds_at(first);
        frame.note(bounds, texture);
        if (texture == kAreaMapTexture && is_drawn_map(bounds, texture)) frame.note_map_zoom(vertices, first);
    }
    frame.dismiss_false_menu();
    frame.adopt_travelling_map();
    for (std::size_t first = 0; first + 6u <= vertices.size(); first += 6u) {
        const Rect bounds = bounds_at(first);
        // The ammo menu is one panel over the item bar's corner, sized with
        // the item bar. Only this frame's menu counts: the frame after it
        // closes draws the ammo bar in the same place.
        const bool in_ammo_menu = frame.ammo_menu_now &&
            contains({kAmmoMenuLeft, kAmmoMenuTop, kAmmoMenuRight, kAmmoMenuBottom}, bounds);
        Piece piece = in_ammo_menu ? Piece::ItemBar : piece_of(bounds, texture);
        // A window from the area-map texture that sits outside the slot is
        // still the map. piece_of only knows the slot.
        if (!in_ammo_menu && texture == kAreaMapTexture && is_area_map_texture_window(bounds))
            piece = Piece::Map;
        // The arrow's box crosses the map, or lands in the bowgun slot, while
        // the map it belongs to is the one drawn this frame.
        if (!in_ammo_menu && (piece == Piece::None || piece == Piece::Bowgun) && frame.on_map(bounds, texture))
            piece = Piece::Map;
        // The long notice's 16-tall border sits on its 8-tall cap. It is the
        // size of a map mark, so the mark rule would take it.
        if ((piece == Piece::Map || piece == Piece::Bowgun) && frame.beside_notice_edge(bounds)) continue;
        if (piece == Piece::None || (!in_ammo_menu && !frame.allows(piece, bounds, texture))) continue;
        // The no-map window and the marks on it are enlarged. Put them on the
        // slot's placement first, then apply the Map size. At 100% the
        // placement is the whole change.
        if (piece == Piece::Map && frame.map_zoomed)
            frame.unzoom_quad(vertices, first, texture == kAreaMapTexture && is_drawn_map(bounds, texture));
        const float scale = scales.of(piece);
        const Placement place = piece == Piece::Map
            ? Placement{frame.map_corner.x, frame.map_corner.y, frame.map_corner.x, frame.map_corner.y}
            : placement_of(piece, scales);
        const bool moved = place.attach_x != place.origin_x || place.attach_y != place.origin_y;
        if (scale == 1.0f && !moved) continue;
        for (std::size_t i = 0; i < 6u; ++i) place_point(vertices[first + i].x, vertices[first + i].y, place, scale);
    }
}

} // namespace mhfu::gpu::hud_scale
