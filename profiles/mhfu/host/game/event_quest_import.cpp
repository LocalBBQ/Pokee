#include "game/event_quest_import.hpp"

#include "platform/utf8_path.hpp"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <fstream>
#include <iterator>
#include <map>
#include <optional>
#include <span>
#include <string_view>
#include <system_error>

namespace mhfu::game::event_quests {
namespace {

namespace fs = std::filesystem;

// FUComplete's quest tables: a name, then quests of a fixed size from 0x30
// bytes after its start.
constexpr std::string_view kTableNames[] = {{"EventQstTbl\0", 12u}, {"ArenaQstTbl\0", 12u}};
constexpr std::size_t kTableStart = 0x30u;
constexpr std::size_t kTableStride = 0x22B0u;
// A file larger than this is no quest table; a disc image is not read.
constexpr std::uintmax_t kLargestTable = 16u * 1024u * 1024u;

std::vector<std::uint8_t> read_file(const fs::path &path) {
    std::ifstream in(path, std::ios::binary);
    return {std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
}

std::string lower_extension(const fs::path &path) {
    std::string extension = path_to_utf8(path.extension());
    for (char &c : extension) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return extension;
}

struct Found {
    Quest quest;
    fs::path from;
};

// The quests of the tables in `bytes`.
void read_tables(std::span<const std::uint8_t> bytes, const fs::path &from, std::vector<Found> &found) {
    for (const std::string_view name : kTableNames) {
        const auto at = std::search(bytes.begin(), bytes.end(), name.begin(), name.end());
        if (at == bytes.end()) continue;
        for (auto offset = static_cast<std::size_t>(at - bytes.begin()) + kTableStart;
             offset <= bytes.size() && bytes.size() - offset >= kTableStride; offset += kTableStride) {
            std::optional<Quest> quest = parse(bytes.subspan(offset, kTableStride));
            if (!quest) break;
            found.push_back({std::move(*quest), from});
        }
    }
}

// The quests in the files of `folder`, sorted by name; counts the .mib files
// that are not quests.
void read_folder(const fs::path &folder, std::vector<Found> &found, std::size_t &unreadable) {
    std::error_code ec;
    std::vector<fs::path> files;
    for (const auto &entry : fs::directory_iterator(folder, ec))
        if (entry.is_regular_file(ec)) files.push_back(entry.path());
    std::sort(files.begin(), files.end());
    for (const fs::path &file : files) {
        const std::uintmax_t size = fs::file_size(file, ec);
        if (ec || size == 0u || size > kLargestTable) continue;
        const std::vector<std::uint8_t> bytes = read_file(file);
        if (std::optional<Quest> quest = parse(bytes)) {
            found.push_back({std::move(*quest), file});
            continue;
        }
        const std::size_t before = found.size();
        if (bytes.size() > kQuestBytes) read_tables(bytes, file, found);
        if (found.size() == before && lower_extension(file) == ".mib") ++unreadable;
    }
}

bool japan_only(std::uint16_t id) {
    return std::find(kJapanOnly.begin(), kJapanOnly.end(), id) != kJapanOnly.end();
}

fs::path quest_path(const fs::path &quest_folder, std::uint16_t id) {
    char name[16];
    std::snprintf(name, sizeof name, "m%05u.mib", static_cast<unsigned>(id));
    return quest_folder / name;
}

} // namespace

ImportCheck check_import(const fs::path &chosen, const fs::path &quest_folder) {
    ImportCheck check;
    check.chosen = chosen;
    std::vector<Found> found;
    read_folder(chosen, found, check.unreadable);
    std::error_code ec;
    std::vector<fs::path> inner;
    for (const auto &entry : fs::directory_iterator(chosen, ec))
        if (entry.is_directory(ec)) inner.push_back(entry.path());
    std::sort(inner.begin(), inner.end());
    for (const fs::path &folder : inner) read_folder(folder, found, check.unreadable);

    std::map<std::uint16_t, ImportCandidate> chosen_quests; // the first of each id
    for (Found &f : found) {
        const std::uint16_t id = f.quest.id;
        const bool hidden = (f.quest.flags & kHiddenFlag) != 0u;
        const bool event = id >= kFirstEventId && id < kFirstChallengeId && !hidden;
        const bool challenge = id >= kFirstChallengeId && id < kLastChallengeId && hidden;
        if (!event && !challenge) {
            ++check.other;
        } else if (japan_only(id)) {
            ++check.japan_only;
        } else if (!chosen_quests.contains(id)) {
            ImportCandidate candidate;
            candidate.challenge = challenge;
            candidate.title = title(f.quest.bytes);
            candidate.from = std::move(f.from);
            candidate.quest = std::move(f.quest);
            chosen_quests.emplace(id, std::move(candidate));
        }
    }
    for (auto &[id, candidate] : chosen_quests) {
        const fs::path installed = quest_path(quest_folder, id);
        if (fs::exists(installed, ec)) {
            if (read_file(installed) == candidate.quest.bytes) {
                ++check.present;
                continue;
            }
            candidate.replaces = true;
        }
        check.quests.push_back(std::move(candidate));
    }
    if (check.quests.empty()) {
        if (check.present > 0u)
            check.problem = "The event quests here are imported already.";
        else if (found.empty())
            check.problem = "No quest files here, or in the folders directly inside it.";
        else
            check.problem = "No event quests here that can be imported.";
    }
    return check;
}

ImportResult import_quests(const ImportCheck &check, const fs::path &quest_folder) {
    ImportResult result;
    std::error_code ec;
    fs::create_directories(quest_folder, ec);
    if (ec) {
        result.error = "Cannot make " + path_to_utf8(quest_folder) + ": " + ec.message();
        return result;
    }
    for (const ImportCandidate &candidate : check.quests) {
        const fs::path target = quest_path(quest_folder, candidate.quest.id);
        fs::path temporary = target;
        temporary += ".tmp";
        bool written = false;
        {
            std::ofstream out(temporary, std::ios::binary | std::ios::trunc);
            out.write(reinterpret_cast<const char *>(candidate.quest.bytes.data()),
                static_cast<std::streamsize>(candidate.quest.bytes.size()));
            written = static_cast<bool>(out.flush());
        }
        if (written) fs::rename(temporary, target, ec);
        if (!written || ec) {
            fs::remove(temporary, ec);
            if (result.error.empty()) result.error = "Cannot write " + path_to_utf8(target);
            continue;
        }
        result.imported.push_back(candidate.quest.id);
    }
    return result;
}

} // namespace mhfu::game::event_quests
