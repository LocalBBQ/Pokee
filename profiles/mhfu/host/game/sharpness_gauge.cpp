#include "game/sharpness_gauge.hpp"

#include "game/guest_ram.hpp"

#include <algorithm>
#include <atomic>
#include <cstdlib>
#include <iostream>
#include <mutex>

namespace mhfu::game::sharpness {

int colour_index(const Ends &ends, int points) noexcept {
    int colour = 0;
    while (colour < static_cast<int>(kColours) && ends[static_cast<std::size_t>(colour)] < points) ++colour;
    return std::min(colour, static_cast<int>(kColours) - 1);
}

std::optional<Reading> gauge_reading(const Ends &ends, int points, int most) noexcept {
    if (most <= 0 || ends[0] < 0) return std::nullopt;
    for (std::size_t i = 1; i < kColours; ++i)
        if (ends[i] < ends[i - 1]) return std::nullopt;
    points = std::clamp(points, 0, most);
    const int colour = colour_index(ends, points);
    Reading reading;
    reading.colour = static_cast<Sharpness>(colour);
    reading.points = points;
    reading.most = most;
    reading.low = colour == 0 ? 0 : ends[static_cast<std::size_t>(colour - 1)];
    // The last colour a hunter can reach ends at the most, not at the table's end.
    reading.high = std::min(std::max(ends[static_cast<std::size_t>(colour)], points), most);
    const int span = reading.high - reading.low;
    reading.fill = span <= 0 ? 1.0f : std::clamp(static_cast<float>(points - reading.low) / span, 0.0f, 1.0f);
    return reading;
}

std::optional<Ends> weapon_ends(const Ram &ram, std::uint32_t hunter) {
    if (!ram.contains(hunter + kWeaponSlot, 4u) || ram.load8(hunter + kWeaponSlot + 1u) != kWeaponKind)
        return std::nullopt;
    const std::uint32_t id = ram.load16(hunter + kWeaponSlot + 2u);
    const std::uint32_t record = kWeaponRecords + id * kWeaponRecordBytes;
    if (!ram.contains(record, kWeaponRecordBytes)) return std::nullopt;
    const std::uint32_t model = ram.load16(record);
    const std::uint32_t row = ram.load8(record + kWeaponRow);
    if (!ram.contains(kModelClasses + model, 1u)) return std::nullopt;
    const std::uint32_t weapon_class = ram.load8(kModelClasses + model);
    if (weapon_class >= kClassCount || !ram.contains(kClassTables + weapon_class * 4u, 4u)) return std::nullopt;
    const std::uint32_t table = ram.load32(kClassTables + weapon_class * 4u);
    const std::uint32_t at = table + row * kRowBytes;
    if (table == 0u || !ram.contains(at, kRowBytes)) return std::nullopt;
    Ends ends{};
    for (std::size_t i = 0; i < kColours; ++i)
        ends[i] = static_cast<std::int16_t>(ram.load16(at + static_cast<std::uint32_t>(i) * 2u));
    return ends;
}

std::optional<Reading> read_hunter(const Ram &ram, std::uint32_t hunter) {
    if (hunter == 0u || !ram.contains(hunter + kPoints, 2u) || !ram.contains(hunter + kMostPoints, 2u))
        return std::nullopt;
    const std::optional<Ends> ends = weapon_ends(ram, hunter);
    if (!ends) return std::nullopt;
    std::optional<Reading> reading = gauge_reading(*ends, static_cast<std::int16_t>(ram.load16(hunter + kPoints)),
        static_cast<std::int16_t>(ram.load16(hunter + kMostPoints)));
    if (reading && ram.contains(hunter + kColourByte, 1u)) reading->game_colour = ram.load8(hunter + kColourByte);
    return reading;
}

std::uint32_t followed_hunter(const Ram &ram) {
    if (!ram.contains(kHunterList, 4u)) return 0u;
    const std::uint32_t list = ram.load32(kHunterList);
    if (list < 0x08800000u || !ram.contains(list + kFollowedHunter, 4u)) return 0u;
    const std::uint32_t hunter = ram.load32(list + kFollowedHunter);
    if (hunter < 0x08800000u || !ram.contains(hunter, kPoints + 2u)) return 0u;
    return hunter;
}

Edge cut_right(float x0, float x1, float u0, float u1, float remaining) noexcept {
    remaining = std::clamp(remaining, 0.0f, 1.0f);
    return {x0 + (x1 - x0) * remaining, u0 + (u1 - u0) * remaining};
}

float cap_shift(float bar_width, float remaining) noexcept {
    return bar_width * (std::clamp(remaining, 0.0f, 1.0f) - 1.0f);
}

namespace {

float fraction_of(const std::optional<Reading> &reading, bool enabled) noexcept {
    if (!enabled || !reading || reading->most <= 0) return -1.0f;
    return std::clamp(static_cast<float>(reading->points) / static_cast<float>(reading->most), 0.0f, 1.0f);
}

float colour_fraction(const std::optional<Reading> &reading, bool enabled) noexcept {
    if (!enabled || !reading) return -1.0f;
    return std::clamp(reading->fill, 0.0f, 1.0f);
}

struct State {
    std::mutex mutex;
    std::optional<Reading> reading;
    // Negative while the gauge draws nothing. The renderer reads these unlocked.
    std::atomic<float> left{-1.0f};
    std::atomic<float> colour{-1.0f};
};

State &state() {
    static State s;
    return s;
}

bool trace_enabled() {
    static const bool enabled = std::getenv("MHFU_TRACE_SHARPNESS") != nullptr;
    return enabled;
}

} // namespace

void gauge_frame(bool enabled, psprecomp::GuestMemory *memory) {
    std::optional<Reading> reading;
    if ((enabled || trace_enabled()) && memory != nullptr) {
        const GuestRam ram(*memory);
        reading = read_hunter(ram, followed_hunter(ram));
    }
    State &s = state();
    std::lock_guard lock(s.mutex);
    if (trace_enabled() && reading != s.reading) {
        if (reading)
            std::cout << "[sharpness] points " << reading->points << "/" << reading->most << " colour "
                      << static_cast<int>(reading->colour) << " (game " << reading->game_colour << ") from "
                      << reading->low << " to " << reading->high << ", fill " << reading->fill << std::endl;
        else
            std::cout << "[sharpness] no blade weapon" << std::endl;
    }
    s.reading = reading;
    s.left.store(fraction_of(reading, enabled), std::memory_order_relaxed);
    s.colour.store(colour_fraction(reading, enabled), std::memory_order_relaxed);
}

float bar_remaining() noexcept {
    return state().left.load(std::memory_order_relaxed);
}

float colour_remaining() noexcept {
    return state().colour.load(std::memory_order_relaxed);
}

void reset_for_tests() {
    State &s = state();
    std::lock_guard lock(s.mutex);
    s.reading.reset();
    s.left.store(-1.0f, std::memory_order_relaxed);
    s.colour.store(-1.0f, std::memory_order_relaxed);
}

void set_reading_for_tests(std::optional<Reading> reading, bool enabled) {
    State &s = state();
    std::lock_guard lock(s.mutex);
    s.reading = std::move(reading);
    s.left.store(fraction_of(s.reading, enabled), std::memory_order_relaxed);
    s.colour.store(colour_fraction(s.reading, enabled), std::memory_order_relaxed);
}

} // namespace mhfu::game::sharpness
