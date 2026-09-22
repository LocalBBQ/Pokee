# Testing

## Automated

- **Framework tests** run with `ctest --test-dir out/framework` and need no game data.
- **Builds on every platform** in CI — planned in [#15](https://github.com/TeamGDB/Yakumo/issues/15).
- **Regression tests on your own copy of the game**, replaying recorded input and comparing frames against reference images — planned in [#16](https://github.com/TeamGDB/Yakumo/issues/16).

Until those exist, changes are checked by playing, with the smoke test below.

## Smoke test

About fifteen minutes. It walks through every part of the game that currently works, so a regression anywhere shows up. Start from a fresh profile — rename `profiles/mhp3rd/game/ms0` aside — so earlier state cannot hide a problem.

Before you start, write down the commit you are testing: `git rev-parse --short HEAD`. A result is only useful with it.

A released build is tested the same way. Note its version and which download it is (Flatpak or tarball) instead of the commit, start it through its launcher (`flatpak run io.github.teamgdb.Yakumo` or `./yakumo`, from a terminal to see the console), and start from a fresh data directory: for the Flatpak, move `~/.var/app/io.github.teamgdb.Yakumo` aside; for the tarball, `~/.local/share/Yakumo`. The first start then runs the setup from your disc image, which is part of the test. [`LINUX.md`](LINUX.md) says where a release keeps its saves.

| # | Step | Expected |
| --- | --- | --- |
| 1 | Start `out/mhp3rd/bin/Yakumo` | A window opens; the console lists 355 overlay corpora, the renderer and the audio device |
| 2 | Wait through the logos | Movies are skipped (see #6) and the title screen appears; streamed music is silent (see #5) |
| 3 | Start a new game | Character creation appears |
| 4 | In character creation, change each option | The character model is whole and textured, animates, and changes with each option |
| 5 | Enter a name, confirm, and save to a slot when asked | The console logs `[savedata] saved … ULJM05800 (encrypted)` and the game moves on to the hot spring scene |
| 6 | Watch the hot spring scene | Water, steam and the waterfall draw correctly; characters have soft shadows, not white patches |
| 7 | Talk through the scene and walk out | The village loads; the marker over an NPC's head is red, characters are shaded, and distant geometry fades into the fog |
| 8 | Walk around the village | Everything draws; it runs slower than elsewhere (see #7) |
| 9 | Take a quest and depart | The quest map loads with the HUD, the minimap and the character's weapon |
| 10 | Hunt a small monster | Monsters appear and animate; attacks, hits and sound effects work |
| 11 | Stand still with no input for ten seconds | The character and the camera stay still |
| 12 | Move the camera with the right stick, if you have a gamepad | The camera turns and stops when the stick is released |
| 13 | Return to the village | The village loads again |
| 14 | Close the window and start the game again | The console logs `[savedata] loaded … (decrypted)`; after the title screen, character select lists the character from step 5 |
| 15 | Pick that character | The game continues from the save |

### What to watch for throughout

- Any line reading `[interpreter] no recompiled function at …` — that code runs about twenty times slower. Note the address.
- Missing, torn or flickering geometry, and black or white patches where effects should be.
- The console's last lines if the game stops or crashes.

### Analog camera regression (#106)

Build `mhp3rd_camera_tests` and run it through CTest. These checks require no game data and cover dispatch interception, proportional rates, fractional yaw, pitch limits, release without filter catch-up, Off passthrough, special modes, scene changes and the extent of guest writes.

For the manual check, start without `MHP3RD_TRACE_CAMERA` or `MHP3RD_FIND_CAMERA`. Enable **Controls → Analog camera**, then enter an ordinary quest. Test small and full stick deflections on both axes, release, reversal, movement near walls, a zone transition, L recentre, physical D-pad commands, and Off/On toggles. With a bow and a bowgun, aim (R, and the bowgun scope) and move the right stick: the aim must move and the camera follow it as with the option off, and the analog camera must take over again after the aim. Confirm that the village cameras retain their own behaviour. Watch for residual vertical coast and camera movement after input has stopped. Repeat on Steam Deck before marking that platform verified.

## Reporting

Open a **Test report** issue with the platform, hardware, commit and the steps you reached. If a result changes a cell in [the compatibility table](COMPATIBILITY.md), update the table in a pull request as well.

Useful settings while testing — all described in [the profile README](../profiles/mhp3rd/README.md#configuration):

- `MHP3RD_SCREENSHOT_DIR` and `MHP3RD_SCREENSHOT_EVERY` capture frames to attach to a report.
- `MHP3RD_TRACE_PAD=1` shows whether input is reaching the game.
- `MHP3RD_TRACE_AUDIO=1` shows audio levels and dropped frames.
- `PSPRECOMP_HLE_HISTOGRAM=1` prints which system calls the game made.
