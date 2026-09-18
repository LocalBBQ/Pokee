#pragma once

#include <filesystem>
#include <vector>

namespace mhp3rd {

// The running executable, resolved through the operating system rather than
// argv[0]; empty if it cannot be determined.
[[nodiscard]] std::filesystem::path executable_path();

// The directory the executable is in. A release keeps everything it ships
// relative to it: overlays/, fonts/ and, on Linux, lib/.
[[nodiscard]] std::filesystem::path executable_directory();

// Font files a release ships in fonts/ next to the executable, as fallbacks
// after the system's own fonts.
[[nodiscard]] std::vector<std::filesystem::path> bundled_fonts();

} // namespace mhp3rd
