// The Item Sets screen. It reads the hunter, the pouch and the box from
// guest memory between two game frames, and changes them only when a set is
// loaded (game::item_sets::load). The sets themselves live in the port's
// item_sets.txt, by hunter name.

#include "ui/item_sets_screen.hpp"

#include "game/game_data.hpp"
#include "game/guest_ram.hpp"
#include "game/item_sets.hpp"
#include "gpu/vulkan_renderer.hpp"
#include "ui/layer.hpp"
#include "ui/text_input.hpp"
#include "ui/widgets.hpp"

#include <array>
#include <iostream>
#include <map>
#include <optional>
#include <string>

namespace mhfu::ui {
namespace {

namespace sets = game::item_sets;

enum class Page { Closed, List, Set, ConfirmRegister, ConfirmDelete };

// What the screen shows of the running game, read once a frame.
struct Snapshot {
    bool loaded{};
    std::string hunter;
    sets::Pouch pouch{};
    std::map<std::uint16_t, std::string> names;
    std::map<std::uint16_t, std::uint32_t> in_box;
};

struct State {
    Page page{Page::Closed};
    std::size_t chosen{};
    bool focus{};
    std::string message;
    bool message_warning{};
    Snapshot snap;
};

State &state() {
    static State s;
    return s;
}

void go(Page page) {
    State &s = state();
    s.page = page;
    s.focus = true;
}

std::string item_name(std::uint16_t item) {
    const auto found = state().snap.names.find(item);
    return found != state().snap.names.end() && !found->second.empty() ? found->second
                                                                        : "Item " + std::to_string(item);
}

std::size_t item_count(const sets::ItemSet &set) {
    std::size_t count = 0u;
    for (const sets::Slot &slot : set.items)
        if (slot.item != 0u && slot.count != 0u) ++count;
    return count;
}

void read_snapshot(const sets::Sets &all) {
    Snapshot snap;
    game::read([&](const game::Ram &ram) {
        const std::uint32_t sys = sets::game_sys(ram);
        if (sys == 0u) return;
        snap.loaded = true;
        snap.hunter = sets::hunter_name(ram, sys);
        snap.pouch = sets::read_pouch(ram, sys);
        const auto note = [&](std::uint16_t item) {
            if (item == 0u || snap.names.contains(item)) return;
            snap.names[item] = game::text_entry(ram, game::kTextBlock, 1, item);
            snap.in_box[item] = sets::box_count(ram, sys, item);
        };
        for (const sets::Slot &slot : snap.pouch) note(slot.item);
        for (const sets::ItemSet &set : all)
            for (const sets::Slot &slot : set.items) note(slot.item);
    });
    if (snap.hunter.empty()) snap.hunter = "Hunter";
    state().snap = std::move(snap);
}

void say(std::string text, bool warning = false) {
    State &s = state();
    s.message = std::move(text);
    s.message_warning = warning;
}

void register_pouch(const sets::ItemSet &current) {
    State &s = state();
    sets::ItemSet set{current.name, s.snap.pouch};
    sets::store().set(s.snap.hunter, s.chosen, set);
    if (sets::store().save())
        say("The pouch is registered as " + set.name + ".");
    else
        say("Could not write " + sets::store().path().string() + ".", true);
}

void delete_set() {
    State &s = state();
    sets::store().set(s.snap.hunter, s.chosen, sets::ItemSet{sets::default_name(s.chosen), {}});
    if (sets::store().save())
        say("The set is deleted.");
    else
        say("Could not write " + sets::store().path().string() + ".", true);
}

void load_set(const sets::ItemSet &set) {
    std::optional<sets::Loaded> loaded;
    game::write([&](game::Ram &ram) {
        const std::uint32_t sys = sets::game_sys(ram);
        if (sys != 0u) loaded = sets::load(ram, sys, set);
    });
    if (!loaded || (!loaded->done && !loaded->box_full)) {
        say("The item box cannot be read.", true);
        return;
    }
    if (loaded->box_full) {
        say("The Item Box is full: the pouch does not fit in it. Nothing was moved.", true);
        return;
    }
    if (loaded->short_items.empty()) {
        say(set.name + " is loaded.");
        return;
    }
    std::string text = set.name + " is loaded, but the box was short of ";
    for (std::size_t i = 0; i < loaded->short_items.size(); ++i) {
        const sets::Short &item = loaded->short_items[i];
        if (i > 0u) text += ", ";
        text += item_name(item.item) + " (" + std::to_string(item.taken) + " of " + std::to_string(item.wanted) + ")";
    }
    say(text + ".", true);
}

void rename_set(const sets::ItemSet &current) {
    TextInputRequest request;
    request.title = "Set name";
    request.initial = current.name;
    request.max_length = 20u;
    open_text_input(std::move(request), [hunter = state().snap.hunter, index = state().chosen,
                                            set = current](std::optional<std::string> text) mutable {
        if (!text) return;
        set.name = *text;
        sets::store().set(hunter, index, set);
        if (!sets::store().save()) say("Could not write " + sets::store().path().string() + ".", true);
        go(Page::Set);
    });
}

void focus_once() {
    State &s = state();
    if (s.focus) {
        focus_next_row();
        s.focus = false;
    }
}

void list_page(const sets::Sets &all) {
    State &s = state();
    focus_once();
    for (std::size_t i = 0; i < sets::kSets; ++i) {
        const std::size_t count = item_count(all[i]);
        ImGui::PushID(static_cast<int>(i));
        if (value_row(all[i].name.c_str(), count == 0u ? "Empty" : std::to_string(count) + " items",
                {false, {}, "Load this set, register the pouch in it, rename it or delete it."})) {
            s.chosen = i;
            s.message.clear();
            go(Page::Set);
        }
        ImGui::PopID();
    }
}

void set_page(const sets::ItemSet &set) {
    State &s = state();
    const bool empty = set.empty();
    const bool pouch_empty = sets::ItemSet{{}, s.snap.pouch}.empty();
    focus_once();
    if (button_row("Load",
            {empty, {},
                "Put the pouch in the item box, then take this set's items out of the box into the same pouch "
                "slots."}))
        load_set(set);
    if (button_row("Register Pouch",
            {pouch_empty, {}, "Keep what the pouch holds now as this set, in its slots."})) {
        if (empty)
            register_pouch(set);
        else
            go(Page::ConfirmRegister);
    }
    if (button_row("Rename", {false, {}, "Give this set a name of its own."})) rename_set(set);
    if (button_row("Delete Set", {empty, {}, "Empty this set. The items in the box and the pouch stay."},
            colors::kDanger))
        go(Page::ConfirmDelete);
    if (!s.message.empty()) {
        ImGui::Dummy({0.0f, ImGui::GetFontSize() * 0.4f});
        paragraph(s.message, s.message_warning ? colors::kDanger : colors::kGood);
    }
    section(empty ? "Items: none yet" : "Items");
    for (std::size_t i = 0; i < set.items.size(); ++i) {
        const sets::Slot &slot = set.items[i];
        if (slot.item == 0u || slot.count == 0u) continue;
        const auto box = s.snap.in_box.find(slot.item);
        const std::uint32_t have = box == s.snap.in_box.end() ? 0u : box->second;
        ImGui::PushID(static_cast<int>(i));
        info_row(item_name(slot.item).c_str(),
            "x" + std::to_string(slot.count) + "    " + std::to_string(have) + " in the box");
        ImGui::PopID();
    }
}

void confirm_page(const sets::ItemSet &set, bool registering) {
    focus_once();
    paragraph(registering ? "Replace " + set.name + " with what the pouch holds now?"
                          : "Delete " + set.name + "? The items in the box and the pouch stay.");
    ImGui::Dummy({0.0f, ImGui::GetFontSize() * 0.4f});
    if (button_row(registering ? "Replace" : "Delete", {}, registering ? colors::kText : colors::kDanger)) {
        if (registering)
            register_pouch(set);
        else
            delete_set();
        go(Page::Set);
    }
    if (button_row("Cancel")) go(Page::Set);
}

void close() {
    State &s = state();
    s.page = Page::Closed;
    Layer &layer = Layer::get();
    layer.set_interactive(false);
    layer.renderer().set_game_input(true);
    sets::screen_closed();
    std::cout << "[item sets] screen closed" << std::endl;
}

} // namespace

bool open_item_sets() {
    Layer &layer = Layer::get();
    if (!layer.attached()) return false;
    State &s = state();
    if (s.page != Page::Closed) return true;
    s.message.clear();
    go(Page::List);
    layer.renderer().set_game_input(false);
    layer.set_interactive(true);
    std::cout << "[item sets] screen opened" << std::endl;
    return true;
}

bool item_sets_open() {
    return state().page != Page::Closed;
}

bool item_sets_frame() {
    State &s = state();
    if (s.page == Page::Closed) return false;
    dropdown_new_frame();
    if (text_input_open()) {
        text_input_frame();
        return true;
    }
    Layer &layer = Layer::get();
    const bool esc = layer.take_back();
    const ImGuiKey cancel = layer.confirm_south() ? ImGuiKey_GamepadFaceRight : ImGuiKey_GamepadFaceDown;
    const bool back = esc || ImGui::IsKeyPressed(cancel, false) || layer.window_closed();

    sets::Sets all = sets::store().sets(s.snap.hunter);
    read_snapshot(all);
    all = sets::store().sets(s.snap.hunter);
    if (!s.snap.loaded) {
        close();
        return false;
    }

    begin_panel("##item_sets", "Item Sets", s.snap.hunter, true);
    begin_content();
    const sets::ItemSet &set = all[s.chosen];
    switch (s.page) {
    case Page::List: list_page(all); break;
    case Page::Set: set_page(set); break;
    case Page::ConfirmRegister: confirm_page(set, true); break;
    case Page::ConfirmDelete: confirm_page(set, false); break;
    case Page::Closed: break;
    }
    touch_scroll();
    begin_footer();
    hints({{Control::Confirm, "Select"}, {Control::Back, s.page == Page::List ? "Close" : "Back"}});
    end_panel();

    if (back) {
        if (s.page == Page::List) {
            close();
            return false;
        }
        go(s.page == Page::Set ? Page::List : Page::Set);
    }
    return true;
}

} // namespace mhfu::ui
