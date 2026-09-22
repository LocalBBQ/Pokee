// No game code or game data is needed. The original rotation helper is a
// stand-in which records calls; the real runtime dispatches the camera hook.
#include "camera/camera_input.hpp"
#include "camera/game_camera.hpp"
#include "settings/settings.hpp"
#include "psprecomp/runtime.hpp"

#include <array>
#include <bit>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <vector>

namespace mhp3rd::settings {
Settings &current() {
    static Settings settings;
    return settings;
}
}

namespace {
using namespace mhp3rd::camera;
constexpr float frame_seconds = 1.0f / 30.0f;
constexpr std::uint32_t helper = 0x08878B70u;
constexpr std::uint32_t return_pc = 0x088E626Cu;
constexpr std::uint32_t camera_address = 0x08900000u;
constexpr std::uint32_t stack_address = 0x08901000u;
constexpr std::uint32_t preset_address = 0x08902000u;
int failures{};
unsigned original_calls{};

void check(bool condition, const char *message) {
    if (!condition) {
        ++failures;
        std::cerr << "FAIL: " << message << '\n';
    }
}

void original(psprecomp::Runtime &, psprecomp::AllegrexContext &ctx) {
    ++original_calls;
    ctx.pc = ctx.gpr[31];
}

void write_float(psprecomp::GuestMemory &memory, std::uint32_t address, float value) {
    memory.store32(address, std::bit_cast<std::uint32_t>(value));
}
float read_float(psprecomp::GuestMemory &memory, std::uint32_t address) {
    return std::bit_cast<float>(memory.load32(address));
}

struct Fixture {
    psprecomp::Runtime runtime;
    psprecomp::AllegrexContext ctx{};
    Fixture() {
        mhp3rd::settings::current() = {};
        runtime.register_generated_unit(29u, 0x08878000u, 0x4000u, &original, nullptr);
        runtime.register_function(helper, &original, "recomp_unit_test");
        auto &memory = runtime.memory();
        // A stand-in for the game code the driver checks before it installs.
        for (const CodeWord &word : game_code_signature()) memory.store32(word.address, word.word);
        check(prepare_game_camera(runtime, &original), "the matching game code is accepted");
        memory.store32(camera_address + 0x70u, preset_address);
        memory.store16(camera_address + 0x80u, 32000u);
        memory.store16(camera_address + 0x82u, 32000u);
        write_float(memory, preset_address + 0x10u, 190.0f);
        write_float(memory, camera_address + 4u, 150.0f);
        ctx.gpr[17] = camera_address;
        ctx.gpr[29] = stack_address;
        ctx.gpr[5] = stack_address + 0x50u;
        ctx.gpr[31] = return_pc;
        reset_offset();
    }
    void reset_offset() {
        write_float(runtime.memory(), stack_address + 0x34u, 150.0f);
        write_float(runtime.memory(), stack_address + 0x38u, 490.0f);
    }
    // One game flip followed by the camera update it leads to, as in
    // present_frame(): the stick's rate, the flip, then the time it covers.
    void flip(float x, float y, float seconds = frame_seconds) {
        set_rate(Source::Stick, x, y);
        game_camera_frame(runtime);
        advance(seconds, mhp3rd::settings::current().camera_speed);
    }
    void update() {
        reset_offset();
        check(runtime.invoke_isolated_aot(helper, ctx), "the rotation helper dispatches");
        check(ctx.pc == ctx.gpr[31], "original helper retains its return PC");
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
        auto &s = mhp3rd::settings::current();
        s.analog_camera = true;
        s.camera_speed = 90.0f;
    }
    float pitch() {
        auto &m = runtime.memory();
        return std::atan2(read_float(m, stack_address + 0x34u) - 190.0f,
                          read_float(m, stack_address + 0x38u)) * 180.0f / 3.14159265358979323846f;
    }
};

void test_passthrough() {
    Fixture f;
    auto before = f.snapshot();
    f.frame(1.0f, 1.0f);
    check(f.snapshot() == before, "Off leaves guest camera, stack and preset unchanged");
    check(!game_camera_driving(), "Off preserves right-stick input");
    f.enable();
    f.ctx.gpr[31] = 0x08812340u;
    f.frame(1.0f, 1.0f);
    check(f.snapshot() == before, "unrelated rotation calls are unchanged");
    f.ctx.gpr[31] = return_pc;
    f.runtime.memory().store8(camera_address + 0x76u, 3u);
    before = f.snapshot();
    f.frame(1.0f, 1.0f);
    check(f.snapshot() == before && !game_camera_driving(), "special camera modes retain control");
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
        return read_float(f.runtime.memory(), stack_address + 0x34u) == 150.0f &&
               read_float(f.runtime.memory(), stack_address + 0x38u) == 490.0f;
    };
    f.runtime.memory().store16(camera_address + 0x84u, 0x100u);
    const auto yaw = f.runtime.memory().load16(camera_address + 0x80u);
    f.frame(1.0f, 1.0f);
    check(baseline() && f.runtime.memory().load16(camera_address + 0x80u) == yaw,
          "recentre takes priority over both axes");
    f.runtime.memory().store16(camera_address + 0x84u, 0x10u);
    f.frame(0.0f, 1.0f);
    check(baseline(), "physical D-pad vertical command takes priority");
    f.runtime.memory().store16(camera_address + 0x84u, 0u);
    f.frame(0.0f, 1.0f);
    mhp3rd::settings::current().analog_camera = false;
    f.frame(0.0f, 0.0f);
    check(baseline() && !game_camera_driving(), "Off restores the stock preset and input");
    mhp3rd::settings::current().analog_camera = true;
    f.frame(0.0f, 1.0f);
    for (int i = 0; i < 3; ++i) f.flip(0.0f, 0.0f);
    check(!game_camera_driving(), "leaving the camera releases input ownership");
    f.frame(0.0f, 0.0f);
    check(baseline(), "returning after a scene change discards stale pitch");
    mhp3rd::settings::current().right_stick = mhp3rd::settings::RightStick::DPad;
    const auto before = f.snapshot();
    f.frame(1.0f, 1.0f);
    check(before == f.snapshot() && !game_camera_driving(), "D-pad mapping disables analog integration");
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
        const bool allowed = (i >= 4u && i < 8u) || (i >= 0x80u && i < 0x84u) || (i >= 0x1034u && i < 0x103Cu) ||
                             (i >= 0x1054u && i < 0x1058u);
        if (!allowed) check(before[i] == after[i], "writes stay inside the identified angles and stack arguments");
    }
}

void test_vertical_filter() {
    Fixture f;
    f.enable();
    for (int i = 0; i < 40; ++i) {
        f.frame(0.0f, i < 20 ? 0.25f : 0.0f);
        auto &m = f.runtime.memory();
        const float target = read_float(m, stack_address + 0x34u);
        const float current = read_float(m, camera_address + 4u);
        // The measured game filter must not introduce latency into stick
        // movement or continue that movement after release.
        const float filtered = current + (target - current) * 0.125f;
        check(std::fabs(filtered - target) < 0.001f, "manual vertical movement leaves no filter catch-up");
        write_float(m, camera_address + 4u, filtered);
    }
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
    check(!f.runtime.invoke_chained_direct<&original, 29u, 1u, helper>(f.ctx),
          "turning the option on installs the hook");
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
    mhp3rd::settings::current().analog_camera = false;
    f.frame(0.0f, 0.0f);
    mhp3rd::settings::current().analog_camera = true;
    f.frame(0.0f, 0.0f);
    check(static_cast<int>(start) - static_cast<int>(f.runtime.memory().load16(camera_address + 0x80u)) == turned,
          "motion made while the camera is not driven is dropped");
}

int main() {
    test_passthrough();
    test_rates_and_release();
    test_ownership();
    test_dispatch_and_write_extent();
    test_vertical_filter();
    test_signature_mismatch();
    test_hook_waits_for_the_option();
    test_frame_rate_independence();
    test_motion_source();
    check(original_calls > 0u, "original rotation helper is called");
    std::cout << (failures ? "FAIL" : "PASS") << ": analog camera (" << failures << " failures)\n";
    return failures ? 1 : 0;
}
