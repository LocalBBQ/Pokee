#include "game/hit_trace.hpp"

#include <algorithm>
#include <array>
#include <atomic>
#include <bit>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <mutex>
#include <set>

namespace mhfu::game::hit_trace {
namespace {

// The resident executable's code, and the two overlay slots a quest's code
// runs from: game_task.ovl and the monster's em overlay (game/guest_layout.hpp).
constexpr std::uint32_t kResidentStart = 0x08804000u;
constexpr std::uint32_t kResidentEnd = 0x08A40000u;

std::atomic<std::uint32_t> &count() {
    static std::atomic<std::uint32_t> hits{0u};
    return hits;
}

// A call remembered until its return: the registers then and the lines
// about the stack and the records, which are read before the callee runs.
struct Pending {
    psprecomp::AllegrexContext ctx;
    std::optional<int> health;
    std::vector<std::string> lines; // all but the first, which gets the hit's number
    std::vector<std::uint8_t> ram;  // MHFU_TRACE_HITS_RAM: user RAM at the call
};

// MHFU_TRACE_HITS_RAM=directory: user RAM as the call begins, written as
// hit_NN.bin for each of the first kRamDumps hits printed. For finding,
// between a hit on one part and a hit on another, what the game kept of
// where each landed, and for reading the code of the callers.
constexpr std::uint32_t kRamDumps = 12u;

const std::filesystem::path &ram_directory() {
    static const std::filesystem::path directory = [] {
        // Each run in its own folder, so a second run keeps the first's.
        const char *text = std::getenv("MHFU_TRACE_HITS_RAM");
        if (text == nullptr) return std::filesystem::path{};
        const auto seconds = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        return std::filesystem::path(text) / ("run_" + std::to_string(seconds));
    }();
    return directory;
}

struct State {
    std::mutex mutex;
    std::map<std::uint32_t, Pending> pending;  // by depth
    std::set<std::uint32_t> quiet_callers;     // ra of calls that changed nothing, named once
    std::map<std::uint32_t, int> last_health;  // by monster, as read at the last frame
    std::set<std::uint32_t> changed_by_call;   // monsters a printed call changed this frame
};

State &state() {
    static State s;
    return s;
}

std::string hex(std::uint32_t value) {
    char text[12];
    std::snprintf(text, sizeof(text), "%08X", value);
    return text;
}

std::string reg(const char *name, std::uint32_t value) {
    return std::string(" ") + name + "=" + hex(value);
}

} // namespace

bool enabled() noexcept {
    static const bool on = std::getenv("MHFU_TRACE_HITS") != nullptr;
    return on;
}

namespace {
std::atomic<GuestClock> &clock_source() {
    static std::atomic<GuestClock> source{nullptr};
    return source;
}
} // namespace

void set_clock(GuestClock clock) noexcept {
    clock_source().store(clock, std::memory_order_relaxed);
}

std::uint64_t now_ms() noexcept {
    const GuestClock clock = clock_source().load(std::memory_order_relaxed);
    return clock != nullptr ? clock() : 0u;
}

std::uint32_t parse_limit(const char *text) noexcept {
    if (text == nullptr || *text == '\0') return kDefaultLimit;
    char *end = nullptr;
    const unsigned long value = std::strtoul(text, &end, 10);
    if (end == text || *end != '\0' || value == 0ul || value > 1000000ul) return kDefaultLimit;
    return static_cast<std::uint32_t>(value);
}

std::uint32_t limit() noexcept {
    static const std::uint32_t value = parse_limit(std::getenv("MHFU_TRACE_HITS_LIMIT"));
    return value;
}

bool looks_like_code(std::uint32_t word) noexcept {
    if ((word & 3u) != 0u) return false;
    return (word >= kResidentStart && word < kResidentEnd) ||
        (word >= layout::kTaskSlot && word < layout::kEmOverlayEnd);
}

std::optional<Frame> frame_of(const psprecomp::GuestMemory &memory, std::uint32_t ra) {
    if (!in_ram(ra)) return std::nullopt;
    Frame frame;
    std::uint32_t at = (ra - 8u) & ~3u;
    for (std::uint32_t steps = 0u;; at -= 4u, ++steps) {
        if (steps == 4096u || !memory.contains(at, 4u)) return std::nullopt;
        const std::uint32_t word = memory.load32(at);
        // addiu sp, sp, -N
        if ((word >> 16u) == 0x27BDu && (word & 0x8000u) != 0u) {
            frame.start = at;
            frame.size = 0x10000u - (word & 0xFFFFu);
            break;
        }
    }
    for (std::uint32_t pc = frame.start + 4u, steps = 0u; steps < 64u && memory.contains(pc, 4u);
         pc += 4u, ++steps) {
        const std::uint32_t word = memory.load32(pc);
        if ((word >> 16u) == 0xAFBFu) { // sw ra, N(sp)
            frame.ra_offset = word & 0xFFFFu;
            break;
        }
        if (word == 0x03E00008u) break; // jr ra: returns without saving it
    }
    return frame;
}

std::string call_chain(const psprecomp::GuestMemory &memory, std::uint32_t ra, std::uint32_t sp, int levels) {
    std::string chain;
    for (int level = 0; level < levels; ++level) {
        const std::optional<Frame> frame = frame_of(memory, ra);
        if (!frame) break;
        chain += " " + hex(ra) + "@" + hex(frame->start);
        if (frame->ra_offset == ~0u || !memory.contains(sp + frame->ra_offset, 4u)) break;
        ra = memory.load32(sp + frame->ra_offset);
        sp += frame->size;
    }
    return chain;
}

std::string words_line(
    const psprecomp::GuestMemory &memory, const char *label, std::uint32_t address, std::uint32_t count) {
    if (!in_ram(address) || !memory.contains(address & ~3u, count * 4u)) return {};
    std::string line = std::string(label) + " " + hex(address & ~3u) + ":";
    for (std::uint32_t i = 0; i < count; ++i) {
        const std::uint32_t word = memory.load32((address & ~3u) + i * 4u);
        line += " " + hex(word);
        if (looks_like_code(word)) line += "*";
    }
    return line;
}

std::string floats_line(
    const psprecomp::GuestMemory &memory, const char *label, std::uint32_t address, std::uint32_t count) {
    if (!in_ram(address) || !memory.contains(address & ~3u, count * 4u)) return {};
    std::string line = std::string(label) + " " + hex(address & ~3u) + ":";
    for (std::uint32_t i = 0; i < count; ++i) {
        char text[24];
        std::snprintf(text, sizeof(text), " %g",
            static_cast<double>(std::bit_cast<float>(memory.load32((address & ~3u) + i * 4u))));
        line += text;
    }
    return line;
}

std::string players_line(const psprecomp::GuestMemory &memory) {
    std::string line;
    layout::for_each_entity(memory, [&](const layout::EntityNode &node) {
        const std::uint32_t entity = node.entity;
        if (node.type != layout::kPlayerNode || !in_ram(entity) || !memory.contains(entity + layout::kPosition, 12u))
            return;
        char text[96];
        std::snprintf(text, sizeof(text), " %s at %g %g %g", hex(entity).c_str(),
            static_cast<double>(std::bit_cast<float>(memory.load32(entity + layout::kPosition))),
            static_cast<double>(std::bit_cast<float>(memory.load32(entity + layout::kPosition + 4u))),
            static_cast<double>(std::bit_cast<float>(memory.load32(entity + layout::kPosition + 8u))));
        line += text;
    });
    return line.empty() ? std::string{} : "[hit]   players" + line;
}

std::vector<Candidate> contact_candidates(
    const psprecomp::GuestMemory &memory, const psprecomp::AllegrexContext &ctx) {
    std::vector<Candidate> found;
    const std::uint32_t monster = ctx.gpr[4];
    if (!in_ram(monster) || !memory.contains(monster + layout::kPosition, 12u)) return found;
    const auto load = [&memory](std::uint32_t address) { return std::bit_cast<float>(memory.load32(address)); };
    const float mx = load(monster + layout::kPosition);
    const float my = load(monster + layout::kPosition + 4u);
    const float mz = load(monster + layout::kPosition + 8u);
    std::vector<std::array<float, 3>> players;
    layout::for_each_entity(memory, [&](const layout::EntityNode &node) {
        if (node.type == layout::kPlayerNode && in_ram(node.entity) &&
            memory.contains(node.entity + layout::kPosition, 12u))
            players.push_back({load(node.entity + layout::kPosition), load(node.entity + layout::kPosition + 4u),
                load(node.entity + layout::kPosition + 8u)});
    });
    std::set<std::uint32_t> seen;
    const auto scan = [&](const char *source, std::uint32_t base, std::uint32_t bytes) {
        if (!in_ram(base) || (base & 3u) != 0u || !memory.contains(base, bytes + 8u)) return;
        for (std::uint32_t offset = 0; offset < bytes && found.size() < kMaxCandidates; offset += 4u) {
            const std::uint32_t at = base + offset;
            const float x = load(at), y = load(at + 4u), z = load(at + 8u);
            if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(z)) continue;
            const float across = std::hypot(x - mx, z - mz);
            const float height = y - my;
            if (!(across <= kCandidateReach) || !(height >= kCandidateLowest) || !(height <= kCandidateReach))
                continue;
            const bool hunter = std::any_of(players.begin(), players.end(), [&](const std::array<float, 3> &p) {
                return std::fabs(p[0] - x) < 1.0f && std::fabs(p[1] - y) < 1.0f && std::fabs(p[2] - z) < 1.0f;
            });
            if (hunter || !seen.insert(at).second) continue;
            found.push_back({source, base, offset, x, y, z, across, height});
        }
    };
    // The callers' frames, then the first part of every record on the stack.
    const std::uint32_t sp = ctx.gpr[29];
    scan("stack", sp, kCandidateStack);
    for (std::uint32_t offset = 0; offset < kCandidateStack && in_ram(sp) && memory.contains(sp + offset, 4u);
        offset += 4u) {
        const std::uint32_t word = memory.load32(sp + offset);
        if (word == monster || word == sp || looks_like_code(word) || !in_ram(word)) continue;
        scan("record", word, kCandidateRecord);
    }
    for (std::uint32_t i = 16u; i < 24u; ++i)
        if (ctx.gpr[i] != monster && !looks_like_code(ctx.gpr[i])) scan("saved", ctx.gpr[i], kCandidateRecord);
    scan("monster", monster, kCandidateMonster);
    return found;
}

std::string candidates_line(const std::vector<Candidate> &candidates) {
    if (candidates.empty()) return {};
    std::string line = "[hit]   near";
    for (const Candidate &c : candidates) {
        char text[128];
        std::snprintf(text, sizeof(text), " %s %s+0x%X: %g %g %g (%g across, %g up);", c.source, hex(c.base).c_str(),
            c.offset, static_cast<double>(c.x), static_cast<double>(c.y), static_cast<double>(c.z),
            static_cast<double>(c.across), static_cast<double>(c.height));
        line += text;
    }
    return line;
}

std::string call_line(
    const psprecomp::AllegrexContext &ctx, std::uint32_t hit, std::uint32_t depth, std::optional<int> health) {
    std::string line = "[hit] call #" + std::to_string(hit) + " depth=" + std::to_string(depth);
    static constexpr const char *kArgs[] = {"a0", "a1", "a2", "a3"};
    for (std::uint32_t i = 0; i < 4u; ++i) line += reg(kArgs[i], ctx.gpr[4u + i]);
    static constexpr const char *kSaved[] = {"s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7"};
    for (std::uint32_t i = 0; i < 8u; ++i) line += reg(kSaved[i], ctx.gpr[16u + i]);
    line += reg("ra", ctx.gpr[31]);
    line += reg("sp", ctx.gpr[29]);
    char floats[96];
    std::snprintf(floats, sizeof(floats), " f12..f15=%g %g %g %g", static_cast<double>(ctx.fpr[12]),
        static_cast<double>(ctx.fpr[13]), static_cast<double>(ctx.fpr[14]), static_cast<double>(ctx.fpr[15]));
    line += floats;
    line += health ? " health=" + std::to_string(*health) : std::string(" health=?");
    return line;
}

std::string return_line(
    const psprecomp::AllegrexContext &ctx, std::uint32_t hit, std::uint32_t depth, int before, int after) {
    return "[hit] return #" + std::to_string(hit) + " depth=" + std::to_string(depth) + reg("v0", ctx.gpr[2]) +
        reg("v1", ctx.gpr[3]) + " health " + std::to_string(before) + " -> " + std::to_string(after) +
        " (" + std::to_string(before - after) + ")";
}

std::vector<std::string> describe_call(const psprecomp::GuestMemory &memory, const psprecomp::AllegrexContext &ctx,
    std::uint32_t hit, std::uint32_t depth, std::optional<int> health) {
    std::vector<std::string> lines;
    lines.push_back(call_line(ctx, hit, depth, health));
    lines.push_back("[hit]   chain" + call_chain(memory, ctx.gpr[31], ctx.gpr[29]));
    if (std::string stack = words_line(memory, "[hit]   stack", ctx.gpr[29], 32u); !stack.empty())
        lines.push_back(std::move(stack));
    // a0 is the monster, which damage numbers already read. The other
    // arguments, when they point into RAM, are the hit's own records.
    static constexpr const char *kLabels[] = {"[hit]   a1->", "[hit]   a2->", "[hit]   a3->"};
    for (std::uint32_t i = 0; i < 3u; ++i) {
        const std::uint32_t address = ctx.gpr[5u + i];
        if (!in_ram(address) || address == ctx.gpr[4]) continue;
        if (std::string words = words_line(memory, kLabels[i], address, 16u); !words.empty())
            lines.push_back(std::move(words));
    }
    // On Akantor a1's record holds a position at +0x14 and the same three
    // floats reversed at +0x24, with a pointer between them that changes from
    // hit to hit.
    const std::uint32_t record = ctx.gpr[5];
    if (std::string floats = floats_line(memory, "[hit]   a1 floats", record, 16u); !floats.empty())
        lines.push_back(std::move(floats));
    if (in_ram(record) && memory.contains(record + 0x20u, 4u)) {
        const std::uint32_t pointer = memory.load32(record + 0x20u);
        if (std::string words = words_line(memory, "[hit]   a1[+0x20]->", pointer, 24u); !words.empty())
            lines.push_back(std::move(words));
        if (std::string floats = floats_line(memory, "[hit]   a1[+0x20] floats", pointer, 24u); !floats.empty())
            lines.push_back(std::move(floats));
    }
    if (std::string near = candidates_line(contact_candidates(memory, ctx)); !near.empty())
        lines.push_back(std::move(near));
    if (std::string players = players_line(memory); !players.empty()) lines.push_back(std::move(players));
    lines.push_back("[hit]   monster" + floats_line(memory, "", ctx.gpr[4] + 0x200u, 3u));
    return lines;
}

void note_call(const psprecomp::GuestMemory &memory, const psprecomp::AllegrexContext &ctx, std::uint32_t depth,
    std::optional<int> health) {
    if (count().load(std::memory_order_relaxed) >= limit()) return;
    Pending pending{ctx, health, describe_call(memory, ctx, 0u, depth, health), {}};
    pending.lines.erase(pending.lines.begin());
    constexpr std::uint32_t ram_bytes = layout::kRamEnd - layout::kRamStart;
    if (!ram_directory().empty() && count().load(std::memory_order_relaxed) < kRamDumps)
        if (const std::uint8_t *ram = memory.raw_pointer(layout::kRamStart, ram_bytes))
            pending.ram.assign(ram, ram + ram_bytes);
    std::lock_guard lock(state().mutex);
    state().pending[depth] = std::move(pending);
    state().last_health.try_emplace(ctx.gpr[4], health.value_or(0));
}

std::uint32_t log_return(const psprecomp::AllegrexContext &ctx, std::uint32_t depth, int before, int after) {
    State &s = state();
    std::lock_guard lock(s.mutex);
    const auto found = s.pending.find(depth);
    if (found == s.pending.end()) return 0u;
    Pending pending = std::move(found->second);
    s.pending.erase(found);
    const std::uint32_t caller = pending.ctx.gpr[31];
    if (before == after) {
        if (s.quiet_callers.insert(caller).second)
            std::cout << "[hit] calls from ra=" << hex(caller) << " change nothing (the monster's own update); "
                      << "such calls are not printed" << std::endl;
        return 0u;
    }
    const std::uint32_t hit = count().fetch_add(1u, std::memory_order_relaxed) + 1u;
    if (hit > limit()) {
        if (hit == limit() + 1u)
            std::cout << "[hit] " << limit() << " hits traced; MHFU_TRACE_HITS_LIMIT raises the limit" << std::endl;
        return 0u;
    }
    std::cout << call_line(pending.ctx, hit, depth, pending.health) << '\n';
    for (const std::string &line : pending.lines) std::cout << line << '\n';
    std::cout << return_line(ctx, hit, depth, before, after) << std::endl;
    if (!pending.ram.empty()) {
        char name[32];
        std::snprintf(name, sizeof(name), "hit_%02u.bin", hit);
        std::error_code error;
        std::filesystem::create_directories(ram_directory(), error);
        const std::filesystem::path path = ram_directory() / name;
        std::ofstream out(path, std::ios::binary);
        out.write(reinterpret_cast<const char *>(pending.ram.data()), static_cast<std::streamsize>(pending.ram.size()));
        std::cout << "[hit]   ram " << hex(layout::kRamStart) << " written to " << path.string()
                  << (out ? "" : " (failed)") << std::endl;
    }
    s.changed_by_call.insert(pending.ctx.gpr[4]);
    s.last_health[pending.ctx.gpr[4]] = after;
    return hit;
}

void frame(const psprecomp::GuestMemory &memory, HealthOf health_of) {
    State &s = state();
    std::lock_guard lock(s.mutex);
    for (auto &[object, last] : s.last_health) {
        const std::optional<int> now = health_of(memory, object);
        if (!now) continue;
        if (*now != last && !s.changed_by_call.contains(object))
            std::cout << "[hit] health of " << hex(object) << " went " << last << " -> " << *now
                      << " between frames with no change through the watched routine: "
                      << "the hit is written elsewhere t=" << now_ms() << "ms" << std::endl;
        last = *now;
    }
    s.changed_by_call.clear();
}

void reset() noexcept {
    count().store(0u, std::memory_order_relaxed);
    State &s = state();
    std::lock_guard lock(s.mutex);
    s.pending.clear();
    s.quiet_callers.clear();
    s.last_health.clear();
    s.changed_by_call.clear();
}

} // namespace mhfu::game::hit_trace
