# Building on Windows

This page builds Yakumo from source on Windows, starting from nothing but your own disc image of *Monster Hunter Portable 3rd HD Ver.* (`NPJB-40001`).

> **Status: not verified yet.** Windows is a target platform (#13), but nobody has run these steps end to end. The build files already handle MSVC (`/bigobj`, the 64 MiB main-thread stack, symbol export to the overlay DLLs), but expect rough edges and please report them with a **Test report** issue.

## Build from the disc image

You do not need an external decryption tool or a decrypted executable from anywhere else. The build needs the decrypted executable (`EBOOT.ELF`) to generate the recompiled code, and Yakumo's installer prepares it from your disc image. The order is:

1. Build a small **bootstrap** `MHP3rdNative` that contains no recompiled code yet. This takes a few minutes.
2. Run it with `--install` and your disc image. It checks the image, prepares `EBOOT.ELF` from it and writes it into the per-user data directory.
3. Point the checkout at the image and that `EBOOT.ELF` with `prepare_game.sh`.
4. Generate the recompiled code and build the real executable.
5. Recompile the code overlays.

The installer accepts only `NPJB-40001`: the disc id in `PARAM.SFO` must match, and so must the SHA-256 of the encrypted `EBOOT.BIN`. Fan translation patches that change only `USRDIR/DATA.BIN` keep that executable and are accepted. The English patch v6.1.0 has been checked: its `EBOOT.BIN` and all 355 code overlays are identical to the original's.

## What you need

| Tool | Notes |
| --- | --- |
| MSVC: Visual Studio 2022 or newer, or just the free *Build Tools for Visual Studio* | The *Desktop development with C++* workload (MSVC and the Windows SDK). The IDE itself is not needed. `clang-cl` works on top of the same workload. MinGW has not been tried; the build files are written for MSVC |
| CMake 3.20 or newer and Ninja | Visual Studio's own copies work, and so do standalone ones on `PATH` |
| Python 3 | On `PATH` as `python3` or `python` |
| Git for Windows | Git Bash runs the `.sh` scripts in `profiles/mhp3rd/scripts/` |
| Vulkan SDK (LunarG) | The Vulkan loader and headers, plus `glslangValidator` for the shaders |
| SDL3 | For example the official `SDL3-devel-*-VC.zip`; pass its directory in `CMAKE_PREFIX_PATH` |
| FFmpeg, optional | `libavcodec` and `libavutil` as shared libraries, found through `pkg-config`. Without FFmpeg the music is silent and the movies are skipped. Configure with `-DMHP3RD_FFMPEG=OFF` to leave it out on purpose |

Plan for about 10 GB of free disk space for the build and several GB of free memory. The generated code is compiled in very large units, so keep the parallelism low (`-j 2`).

## Steps

Open the **x64 Native Tools Command Prompt** of your Visual Studio or Build Tools, so that the MSVC compiler is on `PATH`. From it, start Git Bash so the scripts run with the same environment:

```bat
"C:\Program Files\Git\bin\bash.exe"
```

The commands below run in that Bash. Replace the paths with yours.

```bash
git clone https://github.com/TeamGDB/Yakumo.git
cd Yakumo

# 1. Configure and build the bootstrap executable (no recompiled code yet)
cmake -S . -B out/mhp3rd -G Ninja -DCMAKE_BUILD_TYPE=Release -DPSPRECOMP_PROFILE=mhp3rd \
      -DCMAKE_PREFIX_PATH="C:/libs/SDL3"
cmake --build out/mhp3rd --target MHP3rdNative -j 2

# 2. Prepare EBOOT.ELF from your disc image. --in-place uses the image where it is
#    instead of copying it; drop it to let the installer keep its own copy.
out/mhp3rd/bin/MHP3rdNative.exe --install "D:/Games/MHP3rd.iso" --in-place

# 3. Point the checkout at the image and the prepared executable
profiles/mhp3rd/scripts/prepare_game.sh "D:/Games/MHP3rd.iso" "$APPDATA/Yakumo/MHP3rd/EBOOT.ELF"

# 4. Generate the recompiled code and build the real executable
profiles/mhp3rd/scripts/generate.sh
cmake -S . -B out/mhp3rd                  # pick up the generated units
cmake --build out/mhp3rd --target MHP3rdNative -j 2

# 5. Recompile the code overlays (the longest step; resumable)
profiles/mhp3rd/scripts/build_overlays.sh out/mhp3rd 2

# 6. Play
out/mhp3rd/bin/MHP3rdNative.exe
```

Before step 6, copy `SDL3.dll`, and the FFmpeg DLLs if you built with FFmpeg, next to `MHP3rdNative.exe`, or put their directories on `PATH`.

## Windows specifics

- **The per-user data directory** is `%APPDATA%\Yakumo\MHP3rd\`. Step 2 writes `EBOOT.ELF` and `settings.ini` there. It takes precedence over `profiles/mhp3rd/game`, and both hold the same data after step 3.
- **Symbolic links.** `prepare_game.sh` links the disc image into `profiles/mhp3rd/game`. Git Bash copies the file instead unless Windows Developer Mode is on and `MSYS=winsymlinks:nativestrict` is exported. A copy works too; it costs about 1.3 GB.
- **Saves** live in `profiles/mhp3rd/game/ms0/PSP/SAVEDATA/`. To bring a save from a PSP, see *Importing a save from a PSP* in the [profile README](../profiles/mhp3rd/README.md#importing-a-save-from-a-psp).
- **No build lock.** Unlike macOS and Linux, the build does not lock its directory on Windows yet. Never run two builds of the same directory at once (see [BUILD_SYSTEM.md](BUILD_SYSTEM.md)).
- **Out of memory while compiling** a generated unit means the parallelism is too high. Rerun the same `cmake --build` with `-j 1`; it continues where it stopped.

Every setting and environment variable is described in the [profile README](../profiles/mhp3rd/README.md).
