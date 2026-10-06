#pragma once

#include <cstdint>
#include <functional>
#include <span>
#include <vector>

namespace mhfu::install {

// Turns PSP_GAME/SYSDIR/EBOOT.BIN of a supported disc into the executable the
// installer stores.
//
// This is deliberately not a general tool: it accepts only the files listed in
// kSupportedReleases, handles only the header layout each of those files uses,
// and checks the result against that release's executable hash. Anything else
// is refused with psprecomp::Error. Only the installer calls it, with a
// callback that receives the bytes decrypted so far and the total.
[[nodiscard]] std::vector<std::uint8_t> prepare_executable(std::span<const std::uint8_t> eboot_bin,
    const std::function<void(std::uint64_t done, std::uint64_t total)> &progress = {});

} // namespace mhfu::install
