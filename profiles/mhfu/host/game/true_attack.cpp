#include "game/true_attack.hpp"

#include "psprecomp/runtime.hpp"
#include "settings/settings.hpp"

#include <iostream>
#include <optional>

namespace mhfu::game::true_attack {
namespace {

constexpr std::array<CodeWord, 10> kSignature = {{
    {0x0881B094u, 0x3C020894u}, // lui v0, 0x0894
    {0x0881B098u, 0x2442ACDCu}, // addiu v0, v0, -21284: the factors
    {0x0881B0B0u, 0x3C0251EBu}, // lui v0, 0x51EB: then / 100
    {0x0881F144u, 0x2442ACDCu},
    {0x0881F1BCu, 0x2484ACDCu}, // addiu a0, a0, -21284
    {0x0881F244u, 0x2442ACDCu},
    {0x0881F390u, 0x2442ACDCu},
    {0x0881F464u, 0x2442ACDCu},
    {0x0881F5FCu, 0x2442ACDCu},
    {0x0881F608u, 0x00830018u}, // mult a0, v1: the attack by its factor
}};

bool table_is(const psprecomp::GuestMemory &memory, auto &&factor) {
    for (std::size_t i = 0; i < kClasses; ++i)
        if (memory.load32(kFactors + static_cast<std::uint32_t>(i) * 4u) != factor(i)) return false;
    return true;
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

bool apply(psprecomp::GuestMemory &memory, bool on) {
    if (!code_matches(memory) || !memory.contains(kFactors, kClasses * 4u)) return false;
    const auto game = [](std::size_t i) { return kGameFactors[i]; };
    const auto truth = [](std::size_t) { return kTrueFactor; };
    if (!table_is(memory, game) && !table_is(memory, truth)) return false;
    for (std::size_t i = 0; i < kClasses; ++i)
        memory.store32(kFactors + static_cast<std::uint32_t>(i) * 4u, on ? kTrueFactor : kGameFactors[i]);
    return true;
}

void frame(psprecomp::Runtime &runtime) {
    static std::optional<bool> applied;
    static bool reported{};
    const bool on = settings::current().true_attack;
    if (applied == on) return;
    if (apply(runtime.memory(), on)) {
        applied = on;
        std::cout << "[true-attack] " << (on ? "on" : "off") << std::endl;
    } else if (!reported) {
        reported = true;
        std::cout << "[true-attack] the factors are not the ones traced; left alone" << std::endl;
    }
}

} // namespace mhfu::game::true_attack
