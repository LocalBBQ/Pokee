// Gather marks: the game's own radius, the area list, and the picture.
// No game data. Guest memory is a stand-in.
#include "game/gather_icon.hpp"
#include "gpu/ui_motion.hpp"
#include "input/button_face.hpp"
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
    const std::optional<GatherMark> at =
        gather_mark(memory, GatherPose{0.0f, 100.0f, 0.0f, 0.0f, 100.0f, -10.0f});
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
    check(at && at->glyph == GatherGlyph::Carve && at->button == kGatherCircle, "a carve is ○ and shown as one");
    check(gather_first && gather_first->glyph == GatherGlyph::Plant && gather_first->button == kGatherCircle,
        "a kind 0 point is a plant, gathered with ○");
    plant_node(memory, nodes, 5.0f, 0.0f, -10.0f, 30.0f, 1u, 3u);
    gather_note_draw(picture(), 100u);
    const std::optional<GatherMark> mining = gather_mark(memory, pose);
    check(mining && mining->glyph == GatherGlyph::Mining && mining->button == kGatherSquare,
        "a mining point is gathered with □");
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

void test_glyphs() {
    check(gather_glyph(0u) == GatherGlyph::Plant, "kind 0 is a plant");
    check(gather_glyph(3u) == GatherGlyph::Mining, "kind 3 is mining");
    check(gather_glyph(4u) == GatherGlyph::BugNet, "kind 4 is a bug net");
    check(gather_glyph(7u) == GatherGlyph::Plant, "a kind ○ gathers is shown as a plant");
}

GatherTarget target_at(std::uint32_t address, float x) {
    return GatherTarget{address, x, 0.0f, 0.0f, GatherGlyph::Plant, kGatherCircle};
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
    motion.step(GatherTarget{2u, 100.0f, 0.0f, 0.0f, GatherGlyph::Mining, kGatherSquare});
    shown = motion.shown();
    check(shown && near(shown->x, 50.0f) && shown->glyph == GatherGlyph::Mining && shown->button == kGatherSquare,
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
    check(first && !first->older && near(first->alpha, GatherMotion::kFadeStep) &&
            first->glyph == GatherGlyph::BugNet && first->button == kGatherSquare,
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

void test_button_face() {
    using namespace mhfu::input;
    Bindings bindings{};
    Slots &circle = bindings[static_cast<std::size_t>(Action::Circle)];
    circle[0] = chord(pad(PadInput::LeftShoulder), pad(PadInput::West));
    circle[1] = single(pad(PadInput::East));
    check(pressed_for(bindings, Action::Circle, true, false) == pad(PadInput::East),
        "a single button is preferred to a chord before it");
    check(pressed_for(bindings, Action::Circle, true, true) == pad(PadInput::South),
        "the confirm swap moves the right button to the bottom one");
    circle[1] = {};
    check(pressed_for(bindings, Action::Circle, true, false) == pad(PadInput::LeftShoulder),
        "with only a chord, its first input");
    check(pressed_for(bindings, Action::Square, true, false) == kNone, "nothing bound is nothing");
    Slots &square = bindings[static_cast<std::size_t>(Action::Square)];
    square[0] = single(key(4u));
    check(pressed_for(bindings, Action::Square, false, true) == key(4u), "a key is not swapped");

    ButtonFace face = button_face(pad(PadInput::East), PadStyle::Xbox);
    check(face.shape == ButtonFace::Shape::Letter && face.text == "B" && face.red > face.blue,
        "○ on an Xbox pad is a red B");
    face = button_face(pad(PadInput::West), PadStyle::Xbox);
    check(face.shape == ButtonFace::Shape::Letter && face.text == "X" && face.blue > face.red,
        "□ on an Xbox pad is a blue X");
    check(button_face(pad(PadInput::East), PadStyle::PlayStation).shape == ButtonFace::Shape::Circle,
        "a PlayStation pad shows the circle");
    check(button_face(pad(PadInput::West), PadStyle::PlayStation).shape == ButtonFace::Shape::Square,
        "and the square");
    check(button_face(pad(PadInput::South), PadStyle::Generic).shape == ButtonFace::Shape::Cross,
        "an unknown pad takes the PSP symbols");
    check(button_face(pad(PadInput::North), PadStyle::Generic).shape == ButtonFace::Shape::Triangle,
        "the triangle as well");
    check(button_face(pad(PadInput::East), PadStyle::Nintendo).text == "A", "Nintendo's right button is A");
    check(button_face(pad(PadInput::West), PadStyle::Nintendo).text == "Y", "and its left one is Y");
    face = button_face(pad(PadInput::LeftShoulder), PadStyle::Xbox);
    check(face.shape == ButtonFace::Shape::Label && face.text == label(pad(PadInput::LeftShoulder), PadStyle::Xbox),
        "another pad input is labelled as the bindings menu labels it");
    face = button_face(key(4u), PadStyle::Generic);
    check(face.shape == ButtonFace::Shape::Label && face.text == name(key(4u)), "a key is shown by its name");
    check(button_face(kNone, PadStyle::Xbox).text.empty(), "nothing bound has no label");
}

} // namespace

int main() {
    test_glyphs();
    test_motion();
    test_motion_on_the_picture();
    test_ui_shift();
    test_button_face();
    test_the_radius();
    test_the_area_list();
    test_unmapped();
    test_words_and_the_hunter();
    test_on_the_picture();
    test_size_and_thickness();
    test_holding_circle();
    test_pressing_again_on_the_pad();
    test_carving_a_monster();
    test_mark_over_a_carve();
    std::cout << (failures ? "FAIL" : "PASS") << ": gather icon (" << failures << " failures)\n";
    return failures ? 1 : 0;
}
