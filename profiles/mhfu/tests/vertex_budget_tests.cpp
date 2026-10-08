// Which machines take the larger per-frame vertex budget. No GPU is used.

#include "gpu/vertex_budget.hpp"

#include <cstdint>
#include <iostream>

namespace {

using mhfu::gpu::choose_vertex_hold;
using mhfu::gpu::choose_vertex_region;
using mhfu::gpu::kVertexFullHeap;
using mhfu::gpu::kVertexHoldBase;
using mhfu::gpu::kVertexHoldFull;
using mhfu::gpu::kVertexRegionBase;
using mhfu::gpu::kVertexRegionFull;
using mhfu::gpu::vertex_storage_bytes;

int failures = 0;

void check(bool condition, const char *message) {
    if (!condition) {
        ++failures;
        std::cerr << "FAIL: " << message << '\n';
    }
}

void budgets() {
    check(vertex_storage_bytes(kVertexRegionBase) == 396ull << 20u, "the smaller buffer is 396 MiB");
    check(vertex_storage_bytes(kVertexRegionFull) == 780ull << 20u, "the larger buffer is 780 MiB");

    const std::uint64_t full = vertex_storage_bytes(kVertexRegionFull);
    check(choose_vertex_region(full, kVertexFullHeap) == kVertexRegionFull,
        "a large heap that can allocate the buffer takes it");
    check(choose_vertex_region(0u, kVertexFullHeap) == kVertexRegionFull,
        "an unreported allocation limit still takes it when the heap can");
    check(choose_vertex_region(full, kVertexFullHeap - 1u) == kVertexRegionBase,
        "a heap under 8 GiB keeps the smaller budget");
    check(choose_vertex_region(full - 1u, kVertexFullHeap) == kVertexRegionBase,
        "a device that cannot allocate the larger buffer keeps the smaller one");
    check(choose_vertex_region(512ull << 20u, 4ull << 30u) == kVertexRegionBase,
        "a phone-sized heap and allocation stay small");
    check(choose_vertex_region(0u, 6ull << 30u) == kVertexRegionBase,
        "an unknown limit does not override a small heap");
}

void hold_budgets() {
    check(kVertexHoldBase == 192ull << 20u, "the smaller hold is 192 MiB");
    check(kVertexHoldFull == 384ull << 20u, "the larger hold is 384 MiB");
    check(choose_vertex_hold(kVertexHoldFull, kVertexFullHeap) == kVertexHoldFull,
        "a large heap that can allocate the hold takes it");
    check(choose_vertex_hold(0u, kVertexFullHeap) == kVertexHoldFull,
        "an unreported allocation limit still takes the larger hold when the heap can");
    check(choose_vertex_hold(kVertexHoldFull, kVertexFullHeap - 1u) == kVertexHoldBase,
        "a heap under 8 GiB keeps the smaller hold");
    check(choose_vertex_hold(kVertexHoldFull - 1u, kVertexFullHeap) == kVertexHoldBase,
        "a device that cannot allocate the larger hold keeps the smaller one");
    check(choose_vertex_hold(0u, 6ull << 30u) == kVertexHoldBase,
        "an unknown limit does not override a small heap for the hold");
}

} // namespace

int main() {
    budgets();
    hold_budgets();
    if (failures != 0) {
        std::cerr << failures << " check(s) failed\n";
        return 1;
    }
    std::cout << "vertex budget tests passed\n";
    return 0;
}
