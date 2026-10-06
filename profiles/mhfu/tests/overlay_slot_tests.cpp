// The four overlay bases read from USA Freedom Unite's DATA.BIN. Addresses
// seen interpreted during a village boot must land in the slot the archive
// names, and the Portable 3rd bases must not.
#include "mhfu_profile.hpp"

#include <cstdlib>
#include <iostream>

namespace {
int failures = 0;

void check(bool condition, const char *message) {
    if (!condition) {
        ++failures;
        std::cerr << "FAIL: " << message << '\n';
    }
}

void slots() {
    using mhfu::kOverlaySlots;
    using mhfu::overlay_slot_of;
    check(std::size(kOverlaySlots) == 5u, "four overlay bases and the end of user memory");
    check(kOverlaySlots[0] == 0x09A5F300u, "task overlays");
    check(kOverlaySlots[1] == 0x09C19100u, "sub overlays");
    check(kOverlaySlots[2] == 0x09D1A280u, "monster overlays");
    check(kOverlaySlots[3] == 0x09D63100u, "stage overlays");
    check(kOverlaySlots[4] == 0x0A000000u, "slots stop at the end of 32 MiB user memory");
    for (std::size_t i = 0; i + 1u < std::size(kOverlaySlots); ++i)
        check(kOverlaySlots[i] < kOverlaySlots[i + 1u], "overlay bases increase");

    check(overlay_slot_of(0x09AB8690u).first == 0x09A5F300u, "village game_task code is in the task slot");
    check(overlay_slot_of(0x09C1A1E8u).first == 0x09C19100u, "the story movie player is in the sub slot");
    check(overlay_slot_of(0x09C38CA8u).first == 0x09C19100u, "game_sub code is in the sub slot");
    check(overlay_slot_of(0x09D1A280u).first == 0x09D1A280u, "a monster overlay starts its slot");
    check(overlay_slot_of(0x09D63100u).first == 0x09D63100u, "a stage overlay starts its slot");
    check(overlay_slot_of(0x08804000u).first == 0u, "the executable's load base is not an overlay");
    check(overlay_slot_of(0x0A000000u).first == 0u, "the end address is not itself a slot");
    check(overlay_slot_of(0x0A001780u).first == 0u, "a Portable 3rd overlay base is not a slot");
}
} // namespace

int main() {
    slots();
    if (failures != 0) {
        std::cerr << failures << " overlay slot check(s) failed\n";
        return EXIT_FAILURE;
    }
    std::cout << "overlay slot tests passed\n";
    return EXIT_SUCCESS;
}
