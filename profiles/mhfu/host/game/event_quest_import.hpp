#pragma once

#include "game/event_quests.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

// Importing event quests the player has as files: the event quests found in a
// chosen folder are copied into event_quests/ of the data directory as
// m<id>.mib, the name load() reads (game/event_quests.hpp). A file is a quest
// when it parses as one, whatever its name; FUComplete's tables of quests
// (EventQstTbl, ArenaQstTbl), taken from its DATA.BIN, are read as the quests
// they hold. Needs no game.
namespace mhfu::game::event_quests {

// Event quests only Japan's Portable 2nd G had. Not imported.
inline constexpr std::array<std::uint16_t, 13> kJapanOnly = {
    60001, 60002, 60003, 60004, 60005, 60006, 60007, 60008, 60009, 60018, 60020, 60036, 60038};
// Event quest ids; Training School challenges follow at 61000.
inline constexpr std::uint16_t kFirstEventId = 60000u;
inline constexpr std::uint16_t kFirstChallengeId = 61000u;

struct ImportCandidate {
    Quest quest;
    std::string title;
    std::filesystem::path from;
    bool replaces{}; // a different quest of that id is in event_quests/ already
};

struct ImportCheck {
    std::filesystem::path chosen;
    std::vector<ImportCandidate> quests; // by id
    std::size_t present{};               // the same quest is in event_quests/ already
    std::size_t japan_only{};
    std::size_t challenges{}; // Training School challenges, which the event list does not show
    std::size_t other{};      // quests that are not event quests
    std::size_t unreadable{}; // .mib files that are not decoded quests, such as a PSP's encrypted ones
    std::string problem;      // why nothing can be imported; empty when something can
};

// The quests in `chosen` and the folders directly inside it, sorted into
// those to import and those left out, against what `quest_folder` holds.
[[nodiscard]] ImportCheck check_import(const std::filesystem::path &chosen, const std::filesystem::path &quest_folder);

struct ImportResult {
    std::vector<std::uint16_t> imported;
    std::string error; // the first failure; the rest were still tried
};

// Writes the candidates into `quest_folder`, replacing a quest of the same id.
[[nodiscard]] ImportResult import_quests(const ImportCheck &check, const std::filesystem::path &quest_folder);

} // namespace mhfu::game::event_quests
