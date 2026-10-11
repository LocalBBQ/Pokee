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
    {0x088518C8u, 0x9464003Eu},  // lhu a0,0x3E(v1): legs
    {0x08851920u, 0x9465004Au},  // lhu a1,0x4A(v1): head
    {0x0885197Cu, 0x94640056u},  // lhu a0,0x56(v1): chest
    {0x088519CCu, 0x94640062u},  // lhu a0,0x62(v1): arms
    {0x08851A1Cu, 0x9464006Eu},  // lhu a0,0x6E(v1): waist
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

// 0x088BC7F0: the frame, the hunter in s1, and the task word it dispatches.
constexpr CodeWord kTaskSignature[] = {
    {0x088BC7F0u, 0x27BDEE20u},  // addiu sp,sp,-4576
    {0x088BC7F4u, 0xAFBF002Cu},  // sw ra,44(sp)
    {0x088BC80Cu, 0x00808821u},  // addu s1,a0,zero
    {0x088BC874u, 0x8E22000Cu},  // lw v0,12(s1): the task
};

// 0x08851C3C: the stack frame, the index shifted, and the call that remaps the
// part before the id is read. 0x088516A4: the part masked and checked below 6.
constexpr CodeWord kPresentSignature[] = {
    {0x08851C3Cu, 0x27BDFFF0u},  // addiu sp,sp,-16
    {0x08851C40u, 0x000510C0u},  // sll v0,a1,3
    {0x08851C74u, 0x0E215B45u},  // jal 0x08856D14
};
constexpr CodeWord kBySexSignature[] = {
    {0x088516A4u, 0x30C300FFu},  // andi v1,a2,0xff
    {0x088516A8u, 0x2C610006u},  // sltiu at,v1,6
    {0x088516ACu, 0x10200071u},  // beqz at
};
// 0x088D6388: the part masked, checked below 6, and the model sign-extended
// into a1. A nonzero model is returned unchanged.
constexpr CodeWord kResolveSignature[] = {
    {0x088D6388u, 0x30A200FFu},  // andi v0,a1,0xff
    {0x088D638Cu, 0x2C410006u},  // sltiu at,v0,6
    {0x088D6394u, 0x7C062E20u},  // seh a1,a2
};

// 0x088DBBBC loads one part's file. lobby_task calls it; the load step calls
// it too. It asks the model lookup at 0x088DBC64. A different executable does
// not get the wrapper.
constexpr std::uint32_t kFileLoad = 0x088DBBBCu;
// 0x088DB4C8 jumps here. a3 is the part, a2 the sex, t0 the model the read
// uses. Replacing the model 0x088D6388 returns to this function faulted the
// reader at 0x249D5510; the model has to arrive in t0, and the function then
// runs as the game wrote it.
// 0x08851A68: a1 is the part, and it must be below 6. The equipment change
// stores v0 and passes it to 0x08866E10 as t0 (return 0x09A7DF70).
constexpr std::uint32_t kPartModel = 0x08851A68u;
constexpr CodeWord kPartModelSignature[] = {
    {0x08851A68u, 0x2CA10006u},  // sltiu at,a1,6
    {0x08851A6Cu, 0x248304A0u},  // addiu v1,a0,1184
    {0x08851A70u, 0x10200070u},  // beq at,zero
};
constexpr std::uint32_t kResidentLoad = 0x08866E10u;
constexpr CodeWord kResidentSignature[] = {
    {0x08866E10u, 0x27BDFFE0u},  // addiu sp,sp,-32
    {0x08866E14u, 0xAFBF001Cu},  // sw ra,28(sp)
    {0x08866EA0u, 0x0E2358E2u},  // jal 0x088D6388
};
constexpr CodeWord kFileSignature[] = {
    {0x088DBBBCu, 0x27BDFFD0u},  // addiu sp,sp,-48
    {0x088DBBC0u, 0xAFBF002Cu},  // sw ra,44(sp)
    {0x088DBC64u, 0x0E21461Fu},  // jal 0x0885187C
};

struct State {
    bool checked{};
    bool usable{};
    bool installed{};
    bool trace{};
    const Compiled *original{};
    const Compiled *step{};
    bool step_installed{};
    const Compiled *task{};
    bool task_installed{};
    bool present_ok{};
    bool by_sex_ok{};
    bool present_installed{};
    bool by_sex_installed{};
    bool resolve_ok{};
    bool resolve_installed{};
    // The settings as of the last flip; the menu changes them between frames.
    bool on{};
    Pieces pieces{kReal, kReal, kReal, kReal, kReal};
    // The look last announced. The equipment box is what reloads a part, so a
    // change here only tells the next box change which models to read.
    bool look_known{};
    bool look_on{};
    Pieces look_pieces{kReal, kReal, kReal, kReal, kReal};
    const Compiled *present_fn{};
    const Compiled *by_sex_fn{};
    const Compiled *resolve_fn{};
    psprecomp::Runtime::RecompiledFunction file_fn{};
    bool file_installed{};
    psprecomp::Runtime::RecompiledFunction resident_fn{};
    bool resident_installed{};
    psprecomp::Runtime::RecompiledFunction part_model_fn{};
    bool part_model_installed{};
};
State &state() {
    static State s;
    return s;
}

void note_call(const char *what, const psprecomp::AllegrexContext &ctx) {
    State &s = state();
    if (!s.trace) return;
    // The model lookup and the hunter task run every frame. A repeated return
    // address is the same caller, so only a change of caller is logged. The
    // load step and the file load are logged whenever the game calls them.
    if (what[0] == 'l' && what[1] == 'o' || what[0] == 't') {
        static std::uint32_t last_lookup = 0u;
        static std::uint32_t last_task = 0u;
        std::uint32_t &last = what[0] == 't' ? last_task : last_lookup;
        if (ctx.gpr[31] == last) return;
        last = ctx.gpr[31];
    }
    std::cout << "[layered] call " << what << " ra=0x" << std::hex << ctx.gpr[31] << " a0=0x" << ctx.gpr[4]
              << " a1=0x" << ctx.gpr[5] << " a2=0x" << ctx.gpr[6] << std::dec << "\n";
}

void model_of_part(psprecomp::Runtime &runtime, psprecomp::AllegrexContext &ctx) {
    State &s = state();
    note_call("lookup", ctx);
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

// The file load asks this before it picks a file. A chosen piece is armor, so
// a helm is read from the helm files even when the real head is empty.
void has_armor(psprecomp::Runtime &runtime, psprecomp::AllegrexContext &ctx) {
    State &s = state();
    if (s.on) {
        const GuestRam ram(runtime.memory());
        const std::optional<bool> present = armor_present(ram, ctx.gpr[4], ctx.gpr[5], ctx.gpr[6], s.pieces);
        if (present) {
            ctx.gpr[2] = *present ? 1u : 0u;
            ctx.pc = ctx.gpr[31];
            return;
        }
    }
    if (s.present_fn != nullptr) s.present_fn->function(runtime, ctx);
}

// The guild, the lobby and the equipment change ask for a model by sex and
// part. Legs, chest, arms and waist return a model whatever t0 is, and the
// equipment change (return 0x09A7E360) loads that result even when t0 is 0.
// Answering only when t0 is 1 let that change draw the newly worn armor.
void answer_model(psprecomp::AllegrexContext &ctx, std::uint16_t model) {
    ctx.gpr[2] = model;
    ctx.pc = ctx.gpr[31];
}

void model_by_sex(psprecomp::Runtime &runtime, psprecomp::AllegrexContext &ctx) {
    State &s = state();
    // a2 is the part, a1 the sex. The equipment change stores v0 and later
    // passes it to the file read as t0. The box is mid-update, so this does
    // not ask owns_piece: the menu already limited the choice to a worn or
    // boxed piece. a1 is the sex the file table is indexed by.
    if (s.on && equipment_sex_lookup(ctx.gpr[31]) && sex_lookup_returns_model(ctx.gpr[6])) {
        const GuestRam ram(runtime.memory());
        if (const std::optional<std::uint16_t> model = chosen_model(ram, ctx.gpr[6], ctx.gpr[5], s.pieces)) {
            answer_model(ctx, *model);
            return;
        }
    }
    if (s.by_sex_fn != nullptr) s.by_sex_fn->function(runtime, ctx);
}

// a1 is the part. The equipment change copies v0 into t0 of the file read
// at 0x09A7DF70. While the switch is on, that value is the chosen model.
void part_model(psprecomp::Runtime &runtime, psprecomp::AllegrexContext &ctx) {
    State &s = state();
    std::optional<std::uint16_t> model;
    if (s.on && sex_lookup_returns_model(ctx.gpr[5])) {
        const GuestRam ram(runtime.memory());
        if (ram.contains(kGameBlockPointer, 4u) && ram.load32(kGameBlockPointer) == ctx.gpr[4]) {
            if (const std::optional<std::uint8_t> sex = played_sex(ram))
                model = chosen_model(ram, ctx.gpr[5], *sex, s.pieces);
        }
    }
    if (model) {
        answer_model(ctx, *model);
        return;
    }
    if (s.part_model_fn != nullptr) s.part_model_fn(runtime, ctx);
}

// The equipment change's file read. a3 is the part and a2 the sex the file
// table is indexed by. The character load uses this function too, and its t0
// is already the model 0x0885187C returned; replacing that one faulted at
// 0x24AB8540.
void resident_load(psprecomp::Runtime &runtime, psprecomp::AllegrexContext &ctx) {
    State &s = state();
    if (s.on && equipment_model_read(ctx.gpr[31]) && sex_lookup_returns_model(ctx.gpr[7])) {
        const GuestRam ram(runtime.memory());
        if (const std::optional<std::uint16_t> model = chosen_model(ram, ctx.gpr[7], ctx.gpr[6], s.pieces))
            ctx.gpr[8] = *model;
    }
    // t3 == 0 selects the hair file for the head, and that read then returns
    // no file. A chosen piece is armor, so the helm model in t0 is read from
    // the helm files. The character load keeps the model it already has.
    if (s.on && sex_lookup_returns_model(ctx.gpr[7]) &&
        (ctx.gpr[31] == 0x09A7DF70u || ctx.gpr[31] == 0x088DBCCCu)) {
        const GuestRam ram(runtime.memory());
        if (const std::optional<std::uint8_t> kind = armor_kind(ctx.gpr[7]); kind && s.pieces[*kind] > 0) {
            if (const std::optional<std::uint16_t> model = chosen_model(ram, ctx.gpr[7], ctx.gpr[6], s.pieces)) {
                if (ctx.gpr[31] == 0x09A7DF70u || ctx.gpr[8] == *model) ctx.gpr[11] = 1u;
            }
        }
    }
    if (s.resident_fn != nullptr) s.resident_fn(runtime, ctx);
}

// lobby_task calls this with the model 0x088516A4 just returned. Those
// lobby returns get the chosen model, so the file id is the chosen piece's.
// The resident file load (0x08866E10, which 0x088DB4C8 jumps to) keeps the
// game's model: returning a chosen one there faulted at 0x249D5510.
void resolve_model(psprecomp::Runtime &runtime, psprecomp::AllegrexContext &ctx) {
    State &s = state();
    const std::uint32_t ret = ctx.gpr[31];
    if (s.on && substitute_resolved_model(ret)) {
        const GuestRam ram(runtime.memory());
        const std::optional<std::uint8_t> sex = played_sex(ram);
        if (sex) {
            const std::optional<std::uint16_t> model = chosen_model(ram, ctx.gpr[5], *sex, s.pieces);
            if (model) {
                answer_model(ctx, *model);
                return;
            }
        }
    }
    if (s.resolve_fn != nullptr) s.resolve_fn->function(runtime, ctx);
}

// One pass of the game's step, entered at its first instruction. The driver
// calls it once; a pass that chains out to the dispatcher leaves ctx.pc
// elsewhere and must be left there.
void call_step(psprecomp::Runtime &runtime, psprecomp::AllegrexContext &ctx, const Compiled *step,
               std::uint32_t object, std::uint32_t ret) {
    ctx.pc = kLoadStep;
    ctx.gpr[4] = object;
    ctx.gpr[31] = ret;
    step->function(runtime, ctx);
}

void load_step(psprecomp::Runtime &runtime, psprecomp::AllegrexContext &ctx) {
    State &s = state();
    note_call("load", ctx);
    // The game is already in this step. Setting +0x998 to 0 here runs
    // 0x088DDBD8, which drops the face, and the part files are not read again
    // while their own state is still finished. The flip releases those files;
    // this call only does what the game asked.
    const std::uint32_t object = ctx.gpr[4];
    const std::uint32_t ret = ctx.gpr[31];
    call_step(runtime, ctx, s.step, object, ret);
}

// The load step calls this for one part (return 0x088C1088). a0 is not the
// hunter, and a quest reaches the lookup inside this function without
// entering here, so this does not restart the load. The host does not call it.
void file_load(psprecomp::Runtime &runtime, psprecomp::AllegrexContext &ctx) {
    State &s = state();
    note_call("file", ctx);
    if (s.file_fn != nullptr) s.file_fn(runtime, ctx);
}

// cont_task runs this while a place is loading the hunter, then lobby_task
// replaces it. A quest does not call this. The task word is left alone:
// sending it back to 2 with +0x998 at 0 drops the face.
void hunter_task(psprecomp::Runtime &runtime, psprecomp::AllegrexContext &ctx) {
    State &s = state();
    note_call("task", ctx);
    const std::uint32_t object = ctx.gpr[4];
    const std::uint32_t ret = ctx.gpr[31];
    ctx.pc = kHunterTask;
    ctx.gpr[4] = object;
    ctx.gpr[31] = ret;
    s.task->function(runtime, ctx);
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
    s.task = compiled(kHunterTask);
    if (s.task == nullptr) {
        std::cerr << "[layered] the hunter task is not in the generated code; a new look shows after a restart\n";
    } else {
        for (const CodeWord &expected : kTaskSignature) {
            const std::uint32_t found = runtime.memory().load32(expected.address);
            if (found == expected.word) continue;
            std::cerr << "[layered] game code differs at 0x" << std::hex << expected.address << ": 0x" << found
                      << ", expected 0x" << expected.word << std::dec
                      << "; a new look shows after a restart\n";
            s.task = nullptr;
            break;
        }
    }
    s.present_fn = compiled(kHasArmor);
    s.present_ok = s.present_fn != nullptr;
    for (const CodeWord &expected : kPresentSignature)
        if (runtime.memory().load32(expected.address) != expected.word) s.present_ok = false;
    if (!s.present_ok)
        std::cerr << "[layered] the armor check is not as traced; an empty head stays the hair\n";
    s.by_sex_fn = compiled(kModelBySex);
    s.by_sex_ok = s.by_sex_fn != nullptr;
    for (const CodeWord &expected : kBySexSignature)
        if (runtime.memory().load32(expected.address) != expected.word) s.by_sex_ok = false;
    if (!s.by_sex_ok)
        std::cerr << "[layered] the lobby model lookup is not as traced; a guild hunter keeps its real armor\n";
    s.resolve_fn = compiled(kResolveModel);
    s.resolve_ok = s.resolve_fn != nullptr;
    for (const CodeWord &expected : kResolveSignature)
        if (runtime.memory().load32(expected.address) != expected.word) s.resolve_ok = false;
    if (!s.resolve_ok)
        std::cerr << "[layered] the equipment model resolve is not as traced; changing armor draws the worn piece\n";
    s.file_fn = runtime.function_at(kFileLoad);
    bool file_ok = s.file_fn != nullptr;
    for (const CodeWord &expected : kFileSignature)
        if (runtime.memory().load32(expected.address) != expected.word) file_ok = false;
    if (!file_ok) {
        s.file_fn = nullptr;
        std::cerr << "[layered] the part file load is not as traced; an area keeps the meshes it has\n";
    }
    s.resident_fn = runtime.function_at(kResidentLoad);
    bool resident_ok = s.resident_fn != nullptr;
    for (const CodeWord &expected : kResidentSignature)
        if (runtime.memory().load32(expected.address) != expected.word) resident_ok = false;
    if (!resident_ok) {
        s.resident_fn = nullptr;
        std::cerr << "[layered] the resident file load is not as traced; changing armor draws the worn piece\n";
    }
    s.part_model_fn = runtime.function_at(kPartModel);
    bool part_ok = s.part_model_fn != nullptr;
    for (const CodeWord &expected : kPartModelSignature)
        if (runtime.memory().load32(expected.address) != expected.word) part_ok = false;
    if (!part_ok) {
        s.part_model_fn = nullptr;
        std::cerr << "[layered] the equipment part lookup is not as traced; changing armor draws the worn piece\n";
    }
    return true;
}

} // namespace

void frame(psprecomp::Runtime &runtime) {
    State &s = state();
    const settings::Settings &settings = settings::current();
    s.on = settings.layered_armor;
    s.pieces = settings.layered_pieces;
    // Off from the start: nothing is registered, and the game runs exactly as
    // it was built. The equipment hooks, once installed, stay and follow the
    // switch, so turning it off and on in one session needs no second install.
    if (!s.on && !s.installed) {
        s.look_known = true;
        s.look_on = false;
        s.look_pieces = s.pieces;
        return;
    }
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
        // Stays in place after the switch is turned off, so the real armor loads.
        if (s.task != nullptr && !s.task_installed) {
            runtime.register_function(kHunterTask, &hunter_task, "mhfu_layered_armor_task");
            s.task_installed = true;
        }
        if (s.present_ok && !s.present_installed) {
            runtime.register_function(kHasArmor, &has_armor, "mhfu_layered_armor_present");
            s.present_installed = true;
        }
        if (s.by_sex_ok && !s.by_sex_installed) {
            runtime.register_function(kModelBySex, &model_by_sex, "mhfu_layered_armor_by_sex");
            s.by_sex_installed = true;
        }
        // Stays in place after the switch is turned off. While off, the
        // function runs the game's own code, so the worn piece is drawn.
        if (s.resolve_ok && !s.resolve_installed) {
            runtime.register_function(kResolveModel, &resolve_model, "mhfu_layered_armor_resolve");
            s.resolve_installed = true;
        }
        // Stays in place so a load the game runs after the switch is still seen.
        // It only calls the game's own function.
        if (s.file_fn != nullptr && !s.file_installed) {
            runtime.register_function(kFileLoad, &file_load, "mhfu_layered_armor_file");
            s.file_installed = true;
        }
        // Stays in place. While the switch is off it runs the game's function,
        // so the worn piece is the model the read opens.
        if (s.resident_fn != nullptr && !s.resident_installed) {
            runtime.register_function(kResidentLoad, &resident_load, "mhfu_layered_armor_resident");
            s.resident_installed = true;
        }
        // Stays in place. While the switch is off it runs the game's function,
        // so the equipment change loads the worn piece.
        if (s.part_model_fn != nullptr && !s.part_model_installed) {
            runtime.register_function(kPartModel, &part_model, "mhfu_layered_armor_part");
            s.part_model_installed = true;
        }
        std::cout << "[layered] on\n";
    } else if (!s.on && s.installed) {
        // The game's own lookups again, as the generated code registered them.
        runtime.register_function(kModelOfPart, s.original->function, s.original->name);
        if (s.present_installed && s.present_fn != nullptr) {
            runtime.register_function(kHasArmor, s.present_fn->function, s.present_fn->name);
            s.present_installed = false;
        }
        // The equipment change keeps the sex lookup, the part lookup, the
        // resolve and the resident load. Each runs the game's own function
        // while the switch is off, and putting the generated unit back drops
        // the next change.
        s.installed = false;
        // The hunter stays as it is until the next change of armor in the
        // box. That change reads the worn piece, because the lookups above
        // run the game's own function while the switch is off.
        std::cout << "[layered] off\n";
    }
    // A change of the switch or of the chosen pieces releases the part files
    // once. The equipment change reads its model from 0x08851A68 and passes
    // that to 0x08866E10. Replacing t0 on the character load faulted at
    // 0x24AB8540, and replacing the value 0x088D6388 returns on that read
    // faulted at 0x249D5510. The first flip only records the look: the
    // hunter's own load is already going to read it.
    GuestRam ram(runtime.memory());
    if (!s.look_known) {
        s.look_known = true;
        s.look_on = s.on;
        s.look_pieces = s.pieces;
    } else if ((s.on != s.look_on || s.pieces != s.look_pieces) && request_model_load(ram)) {
        s.look_on = s.on;
        s.look_pieces = s.pieces;
        std::cout << "[layered] the look changed: the next load reads these models\n";
    }
}

bool installed() { return state().installed; }

} // namespace mhfu::game::layered
