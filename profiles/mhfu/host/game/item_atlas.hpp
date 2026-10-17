#pragma once

// The item atlas (ULUS-10391): the 256 by 256 sheet the HUD draws item icons
// from, at GE texture address 0x091874A0 (CLUT8). The icons are grey and the
// game tints each one (game/gather_art.hpp). Its palette is wherever the
// game last loaded it, so the renderer notes the texture state of a draw
// that samples the atlas, and the sheet is decoded from that state.

#include "game/guest_layout.hpp"
#include "gpu/ge_state.hpp"

#include <cstdint>

namespace psprecomp {
class GuestMemory;
}

namespace mhfu::game {

inline constexpr std::uint32_t kItemAtlasAddress = layout::kItemAtlas;

// From the renderer, for each draw: kept when it samples the atlas.
void note_item_atlas(const gpu::TextureState &texture);

// Decodes the sheet when its texels or palette have changed. Nothing until
// a draw has sampled it.
void refresh_item_atlas(const psprecomp::GuestMemory &memory);

// RGBA8888, red in the low byte. `generation` changes when the pixels do.
struct ItemAtlas {
    bool ready{};
    std::uint32_t width{};
    std::uint32_t height{};
    const std::uint32_t *pixels{};
    std::uint64_t generation{};
};
[[nodiscard]] ItemAtlas item_atlas();

} // namespace mhfu::game
