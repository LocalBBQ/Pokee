#pragma once

// What the host knows about the running game's memory and uses in more than
// one place: the game's own text, and the equipment kinds with their name
// tables. Layered armor reads the names. Everything here only reads.
//
// The text block was read from the USA Freedom Unite executable (ULUS-10391).
//
// Pure functions over a Ram, so the unit tests run them on a buffer.

#include <cstdint>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace mhfu::game {

class Ram;

// Little-endian word at any address (the game's data is not word aligned
// throughout), and a zero-terminated UTF-8 string of at most 255 bytes.
[[nodiscard]] std::uint32_t read32(const Ram &ram, std::uint32_t address);
[[nodiscard]] std::string read_text(const Ram &ram, std::uint32_t address);

// Text ----------------------------------------------------------------------------

// The game's text, loaded at this address (0x089D0000 - 15008). 0x0885148C
// reads table `index` as the word at (index + 2) * 4, an offset from here.
// Each table is then a list of 32-bit offsets (from the table) to UTF-8
// strings, ended by 0xFFFFFFFF. Armor names are tables 7, 9, 11, 13 and 15
// (head, chest, arms, waist, legs).
inline constexpr std::uint32_t kTextBlock = 0x089CC560u;

// Table `index` of the text block, or empty when it does not look like one
// (the block not loaded yet, or a different executable).
[[nodiscard]] std::vector<std::string> text_table(const Ram &ram, std::uint32_t block, int index);
// One entry of a table, or "" when there is none.
[[nodiscard]] std::string text_entry(const Ram &ram, std::uint32_t block, int index, std::uint32_t entry);
// How many entries a table has; 0 when it does not look like one.
[[nodiscard]] std::uint32_t text_count(const Ram &ram, std::uint32_t block, int index);

// Equipment kinds ------------------------------------------------------------------

// An equipment slot is 12 bytes. Layered armor walks the hunter's box at
// this stride: u8 used, u8 kind, u16 id, then the rest of the record.
inline constexpr std::uint32_t kEquipmentRecord = 12u;

// The port's number for an equipment kind, and the text table with its names.
// Kinds 0 to 4 are the armor parts (chest, arms, waist, legs, head), 5 to 17
// the weapon classes (10 is not used). This is not the kind byte the game
// stores in an equipment record; layered_armor.hpp (kBoxKind) maps the armor
// parts to that byte.
struct EquipmentKind {
    std::uint8_t kind;
    int name_table;
    const char *label;
};
// Armor parts first, then the weapon classes in the game's own order.
[[nodiscard]] const std::vector<EquipmentKind> &equipment_kinds();
[[nodiscard]] const EquipmentKind *equipment_kind(std::uint8_t kind);
[[nodiscard]] bool is_armor(std::uint8_t kind);
[[nodiscard]] bool is_weapon(std::uint8_t kind);
// The names of one kind's pieces, indexed by id. Id 0 is "no equipment".
[[nodiscard]] std::vector<std::string> equipment_names(const Ram &ram, std::uint8_t kind);
// One piece's name, or "" when there is none.
[[nodiscard]] std::string equipment_name(const Ram &ram, std::uint8_t kind, std::uint16_t id);

} // namespace mhfu::game
