#pragma once

// Eating in the Guild Hall: a hunter seated at one of the hall's tables
// presses △ and orders from the home Felyne Kitchen's menu, with the kitchen's
// chefs, ingredients, prices and skills, as Order Food in the house does. On
// by default (Mods page, `game.hall_meal`; `MHFU_HALL_MEAL=0` turns it off).
// `MHFU_TRACE_HALL_MEAL=1` prints each step.
//
// Read off the USA executable (ULUS-10391), its lobby_task overlay and
// game_sub, which is loaded beside it in the village and the hall alike.
// Traced with a temporary MHFU_TRACE_KITCHEN in the house's kitchen and the
// offline Guild Hall (stage 85):
//
// Sitting. The hall's seats are interaction points of type 8: □ there runs
// 0x09AAF970, which gives the hunter action 15 of group 0 (the hunter's
// +0x298 and +0x299; online it also takes the seat in the member entry's
// +0xAC). Action 15 sits down and 16 is seated; the hunter's frame runs both
// through 0x09AAC0A8(hunter, action), called from 0x09AAAAD0 with 16, and
// sitting is done once the phase byte +0x1D5 is 2. Then, for the console's
// own hunter (+0x1E6 equal to GameSys +0x28) and while the lobby's mode (the
// word at load32(0x09B22E70)) is 0, it reads the buttons new this frame at
// +0x484: □ stands up (action 17), ○, × and the D-pad give the seated
// gestures. △ (0x1000) does nothing there.
//
// The kitchen. In the house, Order Food is the interaction point of type 13.
// It refuses while GameSys +0x69936 is set, the hunter having eaten since the
// last quest (0x09C1B988), or before the kitchen is open (the word at GameSys
// +0x696F4, 0x09C1B968), and otherwise starts the kitchen's event scripts in
// lobby mode 39. Those scripts move the chefs, the camera and the fades
// around the room; the meal itself is game_sub's, on the kitchen object
// (load32(0x09CFB998)), which the hall has too:
// - 0x09C1CBF0(kitchen) opens the menu: Today's Meals, the ingredients the
//   chefs offer, Meal A and Meal B, and the price.
// - 0x09C1CC38(kitchen) is one frame of it, drawn with the game's windows,
//   with the buttons from 0x08A62D8C and the lobby's sounds (0x09A7BB50). It
//   returns 0 while open, -1 when backed out of, and 1 once the meal is
//   confirmed and paid (0x0884FBA4(GameSys, -price), at 0x09C1CFE0).
// - After 1, the menu's script (0x09A71B10) runs 0x09C1E3F0 (which of the
//   flavour lines) and 0x09C1DCA0 (each chef's skill rolled, at kitchen
//   +0xD8); the eating scene's 0x09C1D720 picks the meal's row of effects
//   (kitchen +0xD0); the scene's end runs 0x09C1E858, which writes the
//   effects (GameSys +0x69938 on), the skills (GameSys +0x69942 and +0xA9),
//   the chefs' levels, and the messages; and leaving the kitchen sets GameSys
//   +0x69936 through 0x09C1B9A0.
//
// So the seated frame is wrapped. △ there, with the kitchen open and no meal
// eaten, plays the lobby's confirm and opens the menu; with a meal eaten, the
// refusal. While the menu is open the wrapper runs the menu's frame in place
// of the seated one, so the hunter stays seated and the menu has the
// buttons. A confirmed meal runs the four calls after the menu and sets the
// eaten flag, in the order the kitchen does; the chefs' scene does not play.
// The calls are guest calls in the game's thread, one after the other, each
// returning to the seated frame's caller.
//
// The hint. Near a seat and on it, the bar at the top left of the hall shows
// ":Sit Down" or ":Stand Up". The resident 0x08835F7C draws it: for a seat it
// asks 0x08838AB4 whether the hunter is seated and prints, with the plain
// 0x08891474(font, 36, 18, "%s%s", ":", prompt), the text of the halfword at
// 0x09B0533A, 508 (Stand Up), or 0x09B05338, 507 (Sit Down), each from the
// lobby's text call 0x09A7B890(state, id). It asks for the prompt again
// (from 0x08836C84) to size the bar: 0x08890E48 counts its characters, two
// for a wide one, and the bar is 7 pixels a character and 64 more. The plain
// print knows no markup. 0x088920A8, which Item Sets prints its hints with,
// takes the same arguments (both keep a 0x360-byte frame) and also draws the
// game's button icons for ~B, from the game's own texture, which a texture
// pack replaces. So while a meal can be ordered, the two calls for Stand Up
// are answered with the game's own words for both (508, and 515 at
// 0x09B05348, through the text call's core 0x09A7B820(state, 0, id)): the
// printed one with "Stand Up  ~B02:Order Food" (~B02 is △), the measured one
// with two spaces in the icon's place. The texts are kept in the window the
// host reserves below the load image (kernel/kernel.hpp). The plain print is
// wrapped, and its call from the bar (returning to 0x08836C78) with the hint
// as the prompt goes to 0x088920A8 instead.

#include "psprecomp/guest_memory.hpp"

#include <array>
#include <cstdint>
#include <span>
#include <string>

namespace psprecomp {
class Runtime;
struct AllegrexContext;
} // namespace psprecomp

namespace mhfu::game::hall_meal {

// The seated frame and the hunter's fields it reads.
inline constexpr std::uint32_t kSeatedUpdate = 0x09AAC0A8u;
inline constexpr std::uint32_t kSeated = 16u;
inline constexpr std::uint32_t kAction = 0x299u;
inline constexpr std::uint32_t kPhase = 0x1D5u;
inline constexpr std::uint8_t kSeatedPhase = 2u;
inline constexpr std::uint32_t kIndex = 0x1E6u; // u16: which console's hunter
inline constexpr std::uint32_t kButtons = 0x484u;
inline constexpr std::uint32_t kHunterBytes = kButtons + 4u;
inline constexpr std::uint32_t kTriangle = 0x1000u;
// The lobby's state: its mode at +0, 0 while nothing else runs.
inline constexpr std::uint32_t kLobbyState = 0x09B22E70u;
// GameSys, and what the kitchen keeps there.
inline constexpr std::uint32_t kGameSysPointer = 0x089CC558u;
inline constexpr std::uint32_t kOwnIndex = 0x28u;
inline constexpr std::uint32_t kKitchenOpen = 0x696F4u; // word: 0 before the kitchen opens
inline constexpr std::uint32_t kEaten = 0x69936u;       // u8: eaten since the last quest
// The kitchen object, and game_sub's meal.
inline constexpr std::uint32_t kKitchenPointer = 0x09CFB998u;
inline constexpr std::uint32_t kMenuOpen = 0x09C1CBF0u;
inline constexpr std::uint32_t kMenuFrame = 0x09C1CC38u;
inline constexpr std::uint32_t kFlavour = 0x09C1E3F0u;
inline constexpr std::uint32_t kRollSkills = 0x09C1DCA0u;
inline constexpr std::uint32_t kMealRow = 0x09C1D720u;
inline constexpr std::uint32_t kResults = 0x09C1E858u;
inline constexpr std::uint32_t kSetEaten = 0x09C1B9A0u;
// The lobby's sounds, 0x09A7BB50(state, which): 0 the confirm, 4 the refusal.
inline constexpr std::uint32_t kSound = 0x09A7BB50u;
inline constexpr std::uint32_t kSoundConfirm = 0u;
inline constexpr std::uint32_t kSoundRefuse = 4u;
// The lobby's text call, its core, and the banner's two calls for Stand Up.
inline constexpr std::uint32_t kText = 0x09A7B890u;
inline constexpr std::uint32_t kTextCore = 0x09A7B820u;
inline constexpr std::uint16_t kStandUpText = 508u;
inline constexpr std::uint16_t kOrderFoodText = 515u;
inline constexpr std::uint32_t kBannerPrint = 0x08836C54u; // where the call returns
inline constexpr std::uint32_t kBannerWidth = 0x08836C8Cu;
// The plain print, the one that draws ~B icons, and where the bar's print
// returns.
inline constexpr std::uint32_t kPlainPrint = 0x08891474u;
inline constexpr std::uint32_t kMarkupPrint = 0x088920A8u;
inline constexpr std::uint32_t kBannerPrinted = 0x08836C78u;
inline constexpr std::uint32_t kPromptArgument = 9u; // t1: the "%s%s" format's second string
// The hint as printed and as measured, in the host's reserved window.
inline constexpr std::uint32_t kHint = 0x08803F40u;
inline constexpr std::uint32_t kHintWidth = 0x08803F80u;
inline constexpr std::uint32_t kHintBytes = 0x40u;
// What the menu's frame returns.
inline constexpr std::int32_t kMenuRunning = 0;
inline constexpr std::int32_t kMenuPaid = 1;
// The menu is closed when the seated frame has not run for this many flips:
// the hunter stood or was moved, or the hall was left.
inline constexpr std::uint32_t kLostFrames = 3u;

struct CodeWord {
    std::uint32_t address;
    std::uint32_t word;
};

// What the addresses above were read off. A mismatch means another overlay
// or executable, and nothing is wrapped.
[[nodiscard]] std::span<const CodeWord> code_signature();
[[nodiscard]] bool code_matches(const psprecomp::GuestMemory &memory);

// The kitchen object; 0 when there is none.
[[nodiscard]] std::uint32_t kitchen(const psprecomp::GuestMemory &memory);
// The lobby's state object; 0 when there is none.
[[nodiscard]] std::uint32_t lobby_state(const psprecomp::GuestMemory &memory);

// Whether this seated frame is the console's own hunter, seated, with the
// lobby idle and △ new this frame.
[[nodiscard]] bool asks_for_meal(const psprecomp::GuestMemory &memory, std::uint32_t hunter, std::uint32_t action);
// Whether the kitchen would serve a meal now: open, and none eaten since the
// last quest.
[[nodiscard]] bool can_eat(const psprecomp::GuestMemory &memory);

// Runs a guest function in the calling thread with up to four arguments, and
// then `then` with its v0 and ctx back at the caller's return address, the
// way Kernel::call_guest does. `then` may make the next call. The host
// passes the kernel's.
using ResultFunction = void (*)(psprecomp::AllegrexContext &ctx, std::uint32_t result);
using GuestCaller = void (*)(psprecomp::AllegrexContext &ctx, std::uint32_t function,
    const std::array<std::uint32_t, 4> &arguments, ResultFunction then);

// The bar's words while a meal can be ordered: the game's Stand Up, then △
// and its Order Food, cut on a character to fit kHintBytes with the
// terminator. Printed, △ is the markup for the game's icon; measured, two
// spaces, about its width.
[[nodiscard]] std::string hint_text(const std::string &stand_up, const std::string &order_food, bool printed);

// The menu is open (or the meal being served).
[[nodiscard]] bool menu_open();

// At the flip: puts the wrapper in place each time lobby_task is installed,
// and closes a menu the seated frame no longer runs. Runs every frame.
void frame(psprecomp::Runtime &runtime, GuestCaller caller);

} // namespace mhfu::game::hall_meal
