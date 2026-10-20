#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>

namespace mhfu::audio {

inline constexpr std::uint32_t kSampleRate = 44'100u;
inline constexpr std::uint32_t kChannels = 2u;

// The device's output gain in 0..1 for a gain the player chose: none while
// the window is in the background with background muting on.
[[nodiscard]] float output_gain(float gain, bool background_mute, bool focused);
// The gain the volume and mute settings choose.
[[nodiscard]] float settings_gain(std::uint32_t volume, bool mute);

// Host playback for the guest's 44100 Hz stereo stream.
//
// The guest and the audio device run on unrelated clocks: the kernel's virtual
// clock advances whenever threads block, the device consumes exactly 44100
// frames of real time per second. The sink absorbs that mismatch in a circular
// buffer of monotonic frame indices. Producers mix at their own cursor, so
// several sceAudio channels overlap the way the hardware mixer overlaps them,
// and the device retires frames from the front. Frames that were never written
// play as silence; frames the device could not keep up with are retired
// without being played. Both are counted rather than hidden, because the
// difference between the two tells us whether emulation is running slow or
// fast.
class AudioSink {
public:
    static AudioSink &instance();

    // Opens the device. Safe to call more than once; honours MHFU_NO_AUDIO.
    void initialize();
    void shutdown();

    // Mixes `frames` stereo frames at `cursor`, a monotonic frame index, with
    // volumes in 0..0x8000, and advances the cursor past them. A cursor that
    // has fallen behind the device is snapped forward to the write target.
    void mix(std::uint64_t &cursor, const std::int16_t *frames, std::size_t count, std::uint32_t left_volume,
        std::uint32_t right_volume);

    // Output gain in 0..1, from the volume and mute settings. Only the device
    // output is scaled; MHFU_AUDIO_DUMP keeps the game's own levels. While the
    // window is in the background, background muting still applies to it.
    void set_volume(float gain);
    // set_volume() with the gain the current settings choose.
    void apply_settings();
    // Whether the window has focus, for background muting. Cheap when it is
    // unchanged, so the renderer reports it on every event pump.
    void set_window_focused(bool focused);
    // The gain the device is set to.
    [[nodiscard]] float output_gain() const;
    // Stops and restarts the device, for the in-game menu's pause. What the
    // game queued before the pause stays in the ring and plays on resume.
    void set_paused(bool paused);
    // Whether a playback device is open (not with MHFU_NO_AUDIO, or when
    // none could be opened).
    [[nodiscard]] bool has_device() const;

    // Public only so the device callback, which lives outside the class, can
    // name it.
    struct Impl;

private:
    AudioSink();
    ~AudioSink();
    std::unique_ptr<Impl> impl_;
};

} // namespace mhfu::audio
