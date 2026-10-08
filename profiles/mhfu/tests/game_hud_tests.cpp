// No game code or game data is needed: the words hiding the HUD checks are
// written into guest memory as stand-ins.
#include "gpu/game_hud.hpp"
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
    check(functions.size() == 3u, "village, quest status and quest labels");
    check(functions[0].address == 0x09AE38F8u, "the village layer");
    check(functions[1].address == 0x09A665F0u, "the quest status layer");
    check(functions[2].address == 0x09B62488u, "the quest labels");

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
    memory.store32(functions[0].address, functions[1].first);
    memory.store32(functions[0].address + 4u, functions[1].saved_ra);
    check(!hud_function_matches(memory, functions[0].address), "the other overlay's words do not count");
}

} // namespace

int main() {
    test_signature_accepts_this_executable();
    test_signature_refuses_a_different_word();
    test_hud_functions();
    if (failures != 0) {
        std::cerr << failures << " failure(s)\n";
        return 1;
    }
    std::cout << "game hud tests passed\n";
    return 0;
}
