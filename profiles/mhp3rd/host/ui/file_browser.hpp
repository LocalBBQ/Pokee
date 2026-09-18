#pragma once

#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <vector>

// The setup's own file browser. The system file dialog may not appear at all
// under gamescope (Steam Deck Game Mode), so picking the disc image must work
// with a gamepad alone: folders open with confirm, back goes up a folder,
// common places (home, downloads, SD cards and other removable drives) are one
// press away, and only .iso files are listed unless the player asks for all.
namespace mhp3rd::ui {

class FileBrowser {
public:
    // Starts in `folder`, or the home folder when it does not exist.
    explicit FileBrowser(const std::filesystem::path &folder);
    ~FileBrowser();
    FileBrowser(const FileBrowser &) = delete;
    FileBrowser &operator=(const FileBrowser &) = delete;

    enum class Result { Browsing, Chosen, Cancelled };
    // Draws the browser as the content of the current panel.
    Result frame(bool back);
    [[nodiscard]] const std::filesystem::path &chosen() const noexcept { return chosen_; }
    [[nodiscard]] const std::filesystem::path &folder() const noexcept { return folder_; }

    // The player's home folder.
    static std::filesystem::path home();

private:
    struct Entry {
        std::filesystem::path path;
        std::string name;
        bool directory{};
        std::uint64_t size{};
    };
    struct Place {
        std::string name;
        std::filesystem::path path;
    };
    struct SystemDialog;

    // Lists `folder`, focusing `focus` when it is one of its entries. The
    // latter is taken by value: callers pass folder_ itself.
    void open(const std::filesystem::path &folder, std::filesystem::path focus = {});
    void find_places();

    std::filesystem::path folder_;
    std::filesystem::path chosen_;
    std::vector<Entry> entries_;
    std::vector<Place> places_;
    std::string error_;
    bool show_all_{};
    std::size_t hidden_files_{};
    // Entry to put the focus on when the list is next drawn.
    std::optional<std::filesystem::path> focus_;
    bool focus_first_{};
    std::shared_ptr<SystemDialog> dialog_;
};

// "1.3 GB", "532 MB", "12 KB".
std::string human_size(std::uint64_t bytes);

} // namespace mhp3rd::ui
