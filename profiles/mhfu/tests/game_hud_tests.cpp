// No game code or game data is needed: the words hiding the HUD checks are
// written into guest memory as stand-ins.
#include "gpu/game_hud.hpp"
#include "psprecomp/allegrex_context.hpp"
#include "psprecomp/runtime.hpp"

#include <iostream>

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
// name, and lobby_task then queues the name by a path no wrapper sees. The
// name and its icon only glide under frame interpolation; they are never sized.
void test_hall_names() {
    psprecomp::Runtime runtime;
    auto &memory = runtime.memory();
    constexpr std::uint32_t kLabel = 0x088351F0u;
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
        psprecomp::AllegrexContext ctx = filed_from(memory, kLabel, 0x27BDFFE0u, 0xAFBF001Cu, packet, 9u);
        packet_filed(runtime, ctx);
    };
    check(name_label(kLabel) && !name_tag_only(kLabel), "the hall name's icon is a name label");

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
    check(waiting.is_name && waiting.glide_only && !waiting.has_origin, "the icon waits for its name's point");
    place(0u, 206, 123);
    queued(1u);
    glyph(0u, 0x09008100u);
    const NameDraw letter = name_at(0x09008110u, 0u);
    check(letter.is_name && letter.glyph && letter.glide_only, "a letter of the hall name glides");
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
