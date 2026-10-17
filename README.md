# Pokee

Pokee runs **Monster Hunter Freedom Unite** (USA, `ULUS-10391`, version 1.01) on Windows, Linux, macOS and Android.

> **This project does not include the game.** To install or build Pokee, use the files from your own legally obtained copy of that release.

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

Playable: a new game or a save copied from a PSP, the village and hunts, music, movies, and saves in the PSP's own format. The game runs at the PSP's 30 frames per second; frame interpolation can present it at up to the display's refresh rate. The executable (3095 functions) and all 290 code overlays are recompiled ahead of time; an interpreter covers anything they miss.

| Done | Still open |
| --- | --- |
| Boot, menus, character creation, the village and hunts | Curved surfaces (Bézier and spline patches) |
| PSP-format saves, including saves and downloaded quests copied from a PSP; import, export and backups | Save-data and message dialogs draw nothing and answer as if confirmed ([upstream #33](https://github.com/TeamGDB/Yakumo/issues/33)) |
| Vulkan graphics, frame interpolation, widescreen, HUD scaling and hiding | |
| Analog camera and aim, lock-on and its marker, layered armor | |
| PPSSPP-compatible HD texture packs | |
| File-replacement mods ([profile README](profiles/mhfu/README.md#mods)) | Code mods |
| Sound effects, music and cutscene movies | Download mode (`sceHttp`, `sceNetInet`) |
| Rebindable keyboard, mouse and gamepad controls | |
| Ad hoc between Pokee instances, on a LAN or through an ad hoc server | Play with PPSSPP and on public servers |

The results in [`docs/COMPATIBILITY.md`](docs/COMPATIBILITY.md) are from Portable 3rd, before this tree was retargeted; Freedom Unite still needs its own pass on each platform.

## Play

Windows downloads are on the [releases page](https://github.com/LocalBBQ/Pokee/releases). Building from source stays fully supported; the steps are in [Getting started](#getting-started). The first-run setup checks the image, prepares the game from it and keeps settings and saves in a per-user directory (or in a [portable `data` folder](profiles/mhfu/README.md#portable-copy) next to the executable).

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

- Curved surfaces, and save-data dialogs that draw
- Ad hoc play with PPSSPP and on the public servers PSP players use
- Android on real phones: device testing, graphics drivers and performance ([upstream #127](https://github.com/TeamGDB/Yakumo/issues/127), [#17](https://github.com/TeamGDB/Yakumo/issues/17))
- One consistent visual style across the setup screens, menus and overlays ([upstream #33](https://github.com/TeamGDB/Yakumo/issues/33))

## How it works

The executable is analyzed and every instruction of its code is emitted as C++, which is compiled into the program. The game also loads 290 code overlays at run time into a handful of shared memory slots; each is recompiled into its own library, and when the game loads one, the matching library is installed between frames. An interpreter covers any code the recompiled set does not reach, so nothing stops the game — it only runs slower there.

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
