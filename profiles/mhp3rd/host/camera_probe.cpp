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
// The console gets a summary; the whole list goes to a file, because a set
// that stops shrinking at a thousand is still small enough to read through and
// far too big to print every frame.
constexpr std::size_t kPrintable = 24u;
constexpr std::size_t kMostCandidates = 3u * 1000u * 1000u;

struct Candidate {
    std::uint32_t offset{};
    Kind kind{};
    double ratio{};     // units of this word per degree of camera turn
    double previous{};
};

struct Probe {
    bool started{};
    bool armed{};
    bool have_snapshot{};
    int attempts{};
    std::uint64_t frames{};
    std::vector<std::uint8_t> snapshot;
    std::vector<Candidate> candidates;
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

// Below this a change carries no information and the frame is skipped rather
// than counted for or against the candidate; a whole word of slack covers the
// rounding of an integer angle.
double floor_of(Kind kind) { return kind == Kind::Float32 ? 1e-4 : 2.0; }
double slack_of(Kind kind, double expected) {
    return 0.03 * std::fabs(expected) + (kind == Kind::Float32 ? 1e-5 : 1.5);
}

// MHP3RD_FIND_CAMERA_OUT names a file the surviving list is rewritten into
// whenever it changes, so a run can be read without restarting the game.
void write_list(const Probe &p, std::uint32_t base);

const char *name_of(Kind kind) {
    return kind == Kind::Float32 ? "float" : kind == Kind::Int32 ? "int32" : "int16";
}

void write_list(const Probe &p, std::uint32_t base) {
    const char *path = std::getenv("MHP3RD_FIND_CAMERA_OUT");
    if (path == nullptr || *path == '\0') return;
    std::ofstream out(path, std::ios::trunc);
    if (!out) return;
    out << "# " << p.candidates.size() << " words still tracking the camera after " << p.frames
        << " turning frames\n";
    out << std::setprecision(10);
    for (const Candidate &c : p.candidates)
        out << name_of(c.kind) << " 0x" << std::hex << (base + c.offset) << std::dec << " value=" << c.previous
            << " per_degree=" << c.ratio << "\n";
}

void admit(Probe &p, const std::uint8_t *ram, std::uint32_t size, float turn) {
    const auto try_kind = [&](Kind kind, std::uint32_t stride, std::uint32_t width) {
        for (std::uint32_t offset = 0; offset + width <= size; offset += stride) {
            if (p.candidates.size() >= kMostCandidates) return;
            const double before = read_as(p.snapshot.data(), offset, kind);
            const double now = read_as(ram, offset, kind);
            if (now == before) continue;  // a word that did not move says nothing
            const double ratio = (now - before) / static_cast<double>(turn);
            const double size_of = std::fabs(ratio);
            if (!std::isfinite(ratio) || size_of < kSmallestRatio || size_of > kLargestRatio) continue;
            p.candidates.push_back({offset, kind, ratio, now});
        }
    };
    try_kind(Kind::Float32, 4u, 4u);
    try_kind(Kind::Int32, 4u, 4u);
    try_kind(Kind::Int16, 2u, 2u);
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

    const float turn = reading.turn;
    const bool turning = std::fabs(turn) >= kTurning && std::fabs(turn) <= kCut;
    if (!turning) {
        if (!p.armed) p.have_snapshot = false;
        return;
    }

    if (!p.armed && !p.have_snapshot) {
        p.snapshot.assign(ram, ram + size);
        p.have_snapshot = true;
        return;
    }
    if (!p.armed) {
        p.candidates.clear();
        admit(p, ram, size, turn);
        p.armed = true;
        p.frames = 1u;
        std::cout << "[find-camera] attempt " << p.attempts << " (turn " << turn << "): "
                  << p.candidates.size() << " candidates\n";
        return;
    }

    std::vector<Candidate> kept;
    kept.reserve(p.candidates.size());
    for (Candidate c : p.candidates) {
        const double now = read_as(ram, c.offset, c.kind);
        const double expected = c.ratio * static_cast<double>(turn);
        if (std::fabs(expected) < floor_of(c.kind)) {
            // Too small a turn to judge this word by; carry it unchanged.
            c.previous = now;
            kept.push_back(c);
            continue;
        }
        if (std::fabs((now - c.previous) - expected) > slack_of(c.kind, expected)) continue;
        c.previous = now;
        kept.push_back(c);
    }
    const bool thinned = kept.size() != p.candidates.size();
    p.candidates.swap(kept);
    ++p.frames;

    if (thinned || p.frames % 200u == 0u) {
        std::cout << "[find-camera] after " << p.frames << " turning frames (turn " << turn << "): "
                  << p.candidates.size() << " left\n";
        const std::streamsize precision = std::cout.precision();
        std::cout << std::setprecision(8);
        if (!p.candidates.empty() && p.candidates.size() <= kPrintable)
            for (const Candidate &c : p.candidates)
                std::cout << "[find-camera]   " << name_of(c.kind) << " at 0x" << std::hex << (base + c.offset)
                          << std::dec << " value=" << c.previous << " per-degree=" << c.ratio << "\n";
        std::cout.precision(precision);
        write_list(p, base);
    }
    if (p.candidates.empty() && p.attempts < 12) {
        p.armed = false;
        p.have_snapshot = false;
        ++p.attempts;
        std::cout << "[find-camera] that turn said nothing; waiting for another\n";
    }
#else
    (void)runtime;
    (void)view_matrix_source;
#endif
}

} // namespace mhp3rd::probe
