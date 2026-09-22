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

    const std::array<float, 16> &view = reading.view;
    const bool moving = p.have_previous && changed(p.previous, view);
    p.previous = view;
    p.have_previous = true;
    if (!moving) return;  // a still camera proves nothing about where it is kept

    if (!p.armed) {
        p.hits.clear();
        for (std::uint32_t offset = 0; offset + 64u <= size; offset += 4u)
            if (matches(ram, offset, view)) p.hits.push_back(offset);
        p.armed = true;
        p.frames = 1u;
        std::cout << "[find-camera] attempt " << p.attempts << ": " << p.hits.size()
                  << " places hold the view matrix\n";
        if (p.attempts % 200 == 0)
            std::cout << "[find-camera]   the display list uploaded it from 0x" << std::hex
                      << view_matrix_source << std::dec << "\n";
        if (p.hits.empty()) {
            p.armed = false;
            ++p.attempts;
        }
        return;
    }

    std::vector<std::uint32_t> kept;
    kept.reserve(p.hits.size());
    for (std::uint32_t offset : p.hits)
        if (matches(ram, offset, view)) kept.push_back(offset);
    const bool thinned = kept.size() != p.hits.size();
    p.hits.swap(kept);
    ++p.frames;

    if (thinned || p.frames % 150u == 0u) {
        std::cout << "[find-camera] after " << p.frames << " moving frames: " << p.hits.size() << " left"
                  << " (camera at " << reading.position[0] << "," << reading.position[1] << ","
                  << reading.position[2] << " yaw " << reading.yaw << ")\n";
        if (p.hits.size() <= kPrintable)
            for (std::uint32_t offset : p.hits)
                std::cout << "[find-camera]   view matrix at 0x" << std::hex << (base + offset) << std::dec << "\n";
    }
    if (p.hits.empty()) {
        p.armed = false;
        ++p.attempts;
    }
#else
    (void)runtime;
    (void)view_matrix_source;
#endif
}

} // namespace mhp3rd::probe
