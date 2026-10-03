# Dear ImGui

The files here are copies from [Dear ImGui](https://github.com/ocornut/imgui) release `v1.92.9b`, commit `f1cc2ae15e53a861a874c3034aae6798fde194ab`, under the MIT License (see `LICENSE.txt`).

Only what the port uses is kept: the library itself (`imgui*.cpp`, `imgui*.h`, `imconfig.h` and the `imstb_*.h` headers it includes) and two backends from `backends/`, `imgui_impl_sdl3` for window events and gamepads and `imgui_impl_vulkan` for drawing. The demo, the examples and the other backends are left out.

The port's own interface under `host/ui/` is built on it: the in-game menu and the first-run setup screens.

Local patch: `imgui.cpp` formats debug texture IDs with a matching 64-bit hexadecimal format and argument type, preserving IDs above `UINT32_MAX`. The upstream copyright and license notices are unchanged.

Local patch: `imstb_truetype.h` checks bitmap dimensions, row strides, scaled coordinates and SDF padding before signed arithmetic or allocation. It handles failed SDF allocations, clears packed atlases by their actual row stride, and rejects baked glyphs wider than the atlas. These independently written guards preserve Dear ImGui's existing modifications and the upstream license notices.
