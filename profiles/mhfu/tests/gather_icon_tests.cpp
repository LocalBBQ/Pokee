// Gather marks: the game's own radius, the area list, and the picture.
// No game data. Guest memory is a stand-in.
#include "game/gather_icon.hpp"
#include "gpu/ui_motion.hpp"
#include "settings/settings.hpp"

#include "psprecomp/guest_memory.hpp"

#include <algorithm>
#include <bit>
#include <cmath>
#include <iostream>
#include <string>

namespace mhfu::settings {
Settings &current() {
    static Settings settings;
    return settings;
}
} // namespace mhfu::settings

namespace {
using namespace mhfu::game;

int failures{};

void check(bool condition, const char *message) {
    if (!condition) {
        ++failures;
        std::cerr << "FAIL: " << message << '\n';
    }
}

void write_float(psprecomp::GuestMemory &memory, std::uint32_t address, float value) {
    memory.store32(address, std::bit_cast<std::uint32_t>(value));
}

void plant_words(psprecomp::GuestMemory &memory) {
    constexpr std::uint32_t kTaskSlot = 0x09A5F300u;
    constexpr char kName[] = "game_task.ovl";
    memory.store32(kTaskSlot, 0x336F574Du);
    memory.store32(kTaskSlot + 8u, kTaskSlot);
    for (std::size_t i = 0; i < sizeof(kName); ++i)
        memory.store8(kTaskSlot + 32u + static_cast<std::uint32_t>(i), static_cast<std::uint8_t>(kName[i]));
    for (const GatherWord &word : gather_checked_words()) memory.store32(word.address, word.instruction);
}

// One 24-byte record, then the -1 that ends the list.
void plant_node(psprecomp::GuestMemory &memory, std::uint32_t address, float x, float y, float z, float radius,
    std::uint16_t id, std::uint16_t kind) {
    write_float(memory, address, x);
    write_float(memory, address + 4u, y);
    write_float(memory, address + 8u, z);
    write_float(memory, address + 12u, radius);
    memory.store16(address + 16u, id);
    memory.store16(address + 20u, kind);
    write_float(memory, address + 24u, -1.0f);
}

void test_the_radius() {
    psprecomp::GuestMemory memory(32u * 1024u * 1024u);
    constexpr std::uint32_t list = 0x09000000u;
    plant_node(memory, list, 10.0f, 0.0f, 20.0f, 50.0f, 7u, 0u);
    plant_node(memory, list + 24u, 1000.0f, 0.0f, 1000.0f, 10.0f, 8u, 3u);
    write_float(memory, list + 48u, -1.0f);

    const std::optional<GatherNode> on_it = gather_from_list(memory, list, 10.0f, 0.0f, 20.0f);
    check(on_it && on_it->address == list && on_it->id == 7u && on_it->kind == 0u,
        "standing on a node gathers that node");
    const std::optional<GatherNode> edge = gather_from_list(memory, list, 60.0f, 0.0f, 20.0f);
    check(edge && edge->address == list, "the radius is still inside");
    check(!gather_from_list(memory, list, 60.1f, 0.0f, 20.0f), "just past the radius is not");
    check(
        gather_from_list(memory, list, 10.0f, -200.0f, 20.0f) && !gather_from_list(memory, list, 10.0f, -200.1f, 20.0f),
        "200 below the node is the low edge");
    check(gather_from_list(memory, list, 10.0f, 99.9f, 20.0f) && !gather_from_list(memory, list, 10.0f, 100.0f, 20.0f),
        "100 above the node is already outside");

    write_float(memory, list + 12u, 0.0f);
    check(!gather_from_list(memory, list, 10.1f, 0.0f, 20.0f) && gather_from_list(memory, list, 10.0f, 0.0f, 20.0f),
        "a zero radius matches only the node's own point");
    write_float(memory, list + 12u, 50.0f);

    write_float(memory, list, 0.0f);
    write_float(memory, list + 8u, 0.0f);
    write_float(memory, list + 12u, 1.0f);
    const std::optional<GatherNode> second = gather_from_list(memory, list, 1000.0f, 0.0f, 1000.0f);
    check(second && second->address == list + 24u && second->kind == 3u, "a miss walks on to the next record");

    write_float(memory, list, -1.0f);
    check(!gather_from_list(memory, list, -1.0f, 0.0f, 0.0f), "x of -1 ends the list and is not a node");
    plant_node(memory, list, 0.0f, 0.0f, 0.0f, 5.0f, 1u, 0u);
    memory.store32(list + 4u, 0x7FC00000u);
    check(!gather_from_list(memory, list, 0.0f, 0.0f, 0.0f), "a non-finite height stops the walk");
}

void test_unmapped() {
    psprecomp::GuestMemory memory(32u * 1024u * 1024u);
    check(!gather_code_matches(memory) && !gather_node(memory), "without the quest overlay there is no node");
}

void test_the_area_list() {
    psprecomp::GuestMemory memory(32u * 1024u * 1024u);
    constexpr std::uint32_t gamesys = 0x09100000u;
    constexpr std::uint32_t object = 0x09200000u;
    constexpr std::uint32_t ids = 0x09300000u;
    constexpr std::uint32_t areas = 0x09400000u;
    constexpr std::uint32_t base = 0x09500000u;
    constexpr std::uint32_t kGatherObject = 0x08A62D9Cu;
    constexpr std::uint32_t kGameSysPointer = 0x089CC558u;
    constexpr std::uint32_t kStageTable = 0x09CE4808u;
    memory.store32(kGameSysPointer, gamesys);
    memory.store8(gamesys + 0x6AF10u, 2u);
    memory.store32(kStageTable + 16u, 2u);
    memory.store32(kStageTable + 20u, ids);
    memory.store16(ids, 4u);
    memory.store16(ids + 2u, 9u);
    memory.store32(kGatherObject, object);
    memory.store32(object + 96u, areas);
    memory.store32(object + 80u, 24u);
    memory.store32(areas + 4u, base);
    plant_node(memory, base + 24u, 1.0f, 2.0f, 3.0f, 5.0f, 1u, 0u);

    const std::optional<std::uint32_t> list = gather_list_for(memory, 9u);
    check(list && *list == base + 24u, "the stage id picks that area, plus the object's bias");
    check(!gather_list_for(memory, 3u), "a stage the table does not name has no list");
    memory.store32(areas + 4u, 0u);
    check(!gather_list_for(memory, 9u), "an empty area slot has no list");
}

void test_words_and_the_hunter() {
    psprecomp::GuestMemory memory(32u * 1024u * 1024u);
    constexpr std::uint32_t gamesys = 0x09100000u;
    constexpr std::uint32_t object = 0x09200000u;
    constexpr std::uint32_t ids = 0x09300000u;
    constexpr std::uint32_t areas = 0x09400000u;
    constexpr std::uint32_t nodes = 0x09500000u;
    constexpr std::uint32_t hunter_list = 0x09600000u;
    constexpr std::uint32_t hunter = 0x09700000u;
    memory.store32(0x089CC558u, gamesys);
    memory.store8(gamesys + 0x6AF10u, 0u);
    memory.store32(0x09CE4808u, 1u);
    memory.store32(0x09CE480Cu, ids);
    memory.store16(ids, 4u);
    memory.store32(0x08A62D9Cu, object);
    memory.store32(object + 96u, areas);
    memory.store32(object + 80u, 0u);
    memory.store32(areas, nodes);
    plant_node(memory, nodes, 0.0f, 0.0f, 0.0f, 30.0f, 2u, 4u);
    memory.store32(0x089CBD04u, hunter_list);
    memory.store32(hunter_list + 2692u, hunter);
    write_float(memory, hunter + 0x200u, 0.0f);
    write_float(memory, hunter + 0x204u, 0.0f);
    write_float(memory, hunter + 0x208u, 0.0f);
    memory.store16(hunter + 0x29Au, 4u);

    check(!gather_node(memory), "without the game's instructions there is no node");
    plant_words(memory);
    const std::optional<GatherNode> found = gather_node(memory);
    check(found && found->address == nodes && found->kind == 4u, "the followed hunter gathers the node in range");
    memory.store32(gather_checked_words()[0].address, 0u);
    check(!gather_code_matches(memory) && !gather_node(memory), "a different instruction stays out");
}

GatherPicture picture() {
    GatherPicture out;
    out.projection[0] = out.projection[5] = out.projection[10] = 1.0f;
    out.projection[11] = -1.0f;
    out.x_scale = 240.0f;
    out.x_offset = 240.0f;
    out.y_scale = -136.0f;
    out.y_offset = 136.0f;
    return out;
}

void test_on_the_picture() {
    psprecomp::GuestMemory memory(32u * 1024u * 1024u);
    plant_words(memory);
    constexpr std::uint32_t gamesys = 0x09100000u;
    constexpr std::uint32_t object = 0x09200000u;
    constexpr std::uint32_t ids = 0x09300000u;
    constexpr std::uint32_t areas = 0x09400000u;
    constexpr std::uint32_t nodes = 0x09500000u;
    constexpr std::uint32_t hunter_list = 0x09600000u;
    constexpr std::uint32_t hunter = 0x09700000u;
    memory.store32(0x089CC558u, gamesys);
    memory.store8(gamesys + 0x6AF10u, 0u);
    memory.store32(0x09CE4808u, 1u);
    memory.store32(0x09CE480Cu, ids);
    memory.store16(ids, 1u);
    memory.store32(0x08A62D9Cu, object);
    memory.store32(object + 96u, areas);
    memory.store32(object + 80u, 0u);
    memory.store32(areas, nodes);
    plant_node(memory, nodes, 0.0f, 0.0f, -10.0f, 5.0f, 1u, 0u);
    memory.store32(0x089CBD04u, hunter_list);
    memory.store32(hunter_list + 2692u, hunter);
    write_float(memory, hunter + 0x200u, 0.0f);
    write_float(memory, hunter + 0x204u, 0.0f);
    write_float(memory, hunter + 0x208u, -10.0f);
    memory.store16(hunter + 0x29Au, 1u);

    mhfu::settings::current().gather_icon = false;
    gather_note_draw(picture(), 100u);
    check(!gather_mark(memory, GatherPose{0.0f, 100.0f, 0.0f, 0.0f, 100.0f, -10.0f}), "off, nothing is drawn");

    mhfu::settings::current().gather_icon = true;
    check(!gather_mark(memory, GatherPose{0.0f, 100.0f, 0.0f, 0.0f, 100.0f, -10.0f}),
        "a frame with no world draw draws none");
    gather_note_draw(picture(), 10u);
    GatherPicture flat = picture();
    flat.projection[11] = 0.0f;
    gather_note_draw(flat, 1000u);
    const std::optional<GatherMark> at = gather_mark(memory, GatherPose{0.0f, 100.0f, 0.0f, 0.0f, 100.0f, -10.0f});
    check(at && std::fabs(at->at[0] - 0.5f) < 0.05f && std::fabs(at->at[1] - 0.5f) < 0.05f,
        "the mark sits above the node, and an orthographic draw is not the world");

    plant_node(memory, nodes, 0.0f, 0.0f, 10.0f, 5.0f, 1u, 0u);
    write_float(memory, hunter + 0x208u, 10.0f);
    gather_note_draw(picture(), 100u);
    check(!gather_mark(memory, GatherPose{0.0f, 100.0f, 0.0f, 0.0f, 100.0f, -10.0f}),
        "a node behind the camera is not marked");
}

bool near(float a, float b) {
    return std::fabs(a - b) < 1.0e-4f;
}

// The ring is drawn on the disc's edge, so half of it lies inside. The ? stays
// clear when that inner edge is still outside the stroked glyph.
bool question_stays_clear(const GatherShape &shape) {
    const float inner = shape.radius - shape.ring * 0.5f;
    const float mark = shape.glyph * 0.5f + shape.weight;
    return inner >= mark;
}

void test_size_and_thickness() {
    const GatherShape usual = gather_mark_shape(100u, 1u);
    check(near(usual.radius, kGatherDisc) && near(usual.ring, 1.0f) && near(usual.glyph, kGatherGlyph) &&
            near(usual.weight, 0.0f),
        "the usual mark is an 8-pixel disc, a one-pixel ring and the font's own ?");
    const GatherShape half = gather_mark_shape(mhfu::settings::kMinGatherIconSize, 1u);
    check(near(half.radius, kGatherDisc * 0.5f) && near(half.ring, 0.5f) && near(half.glyph, kGatherGlyph * 0.5f) &&
            near(half.weight, 0.0f),
        "half size keeps the usual proportions");
    const GatherShape doubled = gather_mark_shape(200u, 1u);
    check(near(doubled.radius, kGatherDisc * 2.0f) && near(doubled.ring, 2.0f) &&
            near(doubled.glyph, kGatherGlyph * 2.0f) && near(doubled.weight, 0.0f),
        "double size keeps the usual proportions");
    const GatherShape odd = gather_mark_shape(73u, 1u);
    check(near(odd.radius, kGatherDisc * 0.73f) && near(odd.glyph, kGatherGlyph * 0.73f),
        "a size the menu does not step to is still that percent");
    const GatherShape bold = gather_mark_shape(100u, mhfu::settings::kMaxGatherIconThickness);
    check(near(bold.radius, kGatherDisc) && near(bold.glyph, kGatherGlyph) && near(bold.ring, 3.0f) &&
            near(bold.weight, 1.0f),
        "thickness 3 widens the ring and the ? without changing the disc");
    const GatherShape small_bold = gather_mark_shape(mhfu::settings::kMinGatherIconSize, 3u);
    const GatherShape large_bold = gather_mark_shape(mhfu::settings::kMaxGatherIconSize, 3u);
    check(question_stays_clear(bold) && question_stays_clear(small_bold) && question_stays_clear(large_bold),
        "the thickest mark still leaves the ? clear, small or large");
    const GatherShape huge = gather_mark_shape(1000u, 9u);
    check(near(huge.radius, kGatherDisc * 3.0f) && near(huge.ring, 9.0f) && near(huge.weight, 3.0f),
        "a size or thickness past the limit clamps to it");
    const GatherShape tiny = gather_mark_shape(0u, 0u);
    check(near(tiny.radius, kGatherDisc * 0.5f) && near(tiny.ring, 0.5f) && near(tiny.weight, 0.0f),
        "a size or thickness under the limit clamps up");
}

constexpr std::uint32_t kCircle = 0x2000u;
constexpr std::uint32_t kSquare = 0x8000u;
constexpr std::uint32_t kLead = 0x0100u;

// The pulse a hold sends, `reads` long, as a string of '1' (button down) and '0'.
template <typename Read> std::string pulse(unsigned reads, std::uint32_t button, Read read) {
    std::string out;
    for (unsigned i = 0; i < reads; ++i) out += (read() & button) != 0u ? '1' : '0';
    return out;
}

std::string expected_pulse(unsigned reads, unsigned start) {
    std::string out;
    for (unsigned i = 0; i < reads; ++i)
        out += (start + i) % (kGatherPulseDown + kGatherPulseUp) < kGatherPulseDown ? '1' : '0';
    return out;
}

void test_holding_circle() {
    constexpr unsigned kCycle = kGatherPulseDown + kGatherPulseUp;
    GatherRepeat repeat;
    const auto held = [&](bool ready, std::uint32_t extra = 0u) {
        return repeat.apply(kCircle | extra, ready, kCircle);
    };
    check(pulse(kCycle * 4u, kCircle, [&] { return held(true); }) == expected_pulse(kCycle * 4u, 0u),
        "holding ○ on a point presses, lifts and presses again for as long as it is held");
    check(repeat.centres_stick(), "the stick is centred while the hold runs");

    // Running in, stopping, the gather and the stance after it are all the
    // game's business. The pulse does not change with any of them, so the
    // first read the hunter is free has a press within one cycle.
    repeat.reset();
    check(held(true) == kCircle, "a press from a run starts the hold on a press");
    const std::string run = pulse(kCycle * 30u, kCircle, [&] { return held(true); });
    check(run == expected_pulse(kCycle * 30u, 1u), "a long gather from a run keeps the pulse going all the way");
    std::size_t widest = 0;
    for (std::size_t at = 0, gap = 0; at < run.size(); ++at) {
        gap = run[at] == '0' ? gap + 1u : 0u;
        widest = std::max(widest, gap);
    }
    check(widest == kGatherPulseUp, "○ is never up for longer than the up part");

    repeat.reset();
    check(repeat.apply(kCircle, false, kCircle) == kCircle, "away from a point, a held ○ is left alone");
    check(!repeat.centres_stick(), "away from a point the stick moves");

    repeat.reset();
    check(held(true) == kCircle, "a press starts the hold");
    check(repeat.apply(kCircle | kLead | kSquare, true, kCircle) == kCircle && repeat.centres_stick(),
        "L and another button during the hold are left off, and the stick is centred");
    // ○ can leave the resolved pad while another button is down. The hold
    // continues, and the game is shown the pulse, not that button.
    check(pulse(kCycle * 2u, kCircle, [&] { return repeat.apply(kSquare, true, kCircle); }) ==
                expected_pulse(kCycle * 2u, 2u) &&
            repeat.centres_stick() && repeat.underway(),
        "another button with ○ missing from the pad keeps the pulse");
    check(repeat.apply(0u, true, kCircle) == 0u && !repeat.centres_stick() && !repeat.underway(),
        "letting every button go releases the hold");

    repeat.reset();
    check(repeat.apply(kCircle | kLead, true, kCircle) == (kCircle | kLead) && !repeat.underway() &&
            !repeat.centres_stick(),
        "L+○ before a hold starts is left to the item bar");

    // Already holding ○ on the read before the point is in range.
    repeat.reset();
    check(repeat.apply(kCircle, false, kCircle) == kCircle, "walking up with ○ held is not yet a gather");
    check(pulse(kCycle * 2u, kCircle, [&] { return held(true); }) == expected_pulse(kCycle * 2u, kGatherPulseDown),
        "walking onto a point with ○ already held lifts it first, then presses");

    // Letting go stops the pulse at once, even partway through a press.
    repeat.reset();
    check(held(true) == kCircle, "a hold starts");
    check(repeat.apply(0u, true, kCircle) == 0u && !repeat.centres_stick(), "letting go lifts ○ and frees the stick");
    bool quiet = true;
    for (unsigned read = 0; read < kCycle * 2u; ++read) quiet = quiet && repeat.apply(0u, true, kCircle) == 0u;
    check(quiet, "after letting go, nothing is pressed");
    check(held(true) == kCircle, "pressing again is a new press at once");

    // The point going out of range ends the hold; ○ held through it is then
    // already down, so coming back lifts it first.
    repeat.reset();
    check(held(true) == kCircle, "a hold on a point");
    check(repeat.apply(kCircle, false, kCircle) == kCircle && !repeat.underway(), "leaving the point hands ○ back");
    check((held(true) & kCircle) == 0u, "coming back lifts ○ before the next press");

    // Mining and a bug net use □. The pulse is the same shape as ○.
    check(gather_uses_square(3u) && gather_uses_square(4u) && !gather_uses_square(0u),
        "kinds 3 and 4 are □, and plants are ○");
    repeat.reset();
    const auto mined = [&](bool ready, std::uint32_t extra = 0u) {
        return repeat.apply(kSquare | extra, ready, kSquare);
    };
    check(pulse(kCycle * 4u, kSquare, [&] { return mined(true); }) == expected_pulse(kCycle * 4u, 0u),
        "holding □ on a mining point presses, lifts and presses again");
    check(repeat.button() == kSquare, "a mining hold pulses □");
    repeat.reset();
    check(repeat.apply(kSquare | kLead, true, kSquare) == (kSquare | kLead) && !repeat.underway(),
        "L+□ before a hold starts is left to the item bar");
    repeat.reset();
    check(mined(true) == kSquare, "a □ hold starts");
    check(pulse(kCycle, kSquare, [&] { return repeat.apply(kCircle, true, kSquare); }) == expected_pulse(kCycle, 1u) &&
            repeat.underway(),
        "○ alone while mining keeps the □ pulse");

    // △+○ beside a point or a body is an attack. The game gets both buttons.
    constexpr std::uint32_t kTriangle = 0x1000u;
    constexpr std::uint32_t kCross = 0x4000u;
    constexpr std::uint32_t kRight = 0x0200u;
    repeat.reset();
    check(repeat.apply(kCircle | kTriangle, true, kCircle) == (kCircle | kTriangle) && !repeat.underway() &&
            !repeat.centres_stick(),
        "△+○ on a point is not a gather and reaches the game whole");
    repeat.reset();
    check(held(true) == kCircle && repeat.underway(), "a ○ hold is running");
    check(repeat.apply(kCircle | kTriangle, true, kCircle) == (kCircle | kTriangle) && !repeat.underway() &&
            !repeat.centres_stick(),
        "△ pressed during a hold ends it and the chord reaches the game");
    check((held(true) & kCircle) == 0u && repeat.underway(),
        "with ○ still held after the chord, the next hold lifts ○ first");
    repeat.reset();
    check(repeat.apply(kCircle | kCross, true, kCircle) == (kCircle | kCross) && !repeat.underway(),
        "× with ○ is left to the game");
    check(repeat.apply(kSquare | kRight, true, kSquare) == (kSquare | kRight) && !repeat.underway(),
        "R with □ is left to the game");
    check(repeat.apply(kCircle | kSquare, true, kCircle) == kCircle && repeat.underway(),
        "□ is not a chord: ○ with □ on a point still gathers");
}

// The pad read that reaches the game.
void test_pressing_again_on_the_pad() {
    psprecomp::GuestMemory memory(32u * 1024u * 1024u);
    constexpr std::uint32_t gamesys = 0x09100000u;
    constexpr std::uint32_t object = 0x09200000u;
    constexpr std::uint32_t ids = 0x09300000u;
    constexpr std::uint32_t areas = 0x09400000u;
    constexpr std::uint32_t nodes = 0x09500000u;
    constexpr std::uint32_t hunter_list = 0x09600000u;
    constexpr std::uint32_t hunter = 0x09700000u;
    memory.store32(0x089CC558u, gamesys);
    memory.store8(gamesys + 0x6AF10u, 0u);
    memory.store32(0x09CE4808u, 1u);
    memory.store32(0x09CE480Cu, ids);
    memory.store16(ids, 4u);
    memory.store32(0x08A62D9Cu, object);
    memory.store32(object + 96u, areas);
    memory.store32(object + 80u, 0u);
    memory.store32(areas, nodes);
    plant_node(memory, nodes, 0.0f, 0.0f, 0.0f, 30.0f, 2u, 0u);
    memory.store32(0x089CBD04u, hunter_list);
    memory.store32(hunter_list + 2692u, hunter);
    write_float(memory, hunter + 0x200u, 0.0f);
    write_float(memory, hunter + 0x204u, 0.0f);
    write_float(memory, hunter + 0x208u, 0.0f);
    memory.store16(hunter + 0x29Au, 4u);
    plant_words(memory);

    auto &settings = mhfu::settings::current();
    settings.gather_hold = false;
    check(gather_hold(kCircle, memory) == kCircle, "hold to gather off leaves the button alone");
    check(gather_hold(0u, memory) == 0u, "and lets it go");
    settings.gather_hold = true;

    check(gather_hold(kCircle | kSquare, memory) == kCircle && gather_hold_centres_stick(),
        "holding ○ on the point starts a gather, and another button is left off");
    check(gather_hold(kCircle | kLead, memory) == kCircle && gather_hold_centres_stick(),
        "L during the hold does not open the item bar");
    check(gather_hold(kSquare, memory) == kCircle && gather_hold_centres_stick(),
        "□ alone while the hold runs does not reach the game");
    check(pulse(kGatherPulseDown + kGatherPulseUp, kCircle, [&] { return gather_hold(kCircle, memory); }) ==
            expected_pulse(kGatherPulseDown + kGatherPulseUp, 3u),
        "the pulse keeps going on the pad");
    check(gather_hold(0u, memory) == 0u && !gather_hold_centres_stick(), "letting go lets the hunter move");
    check(gather_hold(kCircle, memory) == kCircle, "pressing again starts a new pulse at once");
    write_float(memory, hunter + 0x200u, 80.0f);
    check(gather_hold(kCircle | kSquare, memory) == (kCircle | kSquare) && !gather_hold_centres_stick(),
        "walking off the point leaves the pad as it is");

    // Mining is kind 3 and uses □. The same hold setting pulses □ there.
    write_float(memory, hunter + 0x200u, 0.0f);
    memory.store16(nodes + 20u, 3u);
    check(gather_hold(kSquare | kCircle, memory) == kSquare && gather_hold_centres_stick(),
        "holding □ on a mining point starts a gather, and ○ is left off");
    check(gather_hold(kSquare | kLead, memory) == kSquare && gather_hold_centres_stick(),
        "L during a □ hold does not open the item bar");
    check(pulse(kGatherPulseDown + kGatherPulseUp, kSquare, [&] { return gather_hold(kSquare, memory); }) ==
            expected_pulse(kGatherPulseDown + kGatherPulseUp, 2u),
        "the □ pulse keeps going on a mining point");
    check(gather_hold(kCircle, memory) == kSquare && gather_hold_centres_stick(),
        "○ alone while mining keeps the □ pulse");
    check(gather_hold(0u, memory) == 0u && !gather_hold_centres_stick(), "letting go of □ frees the stick");
    memory.store16(nodes + 20u, 4u);
    check(gather_hold(kSquare, memory) == kSquare, "a bug net point also pulses □");
    check(gather_hold(0u, memory) == 0u, "letting go of the bug net hold");
    check(
        gather_hold(kCircle, memory) == kCircle && !gather_hold_centres_stick(), "○ on a bug net point is left alone");
    memory.store16(nodes + 20u, 0u);
    check(gather_hold(kSquare, memory) == kSquare && !gather_hold_centres_stick(), "□ on a plant point is left alone");
}

// One 32-byte carve record. +16 empty is 0xFFFF; +20 is the id; +24 the stage.
void plant_body(psprecomp::GuestMemory &memory, std::uint32_t address, float x, float y, float z, float radius,
    std::uint16_t id, std::uint16_t stage) {
    write_float(memory, address, x);
    write_float(memory, address + 4u, y);
    write_float(memory, address + 8u, z);
    write_float(memory, address + 12u, radius);
    memory.store16(address + 16u, 1u);
    memory.store16(address + 20u, id);
    memory.store16(address + 24u, stage);
}

void test_carving_a_monster() {
    psprecomp::GuestMemory memory(32u * 1024u * 1024u);
    constexpr std::uint32_t gamesys = 0x09100000u;
    constexpr std::uint32_t object = 0x09200000u;
    constexpr std::uint32_t ids = 0x09300000u;
    constexpr std::uint32_t areas = 0x09400000u;
    constexpr std::uint32_t nodes = 0x09500000u;
    constexpr std::uint32_t hunter_list = 0x09600000u;
    constexpr std::uint32_t hunter = 0x09700000u;
    constexpr std::uint32_t table = 0x09800000u;
    memory.store32(0x089CC558u, gamesys);
    memory.store8(gamesys + 0x6AF10u, 0u);
    memory.store32(0x09CE4808u, 1u);
    memory.store32(0x09CE480Cu, ids);
    memory.store16(ids, 4u);
    memory.store32(0x08A62D9Cu, object);
    memory.store32(object + 96u, areas);
    memory.store32(object + 80u, 0u);
    memory.store32(areas, nodes);
    write_float(memory, nodes, -1.0f); // no gather point
    memory.store32(0x089CBD04u, hunter_list);
    memory.store32(hunter_list + 2692u, hunter);
    write_float(memory, hunter + 0x200u, 0.0f);
    write_float(memory, hunter + 0x204u, 0.0f);
    write_float(memory, hunter + 0x208u, 0.0f);
    memory.store16(hunter + 0x29Au, 4u);
    memory.store32(0x09A4F1A0u, table);
    plant_body(memory, table + 4u, 0.0f, 0.0f, 0.0f, 30.0f, 9u, 4u);
    plant_words(memory);

    check(carve_in_reach(memory), "standing on a monster body can carve it");
    write_float(memory, hunter + 0x200u, 30.1f);
    check(!carve_in_reach(memory), "just past the body's radius is not a carve");
    write_float(memory, hunter + 0x200u, 0.0f);
    write_float(memory, hunter + 0x204u, 100.0f);
    check(!carve_in_reach(memory), "100 above the body is already outside");
    write_float(memory, hunter + 0x204u, -200.0f);
    check(carve_in_reach(memory), "200 below the body is still a carve");
    write_float(memory, hunter + 0x204u, 0.0f);
    memory.store16(hunter + 0x29Au, 9u);
    check(!carve_in_reach(memory), "a body in another area is not a carve");
    memory.store16(hunter + 0x29Au, 4u);
    memory.store16(table + 4u + 16u, 0xFFFFu);
    check(!carve_in_reach(memory), "an empty carve slot is not a body");
    memory.store16(table + 4u + 16u, 1u);
    memory.store16(table + 4u + 20u, 0xFFFFu);
    check(!carve_in_reach(memory), "a body whose id is empty is not carved");
    memory.store16(table + 4u + 20u, 9u);
    memory.store32(hunter + 1164u, 0x100u);
    check(!carve_in_reach(memory), "the carve stays off while the hunter's block bit is set");
    memory.store32(hunter + 1164u, 0u);

    // A later slot is reached when the first is another stage.
    plant_body(memory, table + 4u + 32u, 0.0f, 0.0f, 0.0f, 30.0f, 3u, 4u);
    memory.store16(table + 4u + 24u, 1u);
    check(carve_in_reach(memory), "a miss walks on to the next body");
    memory.store16(table + 4u + 24u, 4u);
    memory.store16(table + 4u + 20u, 0xFFFFu);
    check(!carve_in_reach(memory), "the first body in reach is the one the game would carve");

    auto &settings = mhfu::settings::current();
    settings.gather_hold = false;
    check(gather_hold(0u, memory) == 0u, "turning hold to gather off clears a gather already underway");
    settings.gather_hold = true;
    memory.store16(table + 4u + 20u, 9u);
    memory.store16(table + 4u + 24u, 4u);
    check(gather_hold(kCircle, memory) == kCircle, "holding ○ on a body starts a carve");
    check(pulse(kGatherPulseDown + kGatherPulseUp, kCircle, [&] { return gather_hold(kCircle, memory); }) ==
            expected_pulse(kGatherPulseDown + kGatherPulseUp, 1u),
        "holding ○ on a body keeps carving");
    check(gather_hold(kSquare, memory) == kCircle && gather_hold_centres_stick(),
        "□ alone while carving keeps the ○ pulse");
    check(gather_hold(kCircle | 0x1000u, memory) == (kCircle | 0x1000u) && !gather_hold_centres_stick(),
        "△+○ beside a body is an attack: the pad passes and the stick moves");
    check(gather_hold(kCircle | 0x1000u, memory) == (kCircle | 0x1000u), "and stays an attack while held");
    write_float(memory, hunter + 0x200u, 80.0f);
    check(gather_hold(kCircle, memory) == kCircle, "walking off the body leaves ○ as it is held");
    (void)gather_hold(0u, memory);
}

// The village: lobby_task in the task slot and game_sub beside it, with the
// farm's instructions in place.
void plant_village(psprecomp::GuestMemory &memory) {
    const auto overlay = [&](std::uint32_t slot, const char *name, std::size_t length) {
        memory.store32(slot, 0x336F574Du);
        memory.store32(slot + 8u, slot);
        for (std::size_t i = 0; i <= length; ++i)
            memory.store8(slot + 32u + static_cast<std::uint32_t>(i), static_cast<std::uint8_t>(name[i]));
    };
    overlay(0x09A5F300u, "lobby_task.ovl", 14u);
    overlay(0x09C19100u, "game_sub.ovl", 12u);
    for (const GatherWord &word : farm_checked_words()) memory.store32(word.address, word.instruction);
}

void test_farm_spots() {
    psprecomp::GuestMemory memory(32u * 1024u * 1024u);
    constexpr std::uint32_t gamesys = 0x09100000u;
    constexpr std::uint32_t farm = gamesys + 0x6A238u;
    constexpr std::uint32_t hunter_list = 0x09600000u;
    constexpr std::uint32_t hunter = 0x09700000u;
    constexpr std::uint32_t state = 0x09800000u;
    constexpr std::uint32_t point = 0x09900000u;
    memory.store32(0x089CC558u, gamesys);
    memory.store8(gamesys + 40u, 1u); // the local hunter is slot 1
    memory.store32(0x089CBD04u, hunter_list);
    memory.store32(hunter_list + 2692u, hunter);
    memory.store16(hunter + 486u, 1u);
    memory.store32(0x09B22E70u, state);
    memory.store32(hunter + 1364u, point);
    memory.store16(point + 2u, kFarmMining);
    memory.store16(point + 22u, 1u);
    memory.store8(farm + 2u, 2u);        // two mining spots
    memory.store16(farm + 8u + 8u, 3u);  // the second has three uses
    memory.store8(farm + 3u, 1u);        // one bug thicket
    memory.store16(farm + 48u + 0u, 5u); // with five uses

    check(!farm_point(memory), "nothing before the village overlays are loaded");
    plant_village(memory);
    check(farm_code_matches(memory), "the planted farm words match");
    const std::optional<FarmPoint> mining = farm_point(memory);
    check(
        mining && mining->address == point && mining->type == kFarmMining && mining->index == 1u && mining->uses == 3u,
        "the mining outcrop in reach is found with its uses");
    memory.store16(farm + 8u + 8u, 0u);
    check(!farm_point(memory), "a mined-out outcrop is not a gather");
    memory.store16(farm + 8u + 8u, 3u);
    memory.store16(point + 22u, 2u);
    memory.store16(farm + 8u + 16u, 4u);
    check(farm_point(memory) && farm_point(memory)->uses == 4u, "an index equal to the count is still a spot");
    memory.store16(point + 22u, 3u);
    check(!farm_point(memory), "an index past the count is not");

    memory.store16(point + 2u, kFarmBugs);
    memory.store16(point + 22u, 0u);
    const std::optional<FarmPoint> bugs = farm_point(memory);
    check(bugs && bugs->type == kFarmBugs && bugs->uses == 5u, "a bug thicket reads its uses at +48");
    memory.store16(point + 22u, 1u);
    check(!farm_point(memory), "past the thicket count is not a spot");
    memory.store16(point + 22u, 0u);
    memory.store16(point + 2u, 17u);
    check(!farm_point(memory), "another farm spot is left alone");
    memory.store16(point + 2u, 1u);
    check(!farm_point(memory), "a person is left alone");
    memory.store16(point + 2u, kFarmMining);
    memory.store16(point + 22u, 1u);

    memory.store32(hunter + 1364u, 0u);
    check(!farm_point(memory), "no point in reach is no spot");
    memory.store32(hunter + 1364u, point);
    memory.store32(state, 54u);
    check(!farm_point(memory), "a gather already running is not a new one");
    memory.store32(state, 0u);
    memory.store16(state + 26u, 1u);
    check(!farm_point(memory), "nor while the state object is held");
    memory.store16(state + 26u, 0u);
    memory.store16(hunter + 486u, 0u);
    check(!farm_point(memory), "a hunter that is not the local one is not read");
    memory.store16(hunter + 486u, 1u);
    memory.store32(0x09B0F33Cu, 0u);
    check(!farm_code_matches(memory) && !farm_point(memory), "a differing word turns the farm off");
    plant_village(memory);
    memory.store8(0x09C19100u + 32u, static_cast<std::uint8_t>('d'));
    check(!farm_point(memory), "another sub overlay is not the farm's");
    plant_village(memory);
    check(farm_point(memory).has_value(), "and the farm is back");

    // The hold pulses □ on the outcrop, as on a quest's mining point.
    auto &settings = mhfu::settings::current();
    settings.gather_hold = true;
    check(gather_hold(kSquare | kCircle, memory) == kSquare && gather_hold_centres_stick(),
        "holding □ at the outcrop starts a gather, and ○ is left off");
    check(pulse(kGatherPulseDown + kGatherPulseUp, kSquare, [&] { return gather_hold(kSquare, memory); }) ==
            expected_pulse(kGatherPulseDown + kGatherPulseUp, 1u),
        "the □ pulse keeps going at the outcrop");
    // While the swing runs the state object is not 0, so the pad goes
    // through as held and the game ignores it.
    memory.store32(state, 54u);
    check(gather_hold(kSquare, memory) == kSquare && !gather_hold_centres_stick(),
        "during the swing □ reaches the game as held");
    memory.store32(state, 0u);
    check(gather_hold(kSquare, memory) == 0u && gather_hold_centres_stick(),
        "after the swing the hold lifts □ first, so the next read is a new press");
    check(pulse(kGatherPulseDown + kGatherPulseUp - 1u, kSquare, [&] { return gather_hold(kSquare, memory); }) ==
            expected_pulse(kGatherPulseDown + kGatherPulseUp - 1u, kGatherPulseDown + 1u),
        "and the pulse carries on into the next press");
    check(gather_hold(0u, memory) == 0u && !gather_hold_centres_stick(), "letting go of □ frees the stick");
    check(gather_hold(kCircle, memory) == kCircle && !gather_hold_centres_stick(), "○ at the outcrop is left alone");
    (void)gather_hold(0u, memory);
    memory.store16(farm + 8u + 8u, 0u);
    check(gather_hold(kSquare, memory) == kSquare && !gather_hold_centres_stick(),
        "□ at a mined-out outcrop is left alone");
    (void)gather_hold(0u, memory);
    memory.store16(farm + 8u + 8u, 3u);
    settings.gather_hold = false;
    check(gather_hold(kSquare, memory) == kSquare, "hold to gather off leaves □ alone at the farm");
    settings.gather_hold = true;
    (void)gather_hold(0u, memory);
}

// The gather mark also stands over a body or a cut tail ○ would carve.
void test_mark_over_a_carve() {
    psprecomp::GuestMemory memory(32u * 1024u * 1024u);
    plant_words(memory);
    constexpr std::uint32_t gamesys = 0x09100000u;
    constexpr std::uint32_t object = 0x09200000u;
    constexpr std::uint32_t ids = 0x09300000u;
    constexpr std::uint32_t areas = 0x09400000u;
    constexpr std::uint32_t nodes = 0x09500000u;
    constexpr std::uint32_t hunter_list = 0x09600000u;
    constexpr std::uint32_t hunter = 0x09700000u;
    constexpr std::uint32_t table = 0x09800000u;
    memory.store32(0x089CC558u, gamesys);
    memory.store8(gamesys + 0x6AF10u, 0u);
    memory.store32(0x09CE4808u, 1u);
    memory.store32(0x09CE480Cu, ids);
    memory.store16(ids, 4u);
    memory.store32(0x08A62D9Cu, object);
    memory.store32(object + 96u, areas);
    memory.store32(object + 80u, 0u);
    memory.store32(areas, nodes);
    write_float(memory, nodes, -1.0f); // no gather point
    memory.store32(0x089CBD04u, hunter_list);
    memory.store32(hunter_list + 2692u, hunter);
    write_float(memory, hunter + 0x200u, 0.0f);
    write_float(memory, hunter + 0x204u, 0.0f);
    write_float(memory, hunter + 0x208u, -10.0f);
    memory.store16(hunter + 0x29Au, 4u);
    memory.store32(0x09A4F1A0u, table);
    // Straight ahead of the camera, so its mark is the picture's centre.
    plant_body(memory, table + 4u, 0.0f, 0.0f, -10.0f, 30.0f, 9u, 4u);

    const std::optional<CarveBody> body = carve_body(memory);
    check(body && body->address == table + 4u && body->x == 0.0f && body->y == 0.0f && body->z == -10.0f,
        "the carve search gives the body's record and position");
    mhfu::settings::current().gather_icon = true;
    gather_note_draw(picture(), 100u);
    const GatherPose pose{0.0f, 100.0f, 0.0f, 0.0f, 100.0f, -10.0f};
    const std::optional<GatherMark> at = gather_mark(memory, pose);
    check(at && std::fabs(at->at[0] - 0.5f) < 0.05f && std::fabs(at->at[1] - 0.5f) < 0.05f,
        "with no gather point in range, the mark stands over the body that can be carved");

    // A gather point in range comes first, as in the game's circle handler.
    plant_node(memory, nodes, 5.0f, 0.0f, -10.0f, 30.0f, 1u, 0u);
    gather_note_draw(picture(), 100u);
    const std::optional<GatherMark> gather_first = gather_mark(memory, pose);
    check(gather_first && gather_first->at[0] > 0.55f, "a gather point in range takes the mark before the body");
    check(at && at->glyph == GatherGlyph::Carve, "a carve is shown as one");
    check(gather_first && gather_first->glyph == GatherGlyph::Plant, "a kind 0 point is a plant");
    plant_node(memory, nodes, 5.0f, 0.0f, -10.0f, 30.0f, 1u, 3u);
    gather_note_draw(picture(), 100u);
    const std::optional<GatherMark> mining = gather_mark(memory, pose);
    check(mining && mining->glyph == GatherGlyph::Mining, "a mining point is shown as mining");
    write_float(memory, nodes, -1.0f);

    memory.store16(table + 4u + 20u, 0xFFFFu);
    gather_note_draw(picture(), 100u);
    check(!carve_body(memory) && !gather_mark(memory, pose), "a body with nothing left to carve is not marked");
    memory.store16(table + 4u + 20u, 9u);
    memory.store32(hunter + 1164u, 0x100u);
    gather_note_draw(picture(), 100u);
    check(!gather_mark(memory, pose), "nor one the hunter cannot carve now");
    memory.store32(hunter + 1164u, 0u);
    write_float(memory, hunter + 0x200u, 40.0f);
    gather_note_draw(picture(), 100u);
    check(!gather_mark(memory, pose), "nor one out of reach");
}

// One 24-byte spot: +2 the type, +4 x, +8 y, +12 z, +16 the radius.
void plant_spot(psprecomp::GuestMemory &memory, std::uint32_t address, std::uint16_t type, float x, float y, float z,
    float radius) {
    memory.store16(address + 2u, type);
    write_float(memory, address + 4u, x);
    write_float(memory, address + 8u, y);
    write_float(memory, address + 12u, z);
    write_float(memory, address + 16u, radius);
}

// The quest object behind 0x09A4F1A0, its stage object, and the stage's
// method +40 returning the spot table: lui v0,0x0985; jr ra; addiu v0,v0,-0x7F00.
constexpr std::uint32_t kSpotTable = 0x09848100u;
constexpr std::uint32_t kSpots = 0x09860000u;

void plant_stage(psprecomp::GuestMemory &memory) {
    constexpr std::uint32_t quest = 0x09800000u;
    constexpr std::uint32_t stage = 0x09810000u;
    constexpr std::uint32_t vtable = 0x09820000u;
    constexpr std::uint32_t method = 0x09830000u;
    memory.store32(0x09A4F1A0u, quest);
    memory.store32(quest, stage);
    memory.store32(stage, vtable);
    memory.store32(vtable + 40u, method);
    memory.store32(method, 0x3C020985u);
    memory.store32(method + 4u, 0x03E00008u);
    memory.store32(method + 8u, 0x24428100u);
    memory.store32(kSpotTable + 16u, kSpots);
    memory.store8(kSpotTable + 62u, 2u);
}

void plant_fishing_words(psprecomp::GuestMemory &memory) {
    for (const GatherWord &word : fishing_checked_words()) memory.store32(word.address, word.instruction);
}

void test_fishing_spots() {
    psprecomp::GuestMemory memory(32u * 1024u * 1024u);
    constexpr std::uint32_t gamesys = 0x09100000u;
    constexpr std::uint32_t object = 0x09200000u;
    constexpr std::uint32_t ids = 0x09300000u;
    constexpr std::uint32_t areas = 0x09400000u;
    constexpr std::uint32_t nodes = 0x09500000u;
    constexpr std::uint32_t hunter_list = 0x09600000u;
    constexpr std::uint32_t hunter = 0x09700000u;
    memory.store32(0x089CC558u, gamesys);
    memory.store8(gamesys + 0x6AF10u, 0u);
    memory.store32(0x09CE4808u, 1u);
    memory.store32(0x09CE480Cu, ids);
    memory.store16(ids, 4u);
    memory.store32(0x08A62D9Cu, object);
    memory.store32(object + 96u, areas);
    memory.store32(object + 80u, 0u);
    memory.store32(areas, nodes);
    write_float(memory, nodes, -1.0f); // no gather point
    memory.store32(0x089CBD04u, hunter_list);
    memory.store32(hunter_list + 2692u, hunter);
    write_float(memory, hunter + 0x200u, 100.0f);
    write_float(memory, hunter + 0x204u, -40.0f);
    write_float(memory, hunter + 0x208u, 0.0f);
    memory.store16(hunter + 0x29Au, 4u);
    plant_stage(memory);
    plant_spot(memory, kSpots, 24u, 5000.0f, 0.0f, 5000.0f, 200.0f); // a supply box far off
    plant_spot(memory, kSpots + 24u, kFishingSpotType, 0.0f, -50.0f, 0.0f, 100.0f);

    const std::optional<std::uint32_t> table = spot_table(memory);
    check(table && *table == kSpotTable, "the stage's getter gives the spot table, its low half signed");
    check(!fishing_spot(memory), "without the game's instructions there is no fishing spot");
    plant_words(memory);
    check(!fishing_spot(memory), "nor with only the gather search's");
    plant_fishing_words(memory);
    const std::optional<FishingSpot> spot = fishing_spot(memory);
    check(spot && spot->address == kSpots + 24u && spot->x == 0.0f && spot->y == -50.0f && spot->z == 0.0f,
        "the hunter at the water's edge is at the fishing spot");

    write_float(memory, hunter + 0x200u, 100.1f);
    check(!fishing_spot(memory), "just past the radius is not");
    write_float(memory, hunter + 0x200u, 0.0f);
    write_float(memory, hunter + 0x204u, -100.0f);
    check(fishing_spot(memory).has_value(), "50 below the spot is still in reach");
    write_float(memory, hunter + 0x204u, -100.1f);
    check(!fishing_spot(memory), "and lower is not");
    write_float(memory, hunter + 0x204u, -0.1f);
    check(fishing_spot(memory).has_value(), "just under 50 above it is in reach");
    write_float(memory, hunter + 0x204u, 0.0f);
    check(!fishing_spot(memory), "50 above it is already outside");
    write_float(memory, hunter + 0x204u, -40.0f);

    // The search gives the first spot in reach, and the bait fishes only at 23.
    plant_spot(memory, kSpots, 24u, 0.0f, -50.0f, 0.0f, 200.0f);
    const std::optional<GatherSpot> first = spot_from_table(memory, kSpotTable, 0.0f, -40.0f, 0.0f);
    check(first && first->address == kSpots && first->type == 24u, "the first spot in reach is returned");
    check(!fishing_spot(memory), "so a camp spot before the water is no fishing");
    plant_spot(memory, kSpots, 24u, 5000.0f, 0.0f, 5000.0f, 200.0f);
    memory.store8(kSpotTable + 62u, 1u);
    check(!fishing_spot(memory), "a spot past the count is not read");
    memory.store8(kSpotTable + 62u, 0x80u);
    check(!fishing_spot(memory), "a negative count is no spots");
    memory.store8(kSpotTable + 62u, 2u);
    memory.store32(kSpotTable + 16u, 0u);
    check(!fishing_spot(memory), "an area without a list has no spots");
    memory.store32(kSpotTable + 16u, kSpots);
    memory.store32(kSpots + 24u + 8u, 0x7FC00000u);
    check(!fishing_spot(memory), "a height that is not a number is skipped");
    write_float(memory, kSpots + 24u + 8u, -50.0f);
    check(fishing_spot(memory).has_value(), "and the spot is back");

    memory.store32(0x09830000u + 4u, 0u);
    check(!spot_table(memory) && !fishing_spot(memory), "a method that is not the getter is not followed");
    memory.store32(0x09830000u + 4u, 0x03E00008u);
    memory.store32(0x09800000u, 0u);
    check(!spot_table(memory), "no stage object is no table");
    memory.store32(0x09800000u, 0x09810000u);
    memory.store32(fishing_checked_words()[4].address, 0u);
    check(!fishing_spot(memory), "a differing spot instruction turns fishing off");
    check(gather_code_matches(memory), "and leaves gathering on");
    plant_fishing_words(memory);

    // The mark: the bait's icon, below a gather point and a carve.
    for (const GatherArtWord &word : gather_art_words()) memory.store32(word.address, word.instruction);
    mhfu::settings::current().gather_icon = true;
    // Straight ahead of the camera, so the mark is the picture's centre.
    write_float(memory, hunter + 0x204u, 0.0f);
    write_float(memory, hunter + 0x208u, -10.0f);
    plant_spot(memory, kSpots + 24u, kFishingSpotType, 0.0f, 0.0f, -10.0f, 100.0f);
    const GatherPose pose{0.0f, 100.0f, 0.0f, 0.0f, 100.0f, -10.0f};
    gather_note_draw(picture(), 100u);
    const std::optional<GatherMark> mark = gather_mark(memory, pose);
    check(mark && mark->glyph == GatherGlyph::Fishing && mark->art == GatherArt::of_item(kBaitItem),
        "a fishing spot is marked with the bait");
    check(mark && std::fabs(mark->at[0] - 0.5f) < 0.05f && std::fabs(mark->at[1] - 0.5f) < 0.05f &&
            mark->world[2] == -10.0f,
        "over the spot");
    plant_node(memory, nodes, 0.0f, 0.0f, -10.0f, 30.0f, 1u, 0u);
    gather_note_draw(picture(), 100u);
    const std::optional<GatherMark> gather_first = gather_mark(memory, pose);
    check(gather_first && gather_first->glyph == GatherGlyph::Plant, "a gather point in range takes the mark first");
    write_float(memory, nodes, -1.0f);
}

void test_glyphs() {
    check(gather_glyph(0u) == GatherGlyph::Plant, "kind 0 is a plant");
    check(gather_glyph(3u) == GatherGlyph::Mining, "kind 3 is mining");
    check(gather_glyph(4u) == GatherGlyph::BugNet, "kind 4 is a bug net");
    check(gather_glyph(7u) == GatherGlyph::Plant, "a kind ○ gathers is shown as a plant");
}

GatherTarget target_at(std::uint32_t address, float x) {
    return GatherTarget{address, x, 0.0f, 0.0f, GatherGlyph::Plant};
}

void test_motion() {
    GatherMotion motion;
    check(!motion.shown(), "nothing is shown before a target");
    motion.step(target_at(1u, 0.0f));
    std::optional<GatherMotion::Shown> shown = motion.shown();
    check(shown && near(shown->alpha, GatherMotion::kFadeStep) && shown->x == 0.0f,
        "a new target starts on its place and fades in");
    for (int i = 0; i < 10; ++i) motion.step(target_at(1u, 0.0f));
    check(motion.shown() && near(motion.shown()->alpha, 1.0f), "and stays at full opacity");

    // A target close by is glided to, half the way each frame, then reached.
    motion.step(GatherTarget{2u, 100.0f, 0.0f, 0.0f, GatherGlyph::Mining});
    shown = motion.shown();
    check(shown && near(shown->x, 50.0f) && shown->glyph == GatherGlyph::Mining,
        "a near target is glided to and its kind shown at once");
    for (int i = 0; i < 12; ++i) motion.step(target_at(2u, 100.0f));
    check(motion.shown() && motion.shown()->x == 100.0f, "the glide ends on the target");

    // A far one is jumped to.
    motion.step(target_at(3u, 100.0f + GatherMotion::kJump + 1.0f));
    check(motion.shown() && motion.shown()->x == 100.0f + GatherMotion::kJump + 1.0f, "a far target is jumped to");

    // Losing the target fades the mark out where it was.
    const float place = motion.shown()->x;
    motion.step(std::nullopt);
    shown = motion.shown();
    check(shown && near(shown->alpha, 1.0f - GatherMotion::kFadeStep) && shown->x == place,
        "with no target the mark fades out in place");
    // A target found again while fading glides from there and fades back in.
    motion.step(target_at(4u, place + 20.0f));
    shown = motion.shown();
    check(shown && near(shown->alpha, 1.0f) && near(shown->x, place + 10.0f),
        "a target found while fading is glided to from the fading mark");
    for (int i = 0; i < 4; ++i) motion.step(std::nullopt);
    check(!motion.shown(), "four frames without a target and the mark is gone");
    motion.step(target_at(5u, -300.0f));
    shown = motion.shown();
    check(shown && shown->x == -300.0f && near(shown->alpha, GatherMotion::kFadeStep),
        "once gone, the next target starts on its own place");
}

void test_motion_on_the_picture() {
    psprecomp::GuestMemory memory(32u * 1024u * 1024u);
    plant_words(memory);
    constexpr std::uint32_t gamesys = 0x09100000u;
    constexpr std::uint32_t object = 0x09200000u;
    constexpr std::uint32_t ids = 0x09300000u;
    constexpr std::uint32_t areas = 0x09400000u;
    constexpr std::uint32_t nodes = 0x09500000u;
    constexpr std::uint32_t hunter_list = 0x09600000u;
    constexpr std::uint32_t hunter = 0x09700000u;
    memory.store32(0x089CC558u, gamesys);
    memory.store8(gamesys + 0x6AF10u, 0u);
    memory.store32(0x09CE4808u, 1u);
    memory.store32(0x09CE480Cu, ids);
    memory.store16(ids, 1u);
    memory.store32(0x08A62D9Cu, object);
    memory.store32(object + 96u, areas);
    memory.store32(object + 80u, 0u);
    memory.store32(areas, nodes);
    plant_node(memory, nodes, 0.0f, 0.0f, -10.0f, 5.0f, 1u, 4u);
    memory.store32(0x089CBD04u, hunter_list);
    memory.store32(hunter_list + 2692u, hunter);
    write_float(memory, hunter + 0x200u, 0.0f);
    write_float(memory, hunter + 0x204u, 0.0f);
    write_float(memory, hunter + 0x208u, -10.0f);
    memory.store16(hunter + 0x29Au, 1u);
    mhfu::settings::current().gather_icon = true;

    GatherMotion motion;
    const GatherPose pose{0.0f, 100.0f, 0.0f, 0.0f, 100.0f, -10.0f};
    gather_note_draw(picture(), 100u);
    const std::optional<GatherMark> first = gather_mark(memory, pose, &motion);
    check(first && !first->older && near(first->alpha, GatherMotion::kFadeStep) && first->glyph == GatherGlyph::BugNet,
        "the first frame fades the mark in, with no older place to blend from");
    // The camera turns a little: the next frame blends from the first one's place.
    const GatherPose turned{0.0f, 100.0f, 0.0f, 1.0f, 100.0f, -10.0f};
    gather_note_draw(picture(), 100u);
    const std::optional<GatherMark> second = gather_mark(memory, turned, &motion);
    check(first && second && second->older && near((*second->older)[0], first->at[0]) &&
            near((*second->older)[1], first->at[1]) && second->at[0] != first->at[0],
        "the next frame keeps where the last one drew the mark");

    // The hunter walks off: the mark fades out where it was.
    write_float(memory, hunter + 0x200u, 40.0f);
    gather_note_draw(picture(), 100u);
    const std::optional<GatherMark> leaving = gather_mark(memory, turned, &motion);
    check(leaving && second && near(leaving->alpha, second->alpha - GatherMotion::kFadeStep) &&
            near(leaving->at[0], second->at[0]),
        "out of reach, the mark fades in place");

    // A frame with no world draw (a menu, a load) ends the mark.
    check(!gather_mark(memory, turned, &motion) && !motion.shown(), "a frame without the world resets the motion");
    gather_note_draw(picture(), 100u);
    check(!gather_mark(memory, turned, &motion), "and nothing is left to fade");
}

void test_ui_shift() {
    mhfu::gpu::ui_motion::Shift shift{{10.0f, -4.0f}, {}};
    std::array<float, 2> move = shift.to(0.0f);
    check(near(move[0], 10.0f) && near(move[1], -4.0f), "weight 0 moves the piece to the older frame's place");
    move = shift.to(0.5f);
    check(near(move[0], -5.0f) && near(move[1], 2.0f), "halfway moves it back by half");
    move = shift.to(0.5f);
    check(move[0] == 0.0f && move[1] == 0.0f, "the same weight again does not move it");
    move = shift.to(2.0f);
    check(near(move[0], -5.0f) && near(move[1], 2.0f) && shift.applied[0] == 0.0f && shift.applied[1] == 0.0f,
        "weight 1 (or past it) puts it back where it was built");
    move = shift.to(-1.0f);
    check(near(move[0], 10.0f), "a weight below 0 is the older frame");
}

// A point's list (game/gather_art.hpp): base is the gather object's +80, and
// the word at 0x08A5C584 is where the point offsets start past it.
constexpr std::uint32_t kArtObject = 0x09200000u;
constexpr std::uint32_t kArtBase = 0x09800000u;
constexpr std::uint32_t kArtOffsets = 0x100u;

void plant_list(psprecomp::GuestMemory &memory, std::uint16_t id, std::uint32_t entry,
    std::initializer_list<std::array<std::uint16_t, 2>> pairs) {
    memory.store32(0x08A62D9Cu, kArtObject);
    memory.store32(kArtObject + 80u, kArtBase);
    memory.store32(0x08A5C584u, kArtOffsets);
    memory.store32(kArtBase + kArtOffsets + static_cast<std::uint32_t>(id) * 4u, entry);
    std::uint32_t at = kArtBase + entry;
    for (const std::array<std::uint16_t, 2> &pair : pairs) {
        memory.store16(at, pair[0]);
        memory.store16(at + 2u, pair[1]);
        at += 4u;
    }
    memory.store16(at, 0xFFFFu);
}

// An item with an icon: its record, the icon's sprite, the rectangle and the tint.
void plant_item(psprecomp::GuestMemory &memory, std::uint16_t item, std::uint8_t icon, std::uint8_t tint,
    std::uint16_t sprite, std::array<std::uint16_t, 4> rect, std::array<std::uint8_t, 3> colour) {
    const std::uint32_t record = 0x0899D2BCu + static_cast<std::uint32_t>(item) * 24u;
    memory.store8(record + 1u, icon);
    memory.store8(record + 2u, tint);
    memory.store16(0x0893ABECu + static_cast<std::uint32_t>(icon) * 2u, sprite);
    const std::uint32_t at = 0x089392C8u + static_cast<std::uint32_t>(sprite) * 12u;
    for (std::uint32_t i = 0; i < 4u; ++i) memory.store16(at + i * 2u, rect[i]);
    const std::uint32_t rgb = 0x0893AB7Cu + static_cast<std::uint32_t>(tint) * 4u;
    for (std::uint32_t i = 0; i < 3u; ++i) memory.store8(rgb + i, colour[i]);
}

void test_gather_items() {
    psprecomp::GuestMemory memory(32u * 1024u * 1024u);
    check(!gather_item(memory, 3u), "no gather object, no item");
    plant_list(memory, 3u, 0x400u, {{10u, 7u}, {60u, 5u}, {60u, 9u}, {20u, 11u}});
    const std::optional<std::uint16_t> item = gather_item(memory, 3u);
    check(item && *item == 5u, "the heaviest item is shown, the first of equal weights");
    plant_list(memory, 4u, 0x800u, {});
    check(!gather_item(memory, 4u), "an empty list has no item");
    // A list that never ends is not read past 64 pairs.
    memory.store32(kArtBase + kArtOffsets + 5u * 4u, 0xC00u);
    for (std::uint32_t i = 0; i < 70u; ++i) memory.store32(kArtBase + 0xC00u + i * 4u, 0x00010001u);
    check(!gather_item(memory, 5u), "a list with no end is no item");

    check(gather_art_for(memory, 3u, 3u) == GatherArt::of_item(kPickaxeItem), "a mining point shows the pickaxe");
    check(gather_art_for(memory, 4u, 3u) == GatherArt::of_item(kBugNetItem), "a bug point shows the bug net");
    check(gather_art_for(memory, 0u, 3u) == GatherArt::of_item(5u), "a ○ point shows what it gives");
    check(gather_art_for(memory, 0u, 4u).kind == GatherArt::Kind::None, "and nothing when it gives nothing");

    // A bone pile as traced: mostly a husk (icon 28), then two bones (icon 10).
    const auto icon_of = [&](std::uint16_t item, std::uint8_t icon, std::uint8_t tint) {
        const std::uint32_t record = 0x0899D2BCu + static_cast<std::uint32_t>(item) * 24u;
        memory.store8(record + 1u, icon);
        memory.store8(record + 2u, tint);
    };
    icon_of(94u, kHuskIcon, 0u);
    icon_of(292u, kBoneIcon, 0u);
    icon_of(290u, kBoneIcon, 4u);
    plant_list(memory, 27u, 0x1400u, {{60u, 94u}, {30u, 292u}, {10u, 290u}});
    check(gather_art_for(memory, 0u, 27u) == GatherArt::of_item(kBoneItem), "a bone pile shows the bone, not the husk");
    plant_list(memory, 28u, 0x1800u, {{50u, 94u}});
    check(gather_art_for(memory, 0u, 28u) == GatherArt::of_item(kBoneItem), "so does a pile of husks alone");
    plant_list(memory, 29u, 0x1C00u, {{40u, 290u}, {10u, 7u}});
    check(gather_art_for(memory, 0u, 29u) == GatherArt::of_item(kBoneItem), "and one whose bone is not plain");
}

void test_item_art() {
    psprecomp::GuestMemory memory(32u * 1024u * 1024u);
    plant_item(memory, 145u, 16u, 0u, 90u, {24u, 48u, 24u, 24u}, {0xEFu, 0xEFu, 0xE9u});
    const std::optional<ItemArt> pick = item_art(memory, 145u);
    check(pick && pick->u == 24u && pick->v == 48u && pick->width == 24u && pick->height == 24u && pick->red == 0xEFu &&
            pick->blue == 0xE9u,
        "an item's icon is its sprite's rectangle in its tint");
    plant_item(memory, 7u, 255u, 0u, 0u, {}, {});
    check(!item_art(memory, 7u), "icon 255 is none");
    plant_item(memory, 8u, 20u, 2u, 91u, {240u, 0u, 24u, 24u}, {1u, 2u, 3u});
    check(!item_art(memory, 8u), "a rectangle past the atlas is none");
    plant_item(memory, 9u, 21u, 11u, 92u, {0u, 0u, 24u, 24u}, {1u, 2u, 3u});
    check(!item_art(memory, 9u), "a tint past the palette is none");
    check(!item_art(memory, 1500u), "nor an id past the table");

    check(!gather_art_code_matches(memory), "the lookups need their code");
    for (const GatherArtWord &word : gather_art_words()) memory.store32(word.address, word.instruction);
    check(gather_art_code_matches(memory), "with every word, they run");
    memory.store32(gather_art_words()[2].address, 0u);
    check(!gather_art_code_matches(memory), "a word that differs stops them");
}

void test_mark_shows_art() {
    psprecomp::GuestMemory memory(32u * 1024u * 1024u);
    plant_words(memory);
    for (const GatherArtWord &word : gather_art_words()) memory.store32(word.address, word.instruction);
    constexpr std::uint32_t gamesys = 0x09100000u;
    constexpr std::uint32_t ids = 0x09300000u;
    constexpr std::uint32_t areas = 0x09400000u;
    constexpr std::uint32_t hunter_list = 0x09600000u;
    constexpr std::uint32_t hunter = 0x09700000u;
    memory.store32(0x089CC558u, gamesys);
    memory.store8(gamesys + 0x6AF10u, 0u);
    memory.store32(0x09CE4808u, 1u);
    memory.store32(0x09CE480Cu, ids);
    memory.store16(ids, 1u);
    plant_list(memory, 6u, 0x400u, {{80u, 41u}});
    // The area list sits past the base, which +80 also adds to the area pointer.
    memory.store32(kArtObject + 96u, areas);
    memory.store32(areas, 0x1000u);
    const std::uint32_t nodes = kArtBase + 0x1000u;
    plant_node(memory, nodes, 0.0f, 0.0f, -10.0f, 5.0f, 6u, 0u);
    memory.store32(0x089CBD04u, hunter_list);
    memory.store32(hunter_list + 2692u, hunter);
    write_float(memory, hunter + 0x208u, -10.0f);
    memory.store16(hunter + 0x29Au, 1u);
    mhfu::settings::current().gather_icon = true;

    gather_note_draw(picture(), 100u);
    GatherMotion motion;
    const std::optional<GatherMark> mark =
        gather_mark(memory, GatherPose{0.0f, 100.0f, 0.0f, 0.0f, 100.0f, -10.0f}, &motion);
    check(mark && mark->art == GatherArt::of_item(41u), "the mark carries the item the point gives");
    check(mark && mark->world[0] == 0.0f && mark->world[2] == -10.0f, "and the point it stands over");

    // Mining shows the pickaxe whatever the list says.
    memory.store16(nodes + 20u, 3u);
    gather_note_draw(picture(), 100u);
    const std::optional<GatherMark> mining =
        gather_mark(memory, GatherPose{0.0f, 100.0f, 0.0f, 0.0f, 100.0f, -10.0f}, &motion);
    check(mining && mining->art == GatherArt::of_item(kPickaxeItem), "a mining point's mark is the pickaxe");
}

} // namespace

int main() {
    test_gather_items();
    test_item_art();
    test_mark_shows_art();
    test_glyphs();
    test_motion();
    test_motion_on_the_picture();
    test_ui_shift();
    test_the_radius();
    test_the_area_list();
    test_unmapped();
    test_words_and_the_hunter();
    test_on_the_picture();
    test_size_and_thickness();
    test_holding_circle();
    test_pressing_again_on_the_pad();
    test_carving_a_monster();
    test_farm_spots();
    test_mark_over_a_carve();
    test_fishing_spots();
    std::cout << (failures ? "FAIL" : "PASS") << ": gather icon (" << failures << " failures)\n";
    return failures ? 1 : 0;
}
