# Compatibility

What works on each platform, as last checked by hand. Each row is a part of the game; each cell is its state on that platform, with the issue that tracks any problem.

| Status | Meaning |
| --- | --- |
| ✅ | Works |
| ⚠️ | Works, with a known problem |
| ❌ | Does not work yet |
| ❔ | Not checked on this platform |

## Game

| | macOS (Apple Silicon) | Linux | Steam Deck | Windows |
| --- | --- | --- | --- | --- |
| Build | ✅ | ❔ [#14] | ✅ in a Debian 13 container [#14] | ❔ [#13] |
| Boot, title and menus | ✅ | ❔ | ✅ | ❔ |
| Character creation | ✅ | ❔ | ✅ | ❔ |
| Village | ❔ re-check since the #7 fix | ❔ | ✅ | ❔ |
| Hunts | ✅ | ❔ | ✅ | ❔ |
| Graphics | ⚠️ no lighting or fog [#3] | ❔ | ⚠️ no lighting or fog [#3] | ❔ |
| Sound effects | ⚠️ run ahead of the picture [#4] | ❔ | ❔ | ❔ |
| Music | ✅ with FFmpeg | ❔ | ❔ | ❔ |
| Cutscene movies | ❌ skipped [#6] | ❌ [#6] | ❌ [#6] | ❌ [#6] |
| Saving and loading | ✅ PSP-format saves; no dialog screens yet [#33] | ❔ | ❔ | ❔ |
| Multiplayer | ❌ [#2] | ❌ [#2] | ❌ [#2] | ❌ [#2] |

Rows marked ❌ on every platform are missing features rather than platform problems.

## Input

| | macOS (Apple Silicon) | Linux | Steam Deck | Windows |
| --- | --- | --- | --- | --- |
| Keyboard | ✅ | ❔ | ✅ | ❔ |
| DualSense | ✅ | ❔ | — | ❔ |
| DualShock 4 | ❔ | ❔ | — | ❔ |
| Xbox controllers | ❔ | ❔ | — | ❔ |
| Built-in controls | — | — | ✅ Game Mode (added to Steam as a non-Steam game); ⚠️ Desktop Mode sends mouse and Esc from Steam's desktop layout | — |

## Tested hardware

| Platform | Machine | GPU and driver | Commit | Date |
| --- | --- | --- | --- | --- |
| macOS 27 | Apple M1, 8 GB | Apple M1, MoltenVK | `v0.1.0` | 2026-09-18 |
| macOS 27 | Apple M1, 8 GB | Apple M1, MoltenVK | `3480dc2` (saving and loading) | 2026-09-18 |
| SteamOS 3.8.16 | Steam Deck | AMD Custom GPU 0932, RADV (Mesa 26.0.0-devel) | `5e4b27c` | 2026-09-18 |

## Updating this page

Run through [the smoke test](TESTING.md) on the platform, then change the cells you checked in the same pull request as the fix, or in a pull request of their own. Add a row to *Tested hardware* with the commit you tested. A result without a commit cannot be compared with anything later, so it does not go in the table.

To report a result without editing the page, open a **Test report** issue.

[#1]: https://github.com/TeamGDB/Yakumo/issues/1
[#2]: https://github.com/TeamGDB/Yakumo/issues/2
[#3]: https://github.com/TeamGDB/Yakumo/issues/3
[#4]: https://github.com/TeamGDB/Yakumo/issues/4
[#5]: https://github.com/TeamGDB/Yakumo/issues/5
[#6]: https://github.com/TeamGDB/Yakumo/issues/6
[#7]: https://github.com/TeamGDB/Yakumo/issues/7
[#13]: https://github.com/TeamGDB/Yakumo/issues/13
[#14]: https://github.com/TeamGDB/Yakumo/issues/14
[#33]: https://github.com/TeamGDB/Yakumo/issues/33
