<p align="center"><img src="docs/images/logo.svg" alt="Pokee" width="720"></p>

<p align="center"><b>English</b> · <a href="README.ru.md">Русский</a> · <a href="README.es.md">Español</a></p>

<p align="center">
  <a href="https://github.com/LocalBBQ/Pokee"><img src="https://img.shields.io/badge/source-LocalBBQ%2FPokee-b98335?style=flat-square&amp;labelColor=2a1a22" alt="Source: LocalBBQ/Pokee"></a>
  <a href="https://github.com/LocalBBQ/Pokee/actions/workflows/tests.yml?query=branch%3Amain"><img src="https://img.shields.io/github/actions/workflow/status/LocalBBQ/Pokee/tests.yml?branch=main&amp;event=push&amp;style=flat-square&amp;label=tests&amp;labelColor=2a1a22" alt="Unit tests on main"></a>
  <a href="LICENSE"><img src="https://img.shields.io/badge/license-MIT-b98335?style=flat-square&amp;labelColor=2a1a22" alt="License: MIT"></a>
  <a href="docs/BUILDING.md"><img src="https://img.shields.io/badge/platforms-Windows%20%C2%B7%20Linux%20%C2%B7%20macOS%20%C2%B7%20Android-b98335?style=flat-square&amp;labelColor=2a1a22" alt="Build targets: Windows, Linux, macOS and Android"></a>
  <a href="https://github.com/LocalBBQ/Pokee/stargazers"><img src="https://img.shields.io/github/stars/LocalBBQ/Pokee?style=flat-square&amp;label=stars&amp;labelColor=2a1a22&amp;color=b98335" alt="GitHub stars"></a>
</p>

# Pokee

A native port of **Monster Hunter Freedom Unite** (USA, `ULUS-10391`, version 1.01) made by static recompilation: the game's PSP code is translated ahead of time into C++ and compiled for your machine, then run on a reimplementation of the PSP system software. It is not an emulator — there is no interpreter or JIT at the heart of it — and it is not a decompilation.

This repository is [LocalBBQ/Pokee](https://github.com/LocalBBQ/Pokee). It continues [Yakumo](https://github.com/TeamGDB/Yakumo), retargeted from Monster Hunter Portable 3rd to Freedom Unite. Yakumo's published builds run Portable 3rd; they do not run this game.

> **This project does not include any game assets.** You must provide the files from your own legally obtained copy of Monster Hunter Freedom Unite (USA, `ULUS-10391`, version 1.01) to install or build Pokee.

## Legal disclaimer

**Pokee** is an independent, open-source project and is not affiliated with, authorized by, sponsored by, or endorsed by CAPCOM, Sony, or any of their affiliates.

Monster Hunter, Monster Hunter Freedom Unite, CAPCOM, PlayStation, PSP, and all related trademarks, game assets, artwork, audio, characters, and other intellectual property belong to their respective owners.

**Pokee** does not include any game assets or original game files: no disc image, no copy of the game's executable or data, and no textures, models, audio or video from the game. You must provide the files from your own legally obtained copy of Monster Hunter Freedom Unite (USA) to install or build **Pokee**; the installer checks that copy and accepts only `ULUS-10391` version 1.01.

To use **Pokee**, users must provide the required files from their own legally obtained copy of Monster Hunter Freedom Unite (USA) for PSP.

Users are solely responsible for obtaining, dumping, extracting, and using their game copy in accordance with the laws applicable in their jurisdiction.

**Pokee** does not support, provide, link to, or encourage the use of unauthorized or pirated copies of the game.

Any references to the original game or its trademarks are made solely for identification, compatibility, and interoperability purposes.

Screenshots and other depictions of the original game may be used solely to document or demonstrate **Pokee's** functionality. All depicted third-party game content remains the property of its respective rights holders.

The license covering **Pokee** applies only to the project's own original code and materials and does not grant any rights to third-party intellectual property.

**Pokee** provides the software, not the game. You must provide your own legally obtained copy.

## Status

The host is aimed at a playable USA Freedom Unite: a save copied from a PSP or a new game, the village and hunts, music, movies and lighting, then a save in the PSP's own format. The simulation stays at the PSP's 30 frames per second. Optional frame interpolation presents it at 45, 60, 90, 120 or the display's refresh rate. While the game loads in silence it can run ahead of real time (*Fast loading*, on by default).

`generate.sh` translates the resident executable (3095 functions) and stamps its SHA-256. Launch registers that corpus only when the prepared executable has the same hash. The game fits in 32 MiB of guest RAM. All 355 code overlays are recompiled ahead of time; an interpreter covers anything they miss.

| In this tree | Still open |
| --- | --- |
| Boot, menus, character creation, the village and hunting areas | Curved surfaces (Bézier and spline patches) |
| PSP-format saves, including saves and downloaded quests copied from a PSP; import, export and backups from the menu | Save-data and message dialogs answer as if confirmed and draw nothing ([upstream #33](https://github.com/TeamGDB/Yakumo/issues/33)) |
| Vulkan graphics: models, animation, textures, transparency, lighting, fog and frame interpolation | Widescreen projection, HUD placement, the lock-on marker and layered armor. Those were traced on Portable 3rd and stay off until they are traced in this executable |
| Analog camera follow, bow and bowgun aim, and lock-on, read from this executable | |
| PPSSPP-compatible HD texture packs for `ULUS10391`, installed from the menu or copied into the data directory | |
| File-replacement mods, managed from the menu (see the [profile README](profiles/mhfu/README.md#mods)) | Code mods |
| Sound effects, streamed music and cutscene movies | Download mode (`sceHttp`, `sceNetInet`) |
| Rebindable keyboard, mouse and gamepad controls, including an analog right-stick camera and aim | |
| First-run setup, the in-game menu, the file browser and the on-screen keyboard | |
| Ad hoc between Pokee instances, on a LAN or through an ad hoc server | Play with PPSSPP, and play on public servers |

The rows in [`docs/COMPATIBILITY.md`](docs/COMPATIBILITY.md) were checked on Yakumo before this tree was retargeted. They describe Portable 3rd sessions. Freedom Unite still needs its own pass on each platform.

## Play

This repository does not publish installers. Build it from source with your own disc image; the steps are in [Getting started](#getting-started). The first-run setup checks the image, prepares the game from it and keeps settings and saves in a per-user directory (or in a [portable `data` folder](profiles/mhfu/README.md#portable-copy) next to the executable).

When a finished build exists, [`docs/LINUX.md`](docs/LINUX.md) and [`docs/MACOS.md`](docs/MACOS.md) cover installing, the first start and where saves live. Android targets 64-bit Android 10 or newer with Vulkan 1.1; touch controls are drawn over the game, and gamepads work too.

## Requirements

Building from source is a fully supported way to play. It needs:

- Your own copy of the game (see above)
- CMake 3.20 or newer, Ninja and a C++20 compiler
- Python 3
- SDL3, Vulkan and `glslangValidator`
- `make` and a C compiler on macOS and Linux: the build makes its own FFmpeg for the music and the movies. Nothing to install for it on Windows
- A few gigabytes of free memory for the build; the recompiled code is large

## Getting started

In short:

1. Prepare the game's executable from your disc image: build `Pokee` once without recompiled code and run it with `--install /path/to/image.iso`. No external decryption tool is needed. Then link the image and that executable into the profile with `profiles/mhfu/scripts/prepare_game.sh`.
2. Configure, generate the recompiled code with `profiles/mhfu/scripts/generate.sh`, and build `Pokee`.
3. Recompile the code overlays with `profiles/mhfu/scripts/build_overlays.sh` (about 40 minutes the first time; resumable).
4. Run `out/mhfu/bin/Pokee`.

The full instructions, including every setting, are in [`profiles/mhfu/README.md`](profiles/mhfu/README.md). Building on every platform, Windows included, how long each stage takes, and how to work on the code without full rebuilds: [`docs/BUILDING.md`](docs/BUILDING.md).

## Controls

On a gamepad the buttons are where you expect them: on a PlayStation pad circle confirms and cross backs out, as the game's prompts say, and the right stick drives the camera and aiming. Control presets (Default, Modern with attacks on the triggers, Left-handed, Classic keyboard, and your own) cover every control, and any of them can be bound to a single button or a combination such as L1 + ○. Keyboard and mouse play is complete; by default WASD moves, the mouse controls the camera, and its buttons attack. The full tables are in the [profile README](profiles/mhfu/README.md#running).

Esc, or both sticks pressed together (L3+R3), opens Pokee's own menu. It holds video, audio, control, network and save settings, including aspect ratio, frame rate, internal resolution and HD texture pack import. The first start sets the game up from your disc image in the same window, and works with a gamepad alone.

## Roadmap

- Trace the widescreen projection, HUD placement, lock-on marker and layered armor in the Freedom Unite executable. They stay off until that work is done
- Curved surfaces, and save-data dialogs that draw
- Ad hoc play with PPSSPP and on the public servers PSP players use
- Android on real phones: device testing, graphics drivers and performance ([upstream #127](https://github.com/TeamGDB/Yakumo/issues/127), [#17](https://github.com/TeamGDB/Yakumo/issues/17))
- One consistent visual style across the setup screens, menus and overlays ([upstream #33](https://github.com/TeamGDB/Yakumo/issues/33))

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
profiles/mhfu/     Everything specific to this game: host, kernel, renderer,
                     audio, input, configuration and build scripts
docs/                Architecture, archive format, profile guide, source rules
```

The recompiled code itself is generated locally from your copy of the game and is never committed.

## Built on PSPRecomp

**Pokee** is built on [PSPRecomp](https://github.com/jessicanataliagta/PSPRecomp), a static recompilation framework for PSP software. The framework is game-neutral and can be built on its own:

```bash
cmake -S . -B out/framework -DPSPRECOMP_PROFILE=""
cmake --build out/framework --config Release
ctest --test-dir out/framework -C Release --output-on-failure
```

To target another title, see [`docs/PROFILE_GUIDE.md`](docs/PROFILE_GUIDE.md). [`docs/SOURCE_PROVENANCE.md`](docs/SOURCE_PROVENANCE.md) sets out the rules for independently written code and third-party source.

## Authors

[Pokee](https://github.com/LocalBBQ/Pokee) is built from [Yakumo](https://github.com/TeamGDB/Yakumo), the Portable 3rd port. That project and its contributors keep the credit for the host this tree starts from:

- [@MHunterG](https://github.com/MHunterG)
- [@mojitosunrise](https://github.com/mojitosunrise)

## Credits

- [Yakumo](https://github.com/TeamGDB/Yakumo) — the original port, by its contributors
- [PSPRecomp](https://github.com/jessicanataliagta/PSPRecomp) — the recompilation framework this project builds on
- [SDL3](https://www.libsdl.org/) — windowing, input and audio output
- [FFmpeg](https://ffmpeg.org/) — music and movie decoding
- [Vulkan](https://www.vulkan.org/) and [MoltenVK](https://github.com/KhronosGroup/MoltenVK) — rendering
- [Dear ImGui](https://github.com/ocornut/imgui) — Pokee's menus and setup screens
- [tiny-AES-c](https://github.com/kokke/tiny-AES-c) — the installer and PSP save-data support
- [stb_truetype](https://github.com/nothings/stb) — font rasterization
- [svanheulen/mhef](https://github.com/svanheulen/mhef) — community documentation of the game's archive format
- [Cinzel](https://github.com/NDISCOVER/Cinzel) and [Shippori Mincho](https://github.com/fontdasu/ShipporiMincho) — the logo's lettering, under the SIL Open Font License

## License

The repository is distributed under the MIT License; see [`LICENSE`](LICENSE). Third-party files keep their own notices beside them; [`docs/SOURCE_PROVENANCE.md`](docs/SOURCE_PROVENANCE.md) lists their origins and licences.
