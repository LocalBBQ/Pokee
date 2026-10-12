#include "game/item_box.hpp"

#include "psprecomp/runtime.hpp"

#include <array>
#include <cstdlib>
#include <iostream>
#include <string>

namespace mhfu::game::item_box {
namespace {

constexpr std::array<CodeWord, 9> kSignature = {{
    {kHudUpdate, 0x27BDFFE0u},    // addiu sp, sp, -32
    {0x0883F75Cu, 0x0E211DBDu},   // jal 0x088476F4
    {0x0883F760u, 0x02202021u},   // addu a0, s1, zr
    {kBoxUpdate, 0x27BDFFD0u},    // addiu sp, sp, -48
    {0x08847740u, 0x9244055Eu},   // lbu a0, 1374(s2)
    {0x08847B98u, 0x2402001Eu},   // addiu v0, zr, 30
    {0x08847BA0u, 0xA0A2055Eu},   // sb v0, 1374(a1)
    {0x0884860Cu, 0x2403001Fu},   // addiu v1, zr, 31
    {0x08848610u, 0xA483059Eu},   // sh v1, 1438(a0)
}};

struct State {
    psprecomp::Runtime::RecompiledFunction original{};
    bool reported{};
    bool trace{};
    std::uint64_t frame{};
};

State &state() {
    static State s;
    return s;
}

// Unset or any value but "0": the box is fast. "0" is only for comparing the
// game's own pause on the same screen.
bool enabled() {
    const char *text = std::getenv("MHFU_FAST_ITEM_BOX");
    return text == nullptr || *text == '\0' || std::string(text) != "0";
}

void apply_fast_box(psprecomp::GuestMemory &memory, std::uint32_t hud, bool on) {
    if (!memory.contains(hud, kAnimHalf + 2u) || !memory.contains(hud + kPlayerPointer, 4u)) return;
    const std::uint32_t player = memory.load32(hud + kPlayerPointer);
    const std::uint16_t anim = memory.load16(hud + kAnimHalf);
    std::uint8_t lock = 0;
    if (memory.contains(player, kLockByte + 1u)) lock = memory.load8(player + kLockByte);
    const std::uint8_t shortened = on ? shortened_lock(lock) : lock;
    const std::uint16_t flight = on ? shortened_anim(anim) : anim;
    if (shortened != lock && memory.contains(player, kLockByte + 1u))
        memory.store8(player + kLockByte, shortened);
    if (flight != anim) memory.store16(hud + kAnimHalf, flight);
    State &s = state();
    if (!s.trace || (lock == 0u && anim == 0u)) return;
    std::cout << "[item box] f=" << s.frame << " hud " << std::hex << hud << " player " << player << std::dec
              << " lock " << int(lock) << " -> " << int(shortened) << " anim " << anim << " -> " << flight
              << (on ? "" : " (off)") << std::endl;
}

void hud_update(psprecomp::Runtime &runtime, psprecomp::AllegrexContext &ctx) {
    const std::uint32_t hud = ctx.gpr[4];
    ctx.pc = kHudUpdate;
    state().original(runtime, ctx);
    apply_fast_box(runtime.memory(), hud, enabled());
}

} // namespace

std::span<const CodeWord> code_signature() {
    return kSignature;
}

bool code_matches(const psprecomp::GuestMemory &memory) {
    for (const CodeWord &expected : kSignature)
        if (!memory.contains(expected.address, 4u) || memory.load32(expected.address) != expected.word) return false;
    return true;
}

std::uint8_t shortened_lock(std::uint8_t lock) noexcept {
    return lock > 1u && lock <= kGameLock ? std::uint8_t{1u} : lock;
}

std::uint16_t shortened_anim(std::uint16_t anim) noexcept {
    return anim > 0u && anim <= kGameAnim ? std::uint16_t{0u} : anim;
}

void frame(psprecomp::Runtime &runtime) {
    State &s = state();
    static const bool trace = [] {
        const char *text = std::getenv("MHFU_TRACE_ITEM_BOX");
        return text != nullptr && *text != '\0' && std::string(text) != "0";
    }();
    s.trace = trace;
    ++s.frame;
    const psprecomp::Runtime::RecompiledFunction current = runtime.function_at(kHudUpdate);
    if (current == &hud_update) return;
    if (current == nullptr || !code_matches(runtime.memory())) return;
    s.original = current;
    runtime.register_function(kHudUpdate, &hud_update, "mhfu_fast_item_box");
    if (!s.reported) {
        s.reported = true;
        std::cout << "[item box] fast withdrawals in place" << std::endl;
    }
}

} // namespace mhfu::game::item_box
