#include "gpu/game_hud.hpp"

#include "settings/settings.hpp"

#include "psprecomp/runtime.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

// The generated units the wrapped functions were compiled into, found by
// CMake in the corpus (MHFU_HUD_FUNCTIONS in CMakeLists.txt).
#if defined(MHFU_HUD_FUNCTIONS_INC)
namespace psprecomp {
#define MHFU_HUD_FUNCTION(address, unit) void unit(Runtime &, AllegrexContext &);
#include MHFU_HUD_FUNCTIONS_INC
#undef MHFU_HUD_FUNCTION
} // namespace psprecomp
#endif

namespace mhfu::gpu::hud {
namespace {

using Clock = std::chrono::steady_clock;

struct Compiled {
    std::uint32_t address;
    psprecomp::Runtime::RecompiledFunction function;
};
constexpr Compiled kCompiled[] = {
#if defined(MHFU_HUD_FUNCTIONS_INC)
#define MHFU_HUD_FUNCTION(address, unit) {address, &psprecomp::unit},
#include MHFU_HUD_FUNCTIONS_INC
#undef MHFU_HUD_FUNCTION
#endif
    {0u, nullptr},
};

psprecomp::Runtime::RecompiledFunction compiled(std::uint32_t address) {
    for (const Compiled &entry : kCompiled)
        if (entry.address == address) return entry.function;
    return nullptr;
}

// Files a packet into the display list's ordering table: a0 the list object,
// a1 the packet, a2 its length in words, a3 the bucket. Leaf: sll v0, a3, 2
// then sw a2, 0x148(t0), the new tail of that bucket.
constexpr std::uint32_t kFilePacket = 0x088596C8u;

// The four functions that append to a text queue (each stores the new length
// at +308), and 0x0889198C, which queues text twice through 0x088915B0. It is
// compiled into the same unit, so those two calls are plain jumps that never
// reach a wrapper, and the text it queues was counted with whatever call came
// before. Traced with MHFU_TRACE_CAPTURE in an ad hoc quest: the name over a
// teammate was a third entry after the party list's two, with no watched
// call for it. Nothing else writes that length except the reset.
constexpr std::array<std::uint32_t, 5> kTextFunctions = {
    0x08891474u,
    0x088915B0u,
    0x08891700u,
    0x0889183Cu,
    0x0889198Cu,
};
// Called from the per-character draw (0x0889377C) after each character, with
// the entry still in s3: the moment button icons the character added to the
// queue's icon list can be told to belong to that entry.
constexpr std::uint32_t kCharacterDrawn = 0x088952A4u;
constexpr std::uint32_t kCharacterDrawnFromQueue = 0x08893860u; // its ra there
constexpr std::size_t kWrapped = 2u + kTextFunctions.size();

constexpr std::uint32_t wrapped_address(std::size_t index) {
    return index == 0u ? kFilePacket : index == 1u ? kCharacterDrawn : kTextFunctions[index - 2u];
}

// The functions the HUD is drawn from. Both overlays load at 0x09A5F300, so
// the words are checked before a match counts: the other overlay's bytes at
// the same address are not this function.
constexpr std::array<HudFunction, 6> kHudFunctions = {{
    {0x09AE38F8u, 0x27BDFFF0u, 0xAFBF000Cu, false}, // lobby_task text: addiu sp,-16; sw ra,12(sp)
    {0x09A6C248u, 0x27BDFFF0u, 0xAFBF000Cu, false}, // lobby_task callback: addiu sp,-16; sw ra,12(sp)
    {0x09A60298u, 0x27BDFFA0u, 0xAFBF001Cu, true},  // game_task HUD: addiu sp,-96; sw ra,28(sp)
    {0x09A617A8u, 0x27BDFFE0u, 0xAFBF000Cu, true},  // game_task HUD state: addiu sp,-32; sw ra,12(sp)
    {0x09A62400u, 0x27BDFFF0u, 0xAFBF000Cu, true},  // game_task HUD state: addiu sp,-16; sw ra,12(sp)
    {0x09B62488u, 0x27BDFFA0u, 0xAFBF001Cu, true},  // game_task labels: addiu sp,-96; sw ra,28(sp)
}};
// The name over another hunter in a quest, and the mark beside it. The first
// two words have to match: the other overlay occupies the same addresses.
//
// The village has no separate name. lobby_task draws its labels from the same
// functions that file the rest of that screen (0x09A6C248 calls through the
// mode at object+44, and 0x09A80660 draws the label together with the
// surrounding interface). Sizing any of those moves the shops, prompts and
// dialogs too, so they stay at the size the game draws. The quest labels draw
// the name and its mark and nothing else.
//
// The names over other hunters in an ad hoc quest are not drawn from there.
// Traced with MHFU_TRACE_CAPTURE: resident 0x088392F4 walks up to 20
// characters, projects each one's position to the screen, and queues its
// name centred on that point through 0x0889198C. It is called from 0x088345FC,
// outside the quest HUD's callbacks. Being resident, it is in memory in the
// village too, so it counts only while game_task's quest HUD is loaded
// (`quest_only`): the names there stay at the size the game draws.
//
// In the gathering hall, traced with MHFU_TRACE_CAPTURE: lobby_task's
// callback (0x09A6C248) calls resident 0x088351F0 for each hunter, which
// files the weapon icon beside the name through 0x08835470; the name itself is
// queued by a path no text wrapper sees. Text queued after that icon is the
// name. The same call draws the player's own name in the village, and there
// 0x088351F0 also draws the prompt over the hunter (":Open Item Box", its
// button and the bar's end) through 0x08835F7C, so only 0x08835470 counts:
// matched on 0x088351F0, the prompt's small pieces were sized as the name.
// It counts only while lobby_task is loaded (`lobby_only`), and only when it is
// the size of a tag (`tag_only`), so a window queued after the last name
// keeps its size. It moves with its hunter under frame interpolation and
// takes the Names size (`hall`), unless MHFU_HUD_HALL_NAMES=0 leaves it as the
// game draws it.
struct NameFunction {
    std::uint32_t address;
    std::uint32_t first;
    std::uint32_t saved_ra;
    bool tag_only;
    bool quest_only;
    bool lobby_only;
    bool hall;
};
constexpr NameFunction kNameFunctions[] = {
    {0x09B62488u, 0x27BDFFA0u, 0xAFBF001Cu, false, false, false, false}, // game_task labels: addiu sp,-96; sw ra,28(sp)
    {0x088392F4u, 0x27BDFF60u, 0xAFBF002Cu, false, true, false,
        false}, // names over characters: addiu sp,-160; sw ra,44(sp)
    // The hall labels, all of them in one call: addiu sp,-176; sw ra,76(sp).
    {0x08835470u, 0x27BDFF50u, 0xAFBF004Cu, true, false, true, true},
};
// More frames than any call stack the game files a packet from; it only stops
// a walk that has gone off the end of a broken stack.
constexpr int kStackFrames = 64;

// The quest HUD function whose words show that game_task is loaded.
constexpr std::uint32_t kQuestLoaded = 0x09A60298u;
// The lobby callback whose words show that lobby_task is loaded.
constexpr std::uint32_t kLobbyLoaded = 0x09A6C248u;

// The quest HUD's parts by the resident function that draws them (Site in
// game_hud.hpp), traced with MHFU_TRACE_CAPTURE. The quest HUD (0x09A60298)
// reaches the dispatcher 0x0883F270 through 0x09A62740 and 0x09A627B8; the
// dispatcher calls the left column, the right column and the pouch's panels.
// Each function's first word is its addiu sp, sp, -N.
constexpr std::array<SiteFunction, 19> kSiteFunctions = {{
    // The left column: the clock, health and stamina, the party list and the
    // weapon's gauges. Whatever else it draws stays with them.
    {0x0883F9F8u, 0x27BDFFF0u, Site::Status},
    {0x08840338u, 0x27BDFFE0u, Site::Status}, // the clock
    {0x088405E4u, 0x27BDFFD0u, Site::Status}, // health and stamina, their caps and fills
    {0x08841908u, 0x27BDFFD0u, Site::Status}, // the party list: plates, names, badges, buffs
    {0x08832B98u, 0x27BDFFE0u, Site::Names},  // a name over a hunter, from the party list's code
    // Sharpness, the spirit gauge, a horn's notes, loaded rounds, a bow's
    // coatings and a gunlance's shells.
    {0x08841100u, 0x27BDFFD0u, Site::Weapon},
    // The right column. Its quest menu, hint bar and description box, and the
    // Combine windows, are its own functions, so the column itself is not
    // listed.
    {0x08842EE8u, 0x27BDFFF0u, Site::Map}, // the area map, its window and its marks
    // The map's drawing, under 0x08842EE8. Listed so that the call recorded
    // with a map draw is the one in here that names the piece: the tile at
    // 0x088432E8, the window at 0x08843610 (hud_scale::is_map_picture_call).
    {0x08842F70u, 0x27BDFF80u, Site::Map},
    // The item bar and the item's name, with the bowgun's ammo menu that
    // opens over it (0x08845EA8).
    {0x08845A34u, 0x27BDFFE0u, Site::ItemBar},
    {0x088474DCu, 0x27BDFFC0u, Site::AmmoBar}, // a bowgun's ammo bar: the shot and its count
    {0x088471D8u, 0x27BDFFB0u, Site::AmmoBar}, // the ammo bar's frame
    {0x08846FACu, 0x27BDFFF0u, Site::ItemBar}, // the ammo menu's arrow and highlight
    {0x088172D8u, 0x27BDFF50u, Site::Window},  // Combine's windows
    {0x08818C94u, 0x27BDFF90u, Site::Window},  // the quest menu
    {0x08818F74u, 0x27BDFFF0u, Site::Window},  // its hint bar ("Beacon", "Map Zoom In")
    {0x08819128u, 0x27BDFFE0u, Site::Window},  // its description box
    {0x08847CB0u, 0x27BDFF70u, Site::Window},  // the item box ("Please select an item.")
    {0x08823A50u, 0x27BDFFF0u, Site::Window},  // a banner: an item obtained, a teammate leaving
    {0x088DB068u, 0x27BDFFF0u, Site::Window},  // a bow's or a bowgun's reticle
}};

// The text context, and in it each queue's length (u8 at +308 + queue) and
// entries (a pointer at +320 + 4 * queue, entries of 12 bytes). The flush
// loads it from 0x08A632C8.
constexpr std::uint32_t kTextContextPointer = 0x08A632C8u;
constexpr std::uint32_t kQueueLengths = 308u;
constexpr std::uint32_t kQueueEntries = 320u;
constexpr std::uint32_t kQueues = 6u;
constexpr std::uint32_t kEntryBytes = 12u;
// Button icons in queued text are listed per queue while it is drawn (count
// u8 at +314 + queue, entries of 8 bytes from +0x22B18 + 80 * queue).
// 0x08892A88 walks that list in s4 and files each icon's packets; the return
// addresses of those two filing calls are below, and s4 is the entry then.
constexpr std::uint32_t kIconCounts = 314u;
constexpr std::uint32_t kIconLists = 0x22B18u;
constexpr std::uint32_t kIconListStride = 80u;
constexpr std::uint32_t kIconBytes = 8u;
constexpr std::uint32_t kIconFiled = 0x08892C88u;
constexpr std::uint32_t kIconFiledAgain = 0x08892E2Cu;

// Drawing one queued character (0x0889377C) calls 0x08893894 for its glyph
// at 0x08893844, with the entry in s3. 0x08893894 saves that s3 at sp+20 and
// ra at sp+44, and files the glyph's packet at 0x08893C24.
constexpr std::uint32_t kGlyphFiled = 0x08893C2Cu;     // ra of the filing call
constexpr std::uint32_t kGlyphFromQueue = 0x0889384Cu; // 0x08893894's ra from the character draw
constexpr std::uint32_t kGlyphSavedRa = 44u;
constexpr std::uint32_t kGlyphSavedEntry = 20u;

// The name over a teammate in an ad hoc quest. Traced with
// MHFU_TRACE_TEXT_FROM: the quest HUD's widget renderer 0x0883F9F8 calls
// resident 0x08842628, and the name is queued while it runs, at the point
// over that hunter. The append it reaches is not a call any wrapper sees, so
// the entry was counted with the party list's name queued before it, and its
// letters took the party list's sizes wherever they crossed a row of it.
constexpr std::uint32_t kOverheadName = 0x08842628u;

// What the addresses above were read off; the switch stays off if any differs.
// The overlay functions are not here: they are not loaded at startup.
constexpr CodeWord kSignature[] = {
    {0x088596C8u, 0x00071080u}, // sll v0, a3, 2: the ordering table insert
    {0x0885971Cu, 0xAD060148u}, // sw a2, 0x148(t0): the bucket's new tail
    {0x08893894u, 0x27BDFFB0u}, // addiu sp, sp, -80: drawing a glyph
    {0x08893898u, 0xAFBF002Cu}, // sw ra, 44(sp)
    {0x088938B0u, 0xAFB30014u}, // sw s3, 20(sp): the caller's entry
    {0x08893C24u, 0x0E2165B2u}, // jal 0x088596C8
    {0x08893844u, 0x0E224E25u}, // jal 0x08893894, from the character draw
    {0x08891A60u, 0x90720134u}, // lbu s2, 308(v1): a queue's length
    {0x08891A7Cu, 0x8C740140u}, // lw s4, 320(v1): its entries
    {0x08891AACu, 0x9072013Au}, // lbu s2, 314(v1): its icon count
    {0x08891AD8u, 0x34212B18u}, // ori at, at, 0x2B18: the icon lists at +0x22B18
    {0x0888FDD0u, 0x3C0208A6u}, // lui v0, 0x8a6
    {0x0888FDD8u, 0x8C4432C8u}, // lw a0, 0x32C8(v0): the text context
    {0x08893858u, 0x0E2254A9u}, // jal 0x088952A4 after each character
    {0x088952A4u, 0x90820130u}, // lbu v0, 0x130(a0)
    {0x08892C80u, 0x0E2165B2u}, // jal 0x088596C8, an icon's first packet
    {0x08892E24u, 0x0E2165B2u}, // jal 0x088596C8, an icon's second packet
    {0x08891474u, 0x27BDFCA0u}, // addiu sp, sp, -864: a queue append
    {0x088915B0u, 0x27BDFC80u}, // addiu sp, sp, -896
    {0x08891700u, 0x27BDFCA0u}, // addiu sp, sp, -864
    {0x0889183Cu, 0x27BDFC80u}, // addiu sp, sp, -896
    {0x0889198Cu, 0x27BDFFE0u}, // addiu sp, sp, -32: text queued twice
    {0x08891990u, 0xAFBF001Cu}, // sw ra, 28(sp)
    {0x08842628u, 0x27BDFF70u}, // addiu sp, sp, -144: the name over a teammate
    {0x0884262Cu, 0xAFBF002Cu}, // sw ra, 44(sp)
};

// Where a function starts, found from a return address into it.
struct Frame {
    std::uint32_t start;
    std::uint32_t size;
    std::uint32_t ra_offset; // ~0u: the function does not save ra
    std::uint32_t start_word;
};

struct State {
    bool hidden{};
    bool installed{};
    bool checked{};
    bool available{};
    std::string note;
    Clock::time_point note_until{};
    std::array<psprecomp::Runtime::RecompiledFunction, kWrapped> originals{};
    // MHFU_TRACE_TEXT_FROM: the functions probed, the generated code each
    // replaces, and how many lines are left to print.
    struct Probe {
        std::uint32_t address{};
        psprecomp::Runtime::RecompiledFunction original{};
    };
    std::array<Probe, 4> probes{};
    // kOverheadName: the generated code it replaces, and whether the call
    // running now came from the quest HUD.
    psprecomp::Runtime::RecompiledFunction overhead_original{};
    bool overhead_quest{};
    bool probes_checked{};
    int probe_lines_left{400};
    std::set<std::string> probe_seen;
    // HUD packets by start address, to their end, and whether the quest HUD
    // filed them. The GE draws a list while the game fills the next one
    // elsewhere, so a packet's range is kept until another packet is filed
    // over it.
    struct Packet {
        std::uint32_t end{};
        bool quest{};
        Site site{};
        std::uint32_t call{};
    };
    std::map<std::uint32_t, Packet> packets;
    // A text queue entry or a button icon the HUD asked for, and whether the
    // quest HUD did.
    struct HudEntry {
        std::uint32_t address{};
        bool quest{};
        Site site{};
    };
    // Text queue entries the HUD asked for, and the queues' lengths at the
    // last watched call.
    std::vector<HudEntry> text_entries;
    std::array<std::uint8_t, kQueues> lengths{};
    // Text queued by a name label, and the point the game placed it (the
    // entry's x and y). Icons of that text keep the same point. `tag_only`
    // is a lobby name: the draw is scaled only when it is the size of a tag.
    // `hall` is a gathering-hall name (NameDraw::hall).
    struct NameEntry {
        std::uint32_t address{};
        bool tag_only{};
        bool hall{};
        std::uint32_t label{};
    };
    std::vector<NameEntry> name_entries;
    struct NamePacket {
        std::uint32_t end{};
        bool has_origin{};
        bool tag_only{};
        bool glyph{};
        float x{};
        float y{};
        bool hall{};
        std::uint32_t label{};
        LabelPart part{};
    };
    std::map<std::uint32_t, NamePacket> name_packets;
    // A hall name's icon packets, filed before the name is queued: they take
    // its point once it is.
    std::vector<std::uint32_t> unplaced_packets;
    struct IconOrigin {
        float x{};
        float y{};
        bool tag_only{};
        bool hall{};
        std::uint32_t label{};
    };
    std::unordered_map<std::uint32_t, IconOrigin> icon_origin;
    // Icons in HUD text, and each queue's icon count when last looked at.
    std::vector<HudEntry> icons;
    std::array<std::uint8_t, kQueues> icon_counts{};
    std::uint64_t icon_frame{~0ull};
    bool last_call_hud{};
    bool last_call_quest{};
    Site last_call_site{};
    // A Status packet filed this frame, and in the frame before (quest_hud_filed).
    bool status_now{};
    bool status_live{};
    bool last_call_name{};
    std::uint32_t last_call_label{};
    // The hall labels started this frame (NameDraw::label), and the piece of
    // one filed last: a badge after any other piece starts the next label.
    std::uint32_t labels{};
    LabelPart label_part{};
    bool last_call_tag_only{};
    bool last_call_hall{};
    std::array<bool, kHudFunctions.size()> hud_word_logged{};
    std::unordered_map<std::uint32_t, Frame> frames;
    // MHFU_TRACE_HUD
    std::uint64_t frame{};
    std::uint64_t trace_first{~0ull};
    std::uint64_t trace_every{};
    // MHFU_TRACE_CAPTURE
    bool capture_on{};
    CaptureSchedule capture;
    // Frame interpolation wants the names over hunters (want_names).
    bool names_wanted{};
    // The renderer draws the HUD over the interface's marks (want_cover).
    bool cover_wanted{};
};

State &state() {
    static State s;
    return s;
}

bool tracing(const State &s) {
    if (s.frame == s.trace_first || s.capture.packets(s.frame)) return true;
    return s.trace_every != 0u && s.frame > s.trace_first && (s.frame - s.trace_first) % s.trace_every == 0u;
}

// The frame of the function a return address points into: the nearest
// addiu sp, sp, -N before it, and where after that it saves ra.
const Frame *frame_of(State &s, const psprecomp::GuestMemory &memory, std::uint32_t ra) {
    if (auto found = s.frames.find(ra); found != s.frames.end()) {
        // Overlays replace code: the cached start must still hold what it did.
        if (memory.load32(found->second.start) == found->second.start_word) return &found->second;
        s.frames.erase(found);
    }
    Frame frame{};
    std::uint32_t at = (ra - 8u) & ~3u;
    for (std::uint32_t steps = 0u;; at -= 4u, ++steps) {
        if (steps == 4096u || !memory.contains(at, 4u)) return nullptr;
        const std::uint32_t word = memory.load32(at);
        if ((word >> 16u) == 0x27BDu && (word & 0x8000u) != 0u) {
            frame.start = at;
            frame.start_word = word;
            frame.size = 0x10000u - (word & 0xFFFFu);
            break;
        }
    }
    frame.ra_offset = ~0u;
    for (std::uint32_t pc = frame.start + 4u, steps = 0u; steps < 64u && memory.contains(pc, 4u); pc += 4u, ++steps) {
        const std::uint32_t word = memory.load32(pc);
        if ((word >> 16u) == 0xAFBFu) {
            frame.ra_offset = word & 0xFFFFu;
            break;
        }
        if (word == 0x03E00008u) break; // jr ra: returns without saving it
    }
    if (s.frames.size() > 4096u) s.frames.clear();
    return &s.frames.emplace(ra, frame).first->second;
}

// The HUD function at `address`, when it still begins with the words it was
// read from. A mismatch is said once: the other overlay may occupy the slot,
// and that is not this function.
const HudFunction *hud_function_here(State &s, const psprecomp::GuestMemory &memory, std::uint32_t address) {
    for (std::size_t i = 0; i < kHudFunctions.size(); ++i) {
        const HudFunction &fn = kHudFunctions[i];
        if (fn.address != address) continue;
        const std::uint32_t first = memory.load32(address);
        const std::uint32_t saved = memory.load32(address + 4u);
        if (first == fn.first && saved == fn.saved_ra) return &fn;
        if (!s.hud_word_logged[i]) {
            s.hud_word_logged[i] = true;
            std::cout << "[hud] unexpected code at 0x" << std::hex << address << ": 0x" << first << " 0x" << saved
                      << std::dec << "; not counted as the HUD\n";
        }
        return nullptr;
    }
    return nullptr;
}

// A name label still has to begin with the words it was read from. The quest
// overlay and the lobby overlay share these addresses.
const NameFunction *name_function_here(const psprecomp::GuestMemory &memory, std::uint32_t address) {
    for (const NameFunction &fn : kNameFunctions) {
        if (fn.address != address) continue;
        if (memory.load32(address) != fn.first || memory.load32(address + 4u) != fn.saved_ra) return nullptr;
        if (fn.quest_only && !hud_function_matches(memory, kQuestLoaded)) return nullptr;
        if (fn.lobby_only && !hud_function_matches(memory, kLobbyLoaded)) return nullptr;
        return &fn;
    }
    return nullptr;
}

// The site table's part for a frame of the stack, or Site::None when its
// function is not listed or no longer begins with the word it was read from.
Site site_here(const Frame &frame) {
    for (const SiteFunction &fn : kSiteFunctions)
        if (fn.address == frame.start) return fn.first == frame.start_word ? fn.site : Site::None;
    return Site::None;
}

// Whether a HUD function is on the guest's call stack; the chain of return
// addresses and function starts goes to `line` when one is given. `quest` is
// whether the innermost HUD function on the stack is the quest HUD's, and
// `site` which part of it drew (Site::None when it is not the quest HUD's).
// For a name label, `name_call` is the call in it that led here.
bool from_hud(State &s, const psprecomp::GuestMemory &memory, const psprecomp::AllegrexContext &ctx, std::string *line,
    bool *name, bool *tag_only, bool *quest, bool *hall = nullptr, Site *site = nullptr, std::uint32_t *call = nullptr,
    std::uint32_t *name_call = nullptr) {
    std::uint32_t ra = ctx.gpr[31];
    std::uint32_t sp = ctx.gpr[29];
    bool hud = false;
    bool from_quest = false;
    bool found_name = false;
    std::uint32_t label_call = 0u;
    bool limit = false;
    bool in_hall = false;
    Site found_site = Site::None;
    std::uint32_t found_call = 0u;
    // To the end of the stack. A cap of 12 frames once cut chains one frame
    // short of the quest HUD: traced with MHFU_TRACE_CAPTURE, a bowgun's
    // reticle and the Combine window's deepest pieces are filed 13 frames
    // down, so they were the quest HUD's on some frames and not on others.
    // kStackFrames only stops a walk through a broken stack.
    for (int depth = 0; depth < kStackFrames; ++depth) {
        const Frame *frame = frame_of(s, memory, ra);
        if (frame == nullptr) break;
        if (line != nullptr) {
            char text[24];
            std::snprintf(text, sizeof(text), " %08X@%08X", ra, frame->start);
            *line += text;
        }
        // The quest label is the name. The village callback is a HUD function
        // and stays on the stack for that whole interface, so the walk stops
        // there without treating it as a name.
        if (!found_name)
            if (const NameFunction *named = name_function_here(memory, frame->start)) {
                found_name = true;
                limit = named->tag_only;
                in_hall = named->hall;
                // `ra` is the call in it.
                label_call = ra;
            }
        // The innermost listed function: the site table's functions are all
        // below the quest HUD's, so they are met before it.
        if (found_site == Site::None && !hud) {
            found_site = site_here(*frame);
            if (found_site != Site::None) found_call = ra;
        }
        if (!hud)
            if (const HudFunction *fn = hud_function_here(s, memory, frame->start)) {
                hud = true;
                from_quest = fn->quest;
                if (line == nullptr) break;
            }
        if (frame->ra_offset == ~0u || !memory.contains(sp + frame->ra_offset, 4u)) break;
        ra = memory.load32(sp + frame->ra_offset);
        sp += frame->size;
    }
    if (name != nullptr) *name = found_name;
    if (tag_only != nullptr) *tag_only = limit;
    if (quest != nullptr) *quest = from_quest;
    if (hall != nullptr) *hall = in_hall;
    if (name_call != nullptr) *name_call = label_call;
    if (site != nullptr)
        *site = !hud || !from_quest ? Site::None : found_site == Site::None ? Site::Unlisted : found_site;
    if (call != nullptr) *call = hud && from_quest ? found_call : 0u;
    return hud;
}

const State::NameEntry *find_name(const State &s, std::uint32_t entry) {
    for (const State::NameEntry &e : s.name_entries)
        if (e.address == entry) return &e;
    return nullptr;
}

// The text entry's x and y, the point 0x08890DFC stored before the glyphs.
bool entry_origin(const psprecomp::GuestMemory &memory, std::uint32_t entry, float &x, float &y) {
    if (!memory.contains(entry, 4u)) return false;
    x = static_cast<float>(static_cast<std::int16_t>(memory.load16(entry)));
    y = static_cast<float>(static_cast<std::int16_t>(memory.load16(entry + 2u)));
    return true;
}

// MHFU_TRACE_HUD: the entries [from, to) of a queue, with the point each is
// drawn at, credited to the call traced on the line above, whatever it was.
// A glyph's packet names its entry, so this ties the glyph to that call.
void trace_entries(const psprecomp::GuestMemory &memory, std::uint64_t frame, std::uint32_t queue,
    std::uint32_t entries, std::uint32_t from, std::uint32_t to, const char *by) {
    std::cout << "[hud] f=" << frame << " queued q=" << queue << " by=" << by << " |";
    for (std::uint32_t i = from; i < to; ++i) {
        const std::uint32_t at = entries + i * kEntryBytes;
        float x = 0.0f;
        float y = 0.0f;
        char text[40];
        if (entry_origin(memory, at, x, y))
            std::snprintf(text, sizeof(text), " %08X(%g,%g)", at, x, y);
        else
            std::snprintf(text, sizeof(text), " %08X", at);
        std::cout << text;
    }
    std::cout << "\n";
}

// MHFU_TRACE_HUD: the argument and saved registers at a watched call, for
// telling apart draws that share a call chain (a widget, a row, an id).
std::string registers_text(const psprecomp::AllegrexContext &ctx) {
    char text[160];
    std::snprintf(text, sizeof(text), " a=%08X,%08X,%08X,%08X s=%08X,%08X,%08X,%08X,%08X,%08X,%08X,%08X", ctx.gpr[4],
        ctx.gpr[5], ctx.gpr[6], ctx.gpr[7], ctx.gpr[16], ctx.gpr[17], ctx.gpr[18], ctx.gpr[19], ctx.gpr[20],
        ctx.gpr[21], ctx.gpr[22], ctx.gpr[23]);
    return text;
}

// The queue entries added since the last watched call belong to the code
// that made it: to the HUD when a HUD function was on the stack then.
void settle_text(State &s, const psprecomp::GuestMemory &memory) {
    const std::uint32_t context = memory.load32(kTextContextPointer);
    if (!memory.contains(context + kQueueLengths, kQueues) || !memory.contains(context + kQueueEntries, kQueues * 4u))
        return;
    for (std::uint32_t queue = 0; queue < kQueues; ++queue) {
        const std::uint8_t length = memory.load8(context + kQueueLengths + queue);
        const std::uint32_t entries = memory.load32(context + kQueueEntries + queue * 4u);
        std::uint8_t &known = s.lengths[queue];
        if (length < known) {
            // Drawn and emptied: its entries are someone else's from now on.
            const std::uint32_t end = entries + 256u * kEntryBytes;
            const auto gone = [&](std::uint32_t e) { return e >= entries && e < end; };
            std::erase_if(s.text_entries, [&](const State::HudEntry &e) { return gone(e.address); });
            std::erase_if(s.name_entries, [&](const State::NameEntry &e) { return gone(e.address); });
            known = 0u;
        }
        if (length > known && tracing(s)) trace_entries(memory, s.frame, queue, entries, known, length, "above");
        // A hall name's text ends its label's badge: the next badge starts
        // the next label (NameDraw::label).
        if (s.last_call_name && s.last_call_hall && length > known) s.label_part = LabelPart::Unknown;
        if (s.last_call_hud || s.last_call_name)
            for (std::uint32_t i = known; i < length; ++i) {
                const std::uint32_t at = entries + i * kEntryBytes;
                if (s.last_call_hud) s.text_entries.push_back(State::HudEntry{at, s.last_call_quest, s.last_call_site});
                if (s.last_call_name)
                    s.name_entries.push_back(
                        State::NameEntry{at, s.last_call_tag_only, s.last_call_hall, s.last_call_label});
            }
        // A hall name's icon was filed before its text: it moves with the
        // point the text was placed at.
        float x = 0.0f;
        float y = 0.0f;
        if (s.last_call_name && s.last_call_hall && length > known && !s.unplaced_packets.empty() &&
            entry_origin(memory, entries + known * kEntryBytes, x, y)) {
            for (const std::uint32_t packet : s.unplaced_packets)
                if (auto found = s.name_packets.find(packet); found != s.name_packets.end()) {
                    found->second.has_origin = true;
                    found->second.x = x;
                    found->second.y = y;
                }
            s.unplaced_packets.clear();
        }
        known = length;
    }
}

const State::HudEntry *find_entry(const std::vector<State::HudEntry> &entries, std::uint32_t address) {
    for (const State::HudEntry &e : entries)
        if (e.address == address) return &e;
    return nullptr;
}

void note_packet(
    State &s, std::uint32_t packet, std::uint32_t words, bool hud, bool quest, Site site, std::uint32_t call) {
    const std::uint32_t end = packet + words * 4u;
    // Whatever was filed where this packet now lies is gone.
    auto it = s.packets.lower_bound(packet);
    if (it != s.packets.begin() && std::prev(it)->second.end > packet) --it;
    while (it != s.packets.end() && it->first < end) it = s.packets.erase(it);
    if (hud) s.packets.emplace(packet, State::Packet{end, quest, site, call});
    if (s.packets.size() > 65536u) s.packets.clear(); // never expected; bounds a runaway
}

void note_name_packet(State &s, std::uint32_t packet, std::uint32_t words, bool name, bool has_origin, bool tag_only,
    bool glyph, float x, float y, bool hall, std::uint32_t label, LabelPart part) {
    const std::uint32_t end = packet + words * 4u;
    auto it = s.name_packets.lower_bound(packet);
    if (it != s.name_packets.begin() && std::prev(it)->second.end > packet) --it;
    while (it != s.name_packets.end() && it->first < end) it = s.name_packets.erase(it);
    if (name)
        s.name_packets.emplace(packet, State::NamePacket{end, has_origin, tag_only, glyph, x, y, hall, label, part});
    if (s.name_packets.size() > 65536u) s.name_packets.clear();
}

} // namespace

void packet_filed(psprecomp::Runtime &runtime, const psprecomp::AllegrexContext &ctx) {
    State &s = state();
    const psprecomp::GuestMemory &memory = runtime.memory();
    settle_text(s, memory);
    const bool trace = tracing(s);
    std::string chain;
    bool name = false;
    bool tag_only = false;
    bool quest = false;
    bool hall = false;
    Site site = Site::None;
    std::uint32_t call = 0u;
    std::uint32_t name_call = 0u;
    bool hud =
        from_hud(s, memory, ctx, trace ? &chain : nullptr, &name, &tag_only, &quest, &hall, &site, &call, &name_call);
    // A hall label's own piece, by the call in the label function that filed
    // it. The function draws every hunter's label in one call, each starting
    // with its badge: traced with MHFU_TRACE_CAPTURE, every frame files a
    // badge, that name's text and its marks, then the next badge. So a badge
    // after any other piece starts the next label, and the pieces after it
    // are that label's.
    LabelPart part = hall ? label_part_of_call(name_call) : LabelPart::Unknown;
    std::uint32_t label = 0u;
    if (hall && name_call != 0u) {
        if (part == LabelPart::Badge && s.label_part != LabelPart::Badge) ++s.labels;
        s.label_part = part;
        label = s.labels;
    }
    if (quest && site == Site::Status) s.status_now = true;
    // Text a HUD function queues after drawing a packet of its own, as the
    // name plate does, is the HUD's too. The stack here is the filer, so a
    // name is the label function on that stack, or text it queued earlier.
    s.last_call_hud = hud;
    s.last_call_quest = quest;
    s.last_call_site = site;
    s.last_call_name = name;
    s.last_call_tag_only = tag_only;
    s.last_call_hall = hall;
    s.last_call_label = label;
    // A hall label's icon waits for its name's point; anything else filed
    // first means the label queued no name.
    const bool label_icon = name && hall;
    if (!label_icon) s.unplaced_packets.clear();
    const std::uint32_t sp = ctx.gpr[29];
    std::uint32_t entry = 0u;
    bool has_origin = false;
    bool glyph = false;
    float origin_x = 0.0f;
    float origin_y = 0.0f;
    if (ctx.gpr[31] == kGlyphFiled && memory.contains(sp + kGlyphSavedRa, 4u) &&
        memory.load32(sp + kGlyphSavedRa) == kGlyphFromQueue) {
        entry = memory.load32(sp + kGlyphSavedEntry);
        // A glyph is drawn at the end of the frame, far from the code that
        // queued its text: it is the part that queued it.
        if (const State::HudEntry *queued = hud ? nullptr : find_entry(s.text_entries, entry)) {
            hud = true;
            quest = queued->quest;
            site = queued->site;
        }
        if (const State::NameEntry *named = find_name(s, entry)) {
            name = true;
            tag_only = named->tag_only;
            hall = named->hall;
            label = named->label;
            part = LabelPart::Unknown;
            glyph = true;
            has_origin = entry_origin(memory, entry, origin_x, origin_y);
        }
    }
    // 0x08892A88 files an icon's packets with the list entry in s4.
    if (ctx.gpr[31] == kIconFiled || ctx.gpr[31] == kIconFiledAgain) {
        entry = ctx.gpr[20];
        if (const State::HudEntry *icon = hud ? nullptr : find_entry(s.icons, entry)) {
            hud = true;
            quest = icon->quest;
            site = icon->site;
        }
        if (const auto placed = s.icon_origin.find(entry); placed != s.icon_origin.end()) {
            name = true;
            tag_only = placed->second.tag_only;
            hall = placed->second.hall;
            label = placed->second.label;
            part = LabelPart::Unknown;
            has_origin = true;
            origin_x = placed->second.x;
            origin_y = placed->second.y;
        }
    }
    note_packet(s, ctx.gpr[5], ctx.gpr[6], hud, quest, site, call);
    note_name_packet(
        s, ctx.gpr[5], ctx.gpr[6], name, has_origin, tag_only, glyph, origin_x, origin_y, hall, label, part);
    if (label_icon && !has_origin) s.unplaced_packets.push_back(ctx.gpr[5]);
    if (s.unplaced_packets.size() > 64u) s.unplaced_packets.clear(); // never expected; bounds a runaway
    if (trace) {
        char head[160];
        std::snprintf(head, sizeof(head), "[hud] f=%llu packet=%08X words=%u bucket=%u list=%08X %s site=%s |",
            static_cast<unsigned long long>(s.frame), ctx.gpr[5], ctx.gpr[6], ctx.gpr[7], ctx.gpr[4],
            hud ? (quest ? "HUD quest" : "HUD") : "-", site_name(site));
        std::cout << head << chain << " |" << registers_text(ctx);
        if (entry != 0u) {
            char text[40];
            std::snprintf(text, sizeof(text), " | queued text %08X", entry);
            std::cout << text;
        }
        if (label != 0u) {
            char text[40];
            std::snprintf(text, sizeof(text), " | label %u part %u", label, static_cast<unsigned>(part));
            std::cout << text;
        }
        std::cout << "\n";
    }
}

void overhead_name_begin(psprecomp::Runtime &runtime, const psprecomp::AllegrexContext &ctx) {
    State &s = state();
    const psprecomp::GuestMemory &memory = runtime.memory();
    // Text queued before this call belongs to the call before it.
    settle_text(s, memory);
    bool quest = false;
    s.overhead_quest = from_hud(s, memory, ctx, nullptr, nullptr, nullptr, &quest) && quest;
}

void overhead_name_end(psprecomp::Runtime &runtime) {
    State &s = state();
    if (!s.overhead_quest) return;
    s.overhead_quest = false;
    const psprecomp::GuestMemory &memory = runtime.memory();
    const std::uint32_t context = memory.load32(kTextContextPointer);
    if (!memory.contains(context + kQueueLengths, kQueues) || !memory.contains(context + kQueueEntries, kQueues * 4u))
        return;
    for (std::uint32_t queue = 0; queue < kQueues; ++queue) {
        const std::uint8_t length = memory.load8(context + kQueueLengths + queue);
        std::uint8_t &known = s.lengths[queue];
        if (length <= known) continue;
        const std::uint32_t entries = memory.load32(context + kQueueEntries + queue * 4u);
        if (tracing(s)) trace_entries(memory, s.frame, queue, entries, known, length, "overhead-name");
        // The quest HUD's text, and a name: its letters take the Names size
        // about the point it was placed at, and no other piece's rule.
        for (std::uint32_t i = known; i < length; ++i) {
            const std::uint32_t at = entries + i * kEntryBytes;
            s.text_entries.push_back(State::HudEntry{at, true, Site::Names});
            s.name_entries.push_back(State::NameEntry{at, false});
        }
        known = length;
    }
}

namespace {

// After a character of queued text is drawn: icons it added to the queue's
// icon list are the HUD's when its entry is.
void on_character_drawn(psprecomp::Runtime &runtime, const psprecomp::AllegrexContext &ctx) {
    State &s = state();
    if (ctx.gpr[31] != kCharacterDrawnFromQueue) return;
    const psprecomp::GuestMemory &memory = runtime.memory();
    const std::uint32_t context = memory.load32(kTextContextPointer);
    const std::uint32_t entry = ctx.gpr[19];
    // The lists are filled anew every frame.
    if (s.icon_frame != s.frame) {
        s.icon_frame = s.frame;
        s.icon_counts.fill(0u);
        s.icons.clear();
        s.icon_origin.clear();
    }
    if (!memory.contains(context + kQueueEntries, kQueues * 4u) || !memory.contains(context + kIconCounts, kQueues))
        return;
    for (std::uint32_t queue = 0; queue < kQueues; ++queue) {
        const std::uint32_t entries = memory.load32(context + kQueueEntries + queue * 4u);
        if (entry < entries || entry >= entries + 256u * kEntryBytes) continue;
        const std::uint8_t count = memory.load8(context + kIconCounts + queue);
        const std::uint32_t list = context + kIconLists + queue * kIconListStride;
        std::uint8_t &known = s.icon_counts[queue];
        if (count < known) {
            const auto in_list = [&](std::uint32_t e) { return e >= list && e < list + kIconListStride; };
            std::erase_if(s.icons, [&](const State::HudEntry &e) { return in_list(e.address); });
            std::erase_if(s.icon_origin, [&](const auto &placed) { return in_list(placed.first); });
            known = 0u;
        }
        if (count > known && tracing(s)) {
            std::cout << "[hud] f=" << s.frame << " icons of text " << std::hex << std::uppercase << entry << " |";
            for (std::uint32_t i = known; i < count; ++i) std::cout << ' ' << list + i * kIconBytes;
            std::cout << std::dec << std::nouppercase << "\n";
        }
        const State::HudEntry *hud_text = find_entry(s.text_entries, entry);
        const State::NameEntry *named = find_name(s, entry);
        if (hud_text != nullptr || named != nullptr) {
            float origin_x = 0.0f;
            float origin_y = 0.0f;
            const bool has_origin = named != nullptr && entry_origin(memory, entry, origin_x, origin_y);
            for (std::uint32_t i = known; i < count; ++i) {
                const std::uint32_t icon = list + i * kIconBytes;
                if (hud_text != nullptr) s.icons.push_back(State::HudEntry{icon, hud_text->quest, hud_text->site});
                if (has_origin)
                    s.icon_origin.emplace(
                        icon, State::IconOrigin{origin_x, origin_y, named->tag_only, named->hall, named->label});
            }
        }
        known = count;
        return;
    }
}

void on_text_call(std::size_t which, psprecomp::Runtime &runtime, const psprecomp::AllegrexContext &ctx) {
    State &s = state();
    const psprecomp::GuestMemory &memory = runtime.memory();
    settle_text(s, memory);
    const bool trace = tracing(s);
    std::string chain;
    bool name = false;
    bool tag_only = false;
    bool hall = false;
    s.last_call_hud = from_hud(
        s, memory, ctx, trace ? &chain : nullptr, &name, &tag_only, &s.last_call_quest, &hall, &s.last_call_site);
    s.last_call_label = 0u;
    s.last_call_name = name;
    s.last_call_tag_only = tag_only;
    s.last_call_hall = hall;
    if (trace)
        std::cout << "[hud] f=" << s.frame << " text 0x" << std::hex << wrapped_address(which) << std::dec
                  << (s.last_call_hud ? " HUD" : " -") << " |" << chain << " |" << registers_text(ctx) << "\n";
}

template <std::size_t I> void wrapper(psprecomp::Runtime &runtime, psprecomp::AllegrexContext &ctx) {
    if constexpr (I == 0u)
        packet_filed(runtime, ctx);
    else if constexpr (I == 1u)
        on_character_drawn(runtime, ctx);
    else
        on_text_call(I, runtime, ctx);
    // The original, with the same context and return address.
    state().originals[I](runtime, ctx);
}

void overhead_name_wrapper(psprecomp::Runtime &runtime, psprecomp::AllegrexContext &ctx) {
    overhead_name_begin(runtime, ctx);
    state().overhead_original(runtime, ctx);
    overhead_name_end(runtime);
}

// Each text queue's length, or nothing when the text context is not there.
std::optional<std::array<std::uint8_t, kQueues>> queue_lengths(const psprecomp::GuestMemory &memory) {
    if (!memory.contains(kTextContextPointer, 4u)) return std::nullopt;
    const std::uint32_t context = memory.load32(kTextContextPointer);
    if (!memory.contains(context + kQueueLengths, kQueues)) return std::nullopt;
    std::array<std::uint8_t, kQueues> lengths{};
    for (std::uint32_t queue = 0; queue < kQueues; ++queue)
        lengths[queue] = memory.load8(context + kQueueLengths + queue);
    return lengths;
}

// MHFU_TRACE_TEXT_FROM: a probed function. When text is queued while it
// runs, by any path, whether or not a watched text call saw it, one line
// gives the queue, each new entry's address and point (the x and y the text
// is drawn at), and the call stack the probed function was entered from.
template <std::size_t I> void probe(psprecomp::Runtime &runtime, psprecomp::AllegrexContext &ctx) {
    State &s = state();
    const psprecomp::GuestMemory &memory = runtime.memory();
    const auto before = queue_lengths(memory);
    const psprecomp::AllegrexContext entered = ctx;
    s.probes[I].original(runtime, ctx);
    const auto after = queue_lengths(memory);
    if (!before || !after || s.probe_lines_left <= 0) return;
    const std::uint32_t context = memory.load32(kTextContextPointer);
    for (std::uint32_t queue = 0; queue < kQueues; ++queue) {
        if ((*after)[queue] <= (*before)[queue]) continue;
        const std::uint32_t entries = memory.load32(context + kQueueEntries + queue * 4u);
        std::string chain;
        bool name = false;
        bool tag_only = false;
        bool quest = false;
        const bool hud = from_hud(s, memory, entered, &chain, &name, &tag_only, &quest);
        // Each probed function, queue and caller once: the point changes as
        // the camera moves, the call stack does not.
        char key[24];
        std::snprintf(key, sizeof(key), "%08X/%u", s.probes[I].address, queue);
        if (!s.probe_seen.insert(std::string(key) + chain).second) continue;
        std::cout << "[textfrom] f=" << s.frame << " fn=0x" << std::hex << s.probes[I].address << std::dec
                  << " queue=" << queue << (hud ? " HUD" : " -") << " entries";
        for (std::uint32_t i = (*before)[queue]; i < (*after)[queue]; ++i) {
            const std::uint32_t entry = entries + i * kEntryBytes;
            float x = 0.0f;
            float y = 0.0f;
            entry_origin(memory, entry, x, y);
            char text[48];
            std::snprintf(text, sizeof(text), " %08X(%g,%g)", entry, x, y);
            std::cout << text;
        }
        std::cout << " |" << chain << "\n";
        --s.probe_lines_left;
    }
}

template <std::size_t... I> void install_probes(psprecomp::Runtime &runtime, std::index_sequence<I...>) {
    State &s = state();
    const char *text = std::getenv("MHFU_TRACE_TEXT_FROM");
    if (text == nullptr) return;
    std::size_t count = 0;
    for (const char *at = text; *at != '\0' && count < s.probes.size();) {
        char *end = nullptr;
        const std::uint32_t address = static_cast<std::uint32_t>(std::strtoul(at, &end, 16));
        if (end == at) break;
        s.probes[count++].address = address;
        at = *end == ',' ? end + 1 : end;
    }
    const auto install = [&](std::size_t index, psprecomp::Runtime::RecompiledFunction wrapper) {
        State::Probe &probe = s.probes[index];
        if (index >= count) return;
        probe.original = runtime.function_at(probe.address);
        if (probe.original == nullptr) {
            std::cout << "[textfrom] 0x" << std::hex << probe.address << std::dec
                      << " is not a function the runtime knows; not probed\n";
            return;
        }
        runtime.register_function(probe.address, wrapper, "mhfu_textfrom");
        std::cout << "[textfrom] probing 0x" << std::hex << probe.address << std::dec << std::endl;
    };
    (install(I, &probe<I>), ...);
}

template <std::size_t... I> bool install_all(psprecomp::Runtime &runtime, std::index_sequence<I...>) {
    State &s = state();
    ((s.originals[I] = compiled(wrapped_address(I))), ...);
    for (const auto original : s.originals)
        if (original == nullptr) return false;
    (runtime.register_function(wrapped_address(I), &wrapper<I>, "mhfu_hud"), ...);
    return true;
}

bool check_code(const psprecomp::Runtime &runtime) {
    for (const CodeWord &expected : kSignature) {
        const std::uint32_t found = runtime.memory().load32(expected.address);
        if (found == expected.word) continue;
        std::cout << "[hud] game code differs at 0x" << std::hex << expected.address << ": 0x" << found
                  << ", expected 0x" << expected.word << std::dec << "; hiding the HUD is unavailable\n";
        return false;
    }
    return true;
}

} // namespace

std::span<const CodeWord> code_signature() {
    return kSignature;
}

std::span<const HudFunction> hud_functions() {
    return kHudFunctions;
}

std::span<const SiteFunction> site_functions() {
    return kSiteFunctions;
}

bool code_matches(const psprecomp::Runtime &runtime) {
    for (const CodeWord &expected : kSignature)
        if (runtime.memory().load32(expected.address) != expected.word) return false;
    return true;
}

bool hud_function_matches(const psprecomp::GuestMemory &memory, std::uint32_t address) {
    for (const HudFunction &fn : kHudFunctions) {
        if (fn.address != address) continue;
        return memory.load32(address) == fn.first && memory.load32(address + 4u) == fn.saved_ra;
    }
    return false;
}

bool quest_function(std::uint32_t address) {
    for (const HudFunction &fn : kHudFunctions)
        if (fn.address == address) return fn.quest;
    return false;
}

bool name_label(std::uint32_t address) {
    for (const NameFunction &fn : kNameFunctions)
        if (fn.address == address) return true;
    return false;
}

bool name_tag_only(std::uint32_t address) {
    for (const NameFunction &fn : kNameFunctions)
        if (fn.address == address) return fn.tag_only;
    return false;
}

void frame(psprecomp::Runtime &runtime, std::uint64_t presented_frames) {
    State &s = state();
    s.frame = presented_frames;
    s.status_live = s.status_now;
    s.status_now = false;
    s.labels = 0u;
    s.label_part = LabelPart::Unknown;
    if (!s.checked) {
        s.checked = true;
        if (const char *text = std::getenv("MHFU_TRACE_HUD"); text != nullptr && *text != '\0') {
            char *end = nullptr;
            s.trace_first = std::strtoull(text, &end, 10);
            s.trace_every = end != nullptr && *end == '/' ? std::strtoull(end + 1, nullptr, 10) : 0ull;
        }
        if (const char *text = std::getenv("MHFU_HIDE_HUD"); text != nullptr && *text == '1') s.hidden = true;
        if (const char *text = std::getenv("MHFU_TRACE_CAPTURE"); text != nullptr && *text == '1') s.capture_on = true;
        s.available = check_code(runtime);
    }
    // At the flip, like the wrappers below. The probes need the text context
    // the HUD signature was read with.
    if (!s.probes_checked && s.available) {
        s.probes_checked = true;
        install_probes(runtime, std::make_index_sequence<std::tuple_size_v<decltype(s.probes)>>{});
    }
    // Only at the flip, from an import: no generated frame is live on the
    // host stack, so the dispatch tables may change. Put in place once first
    // needed, then kept, so that hiding and showing the HUD again works at once.
    // Any piece at another size has to be told from the other 2D draws, or a
    // menu with a panel in that piece's corner is resized with it.
    const settings::Settings::InterfaceScale &hud = settings::current().hud;
    const bool sized = hud.names != 100u || hud.map != 100u || hud.health != 100u || hud.item_bar != 100u;
    if (s.capture.requested) {
        s.capture.at_flip(s.frame);
        std::cout << "[capture] tracing the 2D packets of frames " << s.capture.first << "-"
                  << s.capture.first + CaptureSchedule::kFrames - 1u << " and the sprites of frames "
                  << s.capture.first + 1u << "-" << s.capture.first + CaptureSchedule::kFrames << std::endl;
    }
    // The hunt icons and the sharpness gauge read the parts from the draws
    // too (sites_on), so either one on has them watched as well.
    const bool parts = settings::current().lock_on || settings::current().sharpness_gauge;
    const bool wanted =
        s.hidden || s.trace_first != ~0ull || s.capture.pending() || sized || parts || s.names_wanted || s.cover_wanted;
    if (!s.installed && wanted && s.available) {
        s.installed = true;
        if (!install_all(runtime, std::make_index_sequence<kWrapped>{})) {
            s.available = false;
            std::cout << "[hud] the game's 2D functions are not in the generated code; hiding the HUD is "
                         "unavailable, and interface sizes go by rectangles alone\n";
            return;
        }
        std::cout << "[hud] watching the game's 2D draws" << std::endl;
        // MHFU_HUD_OVERHEAD_NAME=0 leaves the name over a teammate to the
        // shape rules, as before it was told apart, to compare both.
        const char *overhead = std::getenv("MHFU_HUD_OVERHEAD_NAME");
        if (overhead == nullptr || *overhead != '0') {
            s.overhead_original = runtime.function_at(kOverheadName);
            if (s.overhead_original != nullptr)
                runtime.register_function(kOverheadName, &overhead_name_wrapper, "mhfu_hud");
        }
    }
}

namespace {

bool hidden_now(const State &s) {
    return s.hidden;
}

void note_change(State &s, bool before) {
    const bool after = hidden_now(s);
    if (after == before) return;
    s.note = after ? "HUD hidden" : "HUD shown";
    s.note_until = Clock::now() + std::chrono::milliseconds(1500);
    std::cout << "[hud] " << (after ? "hidden" : "shown") << std::endl;
}

} // namespace

void set_hidden(bool hidden) {
    State &s = state();
    const bool before = hidden_now(s);
    s.hidden = hidden;
    note_change(s, before);
}

void toggle() {
    set_hidden(!hidden_now(state()));
}

void screenshot_taken() {
    State &s = state();
    if (s.capture_on) s.capture.request();
}

bool capture_sprites(std::uint64_t frame) {
    return state().capture.sprites(frame);
}

void want_names(bool wanted) {
    state().names_wanted = wanted;
}

void want_cover(bool wanted) {
    state().cover_wanted = wanted;
}

bool hidden() {
    return hidden_now(state());
}

bool active() {
    const State &s = state();
    return s.installed && s.available && hidden_now(s);
}

bool available() {
    const State &s = state();
    return !s.checked || s.available;
}

double note_seconds_left() {
    const State &s = state();
    return std::chrono::duration<double>(s.note_until - Clock::now()).count();
}

std::string note_text() {
    return state().note;
}

bool hides(std::uint32_t command_address, std::uint32_t call_return) {
    return active() && is_hud(command_address, call_return);
}

bool is_hud(std::uint32_t command_address, std::uint32_t call_return) {
    const State &s = state();
    if (!s.installed || !s.available) return false;
    const auto inside = [&](std::uint32_t address) {
        if (address == 0u) return false;
        auto it = s.packets.upper_bound(address);
        return it != s.packets.begin() && address < std::prev(it)->second.end;
    };
    return inside(command_address) || inside(call_return);
}

Owner owner_of(std::uint32_t command_address, std::uint32_t call_return) {
    const State &s = state();
    // Not this game's code, or its 2D functions are missing.
    if (s.checked && !s.available) return Owner::Unknown;
    // Not watched yet: nothing is resized until it can be told apart.
    if (!s.installed) return Owner::Other;
    // The draw's own command first: a CALL's return can lie in another packet.
    const auto packet = [&](std::uint32_t address) -> const State::Packet * {
        if (address == 0u) return nullptr;
        auto it = s.packets.upper_bound(address);
        if (it == s.packets.begin() || address >= std::prev(it)->second.end) return nullptr;
        return &std::prev(it)->second;
    };
    const State::Packet *found = packet(command_address);
    if (found == nullptr) found = packet(call_return);
    return found != nullptr && found->quest ? Owner::QuestHud : Owner::Other;
}

void watch_for_tests() {
    State &s = state();
    s.checked = true;
    s.available = true;
    s.installed = true;
    s.packets.clear();
    s.name_packets.clear();
    s.name_entries.clear();
    s.unplaced_packets.clear();
    s.text_entries.clear();
    s.lengths = {};
    s.icons.clear();
    s.frames.clear();
    s.last_call_hud = s.last_call_quest = s.last_call_name = s.last_call_tag_only = s.last_call_hall = false;
    s.last_call_site = Site::None;
    s.last_call_label = 0u;
    s.labels = 0u;
    s.label_part = LabelPart::Unknown;
    s.status_now = s.status_live = false;
    s.trace_first = ~0ull;
}

void trace_for_tests() {
    State &s = state();
    s.trace_first = s.frame;
    s.trace_every = 0u;
}

SiteDraw site_draw_of(std::uint32_t command_address, std::uint32_t call_return) {
    const State &s = state();
    if (!s.installed || !s.available) return {};
    // The draw's own command first, as owner_of.
    const auto packet = [&](std::uint32_t address) -> const State::Packet * {
        if (address == 0u) return nullptr;
        auto it = s.packets.upper_bound(address);
        if (it == s.packets.begin() || address >= std::prev(it)->second.end) return nullptr;
        return &std::prev(it)->second;
    };
    const State::Packet *found = packet(command_address);
    if (found == nullptr) found = packet(call_return);
    return found != nullptr && found->quest ? SiteDraw{found->site, found->call} : SiteDraw{};
}

Site site_of(std::uint32_t command_address, std::uint32_t call_return) {
    return site_draw_of(command_address, call_return).site;
}

bool sites_on() {
    static const bool switched_on = [] {
        const char *value = std::getenv("MHFU_HUD_SCALE_SITES");
        return value == nullptr || *value != '0';
    }();
    const State &s = state();
    return switched_on && s.installed && s.available;
}

bool quest_hud_filed() {
    const State &s = state();
    return s.status_now || s.status_live;
}

NameDraw name_at(std::uint32_t command_address, std::uint32_t call_return) {
    const State &s = state();
    if (!s.installed) return {};
    const auto found = [&](std::uint32_t address) -> NameDraw {
        if (address == 0u) return {};
        auto it = s.name_packets.upper_bound(address);
        if (it == s.name_packets.begin() || address >= std::prev(it)->second.end) return {};
        const State::NamePacket &packet = std::prev(it)->second;
        return {true, packet.has_origin, packet.tag_only, packet.glyph, packet.x, packet.y, packet.hall, packet.label,
            packet.part};
    };
    const NameDraw command = found(command_address);
    return command.is_name ? command : found(call_return);
}

} // namespace mhfu::gpu::hud
