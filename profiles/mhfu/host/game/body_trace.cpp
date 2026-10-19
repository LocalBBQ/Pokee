#include "game/body_trace.hpp"

#include "game/guest_layout.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <sstream>

namespace mhfu::game::body_trace {

using namespace layout;

bool enabled() noexcept {
    static const bool on = std::getenv("MHFU_TRACE_BODIES") != nullptr;
    return on;
}

std::optional<std::uint32_t> parse_watch(const char *text) noexcept {
    if (text == nullptr || *text == '\0') return std::nullopt;
    char *end = nullptr;
    const unsigned long offset = std::strtoul(text, &end, 0);
    if (end == text || *end != '\0' || offset >= kSpan) return std::nullopt;
    return static_cast<std::uint32_t>(offset);
}

std::optional<std::uint32_t> watch_offset() noexcept {
    static const std::optional<std::uint32_t> offset = parse_watch(std::getenv("MHFU_TRACE_BODIES_WATCH"));
    return offset;
}

bool raptor(std::uint8_t species) noexcept {
    return species == 0x0Du || species == 0x10u || species == 0x1Eu || species == 0x23u;
}

namespace {

using Follow = Fields::Follow;

double read(const std::vector<std::uint8_t> &bytes, Width width, std::uint32_t offset) {
    switch (width) {
    case Width::Byte:
        return bytes[offset];
    case Width::Half: {
        std::int16_t value;
        std::memcpy(&value, bytes.data() + offset, sizeof value);
        return value;
    }
    case Width::Float: {
        float value;
        std::memcpy(&value, bytes.data() + offset, sizeof value);
        return value;
    }
    }
    return 0.0;
}

// A float a timer could hold: a number that is not denormal. The words that
// hold small integers, a halfword counter among them, read as denormals.
bool plain_float(double value) {
    const auto single = static_cast<float>(value);
    return std::isfinite(single) && std::fpclassify(single) != FP_SUBNORMAL;
}

std::uint32_t size_of(Width width) {
    return width == Width::Byte ? 1u : width == Width::Half ? 2u : 4u;
}

void start(std::vector<Follow> &fields, const std::vector<std::uint8_t> &bytes, Width width) {
    const std::uint32_t step = size_of(width);
    fields.assign(bytes.size() / step, {});
    for (std::uint32_t i = 0; i < fields.size(); ++i) {
        Follow &field = fields[i];
        field.first = field.previous = read(bytes, width, i * step);
        if (width == Width::Float && !plain_float(field.first)) field.broken = true;
    }
}

void step(Follow &field, double value, bool unit_steps, std::uint32_t frame) {
    if (field.broken || value == field.previous) return;
    const double change = value - field.previous;
    field.previous = value;
    // The game may set a timer a frame or more after the health reaches
    // zero: the field's first change is then where it starts, not a step.
    if (!field.started) {
        field.started = true;
        if (!unit_steps || std::fabs(change) != 1.0) {
            field.first = value;
            return;
        }
    }
    const int way = change > 0.0 ? 1 : -1;
    if (!std::isfinite(change) || (field.direction != 0 && way != field.direction) ||
        (unit_steps && std::fabs(change) != 1.0)) {
        field.broken = true;
        return;
    }
    if (field.direction == 0) field.from = frame;
    field.direction = way;
    const double size = std::fabs(change);
    field.smallest = field.changes == 0u ? size : std::min(field.smallest, size);
    field.largest = std::max(field.largest, size);
    ++field.changes;
}

void advance(std::vector<Follow> &fields, const std::vector<std::uint8_t> &bytes, Width width, std::uint32_t frame) {
    const std::uint32_t size = size_of(width);
    const bool unit_steps = width != Width::Float;
    for (std::uint32_t i = 0; i < fields.size(); ++i) {
        Follow &field = fields[i];
        if (field.broken) continue;
        const double value = read(bytes, width, i * size);
        if (width == Width::Float && !plain_float(value))
            field.broken = true;
        else
            step(field, value, unit_steps, frame);
    }
}

// A float counts in steps of about the same size: a timer adds the same
// amount each frame, or a multiple of it when frames are skipped.
void collect(const std::vector<Follow> &fields, Width width, std::vector<Counter> &out) {
    const std::uint32_t size = size_of(width);
    for (std::uint32_t i = 0; i < fields.size(); ++i) {
        const Follow &field = fields[i];
        if (field.broken || field.changes < kMinChanges) continue;
        if (width == Width::Float && field.largest > field.smallest * 4.0) continue;
        out.push_back({width, i * size, field.first, field.previous, field.changes, field.from});
    }
}

} // namespace

void Fields::see(const std::vector<std::uint8_t> &bytes) {
    const std::uint32_t frame = frames_++;
    if (frame == 0u) {
        start(bytes_, bytes, Width::Byte);
        start(halves_, bytes, Width::Half);
        start(floats_, bytes, Width::Float);
        flips_.assign(bytes.size(), {});
        for (std::uint32_t i = 0; i < flips_.size(); ++i) flips_[i] = {i, bytes[i], bytes[i], 0u, 0u};
        return;
    }
    advance(bytes_, bytes, Width::Byte, frame);
    advance(halves_, bytes, Width::Half, frame);
    advance(floats_, bytes, Width::Float, frame);
    for (Flag &flip : flips_) {
        const std::uint8_t value = bytes[flip.offset];
        if (value == flip.last) continue;
        if (flip.changes == 0u) flip.at = frame;
        flip.last = value;
        // Only how few changes matters; stop counting past the limit.
        if (flip.changes <= kMaxFlips) ++flip.changes;
    }
}

std::vector<Flag> Fields::flags() const {
    std::vector<Flag> out;
    const std::uint32_t late = frames_ > kLateFrames ? frames_ - kLateFrames : 0u;
    for (const Flag &flip : flips_)
        if (flip.changes != 0u && flip.changes <= kMaxFlips && flip.at >= late) out.push_back(flip);
    std::stable_sort(out.begin(), out.end(), [](const Flag &a, const Flag &b) { return a.at < b.at; });
    if (out.size() > kMaxFlags) out.resize(kMaxFlags);
    return out;
}

std::vector<Counter> Fields::counters() const {
    std::vector<Counter> halves;
    collect(halves_, Width::Half, halves);
    std::vector<Counter> bytes;
    collect(bytes_, Width::Byte, bytes);
    std::vector<Counter> out = halves;
    for (const Counter &byte : bytes) {
        const bool shown = std::any_of(halves.begin(), halves.end(),
            [&](const Counter &half) { return half.offset == (byte.offset & ~1u); });
        if (!shown) out.push_back(byte);
    }
    collect(floats_, Width::Float, out);
    std::stable_sort(out.begin(), out.end(), [](const Counter &a, const Counter &b) { return a.changes > b.changes; });
    if (out.size() > kMaxReported) out.resize(kMaxReported);
    return out;
}

std::string describe(const Counter &counter) {
    std::ostringstream line;
    const char *name = counter.width == Width::Byte ? "u8 " : counter.width == Width::Half ? "s16" : "f32";
    line << name << " +0x" << std::hex << std::uppercase << counter.offset << std::dec << ": " << counter.first
         << " -> " << counter.last << " in " << counter.changes << " changes from frame " << counter.from;
    return line.str();
}

std::string describe(const Flag &flag, std::uint32_t frames) {
    std::ostringstream line;
    line << "flag +0x" << std::hex << std::uppercase << flag.offset << ": 0x" << static_cast<unsigned>(flag.first)
         << " -> 0x" << static_cast<unsigned>(flag.last) << std::dec << " at frame " << flag.at << " of " << frames
         << " (" << flag.changes << (flag.changes == 1u ? " change)" : " changes)");
    return line.str();
}

void Tracker::report(
    const Body &body, const char *why, std::ostream &out, std::chrono::steady_clock::time_point now) const {
    const double seconds = std::chrono::duration<double>(now - body.died_at).count();
    out << "[bodies] 0x" << std::hex << body.entity << std::dec << " species " << static_cast<unsigned>(body.species)
        << " " << why << " after " << (frame_ - body.died) << " frames (" << seconds << " s)" << std::endl;
    const std::vector<Counter> found = body.fields.counters();
    if (found.empty()) out << "[bodies]   no field counted" << std::endl;
    for (const Counter &counter : found) out << "[bodies]   " << describe(counter) << std::endl;
    for (const Flag &flag : body.fields.flags())
        out << "[bodies]   " << describe(flag, body.fields.frames()) << std::endl;
}

void Tracker::frame(const psprecomp::GuestMemory &memory, std::ostream &out, std::chrono::steady_clock::time_point now) {
    if (!quest_running(memory) || entity_manager(memory) == 0u) {
        bodies_.clear();
        return;
    }
    ++frame_;
    for (Body &body : bodies_) body.seen = false;
    std::vector<std::uint8_t> bytes(kSpan);
    for_each_entity(memory, [&](const EntityNode &node) {
        const std::uint32_t at = node.entity;
        if (node.type == kPlayerNode || !entity_readable(memory, at) || !memory.contains(at, kSpan)) return;
        if (memory.load32(at + kVtable) == kHunterVtable) return;
        const auto most = static_cast<std::int16_t>(memory.load16(at + kMostHealth));
        const auto health = static_cast<std::int16_t>(memory.load16(at + kHealth));
        const std::uint8_t species = memory.load8(at + kMonsterId);
        auto known = std::find_if(bodies_.begin(), bodies_.end(), [&](const Body &b) { return b.entity == at; });
        if (known != bodies_.end() && known->seen) return; // a node listed twice
        if (known != bodies_.end() && known->species != species) {
            report(*known, "changed species", out, now);
            bodies_.erase(known);
            known = bodies_.end();
        }
        if (most <= 0) return;
        if (health > 0) {
            if (known != bodies_.end()) {
                report(*known, "came back to life", out, now);
                bodies_.erase(known);
            }
            return;
        }
        if (known == bodies_.end()) {
            out << "[bodies] 0x" << std::hex << at << std::dec << " species " << static_cast<unsigned>(species)
                << " died, stage " << memory.load16(at + kStageId) << std::endl;
            bodies_.push_back({at, species, frame_, now, false, {}});
            known = bodies_.end() - 1;
            if (watch_ && !watched_ && raptor(species)) {
                watched_ = true;
                watch_request_ = at + *watch_;
                out << "[bodies] watching writes to +0x" << std::hex << *watch_ << " of 0x" << at << std::dec
                    << std::endl;
            }
        }
        known->seen = true;
        for (std::uint32_t i = 0; i < kSpan; ++i) bytes[i] = memory.load8(at + i);
        known->fields.see(bytes);
    });
    for (auto it = bodies_.begin(); it != bodies_.end();) {
        if (it->seen) {
            ++it;
            continue;
        }
        report(*it, "left the list", out, now);
        it = bodies_.erase(it);
    }
}

} // namespace mhfu::game::body_trace
