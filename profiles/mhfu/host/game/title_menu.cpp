#include "game/title_menu.hpp"

#include "psprecomp/guest_memory.hpp"

namespace mhfu::game::title_menu {
namespace {

struct CodeWord {
    std::uint32_t address;
    std::uint32_t word;
};

// The menu update's prologue, its step dispatch, the pad reads and the
// cursor's wrap, and the label loop.
constexpr CodeWord kSignature[] = {
    {0x09A61568u, 0x27BDFFD0u}, // addiu sp, sp, -48
    {0x09A6158Cu, 0x8C82001Cu}, // lw v0, 28(a0): the step
    {0x09A61590u, 0x2C410006u}, // sltiu at, v0, 6
    {0x09A61618u, 0x3C0208A6u}, // lui v0, 0x08A6
    {0x09A6161Cu, 0x8C432D94u}, // lw v1, 0x2D94(v0): pressed
    {0x09A61620u, 0x30620010u}, // andi v0, v1, 0x10: up
    {0x09A61628u, 0x30620040u}, // andi v0, v1, 0x40: down
    {0x09A61660u, 0x8C830498u}, // lw v1, 1176(a0): the cursor
    {0x09A61684u, 0x24020005u}, // li v0, 5: below 0
    {0x09A616E8u, 0x28420006u}, // slti v0, v0, 6: past the last
    {0x09A6197Cu, 0x2631CC0Cu}, // addiu s1, s1, 0xCC0C: the labels
    {0x09A61988u, 0x96250000u}, // lhu a1, 0(s1)
    {0x09A61B64u, 0x2A430006u}, // slti v1, s2, 6
};

} // namespace

std::uint32_t cursor_before_input(std::uint32_t cursor, std::uint32_t pressed) noexcept {
    const bool up = (pressed & kUp) != 0u;
    const bool down = !up && (pressed & kDown) != 0u;
    if (cursor == kHiddenEntry) return up || down ? kHiddenEntry : 0u;
    if (up && cursor == 0u) return kHiddenEntry;                 // the game takes it to DOWNLOAD
    if (down && cursor == kHiddenEntry - 1u) return kHiddenEntry; // and this to NEW GAME
    return cursor;
}

bool hide_label(psprecomp::GuestMemory &memory, HiddenLabel &hidden) {
    if (!memory.contains(kTextBlock, 12u)) return false;
    const std::uint32_t table = kTextBlock + memory.load32(kTextBlock + 8u);
    const std::uint32_t entry = table + 4u * (kFirstLabel + kHiddenEntry);
    if (!memory.contains(entry, 4u)) return false;
    const std::uint32_t offset = memory.load32(entry);
    std::uint32_t end = table + offset;
    for (std::uint32_t n = 0; n < 256u; ++n, ++end) {
        if (!memory.contains(end, 1u)) return false;
        if (memory.load8(end) != 0u) continue;
        if (n == 0u) return false;
        hidden = {entry, offset, end - table};
        memory.store32(entry, hidden.hidden_offset);
        return true;
    }
    return false;
}

void restore_label(psprecomp::GuestMemory &memory, const HiddenLabel &hidden) {
    if (memory.load32(hidden.entry) == hidden.hidden_offset) memory.store32(hidden.entry, hidden.offset);
}

bool code_matches(const psprecomp::GuestMemory &memory) {
    for (const CodeWord &expected : kSignature)
        if (!memory.contains(expected.address, 4u) || memory.load32(expected.address) != expected.word) return false;
    for (std::uint32_t i = 0; i < kEntries; ++i)
        if (!memory.contains(kLabels + 2u * i, 2u) || memory.load16(kLabels + 2u * i) != kFirstLabel + i) return false;
    return true;
}

} // namespace mhfu::game::title_menu
