#pragma once

#include <algorithm>
#include <cstddef>
#include <vector>

namespace mhfu::gpu::hud_scale {

// The quest map and the item bar, smaller or larger, pinned to the corners
// they already occupy. Traced on USA Freedom Unite (ULUS-10391) in a quest's base camp
// with MHFU_TRACE_SPRITES, presented frames 2280 (the bar closed) and 2850
// (the bar open, L held). The Portable 3rd HUD classification does not match
// this executable, so the two are told apart by where they are drawn:
//
// - The map is one through-mode sprite at (8, 8)-(60, 56), 52 by 48, from a
//   256-wide atlas (uv 0,32-52,80). It did not move across the traced frames.
//   Anything drawn on top of that rectangle (a marker the camp frame did not
//   have) is scaled about the same corner so it stays on the map.
// - The item bar, open or closed, is the through sprites inside x 312-472 and
//   y 200-264: the panel, the icons, the L prompt and the item's name. The
//   right edge stays at 472. The bottom is 260 with the bar closed and 264
//   with it open. Health, stamina, sharpness and the name plate are above
//   that, and the supply-box icons in the camp sit higher (around y 85 and
//   y 152).

enum class Piece { None, Map, ItemBar };

struct Rect {
    float x0{};
    float y0{};
    float x1{};
    float y1{};
};

// Where a piece stays put while it shrinks or grows: the map's top-left, the
// item bar's bottom-right.
struct Anchor {
    float x{};
    float y{};
};

inline constexpr float kMapLeft = 8.0f;
inline constexpr float kMapTop = 8.0f;
inline constexpr float kItemRight = 472.0f;
inline constexpr float kItemBottom = 264.0f;

[[nodiscard]] inline Piece piece_of(Rect rect) {
    const float x0 = std::min(rect.x0, rect.x1);
    const float x1 = std::max(rect.x0, rect.x1);
    const float y0 = std::min(rect.y0, rect.y1);
    const float y1 = std::max(rect.y0, rect.y1);
    const float width = x1 - x0;
    const float height = y1 - y0;
    if (width < 4.0f || height < 4.0f) return Piece::None;
    const float cx = (x0 + x1) * 0.5f;
    const float cy = (y0 + y1) * 0.5f;
    // Inside the map's rectangle, and not a bar that merely starts beside it.
    if (width <= 72.0f && height <= 64.0f && x0 >= 0.0f && y0 >= 0.0f && x1 <= 72.0f && y1 <= 64.0f && cx <= 60.0f &&
        cy <= 52.0f)
        return Piece::Map;
    // The bottom-right cluster. A single panel piece is at most 128 wide.
    if (width <= 200.0f && height <= 80.0f && x0 >= 300.0f && x1 <= 480.0f && y0 >= 196.0f && y1 <= 272.0f)
        return Piece::ItemBar;
    return Piece::None;
}

[[nodiscard]] inline Anchor anchor_of(Piece piece) {
    if (piece == Piece::Map) return {kMapLeft, kMapTop};
    if (piece == Piece::ItemBar) return {kItemRight, kItemBottom};
    return {};
}

// `scale` is 1 at the game's size. The anchor point does not move.
inline void scale_point(float &x, float &y, Anchor anchor, float scale) {
    x = anchor.x + (x - anchor.x) * scale;
    y = anchor.y + (y - anchor.y) * scale;
}

// One through sprite is six vertices (two triangles) after it is expanded.
// Groups of six are scaled when they are the map or the item bar.
template <typename Vertex> void scale_quads(std::vector<Vertex> &vertices, float scale) {
    if (scale == 1.0f) return;
    for (std::size_t first = 0; first + 6u <= vertices.size(); first += 6u) {
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
        const Piece piece = piece_of({x0, y0, x1, y1});
        if (piece == Piece::None) continue;
        const Anchor anchor = anchor_of(piece);
        for (std::size_t i = 0; i < 6u; ++i) scale_point(vertices[first + i].x, vertices[first + i].y, anchor, scale);
    }
}

} // namespace mhfu::gpu::hud_scale
