// True element: the format rewrite, the code check and the wrapper around a
// stand-in for the game's sprintf. No game data.
#include "game/true_element.hpp"

#include "psprecomp/runtime.hpp"
#include "settings/settings.hpp"

#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>

namespace {
using namespace mhfu::game::true_element;

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
constexpr std::uint32_t kFormat = 0x09800000u;
constexpr std::uint32_t kReturn = 0x08823788u;
std::string seen_format;
int calls{};

void put_string(psprecomp::GuestMemory &memory, std::uint32_t address, const std::string &text) {
    for (std::size_t i = 0; i < text.size(); ++i) memory.store8(address + static_cast<std::uint32_t>(i), text[i]);
    memory.store8(address + static_cast<std::uint32_t>(text.size()), 0u);
}

std::string read_string(const psprecomp::GuestMemory &memory, std::uint32_t address) {
    std::string text;
    for (std::uint8_t c = memory.load8(address); c != 0u; c = memory.load8(++address))
        text.push_back(static_cast<char>(c));
    return text;
}

// The format after the rewrite, as sprintf would read it.
std::string rewritten(const std::string &format) {
    psprecomp::GuestMemory memory(kMemory);
    put_string(memory, kFormat, format);
    const std::optional<Rewrite> rewrite = rewrite_for(memory, kFormat);
    if (!rewrite) return format;
    for (std::uint32_t i = 0; i < 4u; ++i) memory.store8(rewrite->address + i, rewrite->bytes[i]);
    return read_string(memory, kFormat);
}

void test_rewrite() {
    check(rewritten(":~C%02d%3d0") == ":~C%02d%4d", "the element value keeps its width without the zero");
    check(rewritten("(Total Val):~C%02d%2d0") == "(Total Val):~C%02d%3d", "the total keeps its width too");
    check(rewritten("~C%02d%d0") == "~C%02d%d", "a format without a width loses only the zero");
    check(rewritten(":~C%02d%3d") == ":~C%02d%3d", "a format without the zero is the game's");
    check(rewritten("Lvl %d0 ok") == "Lvl %d0 ok", "a zero that does not end the format is the game's");
    check(rewritten("%9d0") == "%9d0", "a width of 9 has no wider width in one digit");
    check(rewritten("100%%d0") == "100%%d0", "an escaped percent is no number");
    check(rewritten("%%3d0") == "%%3d0", "an escaped percent with a width is no number");
    check(rewritten("d0") == "d0" && rewritten("") == "", "short formats are the game's");

    psprecomp::GuestMemory memory(kMemory);
    for (std::uint32_t i = 0; i < kLongestFormat; ++i) memory.store8(kFormat + i, 'x');
    check(!rewrite_for(memory, kFormat), "a format without an end in reach is left alone");
    check(!rewrite_for(memory, 0x7FFFFFF0u), "a format outside memory is left alone");
}

void test_code_check() {
    psprecomp::GuestMemory memory(kMemory);
    check(!code_matches(memory), "empty memory is not the executable");
    for (const CodeWord &word : code_signature()) memory.store32(word.address, word.word);
    check(code_matches(memory), "the traced words are the executable");
    memory.store32(code_signature()[1].address, 0x00C08021u);
    check(!code_matches(memory), "one different word is another executable");
}

// The game's sprintf as far as the format goes.
void game_sprintf(psprecomp::Runtime &runtime, psprecomp::AllegrexContext &ctx) {
    ++calls;
    seen_format = read_string(runtime.memory(), ctx.gpr[5]);
    ctx.pc = ctx.gpr[31];
}

void run_sprintf(psprecomp::Runtime &runtime) {
    psprecomp::AllegrexContext ctx{};
    ctx.gpr[5] = kFormat;
    ctx.gpr[31] = kReturn;
    runtime.function_at(kSprintf)(runtime, ctx);
    check(ctx.pc == kReturn, "sprintf returns to its caller");
}

void test_wrapper() {
    psprecomp::Runtime runtime(kMemory);
    psprecomp::GuestMemory &memory = runtime.memory();
    for (const CodeWord &word : code_signature()) memory.store32(word.address, word.word);
    runtime.register_function(kSprintf, &game_sprintf, "game_sprintf");
    auto &settings = mhfu::settings::current();

    settings.true_attack = false;
    frame(runtime);
    check(runtime.function_at(kSprintf) == &game_sprintf, "off, sprintf is not wrapped");

    settings.true_attack = true;
    frame(runtime);
    check(runtime.function_at(kSprintf) != &game_sprintf, "on, sprintf is wrapped");
    frame(runtime);

    put_string(memory, kFormat, ":~C%02d%3d0");
    calls = 0;
    run_sprintf(runtime);
    check(calls == 1, "the game's sprintf runs once");
    check(seen_format == ":~C%02d%4d", "it prints the element without the zero");
    check(read_string(memory, kFormat) == ":~C%02d%4d",
        "the text stays changed, for a sprintf that finishes after the wrapper returns");
    run_sprintf(runtime);
    check(seen_format == ":~C%02d%4d", "and the next print reads it as changed");

    constexpr std::uint32_t kOther = kFormat + 0x100u;
    put_string(memory, kOther, "%s~C%02d%2d");
    psprecomp::AllegrexContext other{};
    other.gpr[5] = kOther;
    other.gpr[31] = kReturn;
    runtime.function_at(kSprintf)(runtime, other);
    check(seen_format == "%s~C%02d%2d" && read_string(memory, kOther) == "%s~C%02d%2d", "other formats are the game's");

    settings.true_attack = false;
    frame(runtime);
    check(runtime.function_at(kSprintf) != &game_sprintf, "turned off, the wrapper stays for the run");
    check(read_string(memory, kFormat) == ":~C%02d%3d0", "but the game's text is put back");
    run_sprintf(runtime);
    check(seen_format == ":~C%02d%3d0", "and prints whole");

    settings.true_attack = true;
    run_sprintf(runtime);
    check(seen_format == ":~C%02d%4d", "turned on again, the next print loses the zero");
    put_string(memory, kFormat, ":~C%02d%3d0");
    run_sprintf(runtime);
    check(seen_format == ":~C%02d%4d", "text the game loads again is changed again");
    put_string(memory, kFormat, "Other text");
    settings.true_attack = false;
    frame(runtime);
    check(read_string(memory, kFormat) == "Other text", "text loaded over a change is not touched");
}

void test_other_code() {
    psprecomp::Runtime runtime(kMemory);
    runtime.register_function(kSprintf, &game_sprintf, "other_code");
    mhfu::settings::current().true_attack = true;
    frame(runtime);
    check(runtime.function_at(kSprintf) == &game_sprintf, "code that does not match is not wrapped");
}

} // namespace

int main() {
    const std::filesystem::path data = std::filesystem::temp_directory_path() / "pokee_true_element_tests";
    std::filesystem::create_directories(data);
    set_environment("MHFU_DATA_DIR", data.string());
    set_environment("MHFU_TRUE_ATTACK", "");
    set_environment("MHFU_TRACE_ELEMENT", "1");
    check(!mhfu::settings::current().true_attack, "true values are off by default");
    test_rewrite();
    test_code_check();
    test_other_code();
    test_wrapper();
    std::error_code ec;
    std::filesystem::remove_all(data, ec);
    if (failures != 0) {
        std::cerr << failures << " failure(s)\n";
        return 1;
    }
    std::cout << "true element tests passed\n";
    return 0;
}
