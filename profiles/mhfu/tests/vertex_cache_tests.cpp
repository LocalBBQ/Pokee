// Which cached vertex spans are dropped when the cache is over its cap.
// No GPU is used.

#include "gpu/vertex_cache.hpp"

#include <cstdint>
#include <iostream>
#include <vector>

namespace {

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

} // namespace

int main() {
    under_the_cap_drops_nothing();
    an_older_frame_goes_first();
    the_same_frame_keeps_the_new_span();
    a_span_larger_than_the_cap_is_dropped();
    if (failures != 0) {
        std::cerr << failures << " check(s) failed\n";
        return 1;
    }
    std::cout << "vertex cache tests passed\n";
    return 0;
}
