// Damage numbers: the hitzone colour, the health drop, and the hit watch.
// No game data. Guest memory is a stand-in.
#include "camera/lock_on.hpp"
#include "game/damage_numbers.hpp"
#include "game/hit_trace.hpp"

#include "psprecomp/runtime.hpp"

#include <bit>
#include <cmath>
#include <iostream>
#include <limits>
#include <sstream>

namespace {
using namespace mhfu::game;

int failures{};

void check(bool condition, const char *message) {
    if (!condition) {
        ++failures;
        std::cerr << "FAIL: " << message << '\n';
    }
}

void test_sharpness_and_colour() {
    check(sharpness_modifier(Sharpness::Red) == 0.50f && sharpness_modifier(Sharpness::Orange) == 0.75f &&
            sharpness_modifier(Sharpness::Yellow) == 1.00f && sharpness_modifier(Sharpness::Green) == 1.05f &&
            sharpness_modifier(Sharpness::Blue) == 1.20f && sharpness_modifier(Sharpness::White) == 1.32f &&
            sharpness_modifier(Sharpness::Purple) == 1.45f,
        "sharpness modifiers are the Freedom Unite steps");

    const Grade yellow = grade_hit(45, Sharpness::Yellow, false, Affinity::None);
    const Grade just_under = grade_hit(44, Sharpness::Yellow, false, Affinity::None);
    check(yellow.fill == Fill::WeakPoint && just_under.fill == Fill::Ordinary, "45 is a weak point and 44 is not");

    check(grade_hit(90, Sharpness::Red, false, Affinity::None).fill == Fill::WeakPoint &&
            grade_hit(89, Sharpness::Red, false, Affinity::None).fill == Fill::Ordinary,
        "red reaches 45 at hitzone 90");
    check(grade_hit(32, Sharpness::Purple, false, Affinity::None).fill == Fill::WeakPoint &&
            grade_hit(31, Sharpness::Purple, false, Affinity::None).fill == Fill::Ordinary,
        "purple reaches 45 at hitzone 32");
    check(grade_hit(43, Sharpness::Green, false, Affinity::Positive).fill == Fill::WeakPoint &&
            grade_hit(43, Sharpness::Green, false, Affinity::Positive).affinity == Affinity::Positive,
        "green 43 is a weak point and the critical mark stays beside it");

    check(grade_hit(45, Sharpness::Red, true, Affinity::None).fill == Fill::WeakPoint &&
            grade_hit(44, Sharpness::Purple, true, Affinity::Negative).fill == Fill::Ordinary,
        "a gun uses the hitzone alone");
    check(grade_hit(44, Sharpness::Purple, true, Affinity::Negative).affinity == Affinity::Negative,
        "a negative critical is its own mark on a gray number");
    check(grade_hit(-5, Sharpness::Purple, false, Affinity::None).fill == Fill::Ordinary,
        "a negative hitzone is not a weak point");

    const Grade small = grade_hit(80, Sharpness::Yellow, false, Affinity::None);
    const Grade large = grade_hit(80, Sharpness::Yellow, false, Affinity::None);
    check(small.fill == large.fill, "the same part is the same colour whatever the two drops were");

    const Ink orange = damage_number_ink(Fill::WeakPoint);
    const Ink gray = damage_number_ink(Fill::Ordinary);
    const Ink white = damage_number_ink(Fill::Unknown);
    check(orange.red == 255 && orange.green < 200 && orange.blue < 80, "a weak point is orange");
    check(gray.red == gray.green && gray.green == gray.blue && gray.red < 200, "any other known part is gray");
    check(white.red == 255 && white.green == 255 && white.blue == 255, "an unknown part stays white");
}

void test_the_drop() {
    check(damage_shown(100, 80) == 20, "a hit shows how much health fell");
    check(!damage_shown(100, 100) && !damage_shown(100, 130), "a miss or a heal shows nothing");
    check(!damage_shown(0, -5) && !damage_shown(-1, -8), "a monster already at zero shows nothing");
    check(damage_shown(10, -20) == 30, "overkill still shows the whole drop");
    check(damage_shown(1, 0) == 1, "the last point of health is a hit");
}

void test_text_and_place() {
    const Number plain = make_number(27, Grade{}, 0);
    check(damage_number_text(plain) == "27", "a plain hit is the number");
    Grade crit;
    crit.affinity = Affinity::Positive;
    Grade negative;
    negative.affinity = Affinity::Negative;
    check(damage_number_text(make_number(27, crit, 0)) == "27!", "a critical hit adds its mark");
    check(damage_number_text(make_number(27, negative, 0)) == "27~", "a negative critical adds its own mark");

    check(number_anchor(plain).alpha == 0.0f, "a number with no place on the monster is not drawn");
    Number on = plain;
    on.placed = true;
    on.x = 0.40f;
    on.y = 0.55f;
    const Anchor first = number_anchor(on);
    check(first.x == 0.40f && first.y == 0.55f && first.alpha == 1.0f && first.scale == 1.0f,
        "a number starts on the monster, at full size");
    Number second = on;
    second.lane = 1;
    Number third = on;
    third.lane = 2;
    const Anchor stacked = number_anchor(second);
    check(stacked.x == first.x && number_anchor(third).x == first.x,
        "the next hits stay on the hit's point and never step aside");
    check(stacked.y < first.y && number_anchor(third).y < stacked.y, "they stack upward from it");
    Number risen = on;
    risen.age = kNumberSeconds * 0.5f;
    check(number_anchor(risen).y < first.y && number_anchor(risen).alpha == 1.0f, "a number rises and holds its ink");

    check(closeness_scale(0.0f) == 1.0f && closeness_scale(kShrinkFrom) == 1.0f,
        "a monster far away gets the number at full size");
    const float mid = closeness_scale((kShrinkFrom + kShrinkTo) * 0.5f);
    check(mid < 1.0f && mid > kMinScale, "a monster nearer shrinks it");
    check(closeness_scale(kShrinkTo) == kMinScale && closeness_scale(5.0f) == kMinScale,
        "a monster filling the view shrinks it to its least, and no further");
    check(closeness_scale(std::numeric_limits<float>::quiet_NaN()) == 1.0f, "a body of no size does not shrink it");
    Number close = on;
    close.body = kShrinkTo;
    const Anchor near = number_anchor(close);
    check(near.scale == kMinScale && near.x == first.x && near.y == first.y,
        "up close the number is smaller and still on the hit's point");
    Number close_second = close;
    close_second.lane = 1;
    check(first.y - stacked.y > near.y - number_anchor(close_second).y, "a smaller number stacks closer");
    Number fading = on;
    fading.age = kNumberSeconds;
    check(number_anchor(fading).alpha == 0.0f, "a number fades out at the end of its time");

    std::vector<Number> live{plain};
    live = age_numbers(std::move(live), kNumberSeconds);
    check(live.empty(), "a number is gone once its time is up");
    live = age_numbers({plain}, -1.0f);
    check(live.size() == 1u && live[0].age == 0.0f, "time does not run backwards");

    std::vector<Number> crowded;
    for (int i = 0; i < static_cast<int>(kMaxOnScreen) + 1; ++i) remember_number(crowded, make_number(i, Grade{}, i));
    check(crowded.size() == kMaxOnScreen && crowded.front().amount == 1 && crowded.back().amount == 8,
        "only the latest numbers stay on screen");
}

void test_the_hit_watch() {
    std::vector<OpenHit> open;
    watch_hit_enter(open, kStatusTick, 0u, 0x09000000u, 400);
    check(open.empty() && !watch_hit_leave(open, kStatusTick, 0u, 380, 0), "a status tick is not a hit");

    watch_hit_enter(open, kHitAdjust, 1u, 0x09000000u, 400);
    watch_hit_enter(open, kHitAdjust, 2u, 0x09100000u, 200);
    check(open_object(open, kHitAdjust, 2u) == 0x09100000u, "the inner hit is the monster it was called with");
    const std::optional<Number> inner = watch_hit_leave(open, kHitAdjust, 2u, 170, 0);
    check(inner && inner->amount == 30 && inner->lane == 0 && inner->grade.fill == Fill::Unknown &&
            inner->object == 0x09100000u,
        "the inner hit shows its own drop in white, on that monster");
    check(!watch_hit_leave(open, kHitAdjust, 2u, 100, 1), "a return with no matching call shows nothing");
    const std::optional<Number> outer = watch_hit_leave(open, kHitAdjust, 1u, 250, 1);
    check(outer && outer->amount == 150 && outer->lane == 1 && open.empty(), "the outer hit shows what it removed");

    watch_hit_enter(open, kHitAdjust, 0u, 0x09000000u, 80);
    check(!watch_hit_leave(open, kHitAdjust, 0u, 80, 0) && open.empty(),
        "a call that does not change health is silent");
    watch_hit_enter(open, kHitAdjust, 0u, 0x09000000u, 80);
    check(!watch_hit_leave(open, kHitAdjust, 0u, 90, 0) && open.empty(), "a heal through the same routine is silent");
}

void test_health_halfword() {
    psprecomp::GuestMemory memory(32u * 1024u * 1024u);
    constexpr std::uint32_t object = 0x09000000u;
    memory.store16(object + kMonsterHealth, static_cast<std::uint16_t>(-25));
    check(monster_health(memory, object) == -25, "health is the signed halfword at +0x2E4");
    check(!monster_health(memory, 0u) && !monster_health(memory, object + 1u),
        "a null or an odd address is not a monster");
    check(!monster_health(memory, 0x0E000000u), "an address outside guest RAM is not a monster");
}

WorldCamera test_camera() {
    WorldCamera camera;
    camera.view[0] = camera.view[5] = camera.view[10] = camera.view[15] = 1.0f;
    camera.projection[0] = camera.projection[5] = camera.projection[10] = 1.0f;
    camera.projection[11] = -1.0f;
    camera.x_scale = 240.0f;
    camera.x_offset = 240.0f;
    camera.y_scale = -136.0f;
    camera.y_offset = 136.0f;
    return camera;
}

void test_on_the_monster() {
    const WorldCamera camera = test_camera();
    const std::optional<PicturePoint> centre = project_world(camera, 0.0f, 0.0f, -10.0f);
    check(centre && centre->x == 0.5f && centre->y == 0.5f, "a point in front of the camera lands on the picture");
    const std::optional<PicturePoint> high = project_world(camera, 0.0f, 10.0f, -10.0f);
    check(high && high->y == 0.0f, "a point above the camera lands at the top of the picture");
    check(!project_world(camera, 0.0f, 0.0f, 10.0f), "a point behind the camera is not drawn");
    check(!project_world(camera, 100.0f, 0.0f, -10.0f), "a point off the picture is not drawn");
    const std::optional<PicturePoint> below = project_point(camera, 0.0f, -20.0f, -10.0f);
    check(below && below->x == 0.5f && below->y == 1.5f, "a monster's feet off the picture still have a place");
    check(!project_point(camera, 0.0f, 0.0f, 10.0f), "but not behind the camera");
    WorldCamera unscaled = camera;
    unscaled.x_scale = 0.0f;
    check(!project_world(unscaled, 0.0f, 0.0f, -10.0f), "a draw with no viewport is not a camera");

    psprecomp::GuestMemory memory(32u * 1024u * 1024u);
    constexpr std::uint32_t object = 0x09000000u;
    memory.store32(object + kMonsterPosition, std::bit_cast<std::uint32_t>(12.0f));
    memory.store32(object + kMonsterPosition + 4u, std::bit_cast<std::uint32_t>(0.0f));
    memory.store32(object + kMonsterPosition + 8u, std::bit_cast<std::uint32_t>(-40.0f));
    const std::optional<WorldPoint> at = monster_position(memory, object);
    check(at && at->x == 12.0f && at->y == 0.0f && at->z == -40.0f,
        "the monster's position is the three floats at +0x200");
    check(!monster_position(memory, 0u) && !monster_position(memory, object + 1u),
        "a null or an odd address has no position");
    memory.store32(object + kMonsterPosition, 0x7fc00000u);
    check(!monster_position(memory, object), "a position that is not a number is ignored");

    WorldCamera hud = camera;
    hud.projection[11] = 0.0f;
    note_world_camera(hud, 1000u);
    note_world_camera(camera, 10u);
    WorldCamera smaller = camera;
    smaller.x_offset = 0.0f;
    note_world_camera(smaller, 3u);
    const std::optional<WorldCamera> taken = take_world_camera();
    check(taken && taken->x_offset == 240.0f && !take_world_camera(),
        "the perspective view with the most vertices is the world camera, once");

    // Akantor's map: a sky drawn through a view without translation outdraws
    // any one draw of the world, whose view moves with the camera.
    WorldCamera world = camera;
    world.view[12] = 50.0f;
    world.view[14] = -300.0f;
    WorldCamera other = world;
    other.view[13] = 7.0f;
    check(camera_centred(camera) && !camera_centred(world), "a view without translation is centred on the camera");
    note_world_camera(camera, 1000u);
    note_world_camera(world, 300u);
    note_world_camera(world, 300u);
    note_world_camera(other, 500u);
    const std::optional<WorldCamera> moving = take_world_camera();
    check(moving && moving->view[12] == 50.0f && moving->view[13] == 0.0f,
        "the world view wins over the sky, and its draws are counted together");
    note_world_camera(camera, 20u);
    const std::optional<WorldCamera> only_sky = take_world_camera();
    check(only_sky && camera_centred(*only_sky), "a centred view is still a camera when there is no other");
}

void test_the_stroke() {
    const Stroke thin = number_stroke(1u, 24.0f, 24.0f);
    check(thin.outline == 1.0f && thin.weight == 0.0f, "thickness 1 is a one-pixel outline and the font's strokes");
    const Stroke bold = number_stroke(5u, 24.0f, 24.0f);
    check(bold.outline == 5.0f && bold.weight == 2.0f, "each step thickens the outline and the digits");
    const Stroke large = number_stroke(2u, 72.0f, 24.0f);
    check(large.outline == 6.0f && large.weight == 1.5f, "a larger number gets a proportionally larger stroke");
    const Stroke tiny = number_stroke(1u, 6.0f, 24.0f);
    check(tiny.outline == 1.0f, "a small number keeps at least one pixel of outline");
    const Stroke none = number_stroke(0u, 24.0f, 24.0f);
    check(none.outline == 0.0f && number_stroke(1u, 24.0f, 0.0f).outline == 0.0f, "no stroke without a thickness or a unit");

    check(stack_gap_px(10.0f, 3.0f, 60.0f, 7.0f, false) == 10.0f, "the default size and thickness keep the tuned gap");
    check(stack_gap_px(10.0f, 3.0f, 60.0f, 3.0f, true) == 66.0f, "a larger number is lifted by its whole height");
    check(stack_gap_px(10.0f, 1.0f, 20.0f, 7.0f, true) == 34.0f, "a thicker number clears the stroke on both sides");
    check(stack_gap_px(10.0f, 0.5f, 4.0f, 1.0f, true) == 6.0f, "a smaller number keeps the scaled gap when that is taller");
    check(stack_gap_px(-1.0f, 0.0f, -5.0f, -2.0f, true) == 0.0f, "a gap with no size is nothing");

    check(stroke_offsets(0.0f, 3).size() == 1u && stroke_offsets(0.2f, 3).size() == 1u &&
            stroke_offsets(4.0f, 0).size() == 1u,
        "no radius, or no rings, is the text drawn once");
    const std::vector<Offset> ring = stroke_offsets(4.0f, 2);
    check(ring.size() == 25u && ring.front().x == 0.0f && ring.front().y == 0.0f, "the centre and 12 points a ring");
    float furthest = 0.0f;
    for (const Offset &o : ring) furthest = std::max(furthest, std::sqrt(o.x * o.x + o.y * o.y));
    check(std::fabs(furthest - 4.0f) < 1e-4f && std::fabs(std::hypot(ring[1].x, ring[1].y) - 2.0f) < 1e-4f,
        "the rings are spaced out to the radius");
}

void test_the_game_camera_view() {
    // The game's camera far out in the world, looking down the +z axis at a
    // monster; the GE view it draws with has no translation.
    const WorldPoint eye{9500.0f, 300.0f, 13000.0f};
    const WorldPoint monster{9500.0f, 300.0f, 14500.0f};
    const std::optional<std::array<float, 16>> view = look_at_view(eye, monster);
    check(view.has_value(), "a camera looking level has a view");
    WorldCamera camera = test_camera();
    camera.view = *view;
    check(!camera_centred(camera), "the camera's own view carries its place");
    const std::optional<PicturePoint> centre = project_world(camera, monster.x, monster.y, monster.z);
    check(centre && std::fabs(centre->x - 0.5f) < 1e-4f && std::fabs(centre->y - 0.5f) < 1e-4f,
        "the point the camera looks at lands in the middle of the picture");
    const std::optional<PicturePoint> above = project_world(camera, monster.x, monster.y + 500.0f, monster.z);
    check(above && above->y < 0.5f, "a point above it lands higher");
    // Looking down +z, the camera's right is -x.
    const std::optional<PicturePoint> right = project_world(camera, monster.x - 500.0f, monster.y, monster.z);
    check(right && right->x > 0.5f, "a point to the camera's right lands on the right");
    check(!project_world(camera, monster.x, monster.y, eye.z - 1500.0f), "a point behind it is not drawn");

    const std::optional<std::array<float, 16>> tilted = look_at_view(eye, {9600.0f, 0.0f, 14000.0f});
    float tilted_rows = 0.0f;
    for (std::size_t row = 0; row < 3u && tilted; ++row)
        for (std::size_t other = 0; other < 3u; ++other) {
            float dot = 0.0f;
            for (std::size_t axis = 0; axis < 3u; ++axis)
                dot += (*tilted)[axis * 4u + row] * (*tilted)[axis * 4u + other];
            tilted_rows += std::fabs(dot - (row == other ? 1.0f : 0.0f));
        }
    check(tilted && tilted_rows < 1e-4f, "a tilted view is a rotation");
    check(!look_at_view(eye, eye), "a camera looking at its own eye has no view");
    check(!look_at_view(eye, {eye.x, eye.y - 100.0f, eye.z}), "nor one looking straight down");
}

void test_the_setting_does_not_slow_direct_chains() {
    const bool original = psprecomp::g_runtime_chain_observers_active;
    psprecomp::g_runtime_chain_observers_active = false;
    damage_numbers_frame(true);
    check(!psprecomp::g_runtime_chain_observers_active, "watching hits leaves direct chains on the fast path");
    damage_numbers_frame(true);
    check(!psprecomp::g_runtime_chain_observers_active, "leaving the setting on does not mark chains observed");
    damage_numbers_frame(false);
    check(!psprecomp::g_runtime_chain_observers_active && damage_numbers().empty(),
        "turning the setting off removes the watch and the numbers");

    psprecomp::g_runtime_chain_observers_active = true;
    damage_numbers_frame(true);
    check(psprecomp::g_runtime_chain_observers_active, "a chain observer that was already on stays on");
    damage_numbers_frame(false);
    check(psprecomp::g_runtime_chain_observers_active, "turning damage numbers off leaves that observer on");
    psprecomp::g_runtime_chain_observers_active = original;
}

// A quest in progress on the stand-in: game_task.ovl in the task slot, and an
// entity list of a hunter, a large monster, a small one, a companion with no
// health pool, and a second node for the large monster.
constexpr std::uint32_t kLarge = 0x09830000u;
constexpr std::uint32_t kSmall = 0x09840000u;

void stand_in_quest(psprecomp::GuestMemory &memory) {
    namespace layout = mhfu::camera::lock_on_layout;
    memory.store32(layout::kTaskSlot, 0x336F574Du);
    memory.store32(layout::kTaskSlot + 8u, layout::kTaskSlot);
    const char name[] = "game_task.ovl";
    for (std::uint32_t i = 0; i < sizeof(name); ++i)
        memory.store8(layout::kTaskSlot + 32u + i, static_cast<std::uint8_t>(name[i]));
    constexpr std::uint32_t manager = 0x09800000u;
    constexpr std::uint32_t hunter = 0x09820000u, companion = 0x09850000u;
    const std::uint32_t entities[] = {hunter, kLarge, kSmall, companion, kLarge};
    memory.store32(layout::kEntityList, manager);
    memory.store32(manager + layout::kListHead, 0x09810000u);
    for (std::uint32_t i = 0; i < 5u; ++i) {
        const std::uint32_t node = 0x09810000u + i * 0x100u;
        memory.store32(node + layout::kNodeEntity, entities[i]);
        memory.store8(node + layout::kNodeType, i == 0u ? layout::kPlayerNode : 0u);
        memory.store32(node + layout::kNodeOlder, i + 1u < 5u ? node + 0x100u : 0u);
    }
    for (const std::uint32_t entity : {hunter, kLarge}) {
        memory.store16(entity + kMonsterHealth, 2000u);
        memory.store16(entity + kMonsterMostHealth, 3000u);
    }
    memory.store16(kSmall + kMonsterHealth, 120u);
    memory.store16(kSmall + kMonsterMostHealth, 150u);
    memory.store16(companion + kMonsterHealth, 500u);
    memory.store16(companion + kMonsterMostHealth, 0u);
    memory.store32(hunter + layout::kVtable, 0x09A60000u);
    for (const std::uint32_t monster : {kLarge, kSmall, companion})
        memory.store32(monster + layout::kVtable, 0x09D20000u);
}

// Ad hoc play: a hit applied on another console arrives as a new health,
// written without the hit routine. The frame's reading shows it.
void test_the_health_watch() {
    check(parse_health_watch(nullptr) && parse_health_watch("") && parse_health_watch("1"),
        "the health watch is on unless turned off");
    check(!parse_health_watch("0"), "MHFU_DAMAGE_NUMBERS_HEALTH_WATCH=0 turns it off");

    psprecomp::GuestMemory memory(32u * 1024u * 1024u);
    check(quest_health(memory).empty(), "outside a quest nothing is read");
    stand_in_quest(memory);
    const std::vector<HealthSeen> found = quest_health(memory);
    check(found.size() == 2u && found[0].object == kLarge && found[0].health == 2000 && found[0].most == 3000 &&
            found[1].object == kSmall && found[1].health == 120,
        "large and small monsters, once each; no hunter and no companion");

    // Another player's hunter, on a node that is not marked as a player: the
    // same class as this console's hunter gives it away.
    namespace layout = mhfu::camera::lock_on_layout;
    constexpr std::uint32_t hunter_class = 0x09A60000u, monster_class = 0x09D20000u;
    constexpr std::uint32_t remote = 0x09860000u, remote_node = 0x09810500u;
    memory.store32(remote + layout::kVtable, hunter_class);
    memory.store16(remote + kMonsterHealth, 100u);
    memory.store16(remote + kMonsterMostHealth, 150u);
    memory.store32(remote_node + layout::kNodeEntity, remote);
    memory.store8(remote_node + layout::kNodeType, 0u);
    memory.store32(0x09810400u + layout::kNodeOlder, remote_node);
    const std::vector<HealthSeen> with_remote = quest_health(memory);
    check(with_remote.size() == 2u && with_remote[0].object == kLarge && with_remote[0].vtable == monster_class &&
            with_remote[1].object == kSmall,
        "another player's hunter is not watched");
    check(is_hunter(memory, 0x09820000u) && is_hunter(memory, remote) && !is_hunter(memory, kLarge) &&
            !is_hunter(memory, kSmall) && !is_hunter(memory, 0u),
        "this console's hunter and another player's are hunters; the monsters are not");
    memory.store32(remote + layout::kVtable, monster_class);
    check(quest_health(memory).size() == 3u && !is_hunter(memory, remote), "an unmarked entity of another class is");
    memory.store32(0x09810400u + layout::kNodeOlder, 0u);
    memory.store8(mhfu::camera::lock_on_layout::kTaskSlot + 32u, 'x');
    check(quest_health(memory).empty(), "another overlay in the task slot is not a quest");

    std::vector<HealthSeen> seen;
    check(watch_health(seen, {{kLarge, 2000, 3000}}, 0).empty() && seen.size() == 1u,
        "a monster seen for the first time shows nothing");
    const std::vector<Number> two = watch_health(seen, {{kLarge, 1900, 3000}, {kSmall, 100, 150}}, 3);
    check(two.size() == 1u && two[0].amount == 100 && two[0].object == kLarge && two[0].lane == 3 &&
            two[0].grade.fill == Fill::Unknown,
        "a drop on a known monster is a number in white");
    const std::vector<Number> both = watch_health(seen, {{kLarge, 1850, 3000}, {kSmall, 70, 150}}, 1);
    check(both.size() == 2u && both[0].amount == 50 && both[0].lane == 1 && both[1].amount == 30 && both[1].lane == 2,
        "drops on two monsters in one frame take a lane each");
    check(watch_health(seen, {{kLarge, 1900, 3000}, {kSmall, 70, 150}}, 0).empty(), "a heal shows nothing");
    check(watch_health(seen, {{kLarge, 900, 2500}}, 0).empty(),
        "another most health is another monster in the same place");
    check(watch_health(seen, {{kLarge, 0, 2500}}, 0).size() == 1u, "the killing blow shows");
    check(watch_health(seen, {{kLarge, -20, 2500}}, 0).empty(), "nothing below zero");
    check(watch_health(seen, {}, 0).empty() && seen.empty(), "a monster gone from the list is forgotten");

    seen = {{kLarge, 1000, 3000}};
    note_change(seen, kLarge, 1000, 900);
    check(watch_health(seen, {{kLarge, 900, 3000}}, 0).empty(), "a hit the routine showed is not shown twice");
    note_change(seen, kLarge, 850, 850);
    check(watch_health(seen, {{kLarge, 850, 3000}}, 0).size() == 1u,
        "the monster's own update does not hide a drop written from the network");
    note_change(seen, kSmall, 70, 60);
    check(seen.size() == 1u, "a call on a monster not seen yet is left for the frame");
}

void routine_takes_a1(psprecomp::Runtime &runtime, psprecomp::AllegrexContext &ctx) {
    const std::uint32_t at = ctx.gpr[4] + kMonsterHealth;
    runtime.memory().store16(at, static_cast<std::uint16_t>(runtime.memory().load16(at) - ctx.gpr[5]));
}

int shown_total() {
    int total = 0;
    for (const Number &number : damage_numbers()) total += number.amount;
    return total;
}

// The same through the frame and the watched calls: a hit through the routine
// once, a drop from the network once, a status tick not at all.
void test_every_hit_once() {
    psprecomp::Runtime runtime;
    psprecomp::GuestMemory &memory = runtime.memory();
    stand_in_quest(memory);
    runtime.register_function(kHitAdjust, &routine_takes_a1, "recomp_unit_stand_in_hit");
    runtime.register_function(kStatusTick, &routine_takes_a1, "recomp_unit_stand_in_status_tick");
    auto call = [&runtime](std::uint32_t pc, std::uint32_t object, std::uint32_t amount) {
        runtime.cpu().gpr[4] = object;
        runtime.cpu().gpr[5] = amount;
        runtime.cpu().pc = pc;
        (void)runtime.invoke_chained_call(runtime.cpu());
    };

    damage_numbers_frame(true, &memory);
    check(damage_numbers().empty(), "the first frame only learns the health");
    call(kHitAdjust, kLarge, 100u);
    damage_numbers_frame(true, &memory);
    check(damage_numbers().size() == 1u && shown_total() == 100, "a hit through the routine shows once");

    memory.store16(kLarge + kMonsterHealth, 1850u);
    call(kHitAdjust, kLarge, 0u);
    damage_numbers_frame(true, &memory);
    check(damage_numbers().size() == 2u && shown_total() == 150,
        "a drop from the network shows, past the monster's own update");

    memory.store16(kSmall + kMonsterHealth, 90u);
    damage_numbers_frame(true, &memory);
    check(damage_numbers().size() == 3u && shown_total() == 180, "another player's hit on a small monster shows");

    call(kStatusTick, kLarge, 10u);
    damage_numbers_frame(true, &memory);
    check(damage_numbers().size() == 3u, "a status tick on this console is not a hit");

    // The monster hits this console's hunter through the same routine.
    call(kHitAdjust, 0x09820000u, 40u);
    damage_numbers_frame(true, &memory);
    check(damage_numbers().size() == 3u, "damage to the hunter is not a number");

    damage_numbers_frame(false, &memory);
    memory.store16(kLarge + kMonsterHealth, 1000u);
    damage_numbers_frame(true, &memory);
    check(damage_numbers().empty(), "turned off and on, the watch starts again");
    damage_numbers_frame(false, &memory);
}

// MHFU_TRACE_HITS (game/hit_trace.hpp): the call chain walked from a return
// address and the stack, and the lines printed, on a made-up pair of
// functions in the quest overlay's range.
void test_the_hit_records() {
    namespace tr = mhfu::game::hit_trace;
    namespace layout = mhfu::camera::lock_on_layout;
    psprecomp::GuestMemory memory(32u * 1024u * 1024u);
    check(tr::players_line(memory).empty(), "outside a quest there are no players to show");

    constexpr std::uint32_t record = 0x09F00000u;
    memory.store32(record, std::bit_cast<std::uint32_t>(10122.5f));
    memory.store32(record + 4u, std::bit_cast<std::uint32_t>(-0.5f));
    check(tr::floats_line(memory, "r", record, 2u) == "r 09F00000: 10122.5 -0.5", "a record reads as floats");
    check(tr::floats_line(memory, "r", 0x04000000u, 2u).empty(), "nothing outside RAM");

    constexpr std::uint32_t manager = 0x09800000u;
    constexpr std::uint32_t newest = 0x09810000u, older = 0x09810100u;
    constexpr std::uint32_t hunter = 0x09820000u, monster = 0x09830000u;
    memory.store32(layout::kEntityList, manager);
    memory.store32(manager + layout::kListHead, newest);
    memory.store32(newest + layout::kNodeEntity, monster);
    memory.store8(newest + layout::kNodeType, 0u);
    memory.store32(newest + layout::kNodeOlder, older);
    memory.store32(older + layout::kNodeEntity, hunter);
    memory.store8(older + layout::kNodeType, layout::kPlayerNode);
    memory.store32(hunter + layout::kPosition, std::bit_cast<std::uint32_t>(1.0f));
    memory.store32(hunter + layout::kPosition + 4u, std::bit_cast<std::uint32_t>(2.0f));
    memory.store32(hunter + layout::kPosition + 8u, std::bit_cast<std::uint32_t>(3.0f));
    check(tr::players_line(memory) == "[hit]   players 09820000 at 1 2 3", "the players on the list and where they are");
    memory.store32(older + layout::kNodeOlder, newest);
    std::string around = "[hit]   players";
    for (std::uint32_t i = 0; i < layout::kMaxMonsters / 2u; ++i) around += " 09820000 at 1 2 3";
    check(tr::players_line(memory) == around, "a cyclic list stops at the bound");
}

void test_the_hit_trace() {
    namespace tr = mhfu::game::hit_trace;
    check(tr::parse_limit(nullptr) == tr::kDefaultLimit && tr::parse_limit("") == tr::kDefaultLimit &&
            tr::parse_limit("x") == tr::kDefaultLimit && tr::parse_limit("0") == tr::kDefaultLimit,
        "an unset or bad limit is the default");
    check(tr::parse_limit("25") == 25u, "a number is the limit");
    check(tr::in_ram(0x09000000u) && !tr::in_ram(0x04000000u) && !tr::in_ram(0x0A000000u),
        "user RAM is where the game's objects are");
    check(tr::looks_like_code(0x09AC7A08u) && tr::looks_like_code(0x08900A1Cu) && !tr::looks_like_code(0x09AC7A0Au) &&
            !tr::looks_like_code(0x09F52CF4u),
        "a word in the executable's or the quest overlay's code looks like a return address");

    psprecomp::GuestMemory memory(32u * 1024u * 1024u);
    // inner: addiu sp,sp,-0x30; sw ra,0x2C(sp); ... the call returns to +0x20.
    constexpr std::uint32_t inner = 0x09B00000u;
    memory.store32(inner, 0x27BDFFD0u);
    memory.store32(inner + 4u, 0xAFBF002Cu);
    // outer: addiu sp,sp,-0x20; sw ra,0x1C(sp); returns to +0x10.
    constexpr std::uint32_t outer = 0x09C00000u;
    memory.store32(outer, 0x27BDFFE0u);
    memory.store32(outer + 4u, 0xAFBF001Cu);
    // leaf: addiu sp,sp,-0x10; jr ra, never saving ra.
    constexpr std::uint32_t leaf = 0x09C10000u;
    memory.store32(leaf, 0x27BDFFF0u);
    memory.store32(leaf + 4u, 0x03E00008u);
    constexpr std::uint32_t sp = 0x09F00000u;
    memory.store32(sp + 0x2Cu, outer + 0x10u);          // inner's saved ra
    memory.store32(sp + 0x30u + 0x1Cu, leaf + 0x8u);    // outer's saved ra
    const std::optional<tr::Frame> frame = tr::frame_of(memory, inner + 0x20u);
    check(frame && frame->start == inner && frame->size == 0x30u && frame->ra_offset == 0x2Cu,
        "a return address finds its function's start, frame size and saved ra");
    const std::optional<tr::Frame> no_ra = tr::frame_of(memory, leaf + 0x8u);
    check(no_ra && no_ra->ra_offset == ~0u, "a function that never saves ra says so");
    check(!tr::frame_of(memory, 0x04000000u), "an address outside RAM has no frame");
    check(tr::call_chain(memory, inner + 0x20u, sp) == " 09B00020@09B00000 09C00010@09C00000 09C10008@09C10000",
        "the chain walks the saved return addresses up the stack until a function keeps none");
    check(tr::call_chain(memory, inner + 0x20u, sp, 1) == " 09B00020@09B00000", "and stops at the levels asked");

    memory.store32(sp, 0x00000007u);
    memory.store32(sp + 4u, 0x09AC7A08u);
    check(tr::words_line(memory, "stack", sp, 2u) == "stack 09F00000: 00000007 09AC7A08*",
        "stack words are hex, code-looking ones marked");
    check(tr::words_line(memory, "x", 0x04000000u, 2u).empty(), "words outside RAM are not read");

    psprecomp::AllegrexContext ctx;
    ctx.gpr[4] = 0x09000000u;
    ctx.gpr[5] = 0x09000100u;
    ctx.gpr[6] = 7u;
    ctx.gpr[7] = 0x09000000u;
    ctx.gpr[29] = sp;
    ctx.gpr[31] = inner + 0x20u;
    ctx.fpr[12] = 1.5f;
    ctx.gpr[2] = 1u;
    const std::string call = tr::call_line(ctx, 3u, 2u, 400);
    check(call.rfind("[hit] call #3 depth=2 a0=09000000 a1=09000100 a2=00000007 a3=09000000", 0) == 0 &&
            call.find(" ra=09B00020 sp=09F00000 f12..f15=1.5 0 0 0 health=400") != std::string::npos,
        "the call line names the arguments, the saved registers, ra, sp, the floats and the health");
    check(tr::call_line(ctx, 1u, 0u, std::nullopt).find("health=?") != std::string::npos,
        "a monster whose health cannot be read says so");
    check(tr::return_line(ctx, 3u, 2u, 400, 370) ==
            "[hit] return #3 depth=2 v0=00000001 v1=00000000 health 400 -> 370 (30)",
        "the return line pairs with the call and shows the drop");
    const std::vector<std::string> lines = tr::describe_call(memory, ctx, 3u, 2u, 400);
    check(lines.size() == 6u && lines[1] == "[hit]   chain 09B00020@09B00000 09C00010@09C00000 09C10008@09C10000" &&
            lines[2].rfind("[hit]   stack 09F00000:", 0) == 0 && lines[3].rfind("[hit]   a1-> 09000100:", 0) == 0,
        "a call prints its registers, chain, stack, and the records a1..a3 point at, but not the monster again");
    check(lines[4].rfind("[hit]   a1 floats 09000100:", 0) == 0 && lines[5].rfind("[hit]   monster 09000200:", 0) == 0,
        "then a1's record as floats and the monster's position; no pointer in it, and no players, print nothing");

    // What is printed: a call that changes nothing names its caller once and
    // is otherwise silent; a call that takes health off is printed whole.
    tr::reset();
    std::ostringstream out;
    std::streambuf *const previous = std::cout.rdbuf(out.rdbuf());
    tr::note_call(memory, ctx, 1u, 400);
    check(tr::log_return(ctx, 1u, 400, 400) == 0u, "a call that changes nothing is not a hit");
    tr::note_call(memory, ctx, 1u, 400);
    check(tr::log_return(ctx, 1u, 400, 400) == 0u && tr::log_return(ctx, 1u, 400, 380) == 0u,
        "nor is the same call again, and a return with no call is nothing");
    std::string printed = out.str();
    check(printed.find("[hit] calls from ra=09B00020 change nothing") != std::string::npos &&
            printed.find("change nothing") == printed.rfind("change nothing") &&
            printed.find("[hit] call #") == std::string::npos,
        "a caller whose calls change nothing is named once, and its calls are not printed");
    out.str({});
    tr::note_call(memory, ctx, 1u, 400);
    check(tr::log_return(ctx, 1u, 400, 370) == 1u, "a call that takes health off is the first hit");
    printed = out.str();
    check(printed.find("[hit] call #1 depth=1 a0=09000000") != std::string::npos &&
            printed.find("[hit]   chain 09B00020@09B00000") != std::string::npos &&
            printed.find("[hit] return #1 depth=1 v0=00000001 v1=00000000 health 400 -> 370 (30)") != std::string::npos,
        "the hit is printed whole at its return: the call, the chain, the stack, the records and the drop");
    // Between frames: a drop no printed call accounted for is reported.
    out.str({});
    memory.store16(0x09000000u + kMonsterHealth, 370u);
    tr::frame(memory, &monster_health);
    check(out.str().empty(), "a frame whose drop the call explained says nothing");
    memory.store16(0x09000000u + kMonsterHealth, 350u);
    tr::frame(memory, &monster_health);
    check(out.str().find("[hit] health of 09000000 went 370 -> 350 between frames") != std::string::npos,
        "a drop between frames that no call made is reported as written elsewhere");
    out.str({});
    tr::frame(memory, &monster_health);
    check(out.str().empty(), "and only once");
    std::cout.rdbuf(previous);
    tr::reset();
}

} // namespace

int main() {
    test_sharpness_and_colour();
    test_the_drop();
    test_text_and_place();
    test_the_hit_watch();
    test_health_halfword();
    test_on_the_monster();
    test_the_stroke();
    test_the_game_camera_view();
    test_the_setting_does_not_slow_direct_chains();
    test_the_health_watch();
    test_every_hit_once();
    test_the_hit_records();
    test_the_hit_trace();
    if (failures != 0) {
        std::cerr << failures << " failure(s)\n";
        return 1;
    }
    std::cout << "damage number tests passed\n";
    return 0;
}
