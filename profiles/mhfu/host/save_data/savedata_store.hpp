#pragma once

// Saves on the host, laid out as on a PSP memory stick:
//   <ms0>/PSP/SAVEDATA/<gameName><saveName>/PARAM.SFO, ICON0.PNG, ..., <fileName>
// so a folder can be copied to or from a real memory stick unchanged.
#include "save_data/aes128.hpp"

#include <chrono>
#include <cstdint>
#include <ctime>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace mhfu::savedata {

struct SaveFiles {
    std::string game_name;    // e.g. "ULUS10391"
    std::string save_name;    // appended to the game name; may be empty
    std::string file_name;    // the data file, e.g. "MHP2NDG.BIN"
    std::optional<Block> key; // game key; saves are encrypted with it, and plain without one
};

struct SaveContents {
    std::vector<std::uint8_t> data; // plaintext
    std::string title;
    std::string savedata_title;
    std::string detail;
    std::uint32_t parental_level{};
    std::vector<std::uint8_t> icon0; // ICON0.PNG
    std::vector<std::uint8_t> icon1; // ICON1.PMF
    std::vector<std::uint8_t> pic1;  // PIC1.PNG
    std::vector<std::uint8_t> snd0;  // SND0.AT3
};

enum class LoadStatus { Ok, NoData, Broken };

struct LoadResult {
    LoadStatus status{LoadStatus::NoData};
    SaveContents contents;
    std::string reason; // why a save is broken, for the log
};

// PSP/SAVEDATA under the memory stick root.
[[nodiscard]] std::filesystem::path savedata_root(const std::filesystem::path &memory_stick);
[[nodiscard]] std::filesystem::path save_folder(const std::filesystem::path &memory_stick, const SaveFiles &files);

[[nodiscard]] bool save_exists(const std::filesystem::path &memory_stick, const SaveFiles &files);
[[nodiscard]] LoadResult load_save(const std::filesystem::path &memory_stick, const SaveFiles &files);
// Writes the folder; returns false with `error` set on failure. With no file
// name, only PARAM.SFO and the icons are written, as MAKEDATA does for a game
// that adds its files afterwards.
bool write_save(const std::filesystem::path &memory_stick, const SaveFiles &files, const SaveContents &contents,
    std::string &error);
bool delete_save(const std::filesystem::path &memory_stick, const SaveFiles &files);

enum class WriteStatus { Ok, NoData, Failed };

// WRITEDATA: one plain file written into a save folder that already exists,
// leaving PARAM.SFO as it is.
WriteStatus write_data_file(const std::filesystem::path &memory_stick, const SaveFiles &files,
    const std::vector<std::uint8_t> &data, std::string &error);

// Total size of the folder's files in bytes, 0 when it does not exist.
[[nodiscard]] std::uint64_t save_size(const std::filesystem::path &memory_stick, const SaveFiles &files);

struct ListedFile {
    std::string name;
    std::uint64_t size{};
    std::filesystem::file_time_type modified;
};

// FILES: a save folder's files in name order, split as a PSP lists them.
// System files are PARAM.SFO and the icons, secure files those PARAM.SFO
// lists with a hash, and normal files the rest.
struct FileListing {
    bool exists{};
    std::vector<ListedFile> secure;
    std::vector<ListedFile> normal;
    std::vector<ListedFile> system;
};
[[nodiscard]] FileListing list_files(const std::filesystem::path &memory_stick, const SaveFiles &files);

// A file time on the system clock, and a time broken down in local time.
[[nodiscard]] std::chrono::system_clock::time_point to_system(std::filesystem::file_time_type time);
[[nodiscard]] std::tm local_time(std::chrono::system_clock::time_point time);

} // namespace mhfu::savedata
