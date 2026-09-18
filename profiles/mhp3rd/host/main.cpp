#include "mhp3rd_profile.hpp"

#include "install/game_identity.hpp"
#include "install/installer.hpp"
#include "install/user_data.hpp"
#include "kernel/kernel.hpp"

#include "psprecomp/common.hpp"
#include "psprecomp/elf32.hpp"
#include "psprecomp/runtime.hpp"
#include "psprecomp/sha256.hpp"

#include <cerrno>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <limits>
#include <optional>
#include <string>

namespace {

std::uint64_t configured_max_dispatches() {
    constexpr std::uint64_t default_limit = 4'000'000'000ull;
    const char *text = std::getenv("PSPRECOMP_MAX_DISPATCHES");
    if (text == nullptr || *text == '\0') return default_limit;
    errno = 0;
    char *end = nullptr;
    const unsigned long long parsed = std::strtoull(text, &end, 0);
    if (errno == ERANGE || end == text || *end != '\0' || parsed == 0u)
        throw psprecomp::Error(std::string("Invalid PSPRECOMP_MAX_DISPATCHES value: ") + text);
    return static_cast<std::uint64_t>(parsed);
}

constexpr const char *kUsage =
    "usage: MHP3rdNative [game_dir]\n"
    "       MHP3rdNative --install [image.iso [--in-place]]\n"
    "  game_dir        play from a directory holding EBOOT.ELF, disc.iso and ms0/\n"
    "  --install       run the setup again with dialogs, then play\n"
    "  --install image set up from image.iso without dialogs, then exit\n"
    "  --in-place      use the image where it is instead of copying it\n";

struct Options {
    std::optional<std::filesystem::path> game_dir;
    bool install = false;
    std::optional<std::filesystem::path> install_image;
    bool in_place = false;
};

class UsageError final : public std::runtime_error {
public:
    explicit UsageError(const std::string &message) : std::runtime_error(message) {}
};

Options parse_options(int argc, char **argv) {
    Options options;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") throw UsageError("");
        if (arg == "--install") options.install = true;
        else if (arg == "--in-place") options.in_place = true;
        else if (!arg.empty() && arg[0] == '-') throw UsageError("unknown option " + arg);
        else if (options.install && !options.install_image) options.install_image = mhp3rd::install::path_from_utf8(arg);
        else if (!options.install && !options.game_dir) options.game_dir = std::filesystem::path(arg);
        else throw UsageError("unexpected argument " + arg);
    }
    if (options.in_place && !options.install_image) throw UsageError("--in-place needs --install image.iso");
    if (options.install && options.game_dir) throw UsageError("--install does not take a game_dir");
    return options;
}

struct GameFiles {
    std::filesystem::path executable;
    std::filesystem::path disc_image; // empty: disc0: is unavailable
    std::filesystem::path memory_stick;
};

// The layout a game_dir has always had; profiles/mhp3rd/game by default.
GameFiles files_in_game_directory(const std::filesystem::path &game_dir) {
    GameFiles files;
    files.executable = game_dir / "EBOOT.ELF";
    files.disc_image = game_dir / "disc.iso";
    files.memory_stick = game_dir / "ms0";
    if (!std::filesystem::exists(files.disc_image)) {
        std::cerr << "warning: " << files.disc_image.string() << " not found; disc0: is unavailable\n";
        files.disc_image.clear();
    }
    return files;
}

bool has_game_data(const std::filesystem::path &game_dir) {
    std::error_code ec;
    return std::filesystem::exists(std::filesystem::symlink_status(game_dir / "EBOOT.ELF", ec)) ||
           std::filesystem::exists(std::filesystem::symlink_status(game_dir / "disc.iso", ec));
}

// Finds the game: an explicit game_dir, then the per-user data directory the
// installer fills, then profiles/mhp3rd/game. With neither, runs the installer.
// Empty when the player quit or nothing could be set up.
std::optional<GameFiles> locate_game(const Options &options) {
    namespace install = mhp3rd::install;
    if (options.game_dir) return files_in_game_directory(*options.game_dir);
    if (const char *dir = std::getenv("MHP3RD_GAME_DIR"); dir != nullptr && *dir != '\0')
        return files_in_game_directory(dir);

    const std::filesystem::path checkout_game_dir = MHP3RD_DEFAULT_GAME_DIR;
    const std::filesystem::path data_dir = install::user_data_directory();
    bool run_setup = options.install;
    for (;;) {
        if (!run_setup) {
            if (const auto installed = install::find_installation(data_dir)) {
                std::error_code ec;
                if (std::filesystem::is_regular_file(installed->disc_image, ec)) {
                    GameFiles files;
                    files.executable = installed->executable;
                    files.disc_image = installed->disc_image;
                    // Save data stays where it has always been for now.
                    files.memory_stick = checkout_game_dir / "ms0";
                    return files;
                }
                const std::string where = install::path_to_utf8(installed->disc_image);
                const std::string message =
                    installed->image_copied
                        ? "The copy of the disc image Yakumo made is missing:\n" + where +
                              "\n\nSet up again to restore it (MHP3rdNative --install)."
                        : "The disc image Yakumo was set up with is no longer at:\n" + where +
                              "\n\nPut it back there, or set up again to choose where it is now "
                              "(MHP3rdNative --install).";
                if (!install::report_problem("Disc image not found", message, true)) return std::nullopt;
                run_setup = true;
                continue;
            }
            if (has_game_data(checkout_game_dir)) return files_in_game_directory(checkout_game_dir);
        }

        auto ui = install::make_dialog_ui();
        if (!ui) {
            std::cerr << "No game data found in " << install::path_to_utf8(data_dir) << " or "
                      << checkout_game_dir.string() << ".\n"
                      << "Set up from your disc image of " << install::kGameTitle << " (" << install::kDiscIdDisplay
                      << ") with:\n  MHP3rdNative --install /path/to/image.iso\n";
            return std::nullopt;
        }
        if (!install::run_installer(*ui, data_dir)) return std::nullopt;
        run_setup = false;
    }
}

int install_from_command_line(const Options &options) {
    namespace install = mhp3rd::install;
    const std::filesystem::path data_dir = install::user_data_directory();
    try {
        install::install(*options.install_image,
                         options.in_place ? install::ImageStorage::InPlace : install::ImageStorage::Copy, data_dir,
                         install::print_progress);
    } catch (const install::InstallError &e) {
        std::cerr << "Setup failed: " << e.what() << "\n";
        return 1;
    }
    std::cout << "Game data is ready in " << install::path_to_utf8(data_dir) << ". Start MHP3rdNative to play.\n";
    return 0;
}

} // namespace

int main(int argc, char **argv) {
    try {
        Options options;
        try {
            options = parse_options(argc, argv);
        } catch (const UsageError &e) {
            if (*e.what() == '\0') {
                std::cout << kUsage;
                return 0;
            }
            std::cerr << "MHP3rdNative: " << e.what() << "\n" << kUsage;
            return 2;
        }
        if (options.install_image) return install_from_command_line(options);

        const std::optional<GameFiles> files = locate_game(options);
        if (!files) return 1;
        const std::filesystem::path &executable = files->executable;
        mhp3rd::ProfilePaths paths;
        paths.disc_image = files->disc_image;
        paths.memory_stick = files->memory_stick;
        if (!std::filesystem::is_regular_file(executable))
            throw psprecomp::Error("Missing " + executable.string() + " (run profiles/mhp3rd/scripts/prepare_game.sh)");

        const std::string sha256 = psprecomp::sha256_file(executable);
        if (sha256 != mhp3rd::install::kExecutableSha256)
            std::cerr << "warning: unsupported executable hash " << sha256 << "\n";

        const psprecomp::Elf32Image elf = psprecomp::Elf32Image::from_file(executable);
        if (elf.required_ram_size(mhp3rd::kLoadBase) != mhp3rd::kGuestRamBytes)
            throw psprecomp::Error("Executable does not match the 64 MiB MHP3rd HD layout");

        psprecomp::Runtime runtime(mhp3rd::kGuestRamBytes);
        runtime.nids().load_csv(MHP3RD_NIDS_CSV);
        (void)elf.load_and_relocate(runtime.memory(), mhp3rd::kLoadBase);
        psprecomp::register_generated_functions(runtime);
        mhp3rd::install_profile(runtime, elf, paths);

        std::cout << "MHP3rdNative PSP bootstrap\n"
                  << "Executable: " << executable.string() << "\n"
                  << "SHA-256:    " << sha256 << "\n"
                  << "Disc image: " << (paths.disc_image.empty() ? "<none>" : paths.disc_image.string()) << "\n"
                  << "Entry:      " << psprecomp::hex32(elf.runtime_entry(mhp3rd::kLoadBase)) << "\n"
                  << "Functions:  " << runtime.function_count() << "\n";
        if (runtime.function_count() == 0u) {
            std::cout << "No generated functions are linked. Run profiles/mhp3rd/scripts/generate.sh and rebuild.\n";
            return 3;
        }

        runtime.run(elf.runtime_entry(mhp3rd::kLoadBase), configured_max_dispatches());
        std::cout << "Runtime stopped: " << runtime.stop_reason() << "\n";
        std::cout << mhp3rd::kernel().describe_threads() << "\n";
        runtime.report_hle_histogram();
        return runtime.stop_reason().empty() ? 0 : 4;
    } catch (const std::exception &e) {
        std::cerr << "MHP3rdNative error: " << e.what() << "\n";
        return 1;
    }
}
