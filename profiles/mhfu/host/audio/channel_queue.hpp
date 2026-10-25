#pragma once

#include <cstdint>

namespace mhfu::audio {

// The emulated-time queue of one sceAudio channel: when what it was handed
// has finished playing, and so how long a blocking output waits. A blocking
// output returns once the buffers queued before it have played.
//
// A game drains a channel by switching it to short buffers and outputting
// them behind the longer ones still queued. MHFU's main thread does this on
// the music channel at each change of music segment, a few times a fight, and
// waiting there for the rest of a music buffer (up to about 46 ms) costs it a
// vblank. Short buffers queued behind longer ones wait only for the short
// ones before them; the next longer buffer waits for everything. The queue,
// and so what reaches the host sink and when, stays the same: only the
// thread that waits changes.
struct ChannelQueue {
    std::uint64_t queued_until_us{};
    std::uint32_t last_frames{};
    // While draining: where the longer buffers end, and the drain's size.
    std::uint64_t drain_end_us{};
    std::uint32_t drain_frames{};
};

// Queues an output of `frames` at `now_us` and returns how long it blocks, in
// microseconds. `wait_for_drain` (MHFU_OLD_AUDIO_DRAIN) makes every output
// wait for everything queued before it, short buffers included.
[[nodiscard]] std::uint64_t queue_output(
    ChannelQueue &queue, std::uint64_t now_us, std::uint32_t frames, std::uint32_t sample_rate, bool wait_for_drain);

} // namespace mhfu::audio
