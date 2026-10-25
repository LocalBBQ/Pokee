// How long a blocking sceAudio output waits: a stream waits for the buffer
// before it, and short buffers draining a channel behind longer ones wait
// only for each other, while the queue itself stays as it was.
// No game data and no audio device.
#include "audio/channel_queue.hpp"

#include <cstdint>
#include <iostream>
#include <string>

namespace {
using mhfu::audio::ChannelQueue;
using mhfu::audio::queue_output;

constexpr std::uint32_t kRate = 44'100u;
constexpr std::uint64_t kMusicUs = 1024u * 1'000'000u / kRate; // 23219
constexpr std::uint64_t kShortUs = 64u * 1'000'000u / kRate;   // 1451

int failures{};

void check(bool condition, const std::string &message) {
    if (!condition) {
        ++failures;
        std::cerr << "FAIL: " << message << '\n';
    }
}

// Two music buffers queued at 0, as a stream that has just been handed its
// second one: 46 ms queued.
ChannelQueue streaming() {
    ChannelQueue queue;
    (void)queue_output(queue, 0u, 1024u, kRate, false);
    (void)queue_output(queue, 0u, 1024u, kRate, false);
    return queue;
}

void test_a_stream_waits_for_the_buffer_before() {
    ChannelQueue queue;
    check(queue_output(queue, 0u, 1024u, kRate, false) == 0u, "the first buffer of an idle channel does not wait");
    check(queue_output(queue, 0u, 1024u, kRate, false) == kMusicUs, "the second waits for the first");
    check(queue_output(queue, kMusicUs, 1024u, kRate, false) == kMusicUs, "and so on, one buffer at a time");
    check(queue.queued_until_us == 3u * kMusicUs, "the queue ends after the three");
}

void test_a_drain_waits_only_for_its_own_buffers() {
    ChannelQueue queue = streaming();
    const std::uint64_t now = 10'000u;
    check(queue_output(queue, now, 64u, kRate, false) == 0u,
        "a short buffer behind the music does not wait for the music");
    check(queue_output(queue, now, 64u, kRate, false) == kShortUs, "the next short buffer waits for the first");
    check(queue.queued_until_us == 2u * kMusicUs + 2u * kShortUs, "both still queue after the music");
    const std::uint64_t later = now + kShortUs;
    check(queue_output(queue, later, 1024u, kRate, false) == 2u * kMusicUs + 2u * kShortUs - later,
        "the music that follows waits for everything queued");
    check(queue.drain_end_us == 0u, "and ends the drain");
}

void test_the_old_drain_waits_for_everything() {
    ChannelQueue queue = streaming();
    const std::uint64_t now = 10'000u;
    check(queue_output(queue, now, 64u, kRate, true) == 2u * kMusicUs - now,
        "MHFU_OLD_AUDIO_DRAIN: a short buffer waits for the music queued before it");
    check(queue.drain_end_us == 0u, "without starting a drain");
}

void test_a_drained_channel_has_nothing_to_skip() {
    ChannelQueue queue = streaming();
    const std::uint64_t now = 2u * kMusicUs + 5'000u;
    check(queue_output(queue, now, 64u, kRate, false) == 0u, "a short buffer after the music has played");
    check(queue.drain_end_us == 0u, "is no drain");
    check(queue_output(queue, now, 64u, kRate, false) == kShortUs, "the next waits for it as in a stream");
}

void test_a_short_last_music_buffer_stays_bounded() {
    // The music thread ends a segment with a shorter buffer: it skips its
    // wait, and the next full buffer waits for all of it.
    ChannelQueue queue = streaming();
    const std::uint64_t short_us = 704u * 1'000'000u / kRate;
    check(queue_output(queue, kMusicUs, 704u, kRate, false) == 0u, "a shorter last buffer behind the music");
    const std::uint64_t waited = queue_output(queue, kMusicUs, 1024u, kRate, false);
    check(waited == kMusicUs + short_us, "the next full buffer waits for the music and the short one");
    check(queue.queued_until_us == 3u * kMusicUs + short_us, "nothing is dropped from the queue");
}

void test_a_drain_ends_when_the_music_has_played() {
    ChannelQueue queue = streaming();
    (void)queue_output(queue, 10'000u, 64u, kRate, false);
    const std::uint64_t after = 2u * kMusicUs + 100u;
    check(queue_output(queue, after, 64u, kRate, false) == kShortUs - 100u,
        "once the music has played, a short buffer waits for the one before it");
    check(queue.drain_end_us == 0u, "the drain is over");
}

} // namespace

int main() {
    test_a_stream_waits_for_the_buffer_before();
    test_a_drain_waits_only_for_its_own_buffers();
    test_the_old_drain_waits_for_everything();
    test_a_drained_channel_has_nothing_to_skip();
    test_a_short_last_music_buffer_stays_bounded();
    test_a_drain_ends_when_the_music_has_played();
    if (failures != 0) {
        std::cerr << failures << " failure(s)\n";
        return 1;
    }
    std::cout << "audio channel queue tests passed\n";
    return 0;
}
