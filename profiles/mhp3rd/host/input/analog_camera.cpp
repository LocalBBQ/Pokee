#include "input/analog_camera.hpp"

#include "settings/settings.hpp"

#include "psprecomp/guest_memory.hpp"
#include "psprecomp/runtime.hpp"

#include <cmath>
#include <cstring>
#include <iostream>
#include <vector>

namespace mhp3rd::input {
namespace {

// What the game adds to its own yaw each frame it turns: 1150 of the 65536
// units in a turn, which is 6.317139 degrees, and matches the 6.3170-6.3177
// measured from outside in #103.
constexpr int kGameStep = 1150;
constexpr float kUnitsPerTurn = 65536.0f;
// The game runs its camera at thirty frames a second.
constexpr float kFramesPerSecond = 30.0f;
// A turn big enough to be worth searching against, in degrees per frame.
constexpr float kSearching = 1.5f;
// Frames of disagreement before the field is assumed to have moved on -- a
// quest ending or an overlay swap puts something else at that address.
constexpr int kStrikes = 12;

struct Camera {
    enum class State { Looking, Watching, Locked } state{State::Looking};
    std::vector<std::uint8_t> before;       // guest memory, held only while searching
    std::vector<std::uint32_t> candidates;  // and only until one is left
    std::vector<std::int16_t> previous;
    std::uint32_t address{};
    std::int16_t last{};
    int strikes{};
    bool announced{};
    // What the port still owes the camera, kept between frames so that a turn
    // slower than one unit a frame is not rounded away to nothing.
    float owed{};
};

Camera &camera() {
    static Camera value;
    return value;
}

std::int16_t read16(const std::uint8_t *at) {
    std::int16_t value = 0;
    std::memcpy(&value, at, sizeof(value));
    return value;
}

// Is this the game's own step, or a whole number of them? The guest can update
// its camera more than once between two presented frames.
bool moved_by_steps(std::int16_t moved) {
    const int value = moved < 0 ? -moved : moved;
    return value != 0 && (value % kGameStep) == 0;
}

void forget(Camera &c, const char *why) {
    if (c.state == Camera::State::Locked)
        std::cout << "[analog-camera] lost the camera at 0x" << std::hex << c.address << std::dec << " (" << why
                  << "); looking again\n";
    c = Camera{};
}

} // namespace

bool analog_camera_driving() {
    return settings::current().analog_camera && camera().state == Camera::State::Locked;
}

void analog_camera_frame(psprecomp::Runtime &runtime, float turn, float deflection) {
    Camera &c = camera();
    if (!settings::current().analog_camera) {
        if (c.state != Camera::State::Looking) c = Camera{};
        return;
    }

    psprecomp::GuestMemory &memory = runtime.memory();
    const std::uint32_t base = psprecomp::GuestMemory::kPhysicalBase;
    const std::uint32_t size = memory.size();
    const std::uint8_t *ram = memory.raw_pointer(base, size);
    if (ram == nullptr) return;

    if (c.state == Camera::State::Looking) {
        // Nothing to compare against until the game turns the camera itself,
        // which it will as soon as the player pushes the stick far enough --
        // the stick is still being handed through at this point.
        if (std::fabs(turn) < kSearching) return;
        if (c.before.empty()) {
            c.before.assign(ram, ram + size);
            return;
        }
        for (std::uint32_t offset = 0; offset + 2u <= size; offset += 2u) {
            const std::int16_t moved =
                static_cast<std::int16_t>(read16(ram + offset) - read16(c.before.data() + offset));
            if (moved == kGameStep || moved == -kGameStep) {
                c.candidates.push_back(base + offset);
                c.previous.push_back(read16(ram + offset));
            }
        }
        c.before.clear();
        c.before.shrink_to_fit();  // 64 MiB, given back at once
        c.state = Camera::State::Watching;
        std::cout << "[analog-camera] " << c.candidates.size() << " fields move by the game's own step\n";
        return;
    }

    if (c.state == Camera::State::Watching) {
        // Keep the ones that go on behaving like the camera. Every frame, not
        // only the turning ones: the field advances a step per frame, so a
        // skipped frame makes it look as though it jumped several.
        std::vector<std::uint32_t> kept;
        std::vector<std::int16_t> kept_previous;
        for (std::size_t i = 0; i < c.candidates.size(); ++i) {
            const std::int16_t now = read16(ram + (c.candidates[i] - base));
            const std::int16_t moved = static_cast<std::int16_t>(now - c.previous[i]);
            if (moved == 0 || moved_by_steps(moved)) {
                kept.push_back(c.candidates[i]);
                kept_previous.push_back(now);
            }
        }
        c.candidates.swap(kept);
        c.previous.swap(kept_previous);
        if (c.candidates.empty()) {
            forget(c, "nothing kept behaving like it");
            return;
        }
        if (c.candidates.size() == 1u) {
            c.address = c.candidates.front();
            c.last = c.previous.front();
            c.state = Camera::State::Locked;
            c.candidates.clear();
            c.previous.clear();
            std::cout << "[analog-camera] the camera's yaw is at 0x" << std::hex << c.address << std::dec
                      << "; driving it from the stick now\n";
        }
        return;
    }

    // Locked: the port owns the camera.
    const std::int16_t now = static_cast<std::int16_t>(memory.load16(c.address));
    if (now != c.last) {
        // Something other than this code moved it. The game still owns the
        // camera in cutscenes and conversations, and an overlay swap can put
        // something else at this address entirely; a few frames of that and the
        // port steps back and looks again rather than fighting it.
        if (++c.strikes >= kStrikes) {
            forget(c, "it moved on its own");
            return;
        }
    } else {
        c.strikes = 0;
    }

    const float speed = settings::current().camera_speed;              // degrees a second
    // The game's angle counts the other way round from the stick: its own step
    // is added or subtracted from a direction byte, and following the stick's
    // sign turns the camera the wrong way. Inverting here rather than at the
    // caller keeps the player's own "invert camera horizontally" working on top
    // of this instead of cancelling it.
    const float per_frame = -deflection * speed / kFramesPerSecond;    // degrees this frame
    c.owed += per_frame * kUnitsPerTurn / 360.0f;                      // in the game's own units
    const int whole = static_cast<int>(c.owed);
    c.owed -= static_cast<float>(whole);
    if (whole == 0) {
        c.last = now;
        return;
    }
    const std::int16_t written = static_cast<std::int16_t>(now + whole);
    memory.store16(c.address, static_cast<std::uint16_t>(written));
    // The field found is the camera's *target* angle; two bytes later is the
    // one the view is actually built from, which the game eases towards the
    // target by a quarter of the difference each frame. That easing is where
    // the eighteen degrees of coast measured in #103 come from, so setting both
    // leaves the filter nothing to do and the camera stops where the stick is
    // let go. The game still writes the smoothed field itself whenever it wants
    // the camera, so nothing is taken away from it permanently.
    memory.store16(c.address + 2u, static_cast<std::uint16_t>(written));
    c.last = written;
    if (!c.announced) {
        std::cout << "[analog-camera] turning the camera from the stick\n";
        c.announced = true;
    }
}

} // namespace mhp3rd::input
