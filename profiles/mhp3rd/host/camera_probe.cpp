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
#include <iostream>
#include <vector>

namespace mhp3rd::probe {
using mhp3rd::active_renderer;
namespace {

// The GE expands the game's twelve uploaded floats into a 4x4 whose other four
// entries it fills in itself, so only these twelve came from the game and only
// they are worth matching.
constexpr int kMatrixEntries[12] = {0, 1, 2, 4, 5, 6, 8, 9, 10, 12, 13, 14};
// The GE's float24 is the top 24 bits of an IEEE float, so a value read back
// out of a display list has its low 8 mantissa bits cleared. Comparing those
// top 24 bits is exact, and far tighter than any tolerance.
constexpr std::uint32_t kFloat24Mask = 0xFFFFFF00u;
// The matrix has to have changed since the last frame, or every still copy of
// it in 64 MiB matches and nothing is learned.
constexpr float kChanged = 1e-4f;
constexpr std::size_t kPrintable = 40u;

struct Probe {
    bool started{};
    // Words whose change each frame keeps a fixed ratio to the camera's turn.
    // Whatever the game turns its camera with has to be one of them.
    bool angle_armed{};
    bool have_snapshot{};
    std::vector<std::uint8_t> snapshot;
    std::vector<std::uint32_t> angle;
    std::vector<float> angle_ratio;
    std::vector<float> angle_previous;
    std::uint64_t angle_frames{};
    int angle_attempts{};
    bool armed{};
    int attempts{};
    std::uint64_t frames{};
    std::vector<std::uint32_t> hits;
    std::array<float, 16> previous{};
    bool have_previous{};
};

Probe &probe() {
    static Probe value;
    return value;
}

float read_float(const std::uint8_t *base, std::uint32_t offset) {
    float value = 0.0f;
    std::memcpy(&value, base + offset, sizeof(value));
    return value;
}

// Does a 4x4 of floats at this offset hold the matrix the game gave the GE?
std::uint32_t bits_of(float value) {
    std::uint32_t bits = 0u;
    std::memcpy(&bits, &value, sizeof(bits));
    return bits;
}

bool matches(const std::uint8_t *ram, std::uint32_t offset, const std::array<float, 16> &view) {
    for (const int entry : kMatrixEntries) {
        std::uint32_t stored = 0u;
        std::memcpy(&stored, ram + offset + static_cast<std::uint32_t>(entry) * 4u, sizeof(stored));
        if ((stored & kFloat24Mask) != (bits_of(view[static_cast<std::size_t>(entry)]) & kFloat24Mask))
            return false;
    }
    return true;
}

bool changed(const std::array<float, 16> &a, const std::array<float, 16> &b) {
    for (const int entry : kMatrixEntries)
        if (std::fabs(a[static_cast<std::size_t>(entry)] - b[static_cast<std::size_t>(entry)]) > kChanged)
            return true;
    return false;
}

} // namespace

void camera_frame(psprecomp::Runtime &runtime, std::uint32_t view_matrix_source) {
    static const bool enabled = std::getenv("MHP3RD_FIND_CAMERA") != nullptr;
    if (!enabled) return;
#if defined(MHP3RD_HAS_RENDERER)
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
                  << std::hex << base << std::dec << "\n";
        p.started = true;
    }

    // --- words that move with the camera's turn ------------------------------
    const float turn = reading.turn;
    const bool turning = std::fabs(turn) >= 0.3f && std::fabs(turn) <= 40.0f;
    if (!turning) {
        p.have_snapshot = false;
    } else if (!p.angle_armed && !p.have_snapshot) {
        p.snapshot.assign(ram, ram + size);
        p.have_snapshot = true;
    } else if (!p.angle_armed) {
        for (std::uint32_t offset = 0; offset + 4u <= size; offset += 4u) {
            const float before = read_float(p.snapshot.data(), offset);
            const float now = read_float(ram, offset);
            if (!std::isfinite(before) || !std::isfinite(now) || now == before) continue;
            const float ratio = (now - before) / turn;
            if (!std::isfinite(ratio) || std::fabs(ratio) < 1e-5f || std::fabs(ratio) > 1e5f) continue;
            p.angle.push_back(offset);
            p.angle_ratio.push_back(ratio);
            p.angle_previous.push_back(now);
        }
        p.angle_armed = true;
        p.angle_frames = 1u;
        std::cout << "[find-camera] angle attempt " << p.angle_attempts << " (turn " << turn << "): "
                  << p.angle.size() << " candidates\n";
    } else {
        std::vector<std::uint32_t> offsets;
        std::vector<float> ratios;
        std::vector<float> previous;
        for (std::size_t i = 0; i < p.angle.size(); ++i) {
            const float now = read_float(ram, p.angle[i]);
            const float expected = p.angle_ratio[i] * turn;
            if (std::isfinite(now) &&
                std::fabs((now - p.angle_previous[i]) - expected) <= 0.05f * std::fabs(expected) + 1e-4f) {
                offsets.push_back(p.angle[i]);
                ratios.push_back(p.angle_ratio[i]);
                previous.push_back(now);
            }
        }
        const bool thinned = offsets.size() != p.angle.size();
        p.angle.swap(offsets);
        p.angle_ratio.swap(ratios);
        p.angle_previous.swap(previous);
        ++p.angle_frames;
        if (thinned)
            std::cout << "[find-camera] angle after " << p.angle_frames << " turning frames (turn " << turn
                      << "): " << p.angle.size() << " left\n";
        if (!p.angle.empty() && p.angle.size() <= kPrintable)
            for (std::size_t i = 0; i < p.angle.size(); ++i)
                std::cout << "[find-camera]   angle at 0x" << std::hex << (base + p.angle[i]) << std::dec
                          << " value=" << p.angle_previous[i] << " per-degree=" << p.angle_ratio[i] << "\n";
        if (p.angle.empty() && p.angle_attempts < 12) {
            p.angle_armed = false;
            p.have_snapshot = false;
            ++p.angle_attempts;
            std::cout << "[find-camera] that turn said nothing; waiting for another\n";
        }
    }

    (void)view_matrix_source;
#else
    (void)runtime;
    (void)view_matrix_source;
#endif
}

} // namespace mhp3rd::probe
