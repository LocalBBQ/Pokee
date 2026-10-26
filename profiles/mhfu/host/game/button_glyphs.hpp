#pragma once

// The game's own button glyphs (ULUS-10391), for the port's interface. The
// prompts the game writes as ~B01, ~B02 and so on are sprites from the quest
// bar's sheet, the 256 by 256 CLUT8 texture at GE address 0x09156BA0
// (MHFU_TRACE_CAPTURE in the item box: "Info" is (48,0)-(64,16) of it). The
// face buttons are 16 by 16 cells along its top row: ○, ×, △ and □.
//
// A texture pack replaces the whole sheet, and a pack with button icon sets
// has one image of it per set (QuestBar_(Xbox).png, QuestBar_(PS).png,
// QuestBar_(N).png in the community pack), so drawing from the image the
// renderer chose for the sheet shows the glyphs the game's own prompts show,
// in the set the player picked. The renderer notes the sheet's texture state
// from a draw that samples it and, when it looks the sheet up in the pack,
// which image the pack gives it. The sheet is the pack's image, decoded on a
// thread of its own, or with no image the game's texture, decoded with the
// palette it was last drawn with.

#include "gpu/ge_state.hpp"

#include <cstdint>
#include <filesystem>
#include <optional>

namespace psprecomp {
class GuestMemory;
}

namespace mhfu::game {

inline constexpr std::uint32_t kQuestBarAddress = 0x09156BA0u;
inline constexpr std::uint32_t kQuestBarSide = 256u;
inline constexpr std::uint32_t kButtonGlyphSide = 16u;

enum class ButtonGlyph : std::uint8_t { Circle, Cross, Triangle, Square };

// The glyph for a PSP button bit: ○ 0x2000, × 0x4000, △ 0x1000, □ 0x8000.
[[nodiscard]] std::optional<ButtonGlyph> button_glyph_for(std::uint32_t button) noexcept;

// From the renderer: a draw that samples the sheet (anything else is ignored).
void note_quest_bar(const gpu::TextureState &texture);
// From the renderer, each time it looks the sheet up: the pack's image for it,
// or an empty path with no pack or none in it.
void note_quest_bar_image(const std::filesystem::path &file);

// Brings the sheet up to date: starts decoding a newly chosen image, takes a
// finished one, or decodes the game's texture when its texels or palette
// changed. Nothing until the sheet has been drawn.
void refresh_button_glyphs(const psprecomp::GuestMemory &memory);

// RGBA8888, red in the low byte. `generation` changes when the pixels do.
struct ButtonGlyphSheet {
    bool ready{};
    std::uint32_t width{};
    std::uint32_t height{};
    const std::uint32_t *pixels{};
    std::uint64_t generation{};
};
[[nodiscard]] ButtonGlyphSheet button_glyph_sheet();

// A glyph's cell on a sheet `width` by `height`, as 0..1 texture coordinates:
// the game's cell scaled to the sheet's size, for an image drawn at any
// multiple of the game's.
struct GlyphCell {
    float u0{};
    float v0{};
    float u1{};
    float v1{};
};
[[nodiscard]] GlyphCell button_glyph_cell(ButtonGlyph glyph) noexcept;

} // namespace mhfu::game
