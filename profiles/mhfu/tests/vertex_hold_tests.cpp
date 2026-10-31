// Holes in the vertex hold are reused after the frames that read them finish.
// No GPU is used.

#include "gpu/vertex_hold.hpp"

#include <cstdint>
#include <iostream>

namespace {

using mhfu::gpu::VertexHoldHeap;
using mhfu::gpu::kVertexHoldRetireFrames;

int failures = 0;

[[nodiscard]] std::uint64_t align_size(std::uint64_t bytes) {
    return (bytes + mhfu::gpu::kVertexHoldAlign - 1u) & ~(mhfu::gpu::kVertexHoldAlign - 1u);
}

void check(bool condition, const char *message) {
    if (!condition) {
        ++failures;
        std::cerr << "FAIL: " << message << '\n';
    }
}

void reuse_a_hole() {
    VertexHoldHeap heap;
    heap.reset(4096u);
    const auto first = heap.place(40u, 0u);
    const auto second = heap.place(80u, 0u);
    check(first.offset == 0u, "the first span starts at 0");
    check(first.reserved == 48u, "40 bytes reserves an aligned 48");
    check(second.offset == 48u, "the next span follows the reservation");
    check(
        heap.release(first.offset, first.reserved, 0u, 0u) == 0u, "a hole under a live span does not move the cursor");
    const auto again = heap.place(40u, 0u);
    check(again.offset == first.offset, "a freed span is reused");
    check(heap.used() == second.offset + second.reserved, "reusing a hole does not grow the cursor");
}

void a_zone_returns_its_space() {
    VertexHoldHeap heap;
    heap.reset(4096u);
    VertexHoldHeap::Placement spans[4]{};
    for (std::uint64_t i = 0u; i < 4u; ++i) spans[i] = heap.place(100u, 1u);
    const std::uint64_t filled = heap.used();
    std::uint64_t freed = 0u;
    for (const auto &span : spans) freed += heap.release(span.offset, span.reserved, 1u, 1u);
    check(freed == filled, "releasing a whole zone pulls the cursor back");
    check(heap.used() == 0u, "an empty hold has nothing in use");
    const auto next = heap.place(filled, 2u);
    check(next.offset == 0u, "the next zone starts over");
    check(heap.used() == align_size(filled), "the next zone fits in what the previous one used");
}

void retire_before_reuse() {
    VertexHoldHeap heap;
    heap.reset(4096u);
    const auto span = heap.place(64u, 5u);
    check(heap.release(span.offset, span.reserved, 5u + kVertexHoldRetireFrames, 5u) == 0u,
        "a span still being presented is not returned yet");
    const auto early = heap.place(64u, 5u + kVertexHoldRetireFrames - 1u);
    check(early.offset != span.offset, "the hole stays unread until the retire frame");
    const auto later = heap.place(64u, 5u + kVertexHoldRetireFrames);
    check(later.offset == span.offset, "the hole is reused once the retire frame is reached");
}

void adjacent_holes_fit_a_larger_span() {
    VertexHoldHeap heap;
    heap.reset(4096u);
    const auto first = heap.place(32u, 0u);
    const auto second = heap.place(32u, 0u);
    const std::uint64_t end = heap.used();
    heap.release(first.offset, first.reserved, 0u, 0u);
    heap.release(second.offset, second.reserved, 0u, 0u);
    const auto combined = heap.place(first.reserved + second.reserved, 0u);
    check(combined.offset == 0u, "adjacent holes become one");
    check(heap.used() == end, "the combined span fits without appending");
}

void full_hold_refuses() {
    VertexHoldHeap heap;
    heap.reset(64u);
    const auto span = heap.place(64u, 0u);
    check(span.offset == 0u, "a span that fills the hold is placed");
    check(heap.place(16u, 0u).offset == VertexHoldHeap::kNone, "a full hold refuses another span");
    heap.release(span.offset, span.reserved, 0u, 0u);
    check(heap.place(16u, 0u).offset == 0u, "space returned after a refusal can be placed again");
}

} // namespace

int main() {
    reuse_a_hole();
    a_zone_returns_its_space();
    retire_before_reuse();
    adjacent_holes_fit_a_larger_span();
    full_hold_refuses();
    if (failures != 0) {
        std::cerr << failures << " check(s) failed\n";
        return 1;
    }
    std::cout << "vertex hold tests passed\n";
    return 0;
}
