// The Layered armor page. It changes only the port's settings
// (look.layered_* in settings.ini); game/layered_armor_hook.cpp reads them at
// the next flip. What it shows of the game (the hunter, the pieces worn and
// owned, their names) is read from guest memory between frames and never
// written.

#include "ui/layered_armor_screen.hpp"

#include "game/equipment_models.hpp"
#include "game/game_data.hpp"
#include "game/guest_ram.hpp"
#include "game/layered_armor.hpp"
#include "settings/settings.hpp"
#include "ui/widgets.hpp"

#include <array>
#include <string>
#include <vector>

namespace mhfu::ui {
namespace {

namespace layered = game::layered;

enum class Screen { Closed, Page, Pieces };

// The parts in the order a hunter is looked at, head to feet.
constexpr std::array<std::uint8_t, layered::kParts> kShownParts = {4u, 0u, 1u, 2u, 3u};

// What the page shows of the running game, read once a frame.
struct Snapshot {
    bool loaded{};
    std::string sex;
    std::array<std::string, layered::kParts> worn;   // names of the pieces worn
    std::array<std::string, layered::kParts> chosen; // names of the choices
    std::array<std::uint32_t, layered::kParts> owned{}; // worn plus box copies
    std::vector<layered::Offer> offers;                 // the list screen's part
};

struct State {
    Screen screen{Screen::Closed};
    bool focus{};
    std::uint8_t part{};
    Snapshot snap;
};

State &state() {
    static State s;
    return s;
}

void focus_once() {
    State &s = state();
    if (s.focus) {
        focus_next_row();
        s.focus = false;
    }
}

void read_snapshot() {
    State &s = state();
    const settings::Settings &settings = settings::current();
    s.snap = {};
    const bool read = game::read([&](const game::Ram &ram) {
        Snapshot &n = s.snap;
        // The loaded hunter's record. A sex or worn piece read from anywhere
        // else would not change when this hunter buys or sells armor.
        const std::optional<std::uint8_t> played = layered::played_sex(ram);
        n.loaded = played.has_value();
        if (played) n.sex = game::sex_name(*played == 0u ? game::Sex::Male : game::Sex::Female);
        for (std::uint8_t part = 0; part < layered::kParts; ++part) {
            n.chosen[part] = layered::choice_name(&ram, part, settings.layered_pieces[part]);
            if (!n.loaded) continue;
            const std::optional<std::uint16_t> id = layered::played_armor(ram, part);
            if (!id || *id == 0u) {
                n.worn[part] = "nothing";
            } else {
                const std::string name = game::equipment_name(ram, part, *id);
                n.worn[part] = name.empty() ? "Piece " + std::to_string(*id) : name;
            }
            n.owned[part] = layered::owned_count(ram, part);
        }
        if (s.screen == Screen::Pieces) n.offers = layered::offers(ram, s.part);
    });
    if (!read)
        for (std::uint8_t part = 0; part < layered::kParts; ++part)
            s.snap.chosen[part] = layered::choice_name(nullptr, part, settings.layered_pieces[part]);
}

void choose(std::uint8_t part, std::int32_t choice) {
    settings::Settings &settings = settings::current();
    settings.layered_pieces[part] = choice;
    settings::save();
}

bool any_chosen(const settings::Settings &settings) {
    for (const std::int32_t choice : settings.layered_pieces)
        if (choice != layered::kReal) return true;
    return false;
}

void page(bool back) {
    State &s = state();
    if (back) {
        s.screen = Screen::Closed;
        s.focus = true;
        return;
    }
    settings::Settings &settings = settings::current();
    const Snapshot &n = s.snap;
    section("Layered armor");
    focus_once();
    if (toggle_row("Layered armor", settings.layered_armor,
            {false, {},
                "Draws the hunter in the pieces chosen below instead of the armor it wears. Defense, "
                "resistances and skills stay those of the real armor, and the save keeps the real armor. "
                "Off, the game is left exactly as it is."})) {
        settings.layered_armor = !settings.layered_armor;
        settings::save();
    }
    paragraph("A change shows on the next area change or when a quest loads. Changing equipment keeps this "
              "look. Turning it off leaves the hunter already on screen as it is; embarking after that loads "
              "the real armor.",
        colors::kTextDim);

    section("Look");
    if (!n.loaded) paragraph("Load a character to choose from its armor and see the names.", colors::kTextDim);
    for (const std::uint8_t part : kShownParts) {
        RowOptions o;
        o.disabled = !n.loaded;
        if (n.loaded && !n.worn[part].empty())
            o.note = "Wearing " + n.worn[part] + ", " + std::to_string(n.owned[part]) + " owned";
        o.description = std::string("What the hunter's ") + layered::part_label(part) +
            " looks like. Real equipment shows the piece worn; Nothing shows the bare part, or the "
            "inner wear where the game shows it. Only a piece this hunter wears or keeps in the equipment box "
            "can be chosen.";
        const std::string label = std::string(layered::part_label(part)) + "##layered" + std::to_string(part);
        if (value_row(label.c_str(), n.chosen[part], o)) {
            s.part = part;
            s.screen = Screen::Pieces;
            s.focus = true;
        }
    }
    if (button_row("Show the real equipment everywhere",
            {!any_chosen(settings), {}, "Sets every part back to Real equipment."})) {
        settings.layered_pieces.fill(layered::kReal);
        settings::save();
    }

    section("Other players");
    paragraph("Other players see your real armor. The game sends them the equipment you wear, and layered armor "
              "only changes what this game draws for your own hunter; other hunters are drawn as they are.",
        colors::kTextDim);
    paragraph("The choices are kept in the port's settings, not in the game's save, so a save stays the same "
              "for the PSP and other emulators. A piece the loaded hunter's sex cannot wear shows the real one.",
        colors::kTextDim);
}

void pieces(bool back) {
    State &s = state();
    if (back) {
        s.screen = Screen::Page;
        s.focus = true;
        return;
    }
    const settings::Settings &settings = settings::current();
    const Snapshot &n = s.snap;
    const std::int32_t current = settings.layered_pieces[s.part];
    section((std::string(layered::part_label(s.part)) + ": what it looks like").c_str());
    focus_once();
    if (list_row("##real", "Real equipment", n.worn[s.part].empty() ? std::string() : n.worn[s.part], ListIcon::None,
            current == layered::kReal)) {
        choose(s.part, layered::kReal);
        s.screen = Screen::Page;
        s.focus = true;
        return;
    }
    if (list_row("##nothing", "Nothing", "bare, or inner wear", ListIcon::None, current == 0)) {
        choose(s.part, 0);
        s.screen = Screen::Page;
        s.focus = true;
        return;
    }
    std::uint32_t total = 0u;
    for (const layered::Offer &offer : n.offers) total += offer.copies;
    const std::string owned = "Armor you own (" + std::to_string(total) + ")";
    section(owned.c_str());
    for (const layered::Offer &offer : n.offers) {
        const std::string id = "##piece" + std::to_string(offer.id);
        std::string detail = offer.worn ? "worn" : "owned";
        if (offer.copies > 1u) detail += " x" + std::to_string(offer.copies);
        if (list_row(id.c_str(), offer.name, detail, ListIcon::None, current == offer.id)) {
            choose(s.part, offer.id);
            s.screen = Screen::Page;
            s.focus = true;
            return;
        }
    }
    if (n.offers.empty())
        paragraph(n.loaded ? "No piece of this part is worn or kept in the equipment box."
                           : "Load a character first.",
            colors::kTextDim);
}

} // namespace

void layered_armor_row() {
    const settings::Settings &settings = settings::current();
    section("Look");
    if (value_row("Layered armor", settings.layered_armor ? "On" : "Off",
            {false, {},
                "Draw the hunter in other armor than it wears, part by part. The stats and skills stay those of "
                "the real armor."})) {
        state().screen = Screen::Page;
        state().focus = true;
    }
}

bool layered_armor_screen(bool back) {
    State &s = state();
    if (s.screen == Screen::Closed) return false;
    read_snapshot();
    if (s.screen == Screen::Pieces)
        pieces(back);
    else
        page(back);
    return true;
}

bool layered_armor_screen_open() {
    return state().screen != Screen::Closed;
}

} // namespace mhfu::ui
