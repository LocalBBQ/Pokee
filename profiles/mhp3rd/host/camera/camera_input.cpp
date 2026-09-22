#include "camera/camera_input.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>

namespace mhp3rd::camera {
namespace {

constexpr std::size_t kSources = static_cast<std::size_t>(Source::Count);
// A frame longer than this is a pause (the menu, a load), not motion to catch
// up on: one step at most, whatever the stick held meanwhile.
constexpr float kLongestStep = 0.1f;

struct Rate {
    float yaw{};
    float pitch{};
};
std::array<Rate, kSources> rates{};
Turn pending{};

} // namespace

void set_rate(Source source, float yaw, float pitch) {
    const auto index = static_cast<std::size_t>(source);
    if (index >= kSources) return;
    rates[index] = Rate{std::clamp(yaw, -1.0f, 1.0f), std::clamp(pitch, -1.0f, 1.0f)};
}

void add_motion(Source, float yaw_degrees, float pitch_degrees) {
    if (!std::isfinite(yaw_degrees) || !std::isfinite(pitch_degrees)) return;
    pending.yaw_degrees += yaw_degrees;
    pending.pitch_degrees += pitch_degrees;
    pending.yaw_held |= yaw_degrees != 0.0f;
    pending.pitch_held |= pitch_degrees != 0.0f;
}

void advance(float seconds, float degrees_per_second) {
    if (!std::isfinite(seconds) || seconds <= 0.0f) return;
    const float step = std::min(seconds, kLongestStep) * degrees_per_second;
    for (const Rate &rate : rates) {
        pending.yaw_degrees += rate.yaw * step;
        pending.pitch_degrees += rate.pitch * step;
        pending.yaw_held |= rate.yaw != 0.0f;
        pending.pitch_held |= rate.pitch != 0.0f;
    }
}

Turn take() {
    const Turn turn = pending;
    pending = Turn{};
    return turn;
}

void discard() { pending = Turn{}; }

void reset() {
    rates.fill(Rate{});
    pending = Turn{};
}

} // namespace mhp3rd::camera
