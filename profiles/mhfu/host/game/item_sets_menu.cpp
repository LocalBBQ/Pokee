#include "game/item_sets_menu.hpp"

#include <utility>

namespace mhfu::game::item_sets::menu {
namespace {

std::uint8_t wrap(std::uint8_t value, int by, std::uint8_t count) {
    return static_cast<std::uint8_t>((value + by + count) % count);
}

// Up and down, with the game's wrap; whether it moved.
bool move_row(std::uint8_t &row, std::uint16_t buttons, std::uint8_t rows) {
    if ((buttons & kUp) != 0u) {
        row = wrap(row, -1, rows);
        return true;
    }
    if ((buttons & kDown) != 0u) {
        row = wrap(row, 1, rows);
        return true;
    }
    return false;
}

Step ask(State &state, Ask what, bool yes) {
    state.page = Page::Confirm;
    state.ask = what;
    state.yes = yes;
    return {Act::None, state.set, Sound::Confirm};
}

Step refuse(State &state, std::string text) {
    finished(state, std::move(text), true);
    return {Act::None, state.set, Sound::Refuse};
}

Step choose(State &state, std::uint16_t buttons) {
    if (move_row(state.row, buttons, kChooseRows)) return {Act::None, 0u, Sound::Cursor};
    if ((buttons & kConfirm) != 0u) {
        state.page = Page::List;
        state.mode = state.row == 0u ? Mode::Load : Mode::Register;
        state.contents = 0u;
        state.result.clear();
        return {Act::None, 0u, Sound::Confirm};
    }
    if ((buttons & kCancel) != 0u) return {Act::Close, 0u, Sound::Cancel};
    return {};
}

Step list(State &state, std::uint16_t buttons, const Sets &sets, bool pouch_empty) {
    std::uint8_t row = set_row_on_page(state);
    if (move_row(row, buttons, static_cast<std::uint8_t>(kSetsPerPage))) {
        state.set = static_cast<std::uint8_t>(set_page(state) * kSetsPerPage + row);
        state.contents = 0u;
        state.result.clear();
        return {Act::None, state.set, Sound::Cursor};
    }
    if ((buttons & (kL | kR)) != 0u) {
        // The next page of sets, on the same row.
        const std::uint8_t page =
            wrap(set_page(state), (buttons & kL) != 0u ? -1 : 1, static_cast<std::uint8_t>(kSetPages));
        state.set = static_cast<std::uint8_t>(page * kSetsPerPage + row);
        state.contents = 0u;
        state.result.clear();
        return {Act::None, state.set, Sound::Page};
    }
    if ((buttons & (kLeft | kRight)) != 0u) {
        state.contents = wrap(state.contents, (buttons & kLeft) != 0u ? -1 : 1, kPages);
        return {Act::None, state.set, Sound::Page};
    }
    const bool empty = sets[state.set].empty();
    if ((buttons & kConfirm) != 0u) {
        state.result.clear();
        if (state.mode == Mode::Load)
            return empty ? refuse(state, "~C02No set is registered here.") : ask(state, Ask::Load, true);
        if (pouch_empty) return refuse(state, "~C02The pouch is empty.");
        if (empty) return {Act::Register, state.set, Sound::Confirm};
        return ask(state, Ask::Register, false);
    }
    if ((buttons & kDelete) != 0u) {
        state.result.clear();
        return empty ? Step{Act::None, state.set, Sound::Refuse} : ask(state, Ask::Delete, false);
    }
    if ((buttons & kCancel) != 0u) {
        state.page = Page::Choose;
        state.result.clear();
        return {Act::None, state.set, Sound::Cancel};
    }
    return {};
}

Step confirm(State &state, std::uint16_t buttons) {
    if ((buttons & (kLeft | kRight)) != 0u) {
        state.yes = !state.yes;
        return {Act::None, state.set, Sound::Cursor};
    }
    if ((buttons & kConfirm) != 0u) {
        state.page = Page::List;
        if (!state.yes) return {Act::None, state.set, Sound::Cancel};
        const Act act = state.ask == Ask::Load ? Act::Load : state.ask == Ask::Register ? Act::Register : Act::Delete;
        return {act, state.set, Sound::Confirm};
    }
    if ((buttons & kCancel) != 0u) {
        state.page = Page::List;
        return {Act::None, state.set, Sound::Cancel};
    }
    return {};
}

} // namespace

Step step(State &state, std::uint16_t buttons, const Sets &sets, bool pouch_empty) {
    switch (state.page) {
    case Page::Choose: return choose(state, buttons);
    case Page::List: return list(state, buttons, sets, pouch_empty);
    case Page::Confirm: return confirm(state, buttons);
    }
    return {};
}

void finished(State &state, std::string text, bool warning) {
    state.result = std::move(text);
    state.result_warning = warning;
}

std::string message(const State &state) {
    const std::string name = set_row(state.set);
    switch (state.page) {
    case Page::Choose: return {};
    case Page::List:
        if (!state.result.empty()) return state.result;
        return state.mode == Mode::Load ? "Please select the set to load." : "Please select where to\nregister the pouch.";
    case Page::Confirm:
        switch (state.ask) {
        case Ask::Load: return "Load " + name + "?\nThe pouch goes into the box.";
        case Ask::Register: return "Replace " + name + "\nwith the pouch?";
        case Ask::Delete: return "Delete this Set?";
        }
    }
    return {};
}

std::string hint(const State &state) {
    return state.mode == Mode::Load ? "~C00~B01Load Set  ~B02Delete Set" : "~C00~B01Register Set  ~B02Delete Set";
}

std::uint32_t sound_id(Sound sound) {
    switch (sound) {
    case Sound::None: return 0u;
    case Sound::Page: return 17u;
    case Sound::Confirm: return 19u;
    case Sound::Cancel: return 20u;
    case Sound::Refuse: return 21u;
    case Sound::Cursor: return 23u;
    case Sound::Moved: return 136u;
    case Sound::Registered: return 137u;
    }
    return 0u;
}

std::uint32_t sound_bank(Sound sound) {
    return sound == Sound::Moved || sound == Sound::Registered ? 1u : 0u;
}

std::string choose_row(std::uint8_t row) {
    return row == 0u ? "Load Item Set" : "Register Pouch";
}

std::string list_title(Mode mode) {
    return mode == Mode::Load ? "Load Item Set" : "Register Set";
}

std::uint8_t set_page(const State &state) {
    return static_cast<std::uint8_t>(state.set / kSetsPerPage);
}

std::uint8_t set_row_on_page(const State &state) {
    return static_cast<std::uint8_t>(state.set % kSetsPerPage);
}

std::string set_row(std::uint8_t set) {
    return default_name(set);
}

} // namespace mhfu::game::item_sets::menu
