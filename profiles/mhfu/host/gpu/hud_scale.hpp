#pragma once

#include "game/guest_layout.hpp"
#include "gpu/game_hud.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <optional>
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
// and those stay at the size the game chose. Two rules keep them apart:
//
// - Who drew it. gpu/game_hud.hpp follows the guest's call stack when each
//   2D packet is filed, and only a packet from the quest HUD (game_task) is
//   resized (hud::Owner::QuestHud). A shop, the item box, a dialog, the pause
//   menu, and everything in the village and the Guild Hall are filed by other
//   code, so no rectangle below can move them, however well they fit it.
// - What it is. Every rectangle below then says which piece a quest HUD
//   sprite belongs to, and so which size and anchor it takes. A piece is
//   scaled only while its own sprites are on screen.
//
// When the 2D draws cannot be watched (hud::Owner::Unknown, or
// MHFU_HUD_SCALE_GEOMETRY=1) the rectangles alone decide, as they did before
// the owner was known. New rules for a HUD state belong in the second part;
// they no longer need to tell a menu apart.

enum class Piece { None, Map, Vitals, ItemBar, Sharpness, LongSword, Bowgun };

[[nodiscard]] inline const char *piece_name(Piece piece) {
    switch (piece) {
    case Piece::None:
        return "none";
    case Piece::Map:
        return "map";
    case Piece::Vitals:
        return "status";
    case Piece::ItemBar:
        return "items";
    case Piece::Sharpness:
        return "sharpness";
    case Piece::LongSword:
        return "weapon";
    case Piece::Bowgun:
        return "bowgun";
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
// are a different texture, so a mark of a letter's size is still a mark. It
// is the item atlas (game/guest_layout.hpp).
inline constexpr std::uint32_t kMapIconTexture = game::layout::kItemAtlas;
// The quest HUD's atlas: the clock, the bars' frames and caps, the name
// plates, the item bar's corners and a bowgun's reticle.
inline constexpr std::uint32_t kHudAtlasTexture = 0x09156BA0u;
// The game's font: names, the party list and the bowgun slot's ammo digits.
inline constexpr std::uint32_t kFontTexture = 0x040CC000u;
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
    return {
        std::min(rect.x0, rect.x1), std::min(rect.y0, rect.y1), std::max(rect.x0, rect.x1), std::max(rect.y0, rect.y1)};
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

// The bounds of the quad at `first`. One through sprite, or one through quad,
// is six vertices (two triangles) after it is expanded.
template <typename Vertex> [[nodiscard]] Rect quad_bounds(const std::vector<Vertex> &vertices, std::size_t first) {
    Rect r{vertices[first].x, vertices[first].y, vertices[first].x, vertices[first].y};
    for (std::size_t i = 1; i < 6u; ++i) {
        r.x0 = std::min(r.x0, vertices[first + i].x);
        r.y0 = std::min(r.y0, vertices[first + i].y);
        r.x1 = std::max(r.x1, vertices[first + i].x);
        r.y1 = std::max(r.y1, vertices[first + i].y);
    }
    return r;
}

// The bounds of every vertex. `vertices` is not empty.
template <typename Vertex> [[nodiscard]] Rect bounds_of(const std::vector<Vertex> &vertices) {
    Rect r{vertices.front().x, vertices.front().y, vertices.front().x, vertices.front().y};
    for (const Vertex &vertex : vertices) {
        r.x0 = std::min(r.x0, vertex.x);
        r.y0 = std::min(r.y0, vertex.y);
        r.x1 = std::max(r.x1, vertex.x);
        r.y1 = std::max(r.y1, vertex.y);
    }
    return r;
}

// The smallest rectangle holding both.
[[nodiscard]] inline Rect united(Rect a, Rect b) {
    a = normalized(a);
    b = normalized(b);
    return {std::min(a.x0, b.x0), std::min(a.y0, b.y0), std::max(a.x1, b.x1), std::max(a.y1, b.y1)};
}

// Under Fill the renderer pulls a 2D interface draw in about the centre of the
// game's 480 by 272 picture, to keep its pixels square. The rules in this file
// are in the game's pixels. 1 on both axes is a draw that is not pulled in.
struct InterfaceFit {
    static constexpr float kCentreX = 240.0f;
    static constexpr float kCentreY = 136.0f;
    float x{1.0f};
    float y{1.0f};

    [[nodiscard]] bool any() const { return x != 1.0f || y != 1.0f; }
    // From the game's pixels to where the draw lands.
    void apply(float &px, float &py) const {
        px = kCentreX + (px - kCentreX) * x;
        py = kCentreY + (py - kCentreY) * y;
    }
    // From where the draw landed back to the game's pixels.
    [[nodiscard]] Rect to_game(Rect r) const {
        if (x <= 0.0f || y <= 0.0f) return r;
        return {kCentreX + (r.x0 - kCentreX) / x, kCentreY + (r.y0 - kCentreY) / y, kCentreX + (r.x1 - kCentreX) / x,
            kCentreY + (r.y1 - kCentreY) / y};
    }
};

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
    if (width < 28.0f || height < 28.0f || width > 200.0f || height > 200.0f || is_fixed_map_sprite(rect)) return false;
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
    if (width < 16.0f || height < 16.0f || width > 200.0f || height > 200.0f || is_fixed_map_sprite(rect)) return false;
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

// The window frame the item bar is built from. Its top edge, body, bottom
// edge and two sides each have their own texture; the corners come from the
// HUD atlas. Other quest windows use the same frame: the banner a teammate
// leaves with ("... broke off from the group!", (104, 120)-(376, 152)) and
// the message box under a list. Traced with MHFU_TRACE_HUD_SCALE=all in an
// ad hoc quest: the banner's 16 by 16 left side sits in the party list's
// band, the size of a status icon, and was pulled toward the clock.
inline constexpr std::uint32_t kFrameTopTexture = 0x09163C20u;
inline constexpr std::uint32_t kFrameBodyTexture = 0x09164420u;
inline constexpr std::uint32_t kFrameBottomTexture = 0x09164C20u;
inline constexpr std::uint32_t kFrameLeftTexture = 0x091643A0u;
inline constexpr std::uint32_t kFrameRightTexture = 0x091644A0u;

[[nodiscard]] inline bool is_window_frame_texture(std::uint32_t texture) {
    return texture == kFrameTopTexture || texture == kFrameBodyTexture || texture == kFrameBottomTexture ||
        texture == kFrameLeftTexture || texture == kFrameRightTexture;
}

// The yellow ring around a paintballed monster, larger than any other mark.
// Traced with MHFU_TRACE_MAP_MARKS in Volcano area 4: 43 by 43 at
// (328, 121)-(371, 164), from the mark atlas (kMapIconTexture). Taken for a
// window over the map, it held itself, the arrow and the camp icon at the
// game's size while the map shrank to 60%. No window piece over the
// map is square: the ad hoc window's side is 16 by 112 and its body 96 by
// 112. The shop and return-to-town panels are 40 square, but they are filed
// by other code, so only the quest HUD's ring is moved with the map.
// When a paintballed flying monster lands in a new area, the map pings it: a
// ring from the same atlas grows out of the monster's dot, past the map's
// own size. Capped at 72 it was left at the game's size once it grew, and,
// square and over the map, it was taken for a window that held every mark at
// the game's size while it grew. A ring from the atlas stays a ring at any
// size while its centre is on the map.
[[nodiscard]] inline bool is_map_ring(Rect rect, std::uint32_t texture) {
    if (texture == kAreaMapTexture || is_window_frame_texture(texture)) return false;
    const Rect r = normalized(rect);
    const float width = r.x1 - r.x0;
    const float height = r.y1 - r.y0;
    if (width <= 36.0f || std::fabs(width - height) > 2.0f) return false;
    if (width <= 72.0f) return overlaps({kMapLeft - 16.0f, kMapTop - 16.0f, kMapRight + 16.0f, kMapBottom + 16.0f}, r);
    const float cx = 0.5f * (r.x0 + r.x1);
    const float cy = 0.5f * (r.y0 + r.y1);
    return texture == kMapIconTexture && width <= 480.0f && cx >= kMapLeft && cx <= kMapRight && cy >= kMapTop &&
        cy <= kMapBottom;
}

// A window over the area map: a sprite larger than any mark in both
// directions that is neither the map's texture nor the full map tile. Traced
// with MHFU_TRACE_HUD_SCALE in an ad hoc quest: a window at (344, 48)-(456,
// 160), its left side (344, 48)-(360, 160) and its body (360, 48)-(456, 160)
// from 0x09165BA0 and 0x09165C20, with 13 by 14 corners from the mark atlas.
// It overlaps the map, so its side and corners were sized with the map and
// pulled toward the slot's corner while the rest stayed. The map is drawn
// only from kAreaMapTexture and marks of 36 or less. The ammo menu reaches
// below the slot and is not inside it. A paintball's ring is not a window:
// taken for one, it held every mark on the map at the game's size.
[[nodiscard]] inline bool is_map_panel(Rect rect, std::uint32_t texture) {
    if (texture == kAreaMapTexture || is_fixed_map_sprite(rect) || is_map_ring(rect, texture)) return false;
    const Rect r = normalized(rect);
    const float width = r.x1 - r.x0;
    const float height = r.y1 - r.y0;
    return width > 36.0f && height > 36.0f &&
        contains({kMapLeft - 16.0f, kMapTop - 16.0f, kMapRight + 16.0f, kMapBottom + 16.0f}, r);
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

// Whether the quest HUD is on screen, whatever the interface sizes: the
// quest clock was drawn this frame or the one before. The monster icon
// follows it, so it is gone once the quest's own HUD gives way to the
// screens after a quest. Fed every through-mode sprite draw shown, before
// the HUD sizes or the Fill aspect move it.
struct QuestClock {
    bool now{};
    bool live{};

    // `vertices` are expanded quads, six vertices each.
    template <typename Vertex> void note(const std::vector<Vertex> &vertices) {
        if (now) return;
        for (std::size_t first = 0; first + 6u <= vertices.size(); first += 6u)
            if (is_clock_sprite(quad_bounds(vertices, first))) {
                now = true;
                return;
            }
    }
    // At each flip.
    void begin() {
        live = now;
        now = false;
    }
    [[nodiscard]] bool up() const { return now || live; }
};

// Names under the clock, left of the map. A quest draws up to four hunters
// there, and each cat's name, badge, yellow eye and buffs sit in this band.
// The buffs reach toward the middle of the picture. The names themselves
// stay in the left column: the quest-details window starts just past it, and
// its letters are the same size as a name.
inline constexpr float kPartyLeft = 4.0f;
inline constexpr float kPartyTop = 52.0f;
inline constexpr float kPartyRight = 312.0f;
inline constexpr float kPartyBottom = 148.0f;
inline constexpr float kPartyNameRight = 192.0f;

// The yellow eye shown once a monster is found: 12 by 16. The hunter's own
// is at (104, 56)-(116, 72). A cat and each teammate draw the same eye beside
// their name, on the rows below. Traced with MHFU_TRACE_SPRITES.
[[nodiscard]] inline bool is_monster_eye(Rect rect, std::uint32_t texture = 0) {
    if (texture == kAreaMapTexture || texture == kMapIconTexture) return false;
    const Rect r = normalized(rect);
    const float width = r.x1 - r.x0;
    const float height = r.y1 - r.y0;
    // A quest-details letter is at most 10 wide and 12 to 16 tall. The eye is wider.
    if (width <= 10.0f && height >= 12.0f && height <= 16.0f) return false;
    return width >= 10.0f && width <= 16.0f && height >= 14.0f && height <= 18.0f &&
        contains({kPartyLeft, kPartyTop, kPartyRight, kPartyBottom}, r);
}

// Poison, paralysis, sleep, stun, snowman, muddy, stench, defense down and
// fatigue. The hunter's own are about 12 wide and 14 to 16 tall, from x 112
// on the first row. A cat's badge is that same mark beside the cat's name,
// and it can sit left of x 112. Teammates and their cats draw the same buffs
// on the rows below. The queued-note mark is this size at (128, 56) and is
// recognised first, so it keeps the horn's size. The stars over a stunned
// hunter are larger and are not one of these.
[[nodiscard]] inline bool is_status_icon(Rect rect, std::uint32_t texture = 0) {
    if (texture == kAreaMapTexture || texture == kMapIconTexture) return false;
    const Rect r = normalized(rect);
    const float width = r.x1 - r.x0;
    const float height = r.y1 - r.y0;
    // A letter in the quest-details window is at most 10 wide. A buff is wider.
    if (width <= 10.0f && height >= 12.0f && height <= 16.0f) return false;
    return width >= 10.0f && width <= 16.0f && height >= 12.0f && height <= 18.0f &&
        contains({kPartyLeft, kPartyTop, kPartyRight, kPartyBottom}, r);
}

// Up to four hunters share the quest, and each hunter's weapon badge is the
// wide mark on that name: the first is (4, 56)-(36, 68), and the later names
// repeat it 20 lower. The same column holds smaller icons, such as the face
// of the cat that accompanies the player, about (10, 82)-(20, 89) under the
// cat's name. Everything in the column takes Status from the clock with the
// plates, so it keeps the place the game gave it beside them.
inline constexpr float kBadgeCenter = 62.0f;
inline constexpr float kPlateStride = 20.0f;
inline constexpr int kNamePlates = 4;

// The name plate whose badge is nearest the vertical centre `cy`: 0 is the
// player's own.
[[nodiscard]] inline int nearest_plate(float cy) {
    const int n = static_cast<int>(std::lround((cy - kBadgeCenter) / kPlateStride));
    return std::clamp(n, 0, kNamePlates - 1);
}

// A hunter's weapon mark, already on its name. Four hunters, one mark each.
[[nodiscard]] inline bool is_hunter_badge(Rect rect) {
    const Rect r = normalized(rect);
    const float width = r.x1 - r.x0;
    const float height = r.y1 - r.y0;
    if (width < 24.0f || width > 40.0f || height < 10.0f || height > 16.0f) return false;
    const float cx = 0.5f * (r.x0 + r.x1);
    const float cy = 0.5f * (r.y0 + r.y1);
    if (cx < 0.0f || cx > 40.0f) return false;
    const float slot = kBadgeCenter + static_cast<float>(nearest_plate(cy)) * kPlateStride;
    return std::fabs(cy - slot) <= 4.0f;
}

// A name plate repeats every 20 pixels. The dark badge behind a weapon is a
// tile of that plate. The face paint is narrower and is not one of these.
[[nodiscard]] inline bool inside_name_plate(Rect rect) {
    const Rect r = normalized(rect);
    for (int i = 0; i < kNamePlates; ++i) {
        const float dy = static_cast<float>(i) * kPlateStride;
        if (contains({kNameLeft, kNameTop + dy, kNameRight, kNameBottom + dy}, r)) return true;
    }
    return false;
}

[[nodiscard]] inline bool is_plate_tile(Rect rect) {
    const Rect r = normalized(rect);
    if (!inside_name_plate(r)) return false;
    const float width = r.x1 - r.x0;
    const float height = r.y1 - r.y0;
    return width >= 14.0f && height >= 8.0f && height <= 20.0f;
}

// A small icon in the badge column under the clock, such as the cat's face,
// or the cell it is drawn in. A hunter's weapon mark, a tile of a name plate
// and a name letter right of the column are not one.
[[nodiscard]] inline bool is_party_column_icon(Rect rect) {
    const Rect r = normalized(rect);
    if (is_hunter_badge(r)) return false;
    const float width = r.x1 - r.x0;
    const float height = r.y1 - r.y0;
    if (width < 8.0f || width > 64.0f || height < 4.0f || height > 64.0f) return false;
    // A tile of a name plate, on any row, stays with that plate. The face
    // hangs off the plate, so its box is not inside it.
    if (contains({kNameLeft, kNameTop, kNameRight, kNameBottom}, r) || is_plate_tile(r)) return false;
    const float cx = 0.5f * (r.x0 + r.x1);
    const float cy = 0.5f * (r.y0 + r.y1);
    if (cx < 0.0f || cx > 40.0f || cy < kNameTop || cy > kPartyBottom + 16.0f) return false;
    // A name letter is at most 10 wide and 12 to 16 tall, and it is not in
    // the weapon column. The face's cell is that size, in the column.
    return !(width <= 10.0f && height >= 12.0f && height <= 16.0f && cx > 18.0f);
}

// A teammate's health, the green line under that hunter's name. Traced with
// MHFU_TRACE_SPRITES in an ad hoc quest: an untextured through sprite at
// (26, 90)-(98, 92) on the second name's plate, 72 wide at full health and
// shorter as health drops. It is two pixels tall, below the four-pixel floor
// piece_of applies to everything else, so it is told apart first and keeps
// the place the game gave it under the name.
[[nodiscard]] inline bool is_party_health_bar(Rect rect) {
    const Rect r = normalized(rect);
    const float width = r.x1 - r.x0;
    const float height = r.y1 - r.y0;
    return width > 0.0f && width <= 80.0f && height >= 1.0f && height <= 4.0f && r.x0 >= 22.0f && r.x0 <= 30.0f &&
        inside_name_plate(r);
}

// A letter-sized quad where the party list puts one. Traced with
// MHFU_TRACE_SPRITES and MHFU_TRACE_HUD_SCALE in an ad hoc quest: a name's
// letters are 8 by 14, 8 apart from x 26, on y 58 for the first name and 20
// lower for each after it; the plate's 8 by 16 end is at x 96 on y 56 and
// 76. The name over another hunter uses the same letters, drawn in the same
// batch, and slides across the list with the camera at any x and y, rows
// included (y 116 and 118 from x 45 to 116). Off that grid a letter is not
// part of the list. The badge column left of x 24 holds the cat's face cell,
// on a name's row. A cat's name beside its hunter, past the plate's right
// end, keeps that row with no grid; an overhead letter on a row out there
// still passes. The Felyne Comrade's name can also sit under the hunter's,
// its letters from y 72 over the plate's width, with its face below them.
inline constexpr float kNameLetterLeft = 26.0f;
inline constexpr float kNameLetterAdvance = 8.0f;
inline constexpr float kPlateEndLeft = 96.0f;

[[nodiscard]] inline bool on_party_row(Rect rect) {
    const Rect r = normalized(rect);
    const auto near = [](float a, float b) { return std::fabs(a - b) <= 0.5f; };
    if (near(r.y0, kNameBottom - 4.0f) && r.x0 <= kNameRight + 8.0f) return true;
    const float column = (r.x0 - kNameLetterLeft) / kNameLetterAdvance;
    const bool on_letter = column >= -0.1f && std::fabs(column - std::round(column)) * kNameLetterAdvance <= 0.5f;
    for (int i = 0; i < kNamePlates; ++i) {
        const float plate = kNameTop + 2.0f + static_cast<float>(i) * kPlateStride;
        // The badge column, and a cat's name beside its hunter past the plate.
        if ((near(r.y0, plate) || near(r.y0, plate + 2.0f)) && (r.x0 < 24.0f || r.x0 >= kNameRight)) return true;
        if (near(r.y0, plate + 2.0f) && on_letter) return true;
        if (near(r.y0, plate) && near(r.x0, kPlateEndLeft)) return true;
    }
    return false;
}

// The hunter's own plate is the rectangle above this. The cat's name is the
// letters beside that hunter (about 8 by 14, or one wider label). A badge,
// an eye or a buff is 10 to 16 wide, and a mark of that width is not a name.

[[nodiscard]] inline bool is_party_name(Rect rect) {
    const Rect r = normalized(rect);
    if (!contains({kPartyLeft, kPartyTop, kPartyNameRight, kPartyBottom}, r)) return false;
    const float width = r.x1 - r.x0;
    const float height = r.y1 - r.y0;
    const bool letter = width >= 6.0f && width <= 10.0f && height >= 12.0f && height <= 16.0f;
    // Taller than the name plate (16). A map arrow outside the slot is 18
    // square and is not a name.
    const bool label = width >= 18.0f && width <= 120.0f && height >= 10.0f && height <= 16.0f;
    return letter || label;
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
// They are none of these. A fill is as wide as what is left of it: traced
// with stamina run dry, (56, 22)-(57, 24) and then (56, 22)-(56, 24) for the
// two thirds of a second before it refills. Turned away as too narrow, that
// sliver stayed at the game's place, off a smaller bar, until the fill was
// two pixels wide.
[[nodiscard]] inline bool is_vital_piece(Rect rect) {
    const Rect r = normalized(rect);
    const float width = r.x1 - r.x0;
    const float height = r.y1 - r.y0;
    if (!contains({kVitalLeft, kVitalTop, kRaisedVitalRight, kVitalBottom}, r) || height < 2.0f) return false;
    const bool bar = r.x0 >= 48.0f && r.x0 <= 72.0f && height <= 14.0f;
    if (width < 2.0f) return bar && height <= 4.0f;
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

// The colour painted on the blade, sprite 9 at (60, 29), 44 by 12. The blade
// under it is the shorter sprite at (60, 32), 44 by 7 or 8, and is not this.
[[nodiscard]] inline bool is_sharpness_colour(Rect rect) {
    const Rect r = normalized(rect);
    const float height = r.y1 - r.y0;
    return is_sharpness_frame(r) && height >= 10.0f && r.y0 <= 30.5f;
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
    return width >= 6.0f && width <= 10.0f && height >= 15.0f && height <= 18.0f && r.x0 >= 56.0f && r.x0 <= 136.0f &&
        r.y0 >= 30.0f && r.y0 <= 34.0f;
}

// A letter of a window's text, 7 by 14. The item box's "Please select an
// item." starts at about x 229 on the row under stamina, so its first letters
// sit inside the weapon slot while a weapon piece is up and were moved with
// it, out of the box. A loaded round and a gunlance shell are a pixel taller,
// a single coating is wider, and the coating count is 10 by 20.
[[nodiscard]] inline bool is_window_letter(Rect rect) {
    const Rect r = normalized(rect);
    const float width = r.x1 - r.x0;
    const float height = r.y1 - r.y0;
    return width >= 6.0f && width <= 10.0f && height >= 12.0f && height <= 14.5f;
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
    const bool plate =
        width >= 30.0f && width <= 38.0f && height >= 10.0f && height <= 14.0f && r.x0 >= 56.0f && r.x0 <= 64.0f;
    const bool large =
        width >= 16.0f && width <= 20.0f && height >= 14.0f && height <= 18.0f && r.x0 >= 56.0f && r.x0 <= 64.0f;
    const bool single =
        width >= 10.0f && width <= 14.0f && height >= 14.0f && height <= 18.0f && r.x0 >= 56.0f && r.x0 <= 236.0f;
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

// A 16 by 8 tile of a window's border. Dialogs drawn during a quest build
// their frame from these, and a corner can land in the weapon slot under
// stamina: the Abandon Quest window's top-left is (232, 40)-(248, 48), traced
// with MHFU_TRACE_HUD_SCALE with a bow out. The same run moved such tiles at
// (160, 34) and (160, 42) as well. No weapon piece is this shape.
[[nodiscard]] inline bool is_window_tile(Rect rect) {
    const Rect r = normalized(rect);
    const float width = r.x1 - r.x0;
    const float height = r.y1 - r.y0;
    return width >= 15.5f && width <= 16.5f && height >= 7.5f && height <= 8.5f;
}

// A quest menu's window over the party list: Combine, the kitchen skills,
// the equipment pages. Traced with MHFU_TRACE_CAPTURE and
// MHFU_TRACE_HUD_SCALE=all: each starts at the picture's left edge, with 16
// by 8 corners at (0..12, 40..152). Its letters (7 by 14, or 14 by 14), its
// icons and those corners fall where the party list's names, buffs and badge
// column are, and were sized and moved with Status. A window's corner at the
// left of that band says one is up.
[[nodiscard]] inline bool is_party_window_tile(Rect rect) {
    const Rect r = normalized(rect);
    return is_window_tile(r) && r.x0 <= 24.0f && r.y0 >= 36.0f && r.y0 <= 180.0f;
}

// A letter of a name in the party list: 8 by 14, on the name grid (x 26 and
// every 8 after it), on a name's row. A menu's letters are 7 wide.
[[nodiscard]] inline bool is_party_letter(Rect rect) {
    const Rect r = normalized(rect);
    const float width = r.x1 - r.x0;
    const float height = r.y1 - r.y0;
    if (width < 7.5f || width > 8.5f || height < 12.0f || height > 16.0f || r.x0 < kNameLetterLeft - 0.5f) return false;
    const float column = (r.x0 - kNameLetterLeft) / kNameLetterAdvance;
    if (std::fabs(column - std::round(column)) * kNameLetterAdvance > 0.5f) return false;
    for (int i = 0; i < kNamePlates; ++i)
        if (std::fabs(r.y0 - (kNameTop + 4.0f + static_cast<float>(i) * kPlateStride)) <= 0.5f) return true;
    return false;
}

// The name plate a letter-sized quad sits on as a name's letter: its top on
// a name's row and its left on the plate, from x 26. -1 for anything else.
// The font is proportional: a Felyne named "Ken" draws K at x 26, e at 33
// and n at 40, 7 wide each, while "LOU" is 8 apart. Only the first of those
// is on the 8-pixel grid on_party_row checks, which keeps an overhead name
// crossing the list out of it; the others continue a name already taken.
[[nodiscard]] inline int party_letter_row(Rect rect) {
    const Rect r = normalized(rect);
    const float width = r.x1 - r.x0;
    const float height = r.y1 - r.y0;
    if (width < 6.0f || width > 10.0f || height < 12.0f || height > 16.0f || r.x0 < kNameLetterLeft - 0.5f ||
        r.x0 >= kPlateEndLeft)
        return -1;
    for (int i = 0; i < kNamePlates; ++i)
        if (std::fabs(r.y0 - (kNameTop + 4.0f + static_cast<float>(i) * kPlateStride)) <= 0.5f) return i;
    return -1;
}

// While such a window is up, only what is certainly the party list keeps
// Status in that band: the plates and their tiles, the weapon badges, a
// teammate's health line and the names' own letters. The clock is above the
// band. Everything else there is the window's.
//
// `top` is the highest of those corners this frame. A menu's is at y 40, over
// every row. A quest NPC's dialogue is the same frame at (8, 88)-(184, 264),
// traced with MHFU_TRACE_HUD_SCALE=all and MHFU_TRACE_CAPTURE while a man in
// the Jungle talks: its corner is below the hunter's own row, and the buff at
// (116, 56)-(128, 72) on that row was left at the game's place while the
// plate it belongs to shrank. The cat's name, (26, 78)-(47, 92) on the second
// plate, ends four pixels into the corner; its letters are 7 wide, not the
// name grid's 8, and they were left too. A piece is the window's only when
// its middle is below that corner.
[[nodiscard]] inline bool covered_by_party_window(Rect rect, float top = 36.0f) {
    const Rect r = normalized(rect);
    if (contains({kClockLeft, kClockTop, kClockRight, kClockBottom}, r)) return false;
    if (0.5f * (r.y0 + r.y1) < top) return false;
    if (!overlaps({0.0f, 36.0f, kPartyRight, kPartyBottom + 24.0f}, r)) return false;
    if (is_hunter_badge(r) || is_party_health_bar(r) || is_party_letter(r)) return false;
    // A plate tile stays with its plate; a letter on a plate that is not on
    // the name grid is the window's. Letters are 14 tall; the plate's 8 by
    // 16 end is not one.
    const float width = r.x1 - r.x0;
    const float height = r.y1 - r.y0;
    const bool letter = width >= 6.0f && width <= 10.0f && height >= 12.0f && height <= 15.0f;
    return !(inside_name_plate(r) && !letter);
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
    const bool highlight = width >= 150.0f && width <= 162.0f && height >= 18.0f && height <= 22.0f && r.x0 >= 310.0f &&
        r.x0 <= 318.0f && r.y0 >= 136.0f && r.y0 <= 190.0f;
    return side || highlight;
}

// The ammo bar above the item bar: (312, 184)-(472, 200), a wide strip in the
// band the area map does not cover. Traced with a bowgun out. The equipment
// colour menu draws a checkbox in that same band, under the map, about 16
// pixels square; that checkbox is not the bar. A window's title strip is the
// same shape a row higher: in an ad hoc quest, (312, 176)-(456, 192) and its
// edge (312, 192)-(456, 200), traced with MHFU_TRACE_HUD_SCALE=all, were
// taken for the bar on the window's first frame and shrank with it.
[[nodiscard]] inline bool is_bowgun_icon(Rect rect) {
    const Rect r = normalized(rect);
    const float width = r.x1 - r.x0;
    const float height = r.y1 - r.y0;
    return width >= 100.0f && width <= 170.0f && height >= 12.0f && height <= 20.0f && r.x0 >= 300.0f &&
        r.x0 <= 340.0f && r.y0 >= 180.0f && r.y0 <= 188.0f &&
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
    // A piece of the item bar's window frame is the item bar in the item
    // bar's place, and a window anywhere else.
    if (is_window_frame_texture(texture))
        return contains({kItemLeft, kItemTop, kItemRight, kItemBottom}, r) ? Piece::ItemBar : Piece::None;
    // A window's 16 by 8 corner left of the item bar's column. No piece of
    // the clock, the bars, the weapon or the party list is that shape; a
    // menu's corner at the left edge fell in the clock's box and the badge
    // column and was moved with Status.
    if (is_window_tile(r) && r.x1 <= kItemLeft) return Piece::None;
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
    if (is_party_health_bar(r)) return Piece::Vitals;
    if (width < 4.0f || height < 4.0f) return Piece::None;
    // A letter of the name over another hunter, crossing the party list.
    if (is_text_glyph(r) && contains({kPartyLeft, kPartyTop, kPartyRight, kPartyBottom}, r) && !on_party_row(r))
        return Piece::None;
    // The hunter's name under the clock. It grows from the clock's corner
    // with the bars, so it stays under the clock. The yellow eye is the icon
    // just past the plate's right end, and the status icons continue along
    // that row. A cat's name and face beside a hunter, and the other hunters
    // in a party of four, are the same rows.
    if (inside_name_plate(r) || is_monster_eye(r, texture) || is_status_icon(r, texture) || is_party_column_icon(r) ||
        is_party_name(r))
        return Piece::Vitals;
    // A marker drawn on the map, inside the tile. The player's arrow turns, so
    // its box can cross the edge; that one is caught from the map's own
    // bounds in on_map. The notice's right border and its letters cross this
    // rectangle while the window slides; a mark is neither of those. Nor is
    // anything larger than a mark: the map itself is told apart above, and a
    // window over the map draws its 16 by 112 side before the body that marks
    // the window as up (is_map_panel), so on its first frame that side was
    // sized and moved with the map.
    if (contains({kMapLeft, kMapTop, kMapRight, kMapBottom}, r))
        return is_notice_piece(r, texture) || !is_map_mark(r) ? Piece::None : Piece::Map;
    if (contains({kItemLeft, kItemTop, kItemRight, kItemBottom}, r)) return Piece::ItemBar;
    if (contains({kSpiritLeft, kSpiritTop, kSpiritRight, kSpiritBottom}, r) && !is_sharpness(r) && !is_window_tile(r) &&
        !is_window_letter(r))
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

    [[nodiscard]] static Scales uniform(float scale) { return {scale, scale, scale, scale, scale, scale, scale}; }
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
        case Piece::Map:
            return map;
        case Piece::Vitals:
            return vitals;
        case Piece::ItemBar:
            return item_bar;
        case Piece::Sharpness:
            return sharpness;
        case Piece::LongSword:
            return long_sword;
        case Piece::Bowgun:
            return bowgun;
        case Piece::None:
            return 1.0f;
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
        const Rect drawn = bounds_of(vertices);
        anchor_x = 0.5f * (drawn.x0 + drawn.x1);
        anchor_y = 0.5f * (drawn.y0 + drawn.y1);
    }
    for (Vertex &vertex : vertices) {
        vertex.x = anchor_x + (vertex.x - anchor_x) * scale;
        vertex.y = anchor_y + (vertex.y - anchor_y) * scale;
    }
}

// Whether a name over a hunter takes the Names size. A quest name always does.
// A gathering-hall name does once Names is set to another size, unless
// `hall_names` is off (MHFU_HUD_HALL_NAMES=0): then frame interpolation only
// moves it with its hunter.
[[nodiscard]] inline bool sizes_name(const hud::NameDraw &named, const Scales &scales, bool hall_names) {
    return named.is_name && (!named.hall || (hall_names && scales.names != 1.0f));
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
    // A window over the area map this frame or the one before (is_map_panel).
    bool panel_now{};
    bool panel_live{};
    // A menu's window over the party list (is_party_window_tile), this frame
    // or the one before, and the highest of its corners (party_window_top).
    bool party_window_now{};
    bool party_window_live{};
    float party_window_top_now{};
    float party_window_top_live{};
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
    // The ammo menu's corner tile, and its body, this frame and the one
    // before. The Combine window in a quest (Material A and B, left of the
    // item list) has a 16 by 8 corner on the menu's corner row and nothing
    // else of the menu; taken for the menu, the list's lower rows shrank
    // into the item bar's corner. A corner that stays up with no body is
    // not the menu. The menu's first frame still opens on the corner alone.
    bool menu_corner{};
    bool menu_corner_live{};
    bool menu_body_live{};
    // The obtain window ("Herb obtained."). It slides down from the top of
    // the picture and settles in the middle. A horizontal edge is a short
    // wide strip; on the way down the body is the taller panel the message
    // window starts as. The right end overlaps the enlarged area map, and a
    // cap or the item icon there was taken for a mark and pulled onto the map.
    Rect obtain_banner{};
    bool obtain_banner_now{};
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

    // A bow or a bowgun is aiming (camera/game_camera.hpp), set by the
    // renderer before each draw. Its reticle moves with the aim and can cross
    // the map's slot, where a sprite its size would be taken for a mark.
    // While it aims, each sprite drawn near the middle of the picture that is
    // no piece of the HUD is the reticle; its texture and size are kept, and
    // a sprite with both is left as drawn wherever it goes. Nothing of the
    // map is ever drawn there. Forgotten when the aim ends.
    bool aiming{};
    // The draw being decided was filed by the quest HUD (hud::Owner), set by
    // scale_quads. A paintball's ring is moved with the map only then.
    bool quest_owned{};
    struct Reticle {
        std::uint32_t texture{};
        float width{};
        float height{};
    };
    Reticle reticle[8]{};
    int reticles{};

    // Where the last letter taken for each name plate's name ended this
    // frame (party_letter_row), 0 before any. The next letter of a name
    // starts there, whatever its width.
    float party_letter_end[kNamePlates]{};

    // A gathering-hall label: the draws filed with a name's point this frame
    // (the weapon badge, the letters and their shadow, and any mark the hall
    // adds beside a hunter). The game's point is the first letter's top-left,
    // so a label sized about it drifts up and right off its hunter and the
    // badge slides onto the letters. The next frame sizes the label about the
    // middle of its letters and the badge against them instead, which keeps
    // it over the hunter. Other marks do not count: a hunter who takes a
    // quest or readies up gets one, and one that blinks moved that middle
    // every blink. The shadow's point is a pixel off the letters', so points
    // that close are one label.
    struct NameLabel {
        float origin_x{};
        float origin_y{};
        bool has_letters{};
        Rect letters{};
        static constexpr int kIcons = 4;
        Rect icons[kIcons]{};
        int icon_count{};
    };
    // Where a label is sized about, from its point, and the frames since it
    // was last drawn. A label kept for a second after it is last drawn, and
    // one drawn without its letters, keeps its middle: a mark or a name that
    // blinks does not move it.
    struct NameCentre {
        float origin_x{};
        float origin_y{};
        float dx{};
        float dy{};
        int unseen{};
    };
    static constexpr int kNameLabels = 16;
    static constexpr int kNameLabelKeep = 30;
    // How far a label's point may move between frames and still be the same
    // label. Traced in the hall: a turning camera moves a name 60 pixels in a
    // frame, and a label taken for new then shrank toward its first letter.
    static constexpr float kNameLabelMove = 96.0f;
    NameLabel name_labels[kNameLabels]{};
    int name_label_count{};
    NameCentre name_centres[kNameLabels]{};
    int name_centre_count{};
    // The point the last name draw was sized about, and whether it was a
    // label's kept middle rather than the game's point (MHFU_TRACE_HALL_NAMES).
    float name_pivot_x{};
    float name_pivot_y{};
    bool name_pivot_kept{};

    void note_name_label(float origin_x, float origin_y, Rect drawn, bool glyph) {
        const Rect d = normalized(drawn);
        NameLabel *label = nullptr;
        for (int i = 0; i < name_label_count && label == nullptr; ++i)
            if (std::fabs(name_labels[i].origin_x - origin_x) <= 4.0f &&
                std::fabs(name_labels[i].origin_y - origin_y) <= 4.0f)
                label = &name_labels[i];
        if (label == nullptr) {
            if (name_label_count == kNameLabels) return;
            label = &name_labels[name_label_count++];
            *label = NameLabel{origin_x, origin_y};
        }
        if (glyph) {
            label->letters = label->has_letters ? united(label->letters, d) : d;
            label->has_letters = true;
        } else if (label->icon_count < NameLabel::kIcons) {
            label->icons[label->icon_count++] = d;
        }
    }

    // The nearest kept middle within `max_move` pixels of a point, or -1.
    [[nodiscard]] int nearest_name_centre(float origin_x, float origin_y, float max_move = kNameLabelMove) const {
        int best = -1;
        float best_distance = max_move * max_move;
        for (int i = 0; i < name_centre_count; ++i) {
            const float dx = name_centres[i].origin_x - origin_x;
            const float dy = name_centres[i].origin_y - origin_y;
            const float distance = dx * dx + dy * dy;
            if (distance <= best_distance) {
                best = i;
                best_distance = distance;
            }
        }
        return best;
    }

    // Where a name placed at `origin` is sized about, moved with its point:
    // the nearest label kept within `max_move` pixels. False on a label's
    // first frame.
    [[nodiscard]] bool name_label_centre(
        float origin_x, float origin_y, float &x, float &y, float max_move = kNameLabelMove) const {
        const int best = nearest_name_centre(origin_x, origin_y, max_move);
        if (best < 0) return false;
        x = origin_x + name_centres[best].dx;
        y = origin_y + name_centres[best].dy;
        return true;
    }

    // The kept middle nearest a point, as a point on screen: where an icon
    // the label filed without its own point is sized about. False when no
    // label's middle is within `max_distance`.
    [[nodiscard]] bool nearest_name_middle(
        float x, float y, float &middle_x, float &middle_y, float max_distance = 64.0f) const {
        int best = -1;
        float best_distance = max_distance * max_distance;
        for (int i = 0; i < name_centre_count; ++i) {
            const float dx = name_centres[i].origin_x + name_centres[i].dx - x;
            const float dy = name_centres[i].origin_y + name_centres[i].dy - y;
            const float distance = dx * dx + dy * dy;
            if (distance <= best_distance) {
                best = i;
                best_distance = distance;
            }
        }
        if (best < 0) return false;
        middle_x = name_centres[best].origin_x + name_centres[best].dx;
        middle_y = name_centres[best].origin_y + name_centres[best].dy;
        return true;
    }

    // The weapon badge, left of its own point: traced at (-15, -3)-(-3, 13)
    // from it, 12 wide where the marks are 8 or 16. In the game's pixels.
    [[nodiscard]] static bool is_label_badge(Rect icon, float origin_x, float origin_y) {
        const Rect r = normalized(icon);
        const float gap = origin_x - r.x1;
        const float width = r.x1 - r.x0;
        return width >= 10.0f && width <= 14.0f && gap >= -2.0f && gap <= 8.0f && r.y0 < origin_y + 16.0f &&
            r.y1 > origin_y;
    }

    // Another piece filed with a point no more than 32 pixels from its middle
    // is taken for part of that label.
    [[nodiscard]] static bool own_point(Rect icon, float origin_x, float origin_y) {
        const Rect r = normalized(icon);
        return std::fabs(0.5f * (r.x0 + r.x1) - origin_x) <= 32.0f &&
            std::fabs(0.5f * (r.y0 + r.y1) - origin_y) <= 32.0f;
    }

    // A hall mark's shape: 16 tall, 16 wide (the quest paper) or 8 (the ready
    // mark). In the game's pixels.
    [[nodiscard]] static bool is_hall_mark_shape(Rect icon) {
        const Rect r = normalized(icon);
        const float width = r.x1 - r.x0;
        const float height = r.y1 - r.y0;
        return height >= 15.0f && height <= 17.0f &&
            ((width >= 7.0f && width <= 9.0f) || (width >= 15.0f && width <= 17.0f));
    }

    // The point, this frame, of the label a hall mark sits on. Traced with
    // MHFU_TRACE_HALL_NAMES and four hunters in the hall: the quest paper (16
    // wide) starts 3 pixels right of its hunter's point and the ready mark (8
    // wide) 7, both ending a pixel above it. They are filed with no point or
    // with another hunter's, so the nearest label was taken instead; with two
    // hunters close together the mark flipped between them. `mark` is in the
    // pixels the draw landed in, as is the point returned.
    [[nodiscard]] bool hall_mark_owner(Rect mark, InterfaceFit fit, float &origin_x, float &origin_y) const {
        const Rect g = fit.to_game(normalized(mark));
        int best = -1;
        float best_score = 0.0f;
        for (int i = 0; i < name_label_count; ++i) {
            const Rect point = fit.to_game(
                {name_labels[i].origin_x, name_labels[i].origin_y, name_labels[i].origin_x, name_labels[i].origin_y});
            const float dx = g.x0 - point.x0;
            const float dy = g.y1 - point.y0;
            if (dx < -1.0f || dx > 12.0f || dy < -4.0f || dy > 2.0f) continue;
            const float score = (dx - 5.0f) * (dx - 5.0f) + (dy + 1.0f) * (dy + 1.0f);
            if (best < 0 || score < best_score) {
                best = i;
                best_score = score;
            }
        }
        if (best < 0) return false;
        origin_x = name_labels[best].origin_x;
        origin_y = name_labels[best].origin_y;
        return true;
    }

    // What a hall draw is to its label, and the point it moves with, in the
    // pixels the draw landed in. A letter or the badge is filed under its own
    // point; a mark takes the point of the label it sits on, or none.
    struct HallPiece {
        bool mark{};
        bool has_point{};
        float x{};
        float y{};
    };
    // The last hall draw filed, for frame interpolation: a mark glides with
    // its own hunter's name, not the one whose point it was filed with.
    HallPiece hall_piece{};

    HallPiece file_hall_draw(const hud::NameDraw &named, Rect drawn, InterfaceFit fit) {
        HallPiece piece;
        const Rect game = fit.to_game(normalized(drawn));
        piece.x = named.origin_x;
        piece.y = named.origin_y;
        fit.apply(piece.x, piece.y);
        if (named.tag_only && !is_name_tag(game)) {
            // A plate larger than a name: neither, and not part of the label.
            piece.has_point = named.has_origin;
        } else if (named.glyph || (named.has_origin && is_label_badge(game, named.origin_x, named.origin_y)) ||
            (named.has_origin && !is_hall_mark_shape(game) && own_point(game, named.origin_x, named.origin_y))) {
            piece.has_point = named.has_origin;
            if (piece.has_point) note_name_label(piece.x, piece.y, drawn, named.glyph);
        } else {
            piece.mark = true;
            piece.has_point = hall_mark_owner(drawn, fit, piece.x, piece.y);
        }
        hall_piece = piece;
        return piece;
    }

    // The middle of a label's letters and the badge against them: an icon
    // whose right edge is within a few pixels of the first letter, beside the
    // letters' row. Without letters this frame, the middle the label had.
    [[nodiscard]] bool name_label_middle(const NameLabel &label, float &dx, float &dy) const {
        if (label.has_letters) {
            Rect box = label.letters;
            for (int i = 0; i < label.icon_count; ++i) {
                const Rect icon = label.icons[i];
                const float gap = label.letters.x0 - icon.x1;
                if (gap >= -2.0f && gap <= 8.0f && icon.y0 < label.letters.y1 && icon.y1 > label.letters.y0)
                    box = united(box, icon);
            }
            dx = 0.5f * (box.x0 + box.x1) - label.origin_x;
            dy = 0.5f * (box.y0 + box.y1) - label.origin_y;
            return true;
        }
        const int kept = nearest_name_centre(label.origin_x, label.origin_y);
        if (kept < 0) return false;
        dx = name_centres[kept].dx;
        dy = name_centres[kept].dy;
        return true;
    }

    void keep_name_centres() {
        NameCentre next[kNameLabels]{};
        int count = 0;
        for (int i = 0; i < name_label_count; ++i) {
            float dx = 0.0f;
            float dy = 0.0f;
            if (name_label_middle(name_labels[i], dx, dy))
                next[count++] = {name_labels[i].origin_x, name_labels[i].origin_y, dx, dy, 0};
        }
        // A label not drawn this frame keeps its middle for a while.
        for (int i = 0; i < name_centre_count && count < kNameLabels; ++i) {
            const NameCentre &old = name_centres[i];
            if (old.unseen + 1 > kNameLabelKeep) continue;
            bool drawn = false;
            for (int j = 0; j < name_label_count && !drawn; ++j) {
                const float dx = name_labels[j].origin_x - old.origin_x;
                const float dy = name_labels[j].origin_y - old.origin_y;
                drawn = dx * dx + dy * dy <= kNameLabelMove * kNameLabelMove;
            }
            if (!drawn) next[count++] = {old.origin_x, old.origin_y, old.dx, old.dy, old.unseen + 1};
        }
        for (int i = 0; i < count; ++i) name_centres[i] = next[i];
        name_centre_count = count;
        name_label_count = 0;
    }

    void note_party_letter(Rect rect) {
        const int row = party_letter_row(rect);
        if (row >= 0) party_letter_end[row] = normalized(rect).x1;
    }

    [[nodiscard]] bool continues_party_name(Rect rect) const {
        const int row = party_letter_row(rect);
        return row >= 0 && party_letter_end[row] > 0.0f &&
            std::fabs(normalized(rect).x0 - party_letter_end[row]) <= 0.5f;
    }

    void learn_reticle(Rect rect, std::uint32_t texture) {
        if (is_reticle(rect, texture) || reticles >= 8) return;
        const Rect r = normalized(rect);
        reticle[reticles++] = {texture, r.x1 - r.x0, r.y1 - r.y0};
    }

    [[nodiscard]] bool is_reticle(Rect rect, std::uint32_t texture) const {
        const Rect r = normalized(rect);
        for (int i = 0; i < reticles; ++i) {
            const float dw = (r.x1 - r.x0) - reticle[i].width;
            const float dh = (r.y1 - r.y0) - reticle[i].height;
            if (reticle[i].texture == texture && dw >= -1.5f && dw <= 1.5f && dh >= -1.5f && dh <= 1.5f) return true;
        }
        return false;
    }

    void begin() {
        map_rect_live = map_now ? map_rect : Rect{};
        map_live = map_now;
        vital_live = vital_now;
        bar_live = bar_now;
        sharp_live = sharp_now;
        spirit_live = spirit_now;
        bowgun_live = bowgun_now;
        list_live = list_now;
        panel_live = panel_now;
        party_window_live = party_window_now;
        party_window_top_live = party_window_top_now;
        map_now = vital_now = bar_now = sharp_now = spirit_now = bowgun_now = list_now = ammo_menu_now =
            travelling_now = obtain_banner_now = panel_now = party_window_now = false;
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
        menu_corner_live = menu_corner;
        menu_body_live = menu_body;
        menu_corner = false;
        menu_body = false;
        for (float &end : party_letter_end) end = 0.0f;
        keep_name_centres();
    }

    // The quest clock or the health bar, this frame or the one before.
    [[nodiscard]] bool status_up() const { return vital_now || vital_live; }

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

    // A horizontal edge of the obtain window, or the panel it is while it
    // slides down from the top. The area map's window is much taller.
    void remember_obtain_banner(Rect rect) {
        const Rect r = normalized(rect);
        const float width = r.x1 - r.x0;
        const float height = r.y1 - r.y0;
        if (width < 70.0f || width > 220.0f || height < 2.0f || height > 28.0f) return;
        if (r.x0 < 80.0f || r.x1 > 400.0f || r.y0 < -40.0f || r.y0 > 180.0f) return;
        include_rect(obtain_banner, r, obtain_banner_now);
    }

    // A cap or the item icon sitting on that window. The map's own picture
    // and its marks are not part of the banner.
    [[nodiscard]] bool on_obtain_banner(Rect rect, std::uint32_t texture) const {
        if (!obtain_banner_now || texture == kAreaMapTexture || texture == kMapIconTexture) return false;
        const Rect grown{
            obtain_banner.x0 - 8.0f, obtain_banner.y0 - 4.0f, obtain_banner.x1 + 24.0f, obtain_banner.y1 + 8.0f};
        return overlaps(grown, normalized(rect));
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
        dest = now ? united(dest, incoming) : normalized(incoming);
        now = true;
    }

    // The enlarged window, traced at 60 pixels for 40 texels. A window already
    // on the slot (pixel span matching the texels) clears it. The first vertex
    // carries the texel drawn at its own pixel, which fixes the bias.
    template <typename Vertex> void note_map_zoom(const std::vector<Vertex> &vertices, std::size_t first) {
        map_zoomed = false;
        if constexpr (requires {
                          vertices[first].u;
                          vertices[first].v;
                      }) {
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
    template <typename Vertex> void unzoom_quad(std::vector<Vertex> &vertices, std::size_t first, bool window) {
        if (!map_zoomed) return;
        if (window) {
            if constexpr (requires {
                              vertices[first].u;
                              vertices[first].v;
                          }) {
                for (std::size_t i = 0; i < 6u; ++i) {
                    vertices[first + i].x = kMapLeft + vertices[first + i].u;
                    vertices[first + i].y = kMapTop + vertices[first + i].v;
                }
                return;
            }
        }
        const bool texel_sized = [&] {
            if constexpr (requires {
                              vertices[first].u;
                              vertices[first].v;
                          }) {
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
        } else if (texture == kAreaMapTexture && (is_travelling_map_window(rect) || is_area_map_texture_window(rect)))
            include_rect(travelling_rect, rect, travelling_now);
        // A party-column icon is not enough on its own. A shop draws small
        // icons in the same corner, and treating one as the quest clock scaled
        // the shop. The icon still follows Status once the clock is up.
        if (is_vital_bar(rect) || is_clock_sprite(rect) || is_monster_eye(rect, texture) ||
            is_status_icon(rect, texture))
            vital_now = true;
        if (is_bar_panel(rect)) bar_now = true;
        if (is_sharpness_bar(rect) || is_sharpness_frame(rect)) sharp_now = true;
        if (is_spirit_bar(rect) || is_spirit_fill(rect) || is_loaded_round(rect) || is_bow_coating(rect) ||
            is_gunlance_shell(rect) || is_hunting_horn(rect))
            spirit_now = true;
        if (is_ammo_menu_body(rect)) menu_body = ammo_menu_now = true;
        // The menu opens from the ammo bar, so on the corner alone it opens
        // only while that bar is up. Choosing Combine's first material
        // redraws its windows, and on that frame a corner of them on this
        // row, with no ammo bar anywhere, opened the menu: the whole item
        // list shrank toward the item bar's corner for one frame (traced
        // with MHFU_TRACE_HUD_SCALE=all in a quest).
        if (is_ammo_menu_corner(rect)) {
            menu_corner = true;
            const bool opening = !menu_corner_live && (bowgun_now || bowgun_live);
            if (opening || menu_body_live || menu_body) ammo_menu_now = true;
        }
        remember_notice_edge(rect);
        remember_obtain_banner(rect);
        if (is_bowgun_icon(rect)) bowgun_now = true;
        // A map ping grown past the map reaches the item bar's band, the
        // list's own shape; taken for the list, it held the map as drawn.
        if (is_item_list(rect) && !is_map_ring(rect, texture)) list_now = true;
        if (is_map_panel(rect, texture)) panel_now = true;
        if (is_party_window_tile(rect)) {
            const float top = normalized(rect).y0;
            party_window_top_now = party_window_now ? std::min(party_window_top_now, top) : top;
            party_window_now = true;
        }
    }

    // The highest corner of a menu's window over the party list, this frame
    // or the one before. Only meaningful while one is up.
    [[nodiscard]] float party_window_top() const {
        if (party_window_now && party_window_live) return std::min(party_window_top_now, party_window_top_live);
        return party_window_now ? party_window_top_now : party_window_top_live;
    }

    // A window over the map, now or in the frame before.
    [[nodiscard]] bool panel_up() const { return panel_now || panel_live; }

    // A window between the traced spots is the map only while the quest clock
    // or the health bar is on screen, and only when the quad samples the
    // area-map texture. The shop and the return-to-town prompt have panels of
    // the same size in that corner; those panels use other textures.
    void adopt_travelling_map() {
        if (!travelling_now || map_now || !status_up()) return;
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
        if (!(map_now || map_live) || list_now || list_live || panel_up()) return false;
        if (is_notice_piece(rect, texture) || on_obtain_banner(rect, texture)) return false;
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
            if (texture == kAreaMapTexture && (is_travelling_map_window(rect) || is_area_map_texture_window(rect)))
                return (map_now || map_live) && status_up();
            // A window drawn over the map keeps every piece of its own, its
            // corners from the mark atlas included.
            if (panel_up()) return false;
            if ((map_now || map_live) && !list_now && !list_live && is_map_mark(rect) && in_map_slot(rect) &&
                !is_notice_piece(rect, texture))
                return true;
            return (map_now || map_live) && !list_now && !list_live &&
                overlaps(map_now ? map_rect : map_rect_live, rect);
        // A menu over the party list keeps every piece of its own there.
        case Piece::Vitals:
            return status_up() &&
                !((party_window_now || party_window_live) && covered_by_party_window(rect, party_window_top()));
        // The item list shares the bar's corner, and its right column shares
        // the bowgun slot. Neither is scaled while that list is on screen.
        // The quest item bar is on screen with the clock. A shop draws panels
        // in the same corner, and those stay as the game drew them.
        case Piece::ItemBar:
            return (bar_now || bar_live) && status_up() && !list_now && !list_live;
        case Piece::Sharpness:
            return sharp_now || sharp_live;
        case Piece::LongSword:
            return spirit_now || spirit_live;
        case Piece::Bowgun:
            return (bowgun_now || bowgun_live) && status_up() && !list_now && !list_live;
        case Piece::None:
            return false;
        }
        return false;
    }
};

// Moves the quad at `first` with Status, from the clock, as scale_quads moves
// the plates of the party list.
template <typename Vertex>
void place_with_status(std::vector<Vertex> &vertices, std::size_t first, const Scales &scales) {
    const Placement place = placement_of(Piece::Vitals, scales);
    for (std::size_t i = 0; i < 6u; ++i)
        place_point(vertices[first + i].x, vertices[first + i].y, place, scales.vitals);
}

// Party-column icons the game files outside the through-mode HUD, such as a
// small orthographic draw of the cat's face. Each moves with Status, and only
// while the clock is up. Returns whether any moved.
template <typename Vertex>
bool place_party_icons(std::vector<Vertex> &vertices, const Scales &scales, const Frame &frame) {
    if (!frame.status_up()) return false;
    bool moved = false;
    for (std::size_t first = 0; first + 6u <= vertices.size(); first += 6u) {
        if (!is_party_column_icon(quad_bounds(vertices, first))) continue;
        place_with_status(vertices, first, scales);
        moved = true;
    }
    return moved;
}

// A draw filed by a name label (hud::name_at): a name over another hunter, and
// icons the label files in the party list's badge column, such as the cat's
// face. The name takes the Names size about the point the game placed it, or a
// hall label about its own middle; the icons belong with the plates beside
// them and take Status. `vertices` are
// where the draw lands, after `fit`; the rules are in the game's pixels.
// Returns the bounds the name was moved to, for the scissor, when the Names
// size moved it.
template <typename Vertex>
std::optional<Rect> scale_name_draw(
    std::vector<Vertex> &vertices, const Scales &scales, Frame &frame, const hud::NameDraw &named, InterfaceFit fit) {
    // A hall draw is filed first, so frame interpolation has its point
    // whatever is decided below.
    const Frame::HallPiece piece =
        named.hall && !vertices.empty() ? frame.file_hall_draw(named, bounds_of(vertices), fit) : Frame::HallPiece{};
    // A village label can also file a plate larger than the name. That draw
    // stays as the game drew it.
    if (named.tag_only && !vertices.empty() && !is_name_tag(fit.to_game(bounds_of(vertices)))) return std::nullopt;
    std::vector<std::uint8_t> party_icon(vertices.size() / 6u);
    // A letter of the name is never a party icon: a name over a hunter that
    // crosses the party list, the map or the item bar keeps the Names size.
    if (frame.status_up() && !named.glyph)
        for (std::size_t first = 0; first + 6u <= vertices.size(); first += 6u)
            party_icon[first / 6u] = is_party_column_icon(fit.to_game(quad_bounds(vertices, first))) ? 1u : 0u;
    const std::vector<Vertex> drawn = vertices;
    // The text entry's point is in the game's pixels; the glyphs were pulled
    // in with the draw, so the point moves with them.
    float origin_x = named.origin_x;
    float origin_y = named.origin_y;
    if (named.has_origin) fit.apply(origin_x, origin_y);
    // A hall label is sized about the middle of its letters and badge
    // (Frame::NameLabel), once it has been seen for a frame.
    frame.name_pivot_kept = false;
    bool has_origin = named.has_origin;
    // The hall's marks over a hunter, the quest paper and the blinking ready
    // mark, are filed by the label after its name is queued. They come with
    // no point, or with the next hunter's, so a mark over one name was sized
    // toward another, across the screen, and jumped as it blinked. Such an
    // icon is sized about the middle of the label it sits on
    // (Frame::hall_mark_owner), else of the label nearest it.
    if (piece.mark) {
        const Rect box = bounds_of(drawn);
        float middle_x = piece.x;
        float middle_y = piece.y;
        if (piece.has_point) {
            // Its label's middle, moved with the label's point this frame. On
            // the label's first frame, the point itself, as its letters.
            float centre_x = 0.0f;
            float centre_y = 0.0f;
            if (frame.name_label_centre(piece.x, piece.y, centre_x, centre_y)) {
                middle_x = centre_x;
                middle_y = centre_y;
            }
        }
        if (piece.has_point ||
            frame.nearest_name_middle(0.5f * (box.x0 + box.x1), 0.5f * (box.y0 + box.y1), middle_x, middle_y)) {
            scale_name(vertices, scales.names, true, middle_x, middle_y);
            frame.name_pivot_x = middle_x;
            frame.name_pivot_y = middle_y;
            frame.name_pivot_kept = true;
        } else {
            scale_name(vertices, scales.names, false, 0.0f, 0.0f);
            frame.name_pivot_x = frame.name_pivot_y = 0.0f;
        }
        return scales.names == 1.0f ? std::nullopt : std::optional<Rect>(bounds_of(vertices));
    }
    if (named.hall && has_origin && !vertices.empty()) {
        float centre_x = 0.0f;
        float centre_y = 0.0f;
        if (frame.name_label_centre(origin_x, origin_y, centre_x, centre_y)) {
            origin_x = centre_x;
            origin_y = centre_y;
            frame.name_pivot_kept = true;
        }
    }
    frame.name_pivot_x = origin_x;
    frame.name_pivot_y = origin_y;
    scale_name(vertices, scales.names, has_origin, origin_x, origin_y);
    std::optional<Rect> name_bounds;
    for (std::size_t first = 0; first + 6u <= vertices.size(); first += 6u) {
        if (party_icon[first / 6u] != 0u) {
            std::copy_n(drawn.begin() + static_cast<std::ptrdiff_t>(first), 6u,
                vertices.begin() + static_cast<std::ptrdiff_t>(first));
            place_with_status(vertices, first, scales);
            continue;
        }
        if (scales.names == 1.0f) continue;
        const Rect scaled = quad_bounds(vertices, first);
        name_bounds = name_bounds ? united(*name_bounds, scaled) : scaled;
    }
    return name_bounds;
}

// One draw of a name over a hunter, as frame interpolation remembers it: the
// point the name was placed at, where the draw's first corner lies from that
// point, and what it samples. The game moves a name once per game frame; a
// present between two frames moves each draw by the blended move of its
// name's point, so the names glide with the characters they follow.
struct NamePlace {
    float origin_x{};
    float origin_y{};
    float dx{}; // first vertex minus the origin
    float dy{};
    float u{}; // first vertex's texture coordinates: the letter
    float v{};
    std::uint64_t texture{};
    std::uint32_t vertices{};
};

// The same letter of the same name in the newer frame: the same texture,
// texel and place within its name, on the nearest point no further than
// `max_move` pixels. -1 when there is none: a name that appeared, left, or
// jumped (a cut) stays where its frame drew it.
[[nodiscard]] inline int match_name_place(
    const NamePlace &older, const std::vector<NamePlace> &newer, float max_move = 64.0f) {
    int best = -1;
    float best_distance = max_move * max_move;
    for (std::size_t i = 0; i < newer.size(); ++i) {
        const NamePlace &n = newer[i];
        if (n.texture != older.texture || n.vertices != older.vertices) continue;
        if (std::fabs(n.u - older.u) > 0.01f || std::fabs(n.v - older.v) > 0.01f) continue;
        if (std::fabs(n.dx - older.dx) > 0.5f || std::fabs(n.dy - older.dy) > 0.5f) continue;
        const float mx = n.origin_x - older.origin_x;
        const float my = n.origin_y - older.origin_y;
        const float distance = mx * mx + my * my;
        if (distance <= best_distance) {
            best_distance = distance;
            best = static_cast<int>(i);
        }
    }
    return best;
}

// One arm of a bowgun's reticle: a cross of four sprites from the HUD atlas,
// two 12 by 4 and two 4 by 12, each its own draw. Traced with
// MHFU_TRACE_HUD_SCALE=all and a bowgun aimed in a quest: as an aim starts the
// cross flies in from off the picture (x 1963 to 480 over ten frames) and
// follows the aim from there, so it can reach the map, the bars, the item
// bar or the ammo bar without ever crossing the middle, where an aim's
// reticle is otherwise learned. Taken for those pieces, its arms shrank
// toward their corners. Nothing else in the atlas is either size, in quests,
// menus, the hall and the pouch.
[[nodiscard]] inline bool is_reticle_arm(Rect rect, std::uint32_t texture) {
    if (texture != kHudAtlasTexture) return false;
    const Rect r = normalized(rect);
    const float width = r.x1 - r.x0;
    const float height = r.y1 - r.y0;
    const auto near = [](float a, float b) { return std::fabs(a - b) <= 0.5f; };
    return (near(width, 12.0f) && near(height, 4.0f)) || (near(width, 4.0f) && near(height, 12.0f));
}

// The middle of the picture, (240, 136), where an aim's reticle starts. No
// piece of the HUD is drawn there.
[[nodiscard]] inline bool in_reticle_area(Rect rect) {
    const Rect r = normalized(rect);
    const float cx = 0.5f * (r.x0 + r.x1);
    const float cy = 0.5f * (r.y0 + r.y1);
    const float width = r.x1 - r.x0;
    const float height = r.y1 - r.y0;
    return cx >= 176.0f && cx <= 304.0f && cy >= 88.0f && cy <= 184.0f && width <= 128.0f && height <= 128.0f;
}

// Whether a draw from `owner` may be resized at all. Other code's draws
// still pass through scale_quads, so the frame knows a menu is on screen.
[[nodiscard]] inline bool may_resize(hud::Owner owner) {
    return owner != hud::Owner::Other;
}

// Why a quad was given the piece it was, or left as drawn. Every rule that
// takes a quad for a piece, or turns it away, is one of these, so a menu
// taken for a piece names the rule that took it (MHFU_TRACE_HUD_SCALE).
enum class Reason : std::uint8_t {
    // Taken for `piece`.
    Shape,      // piece_of: the piece's own rectangle and shape
    AmmoMenu,   // inside the open ammo menu, which takes the item bar's size
    MapTexture, // a window of the area-map texture outside the slot
    MapMark,    // a mark whose box crosses the map drawn this frame
    PartyName,  // the next letter of a name in the party list, off the grid
    Site,       // the part of the quest HUD that drew it (hud::Site); with no piece, left as drawn
    // Left as drawn.
    NoPiece,      // no rule took it
    ObtainBanner, // on the obtain window ("Herb obtained.")
    NoticeBorder, // the long notice's 16-tall border, on its 8-tall cap
    Reticle,      // an aim's reticle
    PieceNotUp,   // its piece is not on screen, or a window covers it
    NotOwner,     // filed by code other than the quest HUD
};

[[nodiscard]] inline const char *reason_name(Reason reason) {
    switch (reason) {
    case Reason::Shape:
        return "shape";
    case Reason::AmmoMenu:
        return "ammo-menu";
    case Reason::MapTexture:
        return "map-texture";
    case Reason::MapMark:
        return "map-mark";
    case Reason::PartyName:
        return "party-name";
    case Reason::Site:
        return "site";
    case Reason::NoPiece:
        return "no-piece";
    case Reason::ObtainBanner:
        return "obtain-banner";
    case Reason::NoticeBorder:
        return "notice-border";
    case Reason::Reticle:
        return "reticle";
    case Reason::PieceNotUp:
        return "piece-not-up";
    case Reason::NotOwner:
        return "not-owner";
    }
    return "no-piece";
}

// `piece` is what the quad is sized as, None when it is left as drawn.
// `candidate` is what the rules took it for before it was turned away, so a
// trace shows a menu that was nearly taken for the map.
struct Decision {
    Piece piece{Piece::None};
    Reason reason{Reason::NoPiece};
    Piece candidate{Piece::None};
};

// The piece one quest HUD quad belongs to, with the rule that decided it.
// `frame` already holds every quad of this draw. It learns an aim's reticle
// here, so each quad is decided once and in the order drawn.
[[nodiscard]] inline Decision decide(Frame &frame, Rect bounds, std::uint32_t texture) {
    // A bowgun's reticle stays as drawn wherever the aim takes it, the ammo
    // menu's corner included.
    if (is_reticle_arm(bounds, texture)) return {Piece::None, Reason::Reticle, piece_of(bounds, texture)};
    // The ammo menu is one panel over the item bar's corner, sized with
    // the item bar. Only this frame's menu counts: the frame after it
    // closes draws the ammo bar in the same place.
    // The ammo menu opens over the quest item bar, so the clock is up.
    // A shop highlight is the same shape and must not take this size.
    const bool in_ammo_menu = frame.ammo_menu_now && frame.status_up() &&
        contains({kAmmoMenuLeft, kAmmoMenuTop, kAmmoMenuRight, kAmmoMenuBottom}, bounds);
    Piece piece = in_ammo_menu ? Piece::ItemBar : piece_of(bounds, texture);
    Reason reason = in_ammo_menu ? Reason::AmmoMenu : Reason::Shape;
    // A window from the area-map texture that sits outside the slot is
    // still the map. piece_of only knows the slot.
    if (!in_ammo_menu && texture == kAreaMapTexture && is_area_map_texture_window(bounds)) {
        piece = Piece::Map;
        reason = Reason::MapTexture;
    }
    // The arrow's box crosses the map, or lands in the bowgun slot, while
    // the map it belongs to is the one drawn this frame.
    if (!in_ammo_menu && (piece == Piece::None || piece == Piece::Bowgun) && frame.on_map(bounds, texture)) {
        piece = Piece::Map;
        reason = Reason::MapMark;
    }
    // A paintball's ring is larger than a mark, so neither rule above takes
    // it. Left as drawn, it sat off a smaller map beside the monster's dot.
    if (!in_ammo_menu && piece == Piece::None && frame.quest_owned && is_map_ring(bounds, texture)) {
        piece = Piece::Map;
        reason = Reason::MapMark;
    }
    // A name in the party list goes on where its last letter ended, though
    // a narrow letter puts the rest off the grid piece_of knows.
    if (!in_ammo_menu && piece == Piece::None && frame.continues_party_name(bounds)) {
        piece = Piece::Vitals;
        reason = Reason::PartyName;
    }
    const Piece candidate = piece;
    // The obtain window ("Herb obtained.") crosses the map and the name
    // rows. Its frame, the item icon and the name stay as drawn. A weapon
    // piece is the same short wide shape and still takes its own size.
    bool on_banner = false;
    if (frame.on_obtain_banner(bounds, texture) &&
        (piece == Piece::Map || piece == Piece::Vitals || piece == Piece::ItemBar || piece == Piece::Bowgun)) {
        piece = Piece::None;
        on_banner = true;
    }
    // The long notice's 16-tall border sits on its 8-tall cap. It is the
    // size of a map mark, so the mark rule would take it.
    if ((piece == Piece::Map || piece == Piece::Bowgun) && frame.beside_notice_edge(bounds))
        return {Piece::None, Reason::NoticeBorder, candidate};
    // A reticle is learned by its texture and size, and the font and the
    // HUD atlas are shared: an overhead name's letter or a 16 by 8 window
    // tile near the middle was learned, and every letter of the party list
    // and every corner of the item bar then kept the game's size until the
    // aim ended (traced with MHFU_TRACE_HUD_SCALE=all in a quest). The
    // reticle only needs keeping off the map and the bowgun slot, the pieces
    // a mark-sized sprite crossing the right of the picture is taken for.
    // A letter of the font is never the reticle: an overhead name's letter
    // learned near the middle kept the bowgun slot's ammo digits, the same
    // size in the same font, at the game's size while the slot shrank.
    if (frame.aiming && texture != 0u) {
        if (piece == Piece::None && texture != kFontTexture && in_reticle_area(bounds))
            frame.learn_reticle(bounds, texture);
        if ((piece == Piece::Map || piece == Piece::Bowgun) && frame.is_reticle(bounds, texture))
            return {Piece::None, Reason::Reticle, candidate};
    }
    if (piece == Piece::None) return {Piece::None, on_banner ? Reason::ObtainBanner : Reason::NoPiece, candidate};
    if (!in_ammo_menu && !frame.allows(piece, bounds, texture)) return {Piece::None, Reason::PieceNotUp, candidate};
    if (piece == Piece::Vitals) frame.note_party_letter(bounds);
    return {piece, reason, candidate};
}

// The piece a part of the quest HUD is sized as (hud::Site), or None when it
// stays as the game drew it. The weapon's gauges are the stack under stamina
// and the ammo bar sits on the item bar: placement_of keeps each with them.
[[nodiscard]] inline Piece piece_of_site(hud::Site site) {
    switch (site) {
    case hud::Site::Status:
        return Piece::Vitals;
    case hud::Site::Weapon:
        return Piece::LongSword;
    case hud::Site::Map:
        return Piece::Map;
    case hud::Site::ItemBar:
        return Piece::ItemBar;
    case hud::Site::AmmoBar:
        return Piece::Bowgun;
    case hud::Site::None:
    case hud::Site::Unlisted:
    case hud::Site::Window:
    case hud::Site::Names:
        return Piece::None;
    }
    return Piece::None;
}

// Whether a map draw is the map's picture, by the call in 0x08842F70 that
// drew it (hud::SiteDraw::call): the full tile at 0x088432E8, 152 by 152, or
// the window into it at 0x08843610 that the hunter's zone shows without the
// Map item. Traced with MHFU_TRACE_CAPTURE. Every other call there draws a
// mark on it: the camp icon at 0x088437B4, the hunters' arrows at 0x0884398C
// and 0x08843A2C (turned quads), a monster's dot.
[[nodiscard]] constexpr bool is_map_picture_call(std::uint32_t call) {
    return call == 0x088432E8u || call == 0x08843610u;
}

// Whether a quest HUD draw is decided by its site rather than its shape: the
// site is known because the draws are watched and the code is the code it was
// read from. The renderer passes Site::None under MHFU_HUD_SCALE_SITES=0,
// which goes back to the shapes alone.
[[nodiscard]] inline bool by_site(hud::Owner owner, hud::Site site) {
    return owner == hud::Owner::QuestHud && site != hud::Site::None;
}

// One through sprite, or one through quad, is six vertices (two triangles)
// after it is expanded. Groups of six are scaled when decide() gives them a
// piece. Every draw is noted in `frame`, so an item list or a notice over the
// HUD still holds the pieces under it; only a draw `owner` allows is moved.
// `decisions`, when given, gets one Decision per quad, in order, even for a
// draw that is not moved.
//
// With a site (by_site), the part of the quest HUD that drew the draw is its
// piece, every quad of it, whatever its shape or what else is on screen. The
// rectangles then only place it: each piece keeps its corner, and the map's
// enlarged window is put back on the slot. Which map quads are the map's
// picture, and so measure the enlargement, is the call that drew them
// (is_map_picture_call).
template <typename Vertex>
void scale_quads(std::vector<Vertex> &vertices, const Scales &scales, Frame &frame, std::uint32_t texture = 0,
    hud::Owner owner = hud::Owner::Unknown, std::vector<Decision> *decisions = nullptr, hud::SiteDraw drawn = {}) {
    if (decisions != nullptr) decisions->clear();
    if (!scales.any() || vertices.size() < 6u) return;
    const bool sited = by_site(owner, drawn.site);
    const bool picture = sited && drawn.site == hud::Site::Map && is_map_picture_call(drawn.call);
    for (std::size_t first = 0; first + 6u <= vertices.size(); first += 6u) {
        const Rect bounds = quad_bounds(vertices, first);
        frame.note(bounds, texture);
        if (sited ? picture : texture == kAreaMapTexture && is_drawn_map(bounds, texture))
            frame.note_map_zoom(vertices, first);
    }
    frame.dismiss_false_menu();
    frame.adopt_travelling_map();
    if (!may_resize(owner)) {
        if (decisions != nullptr)
            for (std::size_t first = 0; first + 6u <= vertices.size(); first += 6u)
                decisions->push_back({Piece::None, Reason::NotOwner, piece_of(quad_bounds(vertices, first), texture)});
        return;
    }
    if (!frame.aiming) frame.reticles = 0;
    frame.quest_owned = owner == hud::Owner::QuestHud;
    const Piece site_piece = piece_of_site(drawn.site);
    for (std::size_t first = 0; first + 6u <= vertices.size(); first += 6u) {
        const Rect bounds = quad_bounds(vertices, first);
        const Decision decision =
            sited ? Decision{site_piece, Reason::Site, site_piece} : decide(frame, bounds, texture);
        if (decisions != nullptr) decisions->push_back(decision);
        const Piece piece = decision.piece;
        if (piece == Piece::None) continue;
        // The no-map window and the marks on it are enlarged. Put them on the
        // slot's placement first, then apply the Map size. At 100% the
        // placement is the whole change.
        if (piece == Piece::Map && frame.map_zoomed)
            frame.unzoom_quad(
                vertices, first, sited ? picture : texture == kAreaMapTexture && is_drawn_map(bounds, texture));
        const float scale = scales.of(piece);
        const Placement place = piece == Piece::Map
            ? Placement{frame.map_corner.x, frame.map_corner.y, frame.map_corner.x, frame.map_corner.y}
            : placement_of(piece, scales);
        const bool moved = place.attach_x != place.origin_x || place.attach_y != place.origin_y;
        if (scale == 1.0f && !moved) continue;
        for (std::size_t i = 0; i < 6u; ++i) place_point(vertices[first + i].x, vertices[first + i].y, place, scale);
    }
}

// Whether the quad at `first` is not where the game drew it.
template <typename Vertex>
[[nodiscard]] bool quad_moved(const std::vector<Vertex> &drawn, const std::vector<Vertex> &scaled, std::size_t first) {
    for (std::size_t i = first; i < first + 6u; ++i)
        if (scaled[i].x != drawn[i].x || scaled[i].y != drawn[i].y) return true;
    return false;
}

// Where the area map's quads went, when scale_quads moved any of them. The
// game's scissor is often the rectangle it drew the map in, so a larger map
// is clipped back to that size unless the scissor takes these bounds, and the
// renderer redraws a moved map over the one already in the frame.
// With a site, the draw is the map's picture or it is not (`picture`, from
// is_map_picture_call); without one, its rectangles say.
template <typename Vertex>
[[nodiscard]] std::optional<Rect> moved_map_bounds(const std::vector<Vertex> &drawn, const std::vector<Vertex> &scaled,
    std::uint32_t texture, std::optional<bool> picture = std::nullopt) {
    std::optional<Rect> bounds;
    for (std::size_t first = 0; first + 6u <= drawn.size(); first += 6u) {
        const bool map = picture ? *picture : is_drawn_map(quad_bounds(drawn, first), texture);
        if (!quad_moved(drawn, scaled, first) || !map) continue;
        const Rect moved = quad_bounds(scaled, first);
        bounds = bounds ? united(*bounds, moved) : moved;
    }
    return bounds;
}

} // namespace mhfu::gpu::hud_scale
