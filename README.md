<p align="center"><img src="docs/images/logo.svg" alt="Yakumo" width="720"></p>

<p align="center"><b>English</b> · <a href="README.ru.md">Русский</a> · <a href="README.es.md">Español</a></p>

# Yakumo

A native port of **Monster Hunter Portable 3rd HD Ver.** made by static recompilation: the game's PSP code is translated ahead of time into C++ and compiled for your machine, then run on a reimplementation of the PSP system software. It is not an emulator — there is no interpreter or JIT at the heart of it — and it is not a decompilation.

> **This project does not include any game assets.** You must provide the files from your own legally obtained copy of Monster Hunter Portable 3rd HD Ver. (`NPJB-40001`) to install or build Yakumo.

## Legal disclaimer

**Yakumo** is an independent, open-source project and is not affiliated with, authorized by, sponsored by, or endorsed by CAPCOM, Sony, or any of their affiliates.

Monster Hunter, Monster Hunter Portable 3rd HD Ver., CAPCOM, PlayStation, PSP, and all related trademarks, game assets, artwork, audio, characters, and other intellectual property belong to their respective owners.

**Yakumo** does not include any game assets or original game files: no disc image, no copy of the game's executable or data, and no textures, models, audio or video from the game. You must provide the files from your own legally obtained copy of Monster Hunter Portable 3rd HD Ver. to install or build **Yakumo**; the installer checks that copy and accepts only the original release.

To use **Yakumo**, users must provide the required files from their own legally obtained copy of Monster Hunter Portable 3rd HD Ver. for PlayStation 3.

Users are solely responsible for obtaining, dumping, extracting, and using their game copy in accordance with the laws applicable in their jurisdiction.

**Yakumo** does not support, provide, link to, or encourage the use of unauthorized or pirated copies of the game.

Any references to the original game or its trademarks are made solely for identification, compatibility, and interoperability purposes.

Screenshots and other depictions of the original game may be used solely to document or demonstrate **Yakumo's** functionality. All depicted third-party game content remains the property of its respective rights holders.

The license covering **Yakumo** applies only to the project's own original code and materials and does not grant any rights to third-party intellectual property.

**Yakumo** provides the software, not the game. You must provide your own legally obtained copy.

## Status: playable

You can load a save copied from a PSP or start a new game, hunt, and save your progress, with music, the opening movie, lighting, a keyboard or a gamepad, and an in-game settings menu (Esc, or L3+R3). The game runs at the PSP's speed, 30 frames per second.

| Works | Missing or rough |
| --- | --- |
| Booting, menus, character creation, the village and hunting areas | The quest reward screen shows noise behind its panels (#48) |
| Saves in the PSP's own format, including saves and downloaded quests copied from a PSP | Faint seams on tiled 2D screens above ×1 (#55) |
| 3D models, animation, textures, transparency, lighting and fog | Curved surfaces; the save-data dialogs draw nothing yet (#33) |
| Sound effects, streamed music and cutscene movies (with FFmpeg) | |
| Keyboard, and gamepads with a real right-stick camera | |
| An in-game menu with video, audio and control settings | |
| All 355 code overlays recompiled | |
| Multiplayer through the ad hoc servers PSP players use: gathering hall and quests together | |

Tested on macOS (Apple Silicon, Vulkan through MoltenVK) and on a Steam Deck in Game Mode with native Vulkan and the built-in controls. Windows builds with MSVC and the game starts, but it closes at the first save (#13).

The state of each part of the game on each platform is in [`docs/COMPATIBILITY.md`](docs/COMPATIBILITY.md).

## Requirements

- Your own copy of the game (see above)
- CMake 3.20 or newer, Ninja and a C++20 compiler
- Python 3
- SDL3, Vulkan and `glslangValidator`
- `make` and a C compiler on macOS and Linux: the build makes its own FFmpeg for the music and the movies. Nothing to install for it on Windows
- A few gigabytes of free memory for the build; the recompiled code is large

## Getting started

In short:

1. Prepare the game's executable from your disc image: build `MHP3rdNative` once without recompiled code and run it with `--install /path/to/image.iso`. No external decryption tool is needed. Then link the image and that executable into the profile with `profiles/mhp3rd/scripts/prepare_game.sh`.
2. Configure, generate the recompiled code with `profiles/mhp3rd/scripts/generate.sh`, and build `MHP3rdNative`.
3. Recompile the code overlays with `profiles/mhp3rd/scripts/build_overlays.sh` (about 40 minutes the first time; resumable).
4. Run `out/mhp3rd/bin/MHP3rdNative`.

The full instructions, including every setting, are in [`profiles/mhp3rd/README.md`](profiles/mhp3rd/README.md). Building on every platform, Windows included, how long each stage takes, and how to work on the code without full rebuilds: [`docs/BUILDING.md`](docs/BUILDING.md).

## Controls

On a gamepad the buttons are where you expect them: on a PlayStation pad circle confirms and cross backs out, as the game's prompts say, and the right stick drives the camera. On a keyboard the arrow keys are the D-pad, I/J/K/L the analog stick, X and Z are ○ and ✕, A and S are □ and △, Q and W are L and R, and Enter is START. Both tables are in the [profile README](profiles/mhp3rd/README.md#running).

Esc, or both sticks pressed together (L3+R3), opens Yakumo's own menu: it pauses the game and holds the settings for video, sound and controls, which are kept between runs. The first start sets the game up from your disc image in the same window, and works with a gamepad alone.

## Roadmap

- Prebuilt releases, starting with Linux and the Steam Deck (#29)
- Windows: fix the crash at the first save (#13)
- 60 fps through frame interpolation, with the game still simulating at 30 (#39)

## How it works

The executable is analyzed and every instruction of its code is emitted as C++, which is compiled into the program. The game also loads 355 code overlays at run time into a handful of shared memory slots; each is recompiled into its own library, and when the game loads one, the matching library is installed between frames. An interpreter covers any code the recompiled set does not reach, so nothing stops the game — it only runs slower there.

Around that code sits a reimplementation of the PSP system: a kernel with threads, semaphores, event flags and timers; disc I/O read straight from the image; a Vulkan renderer for the PSP's graphics engine; software voice mixing for audio; and input from SDL3.

[`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md) describes the execution model and [`docs/DATA_BIN.md`](docs/DATA_BIN.md) the game's archive format. [`docs/TESTING.md`](docs/TESTING.md) has the smoke test and how to report results.

## Repository layout

```text
include/psprecomp/   Framework interfaces: runtime, memory, Allegrex state
src/                 Framework: ELF loading, decoder, runtime, interpreter
tools/               Framework: analyzer and C++ code generator
tests/               Framework regression tests
configs/             PSP NID data and generic examples
profiles/mhp3rd/     Everything specific to this game: host, kernel, renderer,
                     audio, input, configuration and build scripts
docs/                Architecture, archive format, profile guide, source rules
```

The recompiled code itself is generated locally from your copy of the game and is never committed.

## Built on PSPRecomp

**Yakumo** is built on [PSPRecomp](https://github.com/jessicanataliagta/PSPRecomp), a static recompilation framework for PSP software. The framework is game-neutral and can be built on its own:

```bash
cmake -S . -B out/framework -DPSPRECOMP_PROFILE=""
cmake --build out/framework --config Release
ctest --test-dir out/framework -C Release --output-on-failure
```

To target another title, see [`docs/PROFILE_GUIDE.md`](docs/PROFILE_GUIDE.md). [`docs/SOURCE_PROVENANCE.md`](docs/SOURCE_PROVENANCE.md) sets out the rules for independently written code and third-party source.

## Authors

- [@MHunterG](https://github.com/MHunterG)
- [@mojitosunrise](https://github.com/mojitosunrise)

## Credits

- [PSPRecomp](https://github.com/jessicanataliagta/PSPRecomp) — the recompilation framework this project builds on
- [SDL3](https://www.libsdl.org/) — windowing, input and audio output
- [FFmpeg](https://ffmpeg.org/) — music and movie decoding
- [Vulkan](https://www.vulkan.org/) and [MoltenVK](https://github.com/KhronosGroup/MoltenVK) — rendering
- [stb_truetype](https://github.com/nothings/stb) — font rasterization
- [svanheulen/mhef](https://github.com/svanheulen/mhef) — community documentation of the game's archive format
- [Cinzel](https://github.com/NDISCOVER/Cinzel) and [Shippori Mincho](https://github.com/fontdasu/ShipporiMincho) — the logo's lettering, under the SIL Open Font License

## License

The repository is distributed under the MIT License; see [`LICENSE`](LICENSE). Third-party files keep their own notices beside them — currently `profiles/mhp3rd/third_party/stb_truetype.h`, under MIT or public domain.
