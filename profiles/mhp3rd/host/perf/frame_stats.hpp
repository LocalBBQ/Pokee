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
// which is also where the renderer presents; with frame interpolation it
// presents between flips instead, and `fps`, the frame times and the graph
// follow the presents while `game` and the time split follow the flips, the
// presents' own time included. Within it, time spent turning
// display lists into Vulkan commands and recording the present is "render",
// time blocked on the GPU — the frame fence, swapchain acquire, queue submit
// and present, and texture uploads waiting for the queue — or holding the
// game to real time is "wait", and
// everything else — recompiled code, HLE, the kernel, input — is "guest".
// Render includes the GPU waits that happen inside it; the summary subtracts
// those, and only those, from it.
void add_render_time(Clock::duration duration);

// Where the render thread stops, for MHP3RD_TRACE_STALLS. The waits count
// towards "wait"; the copies are CPU work inside "render" that reads memory
// the GPU wrote, which can be slow when that memory is not cached.
enum class Stall : std::uint8_t {
    Fence,     // the frame fence, before recording the next frame
    Acquire,   // vkAcquireNextImageKHR
    Submit,    // the frame's vkQueueSubmit (MoltenVK waits for a drawable here)
    Present,   // vkQueuePresentKHR
    Upload,    // a texture upload waiting for the queue to go idle
    Evict,     // the queue idle wait before a cached texture is destroyed
    Readback,  // a framebuffer read back for a GE block transfer
    Idle,      // other device or queue idle waits: settings changes, captures
    Pacing,    // the kernel holding the game to real time
    Copy,      // copying the written-back frame out of mapped memory
    Store,     // converting that frame into guest memory (store_frame)
    Count,
};
[[nodiscard]] const char *stall_name(Stall kind);

// Time blocked on the GPU inside a render call.
void add_wait_time(Clock::duration duration, Stall kind);
// CPU work that reads GPU-written memory; only traced, it is already part of
// the render call it happens in.
void note_stall(Stall kind, Clock::duration duration);
// GPU execution time of the frame recorded before this one, measured with
// timestamp queries, in milliseconds.
void add_gpu_time(double milliseconds);
// The device cannot time the GPU (no timestamp support, or turned off).
void set_gpu_time_unavailable();
// Time spent holding the game to real time, outside any render call.
void add_pacing_time(Clock::duration duration);
void add_overlay_time(Clock::duration duration);
void count_display_list();
// A draw the GE made, and a draw call the renderer recorded: fewer when
// consecutive draws are merged.
void count_draw();
void count_recorded_draws(std::uint32_t count);

// Closes the current frame. `virtual_us` is the kernel's clock, which the
// game's own frame rate and the emulation speed are measured against.
// `presented`: the flip also put a picture on the screen, which it does
// unless frame interpolation presents between flips.
void end_frame(std::uint64_t virtual_us, bool presented = true);

// A present between the game's flips (frame interpolation).
void count_present();
// The rate frame interpolation presents at now and the one the setting asks
// for, shown on the perf line; 0 when it is off.
void set_frame_rate_info(double rate, double requested);

// Drops the frame and the second in progress, so time spent paused in the
// in-game menu shows up in neither the frame times nor the next log line.
void restart_measurement();

// Shown next to the numbers: the present mode, the swapchain size and the
// display's refresh rate (0 when SDL cannot tell).
void set_display_info(const std::string &present_mode, std::uint32_t width, std::uint32_t height, float refresh_hz);

// Averages over the last whole second of real time.
struct Summary {
    bool valid{};
    std::uint64_t second{};   // counts the summaries, so a reader can tell a new one
    double fps{};             // presents per real second
    double game_fps{};        // guest flips per emulated second
    double speed{};           // emulated time per real time, 1.0 = real time
    double lists{};           // display lists enqueued per real second
    double draws{};           // GE draws per frame
    double recorded_draws{};  // Vulkan draw calls per frame
    double frame_avg_ms{};
    double frame_max_ms{};
    double guest_ms{};
    double render_ms{};
    double wait_ms{};
    double pacing_ms{};       // of wait: holding the game to real time, when it had nothing to do
    double overlay_ms{};
    double frame_rate{};      // frame interpolation's rate now and the setting's, 0 when off
    double requested_rate{};
    // GPU time per frame from timestamp queries, when the device has them.
    bool gpu_valid{};
    double gpu_avg_ms{};
    double gpu_max_ms{};
    std::string present_mode;
    std::uint32_t width{};
    std::uint32_t height{};
    float refresh_hz{};
};
[[nodiscard]] const Summary &last_second();

// MHP3RD_PERF_ALTERNATE=name[,name...]: the named new renderer paths are
// turned off every other second, so one run measures them against the paths
// they replaced under the same load. Each [perf] line ends in "alt on" or
// "alt off" for the second it covers. Names: direct, lookup, reuse, merge.
enum class NewPath : std::uint8_t { Direct, Lookup, Reuse, Merge };
// True while `path` is to take its old route this second.
[[nodiscard]] bool alternate_off(NewPath path);

// Frame times in milliseconds, a ring written at `history_cursor()`.
inline constexpr std::size_t kHistoryFrames = 192u;
[[nodiscard]] const std::array<float, kHistoryFrames> &frame_history();
[[nodiscard]] std::size_t history_cursor();

} // namespace mhp3rd::perf
