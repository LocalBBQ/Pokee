#pragma once

#include <cstdint>
#include <optional>

namespace psprecomp {
class GuestMemory;
}

namespace mhfu::camera {

// One cell of the monster icon sheet, 36 by 36 texels, origin at the top left.
// The species byte at entity +0x1E8 selects the cell. Nothing when the sheet
// has no picture for that id.
struct HuntIconCell {
    std::uint16_t u{};
    std::uint16_t v{};
};
inline constexpr std::uint16_t kHuntIconTexels = 36u;

[[nodiscard]] std::optional<HuntIconCell> hunt_icon_cell(std::uint8_t id);

// The sheet those cells are cut from, decoded to RGBA8888 (red in the low
// byte). Empty until a quest has filled the buffer. `generation` changes
// when the pixels do.
struct HuntIconSheet {
    bool ready{};
    std::uint32_t width{};
    std::uint32_t height{};
    const std::uint32_t *pixels{};
    std::uint64_t generation{};
};

void refresh_hunt_icons(const psprecomp::GuestMemory &memory);
[[nodiscard]] HuntIconSheet hunt_icon_sheet();

} // namespace mhfu::camera
