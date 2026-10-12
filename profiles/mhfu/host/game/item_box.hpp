#pragma once

// Fast item box: items come out of the box one after another, without the
// pause after each one, closer to how the later games take items out of the
// box. Always on. `MHFU_FAST_ITEM_BOX=0` leaves the game's own timing.
//
// In a quest the executable (ULUS-10391) runs the hunter HUD at 0x0883F270
// once a frame. That function is reached through a function pointer, so a
// wrapper on it is actually called. While the box is open it updates the box
// at 0x088476F4 (jal at 0x0883F75C, a0 is the HUD). Taking an item stores 30
// in the player's byte at +1374 and 31 in the HUD halfword at +1438. The
// update counts the byte down and ignores input until it reaches zero; the
// HUD counts the halfword down while the icon flies to the pouch. Both are
// about a second at 30 frames a second.
//
// The wrapper runs the HUD and then cuts a lock longer than one frame to one
// and clears a flight that is still in the game's 31-frame range, so the next
// frame takes input and the icon does not sit on the way to the pouch.

#include "psprecomp/guest_memory.hpp"

#include <cstdint>
#include <span>

namespace psprecomp {
class Runtime;
}

namespace mhfu::game::item_box {

inline constexpr std::uint32_t kHudUpdate = 0x0883F270u;
inline constexpr std::uint32_t kBoxUpdate = 0x088476F4u;
inline constexpr std::uint32_t kPlayerPointer = 1328u;
inline constexpr std::uint32_t kLockByte = 1374u;
inline constexpr std::uint32_t kAnimHalf = 1438u;
inline constexpr std::uint8_t kGameLock = 30u;
inline constexpr std::uint16_t kGameAnim = 31u;

struct CodeWord {
    std::uint32_t address;
    std::uint32_t word;
};

// What the addresses above were read off. A mismatch means another
// executable, and nothing is wrapped.
[[nodiscard]] std::span<const CodeWord> code_signature();
[[nodiscard]] bool code_matches(const psprecomp::GuestMemory &memory);

// The lock the box is left with: one frame for anything longer.
[[nodiscard]] std::uint8_t shortened_lock(std::uint8_t lock) noexcept;

// The flight the HUD is left with: none for the game's own 31-frame icon.
[[nodiscard]] std::uint16_t shortened_anim(std::uint16_t anim) noexcept;

// At the flip: puts the wrapper in place once the executable's code is
// installed. Runs every frame; wrapping again is a no-op.
void frame(psprecomp::Runtime &runtime);

} // namespace mhfu::game::item_box
