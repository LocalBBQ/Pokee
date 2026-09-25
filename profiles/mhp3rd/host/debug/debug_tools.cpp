#include "debug/debug_tools.hpp"

#include "debug/debug_console.hpp"
#include "debug/game_state.hpp"
#include "game/guest_ram.hpp"

#include "adhoc/session.hpp"
#include "hle/hle_common.hpp"

#include "psprecomp/guest_memory.hpp"
#include "psprecomp/runtime.hpp"

#include <cstdlib>
#include <cstring>
#include <deque>
#include <iostream>
#include <utility>

namespace mhp3rd::debug {

using game::GuestRam;

namespace {

constexpr std::size_t kRecentLines = 6u;

std::deque<std::string> &recent_lines() {
    static std::deque<std::string> lines;
    return lines;
}

struct Request {
    std::string what;
    std::function<std::string(Ram &)> change;
};

struct State {
    psprecomp::Runtime *runtime{};
    std::deque<Request> pending;
    HeldCheats held;
    std::vector<std::string> quest;
};

State &state() {
    static State s;
    return s;
}

void run_pending(Ram &ram) {
    State &s = state();
    while (!s.pending.empty()) {
        Request r = std::move(s.pending.front());
        s.pending.pop_front();
        if (const std::string why = blocked_reason(); !why.empty()) {
            log("refused (" + why + "): " + r.what);
            continue;
        }
        log(r.change(ram));
    }
}

} // namespace

bool enabled() {
    static const bool on = [] {
        const char *text = std::getenv("MHP3RD_DEBUG_MENU");
        const bool value = text != nullptr && std::strcmp(text, "1") == 0;
        if (value)
            std::cout << "[debug] developer tools on (MHP3RD_DEBUG_MENU=1): Debug page in the menu" << std::endl;
        return value;
    }();
    return on;
}

std::string blocked_reason() {
    if (adhoc_session_active() || adhoc_networking_on()) return "ad hoc play is on";
    return {};
}

void request(std::string what, std::function<std::string(Ram &)> change) {
    if (!enabled()) return;
    state().pending.push_back({std::move(what), std::move(change)});
    // The menu runs between game frames; when it pauses the game there is no
    // flip until it closes, so run the request here, still between frames.
    if (psprecomp::Runtime *rt = state().runtime) {
        GuestRam ram(rt->memory());
        run_pending(ram);
    }
}

void read(const std::function<void(const Ram &)> &reader) {
    psprecomp::Runtime *rt = state().runtime;
    if (rt == nullptr) return;
    const GuestRam ram(rt->memory());
    reader(ram);
}

void frame(psprecomp::Runtime &runtime) {
    if (!enabled()) return;
    State &s = state();
    s.runtime = &runtime;
    GuestRam ram(runtime.memory());
    run_pending(ram);
    console_frame(ram);
    if (blocked_reason().empty()) held_cheats_frame(ram, s.held);
    s.quest = quest_lines(ram);
}

HeldCheats held_cheats() { return state().held; }

void set_held_cheats(const HeldCheats &cheats) {
    HeldCheats &held = state().held;
    const auto note = [](const char *name, bool before, bool after) {
        if (before != after) log(std::string(name) + (after ? " on" : " off"));
    };
    note("infinite health", held.health, cheats.health);
    note("infinite stamina", held.stamina, cheats.stamina);
    note("frozen quest timer", held.timer, cheats.timer);
    note("monsters at 1 health", held.one_hit, cheats.one_hit);
    held = cheats;
}

std::vector<std::string> quest_status() { return state().quest; }

void log(const std::string &line) {
    std::cout << "[debug] " << line << std::endl;
    std::deque<std::string> &recent = recent_lines();
    recent.push_back(line);
    while (recent.size() > kRecentLines) recent.pop_front();
}

std::vector<std::string> recent_log() { return {recent_lines().begin(), recent_lines().end()}; }

} // namespace mhp3rd::debug
