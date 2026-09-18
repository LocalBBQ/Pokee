Yakumo for Linux (x86-64)
=========================

A native port of Monster Hunter Portable 3rd HD Ver. It does not include the
game: you need your own disc image of it (NPJB-40001).

Start it with

    ./yakumo

The first start asks for your disc image, copies it into Yakumo's data
directory (~/.local/share/Yakumo/MHP3rd) and prepares the game from it.
After that the game starts straight away. Saves are kept in the same
directory, under ms0/PSP/SAVEDATA.

    ./yakumo --install /path/to/image.iso   set up from a terminal instead
    ./yakumo --help                         all options

Needs: an x86-64 Linux with glibc 2.31 or newer, a Vulkan driver (Mesa on
AMD and Intel, or NVIDIA's), and Wayland or X11. On a Steam Deck the
Flatpak is easier; see the guide below.

Guide for players, including the Steam Deck:
    https://github.com/TeamGDB/Yakumo/blob/main/docs/LINUX.md

Third-party software and its licenses: licenses/THIRD_PARTY_NOTICES.md
