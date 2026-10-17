// What several features read in the game (game/guest_layout.hpp): the overlay
// in a slot, the camera object and the hunter it follows, and the quest
// entity list. No game data: guest memory is a zero-filled buffer.
#include "game/guest_layout.hpp"
#include "game/guest_ram.hpp"
#include "mhfu_profile.hpp"

#include <cstdlib>
#include <iostream>
#include <string_view>
#include <vector>

namespace {
namespace layout = mhfu::game::layout;
using mhfu::game::BufferRam;

int failures = 0;

void check(bool condition, const char *message) {
    if (!condition) {
        ++failures;
        std::cerr << "FAIL: " << message << '\n';
    }
}

BufferRam user_ram() {
    return BufferRam(layout::kRamStart, layout::kRamEnd - layout::kRamStart);
}

void load_overlay(BufferRam &ram, std::uint32_t slot, std::string_view name) {
    ram.store32(slot, layout::kOverlayMagic);
    ram.store32(slot + 8u, slot);
    for (std::size_t i = 0; i < name.size(); ++i)
        ram.store8(slot + layout::kOverlayName + static_cast<std::uint32_t>(i), static_cast<std::uint8_t>(name[i]));
    ram.store8(slot + layout::kOverlayName + static_cast<std::uint32_t>(name.size()), 0u);
}

void ram_bounds() {
    check(layout::in_ram(layout::kRamStart) && layout::in_ram(layout::kRamEnd - 1u), "user RAM is in RAM");
    check(!layout::in_ram(layout::kRamStart - 1u) && !layout::in_ram(layout::kRamEnd), "its ends are not");
    check(!layout::in_ram(0u) && !layout::in_ram(0x04000000u), "neither is null or VRAM");
}

void slots_match_the_profile() {
    check(layout::kTaskSlot == mhfu::kOverlaySlots[0], "the task slot is the profile's");
    check(layout::kSubSlot == mhfu::kOverlaySlots[1], "the sub slot is the profile's");
    check(layout::kEmOverlay == mhfu::kOverlaySlots[2], "the em slot is the profile's");
    check(layout::kEmOverlayEnd == mhfu::kOverlaySlots[3], "the em slot ends where the stage slot starts");
    check(layout::kRamEnd == mhfu::kOverlaySlots[4], "user RAM ends where the slots end");
}

void overlays() {
    BufferRam ram = user_ram();
    check(!layout::quest_running(ram), "no quest while the task slot is empty");
    load_overlay(ram, layout::kTaskSlot, layout::kQuestOverlay);
    check(layout::quest_running(ram), "game_task.ovl in the task slot is a quest");
    check(!layout::overlay_loaded(ram, layout::kTaskSlot, layout::kVillageOverlay), "it is not lobby_task.ovl");
    check(!layout::overlay_loaded(ram, layout::kSubSlot, layout::kQuestOverlay), "nor is the sub slot loaded");

    load_overlay(ram, layout::kTaskSlot, "game_task.ovlx");
    check(!layout::quest_running(ram), "a longer name that starts the same is not game_task.ovl");
    load_overlay(ram, layout::kTaskSlot, layout::kQuestOverlay);
    ram.store32(layout::kTaskSlot, 0u);
    check(!layout::quest_running(ram), "without the MWo3 magic the slot holds no overlay");
    load_overlay(ram, layout::kTaskSlot, layout::kQuestOverlay);
    ram.store32(layout::kTaskSlot + 8u, layout::kSubSlot);
    check(!layout::quest_running(ram), "an image that names another load address is not in this slot");

    load_overlay(ram, layout::kTaskSlot, layout::kVillageOverlay);
    load_overlay(ram, layout::kSubSlot, layout::kSubOverlay);
    check(layout::overlay_loaded(ram, layout::kTaskSlot, layout::kVillageOverlay) &&
            layout::overlay_loaded(ram, layout::kSubSlot, layout::kSubOverlay) && !layout::quest_running(ram),
        "the village loads lobby_task and game_sub, and is not a quest");
}

void camera_and_hunter() {
    BufferRam ram = user_ram();
    check(layout::camera_object(ram) == 0u && layout::followed_hunter(ram, 4u) == 0u, "nothing before the pointer");

    constexpr std::uint32_t kObject = 0x09100000u;
    constexpr std::uint32_t kHunter = 0x09200000u;
    ram.store32(layout::kCameraObject, 0x04000000u);
    check(layout::camera_object(ram) == 0u, "a pointer outside RAM is no camera object");
    ram.store32(layout::kCameraObject, kObject);
    check(layout::camera_object(ram) == kObject, "the camera object is the word at its pointer");
    check(layout::followed_hunter(ram, 4u) == 0u, "no hunter while the object names none");

    ram.store32(kObject + layout::kFollowedHunter, kHunter);
    check(layout::followed_hunter(ram, 0x1000u) == kHunter, "the followed hunter is at +2692");
    check(layout::followed_hunter(ram, layout::kRamEnd - kHunter + 1u) == 0u,
        "a hunter that cannot be read as far as the caller needs is none");
    ram.store32(kObject + layout::kFollowedHunter, 0x0A000100u);
    check(layout::followed_hunter(ram, 4u) == 0u, "a hunter outside RAM is none");
}

// A manager whose list holds `nodes`, newest first. Each node's entity is
// its own address plus 0x100, with type `type`.
void build_list(BufferRam &ram, const std::vector<std::uint32_t> &nodes, std::uint8_t type = 0u) {
    constexpr std::uint32_t kManager = 0x09300000u;
    ram.store32(layout::kEntityList, kManager);
    ram.store32(kManager + layout::kListHead, nodes.empty() ? 0u : nodes.front());
    for (std::size_t i = 0; i < nodes.size(); ++i) {
        const std::uint32_t node = nodes[i];
        ram.store32(node + layout::kNodeOlder, i + 1u < nodes.size() ? nodes[i + 1u] : 0u);
        ram.store32(node + layout::kNodeEntity, node + 0x100u);
        ram.store8(node + layout::kNodeType, type);
        ram.store16(node + layout::kNodeClass, static_cast<std::uint16_t>(30u + i));
    }
}

std::vector<layout::EntityNode> walk(const BufferRam &ram) {
    std::vector<layout::EntityNode> seen;
    const std::uint32_t count =
        layout::for_each_entity(ram, [&](const layout::EntityNode &node) { seen.push_back(node); });
    check(count == seen.size(), "the walk counts the nodes it visits");
    return seen;
}

void entity_list() {
    BufferRam ram = user_ram();
    check(layout::entity_manager(ram) == 0u && walk(ram).empty(), "no manager, no nodes");
    ram.store32(layout::kEntityList, 0x00010000u);
    check(layout::entity_manager(ram) == 0u, "a manager outside RAM is none");

    build_list(ram, {0x09400000u, 0x09400100u, 0x09400200u}, layout::kPlayerNode);
    std::vector<layout::EntityNode> seen = walk(ram);
    check(seen.size() == 3u, "three nodes");
    check(seen.size() == 3u && seen[0].node == 0x09400000u && seen[2].node == 0x09400200u, "newest first");
    check(seen.size() == 3u && seen[1].entity == 0x09400200u && seen[1].type == layout::kPlayerNode &&
            seen[1].class_id == 31u,
        "each node's entity, type and class");

    build_list(ram, {0x09400000u, 0x09400100u, 0x09400200u});
    ram.store32(0x09400200u + layout::kNodeOlder, 0x09400100u);
    check(walk(ram).size() == 3u, "a cyclic list is walked once");

    build_list(ram, {0x09400000u, 0x09400100u});
    ram.store32(0x09400100u + layout::kNodeOlder, 0x0A100000u);
    check(walk(ram).size() == 2u, "the walk stops at a node outside RAM");

    build_list(ram, {0x09400000u});
    ram.store32(0x09400000u + layout::kNodeOlder, layout::kRamEnd - layout::kNodeBytes + 1u);
    check(walk(ram).size() == 1u, "and at one that cannot be read through its class");

    std::vector<std::uint32_t> long_list;
    for (std::uint32_t i = 0; i < layout::kMaxMonsters + 10u; ++i) long_list.push_back(0x09500000u + i * 0x80u);
    build_list(ram, long_list);
    check(walk(ram).size() == layout::kMaxMonsters, "a list longer than any quest's is cut at kMaxMonsters");

    check(layout::entity_readable(ram, 0x09400000u), "an entity in RAM is readable");
    check(!layout::entity_readable(ram, layout::kRamEnd - layout::kExtent + 1u), "one running past RAM is not");
    check(!layout::entity_readable(ram, 0u), "nor is a null one");
}
} // namespace

int main() {
    ram_bounds();
    slots_match_the_profile();
    overlays();
    camera_and_hunter();
    entity_list();
    if (failures != 0) {
        std::cerr << failures << " guest layout check(s) failed\n";
        return EXIT_FAILURE;
    }
    std::cout << "guest layout tests passed\n";
    return EXIT_SUCCESS;
}
