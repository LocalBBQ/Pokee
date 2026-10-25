#include "audio/channel_queue.hpp"

namespace mhfu::audio {

std::uint64_t queue_output(
    ChannelQueue &queue, std::uint64_t now_us, std::uint32_t frames, std::uint32_t sample_rate, bool wait_for_drain) {
    const std::uint64_t buffer_us = static_cast<std::uint64_t>(frames) * 1'000'000u / sample_rate;
    if (queue.queued_until_us < now_us) queue.queued_until_us = now_us;
    const std::uint64_t previous_end = queue.queued_until_us;
    // A drain ends once the longer buffers have played, or with a buffer
    // longer than its own.
    if (queue.drain_end_us != 0u && (queue.drain_end_us <= now_us || frames > queue.drain_frames)) {
        queue.drain_end_us = 0u;
        queue.drain_frames = 0u;
    }
    // A shorter buffer than the last, behind more than it lasts itself.
    if (!wait_for_drain && queue.drain_end_us == 0u && frames < queue.last_frames &&
        previous_end > now_us + buffer_us) {
        queue.drain_end_us = previous_end;
        queue.drain_frames = frames;
    }
    queue.queued_until_us += buffer_us;
    queue.last_frames = frames;
    // While draining, only what was queued after the longer buffers counts.
    const std::uint64_t waits_until =
        queue.drain_end_us != 0u ? now_us + (previous_end - queue.drain_end_us) : previous_end;
    return waits_until > now_us ? waits_until - now_us : 0u;
}

} // namespace mhfu::audio
