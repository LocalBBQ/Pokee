// Fast item box: the shortened lock and flight, the code check, and the
// wrapper around a stand-in for the quest HUD. No game data.
#include "game/item_box.hpp"

#include "psprecomp/runtime.hpp"

#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>

namespace {
using namespace mhfu::game::item_box;

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

constexpr std::uint32_t kHud = 0x09900000u;
constexpr std::uint32_t kPlayer = 0x09901000u;
constexpr std::uint32_t kReturn = 0x0883F8A4u;
int calls{};
std::uint8_t lock_to_leave{kGameLock};
std::uint16_t anim_to_leave{kGameAnim};

// What the HUD does on the frame of a withdrawal: the box has stored its
// lock and started the icon's flight.
void game_hud(psprecomp::Runtime &runtime, psprecomp::AllegrexContext &ctx) {
    ++calls;
    runtime.memory().store32(ctx.gpr[4] + kPlayerPointer, kPlayer);
    runtime.memory().store8(kPlayer + kLockByte, lock_to_leave);
    runtime.memory().store16(ctx.gpr[4] + kAnimHalf, anim_to_leave);
    ctx.gpr[4] = 0u;
    ctx.pc = ctx.gpr[31];
}

void write_signature(psprecomp::GuestMemory &memory) {
    for (const CodeWord &word : code_signature()) memory.store32(word.address, word.word);
}

void run_hud(psprecomp::Runtime &runtime) {
    psprecomp::AllegrexContext ctx{};
    ctx.gpr[4] = kHud;
    ctx.gpr[31] = kReturn;
    runtime.function_at(kHudUpdate)(runtime, ctx);
    check(ctx.pc == kReturn, "the HUD returns to its caller");
}

void test_shortened_values() {
    check(shortened_lock(kGameLock) == 1u, "the game's 30-frame lock becomes one frame");
    check(shortened_lock(2u) == 1u, "any lock left of more than a frame becomes one frame");
    check(shortened_lock(1u) == 1u && shortened_lock(0u) == 0u, "one frame or none is left as it is");
    check(shortened_lock(kGameLock + 1u) == kGameLock + 1u && shortened_lock(0xFFu) == 0xFFu,
        "a lock the game never sets is not touched");
    check(shortened_anim(kGameAnim) == 0u, "the game's 31-frame flight is cleared");
    check(shortened_anim(1u) == 0u, "a flight still in the game's range is cleared");
    check(shortened_anim(0u) == 0u, "no flight is left as none");
    check(shortened_anim(kGameAnim + 1u) == kGameAnim + 1u, "a halfword the game never sets is not touched");
}

void test_code_check() {
    psprecomp::GuestMemory memory(32u * 1024u * 1024u);
    check(!code_matches(memory), "empty memory is not the executable");
    write_signature(memory);
    check(code_matches(memory), "the traced words are the executable");
    memory.store32(code_signature()[2].address, 0u);
    check(!code_matches(memory), "one different word is another executable");
}

void test_wrapper() {
    psprecomp::Runtime runtime(32u * 1024u * 1024u);
    write_signature(runtime.memory());
    mhfu::game::item_box::frame(runtime);
    check(runtime.function_at(kHudUpdate) == nullptr, "nothing is put in place before the code is installed");

    runtime.register_function(kHudUpdate, &game_hud, "game_hud");
    mhfu::game::item_box::frame(runtime);
    check(runtime.function_at(kHudUpdate) != &game_hud, "the HUD is wrapped");
    mhfu::game::item_box::frame(runtime);
    calls = 0;
    run_hud(runtime);
    check(calls == 1, "the game's HUD runs once");
    check(runtime.memory().load8(kPlayer + kLockByte) == 1u, "the box's lock is one frame");
    check(runtime.memory().load16(kHud + kAnimHalf) == 0u, "the icon's flight is cleared");

    lock_to_leave = 0u;
    anim_to_leave = 0u;
    run_hud(runtime);
    check(runtime.memory().load8(kPlayer + kLockByte) == 0u, "no lock is left as none");
    check(runtime.memory().load16(kHud + kAnimHalf) == 0u, "no flight is left as none");
    lock_to_leave = kGameLock;
    anim_to_leave = kGameAnim;

    set_environment("MHFU_FAST_ITEM_BOX", "0");
    run_hud(runtime);
    check(runtime.memory().load8(kPlayer + kLockByte) == kGameLock, "MHFU_FAST_ITEM_BOX=0 leaves the lock");
    check(runtime.memory().load16(kHud + kAnimHalf) == kGameAnim, "and the flight");
    set_environment("MHFU_FAST_ITEM_BOX", "");
}

void test_other_code() {
    psprecomp::Runtime runtime(32u * 1024u * 1024u);
    runtime.register_function(kHudUpdate, &game_hud, "other_code");
    mhfu::game::item_box::frame(runtime);
    check(runtime.function_at(kHudUpdate) == &game_hud, "code that does not match is not wrapped");
}

} // namespace

int main() {
    const std::filesystem::path data = std::filesystem::temp_directory_path() / "pokee_item_box_tests";
    std::filesystem::create_directories(data);
    set_environment("MHFU_DATA_DIR", data.string());
    set_environment("MHFU_FAST_ITEM_BOX", "");
    set_environment("MHFU_TRACE_ITEM_BOX", "");
    test_shortened_values();
    test_code_check();
    test_other_code();
    test_wrapper();
    std::error_code ec;
    std::filesystem::remove_all(data, ec);
    if (failures != 0) {
        std::cerr << failures << " failure(s)\n";
        return 1;
    }
    std::cout << "item box tests passed\n";
    return 0;
}
