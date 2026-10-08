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
// and a mark drawn on it. It grows from its top-right. In camp that tile is
// absent; the supply icons that sit in the same rectangle stay as drawn.
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
// (56, 12)-(320, 20), and (56, 12)-(399, 20) when maximum health is raised;
// stamina stays at (56, 21)-(320, 29). Sharpness begins at y 29 and runs past
// this, so it is not inside.
inline constexpr float kVitalLeft = 52.0f;
inline constexpr float kVitalTop = 6.0f;
inline constexpr float kVitalRight = 408.0f;
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

// The area map: (316, 32)-(468, 184), 152 by 152, or the small one the game
// can show instead, (425, 78)-(465, 118), 40 by 40, a window into the same
// map texture with the player's arrow on it. Traced in a hunting area with
// MHFU_TRACE_SPRITES. Each has a little room for the position to sit a pixel
// or two off. The quest clock is neither size.
[[nodiscard]] inline bool is_map_sprite(Rect rect) {
    const Rect r = normalized(rect);
    const float width = r.x1 - r.x0;
    const float height = r.y1 - r.y0;
    const bool large = r.x0 >= 300.0f && r.x0 <= 340.0f && r.y0 >= 20.0f && r.y0 <= 48.0f && width >= 140.0f &&
        width <= 164.0f && height >= 140.0f && height <= 164.0f;
    const bool small = r.x0 >= 418.0f && r.x0 <= 432.0f && r.y0 >= 70.0f && r.y0 <= 86.0f && width >= 36.0f &&
        width <= 46.0f && height >= 36.0f && height <= 46.0f;
    return large || small;
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

// The long health and stamina bars, frames and fills. The fills are two pixels
// tall; a menu strip in the same corner is not this wide and this short.
[[nodiscard]] inline bool is_vital_bar(Rect rect) {
    const Rect r = normalized(rect);
    const float width = r.x1 - r.x0;
    const float height = r.y1 - r.y0;
    return width >= 200.0f && width <= 360.0f && height >= 2.0f && height <= 14.0f && r.x0 >= 48.0f && r.x0 <= 72.0f &&
        contains({kVitalLeft, kVitalTop, kVitalRight, kVitalBottom}, r);
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
    const bool note = width >= 10.0f && width <= 23.0f && height >= 14.0f && height <= 34.0f && r.x0 >= 140.0f &&
        r.x0 <= 250.0f && r.y0 >= 18.0f && r.y0 <= 38.0f;
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

// A sprite only the open menu draws. Its frame's top-left corner, 16 by 8,
// is the first sprite of the menu, so the frame is recognised before any of
// it is drawn; the closed item bar draws the same corner at y 208. The tall
// left side of the frame and the highlight bar also count.
[[nodiscard]] inline bool is_ammo_menu_sprite(Rect rect) {
    const Rect r = normalized(rect);
    const float width = r.x1 - r.x0;
    const float height = r.y1 - r.y0;
    const bool corner = width >= 14.0f && width <= 18.0f && height >= 6.0f && height <= 10.0f && r.x0 >= 310.0f &&
        r.x0 <= 314.0f && r.y0 >= 120.0f && r.y0 <= 160.0f;
    const bool side = width >= 14.0f && width <= 18.0f && height >= 80.0f && height <= 140.0f && r.x0 >= 310.0f &&
        r.x0 <= 314.0f && r.y1 >= 254.0f && r.y1 <= 258.0f;
    const bool highlight = width >= 150.0f && width <= 162.0f && height >= 18.0f && height <= 22.0f &&
        r.x0 >= 310.0f && r.x0 <= 318.0f && r.y0 >= 136.0f && r.y0 <= 190.0f;
    return corner || side || highlight;
}

[[nodiscard]] inline bool is_bowgun_icon(Rect rect) {
    const Rect r = normalized(rect);
    const float width = r.x1 - r.x0;
    const float height = r.y1 - r.y0;
    // A mark on the area map sits in this corner too. It belongs to the map.
    return width >= 12.0f && height >= 12.0f &&
        contains({kBowgunLeft, kBowgunTop, kBowgunRight, kBowgunBottom}, r) &&
        !contains({kMapLeft, kMapTop, kMapRight, kMapBottom}, r);
}

[[nodiscard]] inline Piece piece_of(Rect rect) {
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
    if (width >= 2.0f && height >= 2.0f &&
        (contains({kVitalLeft, kVitalTop, kVitalRight, kVitalBottom}, r) ||
            contains({kClockLeft, kClockTop, kClockRight, kClockBottom}, r)))
        return Piece::Vitals;
    if (width < 4.0f || height < 4.0f) return Piece::None;
    // The hunter's name under the clock. It grows from the clock's corner
    // with the bars, so it stays under the clock.
    if (contains({kNameLeft, kNameTop, kNameRight, kNameBottom}, r)) return Piece::Vitals;
    // A marker drawn on the map, inside the tile. Anything that sticks out
    // is not one.
    if (contains({kMapLeft, kMapTop, kMapRight, kMapBottom}, r)) return Piece::Map;
    if (contains({kItemLeft, kItemTop, kItemRight, kItemBottom}, r)) return Piece::ItemBar;
    if (contains({kSpiritLeft, kSpiritTop, kSpiritRight, kSpiritBottom}, r) && !is_sharpness(r))
        return Piece::LongSword;
    if (contains({kBowgunLeft, kBowgunTop, kBowgunRight, kBowgunBottom}, r)) return Piece::Bowgun;
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

    [[nodiscard]] static Scales uniform(float scale) {
        return {scale, scale, scale, scale, scale, scale};
    }
    // The three sizes a player sets, in percent. The clock, health and the
    // weapon pieces under stamina are one group, so they stay together. The
    // bowgun's ammo bar and the bow's coatings sit on the item bar, and the
    // ammo bar opens into the ammo menu over it, so they take its size.
    [[nodiscard]] static Scales from_percent(std::uint32_t map, std::uint32_t health, std::uint32_t item_bar) {
        const float group = static_cast<float>(health) * 0.01f;
        const float bar = static_cast<float>(item_bar) * 0.01f;
        return {static_cast<float>(map) * 0.01f, group, bar, group, group, bar};
    }
    [[nodiscard]] bool any() const {
        return map != 1.0f || vitals != 1.0f || item_bar != 1.0f || sharpness != 1.0f || long_sword != 1.0f ||
            bowgun != 1.0f;
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
    // The top-right corner of the map sprite last drawn, large or small. The
    // map and the marks on it grow from there.
    Anchor map_corner{kMapRight, kMapTop};

    void begin() {
        map_live = map_now;
        vital_live = vital_now;
        bar_live = bar_now;
        sharp_live = sharp_now;
        spirit_live = spirit_now;
        bowgun_live = bowgun_now;
        list_live = list_now;
        map_now = vital_now = bar_now = sharp_now = spirit_now = bowgun_now = list_now = ammo_menu_now = false;
    }

    void note(Rect rect) {
        if (is_map_sprite(rect)) {
            map_now = true;
            const Rect r = normalized(rect);
            map_corner = {r.x1, r.y0};
        }
        if (is_vital_bar(rect) || is_clock_sprite(rect)) vital_now = true;
        if (is_bar_panel(rect)) bar_now = true;
        if (is_sharpness_bar(rect) || is_sharpness_frame(rect)) sharp_now = true;
        if (is_spirit_bar(rect) || is_spirit_fill(rect) || is_loaded_round(rect) || is_bow_coating(rect) ||
            is_gunlance_shell(rect) || is_hunting_horn(rect))
            spirit_now = true;
        if (is_ammo_menu_sprite(rect)) ammo_menu_now = true;
        if (is_bowgun_icon(rect)) bowgun_now = true;
        if (is_item_list(rect)) list_now = true;
    }

    [[nodiscard]] bool allows(Piece piece, Rect rect) const {
        switch (piece) {
        // The item list covers the map's side of the picture. A cell of it
        // that lands in the map's rectangle stays as the game drew it. The
        // area-map tile itself is not one of those cells: it still scales
        // while the list is open, so a list-shaped sprite earlier in the
        // frame cannot pin the map at the size the game drew.
        case Piece::Map:
            if (is_map_sprite(rect)) return true;
            return (map_now || map_live) && !list_now && !list_live;
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
template <typename Vertex> void scale_quads(std::vector<Vertex> &vertices, const Scales &scales, Frame &frame) {
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
    for (std::size_t first = 0; first + 6u <= vertices.size(); first += 6u) frame.note(bounds_at(first));
    for (std::size_t first = 0; first + 6u <= vertices.size(); first += 6u) {
        const Rect bounds = bounds_at(first);
        // The ammo menu is one panel over the item bar's corner, sized with
        // the item bar. Only this frame's menu counts: the frame after it
        // closes draws the ammo bar in the same place.
        const bool in_ammo_menu = frame.ammo_menu_now &&
            contains({kAmmoMenuLeft, kAmmoMenuTop, kAmmoMenuRight, kAmmoMenuBottom}, bounds);
        const Piece piece = in_ammo_menu ? Piece::ItemBar : piece_of(bounds);
        if (piece == Piece::None || (!in_ammo_menu && !frame.allows(piece, bounds))) continue;
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
