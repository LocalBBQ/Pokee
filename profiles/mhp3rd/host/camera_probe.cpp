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
// Only judge a candidate on a frame that turns enough to be worth judging by.
constexpr float kWorthJudging = 1.5f;
// The console gets a summary; the whole list goes to a file, because a set
// that stops shrinking at a thousand is still small enough to read through and
// far too big to print every frame.
constexpr std::size_t kPrintable = 24u;
// Per kind, not overall: a single cap ran out during the float pass and the
// int16 pass never ran at all.
constexpr std::size_t kMostPerKind = 3u * 1000u * 1000u;
// One disagreeing frame is not proof. The filter's knee, where the camera goes
// from driven to coasting, moved every survivor of one hunt out of step for a
// single frame and threw all of them away.
constexpr int kStrikes = 3;

// A camera can be kept either way round: as an angle, which moves by the turn
// each frame, or as the turn itself, which *is* the rate the filter carries.
// The second is the one worth having, since scaling it is the whole point.
// A rate may also be read a frame before it shows up in the matrix, so it is
// tried both aligned with this frame's turn and with the next one's.
enum class Shape : std::uint8_t { Angle, Rate, RateLead };

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
    int attempts{};
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

const char *shape_of(Shape shape) {
    return shape == Shape::Angle ? "angle" : shape == Shape::Rate ? "rate" : "rate-lead";
}

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

void admit(Hunt &p, const std::uint8_t *ram, std::uint32_t size, float turn) {
    const auto try_kind = [&](Kind kind, std::uint32_t stride, std::uint32_t width) {
        const std::size_t ceiling = p.candidates.size() + kMostPerKind;
        for (std::uint32_t offset = 0; offset + width <= size; offset += stride) {
            if (p.candidates.size() >= ceiling) return;
            const double before = read_as(p.snapshot.data(), offset, kind);
            const double now = read_as(ram, offset, kind);
            // An angle moves by the turn; a rate simply is the turn.
            const double angle_ratio = (now - before) / static_cast<double>(turn);
            if (now != before && plausible(kind, angle_ratio))
                p.candidates.push_back({offset, kind, Shape::Angle, angle_ratio, now});
            const double rate_ratio = now / static_cast<double>(turn);
            if (now != 0.0 && plausible(kind, rate_ratio))
                p.candidates.push_back({offset, kind, Shape::Rate, rate_ratio, now});
            const double lead_ratio = before / static_cast<double>(turn);
            if (before != 0.0 && plausible(kind, lead_ratio))
                p.candidates.push_back({offset, kind, Shape::RateLead, lead_ratio, now});
        }
    };
    try_kind(Kind::Float32, 4u, 4u);
    try_kind(Kind::Int32, 4u, 4u);
    try_kind(Kind::Int16, 2u, 2u);
}

void step(Hunt &h, const std::uint8_t *ram, std::uint32_t size, float signal, std::uint32_t base) {
    const bool moving = std::fabs(signal) >= kTurning && std::fabs(signal) <= kCut;
    if (!moving) {
        if (!h.armed) h.have_snapshot = false;
        return;
    }
    if (!h.armed && !h.have_snapshot) {
        h.snapshot.assign(ram, ram + size);
        h.have_snapshot = true;
        return;
    }
    if (!h.armed) {
        h.candidates.clear();
        admit(h, ram, size, signal);
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
        const double seen = c.shape == Shape::Angle    ? now - c.previous
                            : c.shape == Shape::Rate   ? now
                                                       : c.previous;
        if (std::fabs(seen - expected) > slack_of(c.kind, expected)) {
            if (++c.strikes >= kStrikes) continue;
        } else if (c.strikes != 0u) {
            --c.strikes;  // it came back into step, so forgive the earlier frame
        }
        c.previous = now;
        kept.push_back(c);
    }
    const bool thinned = kept.size() != h.candidates.size();
    h.candidates.swap(kept);
    ++h.frames;

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
