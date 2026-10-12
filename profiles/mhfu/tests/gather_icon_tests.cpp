// Gather marks: the game's own radius, the area list, and the picture.
// No game data. Guest memory is a stand-in.
#include "game/gather_icon.hpp"
#include "settings/settings.hpp"

#include "psprecomp/guest_memory.hpp"

#include <bit>
#include <cmath>
#include <iostream>

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
    check(gather_from_list(memory, list, 10.0f, -200.0f, 20.0f) &&
            !gather_from_list(memory, list, 10.0f, -200.1f, 20.0f),
        "200 below the node is the low edge");
    check(gather_from_list(memory, list, 10.0f, 99.9f, 20.0f) &&
            !gather_from_list(memory, list, 10.0f, 100.0f, 20.0f),
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
    const std::optional<std::array<float, 2>> at =
        gather_mark(memory, GatherPose{0.0f, 100.0f, 0.0f, 0.0f, 100.0f, -10.0f});
    check(at && std::fabs((*at)[0] - 0.5f) < 0.05f && std::fabs((*at)[1] - 0.5f) < 0.05f,
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

void test_holding_circle() {
    GatherRepeat repeat;
    const auto held = [&](bool ready, std::uint16_t action, std::uint32_t extra = 0u) {
        return repeat.apply(kCircle | extra, ready, action);
    };
    check(held(true, 0u) == kCircle, "the first press on a point stays down");
    check(held(true, 0u) == kCircle, "○ stays down while the hunter has not started the gather");
    check((held(true, 40u) & kCircle) == 0u, "○ comes up once the gather action starts");
    check((held(true, 40u) & kCircle) == 0u, "○ stays up for the whole gather");
    bool gap = true;
    for (unsigned read = 0; read < kGatherHoldGap; ++read)
        gap = gap && (held(true, 0u) & kCircle) == 0u;
    check(gap, "after the gather, ○ stays up for the gap");
    check(held(true, 0u) == kCircle, "then ○ goes down, which is the next gather");
    check((held(true, 40u) & kCircle) == 0u, "the next gather lifts ○ again");

    repeat.reset();
    check(repeat.apply(kCircle, false, 0u) == kCircle, "away from a point, a held ○ is left alone");
    check(repeat.apply(kCircle, false, 40u) == kCircle, "an action change away from a point does not pulse ○");

    repeat.reset();
    check(held(true, 0u) == kCircle, "a press starts the hold");
    check((repeat.apply(kCircle | kLead, true, 40u) & kCircle) != 0u, "L+○ is left to the item bar");
    check((held(true, 40u) & kCircle) == 0u, "the gather still lifts ○ once L is up");

    repeat.reset();
    check(repeat.apply(kCircle, true, std::nullopt) == kCircle, "with no readable action, ○ is left alone");
    // Already holding ○ on the read before the point is in range.
    check(repeat.apply(kCircle, false, 0u) == kCircle, "walking up with ○ held is not yet a gather");
    bool release = true;
    for (unsigned read = 0; read < kGatherHoldGap; ++read)
        release = release && (held(true, 0u) & kCircle) == 0u;
    check(release && held(true, 0u) == kCircle,
        "walking onto a point with ○ already held releases it, then presses");

    repeat.reset();
    check(repeat.apply(kSquare, true, 0u) == kSquare && repeat.apply(kSquare, true, 40u) == kSquare,
        "□ is not the gather button, so a hold of it is left alone");
}

} // namespace

int main() {
    test_the_radius();
    test_the_area_list();
    test_unmapped();
    test_words_and_the_hunter();
    test_on_the_picture();
    test_size_and_thickness();
    test_holding_circle();
    std::cout << (failures ? "FAIL" : "PASS") << ": gather icon (" << failures << " failures)\n";
    return failures ? 1 : 0;
}
