// The MHFU_TRACE_BODIES trace (game/body_trace.hpp) without game data: a
// stand-in for the quest overlay's header and the entity list, with a body
// whose countdown the trace has to find.
#include "game/body_trace.hpp"
#include "game/guest_layout.hpp"

#include <bit>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace {
using namespace mhfu::game::body_trace;
namespace game = mhfu::game::layout;

constexpr std::uint32_t manager = 0x09100000u;
constexpr std::uint32_t node = 0x09200000u;
constexpr std::uint32_t player_node = 0x09200040u;
constexpr std::uint32_t monster = 0x09000000u;
constexpr std::uint32_t player = 0x09003000u;
int failures{};

void check(bool condition, const char *message) {
    if (!condition) {
        ++failures;
        std::cerr << "FAIL: " << message << '\n';
    }
}

void start_quest(psprecomp::GuestMemory &memory) {
    memory.store32(game::kTaskSlot, game::kOverlayMagic);
    memory.store32(game::kTaskSlot + 8u, game::kTaskSlot);
    const char name[] = "game_task.ovl";
    for (std::uint32_t i = 0; i < sizeof(name); ++i)
        memory.store8(game::kTaskSlot + game::kOverlayName + i, static_cast<std::uint8_t>(name[i]));
}

void end_quest(psprecomp::GuestMemory &memory) {
    memory.store32(game::kTaskSlot, 0u);
}

// The list: the monster's node first, then the player's, which ends it.
void list_monster(psprecomp::GuestMemory &memory, bool listed) {
    memory.store32(game::kEntityList, manager);
    memory.store32(manager + game::kListHead, listed ? node : player_node);
    memory.store32(node + game::kNodeOlder, player_node);
    memory.store32(node + game::kNodeEntity, monster);
    memory.store8(node + game::kNodeType, 2u);
    memory.store32(player_node + game::kNodeOlder, 0u);
    memory.store32(player_node + game::kNodeEntity, player);
    memory.store8(player_node + game::kNodeType, game::kPlayerNode);
}

void set_monster(psprecomp::GuestMemory &memory, std::int16_t health, std::uint8_t species) {
    memory.store16(monster + game::kHealth, static_cast<std::uint16_t>(health));
    memory.store16(monster + game::kMostHealth, 120u);
    memory.store8(monster + game::kMonsterId, species);
}

std::vector<std::uint8_t> snapshot(std::int16_t timer, std::uint8_t noise, float clock) {
    std::vector<std::uint8_t> bytes(64u, 0u);
    bytes[0x10] = static_cast<std::uint8_t>(timer & 0xFF);
    bytes[0x11] = static_cast<std::uint8_t>(static_cast<std::uint16_t>(timer) >> 8);
    bytes[0x20] = noise;
    const auto bits = std::bit_cast<std::uint32_t>(clock);
    for (int i = 0; i < 4; ++i) bytes[0x30 + i] = static_cast<std::uint8_t>(bits >> (8 * i));
    return bytes;
}

void counting_fields() {
    Fields fields;
    // A halfword counting down by one each frame, a byte that jumps around,
    // and a float counting up by a thirtieth.
    for (int frame = 0; frame < 60; ++frame) {
        const auto timer = static_cast<std::int16_t>(900 - frame);
        fields.see(snapshot(timer, static_cast<std::uint8_t>((frame * 37) & 0xFF), static_cast<float>(frame) / 30.0f));
    }
    const std::vector<Counter> found = fields.counters();
    bool half = false, byte_again = false, noise = false, clock = false;
    for (const Counter &counter : found) {
        if (counter.width == Width::Half && counter.offset == 0x10u) {
            half = true;
            check(counter.first == 900.0 && counter.last == 841.0, "the countdown's first and last values are kept");
            check(counter.changes == 59u, "every step of the countdown is counted");
        }
        if (counter.width == Width::Byte && (counter.offset == 0x10u || counter.offset == 0x11u)) byte_again = true;
        if (counter.offset == 0x20u) noise = true;
        if (counter.width == Width::Float && counter.offset == 0x30u) clock = true;
    }
    check(half, "a halfword counting down by one is reported");
    check(!byte_again, "its low byte is not reported again");
    check(!noise, "a field that moves both ways is not reported");
    check(clock, "a float counting up in even steps is reported");
    bool denormal = false;
    for (const Counter &counter : found)
        if (counter.width == Width::Float && counter.offset == 0x10u) denormal = true;
    check(!denormal, "the countdown's word is not reported again as a denormal float");
    check(!found.empty() && found.front().changes >= found.back().changes, "the most changes come first");
}

void too_few_changes() {
    Fields fields;
    for (int frame = 0; frame < static_cast<int>(kMinChanges); ++frame)
        fields.see(snapshot(static_cast<std::int16_t>(100 - frame), 0u, 0.0f));
    check(fields.counters().empty(), "a field one change short of kMinChanges is not reported");
    fields.see(snapshot(static_cast<std::int16_t>(100 - static_cast<int>(kMinChanges)), 0u, 0.0f));
    check(fields.counters().size() == 1u, "kMinChanges changes are reported");
}

void steps_of_two_break_a_counter() {
    Fields fields;
    for (int frame = 0; frame < 40; ++frame) fields.see(snapshot(static_cast<std::int16_t>(400 - frame * 2), 0u, 0.0f));
    for (const Counter &counter : fields.counters())
        check(counter.width != Width::Half || counter.offset != 0x10u, "a halfword stepping by two is not a timer");
}

void describes_a_counter() {
    const std::string line = describe({Width::Half, 0x2A4u, 900.0, 0.0, 900u, 3u});
    check(line == "s16 +0x2A4: 900 -> 0 in 900 changes from frame 3",
        "the counter's line names width, offset, values and its first step");
    const std::string flag = describe({0x3Cu, 0x01u, 0x05u, 2u, 470u}, 477u);
    check(flag == "flag +0x3C: 0x1 -> 0x5 at frame 470 of 477 (2 changes)", "the flag's line names when it changed");
    check(describe({0x3Cu, 0x00u, 0x01u, 1u, 9u}, 10u).find("(1 change)") != std::string::npos, "one change reads so");
}

void late_flags() {
    Fields fields;
    // 300 frames: the byte at 0x08 flips at frame 50, too early; the one at
    // 0x09 at 250 and again at 260; the one at 0x0A flips at 200 and then
    // every frame; the one at 0x0B flips at 290.
    for (std::uint32_t frame = 0; frame < 300u; ++frame) {
        std::vector<std::uint8_t> bytes(16u, 0u);
        bytes[0x08] = frame >= 50u ? 1u : 0u;
        bytes[0x09] = frame >= 260u ? 7u : frame >= 250u ? 3u : 0u;
        bytes[0x0A] = frame >= 200u ? static_cast<std::uint8_t>(frame) : 0u;
        bytes[0x0B] = frame >= 290u ? 2u : 0u;
        fields.see(bytes);
    }
    check(fields.frames() == 300u, "every snapshot is counted");
    const std::vector<Flag> flags = fields.flags();
    check(flags.size() == 2u, "only the late bytes that rarely change are flags");
    if (flags.size() == 2u) {
        check(flags[0].offset == 0x09u && flags[0].at == 250u && flags[0].changes == 2u && flags[0].last == 7u,
            "a flag keeps its first change, its count and its last value");
        check(flags[1].offset == 0x0Bu && flags[1].at == 290u, "flags come earliest first");
    }
}

void counters_name_their_first_step() {
    Fields fields;
    for (std::uint32_t frame = 0; frame < 40u; ++frame) {
        std::vector<std::uint8_t> bytes(4u, 0u);
        if (frame >= 10u) bytes[0] = static_cast<std::uint8_t>(frame);
        fields.see(bytes);
    }
    const std::vector<Counter> found = fields.counters();
    // Frame 10 jumps from 0 to 10, where it starts; frame 11 is the first step.
    check(found.size() == 1u && found[0].from == 11u && found[0].first == 10.0, "a counter starts after its jump");
}

std::string run(Tracker &tracker, psprecomp::GuestMemory &memory, std::chrono::steady_clock::time_point at) {
    std::ostringstream out;
    tracker.frame(memory, out, at);
    return out.str();
}

void reports_a_body_that_leaves() {
    psprecomp::GuestMemory memory(64u * 1024u * 1024u);
    start_quest(memory);
    list_monster(memory, true);
    set_monster(memory, 50, 0x10u);
    Tracker tracker;
    const auto t0 = std::chrono::steady_clock::time_point{};
    check(run(tracker, memory, t0).empty(), "a living monster prints nothing");
    set_monster(memory, 0, 0x10u);
    const std::string died = run(tracker, memory, t0);
    check(died.find("species 16 died") != std::string::npos, "the death is printed with the species");
    for (int frame = 1; frame <= 30; ++frame) {
        memory.store16(monster + 0x500u, static_cast<std::uint16_t>(300 - frame));
        if (frame == 25) memory.store8(monster + 0x600u, 1u);
        check(run(tracker, memory, t0 + std::chrono::milliseconds(frame * 33)).empty(), "a lying body prints nothing");
    }
    list_monster(memory, false);
    const std::string gone = run(tracker, memory, t0 + std::chrono::seconds(2));
    check(gone.find("left the list after 31 frames (2 s)") != std::string::npos, "the frames and seconds are printed");
    // The timer is set a frame after the death: 299 is where it starts.
    check(gone.find("s16 +0x500: 299 -> 270 in 29 changes from frame 2") != std::string::npos,
        "the countdown is printed");
    check(gone.find("flag +0x600: 0x0 -> 0x1 at frame 25 of 31 (1 change)") != std::string::npos,
        "the byte that flipped near the end is printed");
    check(run(tracker, memory, t0 + std::chrono::seconds(3)).empty(), "a body is reported once");
}

void reports_a_reused_slot() {
    psprecomp::GuestMemory memory(64u * 1024u * 1024u);
    start_quest(memory);
    list_monster(memory, true);
    set_monster(memory, -5, 0x10u);
    Tracker tracker;
    run(tracker, memory, {});
    set_monster(memory, 120, 0x10u);
    check(run(tracker, memory, {}).find("came back to life") != std::string::npos, "a revived slot ends the body");
    set_monster(memory, 0, 0x10u);
    run(tracker, memory, {});
    set_monster(memory, 0, 0x04u);
    const std::string changed = run(tracker, memory, {});
    check(changed.find("species 16 changed species") != std::string::npos, "a new species ends the old body");
    check(changed.find("species 4 died") != std::string::npos, "and starts the new one");
}

void forgets_bodies_after_the_quest() {
    psprecomp::GuestMemory memory(64u * 1024u * 1024u);
    start_quest(memory);
    list_monster(memory, true);
    set_monster(memory, 0, 0x10u);
    Tracker tracker;
    run(tracker, memory, {});
    end_quest(memory);
    check(run(tracker, memory, {}).empty(), "leaving the quest reports nothing");
    start_quest(memory);
    list_monster(memory, false);
    check(run(tracker, memory, {}).empty(), "a body from the last quest is not reported in the next");
}

void skips_what_is_not_a_body() {
    psprecomp::GuestMemory memory(64u * 1024u * 1024u);
    start_quest(memory);
    list_monster(memory, true);
    set_monster(memory, 0, 0x10u);
    memory.store16(monster + game::kMostHealth, 0u); // a companion
    Tracker tracker;
    check(run(tracker, memory, {}).empty(), "an entity without a health pool is not a body");
    memory.store16(monster + game::kMostHealth, 120u);
    memory.store32(monster + game::kVtable, game::kHunterVtable);
    check(run(tracker, memory, {}).empty(), "a hunter is not a body");
    // The player's node is skipped even with a body's fields.
    memory.store32(monster + game::kVtable, 0u);
    memory.store16(player + game::kHealth, 0u);
    memory.store16(player + game::kMostHealth, 100u);
    const std::string out = run(tracker, memory, {});
    check(out.find("0x9003000") == std::string::npos, "the player's node is not a body");
}

void parses_the_watch() {
    check(parse_watch("0x244") == 0x244u, "a hex offset is read");
    check(parse_watch("580") == 580u, "a decimal offset is read");
    check(!parse_watch(nullptr) && !parse_watch(""), "unset is no watch");
    check(!parse_watch("0x24x") && !parse_watch("abc"), "text that is not a number is no watch");
    check(!parse_watch("0x1000"), "an offset past the followed span is no watch");
    check(raptor(0x0Du) && raptor(0x10u) && raptor(0x1Eu) && raptor(0x23u), "the four raptors arm it");
    check(!raptor(0x45u) && !raptor(0x46u), "an Anteka or a Popo does not");
}

void watches_the_first_raptor() {
    psprecomp::GuestMemory memory(64u * 1024u * 1024u);
    start_quest(memory);
    list_monster(memory, true);
    set_monster(memory, 0, 0x45u); // an Anteka
    Tracker tracker(0x244u);
    run(tracker, memory, {});
    check(!tracker.take_watch(), "a body that is not a raptor asks for nothing");
    set_monster(memory, 0, 0x23u); // a Giaprey in the same slot
    const std::string out = run(tracker, memory, {});
    check(out.find("watching writes to +0x244 of 0x9000000") != std::string::npos, "the watch is printed");
    check(tracker.take_watch() == monster + 0x244u, "the first raptor's field is asked for");
    check(!tracker.take_watch(), "the request is handed over once");
    set_monster(memory, 0, 0x10u); // a Velociprey after it
    run(tracker, memory, {});
    check(!tracker.take_watch(), "a later raptor does not move the watch");
    Tracker plain;
    set_monster(memory, 0, 0x23u);
    run(plain, memory, {});
    check(!plain.take_watch(), "without an offset nothing is watched");
}

} // namespace

int main() {
    parses_the_watch();
    watches_the_first_raptor();
    counting_fields();
    too_few_changes();
    steps_of_two_break_a_counter();
    describes_a_counter();
    late_flags();
    counters_name_their_first_step();
    reports_a_body_that_leaves();
    reports_a_reused_slot();
    forgets_bodies_after_the_quest();
    skips_what_is_not_a_body();
    if (failures == 0) std::cout << "body trace tests passed\n";
    return failures == 0 ? 0 : 1;
}
