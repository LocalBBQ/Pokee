#pragma once

// Two synthetic overlays for the same slot. The bytes are not from the game:
// an MWo3 header plus 64 bytes of code, differing only in the name and one
// code byte, so each has its own identity hash.

#include <array>
#include <cstddef>
#include <cstdint>

inline constexpr std::uint32_t kOverlayFixtureBase = 0x09D63100u;
inline constexpr std::uint32_t kOverlayFixtureEntry = 0x09D63140u;
inline constexpr std::uint32_t kOverlayFixtureCodeBytes = 64u;
inline constexpr std::size_t kOverlayFixtureBytes = 128u;

inline std::array<std::uint8_t, kOverlayFixtureBytes> overlay_fixture_image(std::uint32_t mark) {
    std::array<std::uint8_t, kOverlayFixtureBytes> image{};
    image[0] = 'M';
    image[1] = 'W';
    image[2] = 'o';
    image[3] = '3';
    image[8] = static_cast<std::uint8_t>(kOverlayFixtureBase);
    image[9] = static_cast<std::uint8_t>(kOverlayFixtureBase >> 8u);
    image[10] = static_cast<std::uint8_t>(kOverlayFixtureBase >> 16u);
    image[11] = static_cast<std::uint8_t>(kOverlayFixtureBase >> 24u);
    image[12] = static_cast<std::uint8_t>(kOverlayFixtureCodeBytes);
    const char *name = mark == 1u ? "stageA" : "stageB";
    for (std::size_t index = 0u; name[index] != '\0'; ++index) image[32u + index] = static_cast<std::uint8_t>(name[index]);
    image[64] = static_cast<std::uint8_t>(mark);
    return image;
}

inline std::uint64_t overlay_fixture_hash(const std::uint8_t *data, std::size_t size) {
    std::uint64_t hash = 0xCBF29CE484222325ull;
    for (std::size_t index = 0u; index < size; ++index) {
        hash ^= data[index];
        hash *= 0x100000001B3ull;
    }
    return hash;
}
