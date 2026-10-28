#pragma once

#include "install/game_identity.hpp"

#include "psprecomp/elf32.hpp"
#include "psprecomp/runtime.hpp"

#include <cstdint>
#include <filesystem>
#include <iterator>
#include <span>
#include <utility>

namespace mhfu {

inline constexpr std::uint32_t kLoadBase = psprecomp::kDefaultPspUserLoadBase;
inline constexpr std::uint32_t kGuestRamBytes = 64u * 1024u * 1024u;

struct ProfilePaths {
    std::filesystem::path disc_image;   // UMD ISO; empty disables disc0:
    std::filesystem::path memory_stick; // host directory backing ms0:
};

// Installs the kernel and HLE modules, binds logging stubs for the remaining
// imports (unless MHFU_STRICT_HLE is set) and prepares the loader thread
// that runs module_start.
void install_profile(psprecomp::Runtime &runtime, const psprecomp::Elf32Image &elf, const ProfilePaths &paths);

// The host directory that backs ms0:, where the game's saves live under
// PSP/SAVEDATA. This is the only place that decides it; the rest of the host
// receives the result, so moving saves to a per-user location changes only
// this function.
[[nodiscard]] std::filesystem::path memory_stick_directory(const std::filesystem::path &game_dir);

// Load addresses of the MWo3 overlays in USA Freedom Unite's DATA.BIN, read
// from that archive: 298 images, four bases. Each image's header, code and
// data end before the next base. The last value is the end of the 32 MiB
// user map, not another overlay. The game copies one image into a base at
// a time; a jump there has no corpus until that image is recompiled.
inline constexpr std::uint32_t kOverlaySlots[] = {
    0x09A5F300u, // *_task.ovl (demo_task, game_task, movie_task, ...)
    0x09C19100u, // demo_sub.ovl, game_sub.ovl
    0x09D1A280u, // em*.ovl
    0x09D63100u, // stage*.ovl
    0x0A000000u, // end of user memory
};

// The same four bases in Freedom Unite Complete's DATA.BIN, also 298 images,
// each a little below USA Freedom Unite's (0x4D80 to 0x5180).
inline constexpr std::uint32_t kCompleteOverlaySlots[] = {
    0x09A5A580u, // *_task.ovl
    0x09C14280u, // demo_sub.ovl, game_sub.ovl
    0x09D15100u, // em*.ovl
    0x09D5DF80u, // stage*.ovl
    0x0A000000u, // end of user memory
};

// The slots of the release that runs; USA Freedom Unite's before launch.
[[nodiscard]] inline std::span<const std::uint32_t> overlay_slots() {
    if (install::running_release() == &install::kFreedomUniteComplete) return kCompleteOverlaySlots;
    return kOverlaySlots;
}

// The slot containing `address` in `slots`, or {0, 0} when it is outside every slot.
[[nodiscard]] inline std::pair<std::uint32_t, std::uint32_t> overlay_slot_of(
    std::uint32_t address, std::span<const std::uint32_t> slots) {
    for (std::size_t i = 0; i + 1u < slots.size(); ++i) {
        if (address >= slots[i] && address < slots[i + 1u]) return {slots[i], slots[i + 1u]};
    }
    return {0u, 0u};
}

// The slot containing `address` in the running release.
[[nodiscard]] inline std::pair<std::uint32_t, std::uint32_t> overlay_slot_of(std::uint32_t address) {
    return overlay_slot_of(address, overlay_slots());
}

} // namespace mhfu
