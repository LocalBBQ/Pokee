#include "camera/field_of_view.hpp"

#include "game/guest_layout.hpp"
#include "psprecomp/runtime.hpp"

#include <algorithm>
#include <array>
#include <bit>
#include <cstdlib>
#include <iomanip>
#include <iostream>

namespace mhfu::camera {
namespace {

constexpr std::uint32_t kUpdate = 0x08815384u;
constexpr std::uint32_t kSmoothUpdate = 0x08815964u;
// In the camera object (game::layout::camera_object).
constexpr std::uint32_t kFieldOfView = 0x0Cu;
constexpr std::uint32_t kBuiltFieldOfView = 0x10u;
constexpr std::uint32_t kTargetFieldOfView = 0xA8u;
// The setting's degrees that leave the game's view as it is.
constexpr std::uint32_t kGameDegrees = 50u;
// Wider than this the projection's cot(fov/2) heads for zero; a camera the
// setting would take past it keeps the game's view.
constexpr float kMaxRadians = 2.8f;

constexpr std::array<CodeWord, 10> kSignature{{
    {0x08812D98u, 0x0E2054E1u, "jal 0x08815384: the camera's frame calls its update from another unit"},
    {0x08815384u, 0x27BDFFB0u, "addiu sp,sp,-80: the update's prologue"},
    {0x08815390u, 0xC4810010u, "lwc1 f1,0x10(a0): the field of view the projection was built with"},
    {0x08815394u, 0xC480000Cu, "lwc1 f0,0xc(a0): the update builds again when the current one differs"},
    {0x08815354u, 0xC48C000Cu, "lwc1 f12,0xc(a0): the projection is built from +0xC"},
    {0x08815964u, 0x27BDFFB0u, "addiu sp,sp,-80: the smoothed update's prologue"},
    {0x08815978u, 0x3C024110u, "lui v0,0x4110: it moves a ninth of the way"},
    {0x08815A0Cu, 0xC48100A8u, "lwc1 f1,0xa8(a0): towards the target field of view"},
    {0x08815A10u, 0xC482000Cu, "lwc1 f2,0xc(a0): from the current one"},
    {0x08815A24u, 0xE480000Cu, "swc1 f0,0xc(a0): and keeps the result"},
}};

using Function = psprecomp::Runtime::RecompiledFunction;

struct State {
    bool prepared{};
    bool wrapped{};
    // The setting over the game's 50 degrees; exactly 1 passes through.
    float scale{1.0f};
    Function update{};
    Function smooth_update{};
    // MHFU_TRACE_FOV
    std::uint32_t fov{};
    std::uint32_t built{};
    std::uint32_t target{};
    std::uint64_t frame{};
};

State state;

float load_float(const psprecomp::GuestMemory &memory, std::uint32_t address) {
    return std::bit_cast<float>(memory.load32(address));
}

void store_float(psprecomp::GuestMemory &memory, std::uint32_t address, float value) {
    memory.store32(address, std::bit_cast<std::uint32_t>(value));
}

// The camera an update was called with, when it looks like one.
bool plausible(const psprecomp::GuestMemory &memory, std::uint32_t camera) {
    if (camera == 0u || !memory.contains(camera, kTargetFieldOfView + 4u)) return false;
    const float fov = load_float(memory, camera + kFieldOfView);
    return fov > 0.01f && fov < 3.1f;
}

// True when this update should see the setting's field of view.
bool scaling(const psprecomp::GuestMemory &memory, std::uint32_t camera) {
    if (state.scale == 1.0f || !plausible(memory, camera)) return false;
    const float widest =
        std::max(load_float(memory, camera + kFieldOfView), load_float(memory, camera + kTargetFieldOfView));
    return widest * state.scale < kMaxRadians;
}

// The update builds the projection and the culling planes from +0xC, and
// keeps what it built with at +0x10. That stays scaled, so the next frame
// builds again only when the game's value or the setting has changed.
void update(psprecomp::Runtime &runtime, psprecomp::AllegrexContext &ctx) {
    psprecomp::GuestMemory &memory = runtime.memory();
    const std::uint32_t camera = ctx.gpr[4];
    if (!scaling(memory, camera)) {
        state.update(runtime, ctx);
        return;
    }
    const std::uint32_t game = memory.load32(camera + kFieldOfView);
    const std::uint32_t wide = std::bit_cast<std::uint32_t>(std::bit_cast<float>(game) * state.scale);
    memory.store32(camera + kFieldOfView, wide);
    state.update(runtime, ctx);
    if (memory.load32(camera + kFieldOfView) == wide) memory.store32(camera + kFieldOfView, game);
}

// The smoothed update moves +0xC towards +0xA8 by a fixed share. Scaling
// both by the same factor scales the step too, so dividing the result gives
// the game the value it would have reached itself.
void smooth_update(psprecomp::Runtime &runtime, psprecomp::AllegrexContext &ctx) {
    psprecomp::GuestMemory &memory = runtime.memory();
    const std::uint32_t camera = ctx.gpr[4];
    if (!scaling(memory, camera)) {
        state.smooth_update(runtime, ctx);
        return;
    }
    const std::uint32_t game_target = memory.load32(camera + kTargetFieldOfView);
    const std::uint32_t wide_target = std::bit_cast<std::uint32_t>(std::bit_cast<float>(game_target) * state.scale);
    store_float(memory, camera + kFieldOfView, load_float(memory, camera + kFieldOfView) * state.scale);
    memory.store32(camera + kTargetFieldOfView, wide_target);
    state.smooth_update(runtime, ctx);
    if (memory.load32(camera + kTargetFieldOfView) != wide_target) return;
    memory.store32(camera + kTargetFieldOfView, game_target);
    const std::uint32_t reached = memory.load32(camera + kFieldOfView);
    // At the target, the game's own target bit for bit, so the game sees
    // its camera settle as it would.
    if (reached == wide_target)
        memory.store32(camera + kFieldOfView, game_target);
    else
        store_float(memory, camera + kFieldOfView, std::bit_cast<float>(reached) / state.scale);
}

void wrap(psprecomp::Runtime &runtime) {
    // Only ever at the game's flip, from an import: no generated frame is
    // live on the host stack, so the dispatch tables can change here.
    state.update = runtime.function_at(kUpdate);
    state.smooth_update = runtime.function_at(kSmoothUpdate);
    if (state.update == nullptr || state.smooth_update == nullptr) {
        std::cerr << "[fov] the camera's update is not in the generated code; the field of view stays the game's\n";
        state.prepared = false;
        return;
    }
    runtime.register_function(kUpdate, &update, "mhfu_fov_update");
    runtime.register_function(kSmoothUpdate, &smooth_update, "mhfu_fov_smooth_update");
    state.wrapped = true;
}

void trace_frame(const psprecomp::GuestMemory &memory) {
    static const bool on = std::getenv("MHFU_TRACE_FOV") != nullptr;
    if (!on) return;
    ++state.frame;
    const std::uint32_t camera = game::layout::camera_object(memory);
    if (!plausible(memory, camera)) return;
    const std::uint32_t fov = memory.load32(camera + kFieldOfView);
    const std::uint32_t built = memory.load32(camera + kBuiltFieldOfView);
    const std::uint32_t target = memory.load32(camera + kTargetFieldOfView);
    if (fov == state.fov && built == state.built && target == state.target) return;
    state.fov = fov;
    state.built = built;
    state.target = target;
    std::cout << "[fov] frame " << state.frame << " camera 0x" << std::hex << camera << std::dec << " fov "
              << load_float(memory, camera + kFieldOfView) << " built "
              << load_float(memory, camera + kBuiltFieldOfView) << " target "
              << load_float(memory, camera + kTargetFieldOfView) << " scale " << state.scale << std::endl;
}

} // namespace

std::span<const CodeWord> field_of_view_signature() {
    return kSignature;
}

bool prepare_field_of_view(psprecomp::Runtime &runtime) {
    state = State{};
    const psprecomp::GuestMemory &memory = runtime.memory();
    for (const CodeWord &expected : kSignature) {
        const std::uint32_t found = memory.load32(expected.address);
        if (found == expected.word) continue;
        std::cerr << "[fov] game code differs at 0x" << std::hex << std::uppercase << std::setfill('0') << std::setw(8)
                  << expected.address << ": 0x" << std::setw(8) << found << ", expected 0x" << std::setw(8)
                  << expected.word << std::dec << std::nouppercase << std::setfill(' ') << " (" << expected.what
                  << "); the field of view stays the game's\n";
        return false;
    }
    state.prepared = true;
    return true;
}

bool field_of_view_available() {
    return state.prepared;
}

void field_of_view_frame(psprecomp::Runtime &runtime, std::uint32_t degrees) {
    if (state.prepared) {
        // Only on a quest; the village and the Guild Hall keep the game's
        // own view.
        const bool quest = game::layout::quest_running(runtime.memory());
        const float scale = quest ? static_cast<float>(degrees) / static_cast<float>(kGameDegrees) : 1.0f;
        if (scale != state.scale)
            std::cout << "[fov] the game's view is " << (quest ? degrees : kGameDegrees) << " degrees high"
                      << (quest ? " on this quest" : " off a quest") << std::endl;
        state.scale = scale;
        if (state.scale != 1.0f && !state.wrapped) wrap(runtime);
    }
    trace_frame(runtime.memory());
}

} // namespace mhfu::camera
