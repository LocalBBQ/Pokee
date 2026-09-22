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
constexpr int kStrikes = 12;  // only used for how often the log mentions it

struct Camera {
    enum class State { Looking, Watching, Locked } state{State::Looking};
    std::vector<std::uint8_t> misses;   // frames each candidate has disagreed
    std::vector<std::int16_t> offsets;  // how far each sits from the view's yaw
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
    std::vector<std::uint32_t> rejected;  // fields that turned out to follow, not drive
    int overwritten{};         // frames the game put its own value back
    bool drive_view{};         // and so the view's own angle is driven as well
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
    std::vector<std::uint8_t> held;   // the level each candidate last showed
    int watched{};                    // frames of level-against-pitch still to print
    std::size_t named{};              // how many were last printed
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

void look_for_vertical(const std::uint8_t *ram, std::uint32_t base, std::uint32_t size, float pitch_change,
                       float stick_y) {
    static const bool wanted = std::getenv("MHP3RD_FIND_VERTICAL") != nullptr;
    if (!wanted) return;
    Vertical &v = vertical();
    if (v.reported) return;

    // The pitch wobbles for all sorts of reasons -- the ground, a turn, the
    // hunter moving. What marks the vertical command is a jump *while the stick
    // is pushed past the threshold the game answers*. Without that second
    // condition the search admitted noise and narrowed noise against noise.
    const bool fired = std::fabs(pitch_change) >= kVerticalFired && std::fabs(stick_y) >= 0.6f;
    if (!v.searching) {
        if (!fired) {
            if (v.before.empty()) v.before.assign(ram, ram + size);
            return;
        }
        if (v.before.empty()) return;
        // A byte holding one of five values is most of memory. What is not is a
        // byte that holds one of five values, changes on the frame the vertical
        // fires, and holds still in between -- so both halves are asked for,
        // the first here and the second on every quiet frame after.
        for (std::uint32_t offset = 0; offset < size; ++offset) {
            const std::uint8_t now = ram[offset];
            const std::uint8_t was = v.before[offset];
            if (now != was && now <= 4u && was <= 4u) v.levels.push_back(base + offset);
        }
        v.searching = true;
        v.firings = 1;
        v.held.assign(v.levels.size(), 0u);
        for (std::size_t i = 0; i < v.levels.size(); ++i) v.held[i] = ram[v.levels[i] - base];
        std::cout << "[analog-camera] vertical: " << v.levels.size()
                  << " bytes hold 0 to 4 and changed when it fired\n";
        v.before.clear();
        v.before.shrink_to_fit();
        return;
    }

    std::vector<std::uint32_t> kept;
    std::vector<std::uint8_t> kept_held;
    for (std::size_t i = 0; i < v.levels.size(); ++i) {
        const std::uint8_t now = ram[v.levels[i] - base];
        if (now > 4u) continue;                       // never a level
        if (!fired && now != v.held[i]) continue;     // a level does not drift
        if (fired && now == v.held[i] && v.firings > 1) continue;  // and it does step when told
        kept.push_back(v.levels[i]);
        kept_held.push_back(now);
    }
    const std::size_t was = v.levels.size();
    v.levels.swap(kept);
    v.held.swap(kept_held);
    if (fired) ++v.firings;
    if (v.levels.size() != was || fired)
        std::cout << "[analog-camera] vertical: " << v.levels.size() << " left after " << v.firings
                  << " firings\n";
    // Name them whenever the set is small, but keep narrowing: another firing
    // may take two to one, and stopping at the first small answer is how a
    // search gets believed too early.
    if (!v.levels.empty() && v.levels.size() <= 8u && v.firings >= 3 && v.levels.size() != v.named) {
        std::cout << "[analog-camera] vertical level byte candidates:";
        for (std::uint32_t address : v.levels) std::cout << " 0x" << std::hex << address << std::dec;
        std::cout << "\n";
        v.named = v.levels.size();
        if (v.levels.size() == 1u) v.reported = true;
    }
    if (v.levels.empty()) {
        std::cout << "[analog-camera] vertical: none of them stepped when told; looking again\n";
        v = Vertical{};
    }
}

} // namespace

bool analog_camera_driving() {
    return settings::current().analog_camera && camera().state == Camera::State::Locked;
}

// The view's own yaw, in the game's units, so a candidate can be checked
// against the camera the player is actually looking through rather than merely
// against the step. Several angles in this game move by that step -- companions
// have cameras too -- and picking the wrong one drives something invisible while
// the player's camera carries on exactly as the game left it.
std::int16_t to_units(float degrees) {
    float wrapped = std::fmod(degrees, 360.0f);
    if (wrapped < 0.0f) wrapped += 360.0f;
    return static_cast<std::int16_t>(static_cast<int>(wrapped * kUnitsPerTurn / 360.0f) & 0xFFFF);
}

// Once the level bytes are named, the value the glide moves should be beside
// one of them, and the game is said to keep it as roughly the angle times ten.
// Rather than guess at the convention, print each candidate's level next to the
// pitch the view actually has, and the 16-bit words around it, so the mapping
// can be read off instead of assumed.
// Before anything can be found by "it changed when the vertical fired", the
// firing itself has to be visible. The first search converged on bytes that
// differed completely between runs and held values far outside 0 to 4, while
// the pitch moved by hundredths of a degree throughout -- which says the
// vertical was never seen to fire at all, and the search was comparing noise.
// So log what the vertical stick is doing beside what the pitch does, and
// settle that first.
void watch_vertical(float pitch, float stick_y) {
    static int printed = 0;
    static float last_pitch = 0.0f;
    static bool seen_push = false;
    if (std::fabs(stick_y) > 0.3f) seen_push = true;
    if (!seen_push || printed >= 80) return;
    // Only when something happens: a long hold otherwise fills the whole window
    // with one settled value and hides the pushes that follow it.
    if (std::fabs(pitch - last_pitch) < 0.05f) return;
    last_pitch = pitch;
    ++printed;
    std::cout << "[analog-camera] vertical stick " << stick_y << " -> pitch " << pitch << " deg\n";
}

// Drives the camera's vertical, if the player has asked for it and the search
// has named something to drive. Only bytes that stepped when the vertical fired
// and held still in between are ever written, and only ever to one of the five
// values the game itself uses, so the worst a wrong guess can do is move some
// other object's camera -- or nothing at all.
// The pitch, found the way the yaw was found rather than by matching a model
// borrowed from another release. The yaw was certain the moment a candidate had
// to keep a fixed offset from the angle the view is actually built with; the
// pitch is an angle of that same view, in the same 16-bit units, so the same
// test applies -- and it needs no command to fire and no five levels to exist.
struct Pitch {
    std::vector<std::uint8_t> before;
    std::vector<std::uint32_t> fields;
    std::vector<std::int16_t> offsets;
    std::vector<std::uint8_t> misses;
    bool started{};
    std::size_t named{};
    float last{};
    // The survivors, kept when the set empties: a command firing moves the
    // pitch faster than the field follows and drops everything at once, and
    // the answer was in the set just before that.
    std::vector<std::uint32_t> best;
    // Proving it without a player: nudge one candidate while the stick is
    // untouched and see whether the view's pitch follows.
    std::size_t trying{};
    int settle{};
    float before_pitch{};
    std::int16_t nudged{};
    bool waiting{};
    std::uint32_t confirmed{};
    int attempts{};
    std::vector<std::uint32_t> rejected;  // fields that followed rather than moved
    std::int16_t driven{};
    int ignored{};
    int reported_drive{};
    float last_seen{};
    bool tried_derived{};
};

Pitch &pitch_search() {
    static Pitch value;
    return value;
}

void look_for_pitch(const std::uint8_t *ram, std::uint32_t base, std::uint32_t size, float pitch) {
    if (!settings::current().vertical_camera) return;
    Pitch &p = pitch_search();
    // Only worth judging while the pitch is really changing. A twentieth of a
    // degree is noise: it eliminates almost nothing and leaves the whole set to
    // be walked again next frame.
    const bool moving = std::fabs(pitch - p.last) >= 1.0f;
    p.last = pitch;
    if (!moving) return;
    const std::int16_t units = to_units(pitch);

    if (!p.started) {
        for (std::uint32_t offset = 0; offset + 2u <= size; offset += 2u) {
            std::int16_t value = 0;
            std::memcpy(&value, ram + offset, sizeof(value));
            // Admitting every field in sixty-four megabytes means thirty-three
            // million candidates, three hundred megabytes of bookkeeping and a
            // pass over all of it every frame -- which is why the search sat at
            // 33,525,664 and the vertical never got as far as confirming
            // anything. The camera's own angle sits within a degree or two of
            // the view's, as both of the fields found earlier did at +41 and
            // +440, so anything further away than ten degrees is not it.
            const std::int16_t apart = static_cast<std::int16_t>(units - value);
            if (apart > 1800 || apart < -1800) continue;
            bool refused = false;
            for (std::uint32_t bad : p.rejected)
                if (bad == base + offset) refused = true;
            if (refused) continue;
            p.fields.push_back(base + offset);
            p.offsets.push_back(apart);
            p.misses.push_back(0u);
        }
        p.started = true;
        std::cout << "[analog-camera] pitch: watching every 16-bit field for one that keeps step with the view\n";
        return;
    }
    std::vector<std::uint32_t> kept;
    std::vector<std::int16_t> kept_offsets;
    std::vector<std::uint8_t> kept_misses;
    for (std::size_t i = 0; i < p.fields.size(); ++i) {
        std::int16_t value = 0;
        std::memcpy(&value, ram + (p.fields[i] - base), sizeof(value));
        const std::int16_t offset = static_cast<std::int16_t>(units - value);
        const int wander = static_cast<std::int16_t>(offset - p.offsets[i]);
        std::uint8_t misses = p.misses[i];
        if (wander > 400 || wander < -400) {   // about two degrees
            if (++misses >= 3u) continue;
        } else {
            misses = 0u;
        }
        kept.push_back(p.fields[i]);
        kept_offsets.push_back(p.offsets[i]);
        kept_misses.push_back(misses);
    }
    p.fields.swap(kept);
    p.offsets.swap(kept_offsets);
    p.misses.swap(kept_misses);
    if (!p.fields.empty() && p.fields.size() <= 8u) p.best = p.fields;
    if (p.fields.size() != p.named) {
        p.named = p.fields.size();
        std::cout << "[analog-camera] pitch: " << p.fields.size() << " fields still keeping step";
        if (p.fields.size() <= 12u)
            for (std::size_t i = 0; i < p.fields.size(); ++i)
                std::cout << " 0x" << std::hex << p.fields[i] << std::dec << "(+" << p.offsets[i] << ")";
        std::cout << "\n";
    }
}

// Writes a couple of degrees into one candidate while nothing else is asking
// for the camera, and watches whether the view's pitch answers. The one that
// answers is the pitch; the others are whatever else keeps step with it.
// Never write where the horizontal lives. The pitch candidates are fields that
// keep step with the view, and the yaw's own fields do exactly that whenever
// the camera moves in both directions at once -- so the vertical's self-test
// was writing five degrees into the horizontal's angle, which is what has been
// taking it down. Addresses anywhere near the yaw's are refused outright.
bool clashes_with_horizontal(std::uint32_t address) {
    const Camera &c = camera();
    if (c.state != Camera::State::Locked) return false;
    const std::uint32_t apart = address > c.address ? address - c.address : c.address - address;
    return apart < 0x2000u;
}

void confirm_pitch(psprecomp::GuestMemory &memory, float pitch, float stick_y, float turn) {
    // This *writes* five degrees into a candidate to see whether the view
    // answers, and most candidates are not the camera. It was gated only on the
    // search being switched on, so turning Vertical camera off did not stop it
    // -- which is why a claim that the vertical could not touch the horizontal
    // turned out to be false, four times over. The switch now governs every
    // write the vertical makes, including this one.
    if (!settings::current().vertical_camera) return;
    // Wait for the horizontal to know where it lives. Until it does there is
    // nothing to keep clear of, and the self-test would be free to write five
    // degrees into the yaw's own angle before anyone could stop it.
    if (camera().state != Camera::State::Locked) return;
    Pitch &p = pitch_search();
    if (p.confirmed != 0u || p.best.empty()) return;
    // And it gives up rather than cycling for ever, writing into one field
    // after another that was never the camera.
    if (p.attempts > 24) return;
    // Only while the player and the game are both leaving the camera alone.
    if (std::fabs(stick_y) > 0.1f || std::fabs(turn) > 0.2f) return;

    if (p.waiting) {
        const float moved = pitch - p.before_pitch;
        if (std::fabs(moved) > 1.0f) {
            p.confirmed = p.best[p.trying];
            std::cout << "[analog-camera] pitch confirmed at 0x" << std::hex << p.confirmed << std::dec
                      << ": writing it moved the view by " << moved << " degrees\n";
        } else if (++p.settle > 4) {
            std::cout << "[analog-camera] 0x" << std::hex << p.best[p.trying] << std::dec
                      << " is not the pitch: writing it moved the view by " << moved << "\n";
            p.trying = (p.trying + 1u) % p.best.size();
            p.waiting = false;
            p.settle = 0;
            if (++p.attempts > 24)
                std::cout << "[analog-camera] none of the candidates moved the view; giving up rather than "
                             "writing into more of them\n";
        }
        return;
    }
    const std::uint32_t address = p.best[p.trying];
    if (clashes_with_horizontal(address)) {
        p.trying = (p.trying + 1u) % p.best.size();
        return;
    }
    const std::int16_t now = static_cast<std::int16_t>(memory.load16(address));
    p.before_pitch = pitch;
    p.nudged = static_cast<std::int16_t>(now + 900);  // about five degrees
    memory.store16(address, static_cast<std::uint16_t>(p.nudged));
    p.waiting = true;
    p.settle = 0;
}

// The vertical axis, on the field the self-test proved: writing 0x8ABDFB0-style
// fields moves the view, so the port can put the player's own angle there. The
// game eases the view towards it, which is what keeps the camera feeling like
// this game rather than a twin-stick shooter.
float &vertical_owed() {
    static float value = 0.0f;
    return value;
}

void drive_vertical(psprecomp::GuestMemory &memory, float stick_y, float pitch_seen) {
    const settings::Settings &player = settings::current();
    if (!player.analog_camera || !player.vertical_camera) return;
    Pitch &p = pitch_search();
    if (p.confirmed == 0u) return;
    // Below the dead zone the game keeps the camera, so its own behaviour --
    // the recentre included -- is left alone.
    if (std::fabs(stick_y) < 0.15f) {
        vertical_owed() = 0.0f;
        return;
    }
    const float speed = player.camera_speed;
    float per_frame = stick_y * speed / kFramesPerSecond;
    if (player.invert_camera_y) per_frame = -per_frame;
    float &owed = vertical_owed();
    owed += per_frame * kUnitsPerTurn / 360.0f;
    const int whole = static_cast<int>(owed);
    owed -= static_cast<float>(whole);
    if (whole == 0) return;

    // Keep the camera out of the floor and off the ceiling.
    const int limit = static_cast<int>(60.0f * kUnitsPerTurn / 360.0f);
    // The field the self-test proved drives the view, and -- if the player
    // asks for it -- its nearest companion in the same structure, in case the
    // game eases one towards the other the way it does for the yaw. Never the
    // whole set of fields that merely track the view: that is what stopped the
    // camera altogether.
    const auto turn_field = [&](std::uint32_t address) {
        if (clashes_with_horizontal(address)) return;
        const std::int16_t now = static_cast<std::int16_t>(memory.load16(address));
        int next = now + whole;
        if (next > limit) next = limit;
        if (next < -limit) next = -limit;
        memory.store16(address, static_cast<std::uint16_t>(static_cast<std::int16_t>(next)));
    };
    // A field that keeps step with the view need not be the one that moves it,
    // and writing a follower looks exactly like the vertical not working. The
    // yaw learned this the hard way tonight; the pitch gets it for free.
    if (p.driven != 0u) {
        const std::int16_t back = static_cast<std::int16_t>(memory.load16(p.confirmed));
        if (back != p.driven) {
            if (++p.ignored >= 40) {
                std::cout << "[analog-camera] 0x" << std::hex << p.confirmed << std::dec
                          << " follows the view rather than moving it; rejecting it and looking again\n";
                p.rejected.push_back(p.confirmed);
                p.confirmed = 0u;
                p.best.clear();
                p.started = false;
                p.fields.clear();
                p.offsets.clear();
                p.misses.clear();
                p.ignored = 0;
                p.driven = 0;
                return;
            }
        } else {
            p.ignored = 0;
        }
    }
    turn_field(p.confirmed);
    // Only the field proved to move the view. Writing the neighbour two bytes
    // on as well -- the yaw's own pairing -- broke the camera outright, so
    // whatever sits there is state the game needs and it is not on offer.
    p.driven = static_cast<std::int16_t>(memory.load16(p.confirmed));
    static bool said = false;
    if (!said) {
        said = true;
        std::cout << "[analog-camera] driving up and down at 0x" << std::hex << p.confirmed << std::dec << "\n";
    }
    // Turn "maybe it twitched" into a number, with no extra writes: how far the
    // port asked the camera to move this frame, against how far the view
    // actually moved.
    if (p.reported_drive < 20) {
        ++p.reported_drive;
        const float asked = static_cast<float>(whole) * 360.0f / kUnitsPerTurn;
        std::cout << "[analog-camera] vertical: asked for " << asked << " deg, the view moved "
                  << (pitch_seen - p.last_seen) << " deg\n";
    }
    p.last_seen = pitch_seen;
}

bool analog_camera_vertical_driving() {
    const settings::Settings &player = settings::current();
    return player.analog_camera && player.vertical_camera && pitch_search().confirmed != 0u;
}

void analog_camera_frame(psprecomp::Runtime &runtime, float turn, float deflection, float pitch_change,
                         float yaw_degrees, float pitch_now, float stick_y) {
    Camera &c = camera();
    // Said once, so a window running an older binary than intended says so
    // rather than leaving everyone to wonder why a setting is missing.
    static bool announced_settings = false;
    if (!announced_settings) {
        announced_settings = true;
        const settings::Settings &player = settings::current();
        std::cout << "[analog-camera] built " << __DATE__ << " " << __TIME__ << "; analog camera "
                  << (player.analog_camera ? "on" : "off") << ", vertical "
                  << (player.vertical_camera ? "on" : "off") << ", speed " << player.camera_speed << " deg/s\n";
    }
    if (!settings::current().analog_camera) {
        if (c.state != Camera::State::Looking) c = Camera{};
        return;
    }

    psprecomp::GuestMemory &memory = runtime.memory();
    const std::uint32_t base = psprecomp::GuestMemory::kPhysicalBase;
    const std::uint32_t size = memory.size();
    const std::uint8_t *ram = memory.raw_pointer(base, size);
    if (ram == nullptr) return;

    look_for_vertical(ram, base, size, pitch_change, stick_y);
    look_for_pitch(ram, base, size, pitch_now);
    confirm_pitch(memory, pitch_now, stick_y, turn);
    drive_vertical(memory, stick_y, pitch_now);
    watch_vertical(pitch_now, stick_y);

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
                bool refused = false;
                for (std::uint32_t bad : c.rejected)
                    if (bad == base + offset) refused = true;
                if (refused) continue;
                c.candidates.push_back(base + offset);
                c.previous.push_back(read16(ram + offset));
                c.misses.push_back(0u);
                c.offsets.push_back(0);
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
        std::vector<std::int16_t> kept_offsets;
        for (std::size_t i = 0; i < c.candidates.size(); ++i) {
            const std::int16_t now = read16(ram + (c.candidates[i] - base));
            const std::int16_t moved = static_cast<std::int16_t>(now - c.previous[i]);
            // Whatever this field is, the angle it holds must keep a fixed
            // offset from the yaw the view is built with. A field that steps
            // like the camera but drifts away from where the player is looking
            // belongs to some other camera.
            const std::int16_t offset = static_cast<std::int16_t>(to_units(yaw_degrees) - now);
            if (c.offsets.size() <= i) c.offsets.push_back(offset);
            const int wander = static_cast<std::int16_t>(offset - c.offsets[i]);
            const int slack = 1150 * 3;  // three of the game's own steps
            std::uint8_t misses = c.misses[i];
            if (wander > slack || wander < -slack) {
                if (++misses >= 3u) continue;
            }
            // While the camera is turning the yaw *must* move: a field that
            // sits still through a turn is not it. Accepting "did not move" on
            // every frame is why seventy-eight candidates never narrowed and
            // the camera was never driven at all -- every still word in memory
            // satisfied it for ever.
            const bool turning = std::fabs(turn) >= kSearching;
            const bool behaved = turning ? moved_by_steps(moved) : (moved == 0 || moved_by_steps(moved));
            if (behaved) {
                misses = 0u;
            } else if (++misses >= 3u) {
                continue;
            }
            kept.push_back(c.candidates[i]);
            kept_previous.push_back(now);
            kept_misses.push_back(misses);
            kept_offsets.push_back(c.offsets[i]);
        }
        const std::size_t was = c.candidates.size();
        c.candidates.swap(kept);
        c.previous.swap(kept_previous);
        c.misses.swap(kept_misses);
        c.offsets.swap(kept_offsets);
        if (c.candidates.size() != was)
            std::cout << "[analog-camera] narrowing: " << c.candidates.size() << " left\n";
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
    // If the game keeps putting its own value back, writing the target can
    // never move the camera. The angle the view is built from is two bytes on,
    // and the game only eases that towards the target -- so drive it as well,
    // and only while the player is actually turning, so the game keeps it the
    // rest of the time.
    if (c.has_wrote) {
        if (static_cast<std::int16_t>(now - c.wrote) != 0) {
            ++c.overwritten;
            if (c.overwritten == 5 && !c.drive_view) {
                c.drive_view = true;
                std::cout << "[analog-camera] the game rewrites the target every frame, so the port will drive "
                             "the angle the view is built from too, while the stick is deflected\n";
            } else if (c.overwritten >= 40) {
                // Writing it changes nothing, whichever of the pair is written:
                // this field follows the camera rather than driving it. Several
                // fields keep a fixed offset from the view and only one of them
                // is the camera, so reject this one and look again rather than
                // driving something the game overwrites every frame -- which
                // looks exactly like the analog camera not working at all.
                std::vector<std::uint32_t> rejected = c.rejected;
                rejected.push_back(c.address);
                std::cout << "[analog-camera] 0x" << std::hex << c.address << std::dec
                          << " follows the camera rather than driving it; rejecting it and looking again\n";
                c = Camera{};
                c.rejected = rejected;
                return;
            }
        } else {
            c.overwritten = 0;
        }
    }
    if (now != c.last) {
        if ((c.strikes % kStrikes) == 0)
            std::cout << "[analog-camera] 0x" << std::hex << c.address << std::dec << " moved on its own by "
                      << static_cast<int>(static_cast<std::int16_t>(now - c.last))
                      << "; adding the stick on top of it\n";
        // Something other than this code moved it, which is entirely normal:
        // the game still owns the camera in cutscenes and conversations, the
        // vertical commands swing it, and the recentre snaps it. None of that
        // is a reason to stop -- the port simply adds the player's turn to
        // whatever is there now, so the game's own movement is kept and the
        // stick is answered on top of it. Losing the lock over this is what
        // made a working camera stop working as soon as the player did
        // something ordinary.
        ++c.strikes;
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
    if (instant || c.drive_view) memory.store16(c.address + 2u, static_cast<std::uint16_t>(written));
    c.last = written;
    c.wrote = written;
    c.has_wrote = true;
    if (!c.announced) {
        std::cout << "[analog-camera] turning the camera from the stick\n";
        c.announced = true;
    }
}

} // namespace mhp3rd::input
