// True attack: the code check, the factor table and the switch. No game data.
#include "game/true_attack.hpp"

#include "psprecomp/runtime.hpp"
#include "settings/settings.hpp"

#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>

namespace {
using namespace mhfu::game::true_attack;

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
    // _putenv_s removes a variable set to "", so do the same here.
    if (value.empty())
        unsetenv(name);
    else
        setenv(name, value.c_str(), 1);
#endif
}

constexpr std::uint32_t kMemory = 32u * 1024u * 1024u;
constexpr std::uint32_t kGreatSword = 0u;
constexpr std::uint32_t kHammer = 2u;
constexpr std::uint32_t kBow = 10u;

void write_signature(psprecomp::GuestMemory &memory) {
    for (const CodeWord &word : code_signature()) memory.store32(word.address, word.word);
}

void write_game_table(psprecomp::GuestMemory &memory) {
    for (std::uint32_t i = 0; i < kClasses; ++i) memory.store32(kFactors + i * 4u, kGameFactors[i]);
}

std::uint32_t factor(const psprecomp::GuestMemory &memory, std::uint32_t weapon_class) {
    return memory.load32(kFactors + weapon_class * 4u);
}

// What a screen shows for `attack`, as 0x0881F5FC computes it.
std::uint32_t shown(const psprecomp::GuestMemory &memory, std::uint32_t weapon_class, std::uint32_t attack) {
    return attack * factor(memory, weapon_class) / 100u;
}

bool table_is_game(const psprecomp::GuestMemory &memory) {
    for (std::uint32_t i = 0; i < kClasses; ++i)
        if (factor(memory, i) != kGameFactors[i]) return false;
    return true;
}

void test_code_check() {
    psprecomp::GuestMemory memory(kMemory);
    check(!code_matches(memory), "empty memory is not the executable");
    write_signature(memory);
    check(code_matches(memory), "the traced words are the executable");
    memory.store32(code_signature()[4].address, 0x2484ACE0u);
    check(!code_matches(memory), "one different word is another executable");
}

void test_apply() {
    psprecomp::GuestMemory memory(kMemory);
    write_game_table(memory);
    check(!apply(memory, true), "without the readers' code nothing is applied");
    check(table_is_game(memory), "and the table is left as it was");

    write_signature(memory);
    check(shown(memory, kGreatSword, 140u) == 672u, "the game shows a great sword's 140 as 672");
    check(apply(memory, true), "the game's table is recognised");
    for (std::uint32_t i = 0; i < kClasses; ++i) check(factor(memory, i) == kTrueFactor, "on, every factor is 100");
    check(shown(memory, kGreatSword, 140u) == 140u, "on, the great sword shows 140");
    check(shown(memory, kHammer, 37u) == 37u && shown(memory, kBow, 1u) == 1u, "on, any attack shows itself");
    check(apply(memory, true), "applying on twice is recognised");

    check(apply(memory, false), "the true table is recognised");
    check(table_is_game(memory), "off, the game's factors are back");
    check(shown(memory, kHammer, 100u) == 520u, "off, a hammer's 100 shows 520 again");

    memory.store32(kFactors + 3u * 4u, 250u);
    check(!apply(memory, true), "a table that is neither is not touched");
    check(factor(memory, 3u) == 250u && factor(memory, 0u) == kGameFactors[0], "and keeps every word");

    write_game_table(memory);
    memory.store32(kFactors, kTrueFactor);
    check(!apply(memory, false), "a half-written table is not recognised");
}

void test_frame() {
    psprecomp::Runtime runtime(kMemory);
    psprecomp::GuestMemory &memory = runtime.memory();
    write_game_table(memory);
    auto &settings = mhfu::settings::current();
    settings.true_attack = true;
    frame(runtime);
    check(table_is_game(memory), "on, other code keeps the game's table");

    write_signature(memory);
    frame(runtime);
    check(factor(memory, kGreatSword) == kTrueFactor, "on, the frame writes the true factors");
    frame(runtime);
    check(factor(memory, kBow) == kTrueFactor, "and keeps them");

    settings.true_attack = false;
    frame(runtime);
    check(table_is_game(memory), "turned off in the session, the game's factors come back");
    settings.true_attack = true;
    frame(runtime);
    check(factor(memory, kHammer) == kTrueFactor, "and turned on again, the true ones");
}

} // namespace

int main() {
    const std::filesystem::path data = std::filesystem::temp_directory_path() / "pokee_true_attack_tests";
    std::filesystem::create_directories(data);
    set_environment("MHFU_DATA_DIR", data.string());
    set_environment("MHFU_TRUE_ATTACK", "");
    check(!mhfu::settings::current().true_attack, "true attack is off by default");
    test_code_check();
    test_apply();
    test_frame();
    std::error_code ec;
    std::filesystem::remove_all(data, ec);
    if (failures != 0) {
        std::cerr << failures << " failure(s)\n";
        return 1;
    }
    std::cout << "true attack tests passed\n";
    return 0;
}
