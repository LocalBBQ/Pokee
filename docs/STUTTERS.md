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
| Fights | Measured: the main thread draining the music channel at each change of music segment (see below). Also possible: a pipeline compiled inline the first time a state is drawn | `audio_output`, `pipeline_for` |
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

## Second measurement: the vertex cache

Same machine, GPU compatibility mode off. The pack is ruled out: `pack` is 1 to 3 ms on the first frame of an area. With the render split on each `[slow-frame]` line, that frame is almost all `vertices` (writing a draw's vertices, with the copies a cache miss makes) and `decode`:

| Area | Frame | vertices | decode | texture | rest |
| --- | --- | --- | --- | --- | --- |
| stage016 | 43 ms | 14.7 | 8.0 | 2.5 | ~2 |
| stage044 | 68 ms | 40.5 | 17.8 | 2.3 | ~2 |
| stage068 | 80 ms | 40.1 | 17.8 | 1.4 | ~2 |
| stage075, second visit | 48 ms | 25.5 | 12.1 | 0.3 | ~2 |
| stage070 | 139 ms | 70.1 | 33.5 | 2.1 | ~4 |

Two causes. The cache dropped every span not drawn in the last three frames once a second, so an area left and entered again, or a building the camera turned away from, was decoded again. And each miss allocated a new vector: a standalone benchmark of 55 MiB in 4000 spans on the same machine took 22 ms, 12 to 15 ms of it in allocating and touching fresh memory, which Windows takes back when it is freed. Reusing the vectors of dropped spans brought it to 8.8 ms.

Changed: spans stay cached until the cache or the hold is full, and then the least recently drawn give way; the hold is 384 MiB where `choose_vertex_hold` allows (it was written for this and never used); the vectors of dropped spans are kept, up to a quarter of the hold, for the next misses; a miss's built span moves into the cache instead of being copied. `MHFU_OLD_VERTEX_CACHE=1` restores the old behaviour. Not measured in the game yet. The first visit to an area still misses for every draw until the cache has filled once, and `decode` is unchanged.

## Third measurement: the cache in the game

Same machine, the same quest route with the new cache and with `MHFU_OLD_VERTEX_CACHE=1`. Returning to an area no longer stalls the renderer: with the old cache the first frame back in stage044 took 78.8 ms (40.2 `vertices`) and in stage067 64.0 and 46.3 ms; with the new one none of the three revisits made a `[slow-frame]`, and the frame's buffer took 1.2 to 1.5 MiB of vertices instead of 55 to 86. The cache held the four areas, about 257 MiB, without dropping a span.

First visits did not change: 55 to 73 ms, of which `vertices` 24 to 42 and `decode` 12 to 18 (stage044 73.4 ms, 35.5 `vertices`, 17.8 `decode`; stage068 68.4, 41.5, 18.1). A miss converted its vertices into memory of its own, which a new area had none of to reuse, copied them into the frame's buffer and then copied them into the hold.

Changed: a transformed draw without bone weights that the cache misses is converted once, straight into the hold, and drawn from there in that frame too. The span keeps no other copy; when the hold takes its range back, it leaves the cache. Skinned draws, expanded draws and a hold with no room keep the old path. `MHFU_NO_MISS_TO_HOLD=1` restores it.

## Fourth measurement: what the player sees

Same machine and route, with misses written into the hold. `vertices` on the first frame of a new area fell from 24 to 42 ms to 6 to 10, and the frame from 55 to 73 ms to 40 to 50 (stage044 73.4 to 50.2 ms, stage067 57.1 to 40.1, stage075 55.5 to 45.2); stage016 no longer made a `[slow-frame]`.

Every area entry, revisits too, also has a frame of 85 to 92 ms, about 58 `guest` and 31 `pacing`, as the stage overlay is installed, and sometimes a second one of 60 to 150 ms of `guest`. `MHFU_TRACE_LOAD` puts it in `sha1Thread` (40 to 60 ms, once 163) checking the 1.2 to 2.7 MiB just read, and `user_main`. Each of those frames has 16 draws, the black screen between areas, against 7000 to 8500 in a frame of an area, so the player does not see them. The hitch seen as an area appears is its first frame: 37 to 50 ms, most of it `decode` (8 to 19 ms).

With `MHFU_TRACE_STALLS_MS=36`, play also has one to three frames a second of 36 to 38 ms with about 14 ms of work and the rest `pacing`: the sleep that holds the game to real time overshooting by about 3 ms. Not looked into yet.

## Decode

A standalone benchmark of `decode_vertices` on this machine, about 2 million vertices in 4000 draws (one first visit), took 13 to 15 ms whatever the format, against 4 ms for a plain loop decoding the same bytes. The difference was the fast path building each vertex on the stack and copying it into the output, which MSVC compiled into a slow sequence; writing each field in place took the decoder to 4.5 to 8.7 ms by format, 1.7 ms of it the output vector's default fill. `MHFU_CHECK_DECODE` still finds the fast path and the general loop the same bit for bit. In the game, along the same route, `decode` on the first frame of an area halved (stage044 18.9 to 9.9 ms, stage067 13.2 to 6.9) and so did the frame's excess: stage044 50.3 to 41.2 ms, stage067 47.9 to 37.4, while stage016 and stage075 no longer made a `[slow-frame]` at 36 ms.

A few seconds into stage075 the same run had five frames of 40 to 52 ms with 5 ms of `guest`, about 10 of `render` and 16 to 25 ms of `fence`, against 2 to 5 ms of GPU time for the game's own frame: the GPU busy with something that figure leaves out, most likely the frames interpolation draws between the game's (about eight per game frame at 240 Hz, at 3840x2160). The town showed the same once before. Converting into the hold stays at about 5 ms, the speed memory takes 80 MB at; hashing the source for the cache is 2 to 4 ms.

## Frame interpolation on loading into a quest

The stutter seen loading into the first area of a quest, at 240 Hz, was frame interpolation, not a slow frame. For the first second or two, about half the presents showed the older game frame again instead of a blend (123 of 218 in the first second, 76 of 237 in the next, against about 30 in play) and the delay from a frame's moment to its present was 51 ms instead of 36. `MHFU_TRACE_INTERPOLATION=frames` showed why: the game flips 17 to 27 ms after a frame's moment in the town and 6 ms in a quest, and the delay allows for the ninth longest of the last 60 frames, so it kept the town's for about 52 quest frames. Presents that late fall before the older of the two frames they blend and show it as it is. A run that waited on a screen with early flips before the quest had no such second.

Changed: the code time the delay allows for is also no more than the longest of the last 15 frames, so it follows a scene with earlier flips within half a second; a later scene lengthens it after nine frames, as before. `MHFU_INTERPOLATION_SLOW_DELAY=1` restores the old estimate. Not measured in the game yet.

## Fights: the music channel drain

Measured on Linux (Radeon RX 6700S, Mesa RADV, 120 Hz, frame interpolation to 90), one quest fought several times with `MHFU_PERF=log`, `MHFU_TRACE_STALLS`, `MHFU_TRACE_RENDER` and `MHFU_TRACE_KERNEL=user_main`.

- **Not the CPU.** The game thread, which runs the guest and the renderer, took 38% of a core in a quiet spot and 46% in the fight: 11.1 and 14.0 ms of each 33.3 ms frame. Other threads stayed near zero. All the extra was render work: CPU skinning in `decode` (+1.1 ms), frame interpolation's `replay` (+0.9) and vertex hashing (+0.9). Guest time did not grow.
- **Not the starvation charge.** The `charged` field of the stall trace (emulated time charged for guest code without import calls) was 0 to 1 ms in the hitch frames and a steady 0.2 ms a frame otherwise.
- **The hitch.** A few times a fight, at each change of music segment, a frame of 57 to 77 ms with about 15 ms of work and the rest `pacing`: the game took three vblanks for a frame. `MHFU_TRACE_AUDIO_CALLS` and the `ra=` of the kernel trace put it in `user_main` draining the music channel (ch2): it sets the channel to 64 frames and outputs a short buffer, and `sceAudioOutputPannedBlocking` made it wait for the music still queued, 22 to 43 ms (up to two 1024-frame buffers).

Changed (`audio/channel_queue.cpp`): a shorter buffer behind longer ones waits only for the short ones before it; the music thread's next buffer waits for everything. The queue and what reaches the sink are unchanged, so the music change sounds the same (checked by ear). `MHFU_OLD_AUDIO_DRAIN=1` restores the old wait. In an A/B run each of five drains with the old wait missed a vblank and none with the new one.

The large-`guest` frames that remained were two things. Each `[slow-frame]` line now names the frame's slowest HLE call and gives the game thread's CPU time, page faults and context switches, and `MHFU_TRACE_STALL_STACKS` prints the stack of a call that runs past 45 ms.

- **Quest start, every run:** about 50 ms of `guest` with 8,203 page faults on the game thread. Key quests copied all 32 MiB of guest RAM into a new vector each time the game read a quest pack, before searching it. It now searches RAM in place (`key_quests.cpp`); the frame is gone. Turning Key quests on still searches RAM once per pack, about 90 ms in a flip at boot on the black screen.
- **Rare stalls of 45 to 100 ms with little CPU:** the game thread polled, waking every 2.1 ms, inside a call, outside the timed render and pacing. One run in four minutes of fighting had a single one, of 43 ms, under the stack threshold, so the code behind it is not named yet.

Other stacks the sampler caught were outside play: the in-game menu (paused, kept out of the statistics), GTK window decorations loading their icons through the Wayland event pump (94 ms, once), SDL scanning udev for gamepads (48 ms), save data decryption on loading a save, and an 88 ms disc read. One run with `MHFU_OLD_AUDIO_DRAIN=1` stayed on the black screen after the first logo overlay for 14 minutes; the same command booted normally the next time, and the setting leaves the old code path unchanged. Not on Windows or NVIDIA yet.

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
