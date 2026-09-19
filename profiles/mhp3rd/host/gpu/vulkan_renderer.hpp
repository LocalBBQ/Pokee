#pragma once

#include "ge_state.hpp"

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "settings/settings.hpp"

union SDL_Event;
struct SDL_Window;
struct SDL_Gamepad;
struct ImDrawData;

namespace mhp3rd::gpu {

// PSP pad state gathered from the keyboard and the gamepad.
struct PadState {
    std::uint32_t buttons{};
    std::uint8_t analog_x{0x80u};
    std::uint8_t analog_y{0x80u};
    // The HD release reads a second stick from the two SceCtrlData bytes after
    // Ly, which the PSP itself left reserved. 0x80 is its centre: the guest
    // skips its camera path entirely only when both bytes are exactly centred.
    std::uint8_t right_x{0x80u};
    std::uint8_t right_y{0x80u};
};

struct RendererConfig {
    std::string title{"MHP3rdNative"};
};

// Vulkan backend for the GE. Draw calls are rendered into an offscreen target
// the size of the PSP framebuffer times the internal scale, which is blitted to
// the window once per guest frame.
class VulkanRenderer {
public:
    VulkanRenderer();
    ~VulkanRenderer();
    VulkanRenderer(const VulkanRenderer &) = delete;
    VulkanRenderer &operator=(const VulkanRenderer &) = delete;

    // Returns false and fills `error` when the window or device cannot be created.
    bool initialize(const RendererConfig &config, std::string &error);
    void shutdown();
    [[nodiscard]] bool available() const noexcept;

    // Pumps window events; returns false once the window has been closed.
    bool pump_events();
    [[nodiscard]] PadState pad() const noexcept;

    [[nodiscard]] bool quit_requested() const noexcept;

    void begin_frame();
    // Call before walking each display list. Guest memory cannot change while a
    // list is walked, so texture contents are hashed once per list, not per draw.
    void begin_display_list();
    void submit(const DrawCall &call, const GuestMemory &memory);
    // Ends the frame and shows the target the guest just flipped to. Draws go to
    // a separate offscreen target per guest framebuffer address, so only the
    // displayed one reaches the window.
    void present(std::uint32_t display_address);
    // Shows a frame the game wrote to memory itself instead of drawing it
    // with the GE, as the movie player does: the next present of
    // `display_address` shows these `width` x `height` pixels (R, G, B, A in
    // memory order, rows `stride` pixels apart), scaled to the target. Call
    // it at most once per presented frame.
    void upload_frame(std::uint32_t display_address, const std::uint8_t *pixels, std::uint32_t width,
                      std::uint32_t height, std::uint32_t stride);

    // Writes the last rendered frame as a BMP; returns false if it could not be
    // read back. Used for screenshots without touching the window system.
    bool capture_frame(const std::string &path);
    // Writes the next presented window image, with the interface over it, as
    // a BMP once it has been drawn.
    void capture_window(const std::string &path);

    // Display settings, applied at once. The initial values come from
    // settings::current() in initialize().
    void set_internal_scale(std::uint32_t scale);
    void set_window_scale(std::uint32_t scale);
    void set_fullscreen(bool fullscreen);
    void set_present_mode(settings::PresentMode mode);
    [[nodiscard]] bool supports_present_mode(settings::PresentMode mode) const;
    void set_keep_aspect(bool keep_aspect);
    void set_sharp_screen(bool sharp);
    void set_sharp_textures(bool sharp);
    void set_perf_overlay(bool visible);

    [[nodiscard]] SDL_Window *window() const noexcept;
    [[nodiscard]] std::string device_name() const;
    // The pad the game reads, or null.
    [[nodiscard]] SDL_Gamepad *gamepad() const noexcept;

    // The port's own interface (host/ui). Every window event is offered to
    // the hook first; returning true keeps it from the game.
    void set_event_hook(std::function<bool(const SDL_Event &)> hook);
    // While off, the game reads a neutral pad. Turning it back on ignores the
    // buttons still held until they are released, so the button that closed
    // a menu does not reach the game.
    void set_game_input(bool enabled);
    void request_quit() noexcept;
    // While held, the window keeps showing the frame on screen when hold
    // began instead of the frames the game flips to. The game blanks its
    // screen while the PSP's own keyboard would cover it; the port's keyboard
    // is drawn over the held frame instead.
    void hold_frame(bool hold);

    // Sets up Dear ImGui's Vulkan backend on this window; the caller has
    // created the ImGui context and its SDL3 backend.
    bool initialize_ui(std::string &error);
    void shutdown_ui();
    // ImGui_ImplVulkan_NewFrame, before ImGui::NewFrame.
    void begin_ui_frame();
    // Draw data from ImGui::Render, drawn over the next presented image.
    void set_ui_draw_data(ImDrawData *draw_data);
    // Presents a frame outside the game's own flips: the last game frame when
    // there is one and `show_game` is set, a plain background otherwise, with
    // the interface over it. Used while the game is paused or not started.
    void present_ui(bool show_game);

    [[nodiscard]] std::uint64_t frames_presented() const noexcept;
    [[nodiscard]] std::uint64_t draws_submitted() const noexcept;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace mhp3rd::gpu
