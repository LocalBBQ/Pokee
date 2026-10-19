#pragma once

// MHFU_TRACE_BODIES: how long a dead monster lies on the quest entity list
// before the game removes it, and which of its fields count that time.
//
// Once a frame, every entity on the list (game/guest_layout.hpp) that is not
// a hunter, keeps a health pool and has health at or below zero is a body.
// From the frame it is first seen dead, kSpan bytes from the entity are read
// each frame and every byte, aligned halfword and aligned float is followed.
// When the body leaves the list, comes back to life (the slot reused for a
// new monster) or changes species, the trace prints how many frames and
// seconds it lay there, and the fields that moved the way a timer does: in
// one direction only, at least kMinChanges times, by exactly 1 each step
// for a byte or a halfword, and in steps of about the same size for a float.
// A field's first change may be a jump, the game setting the timer after
// the health reached zero; the field then starts from there. Each counter
// names the frame of its first step. Then the bytes that held still and
// changed only near the end are printed, the earliest first.
// Leaving the quest forgets the bodies without a report.
//
// Pure over a GuestMemory, so the tests run it on a buffer. Nothing here
// changes guest memory.

#include "psprecomp/guest_memory.hpp"

#include <chrono>
#include <cstdint>
#include <optional>
#include <ostream>
#include <string>
#include <vector>

namespace mhfu::game::body_trace {

// MHFU_TRACE_BODIES is set. Read once.
[[nodiscard]] bool enabled() noexcept;

// Bytes followed from the entity's start. The fields lock-on knows end at
// 0x420; the entity is larger, and how much larger is not known.
inline constexpr std::uint32_t kSpan = 0x1000u;
// Changes a field needs before it is reported, about two seconds of frames.
inline constexpr std::uint32_t kMinChanges = 20u;
// Fields reported per body, the ones that changed most first.
inline constexpr std::size_t kMaxReported = 16u;

enum class Width : std::uint8_t { Byte, Half, Float };

// A byte that held still from the death and then changed at most kMaxFlips
// times, the first of them in the last kLateFrames before the body went: a
// state or a flag the removal may follow from. The first step of the sinking
// counter shows where the removal began; these show what came before it.
inline constexpr std::uint32_t kMaxFlips = 4u;
inline constexpr std::uint32_t kLateFrames = 120u;
inline constexpr std::size_t kMaxFlags = 32u;

// A field of a body that moved like a timer. `from` is the snapshot of its
// first step, counted from 0 at the death.
struct Counter {
    Width width{};
    std::uint32_t offset{};
    double first{};
    double last{};
    std::uint32_t changes{};
    std::uint32_t from{};
};

// A byte that changed late; `at` is the snapshot of its first change.
struct Flag {
    std::uint32_t offset{};
    std::uint8_t first{};
    std::uint8_t last{};
    std::uint32_t changes{};
    std::uint32_t at{};
};

// Every field of one body, followed snapshot by snapshot. Each snapshot is
// the same size as the first.
class Fields {
public:
    void see(const std::vector<std::uint8_t> &bytes);
    // The fields that count, the ones that changed most first, at most
    // kMaxReported. A byte whose halfword at the same place also counts is
    // left out: a halfword counting by one moves its low byte by one too.
    [[nodiscard]] std::vector<Counter> counters() const;
    // The late flags, the earliest first, at most kMaxFlags.
    [[nodiscard]] std::vector<Flag> flags() const;
    // Snapshots seen.
    [[nodiscard]] std::uint32_t frames() const noexcept { return frames_; }

    struct Follow {
        double first{};
        double previous{};
        bool started{};  // the first change was seen
        int direction{}; // +1, -1, or 0 before the first step
        bool broken{};   // moved both ways, or by a step a timer does not take
        std::uint32_t changes{};
        double smallest{};
        double largest{};
        std::uint32_t from{};
    };

private:
    std::uint32_t frames_{};
    std::vector<Follow> bytes_;
    std::vector<Follow> halves_;
    std::vector<Follow> floats_;
    std::vector<Flag> flips_; // every byte: its changes and the first one
};

// One line per counter or flag, as the trace prints it. A flag's line names
// the snapshots the body was seen in, `frames`.
[[nodiscard]] std::string describe(const Counter &counter);
[[nodiscard]] std::string describe(const Flag &flag, std::uint32_t frames);

// MHFU_TRACE_BODIES_WATCH=OFFSET: the field of a raptor's body whose guest
// writes are logged ([watch-write] lines, with the code address of each).
// PSPRECOMP_WATCH_WRITE must be set too, to any address: it arms the store
// check at start-up, and slows every store while it is armed. The sinking at
// the end of a body's time was seen at +0x244 on a Giaprey. Nothing when
// unset or not a number.
[[nodiscard]] std::optional<std::uint32_t> parse_watch(const char *text) noexcept;
[[nodiscard]] std::optional<std::uint32_t> watch_offset() noexcept;

// The species the watch is armed by: Velociprey, Genprey, Ioprey, Giaprey.
[[nodiscard]] bool raptor(std::uint8_t species) noexcept;

class Tracker {
public:
    // With a watch offset, the first raptor body to die asks for that field
    // to be watched; take_watch() hands the address over once.
    explicit Tracker(std::optional<std::uint32_t> watch = std::nullopt) : watch_(watch) {}

    // Reads the list once. `now` stamps the seconds a body lay there.
    void frame(const psprecomp::GuestMemory &memory, std::ostream &out,
        std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now());

    [[nodiscard]] std::optional<std::uint32_t> take_watch() noexcept {
        const std::optional<std::uint32_t> request = watch_request_;
        watch_request_.reset();
        return request;
    }

private:
    std::optional<std::uint32_t> watch_;
    std::optional<std::uint32_t> watch_request_;
    bool watched_{};

    struct Body {
        std::uint32_t entity{};
        std::uint8_t species{};
        std::uint64_t died{};
        std::chrono::steady_clock::time_point died_at{};
        bool seen{};
        Fields fields;
    };
    void report(const Body &body, const char *why, std::ostream &out, std::chrono::steady_clock::time_point now) const;

    std::vector<Body> bodies_;
    std::uint64_t frame_{};
};

} // namespace mhfu::game::body_trace
