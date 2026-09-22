#include "camera/game_camera.hpp"

#include "camera/camera_input.hpp"
#include "settings/settings.hpp"
#include "psprecomp/runtime.hpp"

#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>

namespace mhp3rd::camera {
namespace {

// NPJB-40001: the ordinary camera update calls the rotation helper at
// 0x088E6264. At that call s1 is the camera, a1 points to the rotation angles,
// and sp+0x30 holds the eye offset from the look-at point. The game's
// subsequent transform, terrain and wall collision checks still run on the
// adjusted offset. These facts come from this project's own reading of the
// executable; each one the driver relies on is listed in kSignature.
constexpr std::uint32_t kRotationHelper = 0x08878B70u;
constexpr std::uint32_t kCameraReturn = 0x088E626Cu;
constexpr float kRadians = 3.14159265358979323846f / 180.0f;
constexpr float kAngleUnits = 65536.0f / 360.0f;

// Camera structure, relative to s1.
constexpr std::uint32_t kEyeY = 0x04u;
constexpr std::uint32_t kPreset = 0x70u;
constexpr std::uint32_t kMode = 0x76u;
constexpr std::uint32_t kYawTarget = 0x80u;
constexpr std::uint32_t kYawCurrent = 0x82u;
constexpr std::uint32_t kButtons = 0x84u;
constexpr std::uint32_t kSnap = 0x8Eu;
// Preset, relative to its pointer, and the update's stack frame.
constexpr std::uint32_t kPresetTargetHeight = 0x10u;
constexpr std::uint32_t kStackEyeY = 0x34u;
constexpr std::uint32_t kStackEyeZ = 0x38u;

// The one camera mode with a driver: the ordinary follow camera in a quest.
// Others (aiming with a bow or a bowgun among them) keep the stock camera and
// the stock stick until each is traced and given its own driver below.
constexpr std::uint8_t kFollowMode = 0u;

// Game code the driver depends on, read from NPJB-40001. If any word differs,
// the executable is not the one these facts were read from, and the driver
// stays out rather than write to places that may mean something else.
constexpr std::array<CodeWord, 15> kSignature{{
    {0x088E6264u, 0x0E21E2DCu, "jal 0x08878B70: the camera update calls the rotation helper"},
    {0x088E6254u, 0x27A50050u, "addiu a1,sp,0x50: the helper's angles are on the update's stack"},
    {0x08878B70u, 0x27BDFFE0u, "addiu sp,sp,-0x20: the rotation helper's first instruction"},
    {0x088E6214u, 0x92230076u, "lbu v1,0x76(s1): the camera mode"},
    {0x088E68F0u, 0x8E220070u, "lw v0,0x70(s1): the camera's preset"},
    {0x088E68F8u, 0xE7A00034u, "swc1 f0,0x34(sp): the eye offset's height, from the preset"},
    {0x088E6904u, 0xE7A20038u, "swc1 f2,0x38(sp): the eye offset's distance, from the preset"},
    {0x088E6908u, 0xC4410010u, "lwc1 f1,0x10(v0): the preset's look-at height"},
    {0x088E6910u, 0x0A23988Fu, "j 0x088E623C: modes 0-2 join the path that calls the helper"},
    {0x088E61A0u, 0xA6220080u, "sh v0,0x80(s1): the target yaw"},
    {0x088E61A4u, 0x86220082u, "lh v0,0x82(s1): the filtered yaw"},
    {0x088E61ACu, 0x9224008Eu, "lbu a0,0x8e(s1): the recentre snap"},
    {0x088E77D4u, 0x96260084u, "lhu a2,0x84(s1): the camera's buttons"},
    {0x088E6B58u, 0xC6220004u, "lwc1 f2,0x4(s1): the eye height the 1/8 filter eases"},
    {0x0896D47Cu, 0x3E000000u, "0.125: that filter's coefficient"},
}};

struct State {
    bool prepared{};
    bool hooked{};
    RotationFunction original{};
    std::uint64_t frame{};
    std::uint64_t last_update{};
    std::uint32_t address{};
    bool available{};
    float yaw_remainder{};
    bool pitch_owned{};
    float pitch{};
    unsigned updates{};
};
State state;

bool option_on() {
    const auto &s = settings::current();
    return s.analog_camera && s.right_stick == settings::RightStick::Camera;
}

bool driving_allowed() { return state.hooked && option_on(); }

float load_float(const psprecomp::GuestMemory &memory, std::uint32_t address) {
    return std::bit_cast<float>(memory.load32(address));
}

void store_float(psprecomp::GuestMemory &memory, std::uint32_t address, float value) {
    memory.store32(address, std::bit_cast<std::uint32_t>(value));
}

void release() {
    state.available = false;
    state.pitch_owned = false;
    state.yaw_remainder = 0.0f;
}

void trace(const psprecomp::GuestMemory &memory, std::uint32_t address, std::uint32_t stack, const Turn &turn,
           bool active, bool recentre, bool vertical_command, float y, float z, float target_height) {
    static const char *path = std::getenv("MHP3RD_TRACE_CAMERA_STATE");
    if (path == nullptr) return;
    static std::ofstream out(path);
    const float eye_dx = load_float(memory, address) - load_float(memory, address + 0x10u);
    const float eye_dy = load_float(memory, address + 4u) - load_float(memory, address + 0x14u);
    const float eye_dz = load_float(memory, address + 8u) - load_float(memory, address + 0x18u);
    out << state.frame << ',' << std::hex << address << std::dec << ',' << active << ',' << turn.yaw_degrees << ','
        << turn.pitch_degrees << ',' << memory.load16(address + kYawTarget) << ','
        << memory.load16(address + kYawCurrent) << ',' << y << ',' << z << ',' << target_height << ','
        << state.pitch_owned << ',' << state.pitch << ',' << recentre << ',' << vertical_command << ','
        << load_float(memory, stack + kStackEyeY) << ',' << load_float(memory, stack + kStackEyeZ) << ','
        << std::atan2(eye_dy, std::hypot(eye_dx, eye_dz)) / kRadians << '\n';
    if (state.updates % 30u == 0u) out.flush();
}

// The ordinary follow camera: yaw on the game's own angle, pitch by orbiting
// the eye around its look-at point at the preset's distance.
void drive_follow(psprecomp::GuestMemory &memory, psprecomp::AllegrexContext &ctx, std::uint32_t address,
                  std::uint32_t stack) {
    if (state.address != address || state.frame - state.last_update > 2u) {
        state.pitch_owned = false;
        state.yaw_remainder = 0.0f;
    }
    state.address = address;
    state.last_update = state.frame;
    state.available = true;
    ++state.updates;

    const bool active = driving_allowed();
    // Taken even when inactive, so nothing built up while the option was off
    // is spent the moment it comes back on.
    const Turn turn = take();
    const float y = load_float(memory, stack + kStackEyeY);
    const float z = load_float(memory, stack + kStackEyeZ);
    const auto preset = memory.load32(address + kPreset);
    if (!memory.raw_pointer(preset, 0x20u)) return;
    const float target_height = load_float(memory, preset + kPresetTargetHeight);
    const float radius = std::hypot(y - target_height, z);
    if (!std::isfinite(radius) || radius < 1.0f || radius > 10000.0f) return;

    // A real D-pad command, recentre or target-camera snap cancels our pitch.
    // The game has already selected the new preset; these stay its controls.
    const auto buttons = memory.load16(address + kButtons);
    const bool recentre = (buttons & 0x100u) != 0u || memory.load8(address + kSnap) != 0u;
    const bool vertical_command = (buttons & 0x50u) != 0u;
    if (!active || recentre || vertical_command) state.pitch_owned = false;

    if (active && !recentre && turn.yaw_held) {
        state.yaw_remainder -= turn.yaw_degrees * kAngleUnits;
        const int step = static_cast<int>(state.yaw_remainder);
        state.yaw_remainder -= static_cast<float>(step);
        const auto yaw = static_cast<std::uint16_t>(memory.load16(address + kYawCurrent) + step);
        // Keeping target and filtered yaw together prevents a delayed turn
        // after release: the filter at 0x088E61A4 has nothing left to close.
        memory.store16(address + kYawTarget, yaw);
        memory.store16(address + kYawCurrent, yaw);
        memory.store32(ctx.gpr[5] + 4u, static_cast<std::uint32_t>(static_cast<std::int16_t>(yaw)));
    } else {
        state.yaw_remainder = 0.0f;
    }

    if (active && !recentre && !vertical_command) {
        float previous_pitch = state.pitch;
        if (turn.pitch_held) {
            if (!state.pitch_owned) {
                state.pitch = std::atan2(y - target_height, std::fabs(z)) / kRadians;
                previous_pitch = state.pitch;
                state.pitch_owned = true;
            }
            state.pitch = std::clamp(state.pitch + turn.pitch_degrees, -60.0f, 70.0f);
        }
        if (state.pitch_owned) {
            // Rotate the eye about its look-at point, keeping the current
            // preset's distance. Only this update's stack values change; shared
            // preset tables and other cameras are never written.
            const float pitch = state.pitch * kRadians;
            store_float(memory, stack + kStackEyeY, target_height + radius * std::sin(pitch));
            store_float(memory, stack + kStackEyeZ, std::copysign(radius * std::cos(pitch), z));
            // 0x088E6B58 eases eye.y towards target.y by 1/8 each update.
            // Advance its current value by just the player's height change;
            // otherwise releasing the stick leaves many frames of catch-up.
            // Terrain movement and collision corrections still use the game's
            // filter, and all subsequent collision checks remain in place.
            if (state.pitch != previous_pitch) {
                const float delta = radius * (std::sin(pitch) - std::sin(previous_pitch * kRadians));
                store_float(memory, address + kEyeY, load_float(memory, address + kEyeY) + delta);
            }
        }
    }
    trace(memory, address, stack, turn, active, recentre, vertical_command, y, z, target_height);
}

void adjust_camera(psprecomp::Runtime &runtime, psprecomp::AllegrexContext &ctx) {
    if (ctx.gpr[31] != kCameraReturn) return;
    auto &memory = runtime.memory();
    const auto address = ctx.gpr[17];
    const auto stack = ctx.gpr[29];
    if (!memory.raw_pointer(address, 0x188u) || !memory.raw_pointer(stack, 0x190u) ||
        !memory.raw_pointer(ctx.gpr[5], 12u))
        return;
    switch (memory.load8(address + kMode)) {
    case kFollowMode:
        drive_follow(memory, ctx, address, stack);
        return;
    // A driver for the aiming camera goes here, once traced.
    default:
        release();
        return;
    }
}

void camera_rotation(psprecomp::Runtime &runtime, psprecomp::AllegrexContext &ctx) {
    adjust_camera(runtime, ctx);
    // Continue the original helper with the same CPU context and return PC.
    // Do not invoke an isolated guest call: normal scheduling must be retained.
    state.original(runtime, ctx);
}

} // namespace

std::span<const CodeWord> game_code_signature() { return kSignature; }

bool prepare_game_camera(psprecomp::Runtime &runtime, RotationFunction original) {
    state = State{};
    reset();
    if (!original || !runtime.has_function(kRotationHelper)) {
        std::cerr << "[camera] the rotation helper is not in the generated code; analog camera unavailable\n";
        return false;
    }
    for (const CodeWord &expected : kSignature) {
        const std::uint32_t found = runtime.memory().load32(expected.address);
        if (found == expected.word) continue;
        std::cerr << "[camera] game code differs at 0x" << std::hex << std::uppercase << std::setfill('0')
                  << std::setw(8) << expected.address << ": 0x" << std::setw(8) << found << ", expected 0x"
                  << std::setw(8) << expected.word << std::dec << std::nouppercase << std::setfill(' ') << " ("
                  << expected.what << "); analog camera unavailable\n";
        return false;
    }
    state.original = original;
    state.prepared = true;
    return true;
}

void game_camera_frame(psprecomp::Runtime &runtime) {
    ++state.frame;
    if (state.prepared && !state.hooked && option_on()) {
        // Only ever at the game's flip, from an import: no generated frame is
        // live on the host stack, so the dispatch tables can change here.
        // A host registration disables this unit's direct-call shortcut, so the
        // generated cross-unit camera call reaches the wrapper without
        // regeneration. It stays until exit; with the option off again the
        // wrapper only passes through.
        runtime.register_function(kRotationHelper, &camera_rotation, "mhp3rd_camera_rotation");
        state.hooked = true;
    }
    if (state.frame - state.last_update > 1u) release();
    if (!driving_allowed()) {
        state.pitch_owned = false;
        state.yaw_remainder = 0.0f;
    }
    // Nothing takes the input while the camera update is not running.
    if (!game_camera_driving()) discard();
}

bool game_camera_driving() { return driving_allowed() && state.available; }

} // namespace mhp3rd::camera
