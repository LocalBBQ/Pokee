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
    check(code_signature().size() == 21u, "every anchor is checked");
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
    watch_for_tests();

    // The quest HUD files a packet: a draw inside it may be resized.
    psprecomp::AllegrexContext ctx = filed_from(memory, quest.address, quest.first, quest.saved_ra, 0x09000000u, 16u);
    packet_filed(runtime, ctx);
    check(owner_of(0x09000010u, 0u) == Owner::QuestHud, "a draw in a quest HUD packet is the quest HUD's");
    check(owner_of(0x09000040u, 0u) == Owner::Other, "a draw past the packet's end is not");
    check(owner_of(0u, 0x09000020u) == Owner::QuestHud, "a draw called from that packet is the quest HUD's");
    check(owner_of(0u, 0u) == Owner::Other, "a draw with no address is no one's");

    // The village interface is HUD for hiding, and a shop or a dialog for
    // resizing: its packets are never resized.
    ctx = filed_from(memory, village.address, village.first, village.saved_ra, 0x09001000u, 16u);
    packet_filed(runtime, ctx);
    check(hides(0x09001010u, 0u) == false, "the HUD is shown, so nothing is left out");
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

} // namespace

int main() {
    test_signature_accepts_this_executable();
    test_signature_refuses_a_different_word();
    test_hud_functions();
    test_draw_owner();
    if (failures != 0) {
        std::cerr << failures << " failure(s)\n";
        return 1;
    }
    std::cout << "game hud tests passed\n";
    return 0;
}
