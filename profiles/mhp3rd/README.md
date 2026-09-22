# MHP3rd HD profile

This profile builds `Yakumo` for **Monster Hunter Portable 3rd HD Ver.** (`NPJB-40001`). The PS3 release ships an ordinary PSP UMD image; this profile recompiles that PSP executable and its code overlays into a native program. No executable, game data or code generated from them is part of the repository — you supply your own copy of the game.

## Status

The game boots, loads its overlays, creates a character or loads a save, walks the village, plays hunts and saves, with sound, music, movies, lighting, a keyboard and a gamepad, at the PSP's speed.

| Area | State |
| --- | --- |
| Code | The whole executable (362 478 instructions, 89 units) and all 355 code overlays are recompiled ahead of time; an interpreter covers anything they miss |
| Kernel | Threads with a deterministic virtual clock, semaphores, event flags, mutexes, callbacks, VTimers, partition memory, VBlank interrupts, file I/O straight from the disc image |
| Imports | 244 of 296 implemented; the rest are logging stubs that return 0 |
| Graphics | Vulkan: textures (palettes, DXT, swizzle), skinning, per-vertex lighting (four directional, point or spot lights and the full material model) and fog, blending, depth and alpha test, sprites, per-framebuffer render targets |
| Audio | `sceSasCore` voice mixing, `sceAudio` output and ATRAC3 music through `sceAtrac3plus` |
| Movies | PSMF playback through `sceMpeg` and `sceJpegCsc`: H.264 video and ATRAC3plus sound |
| Input | Keyboard and SDL3 gamepads, including the HD release's second analog stick |
| Text | `sceLibFont` glyphs rasterized from a host TrueType font |
| Saves | The save-data utility, with saves in the PSP's own format: a save copied from a PSP loads, and one made here can be copied back |
| Multiplayer | Ad hoc play through PSP ad hoc servers: two instances have met in a gathering hall and started a quest together; play with PPSSPP and on public servers is still to be tested. See [Multiplayer](#multiplayer-ad-hoc) |

Not done yet:

- **Curved surfaces** (Bézier and spline patches).
- **Infrastructure networking** (`sceHttp`, `sceNetInet`): the game's download mode. Ad hoc multiplayer works.
- **Dialog screens.** The save-data and message dialogs work but draw nothing; each answers as if the player confirmed it ([#33](https://github.com/TeamGDB/Yakumo/issues/33)).
- **Performance:** on a Steam Deck, the lighting path costs enough CPU to slow the busiest village spots slightly below full speed ([#7](https://github.com/TeamGDB/Yakumo/issues/7)); `MHP3RD_NO_LIGHTING=1` avoids it until that is fixed.

Tested on macOS (Apple Silicon, Vulkan through MoltenVK), on a Steam Deck in Game Mode, built with GCC in a Debian 13 container and running on native Vulkan, and on Windows 11 with MSVC. On Windows, creating a character, saving several times, restarting the game and loading the save all work; see [the compatibility table](../../docs/COMPATIBILITY.md).

## Supported executable

| Item | Value |
| --- | --- |
| Module | `MonsterHunterPortable3rd` 1.1 |
| Encrypted `SYSDIR/EBOOT.BIN` SHA-256 | `79e25f3512d56e0f7bf5c48351d7d0d255269675ffc8811ac5599322bb66945e` |
| Decrypted `EBOOT.ELF` SHA-256 | `55c0598436c0753b04331f8e95d406f832d9217806e3a896fed0e88b33637d8c` |
| Load image | `0x08804000`–`0x0A285200` (needs 64 MiB RAM) |
| Entry / `gp` | `0x0882170C` / `0x08A2A680` |
| Imports | 296 across 35 libraries |

`config/mhp3rd_npjb40001.toml` records the same identity plus the overlay slot layout.

## Requirements

- CMake 3.20 or newer, Ninja and a C++20 compiler
- Optional: `ccache`, which the build uses automatically when it is installed
- Python 3
- SDL3, Vulkan (the loader and headers; MoltenVK on macOS) and `glslangValidator`
- `make` and a C compiler on macOS and Linux, to build FFmpeg (see below)

If SDL3, Vulkan or `glslangValidator` is missing, configuration still succeeds but builds the game **without a window**: CMake prints `mhp3rd: renderer disabled` and the program runs headless. Check for `mhp3rd: Vulkan renderer enabled` in the configure output.

The streamed music (ATRAC3) and the movies (H.264 with ATRAC3plus sound) are decoded by FFmpeg's shared `libavcodec` and `libavutil`. FFmpeg is part of the normal build; `MHP3RD_FFMPEG` chooses where it comes from (`cmake/FFmpeg.cmake` holds the pins):

- `bundled`, the default. The first configure of a build directory downloads FFmpeg 7.1.5, checks its SHA-256 and builds it with only the ATRAC3, ATRAC3plus and H.264 decoders, as LGPL-2.1-or-later shared libraries (configure stops if the result is not LGPL only). This takes a few minutes, once per build directory; a new version or configuration rebuilds it. The libraries and FFmpeg's licence go to `out/mhp3rd/bin/lib/`, which the executable finds through its rpath, so the game needs no FFmpeg on the system. On Windows, where FFmpeg's `configure` does not run with MSVC, it downloads a pinned, checksum-verified prebuilt LGPL shared FFmpeg 7.1.5 and puts its DLLs next to `Yakumo.exe` (see [BUILDING.md](../../docs/BUILDING.md#ffmpeg)). Configuration reports `mhp3rd: bundled FFmpeg 7.1.5 …; music and movies enabled`. To build offline, put the archive in `out/mhp3rd/_deps/downloads/` (`MHP3RD_FFMPEG_DOWNLOAD_DIR`) first.
- `system` uses the FFmpeg that `pkg-config` finds, for example from `brew install ffmpeg` on macOS or `apt install libavcodec-dev libavutil-dev` on Debian and Ubuntu, and stops if there is none.
- `OFF` builds without FFmpeg. This is possible but not recommended: the game then has no music and skips its movies, and configure prints a warning saying so.

Expect a full build to need several gigabytes of memory and some time: the generated code is large. With Ninja, the build compiles at most `PSPRECOMP_GENERATED_JOBS` generated units at once, whatever `-j` you pass; the default is one per 4 GiB of memory, so 2 on an 8 GB machine. Set it when configuring, for example `-DPSPRECOMP_GENERATED_JOBS=1`.

## Quick start

```bash
# 1. Game data: link your disc image and decrypted executable into profiles/mhp3rd/game
#    (or let the program set itself up from the image; see "Game data" below)
profiles/mhp3rd/scripts/prepare_game.sh "/path/to/your.iso" /path/to/EBOOT.ELF

# 2. Recompile the executable
cmake -S . -B out/mhp3rd -G Ninja -DCMAKE_BUILD_TYPE=Release -DPSPRECOMP_PROFILE=mhp3rd
profiles/mhp3rd/scripts/generate.sh
cmake -S . -B out/mhp3rd                          # pick up the generated units
cmake --build out/mhp3rd --target Yakumo -j 2

# 3. Recompile the code overlays (about 40 minutes, resumable)
profiles/mhp3rd/scripts/build_overlays.sh

# 4. Play
out/mhp3rd/bin/Yakumo
```

Each step is described below. [`docs/BUILDING.md`](../../docs/BUILDING.md) covers the platforms, how long each stage takes, and working on the code without full rebuilds.

## Game data

The program needs two things from your own copy of the game: the disc image and the game's executable. It finds them in this order:

1. A directory given on the command line or in `MHP3RD_GAME_DIR`.
2. The **per-user data directory** that the installer fills.
3. `profiles/mhp3rd/game` in the checkout, set up by `prepare_game.sh`. A release build (`-DMHP3RD_RELEASE=ON`, see [Release builds](#release-builds)) has no checkout and skips this.

If neither the per-user directory nor `profiles/mhp3rd/game` holds game data, the program starts its installer instead of the game.

### Installer

The installer needs only your disc image. It runs as a few screens in the game's window, all of them usable with a gamepad alone, a keyboard or a mouse:

1. **Welcome**: what is needed and where the data goes.
2. **Choose the disc image** in Yakumo's own file browser. It starts in your home folder (later in the folder where you last found an image) and lists folders and `.iso` files with their sizes; *Showing .iso only* switches to all files. The row of places above the list holds Home, Downloads, Desktop and Documents, and every removable drive: SD cards and USB drives under `/run/media` and `/media` on Linux (a Steam Deck's SD card among them), volumes under `/Volumes` on macOS, drive letters on Windows. Confirm opens a folder or picks a file; back goes up a folder, and from the top back to the welcome screen. On a gamepad, △ (Y) switches between .iso files and all files. A file dropped onto the window is taken as well, on this screen and on the welcome screen. *System dialog…* opens the system's file dialog instead; it is not offered under gamescope (Steam Deck Game Mode), where that dialog does not appear.
3. **Checks**: the image must be `NPJB-40001` (the disc id in `PARAM.SFO` and the SHA-256 of the encrypted executable). A wrong release or region, a modified image, a PlayStation 3 disc image, a compressed (`.cso`) image or a file that is no disc image at all each get a screen that says so plainly, with *Choose another file*.
4. **Copy or use in place**: copying (the default) puts the image (about 1.3 GB) into the per-user directory, so the game keeps working after the original is moved or deleted; the screen shows the free space and refuses the copy when there is not enough. Using the image where it is saves the space; the program then checks on every start that the image is still there and says so if it is not, offering to run the setup again.
5. **Progress**: a progress bar for the copy and for preparing the game's executable from the image, which is then checked against the hash in the table above. The work runs off the window's thread, so the window stays responsive; *Cancel* (or back) stops it and removes what it wrote.

The game then starts. Later starts go straight to the game. The in-game menu's *Set up game data again…* runs the same setup: the game closes and the program starts again with `--install`.

The per-user directory is SDL's preference path for `Yakumo/MHP3rd`:

| System | Directory |
| --- | --- |
| macOS | `~/Library/Application Support/Yakumo/MHP3rd/` |
| Linux | `~/.local/share/Yakumo/MHP3rd/` (or under `$XDG_DATA_HOME`) |
| Windows | `%APPDATA%\Yakumo\MHP3rd\` |

It holds `EBOOT.ELF`, `disc.iso` when the image was copied, `settings.ini`, which records where the image is and keeps the settings of the [in-game menu](#in-game-menu), and `ms0`, the memory stick with the [saves](#saving-and-loading). `MHP3RD_DATA_DIR` points the program at another directory. The Flatpak keeps this directory inside its own data directory, `~/.var/app/io.github.teamgdb.Yakumo/data/Yakumo/MHP3rd/`.

Saves made before `ms0` moved here stay where they were, in `profiles/mhp3rd/game/ms0`: a developer build keeps using them, and says so at start, until the per-user directory has an `ms0` of its own. Move the folder there to switch.

The same setup runs without any screens from a terminal, for scripts and headless machines:

```bash
out/mhp3rd/bin/Yakumo --install "/path/to/your.iso"             # copy the image
out/mhp3rd/bin/Yakumo --install "/path/to/your.iso" --in-place  # use it where it is
out/mhp3rd/bin/Yakumo --install                                 # run the setup screens again, then play
```

`--install` with an image prepares everything and exits. A build without generated code can already run it, and the `EBOOT.ELF` it writes into the per-user directory is the executable `generate.sh` needs.

When the window cannot be created, for example without a working Vulkan driver, the installer falls back to SDL3 message boxes and the system file dialog (on Linux through the desktop portal or `zenity`). A build without SDL shows neither and prints the `--install` command instead.

### Checkout directory

For development, `profiles/mhp3rd/game` works as before, with an executable decrypted by an external tool:

1. Decrypt `PSP_GAME/SYSDIR/EBOOT.BIN` from your own image with an external tool, or take the `EBOOT.ELF` that `--install` wrote into the per-user directory. `tools/extract_iso.py <iso> <dir> /PSP_GAME/SYSDIR/EBOOT.BIN` extracts the encrypted file; the table above gives the hashes to check both files against.
2. Populate `profiles/mhp3rd/game` (ignored by Git):

   ```bash
   profiles/mhp3rd/scripts/prepare_game.sh "/path/to/your.iso" /path/to/EBOOT.ELF
   ```

The image itself is not unpacked: `game/disc.iso` links to it and the host reads files, and raw `sce_lbn` sectors, from it directly. `game/ms0` backs `ms0:` for save data. If you move the image later, rerun the script — the links point at absolute paths.

When the per-user directory also holds an installation, it takes precedence; start with `profiles/mhp3rd/game` as an argument, or set `MHP3RD_GAME_DIR`, to use the checkout.

## Build

```bash
cmake -S . -B out/mhp3rd -G Ninja -DCMAKE_BUILD_TYPE=Release -DPSPRECOMP_PROFILE=mhp3rd
profiles/mhp3rd/scripts/generate.sh          # writes analysis/ and generated/
cmake -S . -B out/mhp3rd                     # pick up the generated units
cmake --build out/mhp3rd --target Yakumo -j 2
```

`generate.sh` analyzes the executable and writes the recompiled C++ into `generated/`. That corpus is derived from your copy of the game, so it stays local and is never committed.

Rerunning `generate.sh` rewrites only the units whose code changed, so the build after it recompiles only those.

The build protects its incremental state on macOS and Linux:

- **One build at a time.** With Ninja, CMake runs builds through a wrapper, `out/mhp3rd/ninja-locked`, that locks the build directory. A second `cmake --build` of the same directory, from a script or by hand, prints `another build is running` and waits. The lock belongs to the running Ninja, so interrupting `cmake --build` or a script around it does not let a new build start next to the Ninja that is still running. Running `ninja` directly bypasses the lock. `-DPSPRECOMP_BUILD_LOCK=OFF` turns it off. On Windows there is no lock yet: do not start two builds of one directory.
- **Damaged dependency log.** Before each build the wrapper checks `.ninja_deps` and repairs a damaged one with `ninja -t recompact`, which keeps every intact record. Do not delete `.ninja_deps` or `.ninja_log`: either costs a full rebuild.
- **Compiler cache.** If `ccache` is installed, every compile goes through it, so a rebuild of unchanged code takes seconds instead of minutes, also across checkouts at different paths. `-DPSPRECOMP_CCACHE=OFF` turns it off.

CMake prints a warning for Ninja 1.13.2, which cannot recover from a damaged dependency log by itself (upstream issue [#2703](https://github.com/ninja-build/ninja/issues/2703)). Building through `cmake --build` works around it; the fix is due in Ninja 1.14. [`docs/BUILD_SYSTEM.md`](../../docs/BUILD_SYSTEM.md) explains why incremental state gets lost and what the build does about it.

## Release builds

Players can use a prebuilt release instead of building: it contains the program with the recompiled code and all overlay libraries, but no game data, and sets the game up from the player's disc image on first start. `-DMHP3RD_RELEASE=ON` builds the executable for that: it reads nothing from the checkout, and on Linux it loads the libraries it ships from `lib/` next to itself and links the C++ runtime statically. [`docs/RELEASING.md`](../../docs/RELEASING.md) describes how a release is built and published; `scripts/release_linux.sh` builds the Linux artifacts, and `packaging/` holds their manifests, launcher and third-party notices.

## Code overlays

Beyond the main executable, the game loads 355 code overlays (`*.ovl`) from `USRDIR/DATA.BIN` into 12 fixed slots at run time: mode tasks (`game_task`, `lobby_task`, …), maps (`P_m*`, `P_v*`), monsters (`em*m0`–`m3`) and weapons (`we*player00`–`03`). The ELF section table lists them as zero-sized sections, so their code is not part of `EBOOT.ELF`.

Build them all once:

```bash
profiles/mhp3rd/scripts/build_overlays.sh [build_dir] [jobs]
```

It extracts every overlay from `DATA.BIN` into `analysis/overlays`, recompiles each one that has no library yet into `overlays/`, and then builds all their shared libraries into `bin/overlays` in a single `cmake --build` with `jobs` parallel jobs (default 2). Libraries that already exist are skipped and recompiled overlays are not recompiled again, so an interrupted run resumes. The game is playable before this finishes: an overlay with no library runs through the interpreter, which works but is roughly twenty times slower.

### How the host picks an overlay

Many overlays share one address, so each has its own corpus and the host must use the one matching what is loaded right now. An overlay image starts with `MWo3` and a 64-byte header: id, load address, code size, data size, bss size, two end-of-image addresses and a 32-byte name. A corpus is identified by its slot base plus an FNV-1a hash of that header and the code after it — the part the game never writes to, unlike the data section a loaded overlay keeps modifying.

On a dispatch miss inside a slot, the host hashes those bytes in guest memory, unregisters whatever was installed there and registers the matching corpus. Every frame it compares each installed slot's header against the one it installed, which is enough to notice a swap without rehashing the code. An overlay may be larger than the gap to the next slot's base; the slot list records where images load, not how much room they have.

Each library resolves its framework symbols from the executable that loads it, so it only matches a host built from the same sources; `cmake --build out/mhp3rd` rebuilds all of them. `MHP3RD_OVERLAY_DIR` points the host at another library directory.

To build a single overlay by hand — for example one dumped from memory with `MHP3RD_DUMP_OVERLAYS`:

```bash
profiles/mhp3rd/tools/add_overlay.py out/mhp3rd /path/to/overlay_0A05E600.bin 0x0A05E600
```

It builds with 2 parallel jobs; `-j N` changes that. `--no-build` stops after recompiling and prints the target name, for building many overlays in one run. No reconfigure is needed: CMake notices the new overlay directory by itself.

## Running

```bash
out/mhp3rd/bin/Yakumo [game_dir]       # see "Game data" for where it looks without game_dir
```

The window renders at twice the PSP resolution by default (960×544). Esc, or L3+R3 on a gamepad, opens the [in-game menu](#in-game-menu); quit from there, or close the window (Cmd+Q on macOS, Alt+F4 on most Linux desktops). When the game asks for a name, Yakumo's [on-screen keyboard](#on-screen-keyboard) opens; the menu can switch to giving a fixed name at once instead.

### Keyboard

| Key | PSP |
| --- | --- |
| Arrow keys | D-pad |
| I / J / K / L | Analog stick |
| X | ○ (confirm) |
| Z | ✕ (back) |
| A | □ |
| S | △ |
| Q / W | L / R |
| Enter | START |
| Right Shift, Backspace | SELECT |
| Esc | In-game menu |
| F3 | Performance overlay on or off |

The keyboard has no binding for the HD release's second stick; use a gamepad for the right-stick camera. Keys are only read while the window has focus.

### Gamepad

Any controller SDL3 recognises works, and it can be connected before or after the game starts.

| Gamepad | PSP |
| --- | --- |
| South / East / West / North face buttons | ✕ / ○ / □ / △ |
| LB, RB, and LT / RT past their threshold | L, R |
| Start / Back | START / SELECT |
| D-pad | D-pad |
| Left stick | Analog stick |
| Right stick | The HD release's second stick (camera) |
| L3 + R3 (both sticks pressed) | In-game menu |

The face buttons are positional, so on a PlayStation pad circle is circle and confirms, exactly as the game's prompts say. The menu's *Confirm button* setting (or `MHP3RD_PAD_FACE=xbox`) moves confirm to the bottom button for pads labelled the other way round.

## On-screen keyboard

When the game asks for text (the hunter's name at character creation), Yakumo opens its own keyboard over the game. It works with a gamepad alone, and a physical keyboard types into it at the same time; the mouse can click its keys.

| Gamepad | Keyboard | Action |
| --- | --- | --- |
| D-pad, left stick | | Move over the keys |
| Confirm (○, or the bottom button with *Confirm button* set to it) | typing | Type the key |
| Back | Backspace | Delete the character before the cursor |
| □ (X) | | Shift: once, again for caps, again off |
| △ (Y) | Space | Space |
| Select / View | | Letters or symbols |
| L1 / R1 | ← / →, Home, End | Move the cursor |
| Start | Enter | OK |
| the *Cancel* key | Esc | Cancel |

The keys *Shift*, *#+=*, *Space*, *Delete*, *Cancel* and *OK* sit in the bottom row. A counter shows the length against the most the game takes (12 characters for a hunter name); it turns red when a key cannot be typed. Characters the game cannot take are dimmed. A hunter name may hold Latin letters, digits, space and `! # $ & ' ( ) + , - . / : ; = ? @ _ ~`. The game with the English patch draws every printable ASCII character in a name, but the asterisk comes out as a bullet, and the double quote, percent sign, asterisk, angle and square brackets, braces, backslash, caret, backquote and vertical bar are left out because text formatting may claim them.

The game keeps running behind the keyboard, as it does behind the PSP's: it keeps polling the keyboard and playing sound, and its animations go on. It reads a neutral pad until the keyboard closes, and buttons still held then reach it only after they are released. The game blanks its screen while the PSP's keyboard would cover it, so the window keeps the frame from just before the keyboard opened, dimmed. The in-game menu does not open over the keyboard.

Where SDL reports a system on-screen keyboard (such as Steam's in Big Picture or Game Mode), a *Steam* key appears in the bottom row and asks for it; what it types goes into the field like a physical keyboard. The built-in keyboard always works without it.

OK hands the text to the game as the PSP's keyboard would: UTF-16 in the field's output buffer, the field result *changed*, and the dialog status moving from visible to quit. Cancel leaves the buffer alone and reports *cancelled*; the game then keeps the name it had.

The menu's text fields (*Hunter name*, *Server*, *Nickname*) open the same keyboard when a gamepad activates them; with a keyboard or the mouse they are edited in place.

## In-game menu

Esc, or L3+R3 on a gamepad, opens Yakumo's menu over the game; the same again, back at its top level or Start closes it. Esc never quits the game: Steam's desktop controller layout on a Steam Deck sends Esc with the B button, so an Esc that arrives together with a gamepad button is ignored.

By default the game is paused while the menu is open: no guest code runs, emulated time stands still, the audio device stops, and the last frame stays behind the menu, dimmed. On resume the kernel's clock picks up from real time again, so the game neither races to make up the pause nor counts it in the `[perf]` statistics.

Two settings in the System section change that. With *Pause the game when the menu opens* off, the game keeps running behind the menu: it keeps drawing frames at its own pace, the sound keeps playing and its clock keeps running, and the menu is drawn over each frame. During ad hoc play (in a gathering hall, joining one, or hosting a session) the game keeps running behind the menu unless *Pause during multiplayer* is on, whatever the first setting says, because a paused game stops answering the other players and can drop a quest; this one is off by default. The menu's header says which applies: *Paused* or *Running*. Either way, input goes to the menu only, so moving through it never moves the hunter, and buttons still held when it closes reach the game only after they are released.

The menu follows the game's confirm convention: with the default layout the right face button (○ on a PlayStation pad, B on a Steam Deck) selects and the bottom one goes back, as in the game; with *Confirm button* set to the bottom button, both swap. The footer shows the buttons of the pad in use (PlayStation shapes or letters) or the keys, and a line explaining the focused setting. L1/R1 (LB/RB), or Q/W on the keyboard, switch between the sections. Left and right change a value; confirm steps it forward.

Every change applies at once and is saved to `settings.ini` in the per-user directory, next to the installer's `disc_image`. A setting whose environment variable is set is decided by that variable for the run: the menu shows it greyed with *Set by MHP3RD_…* and leaves the file's value alone. So the order is: environment variable, then `settings.ini`, then the default.

| Section | Setting | Key in `settings.ini` | Variable | Values |
| --- | --- | --- | --- | --- |
| Video | Resolution | `video.internal_scale` | `MHP3RD_INTERNAL_SCALE` | ×1–×6 of 480×272 (the variable allows up to ×8); default ×2 |
| Video | Display | `video.fullscreen` | | Window or fullscreen |
| Video | Window size | `video.window_scale` | | ×1–×4 of 480×272; default ×2 |
| Video | Aspect ratio | `video.keep_aspect` | | Original (black bars) or stretched to the window |
| Video | Scaling filter | `video.sharp_screen` | | Smooth or sharp scaling of the finished picture to the window |
| Video | Texture filter | `video.sharp_textures` | | Smooth (bilinear) or sharp (nearest) texture sampling |
| Video | Vsync | `video.present_mode` | | On (FIFO), or off through mailbox or immediate presentation where the driver offers them |
| Video | Game speed | `video.unthrottled` | `MHP3RD_UNTHROTTLED` | Normal (held to real time) or unlimited |
| Video | Performance | `video.performance` | `MHP3RD_PERF` | Off, overlay, overlay and log, log only |
| Video | Font | `text.font` | `MHP3RD_FONT` | Default (a Japanese system font), or an installed font; see [Game text](#game-text) |
| Video | Weight | `text.weight` | | Regular, bold (default) or heavy: thickens the game's text by 0–2 pixel columns |
| Audio | Volume | `audio.volume` | | 0–100% |
| Audio | Mute | `audio.mute` | | |
| Controls | Confirm button | `input.confirm` | `MHP3RD_PAD_FACE` | Right (○, Japanese) or bottom (Western) |
| Controls | Stick dead zone | `input.dead_zone` | `MHP3RD_PAD_DEADZONE` | 0–50% |
| Controls | Trigger point | `input.trigger` | `MHP3RD_PAD_TRIGGER` | 5–100% |
| Controls | Right stick | `input.right_stick` | `MHP3RD_PAD_RSTICK_DPAD` | Camera, D-pad or off |
| Controls | Invert camera horizontally / vertically | `input.invert_camera_x`, `input.invert_camera_y` | | For the right-stick camera |
| Controls | Right stick D-pad point | `input.right_stick_zone` | `MHP3RD_PAD_RSTICK_ZONE` | 10–100%, for the D-pad mode |
| Controls | When the game asks for a name | `input.name_entry` | `MHP3RD_OSK_MODE` | `keyboard` (default): the on-screen keyboard; `fixed`: the name below at once |
| Controls | Hunter name | `input.name` | `MHP3RD_OSK_TEXT` | Default `Hunter`; up to 12 characters. Setting the variable also answers at once unless `MHP3RD_OSK_MODE` says otherwise |
| System | Pause the game when the menu opens | `ui.menu_pause` | `MHP3RD_MENU_PAUSE` | On (default) or off: the game keeps running behind the menu |
| System | Pause during multiplayer | `ui.menu_pause_multiplayer` | `MHP3RD_MENU_PAUSE_MULTIPLAYER` | Off (default): during ad hoc play the game keeps running behind the menu; on: the setting above decides |
| System | Add a timestamp to the backup name | `saves.backup_timestamp` | | On (default): each backup from *Back up saves…* is a new folder named by its time; off: plain folder names, replaced after asking |
| Network | Ad hoc play | `network.adhoc` | `MHP3RD_ADHOC` | Off (default) or on; off, the game reports the wireless switch as off |
| Network | Server | `network.server` | `MHP3RD_ADHOC_SERVER` | Host name or address of a PSP ad hoc server, optionally `host:port`; empty by default |
| Network | Nickname | `network.nickname` | `MHP3RD_ADHOC_NICKNAME` | The name other players see; empty uses the hunter name |

Everything applies without a restart; the name settings take effect the next time the game asks for a name. The Controls section also lists the keyboard's keys, and the System section has *Resume*, *Open the data folder*, *Set up game data again…* and *Quit game* (both of the last two ask first), the *Saves* rows described under [Saving and loading](#importing-a-save-from-a-psp), and the build version, the data and saves folders and the GPU. Each section has a button that restores its defaults.

The Network section also shows the connection and has the troubleshooting tools described under [Multiplayer](#multiplayer-ad-hoc). The file also keeps `network.mac`, the address other players know you by (made up the first time you go on line; `MHP3RD_ADHOC_MAC` overrides it), `ui.menu_hint_seen`, set once the menu has been opened (until then a hint at the bottom of the screen says how to open it during the first seconds of play), and `ui.last_folder`, where the setup's file browser opens.

The interface is drawn with [Dear ImGui](third_party/imgui/README.md). Its text uses a system font: San Francisco or Helvetica on macOS, Noto Sans, DejaVu Sans or Liberation Sans on Linux, Segoe UI on Windows, with a Japanese font merged in for file names; `MHP3RD_UI_FONT` names another `.ttf`. It scales with the window: about 27-pixel text on a Steam Deck's 1280×800 screen.

## Game text

The game draws its text with the PSP's system font, which lives in the console's flash and is not on the disc, so Yakumo draws those glyphs from a font on your computer. *Font* in the menu's Video page lists the installed fonts that have every Latin letter, digit and punctuation mark, marked *Japanese* when they also have the kana and kanji the game still shows. Characters a font lacks come from the default font: Hiragino Sans on macOS, Noto Sans CJK on Linux and the Steam Deck (the `fonts-noto-cjk` package or its equivalent; a release falls back to the copy it ships), MS Gothic or Meiryo on Windows. To use a font that is not installed, put its `.ttf`, `.otf`, `.ttc` or `.otc` file into the `fonts` folder of the per-user directory (*Open the fonts folder* in the same section); those are listed first. A preview line under the setting shows the choice the way the game draws it.

A change applies at once: Yakumo makes the game draw every character again the next time it shows it, so text already on screen changes within a frame or two.

How the text is laid out, as traced with `MHP3RD_TRACE_FONT=1`: the game sizes a glyph cell in a texture atlas from the font's maximum glyph size, renders each glyph into a 20×20 buffer and copies that whole buffer into the cell, and draws text as one sprite per cell, half a character wide for Latin letters and full width for Japanese ones. Yakumo reports a 20×20 maximum so cells and buffer match, and fits every glyph inside its cell with a pixel of margin, shifting it and, when it is too large, scaling it down, so no font can spill into a neighbour or lose its edges. The size of the text is therefore fixed by the game; *Weight* is the adjustment that fits within it.

## Saving and loading

The game saves through the PSP's save-data utility, which the host implements. Saves live where a PSP keeps them, under the directory that backs `ms0:`: `ms0` in the [per-user data directory](#installer) for an installation, or `ms0` in the game directory when the game runs from one (`profiles/mhp3rd/game`, `MHP3RD_GAME_DIR` or the `game_dir` argument):

```text
game/ms0/PSP/SAVEDATA/ULJM05800/
    PARAM.SFO      titles, the file list and the hashes that protect the save
    MHP3RD.BIN     the game data, encrypted
    ICON0.PNG      icon shown in the PSP's save list
    PIC1.PNG       background shown in the PSP's save list
```

`MHP3RD.BIN` is encrypted and `PARAM.SFO` hashed exactly as the PSP's save-data utility does it, with the key the game supplies, so a folder can move between this port and a PSP's memory stick unchanged. The one field that cannot be reproduced is a hash made with a key unique to each PSP; the port writes a placeholder there. Copying a save made here onto a real PSP has not been tried yet.

Nothing is drawn for the save-data or message dialogs yet ([#33](https://github.com/TeamGDB/Yakumo/issues/33)): the game's own screens ask where to save and show the result, and the system dialogs answer as if the player confirmed them. The log shows each request and each message, for example `[savedata] AUTOSAVE (1) game="ULJM05800" …` followed by `[savedata] saved 1183744 bytes to …`.

### Importing a save from a PSP

The in-game menu does it (Esc, or L3+R3 on a gamepad; System section, *Saves*):

1. Copy the save to this machine, or connect the memory stick: on a PSP's memory stick the folder of *Monster Hunter Portable 3rd* is `PSP/SAVEDATA/ULJM05800`, with downloaded quests in `ULJM05800QST`. PPSSPP keeps the same folders in its `memstick/PSP/SAVEDATA`.
2. Choose *Import save…*. The file browser lists folders. Opening a save folder chooses it; *Import from this folder* chooses the folder shown, and every save in it, in its `SAVEDATA` or in its `PSP/SAVEDATA` is found, so a memory stick's root or a `SAVEDATA` folder holding several games works too.
3. Each save found is checked, then shown with its date and size beside the save it would replace. Nothing is copied until you choose *Import* (or *Replace and import*).
4. The game reads its saves at the title screen. *Restart now* closes the game and starts it again there; progress since your last save is lost. If you keep playing instead, do not save before restarting, or the game writes its own data over the imported save.

**Checks.** A folder is imported only when its `PARAM.SFO` names one of this game's folders (`ULJM05800`, `ULJM05800QST`, `ULJM05800DAT`), its own hashes match, and every data file it lists is present, matches its hash and decrypts with the key the game passes to the save-data utility. Other games' saves are left out (and counted), and a damaged one is refused with the reason. The key is taken from the game's own first save-data request, at boot; until then, saves cannot be checked and the menu says to try again at the title screen. A save stored without encryption, as early PPSSPP versions wrote them, has nothing to check and is imported as it is.

**Nothing is deleted.** A save that an import replaces is moved to `ms0/PSP/SAVEDATA/.backup/<date>_<time>/<folder>/`, for example `.backup/2026-09-19_19-05-12/ULJM05800/`; import that folder to go back to it. `.zip` files are not read: unpack them first.

By hand, the same works with the game closed: copy the folder into `ms0/PSP/SAVEDATA/` in the directory above (for example `~/.local/share/Yakumo/MHP3rd/ms0/PSP/SAVEDATA/`), after keeping a copy of any folder of the same name, which holds all three character slots. *Open the saves folder* in the menu shows that folder.

### Exporting and backing up

- **Export save…** copies the game data and the downloaded quests to a folder you choose, as `MHP3rd saves <date>_<time>/PSP/SAVEDATA/ULJM05800…`, the layout of a memory stick: copy its `PSP` folder to the root of a PSP's memory stick, or import it on another machine. It always makes a new folder.
- **Back up saves…** copies every save folder of the game, the install data included, to the backups folder, `save-backups` in the [per-user data directory](#installer), or to a folder you choose. With *Add a timestamp to the backup name* (on by default; `saves.backup_timestamp` in `settings.ini`) each backup is a new folder named by its time, holding the save folders: `save-backups/2026-09-19_19-05-12/ULJM05800/`. With it off, the save folders go straight into the chosen folder (`save-backups/ULJM05800/`), and an earlier backup there is replaced only after you confirm. *Open the backups folder* shows it. To restore a backup, import it.

### Downloadable content

The game keeps downloaded quests and equipment in the `ULJM05800QST` save folder and reads it through the save-data utility, like an ordinary save (AUTOLOAD of `ULJM05800QST` / `MHP3RD.BIN`). The download servers are long gone, so the in-game download mode's network side stays unimplemented. To use DLC you already have, import your `ULJM05800QST` folder from the menu (or copy it into `ms0/PSP/SAVEDATA/`), then open the game's download menu to install the quests. The project does not host, bundle or link to DLC files.

This release (`NPJB-40001`) asks for the original PSP release's folder names (`ULJM05800`), and the key it passes is the PSP release's: a downloaded-quest folder written by a PSP running `ULJM-05800` passes every check with it and decrypts. The two releases therefore share one save format, and saves should move between them in both directions; a save from this release has not yet been loaded on a PSP. When there is no save of its own, the game also looks for saves of *Monster Hunter Portable 2nd G* (`ULJM05500`) and *Monster Hunter Diary: Poka Poka Airu Village* (`ULJM05710`); those would be read from `ms0` the same way, which has not been tried.

To check a save folder without starting the game — for example one that the game reports as corrupted — run the save-data test program on it:

```bash
out/mhp3rd/bin/mhp3rd_savedata_tests --check profiles/mhp3rd/game/ms0/PSP/SAVEDATA/ULJM05800 MHP3RD.BIN <key>
```

`<key>` is the 32-digit key the game passes to the save-data utility; a run with `MHP3RD_TRACE_SAVEDATA=1` prints it as `key=` on every request for the game's own save. The program reports whether the hashes in `PARAM.SFO` and the data file's hash match and whether the file decrypts. Without arguments it runs the self-tests, which need no game data.

## Multiplayer (ad hoc)

The PSP game plays together through ad hoc wireless: up to four consoles in the same room. Yakumo carries that over a network through a **PSP ad hoc server**. One player can host a session from the game itself, with the server built in (see [Play together locally](#play-together-locally)), or everyone can use a public server that PSP and PPSSPP players use, so you can hunt with other Yakumo players and, as the protocol is the same, with players on PPSSPP (not tested yet). Nothing has to be forwarded on your router for a public server: all game traffic goes through the server.

### Play together locally

On one home network, or over a VPN such as Tailscale or ZeroTier, one player hosts and the others join; nothing else needs to be installed and nothing needs to be forwarded.

1. **Host.** Open the menu (Esc, or L3+R3), go to **Network** and choose **Host a session**. The server starts inside the game, and the screen lists the addresses the others can use, one per network (for example `192.168.1.20  Local network (en0)` and `100.101.102.103  Tailscale`). Selecting one copies it. Below them are the players connected to your session and the hall each is in; **Stop hosting** ends the session for everyone.
2. **Join.** The others open **Network** too. Under **Join a session** they see the sessions hosted on their network, with the host's name and player count, and choose **Join**. On a VPN that carries no broadcast, such as Tailscale or most WireGuard setups, the host's session is not listed by itself: type the host's VPN address into **Address** and press Enter. Joined addresses are remembered under the same heading.
3. **Play.** Everyone closes the menu, goes up the stairs in the village to the gathering hall entrance, chooses **Online Guild Hall** and picks the same hall, as with any server.

Hosting and joining turn **Ad hoc play** on and fill in **Server** (the host's own game uses its server on `127.0.0.1`). Joining another session, or hosting, while you are in a hall takes you out of it, as a dropped connection would. While you play together, opening the menu does not pause the game unless *Pause during multiplayer* is on (System section), so the others keep hearing from you.

**Ports.** The built-in server listens on every network interface on TCP **27312** (matchmaking) and **27313** (relay); a host answers address checks on UDP 27312 and announces itself to UDP **27314** on the local network, by broadcast and on the multicast group 239.255.27.14. On a LAN or a VPN nothing needs to be forwarded, though a firewall on the host may ask to let Yakumo accept connections: allow it (on Windows, for private networks). To host over the plain internet, forward TCP 27312 and 27313 on the host's router to the host, and give the others your public address; a VPN is usually simpler. `network.host_port` in `settings.ini` (or `MHP3RD_ADHOC_HOST_PORT`) moves the server to another port pair, for example when another server already uses 27312; others then join `address:port`.

**PPSSPP players** should be able to join a session hosted in Yakumo, since the server speaks the same protocols (not tested yet): in PPSSPP, set the ad hoc server to the host's address and use the relay (*AemuPostoffice*) data mode.

**A server without the game.** `Yakumo --adhoc-server [port]` runs the same server alone in a terminal, announced on the local network, and prints the addresses to join; Ctrl+C stops it. It needs neither game data nor a window.

A server has two parts, both over TCP: the matchmaking service on port **27312**, which knows who is in which gathering hall, and a relay on port **27313**, which carries the game's own traffic between the players. Yakumo needs both, so pick a server that runs the relay (servers list it as *AemuPostoffice* data mode).

### Setting it up

1. Open the menu (Esc, or L3+R3) and go to **Network**.
2. Turn **Ad hoc play** on.
3. Enter the **Server**: a host name or IP address, `host:port` if its matchmaking port is not 27312. There is no default; see [Choosing a server](#choosing-a-server).
4. Optionally set a **Nickname**; otherwise the other players see your hunter name.
5. Close the menu. In the village, go up the stairs to the gathering hall entrance and choose **Online Guild Hall** (✕), then a hall. Everyone who picks the same hall number on the same server meets there.

Server and nickname changes apply the next time the game goes on line: leave the hall and enter it again. Turning ad hoc play off while in a hall takes you out of it, as if the connection dropped. The address other players know you by (`network.mac` in `settings.ini`) is made up once and kept.

Everyone in a hall must play the same game: this release and the PSP's *Monster Hunter Portable 3rd* (`ULJM-05800`) are the same game on the server, so players of the PSP version on PPSSPP can join.

### Choosing a server

PPSSPP's list of public ad hoc servers is in its [`assets/adhoc-servers.json`](https://github.com/hrydgard/ppsspp/blob/master/assets/adhoc-servers.json); its entries say which games each server's community plays and which data mode it runs. Choose one that runs the relay and whose players play Monster Hunter, near you if you can, and agree on it with the people you want to play with. Each server has a status page (usually on port 8888) that shows who is on line in which game.

Public servers are run by volunteers. Yakumo keeps one connection to the matchmaking service and one to the relay per game socket, and pings the matchmaking service every two seconds, like the other clients.

### Running your own server

The simplest is **Host a session** in the game, or `Yakumo --adhoc-server` (see [Play together locally](#play-together-locally)). You can also run [aemu_postoffice](https://github.com/Kethen/aemu_postoffice), the server most public servers use. It is a separate program under its own licence; nothing of it is part of Yakumo.

Natively, on macOS or Linux (a C++20 compiler is all it needs):

```bash
git clone --recursive https://github.com/Kethen/aemu_postoffice
cd aemu_postoffice/server_cpp
bash build_linux.sh
./aemu_postoffice          # config.json and game_db.json must be next to it
```

In a container (Docker or Podman), from the same `aemu_postoffice` checkout:

```bash
docker run --rm -it -p 27312:27312 -p 27313:27313 -p 8888:8888 \
  -v "$PWD":/src -w /src/server_cpp debian:stable \
  bash -c 'apt-get update && apt-get install -y g++ && bash build_linux.sh && ./aemu_postoffice'
```

Then use `127.0.0.1` as the server on the same machine, or the machine's LAN address on the others. Open TCP 27312 and 27313 in its firewall for other machines. The server log shows every login, group join and relay session, and `http://<server>:8888/` lists who is on line.

### Two instances on one machine

Each instance needs its own settings (for its own address and nickname) and its own copy of the save:

```bash
# once: a game directory per instance with its own save
mkdir -p ~/yakumo-b/ms0/PSP/SAVEDATA
ln -s /path/to/disc.iso ~/yakumo-b/disc.iso
ln -s /path/to/EBOOT.ELF ~/yakumo-b/EBOOT.ELF
cp -R profiles/mhp3rd/game/ms0/PSP/SAVEDATA/ULJM05800 ~/yakumo-b/ms0/PSP/SAVEDATA/

# each instance: its own data directory, window title, server and nickname
MHP3RD_DATA_DIR=~/yakumo-a-data MHP3RD_WINDOW_TITLE="Yakumo A" MHP3RD_ADHOC=1 \
  MHP3RD_ADHOC_SERVER=127.0.0.1 MHP3RD_ADHOC_NICKNAME=HunterA out/mhp3rd/bin/Yakumo profiles/mhp3rd/game
MHP3RD_DATA_DIR=~/yakumo-b-data MHP3RD_WINDOW_TITLE="Yakumo B" MHP3RD_ADHOC=1 \
  MHP3RD_ADHOC_SERVER=127.0.0.1 MHP3RD_ADHOC_NICKNAME=HunterB out/mhp3rd/bin/Yakumo ~/yakumo-b
```

Two characters from one save are fine in one hall, since the game tells players apart by their address, and each instance makes up its own.

### When something goes wrong

The menu's **Network** section shows what the connection is doing, updated live:

| Row | Shows |
| --- | --- |
| Connection | Off line, connecting, reconnecting (with the attempt and the last error), or on line with how long and the server connection's round trip |
| Server | While you host: its ports, uptime, players, halls, relay connections and streams, and what it has relayed and dropped |
| Discovery | Whether this instance listens for hosted sessions on UDP 27314 and how many it hears, and while you host, on how many interfaces it announces yours and how many address checks it answered |
| One row per host | Each session heard on the network: its address, players, game and when it was last heard |
| You | Your address and nickname |
| Group | The hall's group (`MHP3Q000` is Hall 01) and how many players are in it, or that it is being rejoined after a dropped connection |
| One row per player | Their address and how long ago their last packet arrived |
| One row per socket | Each ad hoc socket the game has open: its kind (PDP datagrams, PTP streams), port, state and peer |
| Relay links | How many of the sockets' relay connections are up |
| Per second, Since start | Packets and bytes in and out |
| Problems | Datagrams dropped, calls that timed out, reconnections |

Below it:

- **Network overlay** shows the connection, the group, its players and the traffic in the top-right corner while you play. `MHP3RD_ADHOC_OVERLAY=1` turns it on at start.
- **Log every call and packet** is the same as `MHP3RD_TRACE_ADHOC=1`: every ad hoc call the game makes, with its arguments and result, and every packet header goes to the console and to the network log.
- **Save network log** writes the recent network log and the section's state to `logs/adhoc-<date>-<time>.log` in the data folder (menu: System, *Open the data folder*). Attach it to a problem report, ideally with the log turned on before the problem happens.
- **Reconnect now** drops the server connection and connects again; the hall is joined again. **Disconnect** leaves the hall as if the other players were lost. The game reacts to both as to a real dropped connection.

Common problems:

- *The game says the wireless switch is off*: ad hoc play is off in the menu.
- *Join a session lists nothing*: the host is on a VPN without broadcast (type its address), on another network, or a firewall blocks UDP 27314 on your side. **Discovery** says whether this instance listens.
- *Host a session says the port is in use*: another server runs on this machine; stop it, or set `network.host_port`.
- *Connecting never finishes*: the server name is wrong, the server is down, or a firewall blocks TCP 27312. The console says `cannot reach the ad hoc server` or `cannot resolve`.
- *You are in a hall but see nobody*: the other player is on another server, in another hall, or playing a game the server does not group with this one. The server's status page shows where everyone is.
- *Players see each other but a quest cannot be joined or the hall drops*: the server has no relay (TCP 27313), or it is blocked. **Relay links** stays below its total.
- *The connection drops in a quest*: when the server connection comes back within ten seconds, the hall is rejoined; the quest itself usually ends, as it would on a PSP. After ten seconds the game is told the connection is lost.

### How it works

The game uses the PSP's ad hoc libraries (`sceNetAdhocctl`, `sceNetAdhoc`, and the network configuration dialog `sceUtilityNetconf`). Entering the Online Guild Hall, it scans for halls, then asks the network dialog to join the hall's group (`MHP3Q000` for Hall 01); Yakumo joins it on the server and the dialog finishes when the server confirms. In the hall every console broadcasts its state over PDP (datagrams on port 10000), which Yakumo sends through the relay to each player in the group. A quest is a PTP stream: the host listens on port 20001 and each joining player connects to it, also through the relay.

One network thread owns every connection to the server, so the game never waits for the network except where a PSP call itself blocks, and then no longer than the call's own timeout. During ad hoc play the game keeps running behind the menu by default (see [In-game menu](#in-game-menu)); if it is paused, the connection stays up, but the other players stop hearing from the game until the menu closes.

## Configuration

The settings a player needs are in the [in-game menu](#in-game-menu). Environment variables remain for everything else, and override the menu's settings for the run where they overlap.

### Game and paths

| Variable | Default | Effect |
| --- | --- | --- |
| `MHP3RD_GAME_DIR` | unset | Directory holding `EBOOT.ELF`, `disc.iso` and `ms0/` (the saves); skips the per-user directory |
| `MHP3RD_DATA_DIR` | SDL's preference path | Per-user data directory the installer fills, with the saves in its `ms0/` |
| `MHP3RD_OVERLAY_DIR` | `overlays/` next to the executable | Directory of overlay libraries |
| `MHP3RD_FONT` | a system CJK font | Font to draw the game's text with: a `.ttf`, `.otf`, `.ttc` or `.otc` file, with `#N` after the path for the Nth face of a collection. Glyphs it lacks come from the default, a Japanese system font (Hiragino on macOS, Noto Sans CJK on Linux, MS Gothic or Meiryo on Windows; inside a Flatpak, the host's Noto Sans CJK under `/run/host/fonts`), and last the font a release ships in `fonts/` next to the executable |
| `MHP3RD_UI_FONT` | a system font | TrueType font for Yakumo's menu and setup screens |

### Video

| Variable | Default | Effect |
| --- | --- | --- |
| `MHP3RD_INTERNAL_SCALE` | `2` | Render resolution as a multiple of 480×272 (menu: Resolution) |
| `MHP3RD_NO_RENDER` | off | Run without a window; the installer shows no dialogs either. Emulated time is not held to real time |
| `MHP3RD_WINDOW_TITLE` | `Yakumo` | Title of the game window, to tell instances apart |
| `MHP3RD_UNTHROTTLED` | off | Let emulated time run ahead of real time, so the game runs as fast as it can be drawn (menu: Game speed) |
| `MHP3RD_NO_MATERIAL_COLOR` | off | Leave unlit geometry without vertex colours white instead of taking the material colour |
| `MHP3RD_NO_LIGHTING` | off | Draw lit geometry with the flat white stand-in used before lighting existed, and without fog, to compare a scene with and without them |
| `MHP3RD_NO_FOG` | off | Turn fog off and keep lighting |
| `MHP3RD_NO_FB_TEXTURES` | off | Decode every texture from guest memory, as before, instead of sampling the render target when the game textures from a framebuffer it drew, and stop writing framebuffers back to guest memory for the shown frame and for GE block transfers |
| `MHP3RD_NO_SPRITE_CLAMP` | off | Let 2D tiles sample outside their own texels, as before; above ×1 this shows faint lines along the tile edges of 2D screens |
| `MHP3RD_SCREENSHOT_DIR` | unset | Write BMP frames into this directory |
| `MHP3RD_SCREENSHOT_EVERY` | `60` | Frames between screenshots |
| `MHP3RD_PERF` | off | `1` shows the performance overlay and logs frame statistics once per second; `log` only logs them (menu: Performance). See [Performance statistics](#performance-statistics) |

### Audio

| Variable | Default | Effect |
| --- | --- | --- |
| `MHP3RD_NO_AUDIO` | off | Do not open a playback device; the game's audio timing is unchanged |
| `MHP3RD_AUDIO_DUMP` | unset | Write the mixed output to a 44100 Hz stereo WAV file |

### Input

| Variable | Default | Effect |
| --- | --- | --- |
| `MHP3RD_PAD_FACE` | positional | `xbox` puts confirm (○) on the south button (menu: Confirm button) |
| `MHP3RD_PAD_DEADZONE` | `0.15` | Left-stick dead zone, as a fraction of travel (menu: Stick dead zone) |
| `MHP3RD_PAD_TRIGGER` | `0.25` | How far LT/RT travel before they press L/R (menu: Trigger point) |
| `MHP3RD_ANALOG_CAMERA` | off | Turn the camera as far as the stick is pushed, instead of at the game's one fixed speed of 189.6°/s. The port drives the game's own camera angle and keeps the second stick centred as far as the game is concerned, so the two never fight; off writes nothing at all and the camera is the game's own (menu: Analog camera) |
| `MHP3RD_CAMERA_SPEED` | `190` | Degrees a second at full deflection, 20 to 720 (menu: Camera speed) |
| `MHP3RD_VERTICAL_CAMERA` | off | Look up and down as far as the stick is pushed, instead of the game's one-shot steps and glides. Needs the analog camera; below the dead zone the game keeps the camera, so its own vertical behaviour is left alone (menu: Vertical camera) |
| `MHP3RD_PAD_RSTICK_DPAD` | off | Press D-pad bits from the right stick instead of feeding the HD release's second stick; enabling both would turn the camera twice (menu: Right stick) |
| `MHP3RD_PAD_RSTICK_ZONE` | `0.5` | Right-stick threshold for that (menu: Right stick D-pad point) |
| `MHP3RD_OSK_TEXT` | `Hunter` | Fixed name given when the game asks for one, at once and without the on-screen keyboard unless `MHP3RD_OSK_MODE=keyboard` (menu: Hunter name) |
| `MHP3RD_OSK_MODE` | `keyboard` | `keyboard` opens the on-screen keyboard; `fixed` gives the fixed name at once (menu: When the game asks for a name) |
| `MHP3RD_AUTO_CONFIRM` | off | Press ○ every N frames, to walk through menus unattended |

### Network

| Variable | Default | Effect |
| --- | --- | --- |
| `MHP3RD_ADHOC` | off | `1` turns ad hoc play on (menu: Ad hoc play) |
| `MHP3RD_ADHOC_SERVER` | none | PSP ad hoc server, `host` or `host:port` (menu: Server) |
| `MHP3RD_ADHOC_NICKNAME` | the hunter name | Name other players see (menu: Nickname) |
| `MHP3RD_ADHOC_MAC` | made up once | The address other players know you by, `xx:xx:xx:xx:xx:xx` |
| `MHP3RD_ADHOC_OVERLAY` | off | `1` shows the network overlay from the start |
| `MHP3RD_ADHOC_HOST_PORT` | `27312` | TCP port of the built-in server's matchmaking service; the relay uses the next one (`network.host_port`) |

### Diagnostics

| Variable | Effect |
| --- | --- |
| `MHP3RD_STRICT_HLE=1` | Do not bind logging stubs; stop at the first unimplemented import |
| `MHP3RD_TRACE_KERNEL=1`, `MHP3RD_TRACE_IO=1` | Trace thread and file activity |
| `MHP3RD_TRACE_SAVEDATA=1` | Log every field of each save-data request and each status poll |
| `MHP3RD_TRACE_SYNC=1` | Trace semaphores, event flags and mutexes; `MHP3RD_TRACE_SYNC_LIMIT` caps the lines (default 4000) |
| `MHP3RD_STARVATION_INTERVAL` | Dispatches between virtual-clock advances in code that never calls an import |
| `MHP3RD_TRACE_GE=1` | Log the first draws of the run with their state |
| `MHP3RD_TRACE_3D=1` | Per-frame counts of transformed draws, their targets and screen-space bounds |
| `MHP3RD_FIND_CAMERA=1` | Hunt guest memory for the words the camera is kept in, by what they do: one hunt against the yaw the view matrix reports and one against its pitch, trying every word as a float, a 32-bit and a 16-bit number, and as an angle, a rate, or a rate read a frame early. `MHP3RD_FIND_CAMERA_OUT` names a file the surviving list is written to |
| `MHP3RD_FIND_STEP=N` | Keep the 16-bit fields that move by exactly N between turning frames. The camera's own yaw moves by 1150, which is 1150/65536 of a turn |
| `MHP3RD_FIND_VERTICAL=1` | Hunt for the byte holding the camera's vertical level, by the fact that it steps when the vertical command fires and holds still in between. It does not yet find it reliably; see #106 |
| `MHP3RD_FIND_FLOAT=V`, `MHP3RD_FIND_INT32=N` | List every place in guest memory holding that value. `MHP3RD_FIND_INT16_WIDE=1` searches 16-bit fields instead of 32-bit |
| `MHP3RD_POKE_FOUND=V`, `MHP3RD_POKE_INT32=N` | Write a different value into what those found, once a frame. With more than one match, `MHP3RD_POKE_WHICH` must name an index or say `all`, since writing every place that held a number also writes whatever else held it |
| `MHP3RD_POKE_FLOAT=0xADDRESS:V[,...]` | Write floats into guest memory once a frame, to turn a guess about a constant into a measurement |
| `MHP3RD_CAMERA_INSTANT=1` | With the analog camera on, drive the angle the view is built from as well as the camera's target, which removes the game's own smoothing |
| `MHP3RD_TRACE_CAMERA=1` | One line per frame for the camera the game itself set: the second stick's offset from centre, the yaw and pitch read out of the frame's busiest view matrix, the turn since the previous frame, and the camera's world position. Reads the game's own camera, so it tells a stepped turn from a continuous one |
| `MHP3RD_TRACE_FB_TEXTURES=1` | Each distinct texture that lies in a framebuffer the renderer drew (with both layouts), each large texture, `sceDmacMemcpy` copies into or out of VRAM, GE block transfers, new render targets, and the GE commands the renderer ignores. Add `PSPRECOMP_TRACE_VRAM_READS=1` to log game code reading VRAM with the CPU, per 64 KiB block and at most once a second |
| `MHP3RD_TRACE_SPRITES=N` | Every through-mode draw of presented frame N: sprites one by one, other primitives by their bounds, with positions, texture coordinates and texture state |
| `MHP3RD_TRACE_MATERIAL=1` | Every distinct value the game writes to the GE material registers |
| `MHP3RD_TRACE_LIGHTING=1` | Every distinct value the game writes to the GE light and fog registers, and one line per distinct register state a lit draw is made with |
| `MHP3RD_SAMPLED_TEXTURE_KEYS=1` | Recognise changed textures of up to 64 KiB by one word in every 256 bytes, as for larger ones, instead of by all of their contents. Glyphs the game adds to its text atlas are then often missed, and text shows stale or missing characters |
| `MHP3RD_NO_CULL=1`, `MHP3RD_NO_DEPTH=1` | Disable face culling or the depth test, to bisect missing geometry |
| `MHP3RD_TRACE_AUDIO=1` | One line per second of output: frames, peak, RMS, silence and drops |
| `MHP3RD_TRACE_ATRAC=1` | Every `sceAtrac3plus` call with its arguments, result and decode position |
| `MHP3RD_TRACE_FONT=1` | Every `sceLibFont` call with its arguments: the font the game asks for, the font info and character metrics returned, and each glyph image's buffer and 26.6 position, with the caller's return address |
| `MHP3RD_TRACE_MPEG=1` | Every `sceMpeg` and `sceJpegCsc` call, and each call the ring buffer makes to the game's read callback |
| `MHP3RD_SAS_NO_ENV=1` | Hold every SAS voice at full envelope, to separate an envelope bug from a decoding one |
| `MHP3RD_TRACE_PAD=1` | Log the pad state whenever it changes |
| `MHP3RD_TRACE_OSK=1` | Every keyboard utility call with the status it returns, and the words of the parameter block, its first field and the strings they point to |
| `MHP3RD_TRACE_ADHOC=1` | Every ad hoc, network dialog and wireless call with its arguments and result, and every packet header sent to or received from the ad hoc server (menu: Network, *Log every call and packet*) |
| `MHP3RD_INPUT_SCRIPT` | Scripted keys, virtual-gamepad buttons and axes, dropped files and window captures, for testing the menu, the setup and the on-screen keyboard without a person at the controls; the syntax is in `host/ui/input_script.hpp`. Its virtual gamepad also becomes the game's pad, in place of a real one that is connected. Example: `300:key Escape;330:shot menu;360:pad leftstick+rightstick` |
| `MHP3RD_INPUT_LIVE` | A file read while the game runs; each line appended to it is an input-script step timed from when it is read, to drive two instances side by side |
| `MHP3RD_DUMP_OVERLAYS` | Directory to dump an overlay that has no library into |
| `PSPRECOMP_NO_INTERPRETER=1` | Stop at uncompiled code instead of interpreting it |
| `PSPRECOMP_MAX_DISPATCHES` | Stop after this many dispatches |
| `PSPRECOMP_HLE_HISTOGRAM=1` | Print import call counts on exit |

### Performance statistics

With `MHP3RD_PERF=1` the game draws a small overlay into the top-left corner of the presented image, so it appears in window and Steam screenshots and in `MHP3RD_SCREENSHOT_DIR` captures, and prints one line per second to stdout, flushed as it is written:

```text
[perf] fps 30.0 game 30.0 speed 100% | frame avg 33.4 max 34.7 ms | guest 4.1 render 9.8 wait 19.5 ms | lists 60/s | FIFO 1440x816 90Hz | overlay 0.05 ms
```

`MHP3RD_PERF=log` prints the line without the overlay. F3 shows or hides the overlay at any time, with or without the variable; there is deliberately no gamepad combination for it. The menu's *Performance* setting chooses the same modes, plus the overlay without the log. The statistics are collected all the time, so turning them on changes nothing else.

A frame runs from one guest flip (`sceDisplaySetFrameBuf`, where the renderer presents) to the next.

| Field | Meaning |
| --- | --- |
| `fps` | Frames presented per second of real time |
| `game` | Frames the game flips per second of *emulated* time: its own frame rate, 30 when it keeps up with its target |
| `speed` | Emulated time per real time: 100% when the game runs at PSP speed. The kernel holds its clock to real time, so it stays at 100% unless frames take longer than the game's frame time; below 100% the game runs slow |
| `frame avg`, `max` | Real time between presents over the last second |
| `guest` | The rest of the frame: recompiled code, HLE, the kernel, input and audio |
| `render` | CPU time turning display lists into Vulkan commands and recording the present, without the GPU waits inside it. The kernel's hold to real time happens outside it and does not reduce it |
| `wait` | Time blocked on the GPU: the frame fence, swapchain acquire, queue submit and present, and the queue idle waits of texture uploads. With FIFO presentation, pacing to the display shows up here, and so does the time the kernel waits to hold the game to real time |
| `lists` | Display lists enqueued per second of real time |
| last part | Present mode, swapchain size and the display's refresh rate as SDL reports it |
| `overlay` | CPU time spent drawing the overlay, when it is shown |

The overlay shows the same numbers and a graph of the last 192 frame times, from 0 to 50 ms, with guides at 16.7 and 33.3 ms: green up to 34 ms, yellow up to 50 ms, red beyond.

## Host layout

```text
host/main.cpp                    Entry point: finding the game data, executable check, startup
host/app_paths.{hpp,cpp}         The executable's own location, and what a release ships next to it
host/install/                    First-run installer: per-user directory, image checks, executable preparation
host/settings/                   Player settings: settings.ini, environment overrides, defaults
host/ui/                         Yakumo's own interface (Dear ImGui): in-game menu, setup screens, file browser, on-screen keyboard
host/overlays.{hpp,cpp}          Overlay library loading and run-time installation
host/kernel/kernel.{hpp,cpp}     Scheduler, waits, virtual clock, interrupts, memory
host/kernel/iso_image.{hpp,cpp}  Read-only ISO 9660 view of the disc image
host/hle/hle_threadman.cpp       ThreadManForUser, Kernel_Library
host/hle/hle_sysmem.cpp          SysMemUserForUser, sceSuspendForUser, sceDmac
host/hle/hle_io.cpp              IoFileMgrForUser, sceUmdUser
host/hle/hle_system.cpp          Utils, LoadExec, Stdio, ModuleMgr, interrupts, power, RTC
host/hle/hle_media.cpp           sceDisplay, sceCtrl, sceGe_user, sceAudio, sceSasCore
host/hle/hle_atrac.cpp           sceAtrac3plus: ATRAC3 music decoded frame by frame, loops, positions
host/hle/hle_mpeg.cpp            sceMpeg and sceJpegCsc: the movie player's ring buffer, access units and decoding
host/hle/hle_font.cpp            sceLibFont over a host font
host/fonts/game_font.*           The game's text font: loading, fitting glyphs into the game's cells, fallback, installed fonts
host/hle/hle_utility.cpp         sceUtility on-screen keyboard and message dialog
host/hle/hle_savedata.cpp        sceUtility save-data dialog
host/hle/hle_adhoc.cpp           sceNet, sceNetAdhoc, sceNetAdhocctl, sceNetAdhocDiscover, sceWlanDrv, sceUtilityNetconf
host/adhoc/                      Client for PSP ad hoc servers: wire formats, network thread, diagnostics
host/hle/utility_dialog.hpp      Status life cycle shared by the dialogs
host/save_data/                  AES-128, PARAM.SFO, the save-data encryption and hashes, save folders
host/gpu/ge_state.{hpp,cpp}      GE command state machine: display lists to draw calls
host/gpu/vulkan_renderer.*       Vulkan backend, window and input
host/gpu/shaders/                GLSL, compiled to SPIR-V and embedded at build time
host/perf/frame_stats.*          Frame timing, the per-second summary and the [perf] log line
host/perf/perf_overlay.*         Performance overlay drawn on the CPU with a built-in 5x7 font
host/audio/audio_sink.*          SDL3 playback device and the mixing ring buffer
host/audio/sas_core.*            Software SAS: VAG decoding, pitch, envelopes, 32 voices
host/audio/atrac_decoder.*       ATRAC3 and ATRAC3plus frames to PCM through FFmpeg's libavcodec
host/movie/psmf_demuxer.*        PSMF program stream packs to H.264 pictures and ATRAC3plus frames
host/movie/avc_decoder.*         H.264 pictures to planar YCbCr through FFmpeg's libavcodec
```

Every import runs at the outer dispatch level, so a blocking import saves the caller's context with `pc = $ra` and loads another thread's context; the runtime's thread identity check keeps generated code from resuming in the wrong thread.

## Directory layout

```text
config/       Executable identity and overlay slot map
host/         Bootstrap, kernel, HLE, graphics, audio
scripts/      prepare_game.sh, generate.sh, build_overlays.sh, bootstrap_overlays.sh, release_linux.sh
packaging/    Release packaging: third-party notices; linux/ holds the Flatpak manifest, launcher and SDK build
tools/        ISO and DATA.BIN extraction, overlay wrapping, shader and NID table embedding
tests/        Save-data self-tests and save checker (mhp3rd_savedata_tests)
third_party/  stb_truetype, tiny-AES-c (installer and saves), Dear ImGui (menu and setup screens)
game/         Local game data: EBOOT.ELF, disc.iso, ms0/ (ignored)
analysis/     Analyzer output and extracted overlays (ignored)
generated/    Recompiled executable (ignored)
overlays/     One directory per recompiled overlay (ignored)
```
