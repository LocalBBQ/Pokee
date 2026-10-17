#include "game/exchange_rarity.hpp"

#include "psprecomp/runtime.hpp"
#include "settings/settings.hpp"

#include <array>
#include <cstdlib>
#include <iostream>
#include <string>

namespace mhfu::game::exchange_rarity {
namespace {

constexpr std::array<CodeWord, 12> kSignature = {{
    {kCanExchange, 0x27BDFFF0u}, // addiu sp, sp, -16
    {0x08829960u, 0x0E2099CDu},  // jal 0x08826734: the item id
    {0x08829964u, 0x92050586u},  // lbu a1, 1414(s0): the slot
    {0x08829978u, 0x3C02089Au},  // lui v0, 0x089A
    {0x0882997Cu, 0x2442D2BAu},  // addiu v0, v0, -11590: the table's byte 2
    {0x08829984u, 0x90420000u},  // lbu v0, 0(v0)
    {0x08829988u, 0x28420003u},  // slti v0, v0, 3
    {0x08816F88u, 0x0E20A64Au},  // jal 0x08829928: the menu entry
    {0x0881CF64u, 0x0E20A64Au},  // jal 0x08829928: the choice
    {0x0881D24Cu, 0x0E20A64Au},  // jal 0x08829928
    {0x08826744u, 0x0E215A21u},  // jal 0x08856884: the pouch slot
    {0x088568A0u, 0x3421AEA6u},  // ori at, at, 0xAEA6: the pouch
}};

struct State {
    psprecomp::Runtime::RecompiledFunction original{};
    bool reported{};
    bool trace{};
};

State &state() {
    static State s;
    return s;
}

void can_exchange(psprecomp::Runtime &runtime, psprecomp::AllegrexContext &ctx) {
    State &s = state();
    psprecomp::GuestMemory &memory = runtime.memory();
    const std::uint32_t hunter = ctx.gpr[4];
    const std::optional<std::uint32_t> rarity =
        settings::current().exchange_any_rarity ? rarity_address(memory, hunter) : std::nullopt;
    const std::uint8_t game_rarity = rarity ? memory.load8(*rarity) : std::uint8_t{0u};
    const bool lowered = rarity && game_rarity >= kGameLimit;
    if (lowered) memory.store8(*rarity, 0u);
    ctx.pc = kCanExchange;
    s.original(runtime, ctx);
    if (lowered) memory.store8(*rarity, game_rarity);
    if (s.trace)
        std::cout << "[exchange] hunter " << std::hex << hunter << std::dec << " rarity "
                  << (rarity ? std::to_string(game_rarity) : std::string("?")) << (lowered ? " (lifted)" : "") << " -> "
                  << ctx.gpr[2] << std::endl;
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

std::optional<std::uint32_t> rarity_address(const psprecomp::GuestMemory &memory, std::uint32_t hunter) {
    if (!memory.contains(hunter + kSelectedSlot, 1u) || !memory.contains(kGameSysPointer, 4u)) return std::nullopt;
    std::uint32_t slot = memory.load8(hunter + kSelectedSlot);
    if (slot >= kPouchSlots) slot = 0u;
    const std::uint32_t entry = memory.load32(kGameSysPointer) + kPouch + slot * 4u;
    if (!memory.contains(entry, 2u)) return std::nullopt;
    // 0x08826734 loads the id with lh and the check indexes the table with it.
    const std::int16_t id = static_cast<std::int16_t>(memory.load16(entry));
    if (id <= 0) return std::nullopt;
    const std::uint32_t address = kItemTable + static_cast<std::uint32_t>(id) * kItemRecord + kRarity;
    if (!memory.contains(address, 1u)) return std::nullopt;
    return address;
}

void frame(psprecomp::Runtime &runtime) {
    State &s = state();
    static const bool trace = [] {
        const char *text = std::getenv("MHFU_TRACE_EXCHANGE");
        return text != nullptr && *text != '\0' && std::string(text) != "0";
    }();
    s.trace = trace;
    const psprecomp::Runtime::RecompiledFunction current = runtime.function_at(kCanExchange);
    if (current == &can_exchange) return;
    if (current == nullptr || !code_matches(runtime.memory())) return;
    s.original = current;
    runtime.register_function(kCanExchange, &can_exchange, "mhfu_exchange_any_rarity");
    if (!s.reported) {
        s.reported = true;
        std::cout << "[exchange] any rarity in place" << std::endl;
    }
}

} // namespace mhfu::game::exchange_rarity
