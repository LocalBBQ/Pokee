# Releasing

A release gives players the program ready to run: the executable with the recompiled game code built in and the 355 overlay libraries. Like the repository, it does not include any game assets or original game files: no disc image, no copy of the game's executable or data, and no textures, models, audio or video from the game. On first start it asks for the files from the player's own legally obtained copy, the disc image, checks it, accepts only the original release, and prepares the game from it (see [Installer](../profiles/mhp3rd/README.md#installer)).

Releases are built by maintainers, not by CI: the recompiled code is generated from the game's executable, so a build needs a copy of the game. The game data stays on the maintainer's machine. Every artifact is checked for it before it is published.

So far there are Linux and macOS builds. Windows follows in [#29](https://github.com/TeamGDB/Yakumo/issues/29).

## Linux

`profiles/mhp3rd/scripts/release_linux.sh` builds both Linux artifacts from a checkout:

| Artifact | Contents |
| --- | --- |
| `yakumo-<version>-linux-x86_64.flatpak` | Flatpak bundle, app ID `io.github.teamgdb.Yakumo`, on the `org.freedesktop.Platform` 25.08 runtime. The main download, and the one for the Steam Deck. |
| `yakumo-<version>-linux-x86_64.tar.gz` | Portable tarball: the `yakumo` launcher, `Yakumo`, `overlays/`, `lib/` (SDL3, FFmpeg), `fonts/`, `licenses/` |
| `ffmpeg-<version>.tar.xz` | The unmodified source of the FFmpeg both artifacts contain, published next to them as the LGPL asks |
| `SHA256SUMS` | Checksums of the three files above |
| `BUILDINFO.txt` | Commit, build environment, glibc requirement and the FFmpeg configure line, for the release notes |

Flathub cannot host the Flatpak: its builders compile everything from source, and this build needs the game's executable.

### What the script does

1. Checks the game data a source build uses: `profiles/mhp3rd/game/EBOOT.ELF` (the SHA-256 in the profile README) and `profiles/mhp3rd/game/disc.iso`. `prepare_game.sh` sets both up; `Yakumo --install` produces the executable from the image.
2. Starts the Steam Runtime 3 "sniper" SDK container (Debian 11, glibc 2.31), pinned by digest in `packaging/linux/sources.sh`, and runs `packaging/linux/build_in_sdk.sh` in it, which
   - builds SDL3 from a source archive pinned by version and SHA-256 in `sources.sh`;
   - configures Yakumo with `-DMHP3RD_RELEASE=ON`, `-DMHP3RD_FFMPEG=bundled` and GCC 14. The bundled FFmpeg is built by `cmake/FFmpeg.cmake` with only `libavcodec` and `libavutil` and the ATRAC3, ATRAC3plus and H.264 decoders, and configuring stops unless FFmpeg reports the LGPL with no GPL or non-free parts. The build then generates the recompiled code, builds `Yakumo` and runs the save-data self-tests;
   - builds all overlay libraries with `build_overlays.sh`;
   - stages the program with the libraries it needs, the fallback Japanese font and the license texts, strips it, and checks that every library resolves, that no binary needs `libstdc++.so`, and which glibc version it needs.
3. Packs the tarball from the staged tree, with the launcher, in a reproducible order and with fixed timestamps.
4. Builds the Flatpak with `flatpak-builder` (the `org.flatpak.Builder` Flatpak) from `packaging/linux/io.github.teamgdb.Yakumo.yml`, which packs the same staged tree, and exports it as a single-file bundle.
5. Unpacks both artifacts again and refuses them if any file is named like game data (`EBOOT*`, `*.iso`, `*.cso`, `*.bin`, `*.prx`, `*.pbp`, `*.elf`, `DATA.BIN`, `PARAM.SFO`, `SAVEDATA`, `ms0`, …) or looks like it by content: a MIPS ELF, a PBP, an encrypted PSP module, a `PARAM.SFO` or an ISO 9660 image.
6. Prints the checksums.

### Requirements

- An x86-64 Linux machine, a Steam Deck in Desktop Mode included, with `podman` (or `docker`), `flatpak` with the Flathub remote, `curl` and `ostree`. The script installs `org.flatpak.Builder` and the Freedesktop 25.08 SDK for the user.
- The game data, as above.
- About 25 GB of free space. The first build takes one to two hours; later ones reuse the compiler cache, the built dependencies and the finished overlay libraries.

### Build

```bash
profiles/mhp3rd/scripts/release_linux.sh --version 0.2.0
```

Without `--version`, the artifacts are named after `git describe`. `--jobs N` sets the number of parallel compiles (default 4; each generated unit needs over a gigabyte of memory). `--skip-build` packs the staged build of a previous run again, `--no-flatpak` and `--no-tarball` leave one artifact out. Everything is kept in `out/release-linux/`, the artifacts in `out/release-linux/dist/`. Build from a clean checkout of the commit you release: the version shown in Yakumo's menu comes from `git describe` of the checkout.

On a Steam Deck, keep it plugged in and stop it from sleeping during the build, which runs the CPU flat out for hours: a Deck that suspends or runs flat in the middle leaves the build to be resumed, and a hard power-off can damage the half-written files, so after one delete `out/release-linux/build` and let the compiler cache catch up. Over SSH, start the build as a user service so that it also survives the connection closing, and use three jobs to keep the temperature down:

```bash
systemd-run --user --unit=yakumo-release --collect \
    systemd-inhibit --what=sleep:idle --why="Yakumo release build" \
    "$PWD/profiles/mhp3rd/scripts/release_linux.sh" --jobs 3
journalctl --user -fu yakumo-release     # follow it
```

### Check

On a machine with the game, before publishing.

**Never touch a player's own installation or data.** `~/.var/app/io.github.teamgdb.Yakumo` is shared by every installation of the Flatpak, and `~/.local/share/Yakumo` by every tarball: they hold the player's settings, copied disc image and saves. So on a machine where someone plays:

- never uninstall, reinstall or replace an installed Yakumo, and never use `--delete-data`;
- never delete or overwrite anything in those directories, or in Steam's files;
- run every check with its data in a throwaway directory of your own, and clean up only what you created there.

```bash
check=$(mktemp -d)

# The tarball: set up from the disc image alone into a temporary data
# directory, then boot without a window.
tar -xzf out/release-linux/dist/yakumo-*-linux-x86_64.tar.gz -C "$check"
export MHP3RD_DATA_DIR="$check/data"
"$check"/yakumo-*/yakumo --install /path/to/your.iso
MHP3RD_NO_RENDER=1 MHP3RD_NO_AUDIO=1 timeout 120 "$check"/yakumo-*/yakumo
unset MHP3RD_DATA_DIR

# The Flatpak, without installing it: run the committed tree from the build's
# repository with the same runtime, pointing its data at the temporary
# directory. This leaves any installed Yakumo and its data alone.
ostree --repo=out/release-linux/flatpak/repo checkout --user-mode \
    app/io.github.teamgdb.Yakumo/x86_64/stable "$check/app"
flatpak run --command=bash --filesystem="$check" --filesystem=/path/to/iso-folder:ro \
    --env=MHP3RD_DATA_DIR="$check/flatpak-data" --env=MHP3RD_NO_RENDER=1 --env=MHP3RD_NO_AUDIO=1 \
    org.freedesktop.Platform//25.08 -c \
    "$check/app/files/lib/yakumo/yakumo --install /path/to/your.iso &&
     timeout 120 $check/app/files/lib/yakumo/yakumo"

rm -rf "$check"
```

This checks the Flatpak's files on its runtime, not its sandbox permissions. Check those on a machine where Yakumo is not installed, with `flatpak install --user` of the bundle and `flatpak run` using `--env=MHP3RD_DATA_DIR=` pointing at a temporary directory. Uninstall it afterwards without `--delete-data`, and delete only that temporary directory.

The boot should print the executable's SHA-256, `Functions:` with a non-zero count, and overlays being installed as the game loads them; the run then ends at the timeout. Then play the release once with a window, a gamepad, a save and a reload, following [`TESTING.md`](TESTING.md), on a Steam Deck in Game Mode as well.

### Publish

Create the release on GitHub with the tag of the commit that was built, and attach every file from `out/release-linux/dist/` except `BUILDINFO.txt`, whose contents go into the release notes. The FFmpeg source archive must stay attached to every release whose artifacts contain that FFmpeg: that is the source offer in [`THIRD_PARTY_NOTICES.md`](../profiles/mhp3rd/packaging/THIRD_PARTY_NOTICES.md).

### Updating a bundled component

Change its version and SHA-256 where it is pinned, `packaging/linux/sources.sh` for SDL3 and the font or `cmake/FFmpeg.cmake` for FFmpeg, and the matching entry in `THIRD_PARTY_NOTICES.md`; the script refuses to pack when they disagree. A new FFmpeg configure option goes into both as well. To move to a newer SDK, update its digest in `sources.sh`; to move to a newer Flatpak runtime, update `FLATPAK_RUNTIME_VERSION` and `runtime-version` in the manifest together.

## macOS

`profiles/mhp3rd/scripts/release_macos.sh` packages a finished build directory into the macOS artifacts, for Apple Silicon only:

| Artifact | Contents |
| --- | --- |
| `yakumo-<version>-macos-arm64.dmg` | Disk image with `Yakumo.app`, a link to Applications and `Read Me.txt` (the first start, Gatekeeper, where the data lives). The main download. |
| `yakumo-<version>-macos-arm64.zip` | Only with `--zip`: the same app and note as a zip archive |
| `ffmpeg-<version>.tar.xz` | The unmodified source of the FFmpeg the app contains, as for Linux |
| `SHA256SUMS` | Checksums of the files above |
| `BUILDINFO.txt` | Commit, build environment, minimum macOS and the bundled versions, for the release notes |

`Yakumo.app` holds:

```text
Contents/
    Info.plist                 io.github.teamgdb.yakumo, version from git describe, LSMinimumSystemVersion
    MacOS/Yakumo               the executable
    Frameworks/                libSDL3.0.dylib, libvulkan.1.dylib, libMoltenVK.dylib,
                               libavcodec.61.dylib, libavutil.59.dylib
    Frameworks/overlays/       the 355 overlay libraries
    Resources/Yakumo.icns      the icon, from docs/images/emblem.svg (packaging/macos/make_icon.sh)
    Resources/fonts/           Noto Sans CJK JP, the fallback Japanese font
    Resources/vulkan/icd.d/    MoltenVK's driver manifest, found by the bundled Vulkan loader
    Resources/licenses/        LICENSE, THIRD_PARTY_NOTICES.md and every bundled license
```

Inside an app bundle the host looks for the overlays in `Contents/Frameworks/overlays` and for fonts in `Contents/Resources/fonts` (`host/app_paths.cpp`); everywhere else they stay next to the executable.

The app is signed **ad hoc**: no Apple Developer ID, no notarization. Gatekeeper therefore rejects the downloaded app (`spctl --assess` says `rejected`) until the player allows it once in System Settings → Privacy & Security → *Open Anyway*, or removes the quarantine attribute; `packaging/macos/README.txt` and [`MACOS.md`](MACOS.md) walk players through it. It is signed without the hardened runtime, which would refuse to load the separately signed overlay libraries.

### What the script does

1. Checks the build directory: an arm64 `Yakumo` built with `-DMHP3RD_RELEASE=ON` (a developer build, which names its checkout's game directory, is refused), 355 overlay libraries in `bin/overlays`, and the bundled LGPL FFmpeg in `bin/lib` configured as `cmake/FFmpeg.cmake` pins it. Nothing in the build directory is rebuilt or changed.
2. Fetches the pinned sources in `packaging/macos/sources.sh` (which takes SDL3 and the font from `packaging/linux/sources.sh`) and builds SDL3 and the Vulkan loader for arm64 and the deployment target, macOS 13. MoltenVK is the Khronos release build, thinned to arm64.
3. Assembles `Yakumo.app`, points every library reference at `@rpath` with the single rpath `@executable_path/../Frameworks`, and strips local symbols (the symbols the executable exports to the overlays stay).
4. Checks that every symbol the binaries import from the C and C++ runtime exists in the macOS 13 SDK, then marks the binaries that were built for a newer macOS (the build machine's) for macOS 13 with `vtool`. This is what lets a build made on the newest macOS run on older ones without recompiling the game code.
5. Signs every library, then the app, ad hoc, and checks the signature (`codesign --verify --deep --strict`), that no library reference or rpath leads outside the bundle, that no binary needs a newer macOS, and that no file in the app names the home directory, the user name, the checkout or the build directory.
6. Packs the disk image (APFS, LZMA-compressed) and, with `--zip`, the zip, opens them again, checks their signatures and refuses them if any file looks like game data, as the Linux script does.
7. Prints the checksums.

### Requirements

- A Mac with Apple Silicon, Xcode or the Command Line Tools, CMake, Ninja and curl.
- The macOS 13 SDK for the import check: the Command Line Tools keep older SDKs in `/Library/Developer/CommandLineTools/SDKs` (for example `MacOSX13.1.sdk`); `YAKUMO_CHECK_SDK=/path/to/MacOSX13.x.sdk` points at another one.
- A finished release build. From a checkout with the game data, as in the [build instructions](../profiles/mhp3rd/README.md):

  ```bash
  cmake -S . -B out/mhp3rd -G Ninja -DCMAKE_BUILD_TYPE=Release -DPSPRECOMP_PROFILE=mhp3rd \
      -DMHP3RD_RELEASE=ON -DMHP3RD_FFMPEG=bundled
  profiles/mhp3rd/scripts/generate.sh out/mhp3rd
  cmake --build out/mhp3rd -j 4 --target Yakumo
  profiles/mhp3rd/scripts/build_overlays.sh out/mhp3rd 4
  ```

  `MHP3RD_RELEASE` changes only the executable, not the recompiled code or the overlays. A developer build directory can therefore be switched to it, `Yakumo` rebuilt and copied away, and the directory switched back, which relinks only the executable:

  ```bash
  cmake -S . -B out/mhp3rd -DMHP3RD_RELEASE=ON && cmake --build out/mhp3rd -j 2 --target Yakumo
  cp out/mhp3rd/bin/Yakumo out/release-macos/Yakumo
  cmake -S . -B out/mhp3rd -DMHP3RD_RELEASE=OFF && cmake --build out/mhp3rd -j 2 --target Yakumo
  ```

  Build only the `Yakumo` target there: the default target would relink every overlay library against the new executable.

### Build

```bash
profiles/mhp3rd/scripts/release_macos.sh out/mhp3rd                                   # a release build directory
profiles/mhp3rd/scripts/release_macos.sh --executable out/release-macos/Yakumo out/mhp3rd   # a switched-back one
```

Without `--version`, the artifacts are named after `git describe`; commit first, since the version shown in Yakumo's menu comes from `git describe` when `Yakumo` was built. `--zip` adds the zip archive, `--no-dmg` leaves the disk image out, `--jobs N` sets the parallel compiles for SDL3 and the loader. Everything is kept in `out/release-macos/`, the artifacts in `out/release-macos/dist/`. The first run takes a few minutes for SDL3 and the loader; later runs reuse them.

### Check

The same rule as on Linux: **never touch a player's own data.** `~/Library/Application Support/Yakumo/` holds the settings, the copied disc image and the saves of every Yakumo on the Mac, the developer build included. Check the release with its data in a throwaway directory, from a copy of the app outside the checkout, with no `MHP3RD_*` or `DYLD_*` variable pointing at the build tree, and with no other Yakumo running:

```bash
check=$(mktemp -d)
hdiutil attach -readonly -nobrowse -mountpoint "$check/dmg" out/release-macos/dist/yakumo-*-macos-arm64.dmg
ditto "$check/dmg/Yakumo.app" "$check/Yakumo.app"
hdiutil detach "$check/dmg"

# Everything the process loads comes from the bundle or the system.
export MHP3RD_DATA_DIR="$check/data"
"$check/Yakumo.app/Contents/MacOS/Yakumo" --install /path/to/your.iso --in-place
DYLD_PRINT_LIBRARIES=1 VK_LOADER_DEBUG=driver "$check/Yakumo.app/Contents/MacOS/Yakumo" 2>&1 | tee "$check/run.log"
grep -E '/opt/homebrew|/usr/local|out/mhp3rd' "$check/run.log"      # should print nothing
unset MHP3RD_DATA_DIR

# What Gatekeeper does with a download: expect "rejected" for an ad hoc signature.
xattr -w com.apple.quarantine "0081;$(printf %x "$(date +%s)");Safari;" "$check/Yakumo.app"
spctl --assess --type execute -vv "$check/Yakumo.app"

rm -rf "$check"
```

`--install` without `--in-place` copies the 1.3 GB image into the throwaway directory instead. The window run should start the game, print `Overlay corpora: 355 from …/Yakumo.app/Contents/Frameworks/overlays`, and play the intro movie with sound. Then play the release once with a gamepad, a save and a reload, following [`TESTING.md`](TESTING.md), and open it once through Finder from a quarantined copy to see the Gatekeeper steps players will see.

### Publish

As for Linux: attach every file from `out/release-macos/dist/` except `BUILDINFO.txt` to the release of the tag that was built, and keep the FFmpeg source archive next to them. When Linux and macOS artifacts share a release, merge the two `SHA256SUMS` files.

### Updating a bundled component

SDL3 and the font are pinned in `packaging/linux/sources.sh` for both systems; the Vulkan loader, MoltenVK and the deployment target in `packaging/macos/sources.sh`. Update the matching entry in `THIRD_PARTY_NOTICES.md` as well; the script refuses to pack when they disagree. Raising the deployment target needs that release's SDK for the import check.
