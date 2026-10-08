// No game code or game data is needed: the words the aspect ratio checks are
// written into guest memory as stand-ins, with a camera beside them.
#include "camera/game_aspect.hpp"
#include "psprecomp/runtime.hpp"

#include <bit>
#include <iostream>

namespace {
using namespace mhfu::camera;

constexpr std::uint32_t kCameraPointer = 0x089CBD04u;
constexpr std::uint32_t kCullFactor = 0x08815EC4u;
constexpr std::uint32_t kCullFactorWord = 0x3C023FC0u; // lui v0,0x3fc0: the baked 1.5
constexpr std::uint32_t kCamera = 0x08900000u;
constexpr std::uint32_t kGameAspectBits = 0x3FE1E1E2u;
constexpr float kGameAspect = 480.0f / 272.0f;
constexpr float kFieldOfView = 0.8722222f;
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

struct Fixture {
    psprecomp::Runtime runtime;
    explicit Fixture(bool matching = true) {
        auto &memory = runtime.memory();
        for (const CodeWord &word : game_aspect_signature()) memory.store32(word.address, word.word);
        if (!matching) memory.store32(game_aspect_signature().front().address, 0u);
        memory.store32(kCameraPointer, kCamera);
        write_float(memory, kCamera + 0x0u, 30.0f);
        write_float(memory, kCamera + 0x4u, 65000.0f);
        memory.store32(kCamera + 0x8u, kGameAspectBits);
        write_float(memory, kCamera + 0xCu, kFieldOfView);
        write_float(memory, kCamera + 0x10u, kFieldOfView);
    }
    [[nodiscard]] const psprecomp::GuestMemory &memory() const { return runtime.memory(); }
};

void test_own_shape_writes_nothing() {
    Fixture f;
    check(prepare_game_aspect(f.runtime), "the matching game code is accepted");
    write_float(f.runtime.memory(), kCamera + 0x10u, 1.0f); // marks a write, if any
    game_aspect_frame(f.runtime, kGameAspect);
    game_aspect_frame(f.runtime, 1.7650f); // within a hair of it
    check(read_float(f.memory(), kCamera + 0x10u) == 1.0f, "the game's shape leaves the camera alone");
    check(f.memory().load32(kCullFactor) == kCullFactorWord, "the game's shape leaves the depth factor alone");
}

void test_wide_and_back() {
    Fixture f;
    check(prepare_game_aspect(f.runtime), "the matching game code is accepted");
    game_aspect_frame(f.runtime, 21.0f / 9.0f);
    check(read_float(f.memory(), kCamera + 0x8u) == 21.0f / 9.0f, "21:9 reaches the camera");
    check(read_float(f.memory(), kCamera + 0x10u) != kFieldOfView, "the camera builds its projection again");
    check(f.memory().load32(kCullFactor) == kCullFactorWord, "21:9 leaves the baked depth factor");

    // The camera's update rebuilds and records the field of view again.
    write_float(f.runtime.memory(), kCamera + 0x10u, kFieldOfView);
    game_aspect_frame(f.runtime, 21.0f / 9.0f);
    check(read_float(f.memory(), kCamera + 0x10u) == kFieldOfView, "an unchanged shape forces nothing");

    game_aspect_frame(f.runtime, 32.0f / 9.0f);
    check(read_float(f.memory(), kCamera + 0x8u) == 32.0f / 9.0f, "32:9 reaches the camera");
    check(f.memory().load32(kCullFactor) == kCullFactorWord, "32:9 leaves the baked depth factor");

    game_aspect_frame(f.runtime, kGameAspect);
    check(f.memory().load32(kCamera + 0x8u) == kGameAspectBits, "off restores the camera bit for bit");
    check(f.memory().load32(kCullFactor) == kCullFactorWord, "off leaves the depth factor");

    write_float(f.runtime.memory(), kCamera + 0x10u, 1.0f);
    game_aspect_frame(f.runtime, kGameAspect);
    check(read_float(f.memory(), kCamera + 0x10u) == 1.0f, "once restored, nothing more is written");
}

void test_camera_set_up_later() {
    Fixture f;
    f.runtime.memory().store32(kCameraPointer, 0u);
    check(prepare_game_aspect(f.runtime), "the matching game code is accepted");
    game_aspect_frame(f.runtime, 16.0f / 10.0f);
    check(f.memory().load32(kCamera + 0x8u) == kGameAspectBits, "16:10 waits until a camera exists");
    f.runtime.memory().store32(kCameraPointer, kCamera);
    game_aspect_frame(f.runtime, 16.0f / 10.0f);
    check(read_float(f.memory(), kCamera + 0x8u) == 1.6f, "16:10 reaches the camera once it exists");
    game_aspect_frame(f.runtime, kGameAspect);
    check(f.memory().load32(kCamera + 0x8u) == kGameAspectBits, "and leaves it again");
}

void test_other_code_is_left_alone() {
    Fixture f(false);
    check(!prepare_game_aspect(f.runtime), "different game code is refused");
    game_aspect_frame(f.runtime, 21.0f / 9.0f);
    check(f.memory().load32(kCamera + 0x8u) == kGameAspectBits, "nothing is written to other code's camera");
    check(f.memory().load32(kCullFactor) == kCullFactorWord, "nor to its depth factor");
}

} // namespace

int main() {
    test_own_shape_writes_nothing();
    test_wide_and_back();
    test_camera_set_up_later();
    test_other_code_is_left_alone();
    if (failures != 0) {
        std::cerr << failures << " failure(s)\n";
        return 1;
    }
    std::cout << "game aspect tests passed\n";
    return 0;
}
