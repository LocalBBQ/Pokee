#pragma once

#include <algorithm>
#include <cstdint>
#include <vector>

namespace mhfu::gpu {

// How many frames a freed span stays unread. The frame that drew it, the next
// frame blended with it, and the presents of that pair can still read the
// bytes. MHFU_NO_VERTEX_HOLD_RECLAIM never frees a span, so later zones append
// until the hold is full.
inline constexpr std::uint64_t kVertexHoldRetireFrames = 3u;
inline constexpr std::uint64_t kVertexHoldAlign = 16u;

// Placement of decoded vertices in the hold. A span that stops being drawn
// becomes a hole and is reused once `ready` has been reached, so a later zone
// does not stack on top of one that has left the screen.
class VertexHoldHeap {
  public:
    static constexpr std::uint64_t kNone = ~std::uint64_t{0};

    struct Placement {
        std::uint64_t offset = kNone;
        std::uint64_t reserved = 0; // bytes occupied, a multiple of the alignment
    };

    void reset(std::uint64_t capacity) {
        capacity_ = capacity;
        used_ = 0u;
        holes_.clear();
    }

    [[nodiscard]] std::uint64_t used() const { return used_; }
    [[nodiscard]] std::uint64_t capacity() const { return capacity_; }

    // `frame` is the frame being drawn. A hole is usable when its ready frame
    // has been reached.
    [[nodiscard]] Placement place(std::uint64_t bytes, std::uint64_t frame) {
        if (bytes == 0u || capacity_ == 0u) return {};
        const std::uint64_t reserved = align_up(bytes);
        reclaim_tail(frame);
        for (auto it = holes_.begin(); it != holes_.end(); ++it) {
            if (it->ready > frame || it->bytes < reserved) continue;
            const Placement placed{it->offset, reserved};
            const std::uint64_t leftover = it->bytes - reserved;
            if (leftover == 0u) {
                holes_.erase(it);
            } else {
                it->offset += reserved;
                it->bytes = leftover;
            }
            return placed;
        }
        const std::uint64_t at = align_up(used_);
        if (at + reserved > capacity_) return {};
        used_ = at + reserved;
        return {at, reserved};
    }

    // The range may be placed again once `frame` reaches `ready_frame`. Returns
    // how far the append cursor moved back.
    std::uint64_t release(
        std::uint64_t offset, std::uint64_t reserved, std::uint64_t ready_frame, std::uint64_t frame) {
        if (reserved == 0u || offset == kNone) return 0u;
        insert(Hole{offset, reserved, ready_frame});
        return reclaim_tail(frame);
    }

  private:
    struct Hole {
        std::uint64_t offset;
        std::uint64_t bytes;
        std::uint64_t ready;
    };

    std::uint64_t capacity_{};
    std::uint64_t used_{};
    std::vector<Hole> holes_;

    [[nodiscard]] static std::uint64_t align_up(std::uint64_t value) {
        return (value + kVertexHoldAlign - 1u) & ~(kVertexHoldAlign - 1u);
    }

    void insert(Hole hole) {
        auto it = holes_.begin();
        while (it != holes_.end() && it->offset < hole.offset) ++it;
        it = holes_.insert(it, hole);
        if (auto next = it + 1; next != holes_.end() && it->offset + it->bytes == next->offset) {
            it->bytes += next->bytes;
            it->ready = std::max(it->ready, next->ready);
            holes_.erase(next);
        }
        if (it != holes_.begin()) {
            auto prev = it - 1;
            if (prev->offset + prev->bytes == it->offset) {
                prev->bytes += it->bytes;
                prev->ready = std::max(prev->ready, it->ready);
                holes_.erase(it);
            }
        }
    }

    std::uint64_t reclaim_tail(std::uint64_t frame) {
        const std::uint64_t before = used_;
        bool pulled = true;
        while (pulled) {
            pulled = false;
            for (auto it = holes_.begin(); it != holes_.end(); ++it) {
                if (it->ready <= frame && it->offset + it->bytes == used_) {
                    used_ = it->offset;
                    holes_.erase(it);
                    pulled = true;
                    break;
                }
            }
        }
        return before - used_;
    }
};

} // namespace mhfu::gpu
