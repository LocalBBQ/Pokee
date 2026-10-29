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

// A sprite of the sheet, in the game's texels. The HUD draws the sheet's
// sprites by number through EBOOT 0x0882AF70, which reads them from the table
// at 0x089392C8: 12 bytes each, the halfwords u, v, width and height, then the
// texture (0x8001, the sheet) and the palette.
struct QuestBarSprite {
    std::uint16_t u{};
    std::uint16_t v{};
    std::uint16_t width{};
    std::uint16_t height{};
};
inline constexpr std::uint32_t kQuestBarSprites = 0x089392C8u;
inline constexpr std::uint32_t kQuestBarSpriteStride = 12u;
inline constexpr std::uint16_t kQuestBarSpriteTexture = 0x8001u;

// Sprite 36: the hexagon the item bar draws behind the selected item, 40 by
// 40, the table's only sprite of that size (traced at (372,200)-(412,240),
// with the item's 32 by 32 icon at (376,202)-(408,234) over it).
inline constexpr std::uint32_t kItemBarFrameSprite = 36u;
inline constexpr QuestBarSprite kItemBarFrame{152u, 32u, 40u, 40u};

// Whether the game's table holds `expected` as sprite `number` of the sheet.
// False when the entry differs or is not mapped.
[[nodiscard]] bool quest_bar_sprite_matches(
    const psprecomp::GuestMemory &memory, std::uint32_t number, const QuestBarSprite &expected);

// A sprite's place on a sheet of any size, as 0..1 texture coordinates.
[[nodiscard]] GlyphCell quest_bar_cell(const QuestBarSprite &sprite) noexcept;

// True once refresh_button_glyphs has found kItemBarFrame in the game's
// table. Until then, or if the table differs, the frame is not drawn.
[[nodiscard]] bool item_bar_frame_ready();

} // namespace mhfu::game
