#include "camera_probe.hpp"

#include "gpu/vulkan_renderer.hpp"
#include "hle/hle_common.hpp"

#include "psprecomp/guest_memory.hpp"
#include "psprecomp/runtime.hpp"

#include <array>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <fstream>
#include <iostream>
#include <vector>

namespace psprecomp {
// Defined in the core library; declared here rather than in the header so that
// arming a watch does not rebuild every generated unit and every overlay.
void set_write_watch(std::uint32_t address, std::uint32_t size);
} // namespace psprecomp

namespace mhp3rd::probe {
using mhp3rd::active_renderer;
namespace {

// A game does not have to keep its camera angle as a float: a 16-bit angle,
// where a whole turn is 65536, is just as likely, and read as a float it looks
// like noise. So every word is tried three ways.
enum class Kind : std::uint8_t { Float32, Int32, Int16 };

// Enough turning to carry information, and not a camera cut.
constexpr float kTurning = 0.3f;
constexpr float kCut = 40.0f;
// Ratios outside this band are noise rather than an angle in any unit: degrees
// give 1, radians 0.0175, a 16-bit angle 182.
constexpr double kSmallestRatio = 1e-3;
constexpr double kLargestRatio = 1e6;
// A whole-numbered word that moves by only a unit or two per frame cannot be
// told from a counter, because rounding swamps the difference. Asking that a
// full-speed turn move it by twenty units throws those out at the door instead
// of carrying them untested, which is how a thousand counters survived a
// four-hundred-frame hunt.
constexpr double kSmallestWholeRatio = 3.0;
// Only judge a candidate on a frame that turns enough to be worth judging by,
// and only start a hunt from one -- a gentle drift admits millions of words and
// then throws them all away, and each attempt costs a 64 MiB snapshot and three
// full scans. Left uncapped that alone took the game down to six frames a
// second.
constexpr float kWorthJudging = 1.5f;
// Frames a hunt waits before trying again after one comes to nothing.
constexpr int kCooldownFrames = 180;
// The console gets a summary; the whole list goes to a file, because a set
// that stops shrinking at a thousand is still small enough to read through and
// far too big to print every frame.
constexpr std::size_t kPrintable = 24u;
// Per kind, not overall: a single cap ran out during the float pass and the
// int16 pass never ran at all. Kept small on purpose -- a candidate costs
// thirty-two bytes and a read every frame, and three million of them per kind
// is most of a gigabyte and a slideshow on an eight-gigabyte machine. An
// admission worth having collapses to hundreds within a few frames anyway, so
// a kind that overflows this had nothing to say and is dropped whole rather
// than truncated.
constexpr std::size_t kMostPerKind = 400u * 1000u;
// Once a hunt is down to this few, the best of them is worth watching: the guest
// code that writes it is what the hunt was really after, and watching it in the
// same run needs no assumption that the address still means this tomorrow.
constexpr std::size_t kWorthWatching = 6u;
// A hunt that has not collapsed by now was started from a movement too weak to
// discriminate; carrying it costs a read per candidate per frame for nothing.
constexpr std::uint64_t kCollapseBy = 4u;
constexpr std::size_t kCollapsedTo = 20u * 1000u;
// One disagreeing frame is not proof. The filter's knee, where the camera goes
// from driven to coasting, moved every survivor of one hunt out of step for a
// single frame and threw all of them away.
constexpr int kStrikes = 3;

// A camera can be kept either way round: as an angle, which moves by the turn
// each frame, or as the turn itself, which *is* the rate the filter carries.
// The second is the one worth having, since scaling it is the whole point.
// A rate may also be read a frame before it shows up in the matrix. That is a
// question for the filter, which accepts either alignment, rather than for
// admission, where a third shape would let in every word in memory.
enum class Shape : std::uint8_t { Angle, Rate };

struct Candidate {
    std::uint32_t offset{};
    Kind kind{};
    Shape shape{};
    double ratio{};     // units of this word per degree of camera turn
    double previous{};
    std::uint8_t strikes{};
};

// One hunt per thing the camera can be asked about. The vertical direction is
// the harder half of #106 -- measured as one-shot commands rather than an axis
// -- so it gets the same treatment as the turn rather than an assumption.
struct Hunt {
    const char *name{};
    bool armed{};
    bool have_snapshot{};
    float snapshot_signal{};
    int attempts{};
    int cooldown{};
    bool watching{};
    std::uint64_t frames{};
    std::vector<std::uint8_t> snapshot;
    std::vector<Candidate> candidates;
};

struct Probe {
    bool started{};
    Hunt yaw{"yaw"};
    Hunt pitch{"pitch"};
    float previous_pitch{};
    bool have_pitch{};
};

Probe &probe() {
    static Probe value;
    return value;
}

double read_as(const std::uint8_t *base, std::uint32_t offset, Kind kind) {
    if (kind == Kind::Float32) {
        float value = 0.0f;
        std::memcpy(&value, base + offset, sizeof(value));
        return std::isfinite(value) ? static_cast<double>(value) : 0.0;
    }
    if (kind == Kind::Int32) {
        std::int32_t value = 0;
        std::memcpy(&value, base + offset, sizeof(value));
        return static_cast<double>(value);
    }
    std::int16_t value = 0;
    std::memcpy(&value, base + offset, sizeof(value));
    return static_cast<double>(value);
}

// A whole word of slack covers the rounding of an integer angle.
double slack_of(Kind kind, double expected) {
    return 0.03 * std::fabs(expected) + (kind == Kind::Float32 ? 1e-5 : 1.5);
}

// MHP3RD_FIND_CAMERA_OUT names a file the surviving list is rewritten into
// whenever it changes, so a run can be read without restarting the game.
void write_list(const Hunt &h, std::uint32_t base, std::ofstream &out);

const char *name_of(Kind kind) {
    return kind == Kind::Float32 ? "float" : kind == Kind::Int32 ? "int32" : "int16";
}

const char *shape_of(Shape shape) { return shape == Shape::Angle ? "angle" : "rate"; }

bool plausible(Kind kind, double ratio) {
    const double size_of = std::fabs(ratio);
    if (!std::isfinite(ratio) || size_of < kSmallestRatio || size_of > kLargestRatio) return false;
    return kind == Kind::Float32 || size_of >= kSmallestWholeRatio;
}

void write_list(const Hunt &h, std::uint32_t base, std::ofstream &out) {
    out << "# " << h.name << ": " << h.candidates.size() << " words still tracking, after " << h.frames
        << " moving frames\n";
    out << std::setprecision(10);
    for (const Candidate &c : h.candidates)
        out << h.name << " " << name_of(c.kind) << " " << shape_of(c.shape) << " 0x" << std::hex
            << (base + c.offset) << std::dec << " value=" << c.previous << " per_degree=" << c.ratio << "\n";
}

void write_lists(const Probe &p, std::uint32_t base) {
    const char *path = std::getenv("MHP3RD_FIND_CAMERA_OUT");
    if (path == nullptr || *path == '\0') return;
    std::ofstream out(path, std::ios::trunc);
    if (!out) return;
    write_list(p.yaw, base, out);
    write_list(p.pitch, base, out);
}

void admit(Hunt &p, const std::uint8_t *ram, std::uint32_t size, float before_signal, float turn) {
    const auto try_kind = [&](Kind kind, std::uint32_t stride, std::uint32_t width) {
        const std::size_t before_kind = p.candidates.size();
        const std::size_t ceiling = before_kind + kMostPerKind;
        bool overflowed = false;
        for (std::uint32_t offset = 0; offset + width <= size; offset += stride) {
            if (p.candidates.size() >= ceiling) {
                overflowed = true;
                break;
            }
            const double before = read_as(p.snapshot.data(), offset, kind);
            const double now = read_as(ram, offset, kind);
            // An angle moves by the turn; a rate simply is the turn.
            const double angle_ratio = (now - before) / static_cast<double>(turn);
            if (now != before && plausible(kind, angle_ratio))
                p.candidates.push_back({offset, kind, Shape::Angle, angle_ratio, now});
            // A rate is the movement, so it has to keep the same proportion to
            // it on both frames. Asking that at the door is what keeps this
            // from admitting every non-zero word in sixty-four megabytes.
            const double rate_ratio = now / static_cast<double>(turn);
            const double was_ratio = before / static_cast<double>(before_signal);
            if (now != 0.0 && before != 0.0 && plausible(kind, rate_ratio) &&
                std::fabs(rate_ratio - was_ratio) <= 0.05 * std::fabs(rate_ratio))
                p.candidates.push_back({offset, kind, Shape::Rate, rate_ratio, now});
        }
        if (overflowed) {
            // Too many to be a signal; keeping a truncated prefix would only
            // hide whichever kind came after it.
            p.candidates.resize(before_kind);
            std::cout << "[find-camera] " << p.name << ": too many " << name_of(kind)
                      << " words matched to mean anything; that kind is dropped for this attempt\n";
        }
    };
    try_kind(Kind::Float32, 4u, 4u);
    try_kind(Kind::Int32, 4u, 4u);
    try_kind(Kind::Int16, 2u, 2u);
}

void step(Hunt &h, const std::uint8_t *ram, std::uint32_t size, float signal, std::uint32_t base) {
    if (h.cooldown > 0) {
        --h.cooldown;
        return;
    }
    // Judging wants any real movement; starting wants enough of it to be worth
    // a scan.
    const float least = h.armed ? kTurning : kWorthJudging;
    const bool moving = std::fabs(signal) >= least && std::fabs(signal) <= kCut;
    if (!moving) {
        if (!h.armed) h.have_snapshot = false;
        return;
    }
    if (!h.armed && !h.have_snapshot) {
        h.snapshot.assign(ram, ram + size);
        h.snapshot_signal = signal;
        h.have_snapshot = true;
        return;
    }
    if (!h.armed) {
        h.candidates.clear();
        admit(h, ram, size, h.snapshot_signal, signal);
        h.armed = true;
        h.frames = 1u;
        // The snapshot has done its work; 64 MiB is worth giving back.
        h.snapshot.clear();
        h.snapshot.shrink_to_fit();
        h.have_snapshot = false;
        std::cout << "[find-camera] " << h.name << " attempt " << h.attempts << " (" << signal << " deg): "
                  << h.candidates.size() << " candidates\n";
        return;
    }

    std::vector<Candidate> kept;
    kept.reserve(h.candidates.size());
    const bool judge = std::fabs(signal) >= kWorthJudging;
    for (Candidate c : h.candidates) {
        const double now = read_as(ram, c.offset, c.kind);
        if (!judge) {
            c.previous = now;  // too gentle a move to tell anything from
            kept.push_back(c);
            continue;
        }
        const double expected = c.ratio * static_cast<double>(signal);
        // A rate may be read a frame before it reaches the matrix, so either
        // alignment counts as agreement.
        const bool agrees =
            c.shape == Shape::Angle
                ? std::fabs((now - c.previous) - expected) <= slack_of(c.kind, expected)
                : (std::fabs(now - expected) <= slack_of(c.kind, expected) ||
                   std::fabs(c.previous - expected) <= slack_of(c.kind, expected));
        if (!agrees) {
            if (++c.strikes >= kStrikes) continue;
        } else if (c.strikes != 0u) {
            --c.strikes;  // it came back into step, so forgive the earlier frame
        }
        c.previous = now;
        kept.push_back(c);
    }
    const bool thinned = kept.size() != h.candidates.size();
    h.candidates.swap(kept);
    h.candidates.shrink_to_fit();
    ++h.frames;
    if (h.frames >= kCollapseBy && h.candidates.size() > kCollapsedTo) {
        std::cout << "[find-camera] " << h.name << ": " << h.candidates.size() << " still standing after "
                  << h.frames << " frames, so that movement could not tell them apart; starting over\n";
        h.candidates.clear();
        h.candidates.shrink_to_fit();
    }

    // The point of narrowing is to get somewhere worth watching.
    if (!h.watching && !h.candidates.empty() && h.candidates.size() <= kWorthWatching) {
        const Candidate &best = h.candidates.front();
        std::cout << "[find-camera] " << h.name << ": watching guest writes to 0x" << std::hex
                  << (base + best.offset) << std::dec << " (" << name_of(best.kind) << " "
                  << shape_of(best.shape) << ")\n";
        psprecomp::set_write_watch(base + best.offset, best.kind == Kind::Int16 ? 2u : 4u);
        h.watching = true;
    }
    if (thinned) {
        std::cout << "[find-camera] " << h.name << " after " << h.frames << " moving frames (" << signal
                  << " deg): " << h.candidates.size() << " left\n";
        const std::streamsize precision = std::cout.precision();
        std::cout << std::setprecision(8);
        if (!h.candidates.empty() && h.candidates.size() <= kPrintable)
            for (const Candidate &c : h.candidates)
                std::cout << "[find-camera]   " << h.name << " " << name_of(c.kind) << " " << shape_of(c.shape)
                          << " at 0x" << std::hex << (base + c.offset) << std::dec << " value=" << c.previous
                          << " per-degree=" << c.ratio << "\n";
        std::cout.precision(precision);
    }
    if (h.candidates.empty() && h.attempts < 200) {
        h.armed = false;
        h.have_snapshot = false;
        h.cooldown = kCooldownFrames;
        ++h.attempts;
        std::cout << "[find-camera] " << h.name << ": that one said nothing; waiting for another\n";
    }
}

} // namespace

void camera_frame(psprecomp::Runtime &runtime, std::uint32_t view_matrix_source) {
    static const bool enabled = std::getenv("MHP3RD_FIND_CAMERA") != nullptr;
    if (!enabled) return;
#if defined(MHP3RD_HAS_RENDERER)
    (void)view_matrix_source;
    const gpu::VulkanRenderer *renderer = active_renderer();
    if (renderer == nullptr || !renderer->available()) return;
    const gpu::CameraReading reading = renderer->camera();
    if (!reading.valid) return;

    psprecomp::GuestMemory &memory = runtime.memory();
    const std::uint32_t base = psprecomp::GuestMemory::kPhysicalBase;
    const std::uint32_t size = memory.size();
    const std::uint8_t *ram = memory.raw_pointer(base, size);
    if (ram == nullptr) return;

    Probe &p = probe();
    if (!p.started) {
        std::cout << "[find-camera] watching " << (size / (1024u * 1024u)) << " MiB of guest RAM from 0x"
                  << std::hex << base << std::dec << ", as float, int32 and int16\n";
        p.started = true;
    }

    // Both halves of the camera get the same treatment, in one visit to a
    // quest, because getting into one is the expensive part.
    const float pitch_change = p.have_pitch ? reading.pitch - p.previous_pitch : 0.0f;
    p.previous_pitch = reading.pitch;
    p.have_pitch = true;

    step(p.yaw, ram, size, reading.turn, base);
    step(p.pitch, ram, size, pitch_change, base);
    if (p.yaw.frames % 200u == 0u || p.pitch.frames % 200u == 0u) write_lists(p, base);

#else
    (void)runtime;
    (void)view_matrix_source;
#endif
}

} // namespace mhp3rd::probe
