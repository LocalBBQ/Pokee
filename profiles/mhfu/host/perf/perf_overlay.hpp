#pragma once

#include <cstdint>

namespace mhfu::perf {

// The overlay is drawn on the CPU into a small RGBA image, which the renderer
// scales onto the top-left corner of the presented frame. The compact one is
// a single line in the bottom-left corner, clear of the health bar and clock.
inline constexpr std::uint32_t kOverlayWidth = 196u;
inline constexpr std::uint32_t kOverlayHeight = 72u;
inline constexpr std::uint32_t kCompactOverlayHeight = 11u;

struct Summary;

// Fills `pixels` (kOverlayWidth x kOverlayHeight, red in the low byte) with the
// last second's numbers and a graph of recent frame times.
void draw_overlay(std::uint32_t *pixels);
// Fills the first kCompactOverlayHeight rows of `pixels` (kOverlayWidth wide)
// with the frames shown, the game's own frame rate and its speed, from
// `summary`; the line is green at full speed, yellow from 90% and red below.
// Before the first summary it shows dashes rather than zeros.
void draw_compact_overlay(std::uint32_t *pixels, const Summary &summary);

// Rows of the image the overlay in use fills: the full panel or the compact line.
[[nodiscard]] std::uint32_t overlay_rows(bool compact);

// Integer scale for an overlay drawn over an image `height` pixels tall, so it
// stays readable from a 544-line window up to a 4K screen.
[[nodiscard]] std::uint32_t overlay_scale(std::uint32_t height);

} // namespace mhfu::perf
