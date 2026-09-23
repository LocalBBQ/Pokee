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

constexpr std::size_t kStallKinds = static_cast<std::size_t>(Stall::Count);

struct StallTally {
    std::uint32_t count{};
    Clock::duration total{};
    Clock::duration longest{};

    void add(Clock::duration duration) {
        ++count;
        total += duration;
        longest = std::max(longest, duration);
    }
    void add(const StallTally &other) {
        count += other.count;
        total += other.total;
        longest = std::max(longest, other.longest);
    }
};
using StallTallies = std::array<StallTally, kStallKinds>;

// MHP3RD_TRACE_STALLS: a [stalls] line once a second, and a [slow-frame] line
// for every frame longer than MHP3RD_TRACE_STALLS_MS (default 40 ms).
struct StallTrace {
    bool enabled{};
    double slow_frame_ms{40.0};
};

struct Alternate {
    std::uint32_t paths{};  // bit per NewPath named in MHP3RD_PERF_ALTERNATE
    bool off{};             // the second in progress takes the old paths
};

Alternate &alternate() {
    static Alternate value = [] {
        Alternate result{};
        const char *text = std::getenv("MHP3RD_PERF_ALTERNATE");
        if (text == nullptr) return result;
        const std::string names = std::string(",") + text + ",";
        const char *known[] = {"direct", "lookup", "reuse", "merge"};
        for (std::uint32_t i = 0; i < 4u; ++i)
            if (names.find(std::string(",") + known[i] + ",") != std::string::npos) result.paths |= 1u << i;
        return result;
    }();
    return value;
}

const StallTrace &stall_trace() {
    static const StallTrace value = [] {
        StallTrace trace{};
        trace.enabled = std::getenv("MHP3RD_TRACE_STALLS") != nullptr;
        if (const char *text = std::getenv("MHP3RD_TRACE_STALLS_MS"); text != nullptr) {
            const double ms = std::strtod(text, nullptr);
            if (ms > 0.0) trace.slow_frame_ms = ms;
        }
        return trace;
    }();
    return value;
}

struct State {
    // Frame in progress.
    Clock::time_point frame_start{Clock::now()};
    Clock::duration render{};
    Clock::duration wait{};
    Clock::duration pacing{};
    Clock::duration overlay{};
    std::uint32_t lists{};
    std::uint32_t draws{};
    std::uint32_t recorded_draws{};
    StallTallies stalls{};
    double gpu_ms{};
    std::uint32_t gpu_samples{};

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
    std::uint64_t draw_sum{};
    std::uint64_t recorded_draw_sum{};
    StallTallies stall_sum{};
    double gpu_sum_ms{};
    double gpu_max_ms{};
    std::uint32_t gpu_frames{};
    bool gpu_unavailable{};
    std::uint64_t frame_number{};

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
                               "render %.1f wait %.1f ms | lists %.0f/s draws %.0f/%.0f | %s %ux%u",
                               s.fps, s.game_fps, s.speed * 100.0, s.frame_avg_ms, s.frame_max_ms, s.guest_ms,
                               s.render_ms, s.wait_ms, s.lists, s.draws, s.recorded_draws, s.present_mode.c_str(),
                               s.width, s.height);
    if (s.refresh_hz > 0.0f && length > 0 && static_cast<std::size_t>(length) < sizeof(line))
        length += std::snprintf(line + length, sizeof(line) - length, " %.0fHz", s.refresh_hz);
    if (length > 0 && static_cast<std::size_t>(length) < sizeof(line)) {
        if (s.gpu_valid)
            length += std::snprintf(line + length, sizeof(line) - length, " | gpu %.1f max %.1f ms", s.gpu_avg_ms,
                                    s.gpu_max_ms);
        else
            length += std::snprintf(line + length, sizeof(line) - length, " | gpu n/a");
    }
    if (s.overlay_ms > 0.0 && length > 0 && static_cast<std::size_t>(length) < sizeof(line))
        length += std::snprintf(line + length, sizeof(line) - length, " | overlay %.2f ms", s.overlay_ms);
    if (alternate().paths != 0u && length > 0 && static_cast<std::size_t>(length) < sizeof(line))
        std::snprintf(line + length, sizeof(line) - length, " | alt %s", alternate().off ? "off" : "on");
    // Flushed per line: the log is read while the game runs, often through a
    // pipe where stdout would otherwise sit in a block buffer.
    std::cout << line << std::endl;
}

// " name avg max M xN" for every kind that happened: the average per frame
// over `frames` frames, the longest single stall and how many there were. For
// a single frame, " name total xN".
std::string format_stalls(const StallTallies &tallies, double frames, bool per_frame_average) {
    std::string text;
    char part[96];
    for (std::size_t i = 0; i < kStallKinds; ++i) {
        const StallTally &tally = tallies[i];
        if (tally.count == 0u) continue;
        const double total = to_ms(tally.total);
        if (per_frame_average)
            std::snprintf(part, sizeof(part), " %s %.2f max %.2f x%u", stall_name(static_cast<Stall>(i)),
                          total / frames, to_ms(tally.longest), tally.count);
        else
            std::snprintf(part, sizeof(part), " %s %.2f x%u", stall_name(static_cast<Stall>(i)), total, tally.count);
        text += part;
    }
    return text.empty() ? std::string(" none") : text;
}

} // namespace

bool alternate_off(NewPath path) {
    const Alternate &value = alternate();
    return value.off && (value.paths & (1u << static_cast<std::uint32_t>(path))) != 0u;
}

const char *stall_name(Stall kind) {
    switch (kind) {
    case Stall::Fence: return "fence";
    case Stall::Acquire: return "acquire";
    case Stall::Submit: return "submit";
    case Stall::Present: return "present";
    case Stall::Upload: return "upload";
    case Stall::Evict: return "evict";
    case Stall::Readback: return "readback";
    case Stall::Idle: return "idle";
    case Stall::Pacing: return "pacing";
    case Stall::Copy: return "copy";
    case Stall::Store: return "store";
    case Stall::Count: break;
    }
    return "?";
}

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
    s.draws = s.recorded_draws = 0u;
    s.stalls = StallTallies{};
    s.gpu_ms = 0.0;
    s.gpu_samples = 0u;
    s.window_start = now;
    s.window_has_clock = false;
    s.frames = 0u;
    s.frame_sum_ms = 0.0;
    s.frame_max_ms = 0.0;
    s.render_sum = s.wait_sum = s.pacing_sum = s.overlay_sum = Clock::duration{};
    s.list_sum = 0u;
    s.draw_sum = s.recorded_draw_sum = 0u;
    s.stall_sum = StallTallies{};
    s.gpu_sum_ms = s.gpu_max_ms = 0.0;
    s.gpu_frames = 0u;
}

void add_render_time(Clock::duration duration) { state().render += duration; }
void add_wait_time(Clock::duration duration, Stall kind) {
    State &s = state();
    s.wait += duration;
    s.stalls[static_cast<std::size_t>(kind)].add(duration);
}
void note_stall(Stall kind, Clock::duration duration) {
    state().stalls[static_cast<std::size_t>(kind)].add(duration);
}
void add_gpu_time(double milliseconds) {
    State &s = state();
    s.gpu_ms += milliseconds;
    ++s.gpu_samples;
}
void set_gpu_time_unavailable() { state().gpu_unavailable = true; }
void add_pacing_time(Clock::duration duration) {
    State &s = state();
    s.pacing += duration;
    s.stalls[static_cast<std::size_t>(Stall::Pacing)].add(duration);
}
void add_overlay_time(Clock::duration duration) { state().overlay += duration; }
void count_display_list() { ++state().lists; }
void count_draw() { ++state().draws; }
void count_recorded_draws(std::uint32_t count) { state().recorded_draws += count; }

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
    s.draw_sum += s.draws;
    s.recorded_draw_sum += s.recorded_draws;
    for (std::size_t i = 0; i < kStallKinds; ++i) s.stall_sum[i].add(s.stalls[i]);
    if (s.gpu_samples != 0u) {
        s.gpu_sum_ms += s.gpu_ms;
        s.gpu_max_ms = std::max(s.gpu_max_ms, s.gpu_ms);
        ++s.gpu_frames;
    }
    ++s.frame_number;
    const StallTrace &trace = stall_trace();
    if (trace.enabled && frame_ms > trace.slow_frame_ms) {
        // The GPU time is the previous frame's: that is the work a fence wait
        // at the start of this frame was waiting for.
        const double render_ms = std::max(0.0, to_ms(s.render) - to_ms(s.wait));
        const double wait_ms = to_ms(s.wait) + to_ms(s.pacing);
        char head[192];
        std::snprintf(head, sizeof(head),
                      "[slow-frame] %llu %.1f ms | guest %.1f render %.1f wait %.1f ms | gpu(prev) ",
                      static_cast<unsigned long long>(s.frame_number), frame_ms,
                      std::max(0.0, frame_ms - render_ms - wait_ms), render_ms, wait_ms);
        std::string line = head;
        if (s.gpu_samples != 0u) {
            char gpu[32];
            std::snprintf(gpu, sizeof(gpu), "%.1f ms", s.gpu_ms);
            line += gpu;
        } else {
            line += "n/a";
        }
        line += " |" + format_stalls(s.stalls, 1.0, false);
        std::cout << line << std::endl;
    }
    s.render = s.wait = s.pacing = s.overlay = Clock::duration{};
    s.lists = 0u;
    s.draws = s.recorded_draws = 0u;
    s.stalls = StallTallies{};
    s.gpu_ms = 0.0;
    s.gpu_samples = 0u;

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
    out.draws = static_cast<double>(s.draw_sum) / frames;
    out.recorded_draws = static_cast<double>(s.recorded_draw_sum) / frames;
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
    out.gpu_valid = !s.gpu_unavailable && s.gpu_frames != 0u;
    out.gpu_avg_ms = s.gpu_frames != 0u ? s.gpu_sum_ms / static_cast<double>(s.gpu_frames) : 0.0;
    out.gpu_max_ms = s.gpu_max_ms;
    out.present_mode = s.present_mode;
    out.width = s.width;
    out.height = s.height;
    out.refresh_hz = s.refresh_hz;
    if (options().log) print(out);
    if (trace.enabled)
        std::cout << "[stalls] ms per frame over " << s.frames << " frames:" << format_stalls(s.stall_sum, frames, true)
                  << std::endl;

    s.window_start = now;
    if (alternate().paths != 0u) alternate().off = !alternate().off;
    s.window_virtual_us = virtual_us;
    s.frames = 0u;
    s.frame_sum_ms = 0.0;
    s.frame_max_ms = 0.0;
    s.render_sum = s.wait_sum = s.pacing_sum = s.overlay_sum = Clock::duration{};
    s.list_sum = 0u;
    s.draw_sum = s.recorded_draw_sum = 0u;
    s.stall_sum = StallTallies{};
    s.gpu_sum_ms = s.gpu_max_ms = 0.0;
    s.gpu_frames = 0u;
}

const Summary &last_second() { return state().summary; }
const std::array<float, kHistoryFrames> &frame_history() { return state().history; }
std::size_t history_cursor() { return state().cursor; }

} // namespace mhp3rd::perf
