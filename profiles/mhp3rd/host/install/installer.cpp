#include "install/installer.hpp"

#include "install/executable_preparation.hpp"
#include "install/game_identity.hpp"
#include "install/user_data.hpp"

#include "kernel/iso_image.hpp"

#include "psprecomp/sha256.hpp"

#include <array>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <system_error>
#include <vector>

namespace mhp3rd::install {
namespace {

constexpr std::uint64_t kMiB = 1024u * 1024u;
// Room left over after copying, so the copy never fills the disk to the last byte.
constexpr std::uint64_t kFreeSpaceMargin = 64u * kMiB;

std::string display_name(const std::filesystem::path &path) { return path_to_utf8(path.filename()); }

std::string megabytes(std::uint64_t bytes) { return std::to_string((bytes + kMiB / 2u) / kMiB) + " MB"; }

std::uint32_t le16(const std::vector<std::uint8_t> &data, std::size_t offset) {
    return static_cast<std::uint32_t>(data[offset]) | (static_cast<std::uint32_t>(data[offset + 1u]) << 8u);
}

std::uint32_t le32(const std::vector<std::uint8_t> &data, std::size_t offset) {
    return le16(data, offset) | (le16(data, offset + 2u) << 16u);
}

std::vector<std::uint8_t> read_file(IsoImage &iso, const IsoImage::Entry &entry) {
    std::vector<std::uint8_t> data(entry.size);
    if (iso.read(static_cast<std::uint64_t>(entry.lba) * IsoImage::kSectorSize, data) != data.size())
        throw InstallError("The disc image is incomplete: it ends before the files it lists. The file may be truncated "
                           "or damaged; make the image again.");
    return data;
}

// String values of a PARAM.SFO (PSF) file.
std::map<std::string, std::string> parse_sfo(const std::vector<std::uint8_t> &sfo) {
    std::map<std::string, std::string> values;
    if (sfo.size() < 20u || std::memcmp(sfo.data(), "\0PSF", 4u) != 0) return values;
    const std::uint32_t key_table = le32(sfo, 8u);
    const std::uint32_t data_table = le32(sfo, 12u);
    const std::uint32_t count = le32(sfo, 16u);
    for (std::uint32_t i = 0; i < count; ++i) {
        const std::size_t entry = 20u + static_cast<std::size_t>(i) * 16u;
        if (entry + 16u > sfo.size()) break;
        const std::size_t key = key_table + le16(sfo, entry);
        const std::uint32_t format = le16(sfo, entry + 2u);
        const std::uint32_t length = le32(sfo, entry + 4u);
        const std::size_t value = static_cast<std::size_t>(data_table) + le32(sfo, entry + 12u);
        if (key >= sfo.size() || value + length > sfo.size() || format != 0x0204u) continue;
        std::string name;
        for (std::size_t p = key; p < sfo.size() && sfo[p] != 0u; ++p) name.push_back(static_cast<char>(sfo[p]));
        std::string text(reinterpret_cast<const char *>(sfo.data() + value), length);
        if (const auto nul = text.find('\0'); nul != std::string::npos) text.resize(nul);
        values[name] = text;
    }
    return values;
}

struct Inspection {
    ImageInfo info;
    std::vector<std::uint8_t> eboot_bin;
};

Inspection inspect(const std::filesystem::path &path) {
    const std::string name = display_name(path);
    std::error_code ec;
    if (!std::filesystem::is_regular_file(path, ec))
        throw InstallError("\"" + path_to_utf8(path) + "\" is not a file Yakumo can open.");
    {
        std::ifstream in(path, std::ios::binary);
        std::array<char, 4> magic{};
        if (!in.read(magic.data(), magic.size()))
            throw InstallError("\"" + name + "\" is empty or cannot be read.");
        if (std::memcmp(magic.data(), "CISO", 4u) == 0 || std::memcmp(magic.data(), "ZISO", 4u) == 0)
            throw InstallError("\"" + name + "\" is a compressed image. Yakumo needs an uncompressed .iso image of the "
                               "disc; decompress it first.");
    }

    std::optional<IsoImage> iso;
    try {
        iso.emplace(path);
    } catch (const std::exception &) {
        throw InstallError("\"" + name + "\" is not a disc image Yakumo can read. Choose an uncompressed .iso image "
                           "of the game's PSP disc.");
    }

    const auto sfo_entry = iso->find(kParamSfoPathOnDisc);
    if (!sfo_entry || sfo_entry->directory) {
        if (iso->find("PS3_GAME/PARAM.SFO"))
            throw InstallError("\"" + name + "\" is a PlayStation 3 disc image. Yakumo needs the PSP disc image of " +
                               kGameTitle + ": an .iso whose top level holds a PSP_GAME folder.");
        throw InstallError("\"" + name + "\" is not a PSP game disc image: it has no PSP_GAME/PARAM.SFO.");
    }
    const auto sfo = parse_sfo(read_file(*iso, *sfo_entry));
    const auto disc_id = sfo.find("DISC_ID");
    if (disc_id == sfo.end())
        throw InstallError("\"" + name + "\" has no disc id in PSP_GAME/PARAM.SFO, so it is not an image Yakumo "
                           "supports.");
    if (disc_id->second != kDiscId) {
        const auto title = sfo.find("TITLE");
        std::string what = "\"" + name + "\" is ";
        what += title != sfo.end() && !title->second.empty() ? title->second + " (" + disc_id->second + ")"
                                                              : "disc " + disc_id->second;
        what += ". Yakumo supports only " + std::string(kGameTitle) + ", the Japanese release with disc id " +
                kDiscIdDisplay + ".";
        if (disc_id->second == "ULJM05800")
            what += " This is the original PSP release of the game, which Yakumo does not support.";
        else
            what += " Other releases and regions are not supported.";
        throw InstallError(what);
    }

    const auto eboot_entry = iso->find(kExecutablePathOnDisc);
    if (!eboot_entry || eboot_entry->directory)
        throw InstallError("\"" + name + "\" has the right disc id but no PSP_GAME/SYSDIR/EBOOT.BIN. The image is "
                           "incomplete or modified; make it again from your disc.");
    Inspection result;
    result.eboot_bin = read_file(*iso, *eboot_entry);
    if (psprecomp::sha256_bytes(result.eboot_bin) != kEncryptedExecutableSha256)
        throw InstallError("\"" + name + "\" is " + kGameTitle + " (" + kDiscIdDisplay +
                           "), but its executable is not the version Yakumo supports. The image may be patched, "
                           "modified or damaged; make it again from an unmodified disc.");
    result.info.size_bytes = iso->size_bytes();
    return result;
}

void copy_with_progress(const std::filesystem::path &from, const std::filesystem::path &to, std::uint64_t total,
                        const ProgressFn &progress) {
    const std::string stage = "Copying the disc image";
    std::ifstream in(from, std::ios::binary);
    std::ofstream out(to, std::ios::binary | std::ios::trunc);
    if (!in) throw InstallError("Cannot read \"" + path_to_utf8(from) + "\".");
    if (!out) throw InstallError("Cannot write \"" + path_to_utf8(to) + "\".");
    std::vector<char> buffer(8u * kMiB);
    std::uint64_t done = 0u;
    progress(stage, 0u, total);
    while (in) {
        in.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
        const std::streamsize got = in.gcount();
        if (got <= 0) break;
        if (!out.write(buffer.data(), got))
            throw InstallError("Writing the copy of the disc image failed. Check that the disk has free space.");
        done += static_cast<std::uint64_t>(got);
        progress(stage, done, total);
    }
    out.close();
    if (!out || done != total) throw InstallError("Copying the disc image failed after " + megabytes(done) + ".");
}

void write_file(const std::filesystem::path &path, const std::vector<std::uint8_t> &data) {
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    out.write(reinterpret_cast<const char *>(data.data()), static_cast<std::streamsize>(data.size()));
    out.close();
    if (!out) throw InstallError("Cannot write \"" + path_to_utf8(path) + "\".");
}

bool same_file(const std::filesystem::path &a, const std::filesystem::path &b) {
    std::error_code ec;
    return std::filesystem::exists(b, ec) && std::filesystem::equivalent(a, b, ec);
}

void install_checked(const std::filesystem::path &image, ImageStorage storage, const std::filesystem::path &data_dir,
                     const ProgressFn &progress) {
    Inspection inspection = inspect(image);
    std::filesystem::create_directories(data_dir);

    const std::filesystem::path copied = data_dir / kCopiedImageFile;
    const std::filesystem::path copying = data_dir / (std::string(kCopiedImageFile) + ".part");
    const bool copy = storage == ImageStorage::Copy && !same_file(image, copied);
    if (copy) {
        const std::uint64_t needed = inspection.info.size_bytes + kFreeSpaceMargin;
        std::error_code ec;
        const auto space = std::filesystem::space(data_dir, ec);
        if (!ec && space.available < needed)
            throw InstallError("Not enough free space to copy the disc image: it needs " + megabytes(needed) +
                               " in \"" + path_to_utf8(data_dir) + "\" and " + megabytes(space.available) +
                               " is free. Free up space, or choose to use the image where it is.");
        try {
            copy_with_progress(image, copying, inspection.info.size_bytes, progress);
        } catch (...) {
            std::filesystem::remove(copying, ec);
            throw;
        }
        // Prepare from the copy, which also checks that it reads back intact.
        try {
            inspection = inspect(copying);
        } catch (const InstallError &) {
            std::filesystem::remove(copying, ec);
            throw InstallError("The copy of the disc image does not match the original. Check the disk and try "
                               "again.");
        }
    }

    progress("Preparing the executable", 0u, 1u);
    const std::vector<std::uint8_t> executable = prepare_executable(inspection.eboot_bin);
    const std::filesystem::path executable_path = data_dir / kExecutableFile;
    const std::filesystem::path executable_partial = data_dir / (std::string(kExecutableFile) + ".part");
    write_file(executable_partial, executable);
    progress("Preparing the executable", 1u, 1u);

    UserSettings settings;
    if (storage == ImageStorage::Copy) {
        if (copy) std::filesystem::rename(copying, copied);
        settings.disc_image = kCopiedImageFile;
    } else {
        settings.disc_image = std::filesystem::absolute(image);
    }
    save_settings(data_dir, settings);
    std::filesystem::rename(executable_partial, executable_path);

    // A copy left from an earlier setup is no longer used.
    std::error_code ec;
    if (storage == ImageStorage::InPlace && !same_file(image, copied)) std::filesystem::remove(copied, ec);
}

} // namespace

ImageInfo check_image(const std::filesystem::path &path) { return inspect(path).info; }

void install(const std::filesystem::path &image, ImageStorage storage, const std::filesystem::path &data_dir,
             const ProgressFn &progress) {
    try {
        install_checked(image, storage, data_dir, progress);
    } catch (const InstallError &) {
        throw;
    } catch (const std::filesystem::filesystem_error &e) {
        throw InstallError("Setup could not write to \"" + path_to_utf8(data_dir) + "\": " + e.code().message() + ".");
    } catch (const std::exception &e) {
        throw InstallError(std::string("Setup failed: ") + e.what() + ".");
    }
}

bool run_installer(InstallerUi &ui, const std::filesystem::path &data_dir) {
    if (!ui.introduce(data_dir)) return false;
    for (;;) {
        const auto image = ui.choose_image();
        if (!image) return false;
        try {
            const ImageInfo info = check_image(*image);
            const auto storage = ui.choose_storage(*image, info, data_dir);
            if (!storage) return false;
            install(*image, *storage, data_dir,
                    [&ui](const std::string &stage, std::uint64_t done, std::uint64_t total) {
                        ui.progress(stage, done, total);
                    });
            ui.finished(data_dir);
            return true;
        } catch (const InstallError &e) {
            std::cerr << "Setup: " << e.what() << "\n";
            if (!ui.offer_retry(e.what())) return false;
        }
    }
}

void print_progress(const std::string &stage, std::uint64_t done, std::uint64_t total) {
    static std::string last_stage;
    static std::uint64_t last_step = 0u;
    const std::uint64_t percent = total == 0u ? 100u : done * 100u / total;
    const std::uint64_t step = percent / 5u;
    if (stage == last_stage && step == last_step) return;
    last_stage = stage;
    last_step = step;
    std::cout << stage << ": " << percent << "%";
    if (total > kMiB) std::cout << " (" << megabytes(done) << " of " << megabytes(total) << ")";
    std::cout << std::endl;
}

} // namespace mhp3rd::install
