// Checks for the texture pack import that need no game data: finding a pack
// in each layout packs come in, the [games] mapping for a pack named for
// another release, the checks made before copying (the hash, key and file
// counts, missing images), and the copy, the swap and the backup it makes.
//
//   mhp3rd_texture_pack_tests
//   mhp3rd_texture_pack_tests --check <folder>
//
// The second form prints what the menu's import would find in a folder, for
// example a real pack, and reads nothing but that folder.
#include "gpu/texture_pack.hpp"
#include "gpu/texture_pack_import.hpp"

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <random>
#include <string>
#include <thread>

using namespace mhp3rd::gpu;
namespace fs = std::filesystem;

namespace {

constexpr const char *kGame = "NPJB40001";

int failures = 0;

void check(bool condition, const std::string &what) {
    std::printf("%s %s\n", condition ? "ok  " : "FAIL", what.c_str());
    if (!condition) ++failures;
}

void write(const fs::path &path, const std::string &text) {
    fs::create_directories(path.parent_path());
    std::ofstream(path, std::ios::binary) << text;
}

std::string read(const fs::path &path) {
    std::ifstream file(path, std::ios::binary);
    return {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
}

const char *const kIni = "[options]\n"
                         "version = 1\n"
                         "hash = xxh64\n"
                         "ignoreAddress = true\n"
                         "\n"
                         "[hashes]\n"
                         "0000000022585cbda625131a = ui/capcom.png\n"
                         "000000000eede62bedb3dc35 = ui/Dolby.png\n"
                         "00000000f37ab20f00000000 =\n"
                         "000000002f265a2b00000000 = ui/missing.png\n";

// A pack of three keys naming two images that exist (one in another case)
// and one that does not, plus a key that keeps the original texture.
void make_pack(const fs::path &folder, const std::string &ini = kIni) {
    write(folder / "textures.ini", ini);
    write(folder / "ui" / "capcom.png", "capcom");
    write(folder / "ui" / "dolby.png", "dolby!");
    write(folder / "ui" / "notes.txt", "x");
    write(folder / ".DS_Store", "finder");
}

struct Scratch {
    fs::path root;
    Scratch() {
        std::random_device random;
        root = fs::temp_directory_path() / ("mhp3rd_texture_pack_tests_" + std::to_string(random()));
        fs::create_directories(root);
    }
    ~Scratch() {
        std::error_code ec;
        fs::remove_all(root, ec);
    }
};

void test_layouts(const fs::path &root) {
    // The pack folder itself.
    const fs::path itself = root / "layouts" / "MyPack";
    make_pack(itself);
    TexturePackCheck c = check_texture_pack(itself, kGame);
    check(c.ok() && c.folder == itself, "the folder holding textures.ini is the pack");
    check(c.keys == 4u, "four keys are counted");
    check(c.hash == TexturePackHash::Xxh64 && c.ignore_address, "the options are read");
    check(c.images == 2u, "two image files are counted");
    check(c.files == 4u, "hidden files are left out of the copy");
    check(c.bytes == std::string(kIni).size() + 6u + 6u + 1u, "the copy's size is every file's");
    check(c.missing == 1u && c.missing_names.size() == 1u && c.missing_names[0] == "ui/missing.png",
          "a listed image that is not there is counted, one in another case is not");
    check(c.made_for.empty(), "a pack under its own name is not marked as another release's");

    const struct {
        fs::path chosen;
        fs::path pack;
        const char *what;
    } layouts[] = {
        {root / "a", root / "a" / "textures" / kGame, "textures/NPJB40001 in the chosen folder"},
        {root / "b", root / "b" / kGame, "NPJB40001 in the chosen folder"},
        {root / "c", root / "c" / "PSP" / "TEXTURES" / kGame, "PPSSPP's PSP/TEXTURES/NPJB40001"},
        {root / "d", root / "d" / "psp" / "textures" / "npjb40001", "psp/textures/npjb40001, in another case"},
    };
    for (const auto &layout : layouts) {
        make_pack(layout.pack);
        const TexturePackCheck found = check_texture_pack(layout.chosen, kGame);
        check(found.ok() && found.folder == layout.pack, std::string("found in ") + layout.what);
    }

    const TexturePackCheck none = check_texture_pack(root / "layouts", kGame);
    check(!none.found() && !none.problem.empty(), "a folder of unrelated folders has no pack");
    write(root / "zipped" / "pack.zip", "PK");
    const TexturePackCheck zip = check_texture_pack(root / "zipped", kGame);
    check(!zip.found() && zip.problem.find("unpack it first") != std::string::npos,
          "a .zip beside no pack says to unpack it");
    write(root / "zipped2" / kGame / "textures.zip", "PK");
    const TexturePackCheck textures_zip = check_texture_pack(root / "zipped2", kGame);
    check(textures_zip.found() && !textures_zip.ok() && textures_zip.problem.find("Unpack") != std::string::npos,
          "a textures.zip pack is refused with \"unpack it first\"");
}

void test_games(const fs::path &root) {
    const std::string listed = std::string(kIni) + "\n[games]\nULJM05800 = true\nNPJB40001 = true\n";
    // Chosen directly.
    const fs::path direct = root / "games" / "ULJM05800";
    make_pack(direct, listed);
    TexturePackCheck c = check_texture_pack(direct, kGame);
    check(c.ok() && c.made_for == "ULJM05800", "a ULJM05800 pack listing NPJB40001 is taken as made for ULJM05800");
    // Found under a TEXTURES folder.
    make_pack(root / "games2" / "PSP" / "TEXTURES" / "ULJM05800", listed);
    c = check_texture_pack(root / "games2", kGame);
    check(c.ok() && c.folder.filename() == "ULJM05800" && c.made_for == "ULJM05800",
          "a pack named for another release is found through its [games]");
    // Not listing this release.
    const fs::path other = root / "games3" / "ULJM05800";
    make_pack(other, std::string(kIni) + "\n[games]\nULJM05800 = true\n");
    c = check_texture_pack(other, kGame);
    check(!c.ok() && c.problem.find("ULJM05800") != std::string::npos, "a ULJM05800 pack without NPJB40001 is refused");
    make_pack(root / "games4" / "ULJM05800");
    c = check_texture_pack(root / "games4" / "ULJM05800", kGame);
    check(!c.ok(), "a ULJM05800 pack without [games] is refused");
    c = check_texture_pack(root / "games4", kGame);
    check(!c.found(), "and is not picked from its parent folder");
    // An override ini named in [games] for this release is read.
    const fs::path overridden = root / "games5" / kGame;
    make_pack(overridden, std::string(kIni) + "\n[games]\nNPJB40001 = hd.ini\n");
    write(overridden / "hd.ini", "[hashes]\n00000000aaaaaaaabbbbbbbb = ui/capcom.png\n");
    c = check_texture_pack(root / "games5", kGame);
    check(c.ok() && c.keys == 5u, "the ini [games] names for this release adds its keys");
}

void test_hashes(const fs::path &root) {
    std::string quick = kIni;
    quick.replace(quick.find("xxh64"), 5, "quick");
    make_pack(root / "quick" / kGame, quick);
    TexturePackCheck c = check_texture_pack(root / "quick", kGame);
    check(c.found() && !c.ok() && c.problem.find("\"quick\" hash") != std::string::npos,
          "hash = quick is refused with its reason");
    std::string xxh32 = kIni;
    xxh32.replace(xxh32.find("xxh64"), 5, "xxh32");
    make_pack(root / "xxh32" / kGame, xxh32);
    c = check_texture_pack(root / "xxh32", kGame);
    check(c.ok() && c.hash == TexturePackHash::Xxh32, "hash = xxh32 is accepted");
    make_pack(root / "nohash" / kGame, "[hashes]\n0000000022585cbda625131a = ui/capcom.png\n");
    c = check_texture_pack(root / "nohash", kGame);
    check(!c.ok() && !c.problem.empty(), "an ini that names no hash is refused");
}

bool wait(TexturePackCopy &copy) {
    for (int i = 0; i < 2000 && copy.state() == TexturePackCopy::State::Copying; ++i)
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    copy.join();
    return copy.state() != TexturePackCopy::State::Copying;
}

void test_copy(const fs::path &root) {
    const fs::path textures = root / "data" / "textures";
    const fs::path installed = textures / kGame;
    make_pack(installed);
    write(installed / "old.txt", "the old pack");
    write(textures / ".incomplete-2000-01-01_00-00-00" / "junk", "left by a crash");

    const fs::path source = root / "new" / "ULJM05800";
    make_pack(source, std::string(kIni) + "\n[games]\nNPJB40001 = true\n");
    write(source / "maps" / "village.png", "village");
    const TexturePackCheck c = check_texture_pack(source, kGame);
    check(c.ok(), "the new pack passes its checks");

    TexturePackCopy copy;
    copy.start(c, textures);
    check(!fs::exists(textures / ".incomplete-2000-01-01_00-00-00"), "an unfinished copy is cleared first");
    check(wait(copy) && copy.state() == TexturePackCopy::State::Done, "the copy finishes");
    const TexturePackCopy::Progress progress = copy.progress();
    check(progress.files == c.files && progress.bytes == c.bytes, "the progress reaches every file and byte");
    check(fs::exists(copy.staging() / "maps" / "village.png") && !fs::exists(copy.staging() / ".DS_Store"),
          "the staging folder holds the pack without hidden files");
    check(read(installed / "old.txt") == "the old pack", "the installed pack is untouched until the swap");

    const fs::path backup_dir =
        texture_pack_backup_directory(textures, std::chrono::system_clock::from_time_t(1'000'000'000));
    check(backup_dir.parent_path() == textures / ".backup", "backups go to textures/.backup");
    fs::path backup;
    std::string error;
    check(install_staged_texture_pack(copy.staging(), textures, kGame, backup_dir, backup, error),
          "the copy is put in place: " + error);
    check(backup == backup_dir / kGame && read(backup / "old.txt") == "the old pack",
          "the replaced pack moved to .backup/<time>/NPJB40001, whole");
    check(read(installed / "maps" / "village.png") == "village" && !fs::exists(installed / "old.txt"),
          "the new pack is installed as NPJB40001");
    check(!fs::exists(copy.staging()), "no staging folder is left");
    check(texture_pack_backup_directory(textures, std::chrono::system_clock::from_time_t(1'000'000'000)) !=
              backup_dir,
          "a second backup in the same second gets a new folder");

    // Cancelled: nothing changes and nothing is left behind.
    const fs::path big = root / "big" / kGame;
    make_pack(big);
    for (int i = 0; i < 3000; ++i) write(big / "many" / (std::to_string(i) + ".png"), std::string(64, 'x'));
    const TexturePackCheck big_check = check_texture_pack(big, kGame);
    TexturePackCopy cancelled;
    cancelled.start(big_check, textures);
    cancelled.cancel();
    check(wait(cancelled) && cancelled.state() == TexturePackCopy::State::Cancelled, "a copy can be cancelled");
    check(!fs::exists(cancelled.staging()), "a cancelled copy leaves no staging folder");
    check(read(installed / "maps" / "village.png") == "village", "a cancelled copy leaves the installed pack");

    InstalledTexturePack summary = summarize_texture_pack(installed, kGame);
    check(summary.exists && summary.keys == 4u && summary.files == c.files, "the installed pack is summarized");
    summary = summarize_texture_pack(textures / "nothing", kGame);
    check(!summary.exists, "a missing folder has no pack");
}

void test_location(const fs::path &root) {
    const fs::path textures = root / "textures";
#if defined(_WIN32)
    _putenv_s("MHP3RD_TEXTURE_PACK", "");
#else
    unsetenv("MHP3RD_TEXTURE_PACK");
#endif
    TexturePackLocation l = texture_pack_location(textures, kGame, "");
    check(l.source == TexturePackLocation::Source::Installed && l.folder == textures / kGame,
          "the pack is read from textures/NPJB40001 by default");
    l = texture_pack_location(textures, kGame, "/somewhere/pack");
    check(l.source == TexturePackLocation::Source::InPlace && l.folder == fs::path("/somewhere/pack"),
          "a pack used in place is read from its folder");
#if !defined(_WIN32)
    setenv("MHP3RD_TEXTURE_PACK", "1", 1);
    l = texture_pack_location(textures, kGame, "/somewhere/pack");
    check(l.source == TexturePackLocation::Source::InPlace, "MHP3RD_TEXTURE_PACK=1 names no folder");
    setenv("MHP3RD_TEXTURE_PACK", "/elsewhere", 1);
    l = texture_pack_location(textures, kGame, "/somewhere/pack");
    check(l.source == TexturePackLocation::Source::Variable && l.folder == fs::path("/elsewhere"),
          "a folder in MHP3RD_TEXTURE_PACK wins");
    unsetenv("MHP3RD_TEXTURE_PACK");
#endif
}

} // namespace

int check_folder(const char *folder) {
    const TexturePackCheck c = check_texture_pack(folder, kGame);
    std::printf("chosen   %s\n", c.chosen.string().c_str());
    std::printf("pack     %s\n", c.found() ? c.folder.string().c_str() : "(none)");
    std::printf("layout   %s\n", c.layout.c_str());
    if (!c.made_for.empty()) std::printf("made for %s\n", c.made_for.c_str());
    std::printf("hash     %s%s\n", c.hash == TexturePackHash::Xxh64 ? "xxh64" : "xxh32",
                c.ignore_address ? ", ignoreAddress" : "");
    std::printf("keys     %zu\n", c.keys);
    std::printf("images   %zu\n", c.images);
    std::printf("files    %zu, %llu bytes\n", c.files, static_cast<unsigned long long>(c.bytes));
    std::printf("missing  %zu", c.missing);
    for (const std::string &name : c.missing_names) std::printf(" %s", name.c_str());
    std::printf("\n%s\n", c.ok() ? "importable" : ("refused: " + c.problem).c_str());
    return c.ok() ? 0 : 1;
}

int main(int argc, char **argv) {
    if (argc == 3 && std::string(argv[1]) == "--check") return check_folder(argv[2]);
    const Scratch scratch;
    test_layouts(scratch.root);
    test_games(scratch.root);
    test_hashes(scratch.root);
    test_copy(scratch.root);
    test_location(scratch.root);
    std::printf("%s\n", failures == 0 ? "all texture pack checks passed" : "texture pack checks FAILED");
    return failures == 0 ? 0 : 1;
}
