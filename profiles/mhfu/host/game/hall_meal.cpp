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

constexpr std::array<CodeWord, 67> kSignature = {{
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
    {0x08836C3Cu, 0x00408021u}, // move s0, v0: the ":"
    {0x08836C4Cu, 0x0E69EE24u}, // jal 0x09A7B890: Stand Up, printed
    {0x08836C50u, 0x9445533Au}, // lhu a1, 0x533A(v0)
    {0x08836C64u, 0x02004021u}, // move t0, s0
    {0x08836C70u, 0x0E22451Du}, // jal 0x08891474
    {kTextCore, 0x27BDFFE0u},
    {kText, 0x00A03021u},       // move a2, a1
    {0x09A7B894u, 0x0A69EE08u}, // j 0x09A7B820
    {0x09A7B898u, 0x00002821u}, // move a1, zero: table 0
    {kPlainPrint, 0x27BDFCA0u}, // addiu sp, sp, -0x360
    {0x08891478u, 0xAFBF000Cu},
    // The bar, shared by every prompt.
    {0x08837534u, 0x8C4432C8u}, // lw a0, the font
    {0x08837538u, 0x0E224392u}, // jal 0x08890E48: its characters
    {0x08837540u, 0x24430004u}, // addiu v1, v0, 4
    {0x08837544u, 0x000310C0u}, // sll v0, v1, 3
    {0x0883754Cu, 0x24420024u}, // addiu v0, v0, 36
    {0x08837558u, 0x3C02801Du}, // lui v0, 0x801D
    {0x0883756Cu, 0x34493905u}, // ori t1, v0, 0x3905: its colour
    {0x08837570u, 0x0E20E2C2u}, // jal 0x08838B08: the bar
    {0x08837578u, 0x82880535u}, // lb t0, 0x535(s4): the blink
    {0x08837588u, 0x24070035u}, // addiu a3, zero, 53: the button
    {0x0883758Cu, 0x0E20E383u}, // jal 0x08838E0C
    {0x08837590u, 0x24090014u}, // addiu t1, zero, 20
    {kBarMeasure, 0x27BDFFE0u},
    {kBarBox, 0x27BDFFC0u},
    {kBarButton, 0x27BDFFF0u},
    {0x08838E10u, 0x0109182Au}, // slt v1, t0, t1: shown while the blink is below
    {0x08838E1Cu, 0x0E20AA77u}, // jal 0x0882A9DC: the sprite
    {kPrintFormat, 0x73257325u}, // "%s%s"
}};

// What runs after a confirmed meal, in the kitchen's order.
constexpr std::array<std::uint32_t, 5> kServe = {kFlavour, kRollSkills, kMealRow, kResults, kSetEaten};

enum class Phase { Idle, Menu, Serving };

struct State {
    psprecomp::Runtime::RecompiledFunction seated_update{};
    psprecomp::Runtime::RecompiledFunction text{};
    psprecomp::Runtime::RecompiledFunction bar_button{};
    GuestCaller caller{};
    psprecomp::GuestMemory *memory{};
    Phase phase{Phase::Idle};
    std::uint32_t hunter{};
    std::uint32_t kitchen{};
    std::size_t served{};
    // The second row: due since the bar printed Stand Up this frame, and what
    // it is drawn with: the lobby's state for the text call, the bar's state,
    // its blink and ":", and the game's Order Food.
    bool order_row{};
    std::uint32_t text_state{};
    std::uint32_t bar{};
    std::uint32_t blink{};
    std::uint32_t colon{};
    std::uint32_t order_food{};
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

// The lobby's text call: the bar's Stand Up, while a meal can be ordered,
// makes the second row due. The text is the game's.
void text(psprecomp::Runtime &runtime, psprecomp::AllegrexContext &ctx) {
    State &s = state();
    const auto id = static_cast<std::uint16_t>(ctx.gpr[5] & 0xFFFFu);
    if (id == kStandUpText && ctx.gpr[31] == kBannerPrint && settings::current().hall_meal && s.caller != nullptr &&
        s.phase == Phase::Idle && code_matches(runtime.memory()) && can_eat(runtime.memory())) {
        s.memory = &runtime.memory();
        s.order_row = true;
        s.text_state = ctx.gpr[4];
    }
    ctx.pc = kText;
    s.text(runtime, ctx);
}

std::uint32_t font() {
    const psprecomp::GuestMemory &memory = *state().memory;
    return memory.contains(kFontPointer, 4u) ? memory.load32(kFontPointer) : 0u;
}

// The second row, after the game's: its text, its width, its bar, its button
// and its words, each call returning to the next.
void row_words(psprecomp::AllegrexContext &ctx, std::uint32_t) {
    State &s = state();
    ctx.set_gpr(8, s.colon);
    ctx.set_gpr(9, s.order_food);
    s.caller(ctx, kPlainPrint, {font(), kTextX, kBarTop + kRowHeight + kTextDrop, kPrintFormat}, &back_to_caller);
}

void row_button(psprecomp::AllegrexContext &ctx, std::uint32_t) {
    State &s = state();
    ctx.set_gpr(8, s.blink);
    ctx.set_gpr(9, static_cast<std::uint32_t>(kBlinkShown));
    ctx.set_gpr(10, 0u);
    s.caller(ctx, kBarButton, {s.bar, kButtonX, kBarTop + kRowHeight, kTriangleSprite}, &row_words);
}

void row_bar(psprecomp::AllegrexContext &ctx, std::uint32_t characters) {
    State &s = state();
    ctx.set_gpr(8, kRowHeight);
    ctx.set_gpr(9, kBarColour);
    ctx.set_gpr(10, 0u);
    s.caller(ctx, kBarBox, {s.bar, 0u, kBarTop + kRowHeight, bar_width(characters)}, &row_button);
}

void row_text(psprecomp::AllegrexContext &ctx, std::uint32_t order_food) {
    State &s = state();
    if (order_food == 0u) return;
    s.order_food = order_food;
    s.caller(ctx, kBarMeasure, {font(), order_food, kBarTop, 0u}, &row_bar);
}

void row_after_game(psprecomp::AllegrexContext &ctx, std::uint32_t) {
    State &s = state();
    s.caller(ctx, kTextCore, {s.text_state, 0u, kOrderFoodText, 0u}, &row_text);
}

// The bar's button: after the game's, the second row when it is due. Its own
// calls come back here from the return stub and are the game's.
void bar_button(psprecomp::Runtime &runtime, psprecomp::AllegrexContext &ctx) {
    State &s = state();
    if (ctx.gpr[31] == kBarButtonDrawn && s.order_row && s.caller != nullptr) {
        s.order_row = false;
        s.bar = ctx.gpr[4];
        s.blink = ctx.gpr[8];
        s.colon = ctx.gpr[kColonRegister];
        s.caller(ctx, kBarButton, {ctx.gpr[4], ctx.gpr[5], ctx.gpr[6], ctx.gpr[7]}, &row_after_game);
        return;
    }
    ctx.pc = kBarButton;
    s.bar_button(runtime, ctx);
}

} // namespace

std::uint32_t bar_width(std::uint32_t characters) {
    // As the game: (characters + 4) * 7 + 36, kept to a signed halfword.
    return static_cast<std::uint32_t>(static_cast<std::int32_t>(
        static_cast<std::int16_t>(static_cast<std::uint16_t>((characters + 4u) * 7u + 36u))));
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
    // A row the bar did not draw this frame is not drawn in the next.
    s.order_row = false;
    if (runtime.function_at(kSeatedUpdate) == nullptr || runtime.function_at(kText) == nullptr ||
        runtime.function_at(kBarButton) == nullptr)
        return;
    // lobby_task registers its functions again each time it is installed.
    // Item Sets wraps the text call too (game/hooks.hpp).
    (void)hooks::wrap(runtime, kSeatedUpdate, &seated_update, s.seated_update, "mhfu_hall_meal");
    (void)hooks::wrap(runtime, kText, &text, s.text, "mhfu_hall_meal_text");
    // The executable's, wrapped once and kept; outside the bar it only calls it.
    (void)hooks::wrap(runtime, kBarButton, &bar_button, s.bar_button, "mhfu_hall_meal_button");
    if (!s.reported) {
        s.reported = true;
        std::cout << "[hall meal] seated frame wrapped" << std::endl;
    }
}

} // namespace mhfu::game::hall_meal
