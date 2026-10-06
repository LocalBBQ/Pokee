// Installer identities and executable preparation. The disc image is optional:
// pass its path to also prepare that image's executable and check the hash.
#include "corpus_identity.hpp"
#include "install/executable_preparation.hpp"
#include "install/game_identity.hpp"
#include "kernel/iso_image.hpp"

#include "psprecomp/common.hpp"
#include "psprecomp/sha256.hpp"

#include <cstdint>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

namespace {
int failures = 0;

void check(bool condition, const char *message) {
    if (!condition) {
        ++failures;
        std::cerr << "FAIL: " << message << '\n';
    }
}

void identities() {
    using namespace mhfu::install;
    check(find_release_by_disc_id("NPJB40001") == nullptr, "Portable 3rd HD is not accepted");
    check(find_release_by_disc_id("ULJM05800") == nullptr, "the original PSP Portable 3rd disc is not accepted");
    check(find_release_by_disc_id("ULUS10391") == &kSupportedReleases[0], "USA Freedom Unite is the accepted disc");
    check(find_release_by_encrypted_hash(kPortable3rdHd.encrypted_sha256) == nullptr,
        "Portable 3rd's encrypted executable is refused");
    check(find_release_by_encrypted_hash(kFreedomUniteUsa.encrypted_sha256) == &kSupportedReleases[0],
        "Freedom Unite encrypted hash");
    check(find_release_by_executable_hash(kFreedomUniteUsa.executable_sha256) == &kSupportedReleases[0],
        "Freedom Unite prepared hash");
    check(std::string(kDiscId) == kFreedomUniteUsa.disc_id, "the running disc id is Freedom Unite");
    check(kFreedomUniteUsa.guest_ram_bytes == 32u * 1024u * 1024u, "Freedom Unite uses 32 MiB");
    const std::string corpus = mhfu::kCorpusExecutableSha256;
    check(corpus.empty() || corpus == kFreedomUniteUsa.executable_sha256,
        "a linked corpus names the accepted executable, or none");
    const std::string listed = supported_release_list();
    check(listed.find("ULUS-10391") != std::string::npos && listed.find("NPJB-40001") == std::string::npos,
        "the supported list names only USA Freedom Unite");
}

void rejects_unknown_executable() {
    const std::vector<std::uint8_t> junk(32u, 0u);
    bool threw = false;
    try {
        (void)mhfu::install::prepare_executable(junk);
    } catch (const psprecomp::Error &error) {
        threw = true;
        check(std::string(error.what()).find("not a supported executable") != std::string::npos,
            "an unknown executable says it is not supported");
    }
    check(threw, "an unknown executable is refused");
}

void prepare_image(const std::string &path) {
    mhfu::IsoImage image(path);
    const auto entry = image.find(mhfu::install::kExecutablePathOnDisc);
    check(entry && !entry->directory, "the image has EBOOT.BIN");
    if (!entry || entry->directory) return;
    std::vector<std::uint8_t> eboot(entry->size);
    check(image.read(static_cast<std::uint64_t>(entry->lba) * mhfu::IsoImage::kSectorSize, eboot) == eboot.size(),
        "EBOOT.BIN was read");
    const mhfu::install::SupportedRelease *release =
        mhfu::install::find_release_by_encrypted_hash(psprecomp::sha256_bytes(eboot));
    check(release != nullptr, "the image's executable is a supported one");
    if (release == nullptr) return;
    const std::vector<std::uint8_t> executable = mhfu::install::prepare_executable(eboot);
    check(psprecomp::sha256_bytes(executable) == release->executable_sha256, "the prepared executable matches");
    check(executable.size() >= 4u && executable[0] == 0x7fu && executable[1] == 'E' && executable[2] == 'L' &&
            executable[3] == 'F',
        "the prepared executable is an ELF");
    std::cout << "prepared " << release->disc_id_display << "\n";
}
} // namespace

int main(int argc, char **argv) {
    identities();
    rejects_unknown_executable();
    if (argc == 2) prepare_image(argv[1]);
    if (failures != 0) return 1;
    std::cout << "install identity tests passed\n";
    return 0;
}
