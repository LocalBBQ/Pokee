# MHP3rd HD profile

This profile builds `MHP3rdNative` for **Monster Hunter Portable 3rd HD Ver.** (`NPJB-40001`). The PS3 release ships an ordinary PSP UMD image; this profile recompiles that PSP executable and its code overlays into a native program. No executable, game data or code generated from them is part of the repository — you supply your own copy of the game.

## Status

The game boots, loads its overlays, creates a character or loads a save, walks the village, plays hunts and saves, with sound, music, movies, lighting, a keyboard and a gamepad, at the PSP's speed.

| Area | State |
| --- | --- |
| Code | The whole executable (362 478 instructions, 89 units) and all 355 code overlays are recompiled ahead of time; an interpreter covers anything they miss |
| Kernel | Threads with a deterministic virtual clock, semaphores, event flags, mutexes, callbacks, VTimers, partition memory, VBlank interrupts, file I/O straight from the disc image |
| Imports | 185 of 296 implemented; the rest are logging stubs that return 0 |
| Graphics | Vulkan: textures (palettes, DXT, swizzle), skinning, per-vertex lighting (four directional, point or spot lights and the full material model) and fog, blending, depth and alpha test, sprites, per-framebuffer render targets |
| Audio | `sceSasCore` voice mixing, `sceAudio` output and ATRAC3 music through `sceAtrac3plus` |
| Movies | PSMF playback through `sceMpeg` and `sceJpegCsc`: H.264 video and ATRAC3plus sound |
| Input | Keyboard and SDL3 gamepads, including the HD release's second analog stick |
| Text | `sceLibFont` glyphs rasterized from a host TrueType font |
| Saves | The save-data utility, with saves in the PSP's own format: a save copied from a PSP loads, and one made here can be copied back |

Not done yet:

- **Curved surfaces** (Bézier and spline patches).
- **Networking.** The ad hoc client for multiplayer is in review ([#2](https://github.com/TeamGDB/Yakumo/issues/2)).
- **Dialog screens.** The save-data and message dialogs work but draw nothing; each answers as if the player confirmed it ([#33](https://github.com/TeamGDB/Yakumo/issues/33)).
- **Rendering details:** a framebuffer used as a texture shows noise, on the quest reward screen for one ([#48](https://github.com/TeamGDB/Yakumo/issues/48)); some text glyphs are clipped ([#53](https://github.com/TeamGDB/Yakumo/issues/53)); tiled 2D screens show faint seams above ×1 ([#55](https://github.com/TeamGDB/Yakumo/issues/55)).

Tested on macOS (Apple Silicon, Vulkan through MoltenVK) and on a Steam Deck in Game Mode, built with GCC in a Debian 13 container and running on native Vulkan. On Windows the game builds with MSVC and starts, but closes at the first save ([#13](https://github.com/TeamGDB/Yakumo/issues/13)); see [the compatibility table](../../docs/COMPATIBILITY.md).

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
- Optional: FFmpeg's `libavcodec` and `libavutil`, found through `pkg-config`, to decode the streamed music and the movies

If SDL3, Vulkan or `glslangValidator` is missing, configuration still succeeds but builds the game **without a window**: CMake prints `mhp3rd: renderer disabled` and the program runs headless. Check for `mhp3rd: Vulkan renderer enabled` in the configure output.

The streamed music (ATRAC3) and the movies (H.264 with ATRAC3plus sound) are decoded by FFmpeg's shared libraries. Install them with `brew install ffmpeg` on macOS or `apt install libavcodec-dev libavutil-dev libswscale-dev` on Debian and Ubuntu. Configuration reports `mhp3rd: FFmpeg libavcodec … found; music and movies enabled`; without FFmpeg it prints that the music will be silent and the movies skipped, and builds the game without them. `-DMHP3RD_FFMPEG=OFF` leaves FFmpeg out on purpose.

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
cmake --build out/mhp3rd --target MHP3rdNative -j 2

# 3. Recompile the code overlays (about 40 minutes, resumable)
profiles/mhp3rd/scripts/build_overlays.sh

# 4. Play
out/mhp3rd/bin/MHP3rdNative
```

Each step is described below.

## Game data

The program needs two things from your own copy of the game: the disc image and the game's executable. It finds them in this order:

1. A directory given on the command line or in `MHP3RD_GAME_DIR`.
2. The **per-user data directory** that the installer fills.
3. `profiles/mhp3rd/game` in the checkout, set up by `prepare_game.sh`.

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

It holds `EBOOT.ELF`, `disc.iso` when the image was copied, and `settings.ini`, which records where the image is and keeps the settings of the [in-game menu](#in-game-menu). Save data is not there yet: `ms0` stays in `profiles/mhp3rd/game/ms0` for now. `MHP3RD_DATA_DIR` points the program at another directory.

The same setup runs without any screens from a terminal, for scripts and headless machines:

```bash
out/mhp3rd/bin/MHP3rdNative --install "/path/to/your.iso"             # copy the image
out/mhp3rd/bin/MHP3rdNative --install "/path/to/your.iso" --in-place  # use it where it is
out/mhp3rd/bin/MHP3rdNative --install                                 # run the setup screens again, then play
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
cmake --build out/mhp3rd --target MHP3rdNative -j 2
```

`generate.sh` analyzes the executable and writes the recompiled C++ into `generated/`. That corpus is derived from your copy of the game, so it stays local and is never committed.

Rerunning `generate.sh` rewrites only the units whose code changed, so the build after it recompiles only those.

The build protects its incremental state on macOS and Linux:

- **One build at a time.** With Ninja, CMake runs builds through a wrapper, `out/mhp3rd/ninja-locked`, that locks the build directory. A second `cmake --build` of the same directory, from a script or by hand, prints `another build is running` and waits. The lock belongs to the running Ninja, so interrupting `cmake --build` or a script around it does not let a new build start next to the Ninja that is still running. Running `ninja` directly bypasses the lock. `-DPSPRECOMP_BUILD_LOCK=OFF` turns it off. On Windows there is no lock yet: do not start two builds of one directory.
- **Damaged dependency log.** Before each build the wrapper checks `.ninja_deps` and repairs a damaged one with `ninja -t recompact`, which keeps every intact record. Do not delete `.ninja_deps` or `.ninja_log`: either costs a full rebuild.
- **Compiler cache.** If `ccache` is installed, every compile goes through it, so a rebuild of unchanged code takes seconds instead of minutes, also across checkouts at different paths. `-DPSPRECOMP_CCACHE=OFF` turns it off.

CMake prints a warning for Ninja 1.13.2, which cannot recover from a damaged dependency log by itself (upstream issue [#2703](https://github.com/ninja-build/ninja/issues/2703)). Building through `cmake --build` works around it; the fix is due in Ninja 1.14. [`docs/BUILD_SYSTEM.md`](../../docs/BUILD_SYSTEM.md) explains why incremental state gets lost and what the build does about it.

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
out/mhp3rd/bin/MHP3rdNative [game_dir]       # see "Game data" for where it looks without game_dir
```

The window renders at twice the PSP resolution by default (960×544). Esc, or L3+R3 on a gamepad, opens the [in-game menu](#in-game-menu); quit from there, or close the window (Cmd+Q on macOS, Alt+F4 on most Linux desktops). When the game asks for a name, the on-screen keyboard answers immediately with the name set in the menu (default `Hunter`); the menu can switch to typing it in the window instead (Enter confirms, Esc cancels).

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

## In-game menu

Esc, or L3+R3 on a gamepad, opens Yakumo's menu over the game; the same again, back at its top level or Start closes it. Esc never quits the game: Steam's desktop controller layout on a Steam Deck sends Esc with the B button, so an Esc that arrives together with a gamepad button is ignored.

While the menu is open the game is paused: no guest code runs, emulated time stands still, the audio device stops, and the last frame stays behind the menu, dimmed. Input goes to the menu only; buttons still held when it closes reach the game only after they are released. On resume the kernel's clock picks up from real time again, so the game neither races to make up the pause nor counts it in the `[perf]` statistics.

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
| Audio | Volume | `audio.volume` | | 0–100% |
| Audio | Mute | `audio.mute` | | |
| Controls | Confirm button | `input.confirm` | `MHP3RD_PAD_FACE` | Right (○, Japanese) or bottom (Western) |
| Controls | Stick dead zone | `input.dead_zone` | `MHP3RD_PAD_DEADZONE` | 0–50% |
| Controls | Trigger point | `input.trigger` | `MHP3RD_PAD_TRIGGER` | 5–100% |
| Controls | Right stick | `input.right_stick` | `MHP3RD_PAD_RSTICK_DPAD` | Camera, D-pad or off |
| Controls | Invert camera horizontally / vertically | `input.invert_camera_x`, `input.invert_camera_y` | | For the right-stick camera |
| Controls | Right stick D-pad point | `input.right_stick_zone` | `MHP3RD_PAD_RSTICK_ZONE` | 10–100%, for the D-pad mode |
| Controls | When the game asks for a name | `input.type_name` | `MHP3RD_OSK_INTERACTIVE` | Use the name below, or type it in the window |
| Controls | Hunter name | `input.name` | `MHP3RD_OSK_TEXT` | Default `Hunter` |

Everything applies without a restart; the name settings take effect the next time the game asks for a name. The Controls section also lists the keyboard's keys, and the System section has *Resume*, *Open the data folder*, *Set up game data again…* and *Quit game* (both of the last two ask first), with the build version, the data folder and the GPU. Each section has a button that restores its defaults.

The file also keeps `ui.menu_hint_seen`, set once the menu has been opened (until then a hint at the bottom of the screen says how to open it during the first seconds of play), and `ui.last_folder`, where the setup's file browser opens.

The interface is drawn with [Dear ImGui](third_party/imgui/README.md). Its text uses a system font: San Francisco or Helvetica on macOS, Noto Sans, DejaVu Sans or Liberation Sans on Linux, Segoe UI on Windows, with a Japanese font merged in for file names; `MHP3RD_UI_FONT` names another `.ttf`. It scales with the window: about 27-pixel text on a Steam Deck's 1280×800 screen.

## Saving and loading

The game saves through the PSP's save-data utility, which the host implements. Saves live where a PSP keeps them, under the directory that backs `ms0:` — `profiles/mhp3rd/game/ms0` unless `MHP3RD_GAME_DIR` or the `game_dir` argument points elsewhere:

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

1. On the PSP's memory stick, find `PSP/SAVEDATA/ULJM05800` — the folder of *Monster Hunter Portable 3rd*.
2. Quit the game, and copy the whole folder into `profiles/mhp3rd/game/ms0/PSP/SAVEDATA/`, replacing any folder of the same name. Keep a copy of the one you replace: it holds all three character slots.
3. Start the game. The title screen leads to character select with the imported characters.

To take a save back to a PSP, copy the same folder the other way. The downloaded-quest folder `ULJM05800QST` is copied the same way.

### Downloadable content

The game keeps downloaded quests and equipment in the `ULJM05800QST` save folder and reads it through the save-data utility, like an ordinary save (AUTOLOAD of `ULJM05800QST` / `MHP3RD.BIN`). The download servers are long gone, so the in-game download mode's network side stays unimplemented. To use DLC you already have, put your `ULJM05800QST` folder into `game/ms0/PSP/SAVEDATA/`, then open the game's download menu to install the quests. The project does not host, bundle or link to DLC files.

This release (`NPJB-40001`) asks for the original PSP release's folder names (`ULJM05800`), and the key it passes is the PSP release's: a downloaded-quest folder written by a PSP running `ULJM-05800` passes every check with it and decrypts. The two releases therefore share one save format, and saves should move between them in both directions; a save from this release has not yet been loaded on a PSP. When there is no save of its own, the game also looks for saves of *Monster Hunter Portable 2nd G* (`ULJM05500`) and *Monster Hunter Diary: Poka Poka Airu Village* (`ULJM05710`); those would be read from `ms0` the same way, which has not been tried.

To check a save folder without starting the game — for example one that the game reports as corrupted — run the save-data test program on it:

```bash
out/mhp3rd/bin/mhp3rd_savedata_tests --check profiles/mhp3rd/game/ms0/PSP/SAVEDATA/ULJM05800 MHP3RD.BIN <key>
```

`<key>` is the 32-digit key the game passes to the save-data utility; a run with `MHP3RD_TRACE_SAVEDATA=1` prints it as `key=` on every request for the game's own save. The program reports whether the hashes in `PARAM.SFO` and the data file's hash match and whether the file decrypts. Without arguments it runs the self-tests, which need no game data.

## Configuration

The settings a player needs are in the [in-game menu](#in-game-menu). Environment variables remain for everything else, and override the menu's settings for the run where they overlap.

### Game and paths

| Variable | Default | Effect |
| --- | --- | --- |
| `MHP3RD_GAME_DIR` | unset | Directory holding `EBOOT.ELF`, `disc.iso` and `ms0/` (the saves); skips the per-user directory |
| `MHP3RD_DATA_DIR` | SDL's preference path | Per-user data directory the installer fills |
| `MHP3RD_OVERLAY_DIR` | `overlays/` next to the executable | Directory of overlay libraries |
| `MHP3RD_FONT` | a system CJK font | TrueType font to rasterize game text from; macOS and common Linux CJK fonts are tried when unset |
| `MHP3RD_UI_FONT` | a system font | TrueType font for Yakumo's menu and setup screens |

### Video

| Variable | Default | Effect |
| --- | --- | --- |
| `MHP3RD_INTERNAL_SCALE` | `2` | Render resolution as a multiple of 480×272 (menu: Resolution) |
| `MHP3RD_NO_RENDER` | off | Run without a window; the installer shows no dialogs either. Emulated time is not held to real time |
| `MHP3RD_WINDOW_TITLE` | `MHP3rdNative` | Title of the game window, to tell instances apart |
| `MHP3RD_UNTHROTTLED` | off | Let emulated time run ahead of real time, so the game runs as fast as it can be drawn (menu: Game speed) |
| `MHP3RD_NO_MATERIAL_COLOR` | off | Leave unlit geometry without vertex colours white instead of taking the material colour |
| `MHP3RD_NO_LIGHTING` | off | Draw lit geometry with the flat white stand-in used before lighting existed, and without fog, to compare a scene with and without them |
| `MHP3RD_NO_FOG` | off | Turn fog off and keep lighting |
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
| `MHP3RD_PAD_RSTICK_DPAD` | off | Press D-pad bits from the right stick instead of feeding the HD release's second stick; enabling both would turn the camera twice (menu: Right stick) |
| `MHP3RD_PAD_RSTICK_ZONE` | `0.5` | Right-stick threshold for that (menu: Right stick D-pad point) |
| `MHP3RD_OSK_TEXT` | `Hunter` | Name the on-screen keyboard answers with (menu: Hunter name) |
| `MHP3RD_OSK_INTERACTIVE` | off | Type the name in the window instead (menu: When the game asks for a name) |
| `MHP3RD_AUTO_CONFIRM` | off | Press ○ every N frames, to walk through menus unattended |

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
| `MHP3RD_TRACE_MATERIAL=1` | Every distinct value the game writes to the GE material registers |
| `MHP3RD_TRACE_LIGHTING=1` | Every distinct value the game writes to the GE light and fog registers, and one line per distinct register state a lit draw is made with |
| `MHP3RD_NO_CULL=1`, `MHP3RD_NO_DEPTH=1` | Disable face culling or the depth test, to bisect missing geometry |
| `MHP3RD_TRACE_AUDIO=1` | One line per second of output: frames, peak, RMS, silence and drops |
| `MHP3RD_TRACE_ATRAC=1` | Every `sceAtrac3plus` call with its arguments, result and decode position |
| `MHP3RD_TRACE_MPEG=1` | Every `sceMpeg` and `sceJpegCsc` call, and each call the ring buffer makes to the game's read callback |
| `MHP3RD_SAS_NO_ENV=1` | Hold every SAS voice at full envelope, to separate an envelope bug from a decoding one |
| `MHP3RD_TRACE_PAD=1` | Log the pad state whenever it changes |
| `MHP3RD_INPUT_SCRIPT` | Scripted keys, virtual-gamepad buttons and axes, dropped files and window captures, for testing the menu and the setup without a person at the controls; the syntax is in `host/ui/input_script.hpp`. Example: `300:key Escape;330:shot menu;360:pad leftstick+rightstick` |
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
| `render` | CPU time turning display lists into Vulkan commands and recording the present |
| `wait` | Time blocked on the GPU: the frame fence, swapchain acquire, queue submit and present, and the queue idle waits of texture uploads. With FIFO presentation, pacing to the display shows up here, and so does the time the kernel waits to hold the game to real time |
| `lists` | Display lists enqueued per second of real time |
| last part | Present mode, swapchain size and the display's refresh rate as SDL reports it |
| `overlay` | CPU time spent drawing the overlay, when it is shown |

The overlay shows the same numbers and a graph of the last 192 frame times, from 0 to 50 ms, with guides at 16.7 and 33.3 ms: green up to 34 ms, yellow up to 50 ms, red beyond.

## Host layout

```text
host/main.cpp                    Entry point: finding the game data, executable check, startup
host/install/                    First-run installer: per-user directory, image checks, executable preparation
host/settings/                   Player settings: settings.ini, environment overrides, defaults
host/ui/                         Yakumo's own interface (Dear ImGui): in-game menu, setup screens, file browser
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
host/hle/hle_font.cpp            sceLibFont over a host TrueType font
host/hle/hle_utility.cpp         sceUtility on-screen keyboard and message dialog
host/hle/hle_savedata.cpp        sceUtility save-data dialog
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
scripts/      prepare_game.sh, generate.sh, build_overlays.sh, bootstrap_overlays.sh
tools/        ISO and DATA.BIN extraction, overlay wrapping, shader embedding
tests/        Save-data self-tests and save checker (mhp3rd_savedata_tests)
third_party/  stb_truetype, tiny-AES-c (installer and saves), Dear ImGui (menu and setup screens)
game/         Local game data: EBOOT.ELF, disc.iso, ms0/ (ignored)
analysis/     Analyzer output and extracted overlays (ignored)
generated/    Recompiled executable (ignored)
overlays/     One directory per recompiled overlay (ignored)
```
