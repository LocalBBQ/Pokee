// No game code or game data is needed: the words hiding the HUD checks are
// written into guest memory as stand-ins.
#include "gpu/game_hud.hpp"
#include "psprecomp/allegrex_context.hpp"
#include "psprecomp/runtime.hpp"

#include <initializer_list>
#include <iostream>
#include <sstream>
#include <string>

namespace {
using namespace mhfu::gpu::hud;
int failures{};

void check(bool condition, const char *message) {
    if (!condition) {
        ++failures;
        std::cerr << "FAIL: " << message << '\n';
    }
}

void store_signature(psprecomp::GuestMemory &memory) {
    for (const CodeWord &word : code_signature()) memory.store32(word.address, word.word);
}

void test_signature_accepts_this_executable() {
    psprecomp::Runtime runtime;
    store_signature(runtime.memory());
    check(code_matches(runtime), "the Freedom Unite words are accepted");
    check(code_signature().size() == 25u, "every anchor is checked");
}

void test_signature_refuses_a_different_word() {
    psprecomp::Runtime runtime;
    store_signature(runtime.memory());
    const CodeWord &insert = code_signature().front();
    check(insert.address == 0x088596C8u, "the ordering-table insert is the first anchor");
    runtime.memory().store32(insert.address, 0u);
    check(!code_matches(runtime), "a different insert is refused");
}

void test_hud_functions() {
    psprecomp::Runtime runtime;
    auto &memory = runtime.memory();
    const auto functions = hud_functions();
    check(functions.size() == 6u, "village text and labels, the quest HUD and the labels");
    check(functions[0].address == 0x09AE38F8u, "the village layer");
    check(functions[1].address == 0x09A6C248u, "the lobby task callback");
    check(functions[2].address == 0x09A60298u, "the quest HUD");
    check(functions[3].address == 0x09A617A8u, "a quest HUD state");
    check(functions[4].address == 0x09A62400u, "another quest HUD state");
    check(functions[5].address == 0x09B62488u, "the quest labels");

    for (const HudFunction &fn : functions) {
        memory.store32(fn.address, fn.first);
        memory.store32(fn.address + 4u, fn.saved_ra);
        check(hud_function_matches(memory, fn.address), "matching overlay words count");
        memory.store32(fn.address + 4u, 0u);
        check(!hud_function_matches(memory, fn.address), "a replaced overlay does not count");
        memory.store32(fn.address + 4u, fn.saved_ra);
    }
    check(!hud_function_matches(memory, 0x088596C8u), "the packet insert is not a HUD function");
    // The quest slot and the village slot are the same load address, so a
    // quest function's bytes are not the village function.
    memory.store32(functions[0].address, functions[2].first);
    memory.store32(functions[0].address + 4u, functions[2].saved_ra);
    check(!hud_function_matches(memory, functions[0].address), "the other overlay's words do not count");
    check(!name_label(0x09A6C248u), "the lobby callback is not a name; it draws the village interface");
    check(!name_label(0x09A81220u), "a village hunter label is not a separate name");
    check(!name_label(0x09A825B8u), "the village name is drawn with the rest of that screen");
    check(!name_label(0x09A82AD0u), "the other village name is drawn with the rest of that screen");
    check(!name_label(0x09A85C90u), "the mark beside a village name is drawn with that screen");
    check(!name_label(0x09A79030u), "the gathering-hall name is drawn with that screen");
    check(name_label(0x09B62488u), "the quest name is a name");
    check(!name_tag_only(0x09B62488u), "a quest name is scaled with its mark");
    check(!name_label(0x09A60298u), "the quest HUD is not a teammate's name");
    check(!name_tag_only(0x09A60298u), "the quest HUD is not a name tag");

    // Only game_task's functions are the quest HUD the sizes apply to.
    check(!functions[0].quest && !functions[1].quest, "the village functions are not resized");
    check(functions[2].quest && functions[3].quest && functions[4].quest && functions[5].quest,
        "the quest HUD functions are resized");
    check(!quest_function(0x09A6C248u), "the lobby callback draws shops and dialogs");
    check(quest_function(0x09A60298u), "the quest HUD");
    check(!quest_function(0x088596C8u), "the packet insert is no HUD function");
}

// A stand-in for game code that files a packet: a function at `start` with
// the HUD function's prologue, and the stack at the call to the insert. It
// returns into that function, which saved its caller's ra; that caller is
// absent, so the walk ends there.
constexpr std::uint32_t kStack = 0x09F00000u;

psprecomp::AllegrexContext filed_from(psprecomp::GuestMemory &memory, std::uint32_t start, std::uint32_t first,
    std::uint32_t saved_ra, std::uint32_t packet, std::uint32_t words) {
    memory.store32(start, first);
    memory.store32(start + 4u, saved_ra);
    for (std::uint32_t at = start + 8u; at < start + 32u; at += 4u) memory.store32(at, 0u);
    // The caller's ra, where this function saved it: nowhere guest code lies.
    memory.store32(kStack + (saved_ra & 0xFFFFu), 0u);
    psprecomp::AllegrexContext ctx{};
    ctx.gpr[29] = kStack;
    ctx.gpr[31] = start + 24u;
    ctx.gpr[5] = packet;
    ctx.gpr[6] = words;
    return ctx;
}

void test_draw_owner() {
    psprecomp::Runtime runtime;
    auto &memory = runtime.memory();
    const auto functions = hud_functions();
    const HudFunction &village = functions[1];
    const HudFunction &quest = functions[2];

    check(owner_of(0x09000000u, 0u) == Owner::Other, "nothing is resized before the 2D draws are watched");
    check(!is_hud(0x09000000u, 0u), "and nothing is HUD before then");
    watch_for_tests();

    // The quest HUD files a packet: a draw inside it may be resized.
    psprecomp::AllegrexContext ctx = filed_from(memory, quest.address, quest.first, quest.saved_ra, 0x09000000u, 16u);
    packet_filed(runtime, ctx);
    check(owner_of(0x09000010u, 0u) == Owner::QuestHud, "a draw in a quest HUD packet is the quest HUD's");
    check(is_hud(0x09000010u, 0u) && is_hud(0u, 0x09000020u), "and is HUD for the cover while it is shown");
    check(!is_hud(0x09000040u, 0u) && !is_hud(0u, 0u), "a draw past the packet, or with no address, is not");
    check(owner_of(0x09000040u, 0u) == Owner::Other, "a draw past the packet's end is not");
    check(owner_of(0u, 0x09000020u) == Owner::QuestHud, "a draw called from that packet is the quest HUD's");
    check(owner_of(0u, 0u) == Owner::Other, "a draw with no address is no one's");

    // The village interface is HUD for hiding, and a shop or a dialog for
    // resizing: its packets are never resized.
    ctx = filed_from(memory, village.address, village.first, village.saved_ra, 0x09001000u, 16u);
    packet_filed(runtime, ctx);
    check(hides(0x09001010u, 0u) == false, "the HUD is shown, so nothing is left out");
    check(is_hud(0x09001010u, 0u), "the village interface is HUD for the cover");
    check(owner_of(0x09001010u, 0u) == Owner::Other, "a village draw is never resized");

    // A menu's own code: not on the list.
    ctx = filed_from(memory, 0x09A70000u, 0x27BDFFF0u, 0xAFBF000Cu, 0x09002000u, 16u);
    packet_filed(runtime, ctx);
    check(owner_of(0x09002010u, 0u) == Owner::Other, "a menu's packet is not the quest HUD's");

    // The next frame's menu is filed where the HUD's packet was.
    ctx = filed_from(memory, 0x09A70000u, 0x27BDFFF0u, 0xAFBF000Cu, 0x09000008u, 4u);
    packet_filed(runtime, ctx);
    check(owner_of(0x09000010u, 0u) == Owner::Other, "a packet filed over the HUD's takes its range");

    // The other overlay in the quest function's slot is not the quest HUD.
    ctx = filed_from(memory, quest.address, village.first, 0xAFBF0010u, 0x09003000u, 16u);
    packet_filed(runtime, ctx);
    check(owner_of(0x09003010u, 0u) == Owner::Other, "other words at the quest HUD's address do not count");
}

// A function on a stand-in stack: where it starts, its addiu sp and its sw ra.
struct StackFrame {
    std::uint32_t start;
    std::uint32_t first;
    std::uint32_t saved_ra;
};

// A packet filed from the first of `frames`, called by the next and so on;
// the last one's caller is absent, so the walk ends there.
psprecomp::AllegrexContext filed_through(psprecomp::GuestMemory &memory, std::initializer_list<StackFrame> frames,
    std::uint32_t packet, std::uint32_t words) {
    std::uint32_t sp = kStack;
    const StackFrame *previous = nullptr;
    for (const StackFrame &frame : frames) {
        memory.store32(frame.start, frame.first);
        memory.store32(frame.start + 4u, frame.saved_ra);
        for (std::uint32_t at = frame.start + 8u; at < frame.start + 32u; at += 4u) memory.store32(at, 0u);
        if (previous != nullptr) {
            // The previous function saved a return address into this one.
            memory.store32(sp + (previous->saved_ra & 0xFFFFu), frame.start + 24u);
            sp += 0x10000u - (previous->first & 0xFFFFu);
        }
        previous = &frame;
    }
    memory.store32(sp + (previous->saved_ra & 0xFFFFu), 0u);
    psprecomp::AllegrexContext ctx{};
    ctx.gpr[29] = kStack;
    ctx.gpr[31] = frames.begin()->start + 24u;
    ctx.gpr[5] = packet;
    ctx.gpr[6] = words;
    return ctx;
}

// Which part of the quest HUD drew a packet: the innermost function on the
// stack in the site table, while the quest HUD is on it. Traced with
// MHFU_TRACE_CAPTURE across the weapons, the map, the item bar and the menus.
void test_draw_site() {
    psprecomp::Runtime runtime;
    auto &memory = runtime.memory();
    const HudFunction &quest = hud_functions()[2];
    const StackFrame root{quest.address, quest.first, quest.saved_ra};
    const StackFrame weapon{0x08841100u, 0x27BDFFD0u, 0xAFBF001Cu};
    const StackFrame party{0x08841908u, 0x27BDFFD0u, 0xAFBF001Cu};
    const StackFrame overhead{0x08832B98u, 0x27BDFFE0u, 0xAFBF001Cu};
    const StackFrame item_bar{0x08845A34u, 0x27BDFFE0u, 0xAFBF001Cu};
    const StackFrame quest_menu{0x08818C94u, 0x27BDFF90u, 0xAFBF001Cu};

    for (const SiteFunction &fn : site_functions())
        check((fn.first >> 16u) == 0x27BDu && (fn.first & 0x8000u) != 0u, "each site begins with its addiu sp, -N");

    check(site_of(0x09000010u, 0u) == Site::None, "nothing has a site before the 2D draws are watched");
    watch_for_tests();
    packet_filed(runtime, filed_through(memory, {weapon, root}, 0x09000000u, 16u));
    check(site_of(0x09000010u, 0u) == Site::Weapon, "the weapon's gauges are the weapon");
    check(owner_of(0x09000010u, 0u) == Owner::QuestHud, "and the quest HUD's");
    check(site_of(0u, 0x09000020u) == Site::Weapon, "a draw called from that packet has its site");
    check(site_of(0x09000040u, 0u) == Site::None, "a draw past the packet has none");
    check(site_draw_of(0x09000010u, 0u).call == weapon.start + 24u,
        "the call in the weapon's function that led to the packet is kept: it names a sharpness piece");
    check(sites_on(), "sites are read once the draws are watched");

    packet_filed(runtime, filed_through(memory, {party, root}, 0x09001000u, 16u));
    check(site_of(0x09001010u, 0u) == Site::Status, "the party list is Status");
    packet_filed(runtime, filed_through(memory, {overhead, party, root}, 0x09002000u, 16u));
    check(site_of(0x09002010u, 0u) == Site::Names, "a name over a hunter from the party list's code is a name");

    packet_filed(runtime, filed_through(memory, {quest_menu, root}, 0x09003000u, 16u));
    check(site_of(0x09003010u, 0u) == Site::Window, "the quest menu is a window, left as drawn");

    // The map's drawing is listed under the map itself, so the call kept is
    // the one in 0x08842F70 that names the piece (the tile, the window, a mark).
    const StackFrame map_drawing{0x08842F70u, 0x27BDFF80u, 0xAFBF005Cu};
    const StackFrame map{0x08842EE8u, 0x27BDFFF0u, 0xAFBF000Cu};
    packet_filed(runtime, filed_through(memory, {map_drawing, map, root}, 0x0900A000u, 16u));
    check(site_draw_of(0x0900A010u, 0u).site == Site::Map &&
            site_draw_of(0x0900A010u, 0u).call == map_drawing.start + 24u,
        "a map draw keeps the call inside the map's drawing");

    packet_filed(runtime, filed_through(memory, {root}, 0x09004000u, 16u));
    check(site_of(0x09004010u, 0u) == Site::Unlisted, "the quest HUD's own draw is no listed part");

    // A listed address that no longer holds the function it was read from.
    const StackFrame moved{weapon.start, 0x27BDFFC0u, weapon.saved_ra};
    packet_filed(runtime, filed_through(memory, {moved, root}, 0x09005000u, 16u));
    check(site_of(0x09005010u, 0u) == Site::Unlisted, "other code at a listed address is not that part");

    // The quest HUD is on screen while its left column is filed: this frame
    // or the one before, and no longer once two frames pass without it.
    check(quest_hud_filed(), "the party list was filed this frame");
    frame(runtime, 1u);
    check(quest_hud_filed(), "the frame after still counts");
    frame(runtime, 2u);
    check(!quest_hud_filed(), "two frames without the left column, and the quest HUD is gone");
    packet_filed(runtime, filed_through(memory, {quest_menu, root}, 0x09003000u, 16u));
    check(!quest_hud_filed(), "a menu over the quest is not the left column");
    packet_filed(runtime, filed_through(memory, {party, root}, 0x09001000u, 16u));
    check(quest_hud_filed(), "the left column again");

    // The weapon's code outside the quest HUD is no part of it.
    packet_filed(runtime, filed_through(memory, {weapon}, 0x09006000u, 16u));
    check(site_of(0x09006010u, 0u) == Site::None, "outside the quest HUD there is no site");
    check(owner_of(0x09006010u, 0u) == Owner::Other, "and the draw is not the quest HUD's");

    // Text is drawn at the end of the frame. Its glyph has the site of the
    // part that queued it: the item's name under the item bar.
    constexpr std::uint32_t kContext = 0x09100000u;
    constexpr std::uint32_t kEntries = 0x09110000u;
    constexpr std::uint32_t kGlyphStack = 0x09F10000u;
    memory.store32(0x08A632C8u, kContext);
    memory.store32(kContext + 320u, kEntries);
    memory.store8(kContext + 308u, 0u);
    watch_for_tests();
    packet_filed(runtime, filed_through(memory, {item_bar, root}, 0x09007000u, 16u));
    memory.store8(kContext + 308u, 1u);
    memory.store32(kGlyphStack + 44u, 0x0889384Cu);
    memory.store32(kGlyphStack + 20u, kEntries);
    psprecomp::AllegrexContext glyph{};
    glyph.gpr[29] = kGlyphStack;
    glyph.gpr[31] = 0x08893C2Cu;
    glyph.gpr[5] = 0x09007100u;
    glyph.gpr[6] = 16u;
    packet_filed(runtime, glyph);
    check(site_of(0x09007110u, 0u) == Site::ItemBar, "the item's name takes the item bar's site");
}

// MHFU_TRACE_HUD and MHFU_TRACE_CAPTURE: each packet names its site and the
// registers at the filing call, and the entries a call queued are listed under
// it, so a glyph can be followed back to the code that queued its text.
void test_trace_lines() {
    psprecomp::Runtime runtime;
    auto &memory = runtime.memory();
    const HudFunction &quest = hud_functions()[2];
    const StackFrame root{quest.address, quest.first, quest.saved_ra};
    const StackFrame item_bar{0x08845A34u, 0x27BDFFE0u, 0xAFBF001Cu};
    constexpr std::uint32_t kContext = 0x09100000u;
    constexpr std::uint32_t kEntries = 0x09110000u;
    constexpr std::uint32_t kGlyphStack = 0x09F10000u;
    memory.store32(0x08A632C8u, kContext);
    memory.store32(kContext + 320u, kEntries);
    memory.store8(kContext + 308u, 0u);
    memory.store16(kEntries, 336u);
    memory.store16(kEntries + 2u, 244u);
    memory.store16(kEntries + 12u, 186u);
    memory.store16(kEntries + 14u, 122u);
    watch_for_tests();
    trace_for_tests();

    std::ostringstream out;
    std::streambuf *const kept = std::cout.rdbuf(out.rdbuf());
    psprecomp::AllegrexContext filed = filed_through(memory, {item_bar, root}, 0x09008000u, 16u);
    filed.gpr[16] = 0x1234u;
    packet_filed(runtime, filed);
    memory.store8(kContext + 308u, 1u);
    memory.store32(kGlyphStack + 44u, 0x0889384Cu);
    memory.store32(kGlyphStack + 20u, kEntries);
    psprecomp::AllegrexContext glyph{};
    glyph.gpr[29] = kGlyphStack;
    glyph.gpr[31] = 0x08893C2Cu;
    glyph.gpr[5] = 0x09008100u;
    glyph.gpr[6] = 16u;
    packet_filed(runtime, glyph);
    overhead_name_begin(runtime, filed_through(memory, {root}, 0u, 0u));
    memory.store8(kContext + 308u, 2u);
    overhead_name_end(runtime);
    std::cout.rdbuf(kept);
    watch_for_tests();

    const std::string text = out.str();
    check(text.find("packet=09008000 words=16 bucket=0 list=00000000 HUD quest site=items |") != std::string::npos,
        "a packet's line names its site");
    check(text.find(" s=00001234,") != std::string::npos, "and the saved registers at the call");
    check(text.find("queued q=0 by=above | 09110000(336,244)") != std::string::npos,
        "the entry the item bar queued is listed with its point");
    check(text.find("packet=09008100 words=16 bucket=0 list=00000000 HUD quest site=items |") != std::string::npos &&
            text.find(" | queued text 09110000") != std::string::npos,
        "its glyph's packet has the item bar's site and names the entry");
    check(text.find("queued q=0 by=overhead-name | 0911000C(186,122)") != std::string::npos,
        "the name over a teammate is listed as queued by that call");
}

// The names over other hunters in an ad hoc quest come from resident
// 0x088392F4. It counts while game_task's quest HUD is loaded, and in the
// village, where the lobby overlay has that slot, it does not.
void test_names_over_characters() {
    psprecomp::Runtime runtime;
    auto &memory = runtime.memory();
    constexpr std::uint32_t kLabels = 0x088392F4u;
    watch_for_tests();
    memory.store32(0x09A60298u, 0u);

    psprecomp::AllegrexContext ctx = filed_from(memory, kLabels, 0x27BDFF60u, 0xAFBF002Cu, 0x09004000u, 16u);
    packet_filed(runtime, ctx);
    check(!name_at(0x09004010u, 0u).is_name, "outside a quest the labels are not names for the Names size");

    memory.store32(0x09A60298u, 0x27BDFFA0u);
    memory.store32(0x09A6029Cu, 0xAFBF001Cu);
    ctx = filed_from(memory, kLabels, 0x27BDFF60u, 0xAFBF002Cu, 0x09005000u, 16u);
    packet_filed(runtime, ctx);
    const NameDraw named = name_at(0x09005010u, 0u);
    check(named.is_name && !named.glyph, "in a quest a label's packet is a name");
    check(owner_of(0x09005010u, 0u) == Owner::Other,
        "the labels are not the quest HUD, so no other piece's size reaches them");
    check(!name_at(0x09005040u, 0u).is_name, "a draw past the label's packet is not a name");

    ctx = filed_from(memory, kLabels, 0x27BDFF60u, 0xAFBF0028u, 0x09006000u, 16u);
    packet_filed(runtime, ctx);
    check(!name_at(0x09006010u, 0u).is_name, "other words at the labels' address do not count");
}

// The name over a teammate in an ad hoc quest: queued while resident
// 0x08842628 runs, after the party list's own name, by a path no text
// wrapper sees. Traced with MHFU_TRACE_TEXT_FROM.
void test_overhead_name() {
    psprecomp::Runtime runtime;
    auto &memory = runtime.memory();
    const HudFunction &quest = hud_functions()[2];
    constexpr std::uint32_t kContext = 0x09100000u;
    constexpr std::uint32_t kEntries = 0x09110000u;
    memory.store32(0x08A632C8u, kContext);
    memory.store32(kContext + 320u, kEntries);
    const auto queued = [&](std::uint8_t length) { memory.store8(kContext + 308u, length); };
    const auto place = [&](std::uint32_t index, std::int16_t x, std::int16_t y) {
        memory.store16(kEntries + index * 12u, static_cast<std::uint16_t>(x));
        memory.store16(kEntries + index * 12u + 2u, static_cast<std::uint16_t>(y));
    };
    // A glyph of entry `index`, filed as the per-character draw files it.
    constexpr std::uint32_t kGlyphStack = 0x09F10000u;
    const auto glyph = [&](std::uint32_t index, std::uint32_t packet) {
        memory.store32(kGlyphStack + 44u, 0x0889384Cu);
        memory.store32(kGlyphStack + 20u, kEntries + index * 12u);
        psprecomp::AllegrexContext ctx{};
        ctx.gpr[29] = kGlyphStack;
        ctx.gpr[31] = 0x08893C2Cu;
        ctx.gpr[5] = packet;
        ctx.gpr[6] = 16u;
        packet_filed(runtime, ctx);
    };
    watch_for_tests();

    // The party list's name, then the name over the teammate.
    place(0u, 26, 58);
    queued(1u);
    const psprecomp::AllegrexContext hud = filed_from(memory, quest.address, quest.first, quest.saved_ra, 0u, 0u);
    overhead_name_begin(runtime, hud);
    place(1u, 186, 122);
    queued(2u);
    overhead_name_end(runtime);
    glyph(0u, 0x09007000u);
    glyph(1u, 0x09007100u);
    const NameDraw over = name_at(0x09007110u, 0u);
    check(over.is_name && over.glyph, "a letter of the name over a teammate is a name");
    check(over.has_origin && over.origin_x == 186.0f && over.origin_y == 122.0f,
        "it is sized about the point over that hunter");
    check(owner_of(0x09007110u, 0u) == Owner::QuestHud, "it is the quest HUD's, so hiding the HUD hides it");
    check(!name_at(0x09007010u, 0u).is_name, "the party list's name queued before it is not");

    // The same function called by other code queues no name.
    watch_for_tests();
    queued(0u);
    const psprecomp::AllegrexContext menu = filed_from(memory, 0x09A70000u, 0x27BDFFF0u, 0xAFBF000Cu, 0u, 0u);
    overhead_name_begin(runtime, menu);
    place(0u, 186, 122);
    queued(1u);
    overhead_name_end(runtime);
    glyph(0u, 0x09007200u);
    check(!name_at(0x09007210u, 0u).is_name, "outside the quest HUD its text is not a name");
}

// The names over other hunters in the gathering hall. Traced with
// MHFU_TRACE_CAPTURE: resident 0x088351F0 files the weapon icon beside a
// name through 0x08835470, and the name is queued by a path no wrapper sees.
// In the village 0x088351F0 also draws the prompt over the hunter, which is
// not part of the name. The
// name and its icon glide under frame interpolation and take the Names size,
// only where the draw is the size of a tag.
void test_hall_names() {
    psprecomp::Runtime runtime;
    auto &memory = runtime.memory();
    constexpr std::uint32_t kLabel = 0x08835470u;
    constexpr std::uint32_t kOverHunter = 0x088351F0u;
    constexpr std::uint32_t kLobby = 0x09A6C248u;
    constexpr std::uint32_t kContext = 0x09100000u;
    constexpr std::uint32_t kEntries = 0x09110000u;
    memory.store32(0x08A632C8u, kContext);
    memory.store32(kContext + 320u, kEntries);
    const auto queued = [&](std::uint8_t length) { memory.store8(kContext + 308u, length); };
    const auto place = [&](std::uint32_t index, std::int16_t x, std::int16_t y) {
        memory.store16(kEntries + index * 12u, static_cast<std::uint16_t>(x));
        memory.store16(kEntries + index * 12u + 2u, static_cast<std::uint16_t>(y));
    };
    constexpr std::uint32_t kGlyphStack = 0x09F10000u;
    const auto glyph = [&](std::uint32_t index, std::uint32_t packet) {
        memory.store32(kGlyphStack + 44u, 0x0889384Cu);
        memory.store32(kGlyphStack + 20u, kEntries + index * 12u);
        psprecomp::AllegrexContext ctx{};
        ctx.gpr[29] = kGlyphStack;
        ctx.gpr[31] = 0x08893C2Cu;
        ctx.gpr[5] = packet;
        ctx.gpr[6] = 16u;
        packet_filed(runtime, ctx);
    };
    const auto icon = [&](std::uint32_t packet) {
        psprecomp::AllegrexContext ctx = filed_from(memory, kLabel, 0x27BDFF50u, 0xAFBF004Cu, packet, 9u);
        packet_filed(runtime, ctx);
    };
    check(name_label(kLabel) && name_tag_only(kLabel), "the hall name's icon is a name label, limited to a tag");

    // Outside lobby_task the resident function is no name.
    memory.store32(kLobby, 0u);
    watch_for_tests();
    queued(0u);
    icon(0x09008000u);
    check(!name_at(0x09008010u, 0u).is_name, "without lobby_task the icon is not a name");

    // In the hall: the icon, then the name queued after it.
    memory.store32(kLobby, 0x27BDFFF0u);
    memory.store32(kLobby + 4u, 0xAFBF000Cu);
    watch_for_tests();
    queued(0u);
    icon(0x09008000u);
    const NameDraw waiting = name_at(0x09008010u, 0u);
    check(waiting.is_name && waiting.hall && !waiting.has_origin, "the icon waits for its name's point");
    place(0u, 206, 123);
    queued(1u);
    glyph(0u, 0x09008100u);
    const NameDraw letter = name_at(0x09008110u, 0u);
    check(letter.is_name && letter.glyph && letter.hall && letter.tag_only, "a letter of the hall name is a hall tag");
    check(letter.has_origin && letter.origin_x == 206.0f && letter.origin_y == 123.0f,
        "it moves with the point the game placed it at");
    const NameDraw placed = name_at(0x09008010u, 0u);
    check(placed.has_origin && placed.origin_x == 206.0f && placed.origin_y == 123.0f,
        "the icon takes the point of the name queued after it");

    // A label followed by other code's packet queued no name: text after
    // that is not one, and the icon keeps no point.
    watch_for_tests();
    queued(0u);
    icon(0x09009000u);
    psprecomp::AllegrexContext menu = filed_from(memory, 0x09A70000u, 0x27BDFFF0u, 0xAFBF000Cu, 0x09009200u, 4u);
    packet_filed(runtime, menu);
    place(0u, 50, 60);
    queued(1u);
    glyph(0u, 0x09009100u);
    check(!name_at(0x09009110u, 0u).is_name, "text queued after other code's packet is no name");
    check(!name_at(0x09009010u, 0u).has_origin, "an icon with no name after it keeps no point");

    // The prompt over the hunter (":Open Item Box", its button and the bar's
    // end at (155, 16)-(187, 32)) is filed by 0x088351F0 after the name. Its
    // pieces and the text it queues are no name, so the Names size never
    // reaches them.
    check(!name_label(kOverHunter), "the call that also draws the prompt is no name label");
    watch_for_tests();
    queued(0u);
    icon(0x0900A000u);
    place(0u, 229, 78);
    queued(1u);
    glyph(0u, 0x0900A100u);
    psprecomp::AllegrexContext prompt = filed_from(memory, kOverHunter, 0x27BDFFE0u, 0xAFBF001Cu, 0x0900A200u, 12u);
    packet_filed(runtime, prompt);
    place(1u, 36, 18);
    queued(2u);
    glyph(1u, 0x0900A300u);
    check(name_at(0x0900A110u, 0u).is_name, "the name before the prompt is still a name");
    check(!name_at(0x0900A210u, 0u).is_name, "the prompt's bar end is no name");
    check(!name_at(0x0900A310u, 0u).is_name, "the prompt's text is no name");
}

void test_capture_schedule() {
    CaptureSchedule c;
    check(!c.pending() && !c.packets(0u) && !c.sprites(0u), "nothing is traced before a screenshot");
    c.at_flip(10u);
    check(!c.pending() && !c.packets(11u), "a flip without a screenshot arms nothing");

    // A screenshot after frame 10's flip: the flip of frame 11 arms it, and
    // frame 11 is left for the wrappers to go in.
    c.request();
    check(c.pending() && !c.packets(10u) && !c.sprites(11u), "a request traces nothing before the next flip");
    c.at_flip(11u);
    check(!c.packets(11u) && c.packets(12u) && c.packets(13u) && !c.packets(14u),
        "the packets of the two frames after the arming flip are traced");
    check(!c.sprites(12u) && c.sprites(13u) && c.sprites(14u) && !c.sprites(15u),
        "the sprites of the frames those lists are drawn in are traced");
    check(c.pending(), "the wrappers stay wanted once a capture was taken");

    // A later screenshot moves the window; the earlier one is over.
    c.request();
    c.at_flip(100u);
    check(!c.packets(12u) && c.packets(101u) && c.sprites(103u), "a second screenshot traces its own frames");
}

} // namespace

int main() {
    test_signature_accepts_this_executable();
    test_signature_refuses_a_different_word();
    test_hud_functions();
    test_draw_owner();
    test_draw_site();
    test_trace_lines();
    test_names_over_characters();
    test_overhead_name();
    test_hall_names();
    test_capture_schedule();
    if (failures != 0) {
        std::cerr << failures << " failure(s)\n";
        return 1;
    }
    std::cout << "game hud tests passed\n";
    return 0;
}
