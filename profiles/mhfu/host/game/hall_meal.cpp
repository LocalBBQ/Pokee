#include "game/hall_meal.hpp"

#include "game/guest_layout.hpp"
#include "game/hooks.hpp"
#include "psprecomp/runtime.hpp"
#include "settings/settings.hpp"

#include <cstdlib>
#include <iostream>
#include <string>

namespace mhfu::game::hall_meal {
namespace {

constexpr std::array<CodeWord, 51> kSignature = {{
    {0x09AAAAD0u, 0x0E6AB02Au}, // jal 0x09AAC0A8: the seated frame
    {0x09AAAAD4u, 0x24050010u}, // addiu a1, zero, 16
    {kSeatedUpdate, 0x27BDFFF0u},
    {0x09AAC0B8u, 0x908601D5u}, // lbu a2, 0x1D5(a0): the phase
    {0x09AAC1A8u, 0x8C64C558u}, // lw a0, GameSys
    {0x09AAC1ACu, 0x962501E6u}, // lhu a1, 0x1E6(s1): the hunter's console
    {0x09AAC1B0u, 0x90840028u}, // lbu a0, 0x28(a0): this console
    {0x09AAC1CCu, 0x8C632E70u}, // lw v1, 0x2E70(v1): the lobby's state
    {0x09AAC1D0u, 0x8C630000u}, // lw v1, 0(v1): its mode
    {0x09AAC1DCu, 0x8E240484u}, // lw a0, 0x484(s1): the new buttons
    {0x09AAC1E0u, 0x30838000u}, // andi v1, a0, 0x8000: □ stands up
    {kSound, 0x27BDFFE0u},
    {0x09A7BB54u, 0x2CA10014u}, // sltiu at, a1, 20
    {0x09A71C84u, 0x0E70730Eu}, // jal 0x09C1CC38: the menu's frame
    {0x09A71C88u, 0x8C44B998u}, // lw a0, kitchen
    {0x09A71C90u, 0x54430018u}, // bnel v0, v1: 1 is a meal
    {0x09A71CC4u, 0x0E7078FCu}, // jal 0x09C1E3F0
    {0x09A71CDCu, 0x0E707728u}, // jal 0x09C1DCA0
    {kMenuOpen, 0x27BDFFF0u},
    {0x09C1CBFCu, 0xAC800070u}, // sw zero, 0x70(a0)
    {kMenuFrame, 0x27BDFE70u},
    {0x09C1CE58u, 0x2402FFFFu}, // addiu v0, zero, -1: backed out
    {0x09C1CFE0u, 0x0E213EE9u}, // jal 0x0884FBA4: paid
    {0x09C1CFF0u, 0x24020001u}, // addiu v0, zero, 1
    {0x09C1D6F4u, 0x00001021u}, // move v0, zero: still open
    {kFlavour, 0x27BDFFF0u},
    {kRollSkills, 0x27BDFFE0u},
    {kMealRow, 0x8C850080u},
    {kResults, 0x27BDFFF0u},
    {0x09C1E864u, 0x0E707630u}, // jal 0x09C1D8C0
    {0x09C1E87Cu, 0x0E707834u}, // jal 0x09C1E0D0: the skills' messages
    {kSetEaten, 0x3C03089Du},
    {0x09C1B9B8u, 0xA0659936u}, // sb a1, -0x66CA(v1): GameSys + 0x69936
    {0x09C1B988u, 0x3C02089Du},
    {0x09C1B998u, 0x03E00008u},
    {0x09C1B968u, 0x3C02089Du},
    {0x09C1B978u, 0x8C4296F4u}, // lw v0, -0x690C(v0): GameSys + 0x696F4
    {0x08836C10u, 0x0E20E2ADu}, // jal 0x08838AB4: seated?
    {0x08836C4Cu, 0x0E69EE24u}, // jal 0x09A7B890: Stand Up, printed
    {0x08836C50u, 0x9445533Au}, // lhu a1, 0x533A(v0)
    {0x08836C70u, 0x0E22451Du}, // jal 0x08891474
    {0x08836C84u, 0x0E69EE24u}, // jal 0x09A7B890: Stand Up, for the bar's width
    {0x08836C88u, 0x9445533Au}, // lhu a1, 0x533A(v0)
    {kTextCore, 0x27BDFFE0u},
    {kText, 0x00A03021u},       // move a2, a1
    {0x09A7B894u, 0x0A69EE08u}, // j 0x09A7B820
    {0x09A7B898u, 0x00002821u}, // move a1, zero: table 0
    {kPlainPrint, 0x27BDFCA0u},  // addiu sp, sp, -0x360
    {0x08891478u, 0xAFBF000Cu},
    {kMarkupPrint, 0x27BDFCA0u}, // addiu sp, sp, -0x360
    {0x088920ACu, 0xAFBF000Cu},
}};

// What runs after a confirmed meal, in the kitchen's order.
constexpr std::array<std::uint32_t, 5> kServe = {kFlavour, kRollSkills, kMealRow, kResults, kSetEaten};

enum class Phase { Idle, Menu, Serving };

struct State {
    psprecomp::Runtime::RecompiledFunction seated_update{};
    psprecomp::Runtime::RecompiledFunction text{};
    psprecomp::Runtime::RecompiledFunction plain_print{};
    GuestCaller caller{};
    psprecomp::GuestMemory *memory{};
    Phase phase{Phase::Idle};
    std::uint32_t hunter{};
    std::uint32_t kitchen{};
    std::size_t served{};
    // The hint's text call: the lobby's state, and the game's Stand Up.
    std::uint32_t text_state{};
    std::uint32_t stand_up{};
    bool printed{}; // the call is the bar's print, not its measure
    // Flips since the seated frame last ran the menu.
    std::uint32_t unseen{};
    bool reported{};
    bool trace{};
};

State &state() {
    static State s;
    return s;
}

std::uint32_t game_sys(const psprecomp::GuestMemory &memory) {
    if (!memory.contains(kGameSysPointer, 4u)) return 0u;
    const std::uint32_t sys = memory.load32(kGameSysPointer);
    return layout::in_ram(sys) && memory.contains(sys, kEaten + 1u) ? sys : 0u;
}

void close(const char *why) {
    State &s = state();
    if (s.trace && s.phase != Phase::Idle) std::cout << "[hall meal] " << why << std::endl;
    s.phase = Phase::Idle;
    s.hunter = 0u;
}

void back_to_caller(psprecomp::AllegrexContext &, std::uint32_t) {}

void serve_next(psprecomp::AllegrexContext &ctx, std::uint32_t) {
    State &s = state();
    if (s.served >= kServe.size()) {
        close("served");
        return;
    }
    s.caller(ctx, kServe[s.served++], {s.kitchen, 0u, 0u, 0u}, &serve_next);
}

void menu_done(psprecomp::AllegrexContext &ctx, std::uint32_t result) {
    State &s = state();
    const auto answer = static_cast<std::int32_t>(result);
    if (answer == kMenuRunning) return;
    if (answer != kMenuPaid) {
        close("menu closed without a meal");
        return;
    }
    if (s.trace) std::cout << "[hall meal] meal paid; serving it" << std::endl;
    s.phase = Phase::Serving;
    s.served = 0u;
    serve_next(ctx, 0u);
}

void open_menu(psprecomp::AllegrexContext &ctx, std::uint32_t) {
    State &s = state();
    s.caller(ctx, kMenuOpen, {s.kitchen, 0u, 0u, 0u}, &back_to_caller);
}

void seated_update(psprecomp::Runtime &runtime, psprecomp::AllegrexContext &ctx) {
    State &s = state();
    const psprecomp::GuestMemory &memory = runtime.memory();
    const std::uint32_t hunter = ctx.gpr[4];
    const std::uint32_t action = ctx.gpr[5];
    const bool on = settings::current().hall_meal && s.caller != nullptr;
    if (s.phase == Phase::Menu) {
        if (on && hunter == s.hunter && action == kSeated && kitchen(memory) == s.kitchen) {
            s.unseen = 0u;
            s.caller(ctx, kMenuFrame, {s.kitchen, 0u, 0u, 0u}, &menu_done);
            return;
        }
        close(on ? "menu closed: the hunter is no longer seated" : "menu closed: turned off");
    }
    if (on && s.phase == Phase::Idle && asks_for_meal(memory, hunter, action)) {
        const std::uint32_t lobby = lobby_state(memory);
        if (!can_eat(memory)) {
            if (s.trace) std::cout << "[hall meal] refused: the kitchen is not open or a meal was eaten" << std::endl;
            s.caller(ctx, kSound, {lobby, kSoundRefuse, 0u, 0u}, &back_to_caller);
            return;
        }
        if (s.trace)
            std::cout << "[hall meal] menu opened for hunter 0x" << std::hex << hunter << " with kitchen 0x"
                      << kitchen(memory) << std::dec << std::endl;
        s.phase = Phase::Menu;
        s.hunter = hunter;
        s.kitchen = kitchen(memory);
        s.unseen = 0u;
        s.caller(ctx, kSound, {lobby, kSoundConfirm, 0u, 0u}, &open_menu);
        return;
    }
    ctx.pc = kSeatedUpdate;
    s.seated_update(runtime, ctx);
}

std::string guest_string(const psprecomp::GuestMemory &memory, std::uint32_t address) {
    std::string text;
    for (std::uint32_t i = 0; i < kHintBytes && memory.contains(address + i, 1u); ++i) {
        const auto c = static_cast<char>(memory.load8(address + i));
        if (c == '\0') break;
        text += c;
    }
    return text;
}

void hint_ready(psprecomp::AllegrexContext &ctx, std::uint32_t order_food) {
    State &s = state();
    psprecomp::GuestMemory &memory = *s.memory;
    if (s.stand_up == 0u || order_food == 0u) {
        ctx.set_gpr(2, s.stand_up);
        return;
    }
    const std::string hint = hint_text(guest_string(memory, s.stand_up), guest_string(memory, order_food), s.printed);
    const std::uint32_t at = s.printed ? kHint : kHintWidth;
    for (std::size_t i = 0; i <= hint.size(); ++i)
        memory.store8(at + static_cast<std::uint32_t>(i), i < hint.size() ? static_cast<std::uint8_t>(hint[i]) : 0u);
    ctx.set_gpr(2, at);
}

void stand_up_ready(psprecomp::AllegrexContext &ctx, std::uint32_t stand_up) {
    State &s = state();
    s.stand_up = stand_up;
    s.caller(ctx, kTextCore, {s.text_state, 0u, kOrderFoodText, 0u}, &hint_ready);
}

// The lobby's text call: the banner's Stand Up gets the hint while a meal can
// be ordered.
void text(psprecomp::Runtime &runtime, psprecomp::AllegrexContext &ctx) {
    State &s = state();
    const std::uint32_t back = ctx.gpr[31];
    const auto id = static_cast<std::uint16_t>(ctx.gpr[5] & 0xFFFFu);
    if (id == kStandUpText && (back == kBannerPrint || back == kBannerWidth) && settings::current().hall_meal &&
        s.caller != nullptr && s.phase == Phase::Idle && code_matches(runtime.memory()) && can_eat(runtime.memory())) {
        s.memory = &runtime.memory();
        s.text_state = ctx.gpr[4];
        s.printed = back == kBannerPrint;
        s.caller(ctx, kTextCore, {s.text_state, 0u, kStandUpText, 0u}, &stand_up_ready);
        return;
    }
    ctx.pc = kText;
    s.text(runtime, ctx);
}

// The plain print: the bar's, with the hint, draws the game's icon.
void plain_print(psprecomp::Runtime &runtime, psprecomp::AllegrexContext &ctx) {
    State &s = state();
    if (ctx.gpr[31] == kBannerPrinted && ctx.gpr[kPromptArgument] == kHint) {
        ctx.pc = kMarkupPrint;
        return;
    }
    ctx.pc = kPlainPrint;
    s.plain_print(runtime, ctx);
}

} // namespace

std::string hint_text(const std::string &stand_up, const std::string &order_food, bool printed) {
    std::string hint = stand_up + (printed ? "  ~B02:" : "    :") + order_food;
    if (hint.size() < kHintBytes) return hint;
    hint.resize(kHintBytes - 1u);
    // Not in the middle of a character.
    while (!hint.empty() && (static_cast<unsigned char>(hint.back()) & 0xC0u) == 0x80u)
        hint.pop_back();
    if (!hint.empty() && (static_cast<unsigned char>(hint.back()) & 0x80u) != 0u) hint.pop_back();
    return hint;
}

std::span<const CodeWord> code_signature() {
    return kSignature;
}

bool code_matches(const psprecomp::GuestMemory &memory) {
    if (!layout::overlay_loaded(memory, layout::kTaskSlot, layout::kVillageOverlay) ||
        !layout::overlay_loaded(memory, layout::kSubSlot, layout::kSubOverlay))
        return false;
    for (const CodeWord &expected : kSignature)
        if (!memory.contains(expected.address, 4u) || memory.load32(expected.address) != expected.word) return false;
    return true;
}

std::uint32_t kitchen(const psprecomp::GuestMemory &memory) {
    if (!memory.contains(kKitchenPointer, 4u)) return 0u;
    const std::uint32_t object = memory.load32(kKitchenPointer);
    return layout::in_ram(object) ? object : 0u;
}

std::uint32_t lobby_state(const psprecomp::GuestMemory &memory) {
    if (!memory.contains(kLobbyState, 4u)) return 0u;
    const std::uint32_t object = memory.load32(kLobbyState);
    return layout::in_ram(object) && memory.contains(object, 4u) ? object : 0u;
}

bool asks_for_meal(const psprecomp::GuestMemory &memory, std::uint32_t hunter, std::uint32_t action) {
    const std::uint32_t sys = game_sys(memory);
    const std::uint32_t lobby = lobby_state(memory);
    if (action != kSeated || sys == 0u || lobby == 0u || kitchen(memory) == 0u || !layout::in_ram(hunter) ||
        !memory.contains(hunter, kHunterBytes))
        return false;
    return memory.load8(hunter + kPhase) == kSeatedPhase &&
        memory.load16(hunter + kIndex) == memory.load8(sys + kOwnIndex) && memory.load32(lobby) == 0u &&
        (memory.load32(hunter + kButtons) & kTriangle) != 0u;
}

bool can_eat(const psprecomp::GuestMemory &memory) {
    const std::uint32_t sys = game_sys(memory);
    return sys != 0u && memory.load32(sys + kKitchenOpen) != 0u && memory.load8(sys + kEaten) == 0u;
}

bool menu_open() {
    return state().phase != Phase::Idle;
}

void frame(psprecomp::Runtime &runtime, GuestCaller caller) {
    State &s = state();
    s.caller = caller;
    static const bool trace = [] {
        const char *text = std::getenv("MHFU_TRACE_HALL_MEAL");
        return text != nullptr && *text != '\0' && std::string(text) != "0";
    }();
    s.trace = trace;
    if (!code_matches(runtime.memory())) {
        close("menu closed: the hall's code is gone");
        return;
    }
    if (s.phase == Phase::Menu && ++s.unseen > kLostFrames) close("menu closed: the seated frame stopped");
    if (runtime.function_at(kSeatedUpdate) == nullptr || runtime.function_at(kText) == nullptr ||
        runtime.function_at(kPlainPrint) == nullptr)
        return;
    // lobby_task registers its functions again each time it is installed.
    // Item Sets wraps the text call too (game/hooks.hpp).
    (void)hooks::wrap(runtime, kSeatedUpdate, &seated_update, s.seated_update, "mhfu_hall_meal");
    (void)hooks::wrap(runtime, kText, &text, s.text, "mhfu_hall_meal_text");
    // The executable's, wrapped once and kept; outside the bar it only calls it.
    (void)hooks::wrap(runtime, kPlainPrint, &plain_print, s.plain_print, "mhfu_hall_meal_print");
    if (!s.reported) {
        s.reported = true;
        std::cout << "[hall meal] seated frame wrapped" << std::endl;
    }
}

} // namespace mhfu::game::hall_meal
