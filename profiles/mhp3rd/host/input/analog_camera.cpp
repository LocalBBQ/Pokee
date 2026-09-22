#include "input/analog_camera.hpp"

#include "settings/settings.hpp"
#include "psprecomp/runtime.hpp"

#include <algorithm>
#include <bit>
#include <cmath>
#include <cstdlib>
#include <fstream>

namespace mhp3rd::input {
namespace {

// NPJB-40001: the ordinary camera calls the rotation helper at 0x088E6264.
// At that call s1 is the camera, a1 points to the rotation angles, and sp+0x30
// holds the eye offset. The game's subsequent transform, terrain and wall
// collision checks still run on the adjusted offset. These offsets come from
// our own executable, not from a camera mod or a search for correlated words.
constexpr std::uint32_t kRotationHelper = 0x08878B70u;
constexpr std::uint32_t kCameraReturn = 0x088E626Cu;
constexpr float kRadians = 3.14159265358979323846f / 180.0f;
constexpr float kAngleUnits = 65536.0f / 360.0f;
constexpr float kCameraHz = 30.0f;

struct Camera {
    std::uint64_t frame{};
    std::uint64_t last_update{};
    std::uint32_t address{};
    bool available{};
    float x{};
    float y{};
    float yaw_remainder{};
    bool pitch_owned{};
    float pitch{};
    unsigned updates{};
};
Camera camera;
bool installed{};
CameraRotationFunction original_rotation{};

bool enabled() {
    const auto &s = settings::current();
    return installed && s.analog_camera && s.right_stick == settings::RightStick::Camera;
}

float load_float(const psprecomp::GuestMemory &memory, std::uint32_t address) {
    return std::bit_cast<float>(memory.load32(address));
}

void store_float(psprecomp::GuestMemory &memory, std::uint32_t address, float value) {
    memory.store32(address, std::bit_cast<std::uint32_t>(value));
}

void adjust_camera(psprecomp::Runtime &runtime, psprecomp::AllegrexContext &ctx) {
    if (ctx.gpr[31] != kCameraReturn) return;
    auto &memory = runtime.memory();
    const auto address = ctx.gpr[17];
    const auto stack = ctx.gpr[29];
    if (!memory.raw_pointer(address, 0x188u) || !memory.raw_pointer(stack, 0x190u) ||
        !memory.raw_pointer(ctx.gpr[5], 12u)) return;

    // Mode zero is the ordinary follow camera seen in the quest trace. Leave
    // aiming and other special modes to the game until separately verified.
    if (memory.load8(address + 0x76u) != 0u) {
        camera.available = false;
        camera.pitch_owned = false;
        return;
    }
    if (camera.address != address || camera.frame - camera.last_update > 2u) {
        camera.pitch_owned = false;
        camera.yaw_remainder = 0.0f;
    }
    camera.address = address;
    camera.last_update = camera.frame;
    camera.available = true;
    ++camera.updates;

    const auto &player = settings::current();
    const float y = load_float(memory, stack + 0x34u);
    const float z = load_float(memory, stack + 0x38u);
    const auto preset = memory.load32(address + 0x70u);
    if (!memory.raw_pointer(preset, 0x20u)) return;
    const float target_height = load_float(memory, preset + 0x10u);
    const float radius = std::hypot(y - target_height, z);
    if (!std::isfinite(radius) || radius < 1.0f || radius > 10000.0f) return;
    const float eye_dx = load_float(memory, address) - load_float(memory, address + 0x10u);
    const float eye_dy = load_float(memory, address + 4u) - load_float(memory, address + 0x14u);
    const float eye_dz = load_float(memory, address + 8u) - load_float(memory, address + 0x18u);

    // A real D-pad command, recentre or target-camera snap cancels our pitch.
    // The pad's right stick is neutralised separately; these are still the
    // game's own controls, and the game has already selected the new preset.
    const auto buttons = memory.load16(address + 0x84u);
    const bool recentre = (buttons & 0x100u) != 0u || memory.load8(address + 0x8Eu) != 0u;
    const bool vertical_command = (buttons & 0x50u) != 0u;
    const bool active = enabled();
    if (!active || recentre || vertical_command)
        camera.pitch_owned = false;

    if (active && !recentre && camera.x != 0.0f) {
        camera.yaw_remainder += -camera.x * player.camera_speed * kAngleUnits / kCameraHz;
        const int step = static_cast<int>(camera.yaw_remainder);
        camera.yaw_remainder -= static_cast<float>(step);
        const auto yaw = static_cast<std::uint16_t>(memory.load16(address + 0x82u) + step);
        // Both fields are identified in the yaw filter at 0x088E61A4. Keeping
        // target and current together prevents a delayed turn after release.
        memory.store16(address + 0x80u, yaw);
        memory.store16(address + 0x82u, yaw);
        memory.store32(ctx.gpr[5] + 4u, static_cast<std::uint32_t>(static_cast<std::int16_t>(yaw)));
    } else {
        camera.yaw_remainder = 0.0f;
    }

    if (active && !recentre && !vertical_command) {
        float previous_pitch = camera.pitch;
        if (camera.y != 0.0f) {
            if (!camera.pitch_owned) {
                camera.pitch = std::atan2(y - target_height, std::fabs(z)) / kRadians;
                previous_pitch = camera.pitch;
                camera.pitch_owned = true;
            }
            camera.pitch = std::clamp(camera.pitch + camera.y * player.camera_speed / kCameraHz,
                                      -60.0f, 70.0f);
        }
        if (camera.pitch_owned) {
            // Rotate the eye about its look-at point, keeping the current
            // preset's distance. Only this update's stack values change; shared
            // preset tables and other cameras are never written.
            const float pitch = camera.pitch * kRadians;
            store_float(memory, stack + 0x34u, target_height + radius * std::sin(pitch));
            store_float(memory, stack + 0x38u, std::copysign(radius * std::cos(pitch), z));
            // 0x088E6B58 eases eye.y towards target.y by 1/8 each update.
            // Advance its current value by just the player's height change;
            // otherwise releasing the stick leaves many frames of catch-up.
            // Terrain movement and collision corrections still use the game's
            // filter, and all subsequent collision checks remain in place.
            if (camera.pitch != previous_pitch) {
                const float delta = radius * (std::sin(pitch) - std::sin(previous_pitch * kRadians));
                store_float(memory, address + 4u, load_float(memory, address + 4u) + delta);
            }
        }
    }

    static const char *trace_path = std::getenv("MHP3RD_TRACE_CAMERA_STATE");
    if (trace_path) {
        static std::ofstream trace(trace_path);
        trace << camera.frame << ',' << std::hex << address << std::dec << ',' << active << ','
              << camera.x << ',' << camera.y << ',' << memory.load16(address + 0x80u) << ','
              << memory.load16(address + 0x82u) << ',' << y << ',' << z << ',' << target_height << ','
              << camera.pitch_owned << ',' << camera.pitch << ',' << recentre << ',' << vertical_command
              << ',' << load_float(memory, stack + 0x34u) << ',' << load_float(memory, stack + 0x38u)
              << ',' << std::atan2(eye_dy, std::hypot(eye_dx, eye_dz)) / kRadians << '\n';
        if (camera.updates % 30u == 0u) trace.flush();
    }
}

void camera_rotation(psprecomp::Runtime &runtime, psprecomp::AllegrexContext &ctx) {
    adjust_camera(runtime, ctx);
    // Continue the original helper with the same CPU context and return PC.
    // Do not invoke an isolated guest call: normal scheduling must be retained.
    original_rotation(runtime, ctx);
}

} // namespace

void install_analog_camera(psprecomp::Runtime &runtime, CameraRotationFunction original) {
    if (!original || !runtime.has_function(kRotationHelper)) return;
    camera = Camera{};
    original_rotation = original;
    // A host registration disables this unit's direct-call shortcut, so the
    // generated cross-unit camera call reaches the wrapper without regeneration.
    runtime.register_function(kRotationHelper, &camera_rotation, "mhp3rd_camera_rotation");
    installed = true;
}

void analog_camera_frame(float x, float y) {
    ++camera.frame;
    camera.x = std::clamp(x, -1.0f, 1.0f);
    camera.y = std::clamp(y, -1.0f, 1.0f);
    if (camera.frame - camera.last_update > 1u) {
        camera.available = false;
        camera.pitch_owned = false;
    }
    if (!enabled()) {
        camera.pitch_owned = false;
        camera.yaw_remainder = 0.0f;
    }
}

bool analog_camera_driving() {
    return enabled() && camera.available;
}

} // namespace mhp3rd::input
