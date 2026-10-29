#include "camera/game_aspect.hpp"

#include "game/guest_layout.hpp"
#include "install/release_addresses.hpp"
#include "psprecomp/runtime.hpp"

#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <iomanip>
#include <iostream>

namespace mhfu::camera {
namespace {

// ULUS-10391. The set-up at 0x08812A08 writes near 30, far 65000, the aspect
// ratio 480/272 and a vertical field of view of 0.872222 radians. The reset
// at 0x08812B9C writes that same ratio and jumps to the projection
// (0x08815348). The perspective builder (0x088144B4) divides the x scale by
// the aspect, and the camera's update (0x08815384) builds the projection
// again only when the field of view differs from the one kept at +0x10.
// The constructor stores the object at 0x089CBD04. The follow camera reads
// the hunter from that same object, at +0xA84; the projection fields are the
// first words.
//
// The 480/272 ratio and the culling depth of 1.5 are lui/ori immediates.
// The recompiled code has those bits baked in, so writing the instruction
// words would not change what runs. The live camera is data the update
// loads, and that is what a frame writes. A camera created during a frame
// keeps the game's ratio until the next update. The culling planes
// (0x08815EC0) divide by the same aspect and then by that baked 1.5, which
// covers the view up to an aspect ratio of about 3. Wider than that, the
// sides can clip; the factor cannot be shrunk without changing the
// recompiled function.
constexpr std::uint32_t kGameAspectBits = 0x3FE1E1E2u; // 480/272
// In the camera object (game::layout::camera_object).
constexpr std::uint32_t kNear = 0x00u;
constexpr std::uint32_t kFar = 0x04u;
constexpr std::uint32_t kAspect = 0x08u;
constexpr std::uint32_t kFieldOfView = 0x0Cu;
constexpr std::uint32_t kBuiltFieldOfView = 0x10u;
constexpr std::uint32_t kCameraBytes = 0x20u;

constexpr std::array<CodeWord, 22> kSignature{{
    {0x08812948u, 0x3C02089Du, "lui v0,0x89d: the base of the camera pointer"},
    {0x08812954u, 0xAC50BD04u, "sw s0,-0x42fc(v0): the camera object, kept at 0x089CBD04"},
    {0x08812A70u, 0x3C0341F0u, "lui v1,0x41f0: near plane, 30"},
    {0x08812AA8u, 0x3C02477Du, "lui v0,0x477d: far plane, 65000"},
    {0x08812AB0u, 0x3442E800u, "ori v0,v0,0xe800: the low half of 65000"},
    {0x08812AACu, 0xAE030000u, "sw v1,0x0(s0): the near plane"},
    {0x08812AB4u, 0xAE020004u, "sw v0,0x4(s0): the far plane"},
    {0x08812AC4u, 0x3C023FE1u, "lui v0,0x3fe1: the aspect ratio 480/272"},
    {0x08812AC8u, 0x3442E1E2u, "ori v0,v0,0xe1e2: its low half"},
    {0x08812ACCu, 0xAE020008u, "sw v0,0x8(s0): into the camera's aspect"},
    {0x08812BBCu, 0x3C023FE1u, "lui v0,0x3fe1: the same ratio where the camera is reset"},
    {0x08812BC0u, 0x3442E1E2u, "ori v0,v0,0xe1e2: its low half, on the reset"},
    {0x08812BC4u, 0xAC820008u, "sw v0,0x8(a0): into that camera's aspect"},
    {0x08815354u, 0xC48C000Cu, "lwc1 f12,0xc(a0): the projection's field of view"},
    {0x08815358u, 0xC48D0008u, "lwc1 f13,0x8(a0): the projection's aspect"},
    {0x0881456Cu, 0x46160803u, "div.s f0,f1,f22: the projection's x scale is cot(fov/2) / aspect"},
    {0x08815390u, 0xC4810010u, "lwc1 f1,0x10(a0): the field of view the projection was built with"},
    {0x08815394u, 0xC480000Cu, "lwc1 f0,0xc(a0): and the current one"},
    {0x08815398u, 0x46000832u, "c.eq.s f1,f0: the update builds the projection again when they differ"},
    {0x08815EC4u, 0x3C023FC0u, "lui v0,0x3fc0: the culling depth factor, 1.5"},
    {0x08815F50u, 0xC4800008u, "lwc1 f0,0x8(a0): the culling planes read the aspect"},
    {0x08815F54u, 0x46000803u, "div.s f0,f1,f0: and divide by it"},
}};

struct State {
    bool prepared{};
    // The port has written something since the game last had its own values.
    bool changed{};
    float applied{};
};

State state;

float load_float(const psprecomp::GuestMemory &memory, std::uint32_t address) {
    return std::bit_cast<float>(memory.load32(address));
}

void store_float(psprecomp::GuestMemory &memory, std::uint32_t address, float value) {
    memory.store32(address, std::bit_cast<std::uint32_t>(value));
}

// The camera, when the pointer leads to one that looks like it.
std::uint32_t camera_address(const psprecomp::GuestMemory &memory) {
    const std::uint32_t address = game::layout::camera_object(memory);
    if (address == 0u || !memory.contains(address, kCameraBytes)) return 0u;
    const float near_plane = load_float(memory, address + kNear);
    const float far_plane = load_float(memory, address + kFar);
    const float aspect = load_float(memory, address + kAspect);
    const float fov = load_float(memory, address + kFieldOfView);
    const bool plausible = std::isfinite(near_plane) && std::isfinite(far_plane) && near_plane > 0.0f &&
        far_plane > near_plane && aspect > 0.25f && aspect < 8.0f && fov > 0.01f && fov < 3.1f;
    return plausible ? address : 0u;
}

} // namespace

std::span<const CodeWord> game_aspect_signature() {
    return kSignature;
}

bool prepare_game_aspect(psprecomp::Runtime &runtime) {
    state = State{};
    const psprecomp::GuestMemory &memory = runtime.memory();
    for (const CodeWord &expected : kSignature) {
        if (install::code_matches(memory, expected.address, expected.word)) continue;
        const std::uint32_t address = install::port(expected.address);
        const std::uint32_t found = memory.contains(address, 4u) ? memory.load32(address) : 0u;
        std::cerr << "[aspect] game code differs at 0x" << std::hex << std::uppercase << std::setfill('0')
                  << std::setw(8) << address << ": 0x" << std::setw(8) << found << ", expected 0x" << std::setw(8)
                  << expected.word << std::dec << std::nouppercase << std::setfill(' ') << " (" << expected.what
                  << "); the view keeps the PSP's shape\n";
        return false;
    }
    state.prepared = true;
    return true;
}

void game_aspect_frame(psprecomp::Runtime &runtime, float aspect) {
    if (!state.prepared) return;
    psprecomp::GuestMemory &memory = runtime.memory();
    const float game = std::bit_cast<float>(kGameAspectBits);
    // Within a hair of the game's own shape is the game's own shape, written
    // bit for bit as the game has it.
    const bool own = !std::isfinite(aspect) || std::abs(aspect - game) < 0.002f;
    if (own && !state.changed) return;
    const float wanted = own ? game : std::clamp(aspect, 0.5f, 5.0f);
    const std::uint32_t bits = own ? kGameAspectBits : std::bit_cast<std::uint32_t>(wanted);

    // The set-up plants 480/272. The next camera update rebuilds the
    // projection when the field of view it last built with differs from the
    // current one.
    if (const std::uint32_t found = camera_address(memory); found != 0u && memory.load32(found + kAspect) != bits) {
        memory.store32(found + kAspect, bits);
        store_float(memory, found + kBuiltFieldOfView, -load_float(memory, found + kFieldOfView));
        if (!state.changed || wanted != state.applied)
            std::cout << "[aspect] the game's view is " << wanted << " wide to 1 high\n";
        state.applied = wanted;
    }
    // Back to the game's own value once the camera has it too. Until a
    // camera turns up this keeps looking, which costs one read a frame.
    const std::uint32_t camera = camera_address(memory);
    state.changed = !own || camera == 0u || memory.load32(camera + kAspect) != kGameAspectBits;
}

} // namespace mhfu::camera
