// No game code or game data is needed. The original camera update is a
// stand-in which records the offset it was handed; the real runtime
// dispatches the camera hook.
#include "camera/camera_input.hpp"
#include "camera/game_camera.hpp"
#include "settings/settings.hpp"
#include "psprecomp/runtime.hpp"

#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <vector>

namespace mhfu::settings {
Settings &current() {
    static Settings settings;
    return settings;
}
}

namespace {
using namespace mhfu::camera;
constexpr float frame_seconds = 1.0f / 30.0f;
constexpr std::uint32_t helper = 0x08886E50u;
constexpr std::uint32_t camera_address = 0x08900000u;
constexpr std::uint32_t parent_address = camera_address - 0xA0u;
constexpr std::uint32_t preset_address = 0x08902000u;
constexpr float kStockHeight = -40.0f;
constexpr float kStockDistance = 490.0f;
int failures{};
unsigned original_calls{};
// The eye height the camera update was entered with, before the stand-in
// copies the offset it was handed. The game eases that height by an eighth.
float eye_on_entry{};

void check(bool condition, const char *message) {
    if (!condition) {
        ++failures;
        std::cerr << "FAIL: " << message << '\n';
    }
}

void write_float(psprecomp::GuestMemory &memory, std::uint32_t address, float value) {
    memory.store32(address, std::bit_cast<std::uint32_t>(value));
}
float read_float(psprecomp::GuestMemory &memory, std::uint32_t address) {
    return std::bit_cast<float>(memory.load32(address));
}

void original(psprecomp::Runtime &runtime, psprecomp::AllegrexContext &ctx) {
    ++original_calls;
    auto &memory = runtime.memory();
    const auto camera = ctx.gpr[4] + 0xA0u;
    if (memory.raw_pointer(camera, 0x98u) != nullptr) eye_on_entry = read_float(memory, camera + 4u);
    const auto preset = memory.raw_pointer(camera, 0x98u) != nullptr ? memory.load32(camera + 0x70u) : 0u;
    if (memory.raw_pointer(preset, 0x0Cu) != nullptr) {
        // The game reads the preset, then builds the eye. Copying it here lets
        // the test see the offset after the driver has put the preset back.
        memory.store32(camera + 4u, memory.load32(preset + 4u));
        memory.store32(camera + 8u, memory.load32(preset + 8u));
    }
    ctx.pc = ctx.gpr[31];
}

struct Fixture {
    psprecomp::Runtime runtime;
    psprecomp::AllegrexContext ctx{};
    Fixture() {
        mhfu::settings::current() = {};
        // Each test turns the option on itself, to see what changes.
        mhfu::settings::current().analog_camera = false;
        runtime.register_generated_unit(29u, 0x08886000u, 0x1000u, &original, nullptr);
        runtime.register_function(helper, &original, "recomp_unit_test");
        auto &memory = runtime.memory();
        // A stand-in for the game code the driver checks before it installs.
        for (const CodeWord &word : game_code_signature()) memory.store32(word.address, word.word);
        check(prepare_game_camera(runtime, &original), "the matching game code is accepted");
        memory.store32(camera_address + 0x70u, preset_address);
        memory.store16(camera_address + 0x80u, 32000u);
        memory.store16(camera_address + 0x82u, 32000u);
        memory.store8(camera_address + 0x91u, 0xFFu); // the game is not placing the camera itself
        write_float(memory, camera_address + 4u, kStockHeight);
        write_float(memory, camera_address + 8u, kStockDistance);
        ctx.gpr[4] = parent_address;
        ctx.gpr[31] = 0x09C00000u;
        reset_offset();
    }
    void reset_offset() {
        write_float(runtime.memory(), preset_address + 4u, kStockHeight);
        write_float(runtime.memory(), preset_address + 8u, kStockDistance);
    }
    // One game flip followed by the camera update it leads to, as in
    // present_frame(): the stick's rate, the flip, then the time it covers.
    void flip(float x, float y, float seconds = frame_seconds) {
        set_rate(Source::Stick, x, y);
        game_camera_frame(runtime);
        advance(seconds, game_camera_degrees_per_second());
    }
    void update() {
        reset_offset();
        check(runtime.invoke_isolated_aot(helper, ctx), "the camera update dispatches");
        check(ctx.pc == ctx.gpr[31], "original update retains its return PC");
    }
    void frame(float x, float y, float seconds = frame_seconds) {
        flip(x, y, seconds);
        update();
    }
    std::vector<std::uint8_t> snapshot() {
        const auto *p = runtime.memory().raw_pointer(camera_address, 0x2100u);
        return {p, p + 0x2100u};
    }
    void enable() {
        auto &s = mhfu::settings::current();
        s.analog_camera = true;
        s.camera_speed = 90.0f;
    }
    float pitch() {
        auto &m = runtime.memory();
        return std::atan2(read_float(m, camera_address + 4u), read_float(m, camera_address + 8u)) * 180.0f /
            3.14159265358979323846f;
    }
    bool stock_preset() {
        auto &m = runtime.memory();
        return read_float(m, preset_address + 4u) == kStockHeight &&
            read_float(m, preset_address + 8u) == kStockDistance;
    }
};

void test_passthrough() {
    Fixture f;
    auto before = f.snapshot();
    f.frame(1.0f, 1.0f);
    check(f.snapshot() == before, "Off leaves guest camera and preset unchanged");
    check(!game_camera_driving(), "Off preserves right-stick input");
    f.enable();
    f.ctx.gpr[4] = 0u;
    f.frame(1.0f, 1.0f);
    check(f.snapshot() == before, "a call that is not the camera's parent is unchanged");
    f.ctx.gpr[4] = parent_address;
    f.runtime.memory().store8(camera_address + 0x91u, 0u);
    before = f.snapshot();
    f.frame(1.0f, 1.0f);
    check(f.snapshot() == before && !game_camera_driving() && f.stock_preset(),
        "the behind-the-hunter placement keeps the game's camera");
}

void test_rates_and_release() {
    Fixture f;
    f.enable();
    const float initial = f.pitch();
    for (int i = 0; i < 20; ++i) f.frame(0.5f, 0.1f);
    const auto yaw = f.runtime.memory().load16(camera_address + 0x80u);
    check(yaw == 26539u, "half stick integrates 30 degrees in twenty camera updates, including fractions");
    check(f.runtime.memory().load16(camera_address + 0x82u) == yaw, "yaw target and filter agree");
    check(std::fabs(f.pitch() - initial - 6.0f) < 0.001f, "small vertical deflections produce proportional pitch");
    const float held = f.pitch();
    for (int i = 0; i < 30; ++i) f.frame(0.0f, 0.0f);
    check(f.runtime.memory().load16(camera_address + 0x80u) == yaw, "released yaw stops accumulating");
    check(std::fabs(f.pitch() - held) < 0.001f, "released pitch holds its angle");
    check(f.stock_preset(), "the shared preset is put back after the update");
    check(game_camera_driving(), "both digital stick commands are suppressed");
    for (int i = 0; i < 100; ++i) f.frame(0.0f, 1.0f);
    check(std::fabs(f.pitch() - 70.0f) < 0.001f, "upper pitch limit is bounded");
    for (int i = 0; i < 100; ++i) f.frame(0.0f, -1.0f);
    check(std::fabs(f.pitch() + 60.0f) < 0.001f, "lower pitch limit is bounded");
}

void test_ownership() {
    Fixture f;
    f.enable();
    f.frame(0.0f, 1.0f);
    const auto baseline = [&] {
        return read_float(f.runtime.memory(), camera_address + 4u) == kStockHeight &&
            read_float(f.runtime.memory(), camera_address + 8u) == kStockDistance;
    };
    f.runtime.memory().store8(camera_address + 0x8Eu, 1u);
    const auto yaw = f.runtime.memory().load16(camera_address + 0x80u);
    f.frame(1.0f, 1.0f);
    check(baseline() && f.runtime.memory().load16(camera_address + 0x80u) == yaw && f.stock_preset(),
        "a snap takes priority over both axes");
    f.runtime.memory().store8(camera_address + 0x8Eu, 0u);
    f.runtime.memory().store16(camera_address + 0x86u, 0x10u);
    f.frame(0.0f, 1.0f);
    check(baseline(), "physical D-pad vertical command takes priority");
    f.runtime.memory().store16(camera_address + 0x86u, 0u);
    f.frame(0.0f, 1.0f);
    mhfu::settings::current().analog_camera = false;
    f.frame(0.0f, 0.0f);
    check(baseline() && !game_camera_driving(), "Off restores the stock preset and input");
    mhfu::settings::current().analog_camera = true;
    f.frame(0.0f, 1.0f);
    for (int i = 0; i < 3; ++i) f.flip(0.0f, 0.0f);
    check(!game_camera_driving(), "leaving the camera releases input ownership");
    f.frame(0.0f, 0.0f);
    check(baseline(), "returning after a scene change discards stale pitch");
    mhfu::settings::current().right_stick = mhfu::settings::RightStick::DPad;
    const auto before = f.snapshot();
    if constexpr (mhfu::settings::kPlatform == mhfu::settings::Platform::Android) {
        // A physical stick mapping must not turn off the phone's touch camera.
        add_motion(Source::Touch, 10.0f, 0.0f);
        f.frame(0.0f, 0.0f);
        check(before != f.snapshot() && game_camera_driving(),
            "D-pad mapping preserves Android touch camera integration");
    } else {
        f.frame(1.0f, 1.0f);
        check(before == f.snapshot() && !game_camera_driving(), "D-pad mapping disables analog integration");
    }
}

void test_dispatch_and_write_extent() {
    Fixture f;
    f.enable();
    const auto before = f.snapshot();
    f.flip(0.5f, 0.5f);
    check(!f.runtime.invoke_chained_direct<&original, 29u, 1u, helper>(f.ctx),
        "generated direct calls unwind to the registered camera hook");
    check(f.ctx.pc == helper, "dispatch fallback preserves the helper address");
    check(f.runtime.invoke_isolated_aot(helper, f.ctx), "outer dispatch reaches the hook");
    const auto after = f.snapshot();
    for (std::size_t i = 0; i < before.size(); ++i) {
        const bool allowed = (i >= 4u && i < 12u) || (i >= 0x80u && i < 0x84u);
        if (!allowed) check(before[i] == after[i], "writes stay inside the yaw and the eye the update builds");
    }
}

void test_pitch_is_handed_whole() {
    Fixture f;
    f.enable();
    const float initial = f.pitch();
    f.frame(0.0f, 1.0f);
    check(std::fabs(f.pitch() - initial - 3.0f) < 0.001f, "one update hands the game the whole pitch step");
    check(f.stock_preset(), "and the preset table is unchanged afterwards");
}

// 0x08887D10 eases eye height by an eighth and snaps the other axes. Looking
// up is almost entirely that height, so leaving the step for the ease is the
// stickiness. The eye must already be at the handed height when the update
// starts, or the eighth would still be closing the stick.
void test_pitch_is_not_left_for_the_eye_ease() {
    Fixture f;
    f.enable();
    const float down = read_float(f.runtime.memory(), camera_address + 4u);
    for (int i = 0; i < 8; ++i) f.frame(0.0f, -1.0f);
    const float looked_up = read_float(f.runtime.memory(), camera_address + 4u);
    check(looked_up < down - 1.0f, "looking up lowers the eye");
    check(std::fabs(eye_on_entry - looked_up) < 0.001f, "the eye is already at the looked-up height");
    const float up = looked_up;
    for (int i = 0; i < 8; ++i) f.frame(0.0f, 1.0f);
    const float looked_down = read_float(f.runtime.memory(), camera_address + 4u);
    check(looked_down > up + 1.0f, "looking down raises the eye");
    check(std::fabs(eye_on_entry - looked_down) < 0.001f, "the eye is already at the looked-down height");
    f.frame(0.0f, 0.0f);
    check(std::fabs(eye_on_entry - looked_down) < 0.001f, "releasing adds no further height");
}
}

void test_signature_mismatch() {
    Fixture f;
    const CodeWord &first = game_code_signature().front();
    f.runtime.memory().store32(first.address, first.word ^ 1u);
    check(!prepare_game_camera(f.runtime, &original), "different game code is refused");
    f.enable();
    const auto before = f.snapshot();
    f.frame(1.0f, 1.0f);
    check(f.snapshot() == before && !game_camera_driving(), "refused code is never written to");
}

void test_hook_waits_for_the_option() {
    Fixture f;
    f.flip(0.0f, 0.0f);
    check(f.runtime.invoke_chained_direct<&original, 29u, 1u, helper>(f.ctx),
        "with the option off the helper's unit keeps its direct calls");
    f.enable();
    f.flip(0.0f, 0.0f);
    check(
        !f.runtime.invoke_chained_direct<&original, 29u, 1u, helper>(f.ctx), "turning the option on installs the hook");
}

void test_frame_rate_independence() {
    const auto turn_for_one_second = [](int updates) {
        Fixture f;
        f.enable();
        f.frame(0.0f, 0.0f);
        const auto start = f.runtime.memory().load16(camera_address + 0x80u);
        for (int i = 0; i < updates; ++i) f.frame(0.5f, 0.0f, 1.0f / static_cast<float>(updates));
        f.frame(0.0f, 0.0f);
        return static_cast<int>(start) - static_cast<int>(f.runtime.memory().load16(camera_address + 0x80u));
    };
    const int at_30 = turn_for_one_second(30);
    const int at_20 = turn_for_one_second(20);
    // Half deflection at 90 degrees a second: 45 degrees in either case.
    check(std::abs(at_30 - 8192) <= 1, "a second of stick turns 45 degrees at 30 updates a second");
    check(std::abs(at_20 - at_30) <= 1, "a slower game turns the camera the same amount in the same time");
}

void test_motion_source() {
    Fixture f;
    f.enable();
    f.frame(0.0f, 0.0f);
    const auto start = f.runtime.memory().load16(camera_address + 0x80u);
    add_motion(Source::Mouse, 10.0f, 0.0f);
    f.frame(0.0f, 0.0f);
    const int turned = static_cast<int>(start) - static_cast<int>(f.runtime.memory().load16(camera_address + 0x80u));
    check(std::abs(turned - 1820) <= 1, "motion turns by its degrees once");
    f.frame(0.0f, 0.0f);
    check(static_cast<int>(start) - static_cast<int>(f.runtime.memory().load16(camera_address + 0x80u)) == turned,
        "motion is not repeated on the next update");
    add_motion(Source::Mouse, 10.0f, 0.0f);
    mhfu::settings::current().analog_camera = false;
    f.frame(0.0f, 0.0f);
    mhfu::settings::current().analog_camera = true;
    f.frame(0.0f, 0.0f);
    check(static_cast<int>(start) - static_cast<int>(f.runtime.memory().load16(camera_address + 0x80u)) == turned,
        "motion made while the camera is not driven is dropped");
}

// Where the port does not drive the camera the mouse switches the game's turn.
void test_mouse_stock_turn() {
    Fixture f;
    f.flip(0.0f, 0.0f);
    add_motion(Source::Mouse, 0.3f, 5.0f);
    check(game_camera_mouse_stock_turn() == 0, "slow or vertical motion does not turn the game's camera");
    add_motion(Source::Mouse, 0.4f, 0.0f);
    check(game_camera_mouse_stock_turn() == 1, "sideways motion switches the turn on that way");
    f.flip(0.0f, 0.0f);
    check(game_camera_mouse_stock_turn() == 0, "and only for the frame it moved in");
    add_motion(Source::Mouse, -2.0f, 0.0f);
    check(game_camera_mouse_stock_turn() == -1, "left turns left");
    f.enable();
    f.frame(0.0f, 0.0f);
    f.flip(0.0f, 0.0f);
    add_motion(Source::Mouse, 2.0f, 0.0f);
    check(game_camera_mouse_stock_turn() == 0, "while the port drives the camera the mouse goes to the driver");
    f.update();
}

// The hunter the camera follows: list word, list object, then the hunter.
constexpr std::uint32_t kListWord = 0x089CBD04u;
constexpr std::uint32_t kList = 0x08920000u;
constexpr std::uint32_t kHunter = 0x08910000u;
constexpr std::uint32_t kHeading = 800u;
constexpr std::uint32_t kFacing = 500u;
constexpr std::uint32_t kAction = 804u;
constexpr std::uint32_t kMagnitude = 1182u;
constexpr std::uint32_t kPitch = 1662u;

void plant_hunter(psprecomp::GuestMemory &memory, std::uint16_t action) {
    memory.store32(kListWord, kList);
    memory.store32(kList + 2692u, kHunter);
    memory.store16(kHunter + kAction, action);
    memory.store16(kHunter + kHeading, 10000u);
    memory.store32(kHunter + kFacing, 0xABC00000u | 10000u);
    memory.store8(kHunter + kPitch, 0u);
    memory.store16(kHunter + kMagnitude, 0u);
}

void game_steps(psprecomp::GuestMemory &memory, int yaw, int pitch) {
    const auto heading = memory.load16(kHunter + kHeading);
    memory.store16(kHunter + kHeading, static_cast<std::uint16_t>(heading + yaw));
    const auto body = memory.load32(kHunter + kFacing);
    memory.store32(kHunter + kFacing, (body & 0xFFFF0000u) | static_cast<std::uint16_t>(static_cast<std::uint16_t>(body) + yaw));
    const int next = static_cast<std::int8_t>(memory.load8(kHunter + kPitch)) + pitch;
    memory.store8(kHunter + kPitch, static_cast<std::uint8_t>(static_cast<std::int8_t>(next)));
}

// The right stick is shown to the aim function, then the hunter's stick
// fields are put back, so that push does not stay as movement.
std::uint32_t seen_buttons{};
std::uint16_t seen_magnitude{};
void aim_original(psprecomp::Runtime &runtime, psprecomp::AllegrexContext &ctx) {
    auto &memory = runtime.memory();
    seen_buttons = memory.load32(ctx.gpr[4] + 1152u);
    seen_magnitude = memory.load16(ctx.gpr[4] + kMagnitude);
    ctx.pc = ctx.gpr[31];
}

void test_aim_stick_is_not_movement() {
    Fixture f;
    f.enable();
    auto &memory = f.runtime.memory();
    for (const CodeWord &word : aim_code_signature()) memory.store32(word.address, word.word);
    f.runtime.register_function(0x09AA4DD0u, &aim_original, "recomp_unit_aim_test");
    f.flip(0.0f, 0.0f);
    constexpr std::uint32_t kButtons = 1152u;
    memory.store32(kHunter + kButtons, 0x00800001u);
    memory.store16(kHunter + kMagnitude, 10u);
    memory.store8(kHunter + 1380u, 1u);
    game_camera_present_aim(127, -40);
    f.ctx.gpr[4] = kHunter;
    f.ctx.gpr[31] = 0x09C00000u;
    check(f.runtime.invoke_isolated_aot(0x09AA4DD0u, f.ctx), "the aim function dispatches");
    check(seen_magnitude == 127u, "the aim function is shown a full push");
    check(seen_buttons == 0x00300001u, "right and up replace the stick direction and the real button stays");
    check(memory.load16(kHunter + kMagnitude) == 10u, "the movement magnitude is put back");
    check(memory.load32(kHunter + kButtons) == 0x00800001u, "the movement direction is put back");

    game_camera_present_aim(0, 0);
    check(f.runtime.invoke_isolated_aot(0x09AA4DD0u, f.ctx), "the aim function dispatches with no push");
    check(seen_magnitude == 10u, "no push leaves the stick the game copied");
    check(seen_buttons == 0x00800001u, "and leaves its direction");

    // A bowgun leaves +1380 clear, and the aim function then steps from the
    // D-pad bits rather than the stick half.
    memory.store8(kHunter + 1380u, 0u);
    game_camera_present_aim(127, -40);
    check(f.runtime.invoke_isolated_aot(0x09AA4DD0u, f.ctx), "the bowgun aim function dispatches");
    check(seen_buttons == 0x00800031u, "a bowgun aim is shown as D-pad right and up");
    check(seen_magnitude == 10u, "and the movement magnitude is not replaced");
    check(memory.load32(kHunter + kButtons) == 0x00800001u, "the bowgun's movement direction is put back");
}

void test_proportional_aim() {
    Fixture f;
    f.enable();
    mhfu::settings::current().aim_speed = 90.0f;
    auto &memory = f.runtime.memory();
    plant_hunter(memory, 1001u);
    f.frame(0.0f, 0.0f);
    check(memory.load16(kHunter + kHeading) == 10000u, "the first update of an aim only learns the heading");

    f.flip(1.0f, 1.0f);
    game_steps(memory, 400, 4);
    f.update();
    check(memory.load16(kHunter + kHeading) == 10546u, "a full stick replaces the game's yaw step of 400 with 3 degrees");
    check(static_cast<std::int8_t>(memory.load8(kHunter + kPitch)) == 16, "and its pitch step of 4 with the same 3 degrees");
    check(memory.load32(kHunter + kFacing) == (0xABC00000u | 10546u), "the body facing's low half moves with the aim");

    const auto held = memory.load16(kHunter + kHeading);
    f.flip(1.0f, 1.0f);
    f.update();
    check(memory.load16(kHunter + kHeading) == held, "no step from the game means the stick does not move the aim");

    f.flip(1.0f, 0.0f);
    game_steps(memory, 4000, 0);
    f.update();
    check(memory.load16(kHunter + kHeading) == static_cast<std::uint16_t>(held + 4000u),
        "a heading the game sets outright is left as it set it");

    plant_hunter(memory, 1001u);
    f.frame(0.0f, 0.0f);
    memory.store16(kHunter + kMagnitude, 64u);
    f.flip(0.0f, 0.0f);
    game_steps(memory, 128, 1);
    f.update();
    check(memory.load16(kHunter + kHeading) == 10275u, "the left stick's middle deflection is one continuous speed");
    check(static_cast<std::int8_t>(memory.load8(kHunter + kPitch)) == 8, "and the same fraction of the pitch");

    memory.store8(kHunter + kPitch, static_cast<std::uint8_t>(99));
    f.frame(0.0f, 0.0f);
    f.flip(0.0f, 1.0f);
    game_steps(memory, 0, 4);
    f.update();
    check(static_cast<std::int8_t>(memory.load8(kHunter + kPitch)) == 100, "pitch stops at 100");

    plant_hunter(memory, 0u);
    f.frame(0.0f, 0.0f);
    f.flip(1.0f, 0.0f);
    game_steps(memory, 400, 4);
    f.update();
    check(memory.load16(kHunter + kHeading) == 10400u, "another action keeps the game's own step");
    check(static_cast<std::int8_t>(memory.load8(kHunter + kPitch)) == 4, "and its own pitch step");

    plant_hunter(memory, 1001u);
    f.frame(0.0f, 0.0f);
    f.flip(0.0f, 0.0f);
    add_motion(Source::Mouse, 10.0f, 0.0f);
    check(game_camera_mouse_aim().has_value(), "the mouse is shown to the aim while the stick is idle");
    game_steps(memory, 400, 0);
    f.update();
    check(memory.load16(kHunter + kHeading) == 11820u, "the mouse sizes the game's step from its degrees");
}

int main() {
    test_passthrough();
    test_rates_and_release();
    test_ownership();
    test_dispatch_and_write_extent();
    test_pitch_is_handed_whole();
    test_pitch_is_not_left_for_the_eye_ease();
    test_signature_mismatch();
    test_hook_waits_for_the_option();
    test_frame_rate_independence();
    test_motion_source();
    test_mouse_stock_turn();
    test_aim_stick_is_not_movement();
    test_proportional_aim();
    check(original_calls > 0u, "original camera update is called");
    std::cout << (failures ? "FAIL" : "PASS") << ": analog camera (" << failures << " failures)\n";
    return failures ? 1 : 0;
}
