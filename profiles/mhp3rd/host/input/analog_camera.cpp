#include "input/analog_camera.hpp"

#include "settings/settings.hpp"

#include "psprecomp/guest_memory.hpp"
#include "psprecomp/runtime.hpp"

#include <cmath>
#include <cstring>
#include <cstdlib>
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
    std::vector<std::uint8_t> misses;   // frames each candidate has disagreed
    int searches{};                     // how many times the hunt has restarted
    std::vector<std::uint8_t> before;       // guest memory, held only while searching
    std::vector<std::uint32_t> candidates;  // and only until one is left
    std::vector<std::int16_t> previous;
    std::uint32_t address{};
    std::int16_t last{};
    int strikes{};
    bool announced{};
    std::int16_t wrote{};      // what the port put there last frame
    bool has_wrote{};
    int reported{};            // frames of before-and-after still to print
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
    const int searches = c.searches;
    if (c.state == Camera::State::Locked)
        std::cout << "[analog-camera] lost the camera at 0x" << std::hex << c.address << std::dec << ": " << why
                  << "; looking again\n";
    else
        std::cout << "[analog-camera] giving up this search: " << why << "\n";
    c = Camera{};
    c.searches = searches;  // so the log numbers them across restarts
}

// The vertical is not an axis at all: it is a height value plus a byte holding
// a level from 0 to 4, which is why up and down behave as one-shot commands
// with a glide. A byte with five states that changes exactly when the vertical
// fires is a sharp signature and hard to confuse with anything else, so this
// looks for it whenever the measured pitch jumps.
struct Vertical {
    std::vector<std::uint8_t> before;
    std::vector<std::uint32_t> levels;
    bool searching{};
    int firings{};
    bool reported{};
};

Vertical &vertical() {
    static Vertical value;
    return value;
}

// A pitch change this big in one frame is the command firing, not the camera
// drifting with the ground.
constexpr float kVerticalFired = 1.5f;

void look_for_vertical(const std::uint8_t *ram, std::uint32_t base, std::uint32_t size, float pitch_change) {
    // Off unless asked for. "A byte holding 0 to 4" also describes most of the
    // zero bytes in 64 MiB, so as it stands this keeps two hundred thousand
    // candidates and copies guest memory on every firing -- it needs a second
    // condition before it is worth running, and it should not cost anything or
    // fill the log meanwhile.
    static const bool wanted = std::getenv("MHP3RD_FIND_VERTICAL") != nullptr;
    if (!wanted) return;
    Vertical &v = vertical();
    if (v.reported) return;
    if (std::fabs(pitch_change) < kVerticalFired) {
        // Keep a picture of memory from a quiet frame to compare the next
        // firing against.
        if (!v.searching && v.before.empty()) v.before.assign(ram, ram + size);
        return;
    }
    if (v.before.empty()) return;

    if (!v.searching) {
        for (std::uint32_t offset = 0; offset < size; ++offset) {
            const std::uint8_t now = ram[offset];
            const std::uint8_t was = v.before[offset];
            if (now != was && now <= 4u && was <= 4u) v.levels.push_back(base + offset);
        }
        v.searching = true;
        v.firings = 1;
        std::cout << "[analog-camera] vertical: " << v.levels.size()
                  << " bytes hold a level of 0 to 4 and changed when it fired\n";
    } else {
        std::vector<std::uint32_t> kept;
        for (std::uint32_t address : v.levels) {
            const std::uint8_t now = ram[address - base];
            if (now <= 4u) kept.push_back(address);
        }
        v.levels.swap(kept);
        ++v.firings;
        std::cout << "[analog-camera] vertical: " << v.levels.size() << " left after " << v.firings << " firings";
        if (v.levels.size() <= 16u) {
            for (std::uint32_t address : v.levels) std::cout << " 0x" << std::hex << address << std::dec;
            if (!v.levels.empty() && v.firings >= 3) v.reported = true;
        }
        std::cout << "\n";
    }
    v.before.assign(ram, ram + size);
}

} // namespace

bool analog_camera_driving() {
    return settings::current().analog_camera && camera().state == Camera::State::Locked;
}

void analog_camera_frame(psprecomp::Runtime &runtime, float turn, float deflection, float pitch_change) {
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

    look_for_vertical(ram, base, size, pitch_change);

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
            // Several steps at once, not exactly one: the two frames compared
            // need not be consecutive, and the field advances every frame the
            // camera turns. Demanding exactly one step is why a fresh start
            // could hunt for ever without ever converging.
            if (moved_by_steps(moved)) {
                c.candidates.push_back(base + offset);
                c.previous.push_back(read16(ram + offset));
                c.misses.push_back(0u);
            }
        }
        c.before.clear();
        c.before.shrink_to_fit();  // 64 MiB, given back at once
        c.state = Camera::State::Watching;
        std::cout << "[analog-camera] search " << ++c.searches << ": " << c.candidates.size()
                  << " fields move by the game's own step of " << kGameStep << "\n";
        if (c.candidates.empty())
            std::cout << "[analog-camera] nothing moved by it; the camera may not have turned between the two "
                         "frames compared. Turn the camera with the stick and it will try again\n";
        return;
    }

    if (c.state == Camera::State::Watching) {
        // Keep the ones that go on behaving like the camera. Every frame, not
        // only the turning ones: the field advances a step per frame, so a
        // skipped frame makes it look as though it jumped several.
        std::vector<std::uint32_t> kept;
        std::vector<std::int16_t> kept_previous;
        std::vector<std::uint8_t> kept_misses;
        for (std::size_t i = 0; i < c.candidates.size(); ++i) {
            const std::int16_t now = read16(ram + (c.candidates[i] - base));
            const std::int16_t moved = static_cast<std::int16_t>(now - c.previous[i]);
            std::uint8_t misses = c.misses[i];
            if (moved == 0 || moved_by_steps(moved)) {
                misses = 0u;
            } else if (++misses >= 3u) {
                continue;  // three frames out of step is not the camera
            }
            kept.push_back(c.candidates[i]);
            kept_previous.push_back(now);
            kept_misses.push_back(misses);
        }
        c.candidates.swap(kept);
        c.previous.swap(kept_previous);
        c.misses.swap(kept_misses);
        if (c.candidates.empty()) {
            forget(c, "none of them kept behaving like the camera");
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
    // Did what the port wrote last frame survive? If the game puts its own
    // value back every frame, writing this field can never do anything, and
    // that is a different problem from not writing at all.
    if (c.has_wrote && c.reported < 20) {
        const int drift = static_cast<std::int16_t>(now - c.wrote);
        std::cout << "[analog-camera] wrote " << c.wrote << ", found " << now << " a frame later, difference "
                  << drift << (drift == 0 ? " (the write survived)" : " (something else wrote it)") << "\n";
        ++c.reported;
    }
    if (now != c.last) {
        if (c.strikes == 0)
            std::cout << "[analog-camera] 0x" << std::hex << c.address << std::dec << " moved on its own by "
                      << static_cast<int>(static_cast<std::int16_t>(now - c.last)) << "\n";
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
    if (c.reported < 20 && c.reported > 0)
        std::cout << "[analog-camera] speed setting reads " << speed << " deg/s, stick at " << deflection << "\n";
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
        c.has_wrote = false;
        return;
    }
    const std::int16_t written = static_cast<std::int16_t>(now + whole);
    memory.store16(c.address, static_cast<std::uint16_t>(written));
    // The field found is the camera's *target* angle; two bytes later is the
    // one the view is actually built from, which the game eases towards the
    // target by a quarter of the difference each frame. Writing that one too
    // leaves the filter nothing to do, which removes the eighteen degrees of
    // coast measured in #103 -- but it also writes a field the game is driving
    // itself, and the camera stopped responding after that went in. So it is
    // off unless asked for, and the default is the behaviour that worked.
    static const bool instant = std::getenv("MHP3RD_CAMERA_INSTANT") != nullptr;
    if (instant) memory.store16(c.address + 2u, static_cast<std::uint16_t>(written));
    c.last = written;
    c.wrote = written;
    c.has_wrote = true;
    if (!c.announced) {
        std::cout << "[analog-camera] turning the camera from the stick\n";
        c.announced = true;
    }
}

} // namespace mhp3rd::input
