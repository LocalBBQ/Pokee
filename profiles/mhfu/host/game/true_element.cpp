#include "game/true_element.hpp"

#include "game/hooks.hpp"
#include "psprecomp/runtime.hpp"
#include "settings/settings.hpp"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

namespace mhfu::game::true_element {
namespace {

constexpr std::array<CodeWord, 6> kSignature = {{
    {kSprintf, 0x27BDFF70u},    // addiu sp, sp, -144
    {0x0880E918u, 0x00A08021u}, // addu s0, a1, zero: the format
    {0x0880E954u, 0x02003021u}, // addu a2, s0, zero
    {0x0880E958u, 0x0E203DA3u}, // jal 0x0880F68C: the formatter
    {0x08823784u, 0x0E203A3Cu}, // jal 0x0880E8F0: an element value
    {0x08823858u, 0x0E203A3Cu}, // jal 0x0880E8F0
}};

// A format the wrapper changed, and the game's bytes to put back.
struct Changed {
    std::uint32_t address;
    std::array<std::uint8_t, 4> game;
    std::array<std::uint8_t, 4> ours;
};

struct State {
    psprecomp::Runtime::RecompiledFunction original{};
    bool reported{};
    bool trace{};
    std::vector<Changed> changed;
};

State &state() {
    static State s;
    return s;
}

bool is_digit(std::uint8_t c) {
    return c >= '0' && c <= '9';
}

std::array<std::uint8_t, 4> read4(const psprecomp::GuestMemory &memory, std::uint32_t address) {
    std::array<std::uint8_t, 4> bytes{};
    for (std::uint32_t i = 0; i < 4u; ++i) bytes[i] = memory.load8(address + i);
    return bytes;
}

void write4(psprecomp::GuestMemory &memory, std::uint32_t address, const std::array<std::uint8_t, 4> &bytes) {
    for (std::uint32_t i = 0; i < 4u; ++i) memory.store8(address + i, bytes[i]);
}

// The game's text at `address`, for the trace.
std::string guest_text(const psprecomp::GuestMemory &memory, std::uint32_t address) {
    std::string text;
    for (std::uint32_t i = 0; i < kLongestFormat && memory.contains(address + i, 1u); ++i) {
        const std::uint8_t c = memory.load8(address + i);
        if (c == 0u) break;
        text.push_back(c >= 0x20u && c < 0x7Fu ? static_cast<char>(c) : '?');
    }
    return text;
}

// sprintf does not always run to its end in one call here: it can stop at a
// kernel call (the C library's lock) and finish from the dispatcher later.
// So the format is changed before it runs and stays changed; frame() puts
// the game's bytes back when the switch is turned off.
void sprintf_wrapper(psprecomp::Runtime &runtime, psprecomp::AllegrexContext &ctx) {
    State &s = state();
    psprecomp::GuestMemory &memory = runtime.memory();
    const std::uint32_t format = ctx.gpr[5];
    if (settings::current().true_attack) {
        if (const std::optional<Rewrite> rewrite = rewrite_for(memory, format)) {
            const std::string asked = s.trace ? guest_text(memory, format) : std::string();
            const Changed change{rewrite->address, read4(memory, rewrite->address), rewrite->bytes};
            write4(memory, change.address, change.ours);
            std::erase_if(s.changed, [&](const Changed &c) { return c.address == change.address; });
            s.changed.push_back(change);
            if (s.trace)
                std::cout << "[element] from " << std::hex << ctx.gpr[31] << " format " << format << std::dec
                          << " \"" << asked << "\" -> \"" << guest_text(memory, format) << "\"" << std::endl;
        }
    }
    ctx.pc = kSprintf;
    s.original(runtime, ctx);
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

std::optional<Rewrite> rewrite_for(const psprecomp::GuestMemory &memory, std::uint32_t format) {
    std::uint32_t length = 0u;
    for (;; ++length) {
        if (length == kLongestFormat || !memory.contains(format + length, 1u)) return std::nullopt;
        if (memory.load8(format + length) == 0u) break;
    }
    // "%d0" or "%<width>d0", and nothing after it.
    if (length < 3u || memory.load8(format + length - 1u) != '0' || memory.load8(format + length - 2u) != 'd')
        return std::nullopt;
    const std::uint8_t before = memory.load8(format + length - 3u);
    if (before == '%') {
        // A "%%d0" prints "%d0"; it is no number.
        if (length >= 4u && memory.load8(format + length - 4u) == '%') return std::nullopt;
        return Rewrite{format + length - 3u, {'%', 'd', 0u, 0u}};
    }
    if (!is_digit(before) || before == '9' || length < 4u || memory.load8(format + length - 4u) != '%')
        return std::nullopt;
    if (length >= 5u && memory.load8(format + length - 5u) == '%') return std::nullopt;
    return Rewrite{format + length - 4u, {'%', static_cast<std::uint8_t>(before + 1u), 'd', 0u}};
}

void frame(psprecomp::Runtime &runtime) {
    State &s = state();
    static const bool trace = [] {
        const char *text = std::getenv("MHFU_TRACE_ELEMENT");
        return text != nullptr && *text != '\0' && std::string(text) != "0";
    }();
    s.trace = trace;
    psprecomp::GuestMemory &memory = runtime.memory();
    if (!settings::current().true_attack && !s.changed.empty()) {
        // Text the game loaded over a change since is the game's already.
        for (const Changed &change : s.changed)
            if (memory.contains(change.address, 4u) && read4(memory, change.address) == change.ours)
                write4(memory, change.address, change.game);
        s.changed.clear();
        if (s.trace) std::cout << "[element] formats put back" << std::endl;
    }
    // A wrapper there takes the C library's code off the generated units'
    // direct calls, so it waits until the switch is first turned on.
    if (!s.reported && !settings::current().true_attack) return;
    if (!code_matches(memory)) return;
    if (!hooks::wrap(runtime, kSprintf, &sprintf_wrapper, s.original, "mhfu_true_element")) return;
    if (!s.reported) {
        s.reported = true;
        std::cout << "[element] true element in place" << std::endl;
    }
}

} // namespace mhfu::game::true_element
