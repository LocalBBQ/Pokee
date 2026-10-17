#pragma once

// Interface pieces that follow frame interpolation. The interface is built
// once per game frame, over the newer of the two frames an interpolated
// present blends, and the presents until the next flip draw that same build.
// A piece pinned to the world, such as the gather mark, would then sit where
// the newer frame has it while the picture under it is still on its way
// there, and jump 30 times a second. A registered piece is moved, before each
// present draws the interface, to where it was in the older frame blended
// towards the newer one by that present's weight.
//
// MHFU_GATHER_ICON_SMOOTH=0 leaves the gather mark where the game frame
// put it (hle_media.cpp registers nothing), and MHFU_DAMAGE_NUMBERS_SMOOTH=0
// does the same for damage numbers (menu.cpp registers none).

#include <array>

struct ImDrawList;

namespace mhfu::gpu::ui_motion {

// One piece's shift. `older` is the older frame's place minus the newer
// one's, in window pixels. At weight t the piece is drawn (1 - t) * older
// away from where it was built; `applied` is the shift its vertices carry, so
// the next present moves them by the difference only.
struct Shift {
    std::array<float, 2> older{};
    std::array<float, 2> applied{};

    // The move to add to the vertices for weight `t` (clamped to 0..1).
    [[nodiscard]] std::array<float, 2> to(float t) {
        t = t < 0.0f ? 0.0f : t > 1.0f ? 1.0f : t;
        const std::array<float, 2> want{(1.0f - t) * older[0], (1.0f - t) * older[1]};
        const std::array<float, 2> move{want[0] - applied[0], want[1] - applied[1]};
        applied = want;
        return move;
    }
};

// Forgets the pieces of the last build. Called as each interface frame begins.
void begin_frame();
// The vertices [begin, end) of `list`, drawn at the newer frame's place, were
// `older` away in the older frame.
void add(ImDrawList *list, int begin, int end, std::array<float, 2> older);
// Before a present draws the interface: weight 0 shows the older frame, 1
// the newer one.
void apply(float t);

} // namespace mhfu::gpu::ui_motion
