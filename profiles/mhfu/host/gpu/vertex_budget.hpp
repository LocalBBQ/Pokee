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

} // namespace mhfu::gpu
