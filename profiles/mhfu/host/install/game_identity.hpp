#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

// The disc image the installer accepts, pinned by the disc id in PARAM.SFO and
// by the SHA-256 of the encrypted executable and of the executable prepared
// from it. USA Freedom Unite is the only accepted release.
namespace mhfu::install {

struct SupportedRelease {
    const char *disc_id;
    const char *disc_id_display;
    const char *title;
    const char *encrypted_sha256;
    const char *executable_sha256;
    // Tag at offset 0xD0 of the encrypted executable. It selects the header
    // layout prepare_executable handles for this file.
    std::uint32_t header_tag;
    // Guest RAM the prepared executable's load image fits in. Portable 3rd
    // needs the 64 MiB layout; Freedom Unite fits in 32 MiB.
    std::uint32_t guest_ram_bytes;
};

// Monster Hunter Portable 3rd HD Ver. The installer does not accept it. The
// equipment name tables and the equipment box are still this executable's.
inline constexpr SupportedRelease kPortable3rdHd{
    "NPJB40001",
    "NPJB-40001",
    "Monster Hunter Portable 3rd HD Ver.",
    "79e25f3512d56e0f7bf5c48351d7d0d255269675ffc8811ac5599322bb66945e",
    "55c0598436c0753b04331f8e95d406f832d9217806e3a896fed0e88b33637d8c",
    0xD9160BF0u,
    64u * 1024u * 1024u,
};

// Monster Hunter Freedom Unite, USA, disc version 1.01.
inline constexpr SupportedRelease kFreedomUniteUsa{
    "ULUS10391",
    "ULUS-10391",
    "Monster Hunter Freedom Unite",
    "3354adb3ee40821f230c4055b9bf7cc5d4c864e42abb74ce8ac022933f9697a7",
    "3070954e2dc88fd0b0836c0c1dc526531aca4a213b0f18806e324963efb514f4",
    0xC0CB167Cu,
    32u * 1024u * 1024u,
};

inline constexpr SupportedRelease kSupportedReleases[] = {kFreedomUniteUsa};

// Names of the release this build runs. Texture packs and the in-game menu
// use them. The analog camera, the projection, hiding the HUD and layered
// armor read this executable. The equipment name tables and the equipment box
// are still the Portable 3rd addresses.
inline constexpr const char *kDiscId = kFreedomUniteUsa.disc_id;
inline constexpr const char *kDiscIdDisplay = kFreedomUniteUsa.disc_id_display;
inline constexpr const char *kGameTitle = kFreedomUniteUsa.title;
inline constexpr const char *kEncryptedExecutableSha256 = kFreedomUniteUsa.encrypted_sha256;
inline constexpr const char *kExecutableSha256 = kFreedomUniteUsa.executable_sha256;

inline constexpr const char *kExecutablePathOnDisc = "PSP_GAME/SYSDIR/EBOOT.BIN";
inline constexpr const char *kParamSfoPathOnDisc = "PSP_GAME/PARAM.SFO";

[[nodiscard]] inline const SupportedRelease *find_release_by_disc_id(std::string_view disc_id) {
    for (const SupportedRelease &release : kSupportedReleases)
        if (disc_id == release.disc_id) return &release;
    return nullptr;
}

[[nodiscard]] inline const SupportedRelease *find_release_by_encrypted_hash(std::string_view hash) {
    for (const SupportedRelease &release : kSupportedReleases)
        if (hash == release.encrypted_sha256) return &release;
    return nullptr;
}

[[nodiscard]] inline const SupportedRelease *find_release_by_executable_hash(std::string_view hash) {
    for (const SupportedRelease &release : kSupportedReleases)
        if (hash == release.executable_sha256) return &release;
    return nullptr;
}

// "Title (ID)", for messages that say what the installer accepts.
[[nodiscard]] inline std::string supported_release_list() {
    std::string text;
    for (std::size_t i = 0; i < std::size(kSupportedReleases); ++i) {
        if (i != 0u) text += i + 1u == std::size(kSupportedReleases) ? " and " : ", ";
        text += kSupportedReleases[i].title;
        text += " (";
        text += kSupportedReleases[i].disc_id_display;
        text += ")";
    }
    return text;
}

} // namespace mhfu::install
