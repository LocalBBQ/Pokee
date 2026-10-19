// The stall trace (MHFU_TRACE_STALLS): every kind has a name of its own, and
// the time a StallScope measures reaches the [slow-frame] line of its frame.
// No game data and no GPU.
#include "perf/frame_stats.hpp"

#include <cstdlib>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <thread>

namespace {
using namespace mhfu::perf;

int failures{};

void check(bool condition, const std::string &message) {
    if (!condition) {
        ++failures;
        std::cerr << "FAIL: " << message << '\n';
    }
}

void set_environment(const char *name, const char *value) {
#if defined(_WIN32)
    _putenv_s(name, value);
#else
    setenv(name, value, 1);
#endif
}

void test_every_kind_has_its_own_name() {
    std::set<std::string> names;
    for (std::size_t i = 0; i < static_cast<std::size_t>(Stall::Count); ++i) {
        const std::string name = stall_name(static_cast<Stall>(i));
        check(name != "?", "stall kind " + std::to_string(i) + " has a name");
        check(names.insert(name).second, "stall name " + name + " is used once");
    }
    check(std::string(stall_name(Stall::Count)) == "?", "Count is not a kind");
    check(std::string(stall_name(Stall::Texture)) == "texture", "texture creation is named texture");
    check(std::string(stall_name(Stall::Allocate)) == "alloc", "memory allocation is named alloc");
    check(std::string(stall_name(Stall::Cache)) == "cache-save", "the pipeline cache save is named cache-save");
}

std::string slow_frame_line(std::uint64_t virtual_us) {
    std::ostringstream captured;
    std::streambuf *const previous = std::cout.rdbuf(captured.rdbuf());
    end_frame(virtual_us);
    std::cout.rdbuf(previous);
    return captured.str();
}

void test_scopes_reach_the_slow_frame_line() {
    // Starts the frame the stalls below belong to.
    (void)slow_frame_line(0u);
    {
        const StallScope texture(Stall::Texture);
        const StallScope allocate(Stall::Allocate);
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
    {
        const StallScope allocate(Stall::Allocate);
    }
    note_stall(Stall::Cache, std::chrono::milliseconds(3));
    const std::string line = slow_frame_line(33'367u);
    check(line.rfind("[slow-frame] ", 0) == 0u, "a frame over the threshold is reported: " + line);
    check(line.find(" texture ") != std::string::npos && line.find(" texture 0.00 ") == std::string::npos,
        "the texture scope's time is reported: " + line);
    check(line.find(" alloc ") != std::string::npos && line.find(" x2") != std::string::npos,
        "both allocation scopes are counted: " + line);
    check(line.find(" cache-save 3.00 x1") != std::string::npos, "the cache save is reported: " + line);
    check(line.find(" pipeline ") == std::string::npos, "a kind with no stall is left out: " + line);

    // The next frame starts empty.
    const std::string next = slow_frame_line(66'734u);
    check(next.find(" texture ") == std::string::npos && next.find(" none") != std::string::npos,
        "stalls do not carry over to the next frame: " + next);
}

} // namespace

int main() {
    // Read once, at the first use of the trace: every frame counts as slow.
    set_environment("MHFU_TRACE_STALLS", "1");
    set_environment("MHFU_TRACE_STALLS_MS", "0.001");
    test_every_kind_has_its_own_name();
    test_scopes_reach_the_slow_frame_line();
    if (failures != 0) {
        std::cerr << failures << " failure(s)\n";
        return 1;
    }
    std::cout << "frame stats tests passed\n";
    return 0;
}
