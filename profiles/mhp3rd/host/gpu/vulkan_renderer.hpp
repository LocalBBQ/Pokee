#pragma once

#include "ge_state.hpp"

#include <cstdint>
#include <memory>
#include <string>

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
    std::uint32_t internal_scale{2u};  // multiples of 480x272
    bool vsync{true};
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

    // Host text entry, used while the guest shows its on-screen keyboard.
    void begin_text_input(const std::string &initial);
    void end_text_input();
    [[nodiscard]] std::string text_input() const;
    [[nodiscard]] bool text_input_confirmed() const noexcept;
    [[nodiscard]] bool text_input_cancelled() const noexcept;
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

    // Writes the last rendered frame as a BMP; returns false if it could not be
    // read back. Used for screenshots without touching the window system.
    bool capture_frame(const std::string &path);

    [[nodiscard]] std::uint64_t frames_presented() const noexcept;
    [[nodiscard]] std::uint64_t draws_submitted() const noexcept;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace mhp3rd::gpu
