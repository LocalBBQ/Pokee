<p align="center"><b>English</b> · <a href="README.ru.md">Русский</a> · <a href="README.es.md">Español</a></p>

<p align="center">
  <a href="https://github.com/LocalBBQ/Pokee"><img src="https://img.shields.io/badge/source-LocalBBQ%2FPokee-b98335?style=flat-square&amp;labelColor=2a1a22" alt="Source: LocalBBQ/Pokee"></a>
  <a href="https://github.com/LocalBBQ/Pokee/actions/workflows/tests.yml?query=branch%3Amain"><img src="https://img.shields.io/github/actions/workflow/status/LocalBBQ/Pokee/tests.yml?branch=main&amp;event=push&amp;style=flat-square&amp;label=tests&amp;labelColor=2a1a22" alt="Unit tests on main"></a>
  <a href="LICENSE"><img src="https://img.shields.io/badge/license-MIT-b98335?style=flat-square&amp;labelColor=2a1a22" alt="License: MIT"></a>
  <a href="docs/BUILDING.md"><img src="https://img.shields.io/badge/platforms-Windows%20%C2%B7%20Linux%20%C2%B7%20macOS%20%C2%B7%20Android-b98335?style=flat-square&amp;labelColor=2a1a22" alt="Build targets: Windows, Linux, macOS and Android"></a>
  <a href="https://github.com/LocalBBQ/Pokee/stargazers"><img src="https://img.shields.io/github/stars/LocalBBQ/Pokee?style=flat-square&amp;label=stars&amp;labelColor=2a1a22&amp;color=b98335" alt="GitHub stars"></a>
</p>

<p align="center"><img src="docs/images/emblem.svg" width="120" alt="Pokee emblem"></p>

<h1 align="center">Pokee</h1>

<p align="center"><b>Monster Hunter Freedom Unite, running natively on your PC.</b><br>
Widescreen, high resolution, smooth frame rates, HD textures, a real mouse and gamepad, and hunting with friends.</p>

<p align="center">
  <a href="https://github.com/LocalBBQ/Pokee/releases/latest"><b>Download for Windows</b></a> ·
  <a href="docs/LINUX.md">Linux and Steam Deck</a> ·
  <a href="docs/MACOS.md">macOS</a> ·
  <a href="docs/BUILDING.md">Build from source</a>
</p>

<p align="center"><img src="docs/images/readme/village.jpg" alt="Pokke Village in widescreen at high resolution"></p>

> **Pokee does not include the game.** You need the disc image from your own copy of *Monster Hunter Freedom Unite* (USA, `ULUS-10391`, version 1.01). See the [legal disclaimer](#legal-disclaimer).

## Get it running on Windows

1. **Download** the latest zip from the [releases page](https://github.com/LocalBBQ/Pokee/releases/latest).
   - `pokee-…-windows-x86_64.zip` keeps settings and saves in `%APPDATA%\Pokee\MHFU`.
   - `pokee-…-windows-x86_64-portable.zip` keeps everything in a `data` folder next to `Pokee.exe`, so it can live on a USB drive.
2. **Extract the whole folder** somewhere you can write to, such as `C:\Games\Pokee`. Not `Program Files`.
3. **Add a Windows Defender exclusion for that folder.** Pokee loads its 290 recompiled code libraries (`overlays\*.dll`) every time it starts, and Defender scans each one before letting it load. Without an exclusion, startup can take minutes instead of seconds.
   - **Windows Security** → **Virus & threat protection** → **Manage settings** → **Exclusions: Add or remove exclusions** → **Add an exclusion** → **Folder** → choose the Pokee folder.
   - Or, in PowerShell run as administrator:

     ```powershell
     Add-MpPreference -ExclusionPath "C:\Games\Pokee"
     ```

   Only exclude a folder you extracted from this repository's releases page.
4. **Run `Pokee.exe`** and pick your disc image (`.iso`) when asked. Pokee checks it, then either copies it (about 1.3 GB, so you can move the original later) or uses it where it is. A gamepad alone is enough for the whole setup.
5. **Play.** Later starts go straight to the game. Press **Esc**, or **L3 + R3** on a gamepad, to open Pokee's menu.

You need 64-bit Windows and a graphics driver with Vulkan support. SDL3, FFmpeg and the Visual C++ runtime come in the zip.

Already have a save on a PSP? [Import it](profiles/mhfu/README.md#importing-a-save-from-a-psp) and carry on where you left off.

## Sharper, wider, smoother

<table>
  <tr>
    <td><img src="docs/images/readme/forest.jpg" alt="A mountain view through the trees in widescreen"></td>
    <td><img src="docs/images/readme/coast.jpg" alt="A Hermitaur on the coast in widescreen"></td>
  </tr>
</table>

Render up to six times the PSP's resolution, fill a widescreen display, widen the field of view, and let frame interpolation carry the game's 30 fps up to your monitor's refresh rate. PPSSPP-compatible [HD texture packs](profiles/mhfu/README.md#hd-texture-packs) drop straight in; the screenshots on this page use one.

## Hunt together

<table>
  <tr>
    <td><img src="docs/images/readme/guild-hall.jpg" alt="Four hunters in the Guild Hall"></td>
    <td><img src="docs/images/readme/hunt-together.jpg" alt="Three hunters on a quest in the desert"></td>
  </tr>
</table>

The Guild Hall works the way it did on the PSP: ad hoc play between Pokee players on the same network, or across the internet through an ad hoc server. Host or join from the Network section of the menu. [Setting it up](profiles/mhfu/README.md#multiplayer-ad-hoc)

## Less menu, more hunting

<table>
  <tr>
    <td><img src="docs/images/readme/item-sets.jpg" alt="Item Sets in the item box"></td>
    <td><img src="docs/images/readme/hall-equipment.jpg" alt="Change Equipment from the Guild Hall item box"></td>
  </tr>
</table>

Optional additions that keep to the game's own look:

- **[Item sets](profiles/mhfu/README.md#item-sets)**: save your pouch and load it back in one press.
- **Change equipment in the Guild Hall**, and **[order food from a seat](profiles/mhfu/README.md#eating-in-the-guild-hall)** there.
- **[Damage numbers](profiles/mhfu/README.md#damage-numbers)** where each hit lands, shared with the other Pokee players in your hunt.
- **True Attack Value** for weapons, element and status on the equipment screens.
- **[Gather icons](profiles/mhfu/README.md#gather-icon)** that show what a point gives, and **hold to gather**.
- **[Key quests](profiles/mhfu/README.md#key-quests)** marked in the quest lists, and a **[modern sharpness gauge](profiles/mhfu/README.md#modern-sharpness-gauge)**.
- **[Layered armor](profiles/mhfu/README.md#layered-armor)**: wear the look of one set with the stats of another.

## One menu for everything

<table>
  <tr>
    <td><img src="docs/images/readme/menu-video.jpg" alt="Pokee menu, Video section"></td>
    <td><img src="docs/images/readme/menu-mods.jpg" alt="Pokee menu, Mods section"></td>
  </tr>
</table>

Video, audio, controls, game options, multiplayer and mods all live in one menu over the game, and every change applies at once. It works with a gamepad, keyboard or mouse. [Every setting](profiles/mhfu/README.md#in-game-menu)

**Controls.** On a gamepad, the right stick turns the camera and aims, and the buttons match the game's prompts. Choose a preset (Default, Modern with attacks on the triggers, Left-handed, Classic keyboard) or rebind anything, including button combinations such as L1 + ○. Keyboard and mouse are fully supported: WASD to move, the mouse for the camera, its buttons to attack. [Control tables](profiles/mhfu/README.md#running)

## Status

The game is playable from a new character or an imported PSP save through the village, the Guild Hall and hunts, with music, movies and PSP-format saves.

| Works | Still open |
| --- | --- |
| The whole game: village, Guild Hall, hunts, music and movies | Save-data and message dialogs draw nothing and answer as if confirmed ([upstream #33](https://github.com/TeamGDB/Yakumo/issues/33)) |
| PSP-format saves and downloaded quests; import, export and backups | Download mode (`sceHttp`, `sceNetInet`) |
| Ad hoc play between Pokee instances, on a LAN or through an ad hoc server | Play with PPSSPP and on public servers |
| File-replacement [mods](profiles/mhfu/README.md#mods) and HD texture packs | Code mods |

Windows builds are on the releases page. Linux, Steam Deck, macOS and Android can be built from source; see [LINUX.md](docs/LINUX.md), [MACOS.md](docs/MACOS.md) and [BUILDING.md](docs/BUILDING.md). Found a problem? [Open an issue](https://github.com/LocalBBQ/Pokee/issues) with your platform and what you were doing.

## Building from source

You need your own copy of the game, CMake 3.20+, Ninja, a C++20 compiler, Python 3, SDL3, Vulkan and `glslangValidator`, plus a few gigabytes of free memory. In short:

1. Build `Pokee` once and run `Pokee --install /path/to/image.iso` to prepare the game's executable from your image.
2. Generate the recompiled code with `profiles/mhfu/scripts/generate.sh` and build `Pokee`.
3. Recompile the code overlays with `profiles/mhfu/scripts/build_overlays.sh` (about 40 minutes the first time; resumable).
4. Run `out/mhfu/bin/Pokee`.

[docs/BUILDING.md](docs/BUILDING.md) covers every platform and how to avoid full rebuilds. [CONTRIBUTING.md](CONTRIBUTING.md) explains how to contribute.

## How it works

Pokee does not emulate the PSP's processor while you play. The game's MIPS code is translated to C++ ahead of time and compiled into the program, along with 290 code overlays the game swaps in as it runs. An interpreter covers anything the recompiled code misses. Around that code sits a reimplementation of the PSP system: kernel, disc I/O, a Vulkan renderer for the graphics engine, audio mixing, input, save data and ad hoc networking.

[docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) describes the execution model. Pokee is built on [PSPRecomp](https://github.com/jessicanataliagta/PSPRecomp), a game-neutral static recompilation framework; [docs/PROFILE_GUIDE.md](docs/PROFILE_GUIDE.md) explains how to target another title.

## Authors

[Pokee](https://github.com/LocalBBQ/Pokee) is built from [Yakumo](https://github.com/TeamGDB/Yakumo), the Portable 3rd port. That project and its contributors keep the credit for the host this tree starts from:

- [@MHunterG](https://github.com/MHunterG)
- [@mojitosunrise](https://github.com/mojitosunrise)

## Credits

- [Yakumo](https://github.com/TeamGDB/Yakumo): the original port, by its contributors
- [PSPRecomp](https://github.com/jessicanataliagta/PSPRecomp): the recompilation framework this project builds on
- [SDL3](https://www.libsdl.org/): windowing, input and audio output
- [FFmpeg](https://ffmpeg.org/): music and movie decoding
- [Vulkan](https://www.vulkan.org/) and [MoltenVK](https://github.com/KhronosGroup/MoltenVK): rendering
- [Dear ImGui](https://github.com/ocornut/imgui): Pokee's menus and setup screens
- [tiny-AES-c](https://github.com/kokke/tiny-AES-c): the installer and PSP save-data support
- [stb_truetype](https://github.com/nothings/stb): font rasterization
- [svanheulen/mhef](https://github.com/svanheulen/mhef): community documentation of the game's archive format
- [Cinzel](https://github.com/NDISCOVER/Cinzel) and [Shippori Mincho](https://github.com/fontdasu/ShipporiMincho): the logo's lettering, under the SIL Open Font License

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

## License

The repository is distributed under the MIT License; see [`LICENSE`](LICENSE). Third-party files keep their own notices beside them; [`docs/SOURCE_PROVENANCE.md`](docs/SOURCE_PROVENANCE.md) lists their origins and licences.
