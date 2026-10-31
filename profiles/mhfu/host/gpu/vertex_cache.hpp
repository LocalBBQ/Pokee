#pragma once

#include <algorithm>
#include <cstdint>
#include <map>
#include <utility>
#include <vector>

namespace mhfu::gpu {

// Indices of spans to erase so a cache of `used` bytes fits in `cap`.
// `frames[i]` is the frame span i was last drawn; an older frame goes first.
// Spans from the same frame go in the order given. `protect` is the span just
// stored and is erased only when it is larger than the cap by itself: keeping
// it would push every other span out on the next insert.
[[nodiscard]] inline std::vector<std::size_t> vertex_spans_to_drop(const std::uint64_t *frames,
    const std::uint64_t *bytes, std::size_t count, std::uint64_t used, std::uint64_t cap, std::size_t protect) {
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

// A draw the vertex cache missed, as the renderer sees it.
struct HoldMiss {
    bool enabled{};    // not MHFU_NO_MISS_TO_HOLD or MHFU_OLD_VERTEX_CACHE
    bool hold_ready{}; // the hold buffer exists and is mapped
    bool pending{};    // the cache is waiting to store this draw's span
    bool direct{};     // vertices written once, with an index list
    bool raw{};        // handed to the GPU undecoded
    bool skinned{};    // copied each frame for frame interpolation
    std::uint32_t vertex_type{};
    std::size_t count{};
};

// Whether that draw is converted straight into the hold and drawn from there,
// keeping no copy of its own. Only a direct, decoded draw without bone
// weights: a skinned one needs its vertices on the CPU for interpolation.
[[nodiscard]] constexpr bool miss_goes_to_hold(const HoldMiss &miss) {
    const bool weighted = ((miss.vertex_type >> 9u) & 3u) != 0u;
    return miss.enabled && miss.hold_ready && miss.pending && miss.direct && !miss.raw && !miss.skinned && !weighted &&
        miss.count != 0u;
}

// Whether a draw may be served from, or stored in, the vertex cache.
// `switched_off` is a run that never uses it (MHFU_NO_VERTEX_CACHE, or a
// trace that prints every draw). `trace_frame` is a frame such a trace covers
// in a run that otherwise uses it, as MHFU_TRACE_CAPTURE does after each
// screenshot. That trace reads each draw's decoded vertices and keeps the
// direct path off. With the cache on, a span stored in that frame was the
// expanded triangle list under the direct layout's label, and every frame
// after drew the level from it as torn triangles; a hit there had no decoded
// vertices to read.
[[nodiscard]] constexpr bool vertex_cache_serves(bool switched_off, bool trace_frame) {
    return !switched_off && !trace_frame;
}

// The vectors of spans the cache dropped, kept with their memory for the
// spans that miss next. Fresh memory is what makes a miss slow: on Windows the
// 55 MiB of new spans an area brings took 12 to 15 ms in allocations alone,
// and freeing them handed the pages back to the system. At most `limit` bytes
// of capacity are kept.
template <class T> class SpareVectors {
public:
    void set_limit(std::uint64_t limit) { limit_ = limit; }
    [[nodiscard]] std::uint64_t bytes() const { return bytes_; }
    [[nodiscard]] std::size_t count() const { return spares_.size(); }

    // Keeps the memory of `vector`, emptied, unless that would pass the limit.
    void give(std::vector<T> &&vector) {
        const std::size_t capacity = vector.capacity();
        const std::uint64_t bytes = static_cast<std::uint64_t>(capacity) * sizeof(T);
        if (capacity == 0u || bytes_ + bytes > limit_) return;
        vector.clear();
        bytes_ += bytes;
        spares_.emplace(capacity, std::move(vector));
    }

    // Empties `into` and gives it room for `count` elements: the smallest
    // spare that fits, unless that is more than twice what is needed, or
    // else memory of its own. A smaller buffer `into` had becomes a spare.
    void reserve(std::vector<T> &into, std::size_t count) {
        into.clear();
        if (into.capacity() >= count) return;
        const auto found = spares_.lower_bound(count);
        if (found != spares_.end() && found->first <= 2u * count + 64u) {
            std::vector<T> taken = std::move(found->second);
            bytes_ -= static_cast<std::uint64_t>(found->first) * sizeof(T);
            spares_.erase(found);
            std::swap(into, taken);
            give(std::move(taken));
            return;
        }
        into.reserve(count);
    }

private:
    std::multimap<std::size_t, std::vector<T>> spares_;
    std::uint64_t bytes_{};
    std::uint64_t limit_{};
};

} // namespace mhfu::gpu
