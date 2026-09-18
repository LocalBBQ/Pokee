#pragma once

#include <array>
#include <chrono>
#include <cstdint>
#include <string>

namespace mhp3rd::perf {

using Clock = std::chrono::steady_clock;

// What the performance setting asks for (MHP3RD_PERF, or the in-game menu):
// the log line, the overlay or both. The overlay can also be toggled at run
// time (F3).
struct Options {
    bool log{};
    bool overlay{};
};
[[nodiscard]] Options options();

// Host frame statistics, cheap enough to collect all the time.
//
// A frame is the interval between two guest flips (sceDisplaySetFrameBuf),
// which is also where the renderer presents. Within it, time spent turning
// display lists into Vulkan commands and recording the present is "render",
// time blocked on the GPU — the frame fence, swapchain acquire, queue submit
// and present, and texture uploads waiting for the queue — or holding the
// game to real time is "wait", and
// everything else — recompiled code, HLE, the kernel, input — is "guest".
// Render includes the waits that happen inside it; the summary subtracts them.
void add_render_time(Clock::duration duration);
void add_wait_time(Clock::duration duration);
void add_overlay_time(Clock::duration duration);
void count_display_list();

// Closes the current frame. `virtual_us` is the kernel's clock, which the
// game's own frame rate and the emulation speed are measured against.
void end_frame(std::uint64_t virtual_us);

// Drops the frame and the second in progress, so time spent paused in the
// in-game menu shows up in neither the frame times nor the next log line.
void restart_measurement();

// Shown next to the numbers: the present mode, the swapchain size and the
// display's refresh rate (0 when SDL cannot tell).
void set_display_info(const std::string &present_mode, std::uint32_t width, std::uint32_t height, float refresh_hz);

// Averages over the last whole second of real time.
struct Summary {
    bool valid{};
    double fps{};             // frames presented per real second
    double game_fps{};        // guest flips per emulated second
    double speed{};           // emulated time per real time, 1.0 = real time
    double lists{};           // display lists enqueued per real second
    double frame_avg_ms{};
    double frame_max_ms{};
    double guest_ms{};
    double render_ms{};
    double wait_ms{};
    double overlay_ms{};
    std::string present_mode;
    std::uint32_t width{};
    std::uint32_t height{};
    float refresh_hz{};
};
[[nodiscard]] const Summary &last_second();

// Frame times in milliseconds, a ring written at `history_cursor()`.
inline constexpr std::size_t kHistoryFrames = 192u;
[[nodiscard]] const std::array<float, kHistoryFrames> &frame_history();
[[nodiscard]] std::size_t history_cursor();

} // namespace mhp3rd::perf
