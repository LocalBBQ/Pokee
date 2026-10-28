#include "game/title_menu.hpp"

#include "game/hooks.hpp"
#include "game/install_data.hpp"

#include "psprecomp/guest_memory.hpp"
#include "psprecomp/runtime.hpp"

#include <cstdlib>
#include <iostream>
#include <optional>
#include <string>

// The wrapper and its place in the frame (title_menu.hpp); what it does with
// guest memory is in title_menu.cpp, which needs no game.
namespace mhfu::game::title_menu {
namespace {

struct State {
    psprecomp::Runtime::RecompiledFunction original{};
    std::optional<HiddenLabel> hidden;
    bool reported{};
    bool trace{};
};

State &state() {
    static State s;
    return s;
}

void menu_update(psprecomp::Runtime &runtime, psprecomp::AllegrexContext &ctx) {
    State &s = state();
    psprecomp::GuestMemory &memory = runtime.memory();
    const std::uint32_t task = ctx.gpr[4];
    const std::uint32_t object = memory.load32(kCursorObject);
    if (memory.contains(task + kStepOffset, 4u) && memory.load32(task + kStepOffset) == kInputStep &&
        memory.contains(object + kCursorOffset, 4u)) {
        const std::uint32_t cursor = memory.load32(object + kCursorOffset);
        const std::uint32_t pressed = memory.load32(kPressed);
        const std::uint32_t given = cursor_before_input(cursor, pressed);
        if (given != cursor) {
            memory.store32(object + kCursorOffset, given);
            if (s.trace)
                std::cerr << "[title menu] cursor " << cursor << " given as " << given << " (pressed 0x" << std::hex
                          << pressed << std::dec << ")\n";
        }
    }
    ctx.pc = kMenuUpdate;
    s.original(runtime, ctx);
}

} // namespace

void frame(psprecomp::Runtime &runtime) {
    State &s = state();
    psprecomp::GuestMemory &memory = runtime.memory();
    if (install_data::served(memory) == nullptr) return;
    if (!code_matches(memory)) {
        // demo_task has gone: the text is the game's again.
        if (s.hidden) restore_label(memory, *s.hidden);
        s.hidden.reset();
        return;
    }
    // The label stays hidden while the menu is in: the update can go back to
    // the dispatcher at a call into demo_sub and draw after its wrapper has
    // returned (game/hooks.hpp).
    if (!s.hidden || memory.load32(s.hidden->entry) != s.hidden->hidden_offset) {
        HiddenLabel hidden;
        if (hide_label(memory, hidden)) s.hidden = hidden;
    }
    if (runtime.function_at(kMenuUpdate) == &menu_update) return;
    if (!hooks::wrap(runtime, kMenuUpdate, &menu_update, s.original, "mhfu_title_menu")) return;
    if (!s.reported) {
        s.reported = true;
        static const bool trace = [] {
            const char *text = std::getenv("MHFU_TRACE_TITLE_MENU");
            return text != nullptr && *text != '\0' && std::string(text) != "0";
        }();
        s.trace = trace;
        std::cout << "[title menu] DATA INSTALL hidden: the install is served" << std::endl;
    }
}

} // namespace mhfu::game::title_menu
