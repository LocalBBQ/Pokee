#pragma once

#include <cstdint>
#include <span>
#include <string>

namespace psprecomp {
struct AllegrexContext;
class GuestMemory;
class Runtime;
}

// Hiding the game's HUD (issue #184): the health and stamina bars, the clock,
// sharpness, the item bar, the map, name tags and the other labels that
// follow characters, and the prompts in the corners. Menus, dialogs, fades,
// cutscenes and loading screens stay.
//
// How the game's 2D draws were told apart, read from USA Freedom Unite
// (ULUS-10391) and from the lobby_task and game_task overlays, both loaded
// at 0x09A5F300:
//
// - Every 2D element is a packet of GE commands the game files into its
//   display list's ordering table with 0x088596C8 (a0 the list object, a1
//   the packet, a2 its length in words, a3 the bucket). Buckets are drawn in
//   order. The HUD shares its buckets with menus and dialogs, so the bucket
//   alone cannot tell them apart.
// - The code that asks for the packet can. In the village and the Guild Hall,
//   lobby_task draws its text layer from 0x09AE38F8, and its task callback
//   0x09A6C248 is on the stack for the village interface (it calls through
//   the mode at object+44). In a quest, the HUD is a task callback, not the
//   frame function: traced with MHFU_TRACE_HUD in
//   the training quest, the health bar, the map, the item bar and their text
//   are filed while 0x09A60298 is on the stack (through 0x09A62740 and
//   0x09A627B8). 0x09A617A8 and 0x09A62400 file the same sprites for the
//   quest's other HUD states. Name tags in a quest queue their own text from
//   0x09B62488. In the village the same names are drawn with the rest of that
//   interface, so the name size leaves them as the game draws them.
//   Menus are other callbacks and stay. A packet is HUD when one
//   of those is on the guest's call stack when it is filed; the stack is
//   read the way the game's code builds it (each function's addiu sp, sp, -N
//   and sw ra). Each function's first two words are checked, because the two
//   overlays replace each other in the same slot.
// - Most text is not drawn where it is asked for. 0x08A632C8 points at the
//   text context, which keeps six queues, and the game draws every queue at
//   the end of the frame, far from the code that asked. The four functions
//   that append to a queue are watched instead, and the entries added until
//   the next watched call or packet belong to the code on the stack then.
//   When a queue is drawn, the entry each glyph comes from is read off the
//   drawing function's stack frame, and so is the list entry of each button
//   icon in the text.
// - The GE draws a list while the game fills the next elsewhere, so a HUD
//   packet's range is kept until another packet is filed over it.
//
// The renderer then leaves out the draws of HUD packets, and only those,
// while the HUD is hidden. The packets' other commands still run, so the GE
// state they set is the same either way.
namespace mhfu::gpu::hud {

struct CodeWord {
    std::uint32_t address;
    std::uint32_t word;
};

// One function the HUD is drawn from, and the two words that must still be
// there (its addiu sp and its sw ra) before a call stack counts. `quest` is
// game_task's quest HUD, the only draws the interface sizes change. The
// lobby_task functions draw the village interface with its shops and
// dialogs, so they are hidden with the HUD but never resized.
struct HudFunction {
    std::uint32_t address;
    std::uint32_t first;
    std::uint32_t saved_ra;
    bool quest;
};

// The resident code this was read from. Exposed for tests.
[[nodiscard]] std::span<const CodeWord> code_signature();
// The overlay functions. A match also requires the words above.
[[nodiscard]] std::span<const HudFunction> hud_functions();
[[nodiscard]] bool code_matches(const psprecomp::Runtime &runtime);
[[nodiscard]] bool hud_function_matches(const psprecomp::GuestMemory &memory, std::uint32_t address);
// True for a quest HUD function (game_task); the address only.
[[nodiscard]] bool quest_function(std::uint32_t address);
// The name over another hunter in a quest: game_task 0x09B62488, and in an ad
// hoc quest resident 0x088392F4 while game_task is loaded. In the gathering
// hall and the village, the icon resident 0x08835470 files beside a hunter's
// name while lobby_task is loaded, and the name queued after it
// (NameDraw::hall). lobby_task's other labels are drawn with the rest of that
// interface, so they are not names. The words still have to match before a
// call stack counts; this is the address only.
[[nodiscard]] bool name_label(std::uint32_t address);
[[nodiscard]] bool name_tag_only(std::uint32_t address);

// Called at every flip, where guest functions may be wrapped: puts the
// wrappers in place once the HUD is first hidden, any interface size is off
// the game's size, or MHFU_TRACE_HUD asks.
void frame(psprecomp::Runtime &runtime, std::uint64_t presented_frames);

// The player's switch (the Hide HUD bind and the menu).
void set_hidden(bool hidden);
void toggle();
[[nodiscard]] bool hidden();
// Whether the renderer leaves the HUD out now.
[[nodiscard]] bool active();
// False when the game's code is not the code the classification was traced
// on: the switch then does nothing.
[[nodiscard]] bool available();

// A short note after the switch changes: its text, and how long it has left.
[[nodiscard]] double note_seconds_left();
[[nodiscard]] std::string note_text();

// True when a through-mode draw belongs to the HUD, hidden or not, once the
// 2D draws are being watched. The interface's marks go under it
// (VulkanRenderer's HUD cover).
[[nodiscard]] bool is_hud(std::uint32_t command_address, std::uint32_t call_return);
// Set by the renderer while the HUD is drawn over the interface's marks: the
// 2D draws are then watched, so is_hud can tell the HUD apart.
void want_cover(bool wanted);

// True when a through-mode draw belongs to the HUD and the HUD is hidden.
// `command_address` is its PRIM command, `call_return` where the list's
// innermost CALL returns to.
[[nodiscard]] bool hides(std::uint32_t command_address, std::uint32_t call_return);

// A through-mode draw of a name over another hunter, once the 2D draws are
// being watched. `has_origin` is the point the game placed the text (the
// entry's x and y); a plate filed on its own has none, and shrinks about its
// own centre. `tag_only` limits a name to a name-sized draw. The quest name is
// the whole draw, so it is not set. Empty when this draw is not a name.
// `glyph` is a letter of the name's own text, not an icon the label filed: a
// letter takes the Names size wherever it is, and no other piece's rule.
// `hall` is a name over a hunter in the gathering hall: frame interpolation
// moves it with its hunter, and it takes the Names size unless
// MHFU_HUD_HALL_NAMES=0 (hud_scale::sizes_name).
// A gathering-hall label's own pieces, by the call in the label function
// (0x08835470) that files them, traced with MHFU_TRACE_CAPTURE: the weapon
// badge at 0x088357D0, the quest paper at 0x08835C50 and the ready mark at
// 0x08835D0C. The letters are the name's text (NameDraw::glyph).
enum class LabelPart : std::uint8_t { Unknown, Badge, QuestPaper, Ready };
[[nodiscard]] constexpr LabelPart label_part_of_call(std::uint32_t call) {
    switch (call) {
    case 0x088357D0u:
        return LabelPart::Badge;
    case 0x08835C50u:
        return LabelPart::QuestPaper;
    case 0x08835D0Cu:
        return LabelPart::Ready;
    default:
        return LabelPart::Unknown;
    }
}

// `hunter` is the hall hunter the label is drawn for: the object 0x088351F0
// keeps in s0 for each hunter and passes to the label function, which saves
// it at 40(sp). Every piece of one label, its letters included, has it, so a
// label and its marks are told apart by whose they are, not by where they
// are; 0 when not known. `part` is the piece (LabelPart).
struct NameDraw {
    bool is_name{};
    bool has_origin{};
    bool tag_only{};
    bool glyph{};
    float origin_x{};
    float origin_y{};
    bool hall{};
    std::uint32_t hunter{};
    LabelPart part{LabelPart::Unknown};
};
[[nodiscard]] NameDraw name_at(std::uint32_t command_address, std::uint32_t call_return);

// Who filed a 2D draw, for the interface sizes (gpu/hud_scale.hpp). The size
// rules recognise HUD pieces by their rectangles, and menus draw panels of
// the same size in the same corners; only a QuestHud draw may be resized.
// - Unknown: the game's code is not the code this was read from, so the
//   draws cannot be told apart and the rectangles alone decide.
// - QuestHud: filed while a game_task HUD function was on the stack.
// - Other: anything else, including every village and Guild Hall draw, and
//   every draw before the wrappers are in place.
enum class Owner { Unknown, QuestHud, Other };
[[nodiscard]] Owner owner_of(std::uint32_t command_address, std::uint32_t call_return);

// Which part of the quest HUD drew a QuestHud draw, read from the same call
// stack. Traced with MHFU_TRACE_CAPTURE in two ad hoc instances, across the
// bowgun, bow, horn, gunlance and long sword, the camp, the area map with and
// without the Map item, the ammo menu, the item box, the quest menu, Combine,
// the obtain banner and a teammate leaving: every quest HUD packet is filed
// under the dispatcher 0x0883F270, and each piece of the HUD is its own
// function below it. The menus, banners and the item box the game draws in
// the same corners are other functions below it, so they never share a site
// with a piece. The innermost function on the stack that is in the table
// (site_functions()) decides, so a name drawn from the party list's code is
// still a name.
// - None: not a QuestHud draw, or not watched.
// - Unlisted: the quest HUD, from no function in the table. Left as drawn.
// - Window: a menu, banner, prompt or the item box over the HUD. Left as drawn.
// - Names: a name over a hunter, which NameDraw sizes. No other size.
// - Status, Weapon, Map, ItemBar, AmmoBar: the pieces the interface sizes move.
enum class Site : std::uint8_t { None, Unlisted, Window, Names, Status, Weapon, Map, ItemBar, AmmoBar };
[[nodiscard]] inline const char *site_name(Site site) {
    switch (site) {
    case Site::None:
        return "none";
    case Site::Unlisted:
        return "unlisted";
    case Site::Window:
        return "window";
    case Site::Names:
        return "names";
    case Site::Status:
        return "status";
    case Site::Weapon:
        return "weapon";
    case Site::Map:
        return "map";
    case Site::ItemBar:
        return "items";
    case Site::AmmoBar:
        return "ammo-bar";
    }
    return "none";
}
[[nodiscard]] Site site_of(std::uint32_t command_address, std::uint32_t call_return);

// A draw's site, and the call in the site's function that led to it: the
// return address into that function. The sharpness gauge tells its bar, cap,
// spirit fill and colour apart by it (game/sharpness_gauge.hpp). The call is 0
// for text, which is drawn away from the code that queued it.
struct SiteDraw {
    Site site{Site::None};
    std::uint32_t call{};
};
[[nodiscard]] SiteDraw site_draw_of(std::uint32_t command_address, std::uint32_t call_return);

// Whether sites can be read: the draws are watched, the code is the code the
// table was read from, and MHFU_HUD_SCALE_SITES=0 has not turned them off.
// Every user of sites falls back to the rectangles without them.
[[nodiscard]] bool sites_on();
// Whether the quest HUD's left column (Site::Status, where the clock is) was
// filed this frame or the one before: the quest HUD is on screen. The hunt
// icons follow it. Meaningful only while sites_on().
[[nodiscard]] bool quest_hud_filed();

// One function of the site table, and its first word (addiu sp, sp, -N),
// which must still be there when the stack is walked.
struct SiteFunction {
    std::uint32_t address;
    std::uint32_t first;
    Site site;
};
// Exposed for tests.
[[nodiscard]] std::span<const SiteFunction> site_functions();

// MHFU_TRACE_CAPTURE=1: each screenshot also traces the frames around it, so
// one run can catch a moment that cannot be given as a frame number in
// advance (another player's name in view, a menu over the HUD). The 2D
// packets and text calls of the two frames after the screenshot are traced as
// MHFU_TRACE_HUD does. Each list is drawn in the frame after it is built, so
// the sprites of the two frames after those are traced as MHFU_TRACE_SPRITES
// does. The first frame after the request is skipped: it is where the
// wrappers go in when nothing else needed them yet.
struct CaptureSchedule {
    static constexpr std::uint64_t kNone = ~0ull;
    static constexpr std::uint64_t kFrames = 2u;
    bool requested{};
    std::uint64_t first{kNone}; // the first frame whose packets are traced

    void request() { requested = true; }
    // At each flip, with the frame that starts.
    void at_flip(std::uint64_t frame) {
        if (!requested) return;
        requested = false;
        first = frame + 1u;
    }
    [[nodiscard]] bool pending() const { return requested || first != kNone; }
    [[nodiscard]] bool packets(std::uint64_t frame) const {
        return first != kNone && frame >= first && frame < first + kFrames;
    }
    [[nodiscard]] bool sprites(std::uint64_t frame) const {
        return first != kNone && frame >= first + 1u && frame < first + 1u + kFrames;
    }
};

// Arms a capture when MHFU_TRACE_CAPTURE is set; called with each screenshot.
void screenshot_taken();
// Whether the renderer traces the sprites of presented frame `frame`.
[[nodiscard]] bool capture_sprites(std::uint64_t frame);

// Set by the renderer while frame interpolation records frames: the names
// over hunters are then told apart at any size, so the presents between game
// frames can move them with the characters they follow.
void want_names(bool wanted);

// What the wrapper does when the game files a packet (0x088596C8). Exposed
// for tests, which build a stand-in call stack in guest memory.
void packet_filed(psprecomp::Runtime &runtime, const psprecomp::AllegrexContext &ctx);
// The name over a teammate in an ad hoc quest is queued while resident
// 0x08842628 runs, by a path no text wrapper sees. Its wrapper calls these
// around the game's function: text it queues while the quest HUD is on the
// stack is a name for the Names size. Exposed for tests.
void overhead_name_begin(psprecomp::Runtime &runtime, const psprecomp::AllegrexContext &ctx);
void overhead_name_end(psprecomp::Runtime &runtime);
// Has owner_of() answer as it does once the wrappers are in place, and
// forgets every packet seen. For tests.
void watch_for_tests();
// Has the next packets and text be traced as MHFU_TRACE_HUD traces them. For
// tests.
void trace_for_tests();

} // namespace mhfu::gpu::hud
