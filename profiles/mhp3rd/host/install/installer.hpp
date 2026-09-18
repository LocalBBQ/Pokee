#pragma once

#include <cstdint>
#include <filesystem>
#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>

// First-run installer: checks the player's disc image, copies it into the
// per-user data directory (or records where it is), and prepares the game's
// executable from it.
//
// The logic here knows nothing about how questions are asked. InstallerUi is
// the only thing a front end implements; the SDL message-box front end is a
// stand-in until the port has its own interface screens.
namespace mhp3rd::install {

// A problem the player can act on; what() is written for them.
class InstallError final : public std::runtime_error {
public:
    explicit InstallError(const std::string &message) : std::runtime_error(message) {}
};

enum class ImageStorage {
    Copy,    // copy the image into the data directory (default)
    InPlace, // use it where it is; it must stay there
};

struct ImageInfo {
    std::uint64_t size_bytes{};
};

// Checks that path is an unmodified image of the supported release: the disc
// id in PARAM.SFO and the hash of the encrypted executable. Throws
// InstallError saying plainly what is wrong otherwise.
ImageInfo check_image(const std::filesystem::path &path);

// Called with the bytes done so far and the total; stage names the step.
using ProgressFn = std::function<void(const std::string &stage, std::uint64_t done, std::uint64_t total)>;

// Checks the image, stores it according to storage and prepares the
// executable in data_dir. Nothing in data_dir changes until the image has
// passed its checks; files are written under temporary names and renamed at
// the end. Throws InstallError.
void install(const std::filesystem::path &image, ImageStorage storage, const std::filesystem::path &data_dir,
             const ProgressFn &progress);

class InstallerUi {
public:
    virtual ~InstallerUi() = default;
    // Explains what the installer needs. False: the player quit.
    virtual bool introduce(const std::filesystem::path &data_dir) = 0;
    // Lets the player pick the disc image. Empty: cancelled.
    virtual std::optional<std::filesystem::path> choose_image() = 0;
    // Copy the image or use it in place. Empty: cancelled.
    virtual std::optional<ImageStorage> choose_storage(const std::filesystem::path &image, const ImageInfo &info,
                                                       const std::filesystem::path &data_dir) = 0;
    virtual void progress(const std::string &stage, std::uint64_t done, std::uint64_t total) = 0;
    // Reports a failed check or step. True: pick another image; false: quit.
    virtual bool offer_retry(const std::string &message) = 0;
    virtual void finished(const std::filesystem::path &data_dir) = 0;
};

// The whole interactive flow. True when the game is installed and can start.
bool run_installer(InstallerUi &ui, const std::filesystem::path &data_dir);

// Front ends. Console progress is shared by both.
void print_progress(const std::string &stage, std::uint64_t done, std::uint64_t total);
// SDL dialogs, or null when this build or this session cannot show them.
std::unique_ptr<InstallerUi> make_dialog_ui();
// Tells the player (dialog when possible, console always) that something
// prevents the game from starting. With ask_setup, offers to run the installer
// again and returns whether the player chose to.
bool report_problem(const std::string &title, const std::string &message, bool ask_setup);

} // namespace mhp3rd::install
