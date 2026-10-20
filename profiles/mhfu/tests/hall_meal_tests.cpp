// Eating in the Guild Hall: the code check, what counts as asking for a meal,
// whether the kitchen serves one, and the wrapper around a stand-in seated
// frame, menu and meal through a stand-in guest call. No game data.
#include "game/hall_meal.hpp"

#include "game/guest_layout.hpp"
#include "psprecomp/runtime.hpp"
#include "settings/settings.hpp"

#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

namespace {
using namespace mhfu::game::hall_meal;
namespace layout = mhfu::game::layout;

int failures{};

void check(bool condition, const char *message) {
    if (!condition) {
        ++failures;
        std::cerr << "FAIL: " << message << '\n';
    }
}

void set_environment(const char *name, const std::string &value) {
#if defined(_WIN32)
    _putenv_s(name, value.c_str());
#else
    setenv(name, value.c_str(), 1);
#endif
}

// An empty value would count as on (settings::variable_flag), so the
// variable is taken out: Windows' _putenv_s does that for an empty value.
void unset_environment(const char *name) {
#if defined(_WIN32)
    _putenv_s(name, "");
#else
    unsetenv(name);
#endif
}

constexpr std::uint32_t kMemory = 32u * 1024u * 1024u;
constexpr std::uint32_t kHunter = 0x090B3580u;
constexpr std::uint32_t kSys = 0x09000000u;
constexpr std::uint32_t kLobby = 0x09B100C0u;
constexpr std::uint32_t kKitchen = 0x09CFB8B0u;
constexpr std::uint32_t kReturn = 0x09AAAAD8u;     // where the seated frame returns
constexpr std::uint32_t kCallReturn = 0x0880000Cu; // where the stand-in guest call returns
// The game's words for the two texts, and what the stand-in text call gives
// for any other.
constexpr std::uint32_t kStandUpWords = 0x09900000u;
constexpr std::uint32_t kOrderFoodWords = 0x09900040u;
constexpr std::uint32_t kOtherWords = 0x09900080u;

// What the stand-ins saw.
int seated_calls{};
std::vector<std::uint32_t> calls; // every game_sub and sound function entered, in order
std::vector<std::uint32_t> sounds;
std::int32_t menu_answer{};
// The stand-in guest call: the caller's return address and what follows.
std::uint32_t call_return{};
ResultFunction call_then{};

void write_overlay(psprecomp::GuestMemory &memory, std::uint32_t slot, const std::string &name) {
    memory.store32(slot, layout::kOverlayMagic);
    memory.store32(slot + 8u, slot);
    for (std::size_t i = 0; i <= name.size(); ++i)
        memory.store8(slot + layout::kOverlayName + static_cast<std::uint32_t>(i),
            i < name.size() ? static_cast<std::uint8_t>(name[i]) : 0u);
}

void write_code(psprecomp::GuestMemory &memory) {
    write_overlay(memory, layout::kTaskSlot, std::string(layout::kVillageOverlay));
    write_overlay(memory, layout::kSubSlot, std::string(layout::kSubOverlay));
    for (const CodeWord &word : code_signature()) memory.store32(word.address, word.word);
}

// The console's own hunter seated in the hall, the lobby idle, the kitchen
// open and no meal eaten.
void seat(psprecomp::GuestMemory &memory) {
    memory.store32(kGameSysPointer, kSys);
    memory.store8(kSys + kOwnIndex, 1u);
    memory.store32(kSys + kKitchenOpen, 1u);
    memory.store8(kSys + kEaten, 0u);
    memory.store32(kLobbyState, kLobby);
    memory.store32(kLobby, 0u);
    memory.store32(kKitchenPointer, kKitchen);
    memory.store8(kHunter + kAction, kSeated);
    memory.store8(kHunter + kPhase, kSeatedPhase);
    memory.store16(kHunter + kIndex, 1u);
    memory.store32(kHunter + kButtons, 0u);
}

void game_seated(psprecomp::Runtime &, psprecomp::AllegrexContext &ctx) {
    ++seated_calls;
    ctx.pc = ctx.gpr[31];
}

int text_calls{};

// The lobby's text call and its core (0x09A7B820(state, table, id)).
void game_text(psprecomp::Runtime &, psprecomp::AllegrexContext &ctx) {
    ++text_calls;
    ctx.gpr[2] = ctx.gpr[5] == kStandUpText ? kStandUpWords : kOtherWords;
    ctx.pc = ctx.gpr[31];
}

std::uint32_t order_food_words = kOrderFoodWords;

void game_text_core(psprecomp::Runtime &, psprecomp::AllegrexContext &ctx) {
    if (ctx.gpr[4] != kLobby || ctx.gpr[5] != 0u) check(false, "the core gets the lobby's state and table 0");
    ctx.gpr[2] = ctx.gpr[6] == kOrderFoodText ? order_food_words : 0u;
    ctx.pc = ctx.gpr[31];
}

// The bar's calls, each with its arguments: a0 to a3, then t0 to t2.
struct BarCall {
    std::uint32_t function;
    std::array<std::uint32_t, 7> arguments;
    bool operator==(const BarCall &) const = default;
};
std::vector<BarCall> bar_calls;
constexpr std::uint32_t kFont = 0x09A00000u;
constexpr std::uint32_t kBarState = 0x090A0000u;
constexpr std::uint32_t kColon = 0x09900100u;
constexpr std::uint32_t kOrderFoodCharacters = 10u;

void game_bar(psprecomp::Runtime &, psprecomp::AllegrexContext &ctx) {
    bar_calls.push_back({ctx.pc, {ctx.gpr[4], ctx.gpr[5], ctx.gpr[6], ctx.gpr[7], ctx.gpr[8], ctx.gpr[9], ctx.gpr[10]}});
    ctx.gpr[2] = ctx.pc == kBarMeasure ? kOrderFoodCharacters : 0u;
    // What a callee leaves in the registers it need not keep.
    for (std::uint32_t r = 8u; r <= 10u; ++r) ctx.gpr[r] = 0xDEADu;
    ctx.pc = ctx.gpr[31];
}

void game_function(psprecomp::Runtime &runtime, psprecomp::AllegrexContext &ctx) {
    calls.push_back(ctx.pc);
    if (ctx.pc == kSound) {
        sounds.push_back(ctx.gpr[5]);
        if (ctx.gpr[4] != kLobby) check(false, "the sound gets the lobby's state");
    } else if (ctx.gpr[4] != kKitchen && ctx.pc != kSetEaten) {
        check(false, "the meal's functions get the kitchen");
    }
    if (ctx.pc == kSetEaten) runtime.memory().store8(kSys + kEaten, 1u);
    ctx.gpr[2] = ctx.pc == kMenuFrame ? static_cast<std::uint32_t>(menu_answer) : 0u;
    ctx.pc = ctx.gpr[31];
}

// The game's functions, as lobby_task's and game_sub's would be registered.
void install(psprecomp::Runtime &runtime) {
    runtime.register_function(kSeatedUpdate, &game_seated, "seated");
    runtime.register_function(kText, &game_text, "text");
    runtime.register_function(kTextCore, &game_text_core, "text_core");
    for (std::uint32_t function : {kBarMeasure, kBarBox, kBarButton, kPlainPrint})
        runtime.register_function(function, &game_bar, "bar");
    for (std::uint32_t function : {kMenuOpen, kMenuFrame, kFlavour, kRollSkills, kMealRow, kResults, kSetEaten, kSound})
        runtime.register_function(function, &game_function, "game");
}

// Kernel::call_guest as far as the call goes.
void guest_call(psprecomp::AllegrexContext &ctx, std::uint32_t function, const std::array<std::uint32_t, 4> &arguments,
    ResultFunction then) {
    call_return = ctx.gpr[31];
    call_then = then;
    for (std::uint32_t i = 0; i < 4u; ++i) ctx.gpr[4u + i] = arguments[i];
    ctx.gpr[31] = kCallReturn;
    ctx.pc = function;
}

// One seated frame, from the hunter's frame to its return, through every
// guest call and its return.
void run_seated(psprecomp::Runtime &runtime, std::uint32_t action = kSeated) {
    psprecomp::AllegrexContext ctx{};
    ctx.gpr[4] = kHunter;
    ctx.gpr[5] = action;
    ctx.gpr[31] = kReturn;
    ctx.pc = kSeatedUpdate;
    for (int step = 0; step < 32 && ctx.pc != kReturn; ++step) {
        if (ctx.pc == kCallReturn) {
            ctx.gpr[31] = call_return;
            ctx.pc = call_return;
            call_then(ctx, ctx.gpr[2]);
            continue;
        }
        runtime.function_at(ctx.pc)(runtime, ctx);
    }
    check(ctx.pc == kReturn && ctx.gpr[31] == kReturn, "the seated frame returns to its caller");
}

// One text call for `id`, made from `back`, through every guest call; its v0.
std::uint32_t call_text(psprecomp::Runtime &runtime, std::uint16_t id, std::uint32_t back) {
    psprecomp::AllegrexContext ctx{};
    ctx.gpr[4] = kLobby;
    ctx.gpr[5] = id;
    ctx.gpr[31] = back;
    ctx.pc = kText;
    for (int step = 0; step < 16 && ctx.pc != back; ++step) {
        if (ctx.pc == kCallReturn) {
            ctx.gpr[31] = call_return;
            ctx.pc = call_return;
            call_then(ctx, ctx.gpr[2]);
            continue;
        }
        runtime.function_at(ctx.pc)(runtime, ctx);
    }
    check(ctx.pc == back && ctx.gpr[31] == back, "the text call returns to its caller");
    return ctx.gpr[2];
}

// The bar's button call, as the bar makes it from `back`, through every guest
// call; the bar's registers it keeps are checked kept.
void call_button(psprecomp::Runtime &runtime, std::uint32_t back = kBarButtonDrawn, std::uint32_t blink = 7u) {
    psprecomp::AllegrexContext ctx{};
    ctx.gpr[4] = kBarState;
    ctx.gpr[5] = kButtonX;
    ctx.gpr[6] = kBarTop;
    ctx.gpr[7] = 53u;
    ctx.gpr[8] = blink;
    ctx.gpr[9] = static_cast<std::uint32_t>(kBlinkShown);
    ctx.gpr[10] = 0u;
    ctx.gpr[kColonRegister] = kColon;
    ctx.gpr[20] = kBarState;
    ctx.gpr[31] = back;
    ctx.pc = kBarButton;
    for (int step = 0; step < 32 && ctx.pc != back; ++step) {
        if (ctx.pc == kCallReturn) {
            ctx.gpr[31] = call_return;
            ctx.pc = call_return;
            call_then(ctx, ctx.gpr[2]);
            continue;
        }
        runtime.function_at(ctx.pc)(runtime, ctx);
    }
    check(ctx.pc == back && ctx.gpr[31] == back, "the button call returns to the bar");
    check(ctx.gpr[kColonRegister] == kColon && ctx.gpr[20] == kBarState, "with the bar's saved registers kept");
}

// The bar of a frame at a seat: Stand Up printed, then its button.
void bar_frame(psprecomp::Runtime &runtime, std::uint32_t blink = 7u) {
    check(call_text(runtime, kStandUpText, kBannerPrint) == kStandUpWords, "Stand Up is the game's words");
    bar_calls.clear();
    call_button(runtime, kBarButtonDrawn, blink);
}

BarCall game_button() {
    return {kBarButton, {kBarState, kButtonX, kBarTop, 53u, 7u, static_cast<std::uint32_t>(kBlinkShown), 0u}};
}

void press_triangle(psprecomp::GuestMemory &memory, bool down) {
    memory.store32(kHunter + kButtons, down ? kTriangle : 0u);
}

void reset_seen() {
    seated_calls = 0;
    calls.clear();
    sounds.clear();
}

void test_code_check() {
    psprecomp::Runtime runtime(kMemory);
    psprecomp::GuestMemory &memory = runtime.memory();
    check(!code_matches(memory), "nothing loaded is not the game's code");
    write_code(memory);
    check(code_matches(memory), "lobby_task and game_sub with every word are");
    for (const CodeWord &word : code_signature()) {
        memory.store32(word.address, word.word ^ 1u);
        check(!code_matches(memory), "a word that differs is another game's code");
        memory.store32(word.address, word.word);
    }
    write_overlay(memory, layout::kSubSlot, "demo_sub.ovl");
    check(!code_matches(memory), "without game_sub in its slot it is not");
    write_overlay(memory, layout::kSubSlot, std::string(layout::kSubOverlay));
    write_overlay(memory, layout::kTaskSlot, std::string(layout::kQuestOverlay));
    check(!code_matches(memory), "nor during a quest");
}

void test_asks_for_meal() {
    psprecomp::Runtime runtime(kMemory);
    psprecomp::GuestMemory &memory = runtime.memory();
    seat(memory);
    check(!asks_for_meal(memory, kHunter, kSeated), "no △, no meal");
    press_triangle(memory, true);
    check(asks_for_meal(memory, kHunter, kSeated), "△ while seated asks for one");
    check(!asks_for_meal(memory, kHunter, 15u), "not while sitting down");
    memory.store8(kHunter + kPhase, 1u);
    check(!asks_for_meal(memory, kHunter, kSeated), "nor before the hunter has sat");
    memory.store8(kHunter + kPhase, kSeatedPhase);
    memory.store16(kHunter + kIndex, 2u);
    check(!asks_for_meal(memory, kHunter, kSeated), "another console's hunter does not");
    memory.store16(kHunter + kIndex, 1u);
    memory.store32(kLobby, 39u);
    check(!asks_for_meal(memory, kHunter, kSeated), "nor while the lobby runs something else");
    memory.store32(kLobby, 0u);
    memory.store32(kKitchenPointer, 0u);
    check(!asks_for_meal(memory, kHunter, kSeated), "nor without a kitchen object");
    memory.store32(kKitchenPointer, kKitchen);
    memory.store32(kHunter + kButtons, kTriangle | 0x8000u);
    check(asks_for_meal(memory, kHunter, kSeated), "△ with another button still asks");
    check(!asks_for_meal(memory, 0u, kSeated), "no hunter, no meal");
}

void test_can_eat() {
    psprecomp::Runtime runtime(kMemory);
    psprecomp::GuestMemory &memory = runtime.memory();
    check(!can_eat(memory), "without GameSys there is no kitchen");
    seat(memory);
    check(can_eat(memory), "an open kitchen and no meal eaten serves one");
    memory.store8(kSys + kEaten, 1u);
    check(!can_eat(memory), "one meal between quests, as at home");
    memory.store8(kSys + kEaten, 0u);
    memory.store32(kSys + kKitchenOpen, 0u);
    check(!can_eat(memory), "a kitchen not open yet serves nothing");
}

void test_meal() {
    psprecomp::Runtime runtime(kMemory);
    psprecomp::GuestMemory &memory = runtime.memory();
    auto &settings = mhfu::settings::current();
    settings.hall_meal = true;
    write_code(memory);
    seat(memory);
    frame(runtime, &guest_call);
    check(runtime.function_at(kSeatedUpdate) == nullptr, "nothing is wrapped before lobby_task is installed");
    install(runtime);
    frame(runtime, &guest_call);
    check(runtime.function_at(kSeatedUpdate) != &game_seated, "the seated frame is wrapped");

    reset_seen();
    run_seated(runtime);
    check(seated_calls == 1 && calls.empty(), "seated without △ is the game's frame");

    press_triangle(memory, true);
    reset_seen();
    run_seated(runtime);
    check(seated_calls == 0, "△ opens the menu in place of the seated frame");
    check(calls == std::vector<std::uint32_t>({kSound, kMenuOpen}) &&
            sounds == std::vector<std::uint32_t>({kSoundConfirm}),
        "with the lobby's confirm and the kitchen's menu");
    check(menu_open(), "the menu is open");

    press_triangle(memory, false);
    menu_answer = kMenuRunning;
    for (int f = 0; f < 5; ++f) {
        reset_seen();
        run_seated(runtime);
        frame(runtime, &guest_call);
        check(seated_calls == 0 && calls == std::vector<std::uint32_t>({kMenuFrame}),
            "while open, each seated frame is the menu's");
    }
    check(menu_open(), "and it stays open");

    menu_answer = kMenuPaid;
    reset_seen();
    run_seated(runtime);
    check(calls == std::vector<std::uint32_t>({kMenuFrame, kFlavour, kRollSkills, kMealRow, kResults, kSetEaten}),
        "a paid meal is served in the kitchen's order");
    check(!menu_open() && memory.load8(kSys + kEaten) == 1u, "and eaten");

    reset_seen();
    run_seated(runtime);
    check(seated_calls == 1, "after it the seated frame is the game's again");

    press_triangle(memory, true);
    reset_seen();
    run_seated(runtime);
    check(seated_calls == 0 && calls == std::vector<std::uint32_t>({kSound}) &&
            sounds == std::vector<std::uint32_t>({kSoundRefuse}) && !menu_open(),
        "a second meal before the next quest is refused, with the refusal's sound");

    memory.store8(kSys + kEaten, 0u);
    reset_seen();
    run_seated(runtime);
    press_triangle(memory, false);
    menu_answer = -1;
    reset_seen();
    run_seated(runtime);
    check(calls == std::vector<std::uint32_t>({kMenuFrame}) && !menu_open(), "backing out closes it, unserved");
    check(memory.load8(kSys + kEaten) == 0u, "and nothing is eaten");

    // The hunter no longer seated while the menu is open.
    press_triangle(memory, true);
    run_seated(runtime);
    press_triangle(memory, false);
    check(menu_open(), "open again");
    reset_seen();
    run_seated(runtime, 17u);
    check(!menu_open() && seated_calls == 1 && calls.empty(), "another action closes it and is the game's");

    // The seated frame no longer running.
    press_triangle(memory, true);
    run_seated(runtime);
    press_triangle(memory, false);
    for (std::uint32_t f = 0; f <= kLostFrames; ++f) frame(runtime, &guest_call);
    check(!menu_open(), "a menu the seated frame stopped running is closed");

    // Off.
    settings.hall_meal = false;
    press_triangle(memory, true);
    reset_seen();
    run_seated(runtime);
    check(seated_calls == 1 && calls.empty() && !menu_open(), "off, △ is the game's");
    settings.hall_meal = true;
    run_seated(runtime);
    check(menu_open(), "on again, it opens");
    settings.hall_meal = false;
    reset_seen();
    press_triangle(memory, false);
    run_seated(runtime);
    check(!menu_open() && seated_calls == 1, "turned off while open, it closes");
    settings.hall_meal = true;

    // lobby_task installed again registers its own functions.
    install(runtime);
    frame(runtime, &guest_call);
    check(runtime.function_at(kSeatedUpdate) != &game_seated, "wrapped again after lobby_task is installed again");

    // Leaving the hall closes a menu.
    press_triangle(memory, true);
    run_seated(runtime);
    check(menu_open(), "open in the hall");
    write_overlay(memory, layout::kTaskSlot, std::string(layout::kQuestOverlay));
    frame(runtime, &guest_call);
    check(!menu_open(), "closed once lobby_task is gone");
}

void test_bar_width() {
    check(bar_width(0u) == 64u, "an empty prompt's bar is 64 pixels, as the game's");
    check(bar_width(kOrderFoodCharacters) == 134u, "and 7 more a character");
    check(bar_width(0x2000u) == static_cast<std::uint32_t>(static_cast<std::int16_t>(0xE040u)),
        "kept to a signed halfword, as the game keeps it");
}

void test_hint() {
    psprecomp::Runtime runtime(kMemory);
    psprecomp::GuestMemory &memory = runtime.memory();
    auto &settings = mhfu::settings::current();
    settings.hall_meal = true;
    write_code(memory);
    seat(memory);
    memory.store32(kFontPointer, kFont);
    install(runtime);
    frame(runtime, &guest_call);
    check(runtime.function_at(kText) != &game_text, "the text call is wrapped");
    check(runtime.function_at(kBarButton) != &game_bar, "the bar's button is wrapped");

    constexpr std::uint32_t kRow = kBarTop + kRowHeight;
    const std::vector<BarCall> with_row = {
        game_button(),
        {kBarMeasure, {kFont, kOrderFoodWords, kBarTop, 0u, 0xDEADu, 0xDEADu, 0xDEADu}},
        {kBarBox, {kBarState, 0u, kRow, bar_width(kOrderFoodCharacters), kRowHeight, kBarColour, 0u}},
        {kBarButton, {kBarState, kButtonX, kRow, kTriangleSprite, 7u, static_cast<std::uint32_t>(kBlinkShown), 0u}},
        {kPlainPrint, {kFont, kTextX, kRow + kTextDrop, kPrintFormat, kColon, kOrderFoodWords, 0xDEADu}},
    };
    const std::vector<BarCall> game_only = {game_button()};

    text_calls = 0;
    bar_frame(runtime);
    check(text_calls == 1, "the bar's Stand Up goes to the game's text call");
    check(bar_calls == with_row,
        "after the game's button, a second row: Order Food measured, its bar, △ and the words, one row lower");

    bar_calls.clear();
    call_button(runtime);
    check(bar_calls == game_only, "a button call without Stand Up printed before it is the game's alone");

    bar_frame(runtime, 30u);
    check(bar_calls.size() == with_row.size() && bar_calls[3].arguments[4] == 30u,
        "the △ takes the bar's blink, so both buttons blink together");

    check(call_text(runtime, kStandUpText, 0x09A63830u) == kStandUpWords, "Stand Up printed elsewhere");
    bar_calls.clear();
    call_button(runtime);
    check(bar_calls == game_only, "makes no row");
    check(call_text(runtime, kStandUpText, kBannerPrint) == kStandUpWords, "Stand Up printed by the bar");
    bar_calls.clear();
    call_button(runtime, 0x08836000u);
    check(bar_calls == game_only, "and a button drawn elsewhere make none");
    bar_calls.clear();
    call_button(runtime);
    check(bar_calls == with_row, "the bar's next button call draws it");

    check(call_text(runtime, kStandUpText, kBannerPrint) == kStandUpWords, "Stand Up printed by the bar");
    frame(runtime, &guest_call);
    bar_calls.clear();
    call_button(runtime);
    check(bar_calls == game_only, "but not after the flip");

    memory.store8(kSys + kEaten, 1u);
    bar_frame(runtime);
    check(bar_calls == game_only, "after a meal, no row");
    memory.store8(kSys + kEaten, 0u);
    memory.store32(kSys + kKitchenOpen, 0u);
    bar_frame(runtime);
    check(bar_calls == game_only, "nor before the kitchen opens");
    memory.store32(kSys + kKitchenOpen, 1u);

    press_triangle(memory, true);
    run_seated(runtime);
    check(menu_open(), "the menu is open");
    bar_frame(runtime);
    check(bar_calls == game_only, "nor while the menu is open");
    menu_answer = -1;
    press_triangle(memory, false);
    run_seated(runtime);
    menu_answer = kMenuRunning;

    settings.hall_meal = false;
    bar_frame(runtime);
    check(bar_calls == game_only, "off, the game's");
    settings.hall_meal = true;

    order_food_words = 0u;
    bar_frame(runtime);
    check(bar_calls == game_only, "no Order Food text, no row");
    order_food_words = kOrderFoodWords;
    bar_frame(runtime);
    check(bar_calls == with_row, "with it again, the row");
}

void test_other_code() {
    psprecomp::Runtime runtime(kMemory);
    install(runtime);
    mhfu::settings::current().hall_meal = true;
    frame(runtime, &guest_call);
    check(runtime.function_at(kSeatedUpdate) == &game_seated, "code that does not match is not wrapped");
}

} // namespace

int main() {
    const std::filesystem::path data = std::filesystem::temp_directory_path() / "pokee_hall_meal_tests";
    std::filesystem::create_directories(data);
    set_environment("MHFU_DATA_DIR", data.string());
    unset_environment("MHFU_HALL_MEAL");
    set_environment("MHFU_TRACE_HALL_MEAL", "1");
    check(mhfu::settings::current().hall_meal, "Eat in the Guild Hall is on by default");
    test_code_check();
    test_asks_for_meal();
    test_can_eat();
    test_other_code();
    test_meal();
    test_bar_width();
    test_hint();
    if (failures != 0) {
        std::cerr << failures << " hall meal check(s) failed\n";
        return 1;
    }
    std::cout << "hall meal tests passed\n";
    return 0;
}
