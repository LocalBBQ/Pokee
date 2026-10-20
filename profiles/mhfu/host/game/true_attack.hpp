#pragma once

// True attack: the equipment screens show a weapon's attack as the game
// counts it, without the class factor, as FUComplete's True Raw does. A
// great sword shown as 672 shows 140. Off by default (Mods page,
// `game.true_attack`; `MHFU_TRUE_ATTACK=1` turns it on). Only the number on
// screen changes; damage never reads it.
//
// Read from the executable (ULUS-10391). A weapon record keeps its attack as
// the game counts it (0x0895A0EC + id * 24, s16 at +10, read for damage by
// 0x0885D300's stat 0 and summed by 0x088D8EE0). The screens that show it
// multiply by a factor and divide by 100: 11 words at 0x0893ACDC, indexed by
// the weapon class 0x0885CD18 returns, read with lui 0x0894 / addiu -21284 at
// 0x0881B098, 0x0881F144, 0x0881F1BC, 0x0881F244, 0x0881F390, 0x0881F464 and
// 0x0881F5FC, and in lobby_task at 0x09AEB514 and 0x09AEBBFC. Nothing else in
// the executable or any overlay addresses the table, and the game never
// writes it.
//
// With the switch on, every factor is 100, so each screen shows the attack it
// read; off, the game's factors go back. The switch is read at each frame.

#include "psprecomp/guest_memory.hpp"

#include <array>
#include <cstdint>
#include <span>

namespace psprecomp {
class Runtime;
}

namespace mhfu::game::true_attack {

inline constexpr std::uint32_t kFactors = 0x0893ACDCu;
inline constexpr std::size_t kClasses = 11u;
inline constexpr std::uint32_t kTrueFactor = 100u; // the screens divide by 100
// Great sword, heavy bowgun, hammer, lance, sword and shield, light bowgun,
// dual blades, long sword, hunting horn, gunlance, bow.
inline constexpr std::array<std::uint32_t, kClasses> kGameFactors = {
    480u, 120u, 520u, 230u, 140u, 120u, 140u, 480u, 520u, 230u, 120u};

struct CodeWord {
    std::uint32_t address;
    std::uint32_t word;
};

// The readers' code the addresses above were read off. A mismatch means
// another executable, and the table is left alone.
[[nodiscard]] std::span<const CodeWord> code_signature();
[[nodiscard]] bool code_matches(const psprecomp::GuestMemory &memory);

// Writes the factors for `on` when the code matches and the table holds the
// game's factors or the true ones. False when the table is neither, and
// nothing is written.
bool apply(psprecomp::GuestMemory &memory, bool on);

// At the flip: applies the setting.
void frame(psprecomp::Runtime &runtime);

} // namespace mhfu::game::true_attack
