#pragma once

// What the gather mark shows (ULUS-10391): the picture of the item a gather
// point gives, the pickaxe over a mining point, the bug net over a bug point,
// or the monster being carved. Read from the game's own tables, so every map
// and every point is covered by the same lookups.
//
// A point's items. The gather itself (game_task 0x09AAAB40) passes the
// record's id (+16) to 0x08870AC4, which looks the point up with 0x08869750
// and rolls it with 0x0887265C:
//   base  = the word at +80 of the gather object (the pointer at 0x08A62D9C)
//   entry = base + word[base + word[0x08A5C584] + id * 4]
// The entry is a list of halfword pairs, weight then item, that ends where
// the weight is 0xFFFF. The roll picks an item with a chance of its weight in
// the total, so the heaviest is what the point mostly gives.
//
// An item's picture. The item icon draw 0x0882E0D0 reads a 24-byte record at
// 0x0899D2BC + item * 24: +1 the icon (255 none), +2 the tint. 0x088303B8
// turns the icon into a sprite with the halfword at 0x0893ABEC + icon * 2,
// and the sprite draw 0x0882BE60 reads its 12-byte rectangle at
// 0x089392C8 + sprite * 12: u, v, width, height, flags and the texture slot,
// all halfwords. The tint is three bytes, red, green and blue, at
// 0x0893AB7C + tint * 4. Item icons are 24 by 24 on the item atlas.
//
// Tools. The circle handler's mining path (game_task 0x09AA6FA4) checks the
// selected item against 144, 145, 146 and 1225, and against 148, 149, 150 and
// 1226. Their records share icon 16 (the pick) and icon 13 (the net), each in
// the tiers' tints. The plain ones, 145 and 149, are the pickaxe and the bug
// net shown here.
//
// Bait. The item handler fishes with items 136 to 143 (game/gather_icon.hpp).
// Their records share icon 4 in eight tints; the first, 136, is the one a
// fishing spot shows.

#include "psprecomp/guest_memory.hpp"

#include <array>
#include <cstdint>
#include <optional>
#include <span>
#include <vector>

namespace mhfu::game {

inline constexpr std::uint16_t kPickaxeItem = 145u;
inline constexpr std::uint16_t kBugNetItem = 149u;

// Bone piles. Their lists mix the bone husks (items 93 to 95, the only items
// with icon 28) with bones (icon 10, the skull and ribs, in each bone's tint),
// and a husk is usually the heaviest. A point whose heaviest item is either
// shows the plain white bone, item 292, so every bone pile looks the same.
// Herb points list the grey husk (93) as filler, so having one on the list
// is not enough.
inline constexpr std::uint8_t kHuskIcon = 28u;
inline constexpr std::uint8_t kBoneIcon = 10u;
inline constexpr std::uint16_t kBoneItem = 292u;
inline constexpr std::uint16_t kBaitItem = 136u;

// The mark's picture: an item's icon, a monster's, or none (the ?).
struct GatherArt {
    enum class Kind : std::uint8_t { None, Item, Monster };
    Kind kind{Kind::None};
    std::uint16_t item{};
    std::uint8_t monster{}; // the species byte at entity +0x1E8

    [[nodiscard]] static GatherArt of_item(std::uint16_t item) { return {Kind::Item, item, 0u}; }
    [[nodiscard]] static GatherArt of_monster(std::uint8_t id) { return {Kind::Monster, 0u, id}; }
    bool operator==(const GatherArt &) const = default;
};

struct GatherArtWord {
    std::uint32_t address{};
    std::uint32_t instruction{};
};

// The instructions the lookups above were read from.
[[nodiscard]] std::span<const GatherArtWord> gather_art_words();

// True when each of those words is there. A mapped word that differs is
// logged once.
[[nodiscard]] bool gather_art_code_matches(const psprecomp::GuestMemory &memory);

// A gather point's list: each item with its weight.
struct GatherDrop {
    std::uint16_t weight{};
    std::uint16_t item{};
};
// Nothing when the list is not there or runs past 64 pairs.
[[nodiscard]] std::optional<std::vector<GatherDrop>> gather_drops(
    const psprecomp::GuestMemory &memory, std::uint16_t id);

// The item a gather point mostly gives: the heaviest on its list, the first
// of equals. Nothing when the list is not there, is empty, or runs past 64
// pairs.
[[nodiscard]] std::optional<std::uint16_t> gather_item(const psprecomp::GuestMemory &memory, std::uint16_t id);

// An item's icon number and tint number in its record, for the trace.
[[nodiscard]] std::optional<std::array<std::uint8_t, 2>> item_icon_and_tint(
    const psprecomp::GuestMemory &memory, std::uint16_t item);

// The art for a gather record of this kind and id: the pickaxe for mining
// (3), the bug net for a bug point (4), the bone for a bone pile, else the
// item the point gives.
[[nodiscard]] GatherArt gather_art_for(const psprecomp::GuestMemory &memory, std::uint16_t kind, std::uint16_t id);

// An item's icon: its rectangle on the item atlas, in texels, and its tint.
struct ItemArt {
    std::uint16_t u{};
    std::uint16_t v{};
    std::uint16_t width{};
    std::uint16_t height{};
    std::uint8_t red{};
    std::uint8_t green{};
    std::uint8_t blue{};
};

// Nothing for an item without an icon, past the table, or with a rectangle
// that is not on the 256 by 256 atlas.
[[nodiscard]] std::optional<ItemArt> item_art(const psprecomp::GuestMemory &memory, std::uint16_t item);

// The same for an icon and tint number, as an item record holds them.
[[nodiscard]] std::optional<ItemArt> icon_art(
    const psprecomp::GuestMemory &memory, std::uint8_t icon, std::uint8_t tint);

// The game's red ?, for a mark with no picture of its own: icon 33 in tint 1,
// as the game's own unknown items (1168, 1179 and 1180) draw it.
inline constexpr std::uint8_t kUnknownIcon = 33u;
inline constexpr std::uint8_t kUnknownTint = 1u;

} // namespace mhfu::game
