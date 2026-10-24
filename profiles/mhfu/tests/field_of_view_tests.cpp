// No game code or game data is needed: the words the field of view checks are
// written into guest memory as stand-ins, and the camera's two updates are
// host functions that do what the game's do to the fields involved.
#include "camera/field_of_view.hpp"
#include "psprecomp/runtime.hpp"

#include <bit>
#include <cmath>
#include <cstring>
#include <iostream>

namespace {
using namespace mhfu::camera;

constexpr std::uint32_t kUpdate = 0x08815384u;
constexpr std::uint32_t kSmoothUpdate = 0x08815964u;
constexpr std::uint32_t kCamera = 0x08900000u;
constexpr std::uint32_t kTaskSlot = 0x09A5F300u;
constexpr std::uint32_t kFov = kCamera + 0x0Cu;
constexpr std::uint32_t kBuilt = kCamera + 0x10u;
constexpr std::uint32_t kTarget = kCamera + 0xA8u;
constexpr float kQuest = 0.8722222f;
constexpr float kVillage = 1.01118f;
int failures{};

void check(bool condition, const char *message) {
    if (!condition) {
        ++failures;
        std::cerr << "FAIL: " << message << '\n';
    }
}

float read_float(const psprecomp::GuestMemory &memory, std::uint32_t address) {
    return std::bit_cast<float>(memory.load32(address));
}
void write_float(psprecomp::GuestMemory &memory, std::uint32_t address, float value) {
    memory.store32(address, std::bit_cast<std::uint32_t>(value));
}

// What the game's updates saw at +0xC and +0xA8.
float seen_fov{};
float seen_target{};

// 0x08815384: builds the projection from +0xC when +0x10 differs.
void game_update(psprecomp::Runtime &runtime, psprecomp::AllegrexContext &ctx) {
    auto &memory = runtime.memory();
    seen_fov = read_float(memory, ctx.gpr[4] + 0x0Cu);
    if (memory.load32(ctx.gpr[4] + 0x10u) != memory.load32(ctx.gpr[4] + 0x0Cu))
        memory.store32(ctx.gpr[4] + 0x10u, memory.load32(ctx.gpr[4] + 0x0Cu));
    ctx.pc = ctx.gpr[31];
}

// 0x08815964: moves +0xC a ninth of the way to +0xA8, then builds.
void game_smooth_update(psprecomp::Runtime &runtime, psprecomp::AllegrexContext &ctx) {
    auto &memory = runtime.memory();
    const float fov = read_float(memory, ctx.gpr[4] + 0x0Cu);
    const float target = read_float(memory, ctx.gpr[4] + 0xA8u);
    seen_target = target;
    write_float(memory, ctx.gpr[4] + 0x0Cu, fov + (target - fov) / 9.0f);
    seen_fov = read_float(memory, ctx.gpr[4] + 0x0Cu);
    memory.store32(ctx.gpr[4] + 0x10u, memory.load32(ctx.gpr[4] + 0x0Cu));
    ctx.pc = ctx.gpr[31];
}

struct Fixture {
    psprecomp::Runtime runtime;
    explicit Fixture(bool matching = true) {
        auto &memory = runtime.memory();
        for (const CodeWord &word : field_of_view_signature()) memory.store32(word.address, word.word);
        if (!matching) memory.store32(field_of_view_signature().front().address, 0u);
        runtime.register_function(kUpdate, &game_update, "game_update");
        runtime.register_function(kSmoothUpdate, &game_smooth_update, "game_smooth_update");
        write_float(memory, kCamera + 0x0u, 30.0f);
        write_float(memory, kCamera + 0x4u, 65000.0f);
        write_float(memory, kFov, kQuest);
        write_float(memory, kBuilt, kQuest);
        write_float(memory, kTarget, kQuest);
        quest(true);
    }
    // game_task.ovl in the task slot, or lobby_task.ovl (the village).
    void quest(bool on) {
        auto &memory = runtime.memory();
        memory.store32(kTaskSlot, 0x336F574Du); // "MWo3"
        memory.store32(kTaskSlot + 8u, kTaskSlot);
        const char *name = on ? "game_task.ovl" : "lobby_task.ovl";
        for (std::uint32_t i = 0; i <= 14u; ++i)
            memory.store8(kTaskSlot + 32u + i, static_cast<std::uint8_t>(i < std::strlen(name) ? name[i] : 0));
    }
    void call(std::uint32_t address) {
        psprecomp::AllegrexContext ctx{};
        ctx.gpr[4] = kCamera;
        ctx.gpr[31] = 0x08812DA0u;
        runtime.function_at(address)(runtime, ctx);
        check(ctx.pc == 0x08812DA0u, "the update returns to its caller");
    }
    [[nodiscard]] psprecomp::GuestMemory &memory() { return runtime.memory(); }
};

void test_game_view_writes_nothing() {
    Fixture f;
    check(prepare_field_of_view(f.runtime), "the matching game code is accepted");
    check(field_of_view_available(), "the setting is offered");
    field_of_view_frame(f.runtime, 50u);
    check(f.runtime.function_at(kUpdate) == &game_update, "at 50 degrees nothing is wrapped");
    f.call(kUpdate);
    check(seen_fov == kQuest, "the update sees the game's own view");
}

void test_wider_view_reaches_the_update_only() {
    Fixture f;
    check(prepare_field_of_view(f.runtime), "the matching game code is accepted");
    field_of_view_frame(f.runtime, 70u);
    check(f.runtime.function_at(kUpdate) != &game_update, "a wider view wraps the update");
    f.call(kUpdate);
    check(std::fabs(seen_fov - kQuest * 1.4f) < 1e-6f, "the update builds 70 degrees from the game's 50");
    check(read_float(f.memory(), kFov) == kQuest, "the game gets its own value back, bit for bit");
    check(std::fabs(read_float(f.memory(), kBuilt) - kQuest * 1.4f) < 1e-6f, "what was built stays recorded");

    // The village's own wider view widens in proportion.
    write_float(f.memory(), kFov, kVillage);
    f.call(kUpdate);
    check(std::fabs(seen_fov - kVillage * 1.4f) < 1e-6f, "a view the game widens widens in proportion");
    check(read_float(f.memory(), kFov) == kVillage, "and the game keeps its 58 degrees");

    // Back in the village: the game's view, built again.
    f.quest(false);
    field_of_view_frame(f.runtime, 70u);
    f.call(kUpdate);
    check(seen_fov == kVillage, "off a quest the update sees the game's view");
    check(read_float(f.memory(), kBuilt) == kVillage, "and builds it again");
    f.quest(true);
    field_of_view_frame(f.runtime, 70u);
    f.call(kUpdate);
    check(std::fabs(seen_fov - kVillage * 1.4f) < 1e-6f, "the next quest widens again");

    // Back to 50: the wrapper only passes through, and the projection is
    // built again from the game's value.
    field_of_view_frame(f.runtime, 50u);
    f.call(kUpdate);
    check(seen_fov == kVillage, "at 50 again the update sees the game's view");
    check(read_float(f.memory(), kBuilt) == kVillage, "and builds it again");
}

void test_smoothing_runs_in_the_game_s_terms() {
    Fixture f;
    check(prepare_field_of_view(f.runtime), "the matching game code is accepted");
    field_of_view_frame(f.runtime, 60u);
    write_float(f.memory(), kTarget, kVillage);
    float game = kQuest;
    for (int frame = 0; frame < 60; ++frame) {
        f.call(kSmoothUpdate);
        game += (kVillage - game) / 9.0f;
        check(std::fabs(seen_target - kVillage * 1.2f) < 1e-6f, "the step heads for the scaled target");
        check(std::fabs(seen_fov - game * 1.2f) < 1e-5f, "and draws the scaled step");
        check(std::fabs(read_float(f.memory(), kFov) - game) < 1e-5f, "the game keeps its own step");
        check(read_float(f.memory(), kTarget) == kVillage, "and its own target, bit for bit");
    }
}

void test_village_alone_wraps_nothing() {
    Fixture f;
    f.quest(false);
    check(prepare_field_of_view(f.runtime), "the matching game code is accepted");
    field_of_view_frame(f.runtime, 70u);
    check(f.runtime.function_at(kUpdate) == &game_update, "nothing is wrapped before a quest");
}

void test_too_wide_keeps_the_game_s_view() {
    Fixture f;
    check(prepare_field_of_view(f.runtime), "the matching game code is accepted");
    field_of_view_frame(f.runtime, 90u);
    write_float(f.memory(), kFov, 1.7f);
    f.call(kUpdate);
    check(seen_fov == 1.7f, "a view the setting would take past 160 degrees stays the game's");
}

void test_other_code_is_refused() {
    Fixture f(false);
    check(!prepare_field_of_view(f.runtime), "different game code is refused");
    check(!field_of_view_available(), "the setting is not offered");
    field_of_view_frame(f.runtime, 70u);
    check(f.runtime.function_at(kUpdate) == &game_update, "nothing is wrapped in other code");
}

} // namespace

int main() {
    test_game_view_writes_nothing();
    test_wider_view_reaches_the_update_only();
    test_smoothing_runs_in_the_game_s_terms();
    test_village_alone_wraps_nothing();
    test_too_wide_keeps_the_game_s_view();
    test_other_code_is_refused();
    if (failures != 0) {
        std::cerr << failures << " field of view check(s) failed\n";
        return 1;
    }
    std::cout << "field of view tests passed\n";
    return 0;
}
