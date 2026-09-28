#pragma once

#include "input/bindings.hpp"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <span>
#include <vector>

// Matching the held inputs of one device to its chords over time (#198).
//
// The longest bound chord among the held inputs wins, and its inputs do
// nothing else; what is left is matched the same way. A chord that could
// still grow into a longer bound one waits a short window (the chord window,
// ~50 ms) for the rest, but only when acting at once would do harm
// (waits_for): an input in no chord, or one whose longer chord does what it
// does and more (LB in LB + X for L + □), acts at once. An input tapped and
// released inside the window still presses its action, for a moment, when
// it is released. Nothing here needs SDL; the time is the caller's.
namespace mhp3rd::input {

// What the window adds when a tap is released: at least this long pressed,
// so a game reading 30 times a second sees it.
inline constexpr unsigned kMinTapMs = 40u;
inline constexpr unsigned kDefaultChordWindowMs = 50u;
inline constexpr unsigned kMaxChordWindowMs = 200u;

class Resolver {
public:
    // What the held inputs press at `now_ms`, a steady clock in
    // milliseconds. `window_ms` 0 turns the window off. `reserved` are chords
    // the port reads by itself, such as L3 + R3 for the menu: they win like
    // any chord and press nothing.
    PadState update(const Table &table, const std::function<bool(Binding)> &held, std::uint64_t now_ms,
                    unsigned window_ms, std::span<const Chord> reserved = {});
    // Forgets everything held, for when the device goes away.
    void reset();

    // What an update decided, for MHP3RD_TRACE_PAD.
    struct Event {
        enum class Kind : std::uint8_t { Pressed, Waiting, Tapped, Released };
        Kind kind{};
        Chord chord;
        std::uint64_t at_ms{};
        std::uint64_t waited_ms{};  // since its last input went down
    };
    [[nodiscard]] const std::vector<Event> &events() const { return events_; }
    // The targets pressing now, in Table order.
    [[nodiscard]] const std::vector<std::size_t> &targets() const { return targets_; }

private:
    struct Held {
        Binding input{};
        std::uint64_t since{};
        bool spent{};  // left over from a chord that let go: nothing until released
    };
    struct Active {
        Chord chord;
        Chord base;  // what it grew from, which acts again if it is let go
    };
    struct Tap {
        Chord chord;
        std::uint64_t until{};
    };
    std::vector<Held> held_;
    std::vector<Active> active_;
    std::vector<Chord> waiting_;
    std::vector<Tap> taps_;
    std::vector<Event> events_;
    std::vector<std::size_t> targets_;
};

// The PSP state of the targets pressing, as read() gives it.
[[nodiscard]] PadState state_of(const Table &table, std::span<const std::size_t> targets);

} // namespace mhp3rd::input
