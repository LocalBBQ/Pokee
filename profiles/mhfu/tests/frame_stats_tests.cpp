// The stall trace (MHFU_TRACE_STALLS): every kind has a name of its own, and
// the time a StallScope measures reaches the [slow-frame] line of its frame,
// and so does the frame's render split. The performance setting's overlay
// choices, the compact overlay's line, and the audio gain with background
// muting.
// No game data and no GPU.
#include "audio/audio_sink.hpp"
#include "perf/frame_stats.hpp"
#include "perf/perf_overlay.hpp"
#include "settings/settings.hpp"

#include <algorithm>
#include <cstdint>

#include <cstdlib>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

namespace {
using namespace mhfu::perf;

int failures{};

void check(bool condition, const std::string &message) {
    if (!condition) {
        ++failures;
        std::cerr << "FAIL: " << message << '\n';
    }
}

void set_environment(const char *name, const char *value) {
#if defined(_WIN32)
    _putenv_s(name, value);
#else
    setenv(name, value, 1);
#endif
}

void test_every_kind_has_its_own_name() {
    std::set<std::string> names;
    for (std::size_t i = 0; i < static_cast<std::size_t>(Stall::Count); ++i) {
        const std::string name = stall_name(static_cast<Stall>(i));
        check(name != "?", "stall kind " + std::to_string(i) + " has a name");
        check(names.insert(name).second, "stall name " + name + " is used once");
    }
    check(std::string(stall_name(Stall::Count)) == "?", "Count is not a kind");
    check(std::string(stall_name(Stall::Texture)) == "texture", "texture creation is named texture");
    check(std::string(stall_name(Stall::Allocate)) == "alloc", "memory allocation is named alloc");
    check(std::string(stall_name(Stall::Cache)) == "cache-save", "the pipeline cache save is named cache-save");
    check(std::string(stall_name(Stall::Pack)) == "pack", "the texture pack's work is named pack");
}

std::string slow_frame_line(std::uint64_t virtual_us) {
    std::ostringstream captured;
    std::streambuf *const previous = std::cout.rdbuf(captured.rdbuf());
    end_frame(virtual_us);
    std::cout.rdbuf(previous);
    return captured.str();
}

void test_scopes_reach_the_slow_frame_line() {
    // Starts the frame the stalls below belong to.
    (void)slow_frame_line(0u);
    {
        const StallScope texture(Stall::Texture);
        const StallScope allocate(Stall::Allocate);
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
    {
        const StallScope allocate(Stall::Allocate);
    }
    note_stall(Stall::Cache, std::chrono::milliseconds(3));
    const std::string line = slow_frame_line(33'367u);
    check(line.rfind("[slow-frame] ", 0) == 0u, "a frame over the threshold is reported: " + line);
    check(line.find(" texture ") != std::string::npos && line.find(" texture 0.00 ") == std::string::npos,
        "the texture scope's time is reported: " + line);
    check(line.find(" alloc ") != std::string::npos && line.find(" x2") != std::string::npos,
        "both allocation scopes are counted: " + line);
    check(line.find(" cache-save 3.00 x1") != std::string::npos, "the cache save is reported: " + line);
    check(line.find(" pipeline ") == std::string::npos, "a kind with no stall is left out: " + line);

    // The next frame starts empty.
    const std::string next = slow_frame_line(66'734u);
    check(next.find(" texture ") == std::string::npos && next.find(" none") != std::string::npos,
        "stalls do not carry over to the next frame: " + next);
}

void test_split_reaches_the_slow_frame_line() {
    check(split_enabled(), "the stall trace times the render split");
    // The counter is calibrated over the first quarter second of frames.
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    (void)slow_frame_line(100'101u);
    {
        const SplitScope lists(Split::Lists);
        const SplitScope draw(Split::Draw);
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    const std::string line = slow_frame_line(133'468u);
    const std::size_t at = line.find(" | lists ");
    check(at != std::string::npos, "the frame's render split is on its slow-frame line: " + line);
    if (at != std::string::npos)
        check(std::strtod(line.c_str() + at + 9u, nullptr) >= 4.0, "the split gives the scope's time: " + line);
    check(line.find(" draw ") != std::string::npos && line.find(" vertices ") != std::string::npos &&
            line.find(" pipeline ") != std::string::npos && line.find(" other ") != std::string::npos &&
            line.find(" writeback ") != std::string::npos,
        "every part of the split is named: " + line);

    const std::string next = slow_frame_line(166'835u);
    check(next.find(" | lists 0.0 ") != std::string::npos, "the split does not carry over to the next frame: " + next);
}

void test_draws_reach_the_slow_frame_line() {
    (void)slow_frame_line(200'202u);
    for (int i = 0; i < 3; ++i) count_draw();
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    const std::string line = slow_frame_line(233'569u);
    check(line.find(" | draws 3 | ") != std::string::npos, "the frame's draws are on its slow-frame line: " + line);
    check(line.find(" at 233.6ms | ") != std::string::npos, "so is the emulated time it ended at: " + line);
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    const std::string next = slow_frame_line(266'936u);
    check(next.find(" | draws 0 | ") != std::string::npos, "draws do not carry over to the next frame: " + next);
}

void test_charged_time_reaches_the_slow_frame_line() {
    (void)slow_frame_line(300'303u);
    const std::string none = slow_frame_line(333'670u);
    check(none.find(" | charged 0.0 ms | ") != std::string::npos, "a frame without charges says 0: " + none);
    add_charged_time(1000u);
    add_charged_time(1000u);
    add_charged_time(500u);
    const std::string line = slow_frame_line(367'037u);
    check(line.find(" | charged 2.5 ms | ") != std::string::npos,
        "the emulated time charged in the frame is on its slow-frame line: " + line);
    const std::string next = slow_frame_line(400'404u);
    check(next.find(" | charged 0.0 ms | ") != std::string::npos, "charges do not carry over to the next frame: " + next);
}

void test_slowest_call_reaches_the_slow_frame_line() {
    check(stall_trace_enabled(), "MHFU_TRACE_STALLS turns the call timing on");
    (void)slow_frame_line(433'771u);
    const std::string none = slow_frame_line(467'138u);
    check(none.find(" | slowest call none 0.0 ms | thread cpu ") != std::string::npos,
        "a frame without calls names none: " + none);
    note_hle_call("sceAudio::sceAudioOutputPannedBlocking", std::chrono::microseconds(2'000));
    note_hle_call("sceDisplay::sceDisplaySetFrameBuf", std::chrono::microseconds(7'500));
    note_hle_call("sceCtrl::sceCtrlReadBufferPositive", std::chrono::microseconds(500));
    const std::string line = slow_frame_line(500'505u);
    check(line.find(" | slowest call sceDisplay::sceDisplaySetFrameBuf 7.5 ms | thread cpu ") != std::string::npos,
        "the frame's longest call is on its slow-frame line: " + line);
    const std::string next = slow_frame_line(533'872u);
    check(next.find(" | slowest call none 0.0 ms | ") != std::string::npos,
        "the slowest call does not carry over to the next frame: " + next);
}

#if defined(__linux__)
// "thread cpu 12.3 ms" as a number.
double thread_cpu_ms(const std::string &line) {
    const std::size_t at = line.find(" thread cpu ");
    return at == std::string::npos ? -1.0 : std::strtod(line.c_str() + at + 12u, nullptr);
}

void test_thread_usage_tells_work_from_a_stop() {
    (void)slow_frame_line(600'606u);
    std::this_thread::sleep_for(std::chrono::milliseconds(30));
    const std::string asleep = slow_frame_line(633'973u);
    check(thread_cpu_ms(asleep) >= 0.0 && thread_cpu_ms(asleep) < 15.0,
        "a frame the thread slept through takes little of its CPU: " + asleep);
    const auto until = std::chrono::steady_clock::now() + std::chrono::milliseconds(30);
    volatile std::uint64_t spin = 0;
    while (std::chrono::steady_clock::now() < until) spin = spin + 1;
    const std::string busy = slow_frame_line(667'340u);
    check(thread_cpu_ms(busy) >= 15.0, "a frame it worked through takes most of it: " + busy);
}
#endif

void test_accounted_time_covers_render_and_pacing() {
    const Clock::duration before = accounted_time();
    add_render_time(std::chrono::microseconds(300));
    add_pacing_time(std::chrono::microseconds(200));
    check(accounted_time() - before == std::chrono::microseconds(500),
        "render and pacing time are what a call's own time leaves out");
    (void)slow_frame_line(567'239u);
    check(accounted_time() - before == std::chrono::microseconds(500), "and the total is not reset with the frame");
}

void test_performance_setting_options() {
    using mhfu::settings::PerfDisplay;
    mhfu::settings::Settings &s = mhfu::settings::current();
    const PerfDisplay saved = s.perf;
    const auto is = [](bool log, bool overlay, bool compact) {
        const Options o = options();
        return o.log == log && o.overlay == overlay && o.compact == compact;
    };
    s.perf = PerfDisplay::Off;
    check(is(false, false, false), "Off shows nothing");
    s.perf = PerfDisplay::Overlay;
    check(is(false, true, false), "Overlay is the full overlay");
    s.perf = PerfDisplay::OverlayAndLog;
    check(is(true, true, false), "Overlay and log");
    s.perf = PerfDisplay::Log;
    check(is(true, false, false), "Log only");
    s.perf = PerfDisplay::Compact;
    check(is(false, true, true), "Compact is the overlay in its one-line form");
    s.perf = PerfDisplay::CompactAndLog;
    check(is(true, true, true), "Compact and log");
    s.perf = saved;
    check(overlay_rows(true) == kCompactOverlayHeight && overlay_rows(false) == kOverlayHeight,
        "each overlay fills its own rows");
    check(kCompactOverlayHeight < kOverlayHeight, "the compact overlay fits the full overlay's image");
}

constexpr std::uint32_t rgb(std::uint32_t r, std::uint32_t g, std::uint32_t b) {
    return r | (g << 8u) | (b << 16u) | 0xFF000000u;
}

// The compact line's pixels for `summary`, drawn into a full-size image
// filled with `kSentinel` first, so rows it must leave alone can be checked.
constexpr std::uint32_t kSentinel = 0x12345678u;
std::vector<std::uint32_t> compact_pixels(const Summary &summary) {
    std::vector<std::uint32_t> pixels(static_cast<std::size_t>(kOverlayWidth) * kOverlayHeight, kSentinel);
    draw_compact_overlay(pixels.data(), summary);
    return pixels;
}

bool has(const std::vector<std::uint32_t> &pixels, std::uint32_t color) {
    return std::find(pixels.begin(), pixels.begin() + kOverlayWidth * kCompactOverlayHeight, color) !=
        pixels.begin() + kOverlayWidth * kCompactOverlayHeight;
}

void test_compact_overlay() {
    const std::uint32_t text = rgb(235u, 235u, 235u);
    const std::uint32_t good = rgb(80u, 210u, 90u);
    const std::uint32_t slow = rgb(235u, 200u, 60u);
    const std::uint32_t bad = rgb(235u, 70u, 60u);

    const std::vector<std::uint32_t> measuring = compact_pixels(Summary{});
    check(has(measuring, text) && !has(measuring, good), "before the first second the line is dashes, not a speed");
    bool kept = true;
    for (std::size_t i = kOverlayWidth * kCompactOverlayHeight; i < measuring.size(); ++i)
        kept = kept && measuring[i] == kSentinel;
    check(kept, "the compact line writes only its own rows");
    bool filled = true;
    for (std::size_t i = 0; i < kOverlayWidth * kCompactOverlayHeight; ++i) filled = filled && measuring[i] != kSentinel;
    check(filled, "and every pixel of them");

    Summary summary{};
    summary.valid = true;
    summary.fps = 60.0;
    summary.game_fps = 30.0;
    summary.speed = 1.0;
    const std::vector<std::uint32_t> full = compact_pixels(summary);
    check(has(full, good) && !has(full, slow) && !has(full, bad) && !has(full, text), "full speed is green");
    summary.speed = 0.99;
    check(has(compact_pixels(summary), good), "99% still counts as full speed");
    summary.speed = 0.95;
    const std::vector<std::uint32_t> behind = compact_pixels(summary);
    check(has(behind, slow) && !has(behind, good), "95% is yellow");
    summary.speed = 0.5;
    const std::vector<std::uint32_t> slow_motion = compact_pixels(summary);
    check(has(slow_motion, bad) && !has(slow_motion, slow), "half speed is red");
    summary.speed = 1.0;
    summary.game_fps = 20.0;
    check(compact_pixels(summary) != full, "the game's own frame rate is on the line");
}

void test_background_mute_gain() {
    using mhfu::audio::output_gain;
    using mhfu::audio::settings_gain;
    check(settings_gain(100u, false) == 1.0f && settings_gain(50u, false) == 0.5f, "the volume is a fraction");
    check(settings_gain(80u, true) == 0.0f, "mute is silence at any volume");
    check(output_gain(0.7f, true, true) == 0.7f, "with focus, background muting changes nothing");
    check(output_gain(0.7f, false, false) == 0.7f, "without background muting, the background is heard");
    check(output_gain(0.7f, true, false) == 0.0f, "background muting silences an unfocused window");
    check(output_gain(2.0f, false, true) == 1.0f && output_gain(-1.0f, false, true) == 0.0f,
        "the gain stays within 0..1");
}

void test_background_mute_follows_focus() {
    mhfu::settings::Settings &s = mhfu::settings::current();
    const bool saved = s.background_mute;
    auto &sink = mhfu::audio::AudioSink::instance();
    s.volume = 60u;
    s.mute = false;
    s.background_mute = true;
    sink.set_window_focused(true);
    sink.apply_settings();
    check(sink.output_gain() == 0.6f, "the volume applies with focus");
    sink.set_window_focused(false);
    check(sink.output_gain() == 0.0f, "losing focus silences the game");
    s.volume = 90u;
    sink.apply_settings();
    check(sink.output_gain() == 0.0f, "a volume change in the background does not unmute it");
    sink.set_window_focused(true);
    check(sink.output_gain() == 0.9f, "focus brings back the newest volume");
    s.background_mute = false;
    sink.set_window_focused(false);
    sink.apply_settings();
    check(sink.output_gain() == 0.9f, "with the setting off the background is heard");
    sink.set_window_focused(true);
    s.background_mute = saved;
    s.volume = 100u;
    sink.apply_settings();
}

} // namespace

int main() {
    // Read once, at the first use of the trace: every frame counts as slow.
    set_environment("MHFU_TRACE_STALLS", "1");
    set_environment("MHFU_TRACE_STALLS_MS", "0.001");
    test_every_kind_has_its_own_name();
    test_scopes_reach_the_slow_frame_line();
    test_split_reaches_the_slow_frame_line();
    test_draws_reach_the_slow_frame_line();
    test_charged_time_reaches_the_slow_frame_line();
    test_slowest_call_reaches_the_slow_frame_line();
    test_accounted_time_covers_render_and_pacing();
#if defined(__linux__)
    test_thread_usage_tells_work_from_a_stop();
#endif
    test_performance_setting_options();
    test_compact_overlay();
    test_background_mute_gain();
    // The sink is never opened here (no initialize()), so only its gain moves.
    test_background_mute_follows_focus();
    if (failures != 0) {
        std::cerr << failures << " failure(s)\n";
        return 1;
    }
    std::cout << "frame stats tests passed\n";
    return 0;
}
