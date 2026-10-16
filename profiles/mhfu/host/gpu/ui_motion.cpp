#include "gpu/ui_motion.hpp"

#include "imgui.h"

#include <vector>

namespace mhfu::gpu::ui_motion {
namespace {

struct Piece {
    ImDrawList *list{};
    int begin{};
    int end{};
    Shift shift;
};

std::vector<Piece> &pieces() {
    static std::vector<Piece> value;
    return value;
}

} // namespace

void begin_frame() {
    pieces().clear();
}

void add(ImDrawList *list, int begin, int end, std::array<float, 2> older) {
    if (list == nullptr || end <= begin) return;
    pieces().push_back(Piece{list, begin, end, Shift{older, {}}});
}

void apply(float t) {
    for (Piece &piece : pieces()) {
        // A list rebuilt since (another interface frame) is left alone.
        if (piece.end > piece.list->VtxBuffer.Size) continue;
        const std::array<float, 2> move = piece.shift.to(t);
        if (move[0] == 0.0f && move[1] == 0.0f) continue;
        for (int i = piece.begin; i < piece.end; ++i) {
            piece.list->VtxBuffer[i].pos.x += move[0];
            piece.list->VtxBuffer[i].pos.y += move[1];
        }
    }
}

} // namespace mhfu::gpu::ui_motion
