// Which cached vertex spans are dropped when the cache is over its cap, and
// how the memory of dropped spans is kept for later misses. No GPU is used.

#include "gpu/vertex_cache.hpp"

#include <cstdint>
#include <iostream>
#include <vector>

namespace {

using mhfu::gpu::HoldMiss;
using mhfu::gpu::miss_goes_to_hold;
using mhfu::gpu::SpareVectors;
using mhfu::gpu::vertex_cache_serves;
using mhfu::gpu::vertex_spans_to_drop;

int failures = 0;

void check(bool condition, const char *message) {
    if (!condition) {
        ++failures;
        std::cerr << "FAIL: " << message << '\n';
    }
}

bool dropped(const std::vector<std::size_t> &drop, std::size_t index) {
    for (const std::size_t item : drop)
        if (item == index) return true;
    return false;
}

void under_the_cap_drops_nothing() {
    const std::uint64_t frames[] = {1u, 2u};
    const std::uint64_t bytes[] = {40u, 40u};
    const std::vector<std::size_t> drop = vertex_spans_to_drop(frames, bytes, 2u, 80u, 80u, 1u);
    check(drop.empty(), "a cache that fits drops nothing");
}

void an_older_frame_goes_first() {
    const std::uint64_t frames[] = {1u, 4u, 4u};
    const std::uint64_t bytes[] = {50u, 40u, 40u};
    const std::vector<std::size_t> drop = vertex_spans_to_drop(frames, bytes, 3u, 130u, 100u, 2u);
    check(drop.size() == 1u && drop[0] == 0u, "the span from the older frame is the one dropped");
    check(!dropped(drop, 2u), "the span just stored stays");
}

void the_same_frame_keeps_the_new_span() {
    const std::uint64_t frames[] = {3u, 3u, 3u};
    const std::uint64_t bytes[] = {60u, 60u, 60u};
    const std::vector<std::size_t> drop = vertex_spans_to_drop(frames, bytes, 3u, 180u, 100u, 2u);
    check(dropped(drop, 0u) && dropped(drop, 1u), "earlier spans of this frame make room");
    check(!dropped(drop, 2u), "the span just stored is kept while others can go");
}

void a_span_larger_than_the_cap_is_dropped() {
    const std::uint64_t frames[] = {1u, 2u};
    const std::uint64_t bytes[] = {10u, 500u};
    const std::vector<std::size_t> drop = vertex_spans_to_drop(frames, bytes, 2u, 510u, 100u, 1u);
    check(dropped(drop, 1u), "a span larger than the cap is not kept");
    check(!dropped(drop, 0u), "dropping the oversized span leaves the one that fits");
}

void a_dropped_vector_is_reused() {
    SpareVectors<std::uint32_t> spares;
    spares.set_limit(1u << 20u);
    std::vector<std::uint32_t> dropped_span(1000u, 7u);
    const std::uint32_t *memory = dropped_span.data();
    spares.give(std::move(dropped_span));
    check(spares.count() == 1u && spares.bytes() == 4000u, "a dropped span's memory is kept");

    std::vector<std::uint32_t> next;
    spares.reserve(next, 900u);
    check(next.data() == memory && next.empty() && next.capacity() >= 900u,
        "the next miss of a similar size gets that memory, emptied");
    check(spares.count() == 0u && spares.bytes() == 0u, "a spare is taken once");
}

void a_far_larger_spare_is_left() {
    SpareVectors<std::uint32_t> spares;
    spares.set_limit(1u << 20u);
    spares.give(std::vector<std::uint32_t>(10000u));
    std::vector<std::uint32_t> small;
    spares.reserve(small, 100u);
    check(spares.count() == 1u, "a spare more than twice the size needed stays a spare");
    check(small.capacity() >= 100u, "the small span gets memory of its own");
}

void the_spares_keep_to_their_limit() {
    SpareVectors<std::uint32_t> spares;
    spares.set_limit(6000u);
    spares.give(std::vector<std::uint32_t>(1000u));
    spares.give(std::vector<std::uint32_t>(1000u));
    check(spares.count() == 1u && spares.bytes() == 4000u, "a spare past the limit is freed instead");
    spares.give(std::vector<std::uint32_t>{});
    check(spares.count() == 1u, "a vector without memory is not kept");

    SpareVectors<std::uint32_t> off;
    off.give(std::vector<std::uint32_t>(10u));
    check(off.count() == 0u, "with no limit nothing is kept, as with MHFU_OLD_VERTEX_CACHE");
}

void a_smaller_buffer_becomes_a_spare() {
    SpareVectors<std::uint32_t> spares;
    spares.set_limit(1u << 20u);
    spares.give(std::vector<std::uint32_t>(500u));
    std::vector<std::uint32_t> build(100u, 1u);
    spares.reserve(build, 400u);
    check(build.empty() && build.capacity() >= 400u, "the build buffer takes the larger spare");
    check(spares.count() == 1u && spares.bytes() == 400u, "its own smaller memory is kept in exchange");
    std::vector<std::uint32_t> roomy(0u);
    roomy.reserve(50u);
    roomy.push_back(3u);
    spares.reserve(roomy, 20u);
    check(roomy.empty() && roomy.capacity() >= 50u && spares.count() == 1u,
        "a buffer with room already is emptied and keeps its memory");
}

// A scenery draw: direct, decoded, unweighted, waiting to be cached.
HoldMiss scenery() {
    HoldMiss miss;
    miss.enabled = true;
    miss.hold_ready = true;
    miss.pending = true;
    miss.direct = true;
    miss.vertex_type = 0x0000011Fu; // bits 9 and 10, the weight format, clear
    miss.count = 300u;
    return miss;
}

void a_scenery_miss_goes_to_the_hold() {
    check(miss_goes_to_hold(scenery()), "a direct, unweighted miss is written straight into the hold");
}

void what_keeps_a_miss_out_of_the_hold() {
    HoldMiss miss = scenery();
    miss.enabled = false;
    check(!miss_goes_to_hold(miss), "MHFU_NO_MISS_TO_HOLD keeps the copies");
    miss = scenery();
    miss.hold_ready = false;
    check(!miss_goes_to_hold(miss), "no hold, no writing into it");
    miss = scenery();
    miss.pending = false;
    check(!miss_goes_to_hold(miss), "a draw the cache does not store keeps the ring");
    miss = scenery();
    miss.direct = false;
    check(!miss_goes_to_hold(miss), "an expanded draw keeps its own copy");
    miss = scenery();
    miss.raw = true;
    check(!miss_goes_to_hold(miss), "an undecoded draw has nothing to convert");
    miss = scenery();
    miss.skinned = true;
    check(!miss_goes_to_hold(miss), "a skinned draw is copied for interpolation");
    miss = scenery();
    miss.count = 0u;
    check(!miss_goes_to_hold(miss), "an empty draw is not stored");
}

void a_weighted_miss_keeps_its_copy() {
    for (std::uint32_t weights = 1u; weights <= 3u; ++weights) {
        HoldMiss miss = scenery();
        miss.vertex_type |= weights << 9u;
        check(!miss_goes_to_hold(miss), "every weight format keeps a copy on the CPU");
    }
    HoldMiss neighbours = scenery();
    neighbours.vertex_type |= (1u << 8u) | (1u << 11u);
    check(miss_goes_to_hold(neighbours), "the bits beside the weight field do not count as weights");
}

// A capture frame (MHFU_TRACE_CAPTURE) expands every draw. A span stored or
// reused there was drawn in the wrong layout from then on, and the level
// came apart in torn triangles after each screenshot.
void a_trace_frame_keeps_out_of_the_cache() {
    check(vertex_cache_serves(false, false), "an ordinary frame uses the cache");
    check(!vertex_cache_serves(false, true), "a capture frame neither reads nor stores spans");
    check(!vertex_cache_serves(true, false), "a run with the cache off never uses it");
    check(!vertex_cache_serves(true, true), "a capture frame of such a run does not use it either");
}

} // namespace

int main() {
    under_the_cap_drops_nothing();
    an_older_frame_goes_first();
    the_same_frame_keeps_the_new_span();
    a_span_larger_than_the_cap_is_dropped();
    a_dropped_vector_is_reused();
    a_far_larger_spare_is_left();
    the_spares_keep_to_their_limit();
    a_smaller_buffer_becomes_a_spare();
    a_scenery_miss_goes_to_the_hold();
    what_keeps_a_miss_out_of_the_hold();
    a_weighted_miss_keeps_its_copy();
    a_trace_frame_keeps_out_of_the_cache();
    if (failures != 0) {
        std::cerr << failures << " check(s) failed\n";
        return 1;
    }
    std::cout << "vertex cache tests passed\n";
    return 0;
}
