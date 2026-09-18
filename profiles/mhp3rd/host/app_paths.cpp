#include "app_paths.hpp"

#include <algorithm>
#include <cstdint>
#include <string>
#include <system_error>

#if defined(_WIN32)
#include <windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#endif

namespace mhp3rd {

std::filesystem::path executable_path() {
#if defined(_WIN32)
    std::wstring buffer(MAX_PATH, L'\0');
    for (;;) {
        const DWORD written = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
        if (written == 0u) return {};
        if (written < buffer.size()) return std::filesystem::path(buffer.substr(0, written));
        buffer.resize(buffer.size() * 2u);
    }
#elif defined(__APPLE__)
    std::uint32_t size = 0u;
    _NSGetExecutablePath(nullptr, &size);
    std::string buffer(size, '\0');
    if (_NSGetExecutablePath(buffer.data(), &size) != 0) return {};
    return std::filesystem::canonical(buffer.c_str());
#else
    std::error_code ec;
    const std::filesystem::path self = std::filesystem::read_symlink("/proc/self/exe", ec);
    return ec ? std::filesystem::path{} : self;
#endif
}

std::filesystem::path executable_directory() {
    const std::filesystem::path executable = executable_path();
    return executable.empty() ? std::filesystem::path{} : executable.parent_path();
}

std::vector<std::filesystem::path> bundled_fonts() {
    std::vector<std::filesystem::path> fonts;
    const std::filesystem::path directory = executable_directory();
    if (directory.empty()) return fonts;
    std::error_code ec;
    for (const auto &entry : std::filesystem::directory_iterator(directory / "fonts", ec)) {
        const std::filesystem::path extension = entry.path().extension();
        if (extension == ".otf" || extension == ".ttf" || extension == ".ttc") fonts.push_back(entry.path());
    }
    std::sort(fonts.begin(), fonts.end());
    return fonts;
}

} // namespace mhp3rd
