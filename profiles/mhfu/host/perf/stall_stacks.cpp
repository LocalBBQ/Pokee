#include "perf/stall_stacks.hpp"

#include <cstdlib>

#if defined(__linux__) && defined(__GLIBC__)
#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdio>
#include <execinfo.h>
#include <pthread.h>
#include <thread>
#include <unistd.h>
#endif

namespace mhfu::perf {

bool stall_stacks_enabled() {
    static const bool enabled = std::getenv("MHFU_TRACE_STALL_STACKS") != nullptr;
    return enabled;
}

#if defined(__linux__) && defined(__GLIBC__)
namespace {

using Clock = std::chrono::steady_clock;

constexpr auto kFirstSample = std::chrono::milliseconds(45);
constexpr auto kNextSample = std::chrono::milliseconds(15);
constexpr int kSamplesPerCall = 4;
constexpr int kMaxFrames = 48;

std::atomic<const char *> g_call{nullptr};
std::atomic<std::int64_t> g_call_start{0}; // steady clock, ns
std::atomic<std::uint64_t> g_call_serial{0};
pthread_t g_game_thread{};

// Filled by the signal handler on the game thread, read by the watchdog.
void *g_frames[kMaxFrames];
std::atomic<int> g_depth{0};
std::atomic<bool> g_taken{false};

void on_sample(int) {
    g_depth.store(backtrace(g_frames, kMaxFrames), std::memory_order_relaxed);
    g_taken.store(true, std::memory_order_release);
}

std::int64_t now_ns() {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now().time_since_epoch()).count();
}

void watchdog() {
    std::uint64_t sampled_serial = 0;
    int samples = 0;
    for (;;) {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        const char *call = g_call.load(std::memory_order_acquire);
        if (call == nullptr) continue;
        const std::uint64_t serial = g_call_serial.load(std::memory_order_acquire);
        if (serial != sampled_serial) {
            sampled_serial = serial;
            samples = 0;
        }
        if (samples >= kSamplesPerCall) continue;
        const auto inside = std::chrono::nanoseconds(now_ns() - g_call_start.load(std::memory_order_acquire));
        if (inside < kFirstSample + kNextSample * samples) continue;
        ++samples;
        g_taken.store(false, std::memory_order_release);
        if (pthread_kill(g_game_thread, SIGUSR2) != 0) continue;
        for (int i = 0; i < 100 && !g_taken.load(std::memory_order_acquire); ++i)
            std::this_thread::sleep_for(std::chrono::microseconds(100));
        // The call may have ended while the signal was on its way; the stack
        // is then of whatever came next, and says so by its frames.
        if (!g_taken.load(std::memory_order_acquire)) continue;
        std::fprintf(stderr, "[stall-stack] %s after %.1f ms:\n", call,
            std::chrono::duration<double, std::milli>(inside).count());
        std::fflush(stderr);
        backtrace_symbols_fd(g_frames, g_depth.load(std::memory_order_relaxed), STDERR_FILENO);
    }
}

void start_watchdog() {
    struct sigaction action{};
    action.sa_handler = on_sample;
    action.sa_flags = SA_RESTART;
    sigemptyset(&action.sa_mask);
    sigaction(SIGUSR2, &action, nullptr);
    // backtrace() loads its unwinder on first use, which is not safe inside
    // a signal handler: do it here.
    void *warm[2];
    (void)backtrace(warm, 2);
    std::thread(watchdog).detach();
}

} // namespace

void stall_call_begin(const char *name) {
    static const bool started = [] {
        g_game_thread = pthread_self();
        start_watchdog();
        return true;
    }();
    (void)started;
    g_call_start.store(now_ns(), std::memory_order_release);
    g_call_serial.fetch_add(1u, std::memory_order_acq_rel);
    g_call.store(name, std::memory_order_release);
}

void stall_call_end() {
    g_call.store(nullptr, std::memory_order_release);
}
#else
void stall_call_begin(const char *) {}
void stall_call_end() {}
#endif

} // namespace mhfu::perf
