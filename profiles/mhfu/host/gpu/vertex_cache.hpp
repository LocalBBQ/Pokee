#pragma once

#include <algorithm>
#include <cstdint>
#include <vector>

namespace mhfu::gpu {

// Indices of spans to erase so a cache of `used` bytes fits in `cap`.
// `frames[i]` is the frame span i was last drawn; an older frame goes first.
// Spans from the same frame go in the order given. `protect` is the span just
// stored and is erased only when it is larger than the cap by itself: keeping
// it would push every other span out on the next insert.
[[nodiscard]] inline std::vector<std::size_t> vertex_spans_to_drop(
    const std::uint64_t *frames, const std::uint64_t *bytes, std::size_t count, std::uint64_t used,
    std::uint64_t cap, std::size_t protect) {
    std::vector<std::size_t> drop;
    if (count == 0u || used <= cap || frames == nullptr || bytes == nullptr) return drop;
    // A span bigger than the whole cache can never be kept. Drop it before
    // anything else, so it does not push out spans that do fit.
    if (protect < count && bytes[protect] > cap) {
        const std::uint64_t size = bytes[protect];
        used = size > used ? 0u : used - size;
        drop.push_back(protect);
        if (used <= cap) return drop;
    }
    std::vector<std::size_t> order;
    order.reserve(count);
    for (std::size_t i = 0u; i < count; ++i)
        if (i != protect) order.push_back(i);
    std::sort(order.begin(), order.end(), [&](std::size_t a, std::size_t b) {
        if (frames[a] != frames[b]) return frames[a] < frames[b];
        return a < b;
    });
    for (const std::size_t index : order) {
        if (used <= cap) break;
        const std::uint64_t size = bytes[index];
        used = size > used ? 0u : used - size;
        drop.push_back(index);
    }
    return drop;
}

} // namespace mhfu::gpu
