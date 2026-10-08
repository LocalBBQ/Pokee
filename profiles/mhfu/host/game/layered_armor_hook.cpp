#include "game/layered_armor.hpp"

#include "game/guest_ram.hpp"
#include "settings/settings.hpp"

#include "psprecomp/runtime.hpp"

#include <cstdlib>
#include <iostream>
#include <string>

// The generated unit the game's model lookup was compiled into, found by CMake
// in the corpus (MHFU_LAYERED_FUNCTIONS in CMakeLists.txt).
#if defined(MHFU_LAYERED_FUNCTIONS_INC)
namespace psprecomp {
#define MHFU_LAYERED_FUNCTION(address, unit) void unit(Runtime &, AllegrexContext &);
#include MHFU_LAYERED_FUNCTIONS_INC
#undef MHFU_LAYERED_FUNCTION
} // namespace psprecomp
#endif

namespace mhfu::game::layered {
namespace {

struct Compiled {
    std::uint32_t address;
    psprecomp::Runtime::RecompiledFunction function;
    const char *name;
};
constexpr Compiled kCompiled[] = {
#if defined(MHFU_LAYERED_FUNCTIONS_INC)
#define MHFU_LAYERED_FUNCTION(address, unit) {address, &psprecomp::unit, #unit},
#include MHFU_LAYERED_FUNCTIONS_INC
#undef MHFU_LAYERED_FUNCTION
#endif
    {0u, nullptr, nullptr},
};

const Compiled *compiled(std::uint32_t address) {
    for (const Compiled &entry : kCompiled)
        if (entry.address == address && entry.function != nullptr) return &entry;
    return nullptr;
}

// The first instructions of the lookup, as read from the executable: the
// record's address from a1 (a1 * 140 + 0x30), the check that the part is
// below 6, the sex byte, and one id load per armor part. A different
// executable does not get the replacement.
struct CodeWord {
    std::uint32_t address;
    std::uint32_t word;
};
constexpr CodeWord kSignature[] = {
    {0x0885187Cu, 0x000518C0u},  // sll v1,a1,3
    {0x08851880u, 0x00652823u},  // subu a1,v1,a1
    {0x08851884u, 0x00051880u},  // sll v1,a1,2
    {0x08851888u, 0x00A31821u},  // addu v1,a1,v1
    {0x0885188Cu, 0x00031880u},  // sll v1,v1,2
    {0x08851890u, 0x00831821u},  // addu v1,a0,v1          a1 * 140 + block
    {0x08851894u, 0x2CC10006u},  // sltiu at,a2,6
    {0x088518A0u, 0x24630030u},  // addiu v1,v1,0x30
    {0x088518C0u, 0x90620010u},  // lbu v0,0x10(v1): sex, 0 male
    {0x088518C8u, 0x9464003Eu},  // lhu a0,0x3E(v1): chest
    {0x08851920u, 0x9465004Au},  // lhu a1,0x4A(v1): arms
    {0x0885197Cu, 0x94640056u},  // lhu a0,0x56(v1): waist
    {0x088519CCu, 0x94640062u},  // lhu a0,0x62(v1): legs
    {0x08851A1Cu, 0x9464006Eu},  // lhu a0,0x6E(v1): head
};

// What starting the hunter's load over relies on: the step, the state it
// reports done at, and the game's own request that clears the state.
constexpr CodeWord kLoadSignature[] = {
    {0x088C0E58u, 0x27BDFFE0u},  // addiu sp,sp,-0x20
    {0x088C0E70u, 0x8C820998u},  // lw v0,0x998(a0): the state
    {0x088C0EA0u, 0xAE020998u},  // sw v0,0x998(s0): the first step stores the next state
    {0x088C1150u, 0x8E020998u},  // lw v0,0x998(s0)
    {0x088C1154u, 0x38420005u},  // xori v0,v0,5: done when the state is 5
    {0x088C1158u, 0x2C420001u},  // sltiu v0,v0,1
    {0x088BCBB4u, 0xAE20099Cu},  // sw zero,0x99C(s1): the game's request
    {0x088BCBB8u, 0xAE200998u},  // sw zero,0x998(s1)
    {0x088BCBFCu, 0x0E230396u},  // jal 0x088C0E58: the driver steps it
};

struct State {
    bool checked{};
    bool usable{};
    bool installed{};
    bool trace{};
    const Compiled *original{};
    const Compiled *step{};
    bool step_installed{};
    // The settings as of the last flip; the menu changes them between frames.
    bool on{};
    Pieces pieces{kReal, kReal, kReal, kReal, kReal};
    // The look the hunter played here was last loaded with: the one at the
    // first flip, then the one of each load the game ran on it.
    bool loaded_known{};
    Pieces loaded{kReal, kReal, kReal, kReal, kReal};
};
State &state() {
    static State s;
    return s;
}

void model_of_part(psprecomp::Runtime &runtime, psprecomp::AllegrexContext &ctx) {
    State &s = state();
    if (s.on) {
        const GuestRam ram(runtime.memory());
        const std::optional<std::uint16_t> model = replacement_model(ram, ctx.gpr[4], ctx.gpr[5], ctx.gpr[6], s.pieces);
        if (model) {
            if (s.trace) {
                const std::optional<std::uint8_t> kind = armor_kind(ctx.gpr[6]);
                std::cout << "[layered] " << (kind ? part_label(*kind) : "?") << ": model " << *model
                          << " instead of the worn piece's (hunter " << ctx.gpr[5] << ", from 0x" << std::hex
                          << ctx.gpr[31] << std::dec << ")\n";
            }
            // What the game's function leaves for its caller: the model in v0,
            // back to the return address. It only reads memory.
            ctx.gpr[2] = *model;
            ctx.pc = ctx.gpr[31];
            return;
        }
    }
    s.original->function(runtime, ctx);
}

// The look the hunter is to be drawn in; the real one while off.
Pieces wanted(const State &s) { return s.on ? s.pieces : Pieces{kReal, kReal, kReal, kReal, kReal}; }

void load_step(psprecomp::Runtime &runtime, psprecomp::AllegrexContext &ctx) {
    State &s = state();
    GuestRam ram(runtime.memory());
    const std::uint32_t object = ctx.gpr[4];
    if (own_hunter(ram, object)) {
        const Pieces look = wanted(s);
        if (!load_done(ram, object)) {
            // A load the game started: it asks for the files with the look as
            // it is now.
            s.loaded = look;
        } else if (s.loaded != look) {
            restart_load(ram, object);
            s.loaded = look;
            if (s.trace) std::cout << "[layered] the look changed: the hunter loads its models again\n";
        }
    }
    s.step->function(runtime, ctx);
}

bool check(psprecomp::Runtime &runtime) {
    State &s = state();
    s.original = compiled(kModelOfPart);
    if (s.original == nullptr) {
        std::cerr << "[layered] the game's armor model lookup is not in the generated code; layered armor "
                     "unavailable\n";
        return false;
    }
    for (const CodeWord &expected : kSignature) {
        const std::uint32_t found = runtime.memory().load32(expected.address);
        if (found == expected.word) continue;
        std::cerr << "[layered] game code differs at 0x" << std::hex << expected.address << ": 0x" << found
                  << ", expected 0x" << expected.word << std::dec << "; layered armor unavailable\n";
        return false;
    }
    s.step = compiled(kLoadStep);
    for (const CodeWord &expected : kLoadSignature)
        if (runtime.memory().load32(expected.address) != expected.word) s.step = nullptr;
    if (s.step == nullptr)
        std::cerr << "[layered] the hunter's load step is not as traced; a new look shows after a restart\n";
    return true;
}

} // namespace

void frame(psprecomp::Runtime &runtime) {
    State &s = state();
    const settings::Settings &settings = settings::current();
    s.on = settings.layered_armor;
    s.pieces = settings.layered_pieces;
    // The look the game starts with: the hunter's first load asks for it.
    if (!s.loaded_known) {
        s.loaded_known = true;
        s.loaded = wanted(s);
    }
    // Off from the start: nothing is registered, and the game runs exactly as
    // it was built.
    if (!s.on && !s.installed) return;
    if (!s.checked) {
        s.checked = true;
        const char *text = std::getenv("MHFU_TRACE_LAYERED_ARMOR");
        s.trace = text != nullptr && *text != '\0' && std::string(text) != "0";
        s.usable = check(runtime);
    }
    if (!s.usable) return;
    // Only ever at the flip, from an import: no generated code is running, so
    // the dispatch tables can change here.
    if (s.on && !s.installed) {
        runtime.register_function(kModelOfPart, &model_of_part, "mhfu_layered_armor");
        s.installed = true;
        // Stays once in place, to show the real look again after turning off.
        if (s.step != nullptr && !s.step_installed) {
            runtime.register_function(kLoadStep, &load_step, "mhfu_layered_armor_load");
            s.step_installed = true;
        }
        std::cout << "[layered] on\n";
    } else if (!s.on && s.installed) {
        // The game's own lookup again, as the generated code registered it.
        runtime.register_function(kModelOfPart, s.original->function, s.original->name);
        s.installed = false;
        std::cout << "[layered] off\n";
    }
}

bool installed() { return state().installed; }

} // namespace mhfu::game::layered
