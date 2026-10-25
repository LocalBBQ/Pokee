#pragma once

// Where Freedom Unite (ULUS-10391) keeps what more than one host feature
// reads: the overlay slots and the overlay loaded in each, the camera object
// and the hunter it follows, GameSys, and the quest entity list with the
// fields of an entity on it. Each is defined once here; a field that only one
// feature reads stays with that feature.
//
// The functions take any memory with contains() and load8/16/32():
// psprecomp::GuestMemory as the game runs, game::Ram (game/guest_ram.hpp)
// for the code that reads the game's data, and the stand-ins of the tests.

#include <cstddef>
#include <cstdint>
#include <string_view>

namespace mhfu::game::layout {

// User memory, where every pointer the game keeps points.
inline constexpr std::uint32_t kRamStart = 0x08800000u;
inline constexpr std::uint32_t kRamEnd = 0x0A000000u;
[[nodiscard]] constexpr bool in_ram(std::uint32_t address) noexcept {
    return address >= kRamStart && address < kRamEnd;
}

// Overlays -------------------------------------------------------------------

// The overlay slots (kOverlaySlots in mhfu_profile.hpp). A loaded MWo3 image
// starts with the magic, keeps its own load address at +8 and its file name,
// zero-terminated, at +32.
inline constexpr std::uint32_t kTaskSlot = 0x09A5F300u; // *_task.ovl: game_task while a quest runs
inline constexpr std::uint32_t kSubSlot = 0x09C19100u;  // demo_sub.ovl, game_sub.ovl
inline constexpr std::uint32_t kEmOverlay = 0x09D1A280u;
inline constexpr std::uint32_t kEmOverlayEnd = 0x09D63100u;
inline constexpr std::uint32_t kOverlayMagic = 0x336F574Du; // "MWo3"
inline constexpr std::uint32_t kOverlayName = 32u;
inline constexpr std::uint32_t kOverlayHeader = 64u;
inline constexpr std::string_view kQuestOverlay = "game_task.ovl";
inline constexpr std::string_view kVillageOverlay = "lobby_task.ovl";
inline constexpr std::string_view kSubOverlay = "game_sub.ovl";

// True when the image in `slot` is `name`, terminator included, so a longer
// name that starts the same is not it.
template <typename Memory>
[[nodiscard]] bool overlay_loaded(const Memory &memory, std::uint32_t slot, std::string_view name) {
    if (!memory.contains(slot, kOverlayHeader) || memory.load32(slot) != kOverlayMagic ||
        memory.load32(slot + 8u) != slot)
        return false;
    const std::uint32_t at = slot + kOverlayName;
    for (std::size_t i = 0; i < name.size(); ++i)
        if (memory.load8(at + static_cast<std::uint32_t>(i)) != static_cast<std::uint8_t>(name[i])) return false;
    return memory.load8(at + static_cast<std::uint32_t>(name.size())) == 0u;
}

// A quest runs while game_task.ovl is in the task slot.
template <typename Memory> [[nodiscard]] bool quest_running(const Memory &memory) {
    return overlay_loaded(memory, kTaskSlot, kQuestOverlay);
}

// The camera object and the hunter it follows ----------------------------------

// The word here is the camera object (0x089D0000 - 17148). Its near and far
// planes and aspect are at +0 (camera/game_aspect.cpp), the projection at
// +0xC00 and the view at +0xC80 (camera/lock_on.cpp). 0x08886EF8 loads the
// hunter the camera follows from +2692.
inline constexpr std::uint32_t kCameraObject = 0x089CBD04u;
inline constexpr std::uint32_t kFollowedHunter = 2692u;

// The camera object, or 0 when its pointer is not set yet.
template <typename Memory> [[nodiscard]] std::uint32_t camera_object(const Memory &memory) {
    if (!memory.contains(kCameraObject, 4u)) return 0u;
    const std::uint32_t object = memory.load32(kCameraObject);
    return in_ram(object) ? object : 0u;
}

// The hunter the camera follows, with `extent` bytes of it readable, or 0.
template <typename Memory>
[[nodiscard]] std::uint32_t followed_hunter(const Memory &memory, std::uint32_t extent) {
    const std::uint32_t object = camera_object(memory);
    if (object == 0u || !memory.contains(object + kFollowedHunter, 4u)) return 0u;
    const std::uint32_t hunter = memory.load32(object + kFollowedHunter);
    return in_ram(hunter) && memory.contains(hunter, extent) ? hunter : 0u;
}

// The item atlas: the 256 by 256 CLUT8 sheet at this GE texture address. The
// HUD draws item icons from it (game/item_atlas.hpp), and the marks on the
// area map too (gpu/hud_scale.hpp).
inline constexpr std::uint32_t kItemAtlas = 0x091874A0u;

// GameSys: the word at kGameSysPointer. The loaded stage is the halfword at
// +0x6AF0E.
inline constexpr std::uint32_t kGameSysPointer = 0x089CC558u;
inline constexpr std::uint32_t kGameSysStage = 0x6AF0Eu;

// The quest entity list --------------------------------------------------------

// The word at kEntityList is the entity manager. Its list head is the newest
// node; +4 of a node is the next older one, +12 the entity, +36 a type byte.
// Type 1 is a player. The insert is game_sub 0x09C57CA0.
inline constexpr std::uint32_t kEntityList = 0x09D19860u;
inline constexpr std::uint32_t kListHead = 0x19030u;
inline constexpr std::uint32_t kNodeOlder = 0x4u;
inline constexpr std::uint32_t kNodeEntity = 0xCu;
inline constexpr std::uint32_t kNodeType = 0x24u;
inline constexpr std::uint8_t kPlayerNode = 1u;
// game_sub 0x09C899E0 stores this halfword. em overlays pass 32, 34 or 36.
inline constexpr std::uint32_t kNodeClass = 58u;
inline constexpr std::uint32_t kNodeBytes = kNodeClass + 2u;
inline constexpr std::uint32_t kMaxMonsters = 64u; // a bound on a corrupt list

// An entity. +0 is the vtable. A large monster's methods in the first slots
// lie in the em overlay; the word at +8 is the one those overlays replace.
inline constexpr std::uint32_t kVtable = 0x0u;
inline constexpr std::uint32_t kVtableMethod = 0x8u;
inline constexpr std::uint32_t kVtableSpan = 64u;
// A hunter's vtable: the player's, and each partner's in an ad hoc quest.
// Their list nodes are not type 1 (0, 3, 4 and 7 have been seen), they keep a
// hunter's health pool, and +0x1E8 is not a species: it has read 7, 10 and
// small species, which put a Lao-Shan Lung or a Rathian icon on a hunter.
inline constexpr std::uint32_t kHunterVtable = 0x089BB3CCu;
// World position, three floats, y up.
inline constexpr std::uint32_t kPosition = 0x200u;
// Species. 0x088135D0 loads this byte and compares it with species constants.
// The health halfword is read from the same pointer, so the id stays on a
// monster whose position is held in another area.
inline constexpr std::uint32_t kMonsterId = 0x1E8u;
// Zone. 0x08813630 loads this halfword and compares it with the loaded stage,
// the halfword at GameSys + kGameSysStage. A mismatch is another zone.
inline constexpr std::uint32_t kStageId = 0x29Au;
// Health and most health, signed halfwords. Most health is 0 on a companion.
inline constexpr std::uint32_t kHealth = 0x2E4u;
inline constexpr std::uint32_t kMostHealth = 0x41Eu;
// Every field above lies inside this.
inline constexpr std::uint32_t kExtent = 0x420u;

struct EntityNode {
    std::uint32_t node{};
    std::uint32_t entity{}; // as stored: not checked
    std::uint8_t type{};
    std::uint16_t class_id{};
};

// The entity manager, or 0 when the list cannot be read. Not tied to a
// quest: callers that need one check quest_running() first.
template <typename Memory> [[nodiscard]] std::uint32_t entity_manager(const Memory &memory) {
    if (!memory.contains(kEntityList, 4u)) return 0u;
    const std::uint32_t manager = memory.load32(kEntityList);
    return in_ram(manager) && memory.contains(manager + kListHead, 4u) ? manager : 0u;
}

// Calls `visit(const EntityNode &)` for each node from the newest, at most
// kMaxMonsters of them. The walk stops at a node outside RAM or not readable
// through its class, and at a node already visited, so a cyclic list is
// walked once. Returns the number of nodes visited.
template <typename Memory, typename Visit> std::uint32_t for_each_entity(const Memory &memory, Visit &&visit) {
    const std::uint32_t manager = entity_manager(memory);
    if (manager == 0u) return 0u;
    std::uint32_t seen[kMaxMonsters]{};
    std::uint32_t count = 0u;
    std::uint32_t node = memory.load32(manager + kListHead);
    while (count < kMaxMonsters && in_ram(node) && memory.contains(node, kNodeBytes)) {
        for (std::uint32_t earlier = 0; earlier < count; ++earlier)
            if (seen[earlier] == node) return count;
        seen[count++] = node;
        const EntityNode entry{node, memory.load32(node + kNodeEntity), memory.load8(node + kNodeType),
            memory.load16(node + kNodeClass)};
        node = memory.load32(node + kNodeOlder);
        visit(entry);
    }
    return count;
}

// True when `entity` is in RAM and its fields up to kExtent can be read.
template <typename Memory> [[nodiscard]] bool entity_readable(const Memory &memory, std::uint32_t entity) {
    return in_ram(entity) && memory.contains(entity, kExtent);
}

} // namespace mhfu::game::layout
