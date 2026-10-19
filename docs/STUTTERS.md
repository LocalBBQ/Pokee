# Stutters on area entry and in fights

Status of the work on branch `perf/hitches` (draft pull request #10). Players on fast hardware (an RTX 4090 with an i9-13900K, Windows with the NVIDIA driver) report that the game stutters when entering a new area and sometimes during a fight, as if it were loading. This page records what is known, what has changed and what waits on measurements, so the work can be continued on another machine.

## Why a renderer spike stops the game

The renderer runs on the emulation thread. CPU work in it, such as creating a pipeline or allocating GPU memory, also holds the game's clock, and a frame more than 100 ms late makes the kernel drop the debt instead of catching up (`kernel.cpp`). A hitch is therefore a pause of the game itself, not only of the picture.

## Suspected causes

None of these has been measured on the affected hardware yet. Windows with an NVIDIA GPU has not been tested at all ([COMPATIBILITY.md](COMPATIBILITY.md) lists AMD GPUs only).

| Moment | Suspect | Where |
| --- | --- | --- |
| Area entry | One `vkAllocateMemory` (and one `vkFreeMemory` on eviction) per game texture; slow on Windows | `create_image`, `destroy_texture` in `vulkan_renderer.cpp` |
| Area entry, with an HD pack | Per pack image: two allocations, a command buffer, a fence and its own `vkQueueSubmit`; up to 32 MB a frame copied on the game thread | `ReplacementTextures::upload` |
| Fights | A pipeline compiled inline the first time a state is drawn; the background prewarm only covers earlier runs | `pipeline_for` |
| A few seconds after new pipelines | `pipeline_cache.bin` serialized and written on the game thread | `report_pipelines` |
| Quiet moments in a quest | Fast loading taking silence plus a disc read for a load | `load_detector.cpp` |
| First use of a code path | Overlay code paged in from disk (265 MB of libraries) | `overlays.cpp` |

## Done on the branch

- **Stall trace.** `MHFU_TRACE_STALLS` reports three more kinds in `[slow-frame]` and `[stalls]`: `texture` (creating or destroying a texture), `alloc` (the memory allocations and frees in it, and for upload buffers) and `cache-save` (writing the pipeline cache). A `[load] fast …` line says `, in a quest` when a quest was running as the stretch began.
- **Texture pack resources.** Loader threads are a quarter of the hardware threads, one to six (`MHFU_TEXTURE_PACK_THREADS`), unchanged up to 11 hardware threads. The GPU budget is a quarter of a discrete GPU's memory, 1 to 8 GiB; integrated GPUs keep 1 GiB (`MHFU_TEXTURE_PACK_MEMORY` still overrides).
- **Test fix.** The renderer/UI test counts the gather mark on the background draw list, where it is drawn, and no longer checks its thickness.
- **Texture pack timing.** A `pack` kind covers the pack's work on the game thread: the hash that looks up each new game texture, each image's upload, and freeing finished uploads or images over the budget.

## First measurement (RTX 4090, Windows, NVIDIA 616.92)

One run with the pack on, release/0.3 plus this branch, GPU compatibility mode on from the Video setting, 3840x2160 at 240 Hz, frame interpolation to 120. The pack-off run ended before a quest, so it shows nothing about areas. The player saw a stutter on every area entry and when fog rolled into the swamp.

- **Every area entry in a quest** (stage044, 067, 068) gives the same three frames. First, about 60 ms of `guest` once the stage overlay is installed, during the load. Then one or two frames of 30 to 56 ms `pacing`. Last, the first frame of the new area: 48 to 67 ms of `render`, of which `texture` and `alloc` are only about 2.5 ms (30 to 35 textures). Most of that render time is not in any kind; the pack's lookups and uploads were not timed, hence `pack`.
- **First quest load:** 4 pipelines in 75 ms inline (`pipeline 75.24 x4`). GPU compatibility mode turns off the pipeline cache and background pipelines, so every new pipeline is compiled on the game thread in every run.
- **Fog in the swamp (likely frame 3406):** 56 ms, nearly all `pacing` (42 ms), with the game at 29.5 frames that second. The game itself missed a vblank; the renderer did not stall. A disc read the game waits for (emulated read time) is the first suspect; `MHFU_TRACE_IO` shows it.
- Between hitches play is steady: 120 fps, the game at 30, frames at most about 11 ms.

Next run: pack on and off along the same route into a quest, with `MHFU_TRACE_IO=1` added, and once with GPU compatibility mode off.

## Next step: measure on the affected hardware

Build the branch with MSVC ([BUILDING.md](BUILDING.md#windows)). Only host code changed and `include/psprecomp/` matches `main`, so an existing build directory rebuilds `Pokee.exe` only. Back up the saves in `%APPDATA%\Pokee\MHFU\ms0` first.

Run the same route twice: start a quest, walk through three or four areas, fight for a minute or two, then quit from the menu so the log is complete.

```bash
MHFU_PERF=log MHFU_TRACE_STALLS=1 MHFU_TEXTURE_PACK=1 ./out/mhfu/bin/Pokee.exe > pack-on.log 2>&1
MHFU_PERF=log MHFU_TRACE_STALLS=1 MHFU_TEXTURE_PACK=0 ./out/mhfu/bin/Pokee.exe > pack-off.log 2>&1
```

Deleting `pipeline_cache.bin` and `pipeline_keys.bin` from the data folder before the first run reproduces a new player's first run, compiles included. Note the Frame rate setting, the display's refresh rate, and roughly when each hitch happened.

How to read the logs:

| What the slow frames show | Meaning | Next change |
| --- | --- | --- |
| Large `texture` and `alloc` on area entry, smaller with the pack off | Per-texture allocations, the pack's included | Pool the game's texture memory by size and allocate from large blocks; for the pack, block allocation, uploads in the frame's upload batch and loaders decoding straight into staging memory |
| `pipeline`, or `[render] N new pipelines` near fight hitches, gone in a second run | Pipeline compiles | Extended dynamic state for depth, cull, blend and colour mask, so the remaining pipelines are built at start; save the cache off the game thread |
| `cache-save` in slow frames | The cache write | Save it on a background thread |
| `[load] fast …, in a quest` without an area change | Fast loading misfiring | Keep it off during play in a quest |
| Large `guest` with no stall kind | Not covered yet: page faults in overlay code, the interpreter (`[interpreter]` lines) | Measure further before changing code |

The texture pack upload rework is the largest and riskiest of these changes; it is only worth doing if the pack shows up in the slow frames.

Windowed checks need someone at the screen: hand over a checklist with the variables and what to look for rather than launching game windows in a loop.
