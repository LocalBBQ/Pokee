// No game code or game data is needed. The original rotation helper is a
// stand-in which records calls; the real runtime dispatches the camera hook.
#include "input/analog_camera.hpp"
#include "settings/settings.hpp"
#include "psprecomp/runtime.hpp"

#include <array>
#include <bit>
#include <cmath>
#include <iostream>
#include <vector>

namespace mhp3rd::settings {
Settings &current() {
    static Settings settings;
    return settings;
}
}

namespace {
using namespace mhp3rd::input;
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
        install_analog_camera(runtime, &original);
        auto &memory = runtime.memory();
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
    void frame(float x, float y) {
        analog_camera_frame(x, y);
        reset_offset();
        check(runtime.invoke_isolated_aot(helper, ctx), "camera hook is registered");
        check(ctx.pc == ctx.gpr[31], "original helper retains its return PC");
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
    check(!analog_camera_driving(), "Off preserves right-stick input");
    f.enable();
    f.ctx.gpr[31] = 0x08812340u;
    f.frame(1.0f, 1.0f);
    check(f.snapshot() == before, "unrelated rotation calls are unchanged");
    f.ctx.gpr[31] = return_pc;
    f.runtime.memory().store8(camera_address + 0x76u, 3u);
    before = f.snapshot();
    f.frame(1.0f, 1.0f);
    check(f.snapshot() == before && !analog_camera_driving(), "special camera modes retain control");
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
    check(analog_camera_driving(), "both digital stick commands are suppressed");
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
    check(baseline() && !analog_camera_driving(), "Off restores the stock preset and input");
    mhp3rd::settings::current().analog_camera = true;
    f.frame(0.0f, 1.0f);
    for (int i = 0; i < 3; ++i) analog_camera_frame(0.0f, 0.0f);
    check(!analog_camera_driving(), "leaving the camera releases input ownership");
    f.frame(0.0f, 0.0f);
    check(baseline(), "returning after a scene change discards stale pitch");
    mhp3rd::settings::current().right_stick = mhp3rd::settings::RightStick::DPad;
    const auto before = f.snapshot();
    f.frame(1.0f, 1.0f);
    check(before == f.snapshot() && !analog_camera_driving(), "D-pad mapping disables analog integration");
}

void test_dispatch_and_write_extent() {
    Fixture f;
    f.enable();
    const auto before = f.snapshot();
    analog_camera_frame(0.5f, 0.5f);
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

int main() {
    test_passthrough();
    test_rates_and_release();
    test_ownership();
    test_dispatch_and_write_extent();
    test_vertical_filter();
    check(original_calls > 0u, "original rotation helper is called");
    std::cout << (failures ? "FAIL" : "PASS") << ": analog camera (" << failures << " failures)\n";
    return failures ? 1 : 0;
}
