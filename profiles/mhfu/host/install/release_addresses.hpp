#pragma once

// The host features name addresses in Freedom Unite (ULUS-10391), where they
// were traced. Freedom Unite Complete is built from the same source, but its
// functions and data sit elsewhere, and a function calls and loads with other
// addresses in its words. This moves a traced address to the running
// release's (port) and one the game hands back to the traced one
// (traced_address), so a feature keeps its constants, ports them once when it
// installs and compares what the game passes it, such as a return address,
// after moving it back.
//
// complete_addresses.inc pairs the two releases' ranges, written by
// profiles/mhfu/tools/release_map.py from both executables and their overlays.
// An overlay's addresses depend on what is loaded in its slot, so each range
// names the image it belongs to. Addresses below the executable are the
// host's and the kernel's and stay where they are.

#include "install/game_identity.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace mhfu::install {

enum class Image : std::uint8_t { executable, game_task, lobby_task, game_sub };
inline constexpr std::size_t kImages = 4u;

// [start, end) in ULUS-10391's `image` is at `at` onwards in the other release.
struct AddressRange {
    std::uint32_t start{};
    std::uint32_t end{};
    std::uint32_t at{};
    Image image{};
};

// Where the executable loads; lower addresses are not the game's own.
inline constexpr std::uint32_t kGameCodeStart = 0x08804000u;
// Below the overlay slots, everything is the executable's. ULUS-10391's sub
// slot (kOverlaySlots in mhfu_profile.hpp) holds game_sub on a quest; below
// it is the task slot's.
inline constexpr std::uint32_t kOverlayStart = 0x09A00000u;
inline constexpr std::uint32_t kTracedSubSlot = 0x09C19100u;

// The image a traced address is in, for code in `from` that reaches it: the
// executable, game_sub in the sub slot, or the task overlay `from` is, else
// game_task.
[[nodiscard]] constexpr Image image_at(std::uint32_t traced, Image from) {
    if (traced < kOverlayStart) return Image::executable;
    if (traced >= kTracedSubSlot) return Image::game_sub;
    return from == Image::lobby_task ? Image::lobby_task : Image::game_task;
}

inline std::span<const AddressRange> complete_ranges() {
    static constexpr AddressRange kRanges[] = {
#include "install/complete_addresses.inc"
    };
    return kRanges;
}

// The ranges of one image ordered by where they start, on one side or the
// other.
class AddressMap {
public:
    explicit AddressMap(std::span<const AddressRange> ranges) {
        for (const AddressRange &range : ranges) {
            by_traced_[static_cast<std::size_t>(range.image)].push_back(range);
            by_other_[static_cast<std::size_t>(range.image)].push_back(range);
        }
        for (std::size_t i = 0; i < kImages; ++i) {
            std::sort(by_traced_[i].begin(), by_traced_[i].end(),
                [](const AddressRange &a, const AddressRange &b) { return a.start < b.start; });
            std::sort(by_other_[i].begin(), by_other_[i].end(),
                [](const AddressRange &a, const AddressRange &b) { return a.at < b.at; });
        }
    }

    // The other release's address for `traced`, or 0 when it has none.
    [[nodiscard]] std::uint32_t port(std::uint32_t traced, Image image) const {
        if (traced < kGameCodeStart) return traced;
        const std::vector<AddressRange> &ranges = by_traced_[static_cast<std::size_t>(image)];
        auto it = std::upper_bound(ranges.begin(), ranges.end(), traced,
            [](std::uint32_t address, const AddressRange &range) { return address < range.start; });
        if (it == ranges.begin()) return 0u;
        --it;
        return traced < it->end ? it->at + (traced - it->start) : 0u;
    }

    // ULUS-10391's address for `address` in the other release, or 0.
    [[nodiscard]] std::uint32_t traced(std::uint32_t address, Image image) const {
        if (address < kGameCodeStart) return address;
        const std::vector<AddressRange> &ranges = by_other_[static_cast<std::size_t>(image)];
        auto it = std::upper_bound(ranges.begin(), ranges.end(), address,
            [](std::uint32_t value, const AddressRange &range) { return value < range.at; });
        if (it == ranges.begin()) return 0u;
        --it;
        return address - it->at < it->end - it->start ? it->start + (address - it->at) : 0u;
    }

private:
    std::array<std::vector<AddressRange>, kImages> by_traced_{};
    std::array<std::vector<AddressRange>, kImages> by_other_{};
};

// The running release's map; null on ULUS-10391, where nothing moves, and on
// a release the host has no map for.
[[nodiscard]] inline const AddressMap *running_map() {
    if (running_release() != &kFreedomUniteComplete) return nullptr;
    static const AddressMap map{complete_ranges()};
    return &map;
}

// The host's features can run: on the traced release, or on one it can move
// their addresses to.
[[nodiscard]] inline bool features_ported() {
    return traced_executable() || running_map() != nullptr;
}

// `traced` in the running release, or 0 when it has no counterpart there.
// Without a map, on ULUS-10391 or before a release is known, it stays.
[[nodiscard]] inline std::uint32_t port(std::uint32_t traced, Image image = Image::executable) {
    const AddressMap *map = running_map();
    return map != nullptr ? map->port(traced, image) : traced;
}

// The traced address of `address` in the running release, or 0.
[[nodiscard]] inline std::uint32_t traced_address(std::uint32_t address, Image image = Image::executable) {
    const AddressMap *map = running_map();
    return map != nullptr ? map->traced(address, image) : address;
}

namespace detail {

constexpr std::uint32_t opcode(std::uint32_t word) {
    return word >> 26;
}
constexpr bool is_jump(std::uint32_t word) {
    return opcode(word) == 2u || opcode(word) == 3u;
}
constexpr std::uint32_t jump_target(std::uint32_t word, std::uint32_t pc) {
    return ((pc + 4u) & 0xF0000000u) | ((word & 0x03FFFFFFu) << 2);
}
constexpr bool is_address_high(std::uint32_t word) {
    const std::uint32_t imm = word & 0xFFFFu;
    return opcode(word) == 0x0Fu && imm >= 0x0880u && imm < 0x0A00u;
}
// The low halves of an address: addiu, ori and the loads and stores.
constexpr bool is_low_half(std::uint32_t word) {
    switch (opcode(word)) {
    case 0x09u:
    case 0x0Du:
    case 0x20u:
    case 0x21u:
    case 0x23u:
    case 0x24u:
    case 0x25u:
    case 0x28u:
    case 0x29u:
    case 0x2Bu:
    case 0x31u:
    case 0x35u:
    case 0x39u:
    case 0x3Du:
        return true;
    default:
        return false;
    }
}
// How far back a low half looks for the lui that sets its base.
inline constexpr std::uint32_t kHighWindow = 16u;

} // namespace detail

// True when `found`, at `address` in the running release, is the
// instruction `traced_word` traced at `traced` in ULUS-10391's `image`.
// Without a map the words are equal. Elsewhere a word may differ only in
// what holds an address: a jump's target, which must be the traced one
// moved; the high half of an address; or a low half whose base a nearby
// lui loads with one, in the image's own code.
template <typename Memory>
[[nodiscard]] bool same_instruction(const Memory &memory, std::uint32_t address, std::uint32_t found,
    std::uint32_t traced, std::uint32_t traced_word, Image image) {
    using namespace detail;
    if (found == traced_word) return true;
    if (running_map() == nullptr) return false;
    if (is_jump(traced_word)) {
        if (opcode(found) != opcode(traced_word)) return false;
        const std::uint32_t target = jump_target(traced_word, traced);
        return port(target, image_at(target, image)) == jump_target(found, address);
    }
    if ((found & 0xFFFF0000u) != (traced_word & 0xFFFF0000u)) return false;
    if (is_address_high(traced_word)) return is_address_high(found);
    if (!is_low_half(traced_word)) return false;
    const std::uint32_t base = (found >> 21) & 31u;
    for (std::uint32_t back = 1u; back <= kHighWindow && address >= back * 4u; ++back) {
        const std::uint32_t at = address - back * 4u;
        if (!memory.contains(at, 4u)) break;
        const std::uint32_t word = memory.load32(at);
        if (is_address_high(word) && ((word >> 16) & 31u) == base) return true;
    }
    return false;
}

// The word at `traced`'s counterpart is the traced instruction.
template <typename Memory>
[[nodiscard]] bool code_matches(
    const Memory &memory, std::uint32_t traced, std::uint32_t traced_word, Image image = Image::executable) {
    const std::uint32_t address = port(traced, image);
    if (address == 0u || !memory.contains(address, 4u)) return false;
    return same_instruction(memory, address, memory.load32(address), traced, traced_word, image);
}

} // namespace mhfu::install
