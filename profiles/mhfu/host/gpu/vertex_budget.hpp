#pragma once

#include <cstdint>

namespace mhfu::gpu {

// One frame's vertices. Three frames are kept when frame interpolation is on,
// plus a scratch area for each of the two presents that can be in flight.
// The larger budget holds the volcano; it is a single allocation kept for the
// whole session. A phone's heap cannot spare that, so those devices keep the
// budget they already run.
inline constexpr std::uint64_t kVertexRegionFull = 256ull << 20u;
inline constexpr std::uint64_t kVertexRegionBase = 128ull << 20u;
inline constexpr std::uint64_t kVertexRegions = 3ull;
inline constexpr std::uint64_t kVertexScratch = 6ull << 20u;
// Host-visible heap required before the larger budget is even attempted.
// Below this the process stays on the smaller one, including phones.
inline constexpr std::uint64_t kVertexFullHeap = 8ull << 30u;

[[nodiscard]] inline constexpr std::uint64_t vertex_storage_bytes(std::uint64_t region) {
    return kVertexRegions * region + 2ull * kVertexScratch;
}

// `max_allocation` is the device's largest allocation, or 0 when it is not
// reported. `host_heap` is the largest heap that can be mapped by the CPU.
[[nodiscard]] inline constexpr std::uint64_t choose_vertex_region(
    std::uint64_t max_allocation, std::uint64_t host_heap) {
    const std::uint64_t full = vertex_storage_bytes(kVertexRegionFull);
    const bool allocation_fits = max_allocation == 0u || max_allocation >= full;
    return allocation_fits && host_heap >= kVertexFullHeap ? kVertexRegionFull : kVertexRegionBase;
}

// Unchanged decoded vertices, kept for the session and drawn in place. The
// tower's static geometry is about 233 MiB. A 192 MiB hold filled, and the
// cache then dropped every span, so the next frames decoded the whole map
// again. Machines that take the larger per-frame budget can keep 384 MiB.
// A smaller heap stays at 192. A refused allocation falls back, then off.
inline constexpr std::uint64_t kVertexHoldFull = 384ull << 20u;
inline constexpr std::uint64_t kVertexHoldBase = 192ull << 20u;

[[nodiscard]] inline constexpr std::uint64_t choose_vertex_hold(
    std::uint64_t max_allocation, std::uint64_t host_heap) {
    const bool allocation_fits = max_allocation == 0u || max_allocation >= kVertexHoldFull;
    return allocation_fits && host_heap >= kVertexFullHeap ? kVertexHoldFull : kVertexHoldBase;
}

} // namespace mhfu::gpu
