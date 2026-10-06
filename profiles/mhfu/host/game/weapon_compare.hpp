#pragma once

// Figures for comparing weapons. Equipment detail, the status screen and the
// smithy print an attack already multiplied by a class coefficient, and an
// element already multiplied by 10. These divide those printed figures back.
// They do not describe the attack or element a hit uses.

#include <cstdint>
#include <optional>

namespace mhfu::game {

// The class coefficient in tenths (4.8 is 48), or nothing for a kind that is
// not a weapon. Kind bytes are the equipment kinds (game_data.hpp): 5 great
// sword, 6 sword and shield, 7 hammer, 8 lance, 9 heavy bowgun, 11 light
// bowgun, 12 long sword, 13 switch axe, 14 gunlance, 15 bow, 16 dual blades,
// 17 hunting horn. Switch axe's 5.4 is the usual figure; it was not read out
// of this executable.
[[nodiscard]] inline std::optional<int> weapon_coefficient_tenths(std::uint8_t kind) {
    switch (kind) {
    case 5:
    case 12: return 48; // great sword, long sword: 4.8
    case 6:
    case 16: return 14; // sword and shield, dual blades: 1.4
    case 7:
    case 17: return 52; // hammer, hunting horn: 5.2
    case 8:
    case 14: return 23; // lance, gunlance: 2.3
    case 9:
    case 11:
    case 15: return 12; // heavy bowgun, light bowgun, bow: 1.2
    case 13: return 54; // switch axe: 5.4
    default: return std::nullopt;
    }
}

// The printed attack divided by the class coefficient, to the nearest whole
// number. A kind without a coefficient is returned unchanged.
[[nodiscard]] inline int comparable_attack(int displayed, std::uint8_t kind) {
    const std::optional<int> tenths = weapon_coefficient_tenths(kind);
    if (!tenths || displayed < 0) return displayed;
    // Nearest whole number: 336 / 4.8 is 70, 161 / 2.3 is 70.
    return (displayed * 10 + *tenths / 2) / *tenths;
}

// The printed element divided by 10. A negative figure is left as it is.
[[nodiscard]] inline int comparable_element(int displayed) {
    if (displayed < 0) return displayed;
    return displayed / 10;
}

// What the shop prints from the attack and element stored on a weapon. The
// stored attack is the one a hit uses; the shop multiplies it by the class
// coefficient. The stored element is the one a hit uses; the shop multiplies
// it by 10.
[[nodiscard]] inline int shop_attack(int raw, std::uint8_t kind) {
    const std::optional<int> tenths = weapon_coefficient_tenths(kind);
    if (!tenths || raw < 0) return raw;
    return (raw * *tenths + 5) / 10;
}

[[nodiscard]] inline int shop_element(int raw) {
    if (raw < 0) return raw;
    return raw * 10;
}

// `shown` as equipment detail, the status screen or the smithy should print it
// for this weapon. The weapon's own attack becomes the attack a hit uses, and
// its own element loses the extra ten. Any other figure, including one that
// matches both, is left as printed.
[[nodiscard]] inline int reprint_shown(int shown, std::uint8_t kind, int raw_attack, int raw_element) {
    if (shown <= 0) return shown;
    const int attack = shop_attack(raw_attack, kind);
    const int element = shop_element(raw_element);
    const bool attack_hit = attack > 0 && shown == attack;
    const bool element_hit = element > 0 && shown == element;
    if (attack_hit && !element_hit) return comparable_attack(shown, kind);
    if (element_hit && !attack_hit) return comparable_element(shown);
    return shown;
}

// The attack and element on one weapon record, the figures a hit uses. Attack
// is the signed half at +10 and element the signed byte at +12, which is how
// these 28- and 80-byte records are laid out. Nothing when the bytes do not
// look like a record (the model, rarity, price, attack or element is out of
// the range a weapon has), so a table that is not loaded yet is not shown.
struct ComparedWeapon {
    int attack{};
    int element{};
};

[[nodiscard]] inline std::optional<ComparedWeapon> compared_weapon(
    const std::uint8_t *bytes, std::size_t size, std::uint8_t kind) {
    if (bytes == nullptr || size < 14u || !weapon_coefficient_tenths(kind)) return std::nullopt;
    const auto half = [](const std::uint8_t *p) {
        return static_cast<std::uint16_t>(p[0] | (p[1] << 8u));
    };
    const std::uint16_t model = half(bytes);
    const std::uint8_t rarity = bytes[2];
    const std::uint32_t price = static_cast<std::uint32_t>(half(bytes + 4)) |
                                (static_cast<std::uint32_t>(half(bytes + 6)) << 16u);
    const int attack = static_cast<std::int16_t>(half(bytes + 10));
    const int element = static_cast<std::int8_t>(bytes[12]);
    if (model == 0u || model > 400u || rarity < 1u || rarity > 10u || price > 2000000u) return std::nullopt;
    if (attack < 1 || attack > 500 || element < -30 || element > 80) return std::nullopt;
    return ComparedWeapon{attack, element};
}

} // namespace mhfu::game
