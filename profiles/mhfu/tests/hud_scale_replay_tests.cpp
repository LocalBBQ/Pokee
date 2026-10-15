// Interface sizes replayed over traced screens (gpu/hud_scale.hpp).
//
// Each fixture in tests/hud_scale_fixtures/ is the [sprite] lines of one
// MHFU_TRACE_CAPTURE run: every through-mode sprite of the frames after a
// screenshot, with the code that filed it (owner=) and whether it took the
// Names size (name=1). The replay draws them through the same calls the
// renderer makes, frame by frame, at sizes that move every piece, and writes
// one line per quad: the piece it was sized as, the rule that decided it, and
// where it went. That must match the fixture's .expected file, so a rule added
// for one screen that moves a quad on another fails here, with the line.
//
// A changed .expected file is the review: each line that changed is a quad
// that is now drawn somewhere else. MHFU_UPDATE_HUD_SCALE_FIXTURES=1 rewrites
// them after a change that is meant. docs/TESTING.md says how to add a screen.
//
// The trace has no clock hands or turned map arrow (those are strips, not
// sprites), and nothing was aiming; the replay leaves both out.

#include "gpu/hud_scale.hpp"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace {

namespace hs = mhfu::gpu::hud_scale;
namespace hud = mhfu::gpu::hud;

int failures = 0;

void check(bool condition, const std::string &message) {
    if (condition) return;
    std::cerr << "FAIL: " << message << "\n";
    ++failures;
}

struct Vertex {
    float x{};
    float y{};
    float u{};
    float v{};
};

// One traced sprite.
struct Sprite {
    unsigned long long frame{};
    std::string at;
    float x0{}, y0{}, x1{}, y1{};
    float u0{}, v0{}, u1{}, v1{};
    std::uint32_t texture{};
    hud::Owner owner{hud::Owner::Unknown};
    hud::NameDraw name;
};

// The value after `key` up to the next space, or empty.
std::string field(const std::string &line, const std::string &key) {
    const std::size_t at = line.find(key);
    if (at == std::string::npos) return {};
    const std::size_t start = at + key.size();
    const std::size_t end = line.find(' ', start);
    return line.substr(start, end == std::string::npos ? std::string::npos : end - start);
}

// "(a,b)-(c,d)" after `key`.
bool corners(const std::string &line, const std::string &key, float &a, float &b, float &c, float &d) {
    const std::size_t at = line.find(key);
    if (at == std::string::npos) return false;
    return std::sscanf(line.c_str() + at + key.size(), "(%f,%f)-(%f,%f)", &a, &b, &c, &d) == 4;
}

bool parse(const std::string &line, Sprite &sprite) {
    if (line.rfind("[sprite] ", 0) != 0 || line.find(" owner=") == std::string::npos) return false;
    sprite.frame = std::strtoull(field(line, " f=").c_str(), nullptr, 10);
    sprite.at = field(line, " at=");
    if (!corners(line, " pos=", sprite.x0, sprite.y0, sprite.x1, sprite.y1)) return false;
    corners(line, " uv=", sprite.u0, sprite.v0, sprite.u1, sprite.v1);
    // "tex=1 0x419c200 256x256": the address only counts when texturing is on.
    const std::size_t tex = line.find(" tex=");
    if (tex != std::string::npos && line.compare(tex + 5, 1, "1") == 0)
        sprite.texture = static_cast<std::uint32_t>(std::strtoul(line.c_str() + tex + 7, nullptr, 16));
    const std::string owner = field(line, " owner=");
    sprite.owner = owner == "quest" ? hud::Owner::QuestHud : owner == "other" ? hud::Owner::Other : hud::Owner::Unknown;
    if (field(line, " name=") == "1") {
        sprite.name.is_name = true;
        sprite.name.glyph = line.find(" name=1 letter") != std::string::npos;
        const std::size_t origin = line.find(" origin=");
        sprite.name.has_origin = origin != std::string::npos &&
            std::sscanf(line.c_str() + origin + 8, "(%f,%f)", &sprite.name.origin_x, &sprite.name.origin_y) == 2;
    }
    return true;
}

// The renderer's expansion of a sprite: a, top-right, b, a, b, bottom-left.
void expand(const Sprite &s, std::vector<Vertex> &out) {
    const Vertex a{s.x0, s.y0, s.u0, s.v0};
    const Vertex b{s.x1, s.y1, s.u1, s.v1};
    const Vertex top_right{s.x1, s.y0, s.u1, s.v0};
    const Vertex bottom_left{s.x0, s.y1, s.u0, s.v1};
    out.insert(out.end(), {a, top_right, b, a, b, bottom_left});
}

std::string rect_text(hs::Rect r) {
    std::ostringstream text;
    text << "(" << r.x0 << "," << r.y0 << ")-(" << r.x1 << "," << r.y1 << ")";
    return text.str();
}

const char *owner_text(hud::Owner owner) {
    return owner == hud::Owner::QuestHud ? "quest" : owner == hud::Owner::Other ? "other" : "unknown";
}

// Every piece off the game's size, each a different way, so a quad taken
// for the wrong piece lands somewhere else. The diagnostic mode in main()
// replays at the sizes it is given instead.
hs::Scales kScales = hs::Scales::from_percent(150u, 75u, 125u, 60u);

struct Replay {
    std::vector<std::string> lines;
    int other_moved{};
};

// One draw is the consecutive sprites of one PRIM command in one frame.
void draw(Replay &replay, hs::Frame &frame, const std::vector<Sprite> &sprites) {
    std::vector<Vertex> drawn;
    for (const Sprite &s : sprites) expand(s, drawn);
    std::vector<Vertex> scaled = drawn;
    const Sprite &head = sprites.front();
    std::vector<hs::Decision> decisions;
    if (head.name.is_name) {
        hs::scale_name_draw(scaled, kScales, frame, head.name, hs::InterfaceFit{});
    } else {
        hs::scale_quads(scaled, kScales, frame, head.texture, head.owner, &decisions);
    }
    for (std::size_t first = 0; first + 6u <= drawn.size(); first += 6u) {
        const bool moved = hs::quad_moved(drawn, scaled, first);
        if (moved && head.owner == hud::Owner::Other) ++replay.other_moved;
        std::ostringstream line;
        line << "f=" << head.frame << " at=" << head.at << " " << owner_text(head.owner) << " tex=0x" << std::hex
             << head.texture << std::dec << " " << rect_text(hs::quad_bounds(drawn, first)) << " -> ";
        if (head.name.is_name) {
            line << "name";
        } else {
            const hs::Decision d = first / 6u < decisions.size() ? decisions[first / 6u] : hs::Decision{};
            line << hs::piece_name(d.piece) << " " << hs::reason_name(d.reason);
            if (d.piece == hs::Piece::None && d.candidate != hs::Piece::None)
                line << " was " << hs::piece_name(d.candidate);
        }
        if (moved) line << " " << rect_text(hs::quad_bounds(scaled, first));
        replay.lines.push_back(line.str());
    }
}

Replay replay_file(const std::filesystem::path &path) {
    Replay replay;
    std::ifstream in(path);
    hs::Frame frame;
    std::vector<Sprite> pending;
    unsigned long long current = 0;
    bool started = false;
    auto flush = [&] {
        if (!pending.empty()) draw(replay, frame, pending);
        pending.clear();
    };
    for (std::string line; std::getline(in, line);) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        Sprite sprite;
        if (!parse(line, sprite)) continue;
        if (!pending.empty() && (sprite.frame != pending.front().frame || sprite.at != pending.front().at)) flush();
        if (!started || sprite.frame != current) {
            // The renderer starts a frame's state at each flip.
            if (started) frame.begin();
            started = true;
            current = sprite.frame;
        }
        pending.push_back(sprite);
    }
    flush();
    return replay;
}

std::vector<std::string> read_lines(const std::filesystem::path &path) {
    std::vector<std::string> lines;
    std::ifstream in(path);
    for (std::string line; std::getline(in, line);) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (!line.empty()) lines.push_back(line);
    }
    return lines;
}

void test_fixture(const std::filesystem::path &trace, bool update) {
    const std::string name = trace.stem().string();
    const Replay replay = replay_file(trace);
    check(!replay.lines.empty(), name + ": the fixture has sprites with an owner");
    check(replay.other_moved == 0, name + ": no draw filed by other code moves");
    std::filesystem::path expected_path = trace;
    expected_path.replace_extension(".expected");
    if (update) {
        std::ofstream out(expected_path, std::ios::binary);
        for (const std::string &line : replay.lines) out << line << "\n";
        std::cout << "wrote " << expected_path.filename().string() << " (" << replay.lines.size() << " quads)\n";
        return;
    }
    const std::vector<std::string> expected = read_lines(expected_path);
    check(!expected.empty(),
        name + ": " + expected_path.filename().string() + " exists (MHFU_UPDATE_HUD_SCALE_FIXTURES=1 writes it)");
    if (expected.empty()) return;
    int shown = 0;
    const std::size_t count = std::max(expected.size(), replay.lines.size());
    for (std::size_t i = 0; i < count; ++i) {
        const std::string want = i < expected.size() ? expected[i] : "(nothing)";
        const std::string got = i < replay.lines.size() ? replay.lines[i] : "(nothing)";
        if (want == got) continue;
        if (shown++ < 8)
            check(false, name + " quad " + std::to_string(i) + "\n  expected: " + want + "\n  replayed: " + got);
    }
    if (shown > 8) check(false, name + ": " + std::to_string(shown - 8) + " more quads differ");
}

// The format itself, without a fixture: a quest draw on the map moves, the
// same rectangle from other code does not, and a name draw is a name.
void test_parse_and_owner() {
    Sprite map;
    check(parse("[sprite] f=10 at=0x1 ret=0x0 vtx=0x2 fb=0x4000000 z=0 pos=(316,32)-(468,184) uv=(0,0)-(152,152) "
                "tex=1 0x419c200 256x256 fmt=5 filter=0/0 wrap=0/0 blend=1 color=0xffffffff "
                "scissor=(0,0)-(480,272) owner=quest name=0",
              map),
        "a traced sprite parses");
    check(map.frame == 10u && map.texture == hs::kAreaMapTexture && map.owner == hud::Owner::QuestHud &&
            map.x1 == 468.0f && map.v1 == 152.0f && !map.name.is_name,
        "its frame, texture, owner, rectangle and texels are read");
    Sprite name;
    check(parse("[sprite] f=10 at=0x3 ret=0x0 vtx=0x2 fb=0x4000000 z=0 pos=(100,50)-(108,64) uv=(0,0)-(0,0) "
                "tex=1 0x40cc000 256x256 owner=quest name=1 letter origin=(98,49)",
              name),
        "a name sprite parses");
    check(name.name.is_name && name.name.glyph && name.name.has_origin && name.name.origin_x == 98.0f &&
            name.name.origin_y == 49.0f,
        "a name's letter and the point the game placed it are read");
    Sprite none;
    check(!parse("[sprite] f=10 at=0x1 other prim=4 transform n=3 pos=(0,0)-(480,272)", none),
        "a bounds line without an owner is not a sprite");

    Replay quest;
    hs::Frame frame;
    draw(quest, frame, {map});
    check(quest.lines.size() == 1u && quest.lines[0].find("map shape (") != std::string::npos,
        "the quest HUD's map tile is sized as the map");
    Sprite menu = map;
    menu.owner = hud::Owner::Other;
    menu.texture = 0x09165BA0u;
    Replay other;
    hs::Frame fresh;
    draw(other, fresh, {menu});
    check(other.other_moved == 0 && other.lines[0].find("none not-owner was map") != std::string::npos,
        "the same rectangle filed by other code stays, and says what it would have been");
}

} // namespace

int main(int argc, char **argv) {
    // mhfu_hud_scale_replay_tests <log or trace> [map health items names]:
    // prints each quad of that capture as replayed at those sizes (in
    // percent, the player's own by default here 100), to find the rule that
    // moved a quad on a screen before it becomes a fixture.
    if (argc >= 2) {
        std::uint32_t size[4] = {100u, 100u, 100u, 100u};
        for (int i = 0; i < 4 && i + 2 < argc; ++i) size[i] = static_cast<std::uint32_t>(std::atoi(argv[i + 2]));
        kScales = hs::Scales::from_percent(size[0], size[1], size[2], size[3]);
        for (const std::string &line : replay_file(argv[1]).lines) std::cout << line << "\n";
        return 0;
    }
    test_parse_and_owner();
    const std::filesystem::path dir = MHFU_HUD_SCALE_FIXTURE_DIR;
    const bool update = [] {
        const char *value = std::getenv("MHFU_UPDATE_HUD_SCALE_FIXTURES");
        return value != nullptr && *value == '1';
    }();
    std::vector<std::filesystem::path> traces;
    for (const auto &entry : std::filesystem::directory_iterator(dir))
        if (entry.path().extension() == ".trace") traces.push_back(entry.path());
    std::sort(traces.begin(), traces.end());
    for (const std::filesystem::path &trace : traces) test_fixture(trace, update);
    const std::size_t fixtures = traces.size();
    check(fixtures > 0, "at least one traced screen in " + dir.string());
    if (failures != 0) {
        std::cerr << failures << " failure(s)\n";
        return 1;
    }
    std::cout << "hud scale replay: " << fixtures << " screen(s) match\n";
    return 0;
}
