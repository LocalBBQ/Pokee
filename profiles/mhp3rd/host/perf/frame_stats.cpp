#include "frame_stats.hpp"

#include "settings/settings.hpp"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>

namespace mhp3rd::perf {
namespace {

double to_ms(Clock::duration duration) {
    return std::chrono::duration<double, std::milli>(duration).count();
}

struct State {
    // Frame in progress.
    Clock::time_point frame_start{Clock::now()};
    Clock::duration render{};
    Clock::duration wait{};
    Clock::duration pacing{};
    Clock::duration overlay{};
    std::uint32_t lists{};

    // Second in progress.
    Clock::time_point window_start{Clock::now()};
    std::uint64_t window_virtual_us{};
    bool window_has_clock{};
    std::uint32_t frames{};
    double frame_sum_ms{};
    double frame_max_ms{};
    Clock::duration render_sum{};
    Clock::duration wait_sum{};
    Clock::duration pacing_sum{};
    Clock::duration overlay_sum{};
    std::uint32_t list_sum{};

    Summary summary;
    std::string present_mode{"none"};
    std::uint32_t width{};
    std::uint32_t height{};
    float refresh_hz{};
    std::array<float, kHistoryFrames> history{};
    std::size_t cursor{};
};

State &state() {
    static State value;
    return value;
}

void print(const Summary &s) {
    char line[320];
    int length = std::snprintf(line, sizeof(line),
                               "[perf] fps %.1f game %.1f speed %.0f%% | frame avg %.1f max %.1f ms | guest %.1f "
                               "render %.1f wait %.1f ms | lists %.0f/s | %s %ux%u",
                               s.fps, s.game_fps, s.speed * 100.0, s.frame_avg_ms, s.frame_max_ms, s.guest_ms,
                               s.render_ms, s.wait_ms, s.lists, s.present_mode.c_str(), s.width, s.height);
    if (s.refresh_hz > 0.0f && length > 0 && static_cast<std::size_t>(length) < sizeof(line))
        length += std::snprintf(line + length, sizeof(line) - length, " %.0fHz", s.refresh_hz);
    if (s.overlay_ms > 0.0 && length > 0 && static_cast<std::size_t>(length) < sizeof(line))
        std::snprintf(line + length, sizeof(line) - length, " | overlay %.2f ms", s.overlay_ms);
    // Flushed per line: the log is read while the game runs, often through a
    // pipe where stdout would otherwise sit in a block buffer.
    std::cout << line << std::endl;
}

} // namespace

Options options() {
    Options result{};
    switch (settings::current().perf) {
    case settings::PerfDisplay::Off: break;
    case settings::PerfDisplay::Overlay: result.overlay = true; break;
    case settings::PerfDisplay::OverlayAndLog: result.overlay = result.log = true; break;
    case settings::PerfDisplay::Log: result.log = true; break;
    }
    return result;
}

void restart_measurement() {
    State &s = state();
    const Clock::time_point now = Clock::now();
    s.frame_start = now;
    s.render = s.wait = s.pacing = s.overlay = Clock::duration{};
    s.lists = 0u;
    s.window_start = now;
    s.window_has_clock = false;
    s.frames = 0u;
    s.frame_sum_ms = 0.0;
    s.frame_max_ms = 0.0;
    s.render_sum = s.wait_sum = s.pacing_sum = s.overlay_sum = Clock::duration{};
    s.list_sum = 0u;
}

void add_render_time(Clock::duration duration) { state().render += duration; }
void add_wait_time(Clock::duration duration) { state().wait += duration; }
void add_pacing_time(Clock::duration duration) { state().pacing += duration; }
void add_overlay_time(Clock::duration duration) { state().overlay += duration; }
void count_display_list() { ++state().lists; }

void set_display_info(const std::string &present_mode, std::uint32_t width, std::uint32_t height, float refresh_hz) {
    State &s = state();
    s.present_mode = present_mode;
    s.width = width;
    s.height = height;
    s.refresh_hz = refresh_hz;
}

void end_frame(std::uint64_t virtual_us) {
    State &s = state();
    const Clock::time_point now = Clock::now();
    const double frame_ms = to_ms(now - s.frame_start);
    s.frame_start = now;
    s.history[s.cursor] = static_cast<float>(frame_ms);
    s.cursor = (s.cursor + 1u) % kHistoryFrames;

    if (!s.window_has_clock) {
        s.window_virtual_us = virtual_us;
        s.window_has_clock = true;
    }
    ++s.frames;
    s.frame_sum_ms += frame_ms;
    s.frame_max_ms = std::max(s.frame_max_ms, frame_ms);
    s.render_sum += s.render;
    s.wait_sum += s.wait;
    s.pacing_sum += s.pacing;
    s.overlay_sum += s.overlay;
    s.list_sum += s.lists;
    s.render = s.wait = s.pacing = s.overlay = Clock::duration{};
    s.lists = 0u;

    const double window_ms = to_ms(now - s.window_start);
    if (window_ms < 1000.0) return;

    Summary &out = s.summary;
    const double frames = static_cast<double>(s.frames);
    const double virtual_ms = static_cast<double>(virtual_us - s.window_virtual_us) / 1000.0;
    out.valid = true;
    out.fps = frames * 1000.0 / window_ms;
    out.game_fps = virtual_ms > 0.0 ? frames * 1000.0 / virtual_ms : 0.0;
    out.speed = virtual_ms / window_ms;
    out.lists = static_cast<double>(s.list_sum) * 1000.0 / window_ms;
    out.frame_avg_ms = s.frame_sum_ms / frames;
    out.frame_max_ms = s.frame_max_ms;
    const double gpu_wait_ms = to_ms(s.wait_sum) / frames;
    out.wait_ms = gpu_wait_ms + to_ms(s.pacing_sum) / frames;
    // GPU waits happen inside the timed render calls; count them once. Pacing
    // happens outside them, so subtracting it too hid the render time
    // whenever the game was ahead of real time.
    out.render_ms = std::max(0.0, to_ms(s.render_sum) / frames - gpu_wait_ms);
    out.guest_ms = std::max(0.0, out.frame_avg_ms - out.render_ms - out.wait_ms);
    out.overlay_ms = to_ms(s.overlay_sum) / frames;
    out.present_mode = s.present_mode;
    out.width = s.width;
    out.height = s.height;
    out.refresh_hz = s.refresh_hz;
    if (options().log) print(out);

    s.window_start = now;
    s.window_virtual_us = virtual_us;
    s.frames = 0u;
    s.frame_sum_ms = 0.0;
    s.frame_max_ms = 0.0;
    s.render_sum = s.wait_sum = s.pacing_sum = s.overlay_sum = Clock::duration{};
    s.list_sum = 0u;
}

const Summary &last_second() { return state().summary; }
const std::array<float, kHistoryFrames> &frame_history() { return state().history; }
std::size_t history_cursor() { return state().cursor; }

} // namespace mhp3rd::perf
