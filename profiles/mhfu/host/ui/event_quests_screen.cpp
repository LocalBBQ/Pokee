// The Event quests page. It changes the port's setting (game.event_quests)
// and the files in event_quests/ of the data directory, which
// game/event_quests_hook.cpp reads the first time the Guild Hall loads.

#include "ui/event_quests_screen.hpp"

#include "game/event_quest_import.hpp"
#include "game/event_quests.hpp"
#include "install/user_data.hpp"
#include "settings/settings.hpp"
#include "ui/file_browser.hpp"
#include "ui/layer.hpp"
#include "ui/widgets.hpp"

#include "imgui.h"

#include <cmath>
#include <iostream>
#include <memory>
#include <string>
#include <system_error>
#include <vector>

namespace mhfu::ui {
namespace {

namespace fs = std::filesystem;
namespace eq = game::event_quests;

enum class Screen { Closed, Page, Choose, Review, Result };

struct Listed {
    std::uint16_t id{};
    std::string title;
    bool challenge{};
};

struct State {
    Screen screen{Screen::Closed};
    bool focus{};
    std::vector<Listed> listed; // what event_quests/ holds, read when the page opens
    std::unique_ptr<FileBrowser> browser;
    fs::path last_folder;
    eq::ImportCheck check;
    eq::ImportResult result;
};

State &state() {
    static State s;
    return s;
}

fs::path quest_folder() {
    return install::user_data_directory() / "event_quests";
}

std::string count(std::size_t n, const char *one, const char *many) {
    return std::to_string(n) + " " + (n == 1u ? one : many);
}

void go(Screen screen) {
    State &s = state();
    s.screen = screen;
    s.focus = true;
    ImGui::SetScrollY(0.0f);
}

void focus_once() {
    State &s = state();
    if (s.focus) {
        focus_next_row();
        s.focus = false;
    }
}

void open_page() {
    State &s = state();
    s.listed.clear();
    for (const eq::Quest &quest : eq::load(quest_folder()))
        s.listed.push_back({quest.id, eq::title(quest.bytes), (quest.flags & eq::kHiddenFlag) != 0u});
    s.browser.reset();
    go(Screen::Page);
}

const char *applies_note() {
    return eq::loaded() ? "The folder has been read this run: quests imported now are listed from the next start."
                        : "Quests imported now are listed when the Guild Hall or the Training School loads.";
}

void open_browser() {
    State &s = state();
    FileBrowser::Options options;
    options.extensions = {};
    options.filter_name = "folders";
    options.listed_name = "folders";
    options.empty_note = "No folders here.";
    options.choose_folder = "Import from this folder";
    fs::path start = s.last_folder;
    if (start.empty()) {
        start = FileBrowser::home() / "Downloads";
        std::error_code ec;
        if (!fs::is_directory(start, ec)) start = FileBrowser::home();
    }
    s.browser = std::make_unique<FileBrowser>(start, std::move(options));
    go(Screen::Choose);
}

void check_folder(const fs::path &chosen) {
    State &s = state();
    s.check = eq::check_import(chosen, quest_folder());
    std::cout << "[event quests] import from " << install::path_to_utf8(chosen) << ": "
              << (s.check.quests.empty() ? s.check.problem : count(s.check.quests.size(), "quest", "quests") + " found")
              << std::endl;
    go(Screen::Review);
}

void page(bool back) {
    State &s = state();
    if (back) {
        s.screen = Screen::Closed;
        return;
    }
    settings::Settings &settings = settings::current();
    section("Event quests");
    focus_once();
    RowOptions o;
    o.description = "On (default): the Guild Hall's event quests and the Training School's challenges list the "
                    "quests below after the ones in your save, and they start like the save's own. The save is not "
                    "changed. Off: only the save's.";
    if (const char *variable = settings::overridden_by("game.event_quests")) {
        o.disabled = true;
        o.note = std::string("Set by ") + variable;
    }
    if (toggle_row("Event quests", settings.event_quests, o)) {
        settings.event_quests = !settings.event_quests;
        settings::save();
    }
    if (button_row("Import from a folder…",
            {false, {},
                "Choose a folder of quest files, such as FUComplete's event quests and challenges. Japan-only "
                "quests are left out."}))
        open_browser();
    paragraph(applies_note(), colors::kTextDim);

    for (const bool challenges : {false, true}) {
        std::size_t shown = 0u;
        for (const Listed &quest : s.listed) shown += quest.challenge == challenges ? 1u : 0u;
        section(
            ((challenges ? "Training School challenges (" : "Event quests (") + std::to_string(shown) + ")").c_str());
        if (shown == 0u) paragraph(challenges ? "No challenges yet." : "No event quests yet.", colors::kTextDim);
        for (const Listed &quest : s.listed) {
            if (quest.challenge != challenges) continue;
            const std::string label =
                (quest.title.empty() ? std::string("Quest") : quest.title) + "##quest" + std::to_string(quest.id);
            info_row(label.c_str(), std::to_string(quest.id));
        }
    }
    paragraph("The quests are kept in " + install::path_to_utf8(quest_folder()) +
            ", as m<id>.mib. A quest of an id the save holds too is the save's.",
        colors::kTextDim);
}

void browse(bool back) {
    State &s = state();
#if defined(MHFU_ANDROID_APP)
    paragraph("Open the folder that holds the quest files.");
    paragraph("On Android this lists only folders Pokee can read by itself, which leaves out Downloads and SD "
              "cards.",
        colors::kTextDim);
#else
    paragraph("Open the folder that holds the quest files. You can also drop the folder on the window.");
#endif
    if (auto dropped = Layer::get().take_dropped_file()) {
        s.last_folder = s.browser->folder();
        s.browser.reset();
        std::error_code ec;
        check_folder(fs::is_directory(*dropped, ec) ? *dropped : dropped->parent_path());
        return;
    }
    const FileBrowser::Result result = s.browser->frame(back);
    if (result == FileBrowser::Result::Browsing) return;
    s.last_folder = s.browser->folder();
    const fs::path chosen = s.browser->chosen();
    s.browser.reset();
    if (result == FileBrowser::Result::Cancelled) {
        open_page();
        return;
    }
    check_folder(chosen);
}

void review(bool back) {
    State &s = state();
    if (back) {
        open_browser();
        return;
    }
    const eq::ImportCheck &c = s.check;
    section("Import event quests");
    paragraph("From " + install::path_to_utf8(c.chosen), colors::kTextDim);
    if (c.quests.empty()) paragraph(c.problem, colors::kDanger);
    for (const eq::ImportCandidate &candidate : c.quests) {
        const std::string label = (candidate.title.empty() ? std::string("Quest") : candidate.title) + "##import" +
            std::to_string(candidate.quest.id);
        info_row(label.c_str(),
            std::to_string(candidate.quest.id) + (candidate.challenge ? ", Training School" : "") +
                (candidate.replaces ? ", replaces the one in the folder" : ""));
    }
    std::vector<std::string> left_out;
    if (c.present > 0u) left_out.push_back(count(c.present, "quest imported already", "quests imported already"));
    if (c.japan_only > 0u) left_out.push_back(count(c.japan_only, "Japan-only quest", "Japan-only quests"));
    if (c.other > 0u) left_out.push_back(count(c.other, "other quest", "other quests"));
    if (c.unreadable > 0u)
        left_out.push_back(count(c.unreadable, ".mib file that is not a decoded quest (a PSP's are encrypted)",
            ".mib files that are not decoded quests (a PSP's are encrypted)"));
    if (!left_out.empty()) {
        std::string text = "Left out: ";
        for (std::size_t i = 0; i < left_out.size(); ++i) text += (i == 0u ? "" : ", ") + left_out[i];
        paragraph(text + ".", colors::kTextDim);
    }
    ImGui::Dummy({0.0f, std::round(12.0f * Layer::get().scale())});
    focus_once();
    if (!c.quests.empty() &&
        button_row(c.quests.size() == 1u ? "Import this quest" : "Import these quests",
            {false, {}, "Copies them into the event quests folder."})) {
        s.result = eq::import_quests(c, quest_folder());
        std::cout << "[event quests] imported " << s.result.imported.size() << " quest(s)"
                  << (s.result.error.empty() ? "" : "; " + s.result.error) << std::endl;
        go(Screen::Result);
        return;
    }
    if (button_row("Choose another folder", {false, {}, "Back to the folders."})) open_browser();
    if (button_row("Cancel", {false, {}, "Import nothing."})) open_page();
}

void result_screen(bool back) {
    State &s = state();
    if (back) {
        open_page();
        return;
    }
    section("Import event quests");
    info_row("Imported", count(s.result.imported.size(), "quest", "quests"));
    if (!s.result.error.empty()) info_row("Problem", s.result.error);
    paragraph(applies_note(), colors::kTextDim);
    ImGui::Dummy({0.0f, std::round(12.0f * Layer::get().scale())});
    focus_once();
    if (button_row("Done", {false, {}, "Back to the event quests."})) open_page();
}

} // namespace

void event_quests_row() {
    const settings::Settings &settings = settings::current();
    if (value_row("Event quests", settings.event_quests ? "On" : "Off",
            {false, {},
                "Event quests and Training School challenges from files, listed after the ones in your save, "
                "and importing them."}))
        open_page();
}

bool event_quests_screen(bool back) {
    switch (state().screen) {
    case Screen::Closed:
        return false;
    case Screen::Page:
        page(back);
        break;
    case Screen::Choose:
        browse(back);
        break;
    case Screen::Review:
        review(back);
        break;
    case Screen::Result:
        result_screen(back);
        break;
    }
    return true;
}

bool event_quests_screen_open() {
    return state().screen != Screen::Closed;
}

} // namespace mhfu::ui
