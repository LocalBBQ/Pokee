// Damage numbers: the hitzone colour, the health drop, and the hit watch.
// No game data. Guest memory is a stand-in.
#include "game/guest_layout.hpp"
#include "game/damage_numbers.hpp"
#include "game/hit_trace.hpp"

#include "psprecomp/runtime.hpp"

#include <algorithm>
#include <bit>
#include <cmath>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>

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

    const Ink weak = damage_number_ink(Fill::WeakPoint);
    const Ink ordinary = damage_number_ink(Fill::Ordinary);
    const Ink white = damage_number_ink(Fill::Unknown);
    check(weak.red == 255 && weak.green > 180 && weak.blue < 80, "a weak point is yellow");
    check(ordinary.red == 255 && ordinary.green == 255 && ordinary.blue == 255, "any other part is white");
    check(white.red == 255 && white.green == 255 && white.blue == 255, "an unknown part is white too");

    check(grade_share(0.45f, Affinity::None).fill == Fill::WeakPoint &&
            grade_share(0.449f, Affinity::None).fill == Fill::Ordinary,
        "the game's effective hitzone: 0.45 is a weak point and just under it is not");
    check(grade_share(1.3f, Affinity::Negative).affinity == Affinity::Negative &&
            grade_share(std::numeric_limits<float>::quiet_NaN(), Affinity::None).fill == Fill::Ordinary,
        "the mark is kept, and a hitzone that is not a number is no weak point");
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
    check(damage_number_text(make_number(27, crit, 0)) == "27" &&
            damage_number_text(make_number(27, negative, 0)) == "27",
        "a critical hit's mark is a dot, not text");

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
    Number held = on;
    held.age = kNumberSeconds * kHoldShare;
    check(number_anchor(held).y == first.y && number_anchor(held).alpha == 1.0f,
        "a number holds still and keeps its ink at first");
    Number early = on;
    early.age = kNumberSeconds * (kHoldShare + (1.0f - kHoldShare) * 0.25f);
    Number late = on;
    late.age = kNumberSeconds * (kHoldShare + (1.0f - kHoldShare) * 0.75f);
    const Anchor early_exit = number_anchor(early);
    const Anchor late_exit = number_anchor(late);
    check(early_exit.y < first.y && late_exit.y < early_exit.y && early_exit.alpha < 1.0f &&
            late_exit.alpha < early_exit.alpha && late_exit.alpha > 0.0f,
        "then it rises as it fades out");
    Number halfway = on;
    halfway.age = kNumberSeconds * (kHoldShare + (1.0f - kHoldShare) * 0.5f);
    check(first.y - early_exit.y > early_exit.y - number_anchor(halfway).y, "rising quickly at first and slowing");
    Number gone = on;
    gone.age = kNumberSeconds;
    check(std::fabs(first.y - number_anchor(gone).y - kRise) < 1e-5f, "it ends kRise above the hit's point");

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

void test_the_older_frame() {
    Number fresh = make_number(40, Grade{}, 1);
    fresh.placed = true;
    fresh.x = 0.40f;
    fresh.y = 0.55f;
    check(!older_anchor(fresh), "a number placed for the first time has no older place to glide from");

    Number moving = fresh;
    keep_older_place(moving);
    check(moving.older_placed && moving.older_x == 0.40f && moving.older_y == 0.55f && moving.older_age == 0.0f,
        "a new game frame keeps the number's place and age as the older one");
    moving = age_numbers({moving}, 0.1f).front();
    moving.x = 0.50f;
    moving.y = 0.45f;
    const std::optional<Anchor> older = older_anchor(moving);
    const Anchor newer = number_anchor(moving);
    const Anchor expected = number_anchor(fresh);
    check(older && older->x == expected.x && older->y == expected.y,
        "the older anchor is where the last game frame drew it, lane and age included");
    check(older && newer.x > older->x && newer.y < older->y, "and the number moves from there to its new place");

    Number same = fresh;
    same.age = kNumberSeconds * kHoldShare;
    keep_older_place(same);
    same = age_numbers({same}, 0.1f).front();
    const std::optional<Anchor> rising = older_anchor(same);
    check(rising && rising->x == number_anchor(same).x && rising->y > number_anchor(same).y,
        "a number fading out where it was hit still rises between the two frames");

    Number lost = moving;
    lost.placed = false;
    check(!older_anchor(lost), "a number off the picture now does not glide");
    Number back = moving;
    back.older_placed = false;
    check(!older_anchor(back), "a number just back on the picture does not glide in");

    check(parse_smooth(nullptr) && parse_smooth("") && parse_smooth("1"), "the numbers follow interpolation by default");
    check(!parse_smooth("0"), "MHFU_DAMAGE_NUMBERS_SMOOTH=0 turns that off");
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

    check(outer && !outer->contact && !outer->anchored, "a hit with no point of contact waits for the body point");
    watch_hit_enter(open, kHitAdjust, 0u, 0x09000000u, 80, WorldPoint{1.0f, 2.0f, 3.0f});
    const std::optional<Number> landed = watch_hit_leave(open, kHitAdjust, 0u, 60, 0);
    check(landed && landed->contact && landed->anchored && landed->at.y == 2.0f,
        "a hit with a point of contact is drawn there");

    watch_hit_enter(open, kHitAdjust, 0u, 0x09000000u, 80);
    check(
        !watch_hit_leave(open, kHitAdjust, 0u, 80, 0) && open.empty(), "a call that does not change health is silent");
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
    Number hit = make_number(30, Grade{}, 0);
    hit.object = object;
    const std::optional<WorldPoint> struck = hit_place(hit, memory, false);
    check(struck && struck->x == 12.0f && struck->z == -40.0f && hit.anchored,
        "a number takes the monster's place when it is first placed");
    Number riding = hit;
    memory.store32(object + kMonsterPosition, std::bit_cast<std::uint32_t>(80.0f));
    const std::optional<WorldPoint> stays = hit_place(hit, memory, false);
    check(stays && stays->x == 12.0f, "and stays there as the monster walks on");
    const std::optional<WorldPoint> follows = hit_place(riding, memory, true);
    check(follows && follows->x == 80.0f && riding.at.x == 80.0f, "MHFU_DAMAGE_NUMBERS_FOLLOW=1 rides the monster");
    check(parse_follow("1") && !parse_follow(nullptr) && !parse_follow("") && !parse_follow("0"),
        "the numbers stay where the hit landed unless MHFU_DAMAGE_NUMBERS_FOLLOW=1");

    Number struck_there = make_number(30, Grade{}, 0);
    struck_there.object = object;
    struck_there.at = {300.0f, 450.0f, 100.0f};
    struck_there.anchored = true;
    struck_there.contact = true;
    const std::optional<WorldPoint> kept = hit_place(struck_there, memory, false);
    check(kept && kept->y == 450.0f && struck_there.contact, "a number on the point of contact stays on it");
    check(hit_place(struck_there, memory, true) && !struck_there.contact && struck_there.at.x == 80.0f,
        "one that follows rides the monster's body point instead");

    memory.store32(object + kMonsterPosition, 0x7fc00000u);
    check(!monster_position(memory, object), "a position that is not a number is ignored");
    Number unread = make_number(30, Grade{}, 0);
    unread.object = object;
    check(!hit_place(unread, memory, false) && !unread.anchored, "a number never placed has no place in the world");
    check(hit_place(hit, memory, false) && !hit_place(riding, memory, true),
        "a placed number keeps its place when the monster can no longer be read, unless it follows");

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
    check(none.outline == 0.0f && number_stroke(1u, 24.0f, 0.0f).outline == 0.0f,
        "no stroke without a thickness or a unit");

    check(stack_gap_px(10.0f, 3.0f, 60.0f, 7.0f, false) == 10.0f, "the default size and thickness keep the tuned gap");
    check(std::fabs(stack_gap_px(10.0f, 3.0f, 100.0f, 3.0f, true) - 106.0f * kStackOverlap) < 1e-4f,
        "a larger number is lifted by a share of its height, overlapping a little");
    check(std::fabs(stack_gap_px(10.0f, 1.0f, 20.0f, 7.0f, true) - 34.0f * kStackOverlap) < 1e-4f,
        "a thicker number by that share of its height with the stroke");
    check(stack_gap_px(10.0f, 0.5f, 4.0f, 1.0f, true) == 5.0f,
        "a smaller number keeps the scaled gap when that is taller");
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

void store_point(psprecomp::GuestMemory &memory, std::uint32_t at, float x, float y, float z) {
    memory.store32(at, std::bit_cast<std::uint32_t>(x));
    memory.store32(at + 4u, std::bit_cast<std::uint32_t>(y));
    memory.store32(at + 8u, std::bit_cast<std::uint32_t>(z));
}

void stand_in_quest(psprecomp::GuestMemory &memory) {
    namespace layout = mhfu::game::layout;
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
    namespace layout = mhfu::game::layout;
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
    memory.store8(mhfu::game::layout::kTaskSlot + 32u, 'x');
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
    check(!damage_numbers().front().contact, "a hit with nothing to say where it landed waits for the body point");

    // A shot named by the monster: the number is on it, found at the return.
    constexpr std::uint32_t bullet = 0x09930000u;
    memory.store32(kLarge + kLastStrike, bullet);
    memory.store32(bullet, kShotClass);
    memory.store32(bullet + kShotTarget, kLarge);
    store_point(memory, bullet + kShotPlace, 40.0f, 220.0f, -15.0f);
    call(kHitAdjust, kLarge, 20u);
    damage_numbers_frame(true, &memory);
    check(damage_numbers().size() == 2u && damage_numbers().back().contact && damage_numbers().back().at.y == 220.0f,
        "a shot's hit through the routine is drawn where the shot struck");
    memory.store32(kLarge + kLastStrike, 0u);

    memory.store16(kLarge + kMonsterHealth, 1850u);
    call(kHitAdjust, kLarge, 0u);
    damage_numbers_frame(true, &memory);
    check(damage_numbers().size() == 3u && shown_total() == 150,
        "a drop from the network shows, past the monster's own update");

    memory.store16(kSmall + kMonsterHealth, 90u);
    damage_numbers_frame(true, &memory);
    check(damage_numbers().size() == 4u && shown_total() == 180, "another player's hit on a small monster shows");

    call(kStatusTick, kLarge, 10u);
    damage_numbers_frame(true, &memory);
    check(damage_numbers().size() == 5u && damage_numbers().back().poison && damage_numbers().back().amount == 10 &&
            shown_total() == 190,
        "a status tick on this console is poison: its own number, once, not counted again from the health");
    check(number_text(damage_numbers().back(), Breakdown::Split).main_ink.blue == poison_ink().blue &&
            !damage_numbers().back().contact,
        "in light purple, on the monster's body");
    damage_numbers_frame(true, &memory, std::nullopt, Whose::Mine);
    check(std::any_of(damage_numbers().begin(), damage_numbers().end(), [](const Number &n) { return n.poison; }),
        "poison shows with Mine too");

    // The monster hits this console's hunter through the same routine.
    call(kHitAdjust, 0x09820000u, 40u);
    call(kStatusTick, 0x09820000u, 5u);
    damage_numbers_frame(true, &memory);
    check(damage_numbers().size() == 5u, "damage to the hunter, and its poison, are not numbers");

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
    namespace layout = mhfu::game::layout;
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
    check(
        tr::players_line(memory) == "[hit]   players 09820000 at 1 2 3", "the players on the list and where they are");
    memory.store32(older + layout::kNodeOlder, newest);
    check(tr::players_line(memory) == "[hit]   players 09820000 at 1 2 3", "a cyclic list names each player once");
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
    memory.store32(sp + 0x2Cu, outer + 0x10u);       // inner's saved ra
    memory.store32(sp + 0x30u + 0x1Cu, leaf + 0x8u); // outer's saved ra
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

// MHFU_TRACE_HITS: the points near the monster that could be where a hit
// landed, found in the callers' frames, the records they point at and the
// monster.
void test_the_contact_candidates() {
    namespace tr = mhfu::game::hit_trace;
    psprecomp::GuestMemory memory(32u * 1024u * 1024u);
    constexpr std::uint32_t monster = 0x09000000u, sp = 0x09F00000u, record = 0x09010000u;
    auto store_point = [&memory](std::uint32_t at, float x, float y, float z) {
        memory.store32(at, std::bit_cast<std::uint32_t>(x));
        memory.store32(at + 4u, std::bit_cast<std::uint32_t>(y));
        memory.store32(at + 8u, std::bit_cast<std::uint32_t>(z));
    };
    store_point(monster + kMonsterPosition, 1000.0f, 0.0f, 2000.0f);
    psprecomp::AllegrexContext ctx{};
    ctx.gpr[4] = monster;
    ctx.gpr[29] = sp;
    check(tr::contact_candidates(memory, ctx).empty() && tr::candidates_line({}).empty(),
        "nothing above the ground near the monster is nothing to list");
    store_point(sp + 0x40u, 1100.0f, 250.0f, 2100.0f);       // in a caller's frame
    memory.store32(sp + 0x80u, record);                       // a record on the stack
    store_point(record + 0x30u, 900.0f, 400.0f, 1900.0f);    // in it
    store_point(sp + 0x100u, 1000.0f + tr::kCandidateReach + 10.0f, 250.0f, 2000.0f); // too far
    store_point(sp + 0x140u, 1050.0f, 0.0f, 2050.0f);        // on the ground: a foot
    const std::vector<tr::Candidate> found = tr::contact_candidates(memory, ctx);
    const auto has = [&found](const char *source, std::uint32_t base, std::uint32_t offset) {
        return std::any_of(found.begin(), found.end(), [&](const tr::Candidate &c) {
            return std::string(c.source) == source && c.base == base && c.offset == offset;
        });
    };
    check(has("stack", sp, 0x40u) && has("record", record, 0x30u),
        "a point above the monster is found in a caller's frame and in a record the stack points at");
    check(!has("stack", sp, 0x100u) && !has("stack", sp, 0x140u), "one too far away or on the ground is not");
    check(tr::candidates_line(found).find("stack 09F00000+0x40: 1100 250 2100 (141.421 across, 250 up);") !=
            std::string::npos,
        "each is printed with where it was found, how far across and how high");
}

// Where a hit landed: the monster's bone nearest the attacker's weapon hand,
// both skeletons in world space as the game keeps them.
// `count` bone nodes from `first`, the n-th at `place(n)`.
template <typename Place>
void store_skeleton(psprecomp::GuestMemory &memory, std::uint32_t entity, std::uint32_t first, std::uint32_t count,
    Place place) {
    memory.store32(entity + kFirstBone, first);
    for (std::uint32_t n = 0; n < count; ++n) {
        const std::uint32_t node = first + n * kBoneStride;
        memory.store32(node, 0x089BAC28u);
        const WorldPoint at = place(n);
        store_point(memory, node + kBonePlace, at.x, at.y, at.z);
    }
}

void test_where_the_hit_landed() {
    check(parse_contact(nullptr) && parse_contact("1") && !parse_contact("0"),
        "MHFU_DAMAGE_NUMBERS_CONTACT=0 always uses the body point");
    psprecomp::GuestMemory memory(32u * 1024u * 1024u);
    stand_in_quest(memory);
    constexpr std::uint32_t hunter = 0x09820000u, record = 0x09F00010u;
    store_point(memory, hunter + kMonsterPosition, 1000.0f, 0.0f, 0.0f);
    store_point(memory, kLarge + kMonsterPosition, 0.0f, 0.0f, 0.0f);
    check(entity_bones(memory, kLarge).empty() && !hit_contact(memory, kLarge, hunter),
        "a monster without a skeleton has no point of contact");

    // The hunter's hand at 700, 130, 0; the monster's bones in a row from its
    // middle out to 600 along x, the last one high on the tail.
    store_skeleton(memory, hunter, 0x09900000u, 21u,
        [](std::uint32_t n) { return n == kHandBone ? WorldPoint{700.0f, 130.0f, 0.0f} : WorldPoint{1000.0f, 50.0f, 0.0f}; });
    store_skeleton(memory, kLarge, 0x09910000u, 7u,
        [](std::uint32_t n) { return WorldPoint{static_cast<float>(n) * 100.0f, 200.0f, 0.0f}; });
    memory.store32(0x09910000u + 7u * kBoneStride, 0u); // the row ends
    const std::vector<WorldPoint> bones = entity_bones(memory, kLarge);
    check(bones.size() == 7u && bones[3].x == 300.0f && bones[3].y == 200.0f,
        "the bones run while the nodes share the first one's class word");
    const std::optional<WorldPoint> landed = hit_contact(memory, kLarge, hunter);
    check(landed && landed->x == 600.0f && landed->y == 200.0f, "the hit lands on the bone nearest the hand");

    store_point(memory, record + kAttackerPlace, 1000.0f, 0.0f, 0.0f);
    check(hit_attacker(memory, record) == hunter, "the attacker is the hunter standing where the record says");
    store_point(memory, record + kAttackerPlace, 400.0f, 0.0f, 0.0f);
    check(hit_attacker(memory, record) == 0u && hit_attacker(memory, 0u) == 0u,
        "no hunter stands there, or there is no record: no attacker");
    check(!hit_contact(memory, kLarge, 0u) && !hit_contact(memory, kLarge, kLarge), "and no point of contact");

    store_skeleton(memory, hunter, 0x09900000u, 21u,
        [](std::uint32_t n) { return n == kHandBone ? WorldPoint{5000.0f, 130.0f, 0.0f} : WorldPoint{}; });
    const char *far_rule = nullptr;
    const std::optional<WorldPoint> far = hit_contact(memory, kLarge, hunter, &far_rule);
    check(far && far->x == 600.0f && far_rule != nullptr && std::string(far_rule) == "nearest",
        "a hand out of reach of every bone still finds the part facing it");
    store_skeleton(memory, hunter, 0x09900000u, static_cast<std::uint32_t>(kHandBone), [](std::uint32_t) {
        return WorldPoint{};
    });
    memory.store32(0x09900000u + kHandBone * kBoneStride, 0u);
    check(!hit_contact(memory, kLarge, hunter), "a skeleton without the hand has no point of contact");

    // A bow's shot: its path from the entity manager, two nodes left at the
    // origin, then points from the bow at 1000, 150, 0 out along -x past the
    // monster's bones (0..600 along x, 200 up).
    constexpr std::uint32_t manager = 0x09800000u, path_nodes = 0x09920000u;
    check(shot_path(memory).empty(), "no path without one in the manager");
    memory.store32(manager + kShotPath, path_nodes);
    for (std::uint32_t n = 0; n < 8u; ++n) {
        const std::uint32_t node = path_nodes + n * kBoneStride;
        memory.store32(node, 0x089BAC28u);
        const float x = n < 2u ? 0.0f : 1000.0f - static_cast<float>(n - 2u) * 300.0f;
        const float y = n < 2u ? 0.0f : 150.0f + static_cast<float>(n - 2u) * 10.0f;
        store_point(memory, node + kPathPlace, x, y, n < 2u ? 0.0f : 40.0f);
    }
    const std::vector<WorldPoint> path = shot_path(memory);
    check(path.size() == 6u && path.front().x == 1000.0f && path.back().x == -500.0f,
        "the path skips the nodes at the origin and ends with the row");
    check(!path_contact(path, {{0.0f, 2000.0f, 0.0f}}) && !path_contact({path.front()}, bones),
        "a path that passes no bone, or a single point, has struck nothing");
    const std::optional<WorldPoint> passed = path_contact(path, bones);
    check(passed && std::fabs(passed->z - 40.0f) < 1e-3f && passed->x > -50.0f && passed->x < 650.0f,
        "the shot struck where its path passes nearest a bone");

    // The hand is out of reach; the shot from this hunter's bow is not.
    store_skeleton(memory, hunter, 0x09900000u, 21u,
        [](std::uint32_t n) { return n == kHandBone ? WorldPoint{5000.0f, 130.0f, 0.0f} : WorldPoint{}; });
    const std::optional<WorldPoint> shot = hit_contact(memory, kLarge, hunter);
    check(shot && std::fabs(shot->z - 40.0f) < 1e-3f, "a shot lands where its path crossed the monster");
    store_point(memory, hunter + kMonsterPosition, 3000.0f, 0.0f, 0.0f);
    const char *other_rule = nullptr;
    check(hit_contact(memory, kLarge, hunter, &other_rule) && std::string(other_rule) != "path",
        "a path that does not start at the attacker is someone else's");

    // A bowgun's bullet, named by the monster as the last to strike it.
    constexpr std::uint32_t bullet = 0x09930000u;
    check(!struck_by_shot(memory, kLarge), "a monster that names no bullet was not struck by one");
    memory.store32(kLarge + kLastStrike, bullet);
    memory.store32(bullet, kShotClass);
    memory.store32(bullet + kShotTarget, kLarge);
    store_point(memory, bullet + kShotPlace, 250.0f, 260.0f, 30.0f);
    const char *rule = nullptr;
    const std::optional<WorldPoint> fired = hit_contact(memory, kLarge, 0u, &rule);
    check(fired && fired->x == 250.0f && fired->y == 260.0f && rule != nullptr && std::string(rule) == "shot",
        "a shot lands where it struck, whoever fired it, before any other rule");
    memory.store32(bullet, kArrowClass);
    check(struck_by_shot(memory, kLarge).has_value(), "a bow's arrow is a shot too");
    memory.store32(bullet, kShotClass);
    memory.store32(bullet + kShotOwner, hunter);
    check(struck_by_shot(memory, kLarge, hunter).has_value(), "a shot the attacker fired is theirs");
    memory.store32(bullet + kShotOwner, 0x09860000u);
    check(!struck_by_shot(memory, kLarge, hunter), "a teammate's spent shot is not where this attacker struck");
    memory.store32(bullet + kShotTarget, kSmall);
    check(!struck_by_shot(memory, kLarge), "a shot that struck another monster is not this hit");
    memory.store32(bullet + kShotTarget, kLarge);
    memory.store32(bullet, 0x089BD35Cu);
    check(!struck_by_shot(memory, kLarge), "nor is something that is not a shot");
    memory.store32(bullet, kShotClass);
    memory.store32(bullet + kShotPlace, 0x7fc00000u);
    check(!struck_by_shot(memory, kLarge), "nor a shot with no place");
}

// The breakdown, the element colours, the critical dots and the player tints.
void test_breakdown_and_marks() {
    check(element_of(kFireBit) == Element::Fire && element_of(kWaterBit) == Element::Water &&
            element_of(kThunderBit) == Element::Thunder && element_of(kIceBit) == Element::Ice &&
            element_of(kDragonBit) == Element::Dragon,
        "each element bit names its element");
    check(element_of(0x0080u) == Element::Ice && element_of(0x0040u) == Element::Thunder &&
            element_of(0x0010u) == Element::Fire && element_of(0x0020u) == Element::Water &&
            element_of(0x0100u) == Element::Dragon,
        "an ice weapon's 0x80 is ice and 0x40 thunder, as seen in play");
    check(element_of(0u) == Element::None && element_of(0x000Fu) == Element::None,
        "no element bit, or only status bits, is no element");
    check(element_of(kDragonBit | kWaterBit) == Element::Water, "two bits name the first in the hitzone's order");
    const Element elements[] = {Element::Fire, Element::Water, Element::Thunder, Element::Ice, Element::Dragon};
    bool distinct = true;
    for (const Element a : elements)
        for (const Element b : elements) {
            const Ink x = element_ink(a), y = element_ink(b);
            if (a != b && x.red == y.red && x.green == y.green && x.blue == y.blue) distinct = false;
        }
    const Ink fire = element_ink(Element::Fire), water = element_ink(Element::Water);
    check(distinct && fire.red > fire.blue && water.blue > water.red, "each element has its own colour");

    check(!affinity_dot(Affinity::None), "an ordinary hit has no mark");
    const std::optional<Ink> pink = affinity_dot(Affinity::Positive);
    const std::optional<Ink> gray_blue = affinity_dot(Affinity::Negative);
    check(pink && pink->red == 255 && pink->blue > pink->green && pink->green > 80 && gray_blue &&
            gray_blue->blue > gray_blue->green && gray_blue->green > gray_blue->red &&
            gray_blue->blue - gray_blue->red < 80,
        "a critical hit's sparkle is pink and a negative one's a muted blue-gray");
    const std::vector<Offset> sparkle = sparkle_shape();
    bool star = sparkle.size() == 8u;
    for (std::size_t i = 0; star && i < sparkle.size(); ++i) {
        const Offset p = sparkle[i];
        // The tips on the axes, the waist between them close to the middle.
        if (i % 2u == 0u)
            star = std::fabs(p.x) + std::fabs(p.y) == 1.0f && (p.x == 0.0f || p.y == 0.0f);
        else
            star = std::fabs(p.x) < 0.3f && std::fabs(p.y) < 0.5f && p.x != 0.0f && p.y != 0.0f;
    }
    check(star && sparkle[0].x == -1.0f && sparkle[4].x == 1.0f && sparkle[2].y == -1.0f && sparkle[6].y == 1.0f,
        "the sparkle is a four-pointed star, its tips on the axes and a narrow waist");
    check(player_tint(0) && player_tint(3) && !player_tint(-1) && !player_tint(4),
        "four players have a colour and an unknown attacker none");
    check(player_tint(0)->red != player_tint(1)->red, "two players' colours differ");

    Number hit = make_number(150, grade_share(0.6f, Affinity::Positive), 0);
    hit.element = 30;
    hit.kind = Element::Thunder;
    const NumberText total = number_text(hit, Breakdown::Total);
    check(total.main == "150" && total.side.empty() && total.main_ink.green == damage_number_ink(Fill::WeakPoint).green,
        "Total is the whole hit in the hitzone's colour");
    const NumberText split = number_text(hit, Breakdown::Split);
    check(split.main == "120" && split.side == "30" && split.side_ink.red == element_ink(Element::Thunder).red &&
            split.side_ink.blue == element_ink(Element::Thunder).blue,
        "Split is the raw share with the element's beside it in its colour");
    const NumberText both = number_text(hit, Breakdown::TotalAndElement);
    check(both.main == "150" && both.side == "30", "Total + element is the whole hit with the element's share");
    Number pure = hit;
    pure.element = 150;
    const NumberText element_only = number_text(pure, Breakdown::Split);
    check(element_only.main == "150" && element_only.side.empty() &&
            element_only.main_ink.blue == element_ink(Element::Thunder).blue,
        "a hit of element alone is that share in its colour");
    const Number drop = make_number(42, Grade{}, 0);
    check(number_text(drop, Breakdown::Split).main == "42" && number_text(drop, Breakdown::Split).side.empty() &&
            number_text(drop, Breakdown::TotalAndElement).side.empty(),
        "a drop with no element share is one number in every mode");
    Number odd = hit;
    odd.element = 400;
    check(number_text(odd, Breakdown::TotalAndElement).side == "150", "a share is never more than the hit");

    check(split.side_glyph == Element::Thunder && split.main_glyph == Element::None &&
            both.side_glyph == Element::Thunder && total.side_glyph == Element::None &&
            total.main_glyph == Element::None,
        "the element share's glyph goes before the share, and Total shows none");
    check(element_only.main_glyph == Element::Thunder && element_only.side_glyph == Element::None,
        "a hit of element alone has its glyph before the number");
    check(number_text(drop, Breakdown::Split).side_glyph == Element::None &&
            number_text(drop, Breakdown::Split).main_glyph == Element::None,
        "a drop has no glyph");

    const Glyph none = element_glyph(Element::None);
    check(none.fills.empty() && none.highlights.empty() && none.grooves.empty(), "no element has no glyph");
    bool inside = true;
    for (const Element element : elements) {
        const Glyph glyph = element_glyph(element);
        if (glyph.fills.empty() || (!glyph.grooves.empty() && !(glyph.groove_width > 0.0f))) inside = false;
        const auto within = [&inside](const std::vector<Offset> &shape, std::size_t least) {
            if (shape.size() < least) inside = false;
            for (const Offset &p : shape)
                if (!(p.x >= 0.0f && p.x <= 1.0f && p.y >= 0.0f && p.y <= 1.0f)) inside = false;
        };
        for (const std::vector<Offset> &shape : glyph.fills) within(shape, 3u);
        for (const std::vector<Offset> &shape : glyph.highlights) within(shape, 3u);
        for (const std::vector<Offset> &groove : glyph.grooves) within(groove, 2u);
    }
    check(inside, "each element has a glyph, and every point of it lies in its box");
    const Glyph flame = element_glyph(Element::Fire);
    const Glyph drop_glyph = element_glyph(Element::Water);
    const Glyph bolt = element_glyph(Element::Thunder);
    const Glyph ice = element_glyph(Element::Ice);
    const Glyph dragon = element_glyph(Element::Dragon);
    check(flame.fills.size() == 1u && flame.grooves.size() == 1u && flame.grooves[0].size() > 20u,
        "fire is one flame with a spiral groove");
    check(drop_glyph.fills.size() == 1u && drop_glyph.highlights.size() == 1u && drop_glyph.grooves.empty(),
        "water is a drop with a light streak");
    check(bolt.fills.size() == 8u && bolt.fills.back().size() == 3u && bolt.fills.back()[1].x > 0.9f &&
            bolt.fills.back()[1].y > 0.9f,
        "thunder is a zigzag of four strokes and three corners, ending in an arrowhead at the lower right");
    check(
        ice.fills.size() == 2u && ice.fills[0].size() == 6u && ice.fills[1].size() == 6u && ice.highlights.size() == 1u,
        "ice is two hexagonal crystals, the long one with a light face");
    check(dragon.fills.size() == 1u && dragon.fills[0].size() == 13u && dragon.grooves.size() == 3u,
        "dragon is a cluster of crystal spikes with grooves between them");
}

// The registers at the call from kAfterDamage, as the damage routine leaves
// them.
constexpr std::uint32_t kAttack = 0x09940000u;
constexpr std::uint32_t kHitbox = 0x09950000u;

void computed_registers(psprecomp::AllegrexContext &ctx, std::uint32_t monster, std::uint32_t attacker, int total,
    float element, float hitzone) {
    ctx.gpr[31] = kAfterDamage;
    ctx.gpr[17] = static_cast<std::uint32_t>(total);
    ctx.gpr[18] = kHitbox;
    ctx.gpr[19] = monster;
    ctx.gpr[20] = attacker;
    ctx.gpr[21] = kAttack;
    ctx.fpr[26] = element;
    ctx.fpr[29] = hitzone;
}

void test_the_computed_hit() {
    check(parse_computed(nullptr) && parse_computed("1") && !parse_computed("0"),
        "computed hits are read unless MHFU_DAMAGE_NUMBERS_COMPUTED=0");
    psprecomp::GuestMemory memory(32u * 1024u * 1024u);
    stand_in_quest(memory);
    constexpr std::uint32_t hunter = 0x09820000u;
    memory.store32(kLarge + kHealthScale, std::bit_cast<std::uint32_t>(1.0f));
    memory.store16(kHitbox + kHitboxPart, 0x000Bu);
    memory.store16(kAttack + kAttackElements, kFireBit);
    memory.store8(kAttack + kAttackMarks, kCriticalMark | 0x23u);
    psprecomp::AllegrexContext ctx;
    computed_registers(ctx, kLarge, hunter, 87, 21.9f, 0.62f);

    std::optional<ComputedHit> hit = computed_hit(memory, ctx, kAttackFlagCheck);
    check(hit && hit->monster == kLarge && hit->attacker == hunter && hit->attack == kAttack && hit->part == 3,
        "the hit is on the routine's monster and part, by its attacker");
    check(
        hit && hit->total == 87 && hit->element == 21 && hit->kind == Element::Fire, "the element share is trunc(f26)");
    check(hit && hit->affinity == Affinity::Positive && std::fabs(hit->hitzone - 0.62f) < 1e-6f,
        "the critical mark and the effective hitzone are the attack's and f29");
    check(!computed_hit(memory, ctx, kHitAdjust), "another call is no computed hit");
    ctx.gpr[31] = 0x09C44000u;
    check(!computed_hit(memory, ctx, kAttackFlagCheck), "the same function called from elsewhere is no hit");

    memory.store8(kAttack + kAttackMarks, kNegativeMark | 0x23u);
    computed_registers(ctx, kLarge, hunter, 40, 0.0f, 0.3f);
    hit = computed_hit(memory, ctx, kAttackFlagCheck);
    check(hit && hit->affinity == Affinity::Negative && hit->element == 0 && hit->kind == Element::None,
        "a negative critical without element");
    computed_registers(ctx, kLarge, hunter, 40, 0.6f, 0.3f);
    check(computed_hit(memory, ctx, kAttackFlagCheck)->element == 0 &&
            std::fabs(computed_hit(memory, ctx, kAttackFlagCheck)->element_exact - 0.6f) < 1e-6f,
        "under one point of element is none, as the game adds it, and the value before truncation is kept");
    computed_registers(ctx, kLarge, hunter, 40, 90.0f, 0.3f);
    check(computed_hit(memory, ctx, kAttackFlagCheck)->element == 40, "the element share is never more than the total");
    computed_registers(ctx, kLarge, hunter, 40, std::numeric_limits<float>::quiet_NaN(), 0.3f);
    check(computed_hit(memory, ctx, kAttackFlagCheck)->element == 0, "an element that is not a number is none");
    computed_registers(ctx, kLarge, hunter, 0x10000 + 25, 0.0f, 0.3f);
    check(computed_hit(memory, ctx, kAttackFlagCheck)->total == 25, "the total is the halfword the part got");
    computed_registers(ctx, kLarge, hunter, 0, 0.0f, 0.3f);
    check(!computed_hit(memory, ctx, kAttackFlagCheck), "nothing added is no hit");
    computed_registers(ctx, kLarge, hunter, 0xFFF0, 0.0f, 0.3f);
    check(!computed_hit(memory, ctx, kAttackFlagCheck), "a heal is no hit");
    computed_registers(ctx, 0x00001000u, hunter, 40, 0.0f, 0.3f);
    check(!computed_hit(memory, ctx, kAttackFlagCheck), "a monster outside RAM is no hit");
    memory.store32(kLarge + kHealthScale, std::bit_cast<std::uint32_t>(std::numeric_limits<float>::infinity()));
    computed_registers(ctx, kLarge, hunter, 40, 0.0f, 0.3f);
    check(!computed_hit(memory, ctx, kAttackFlagCheck), "nor one whose scale is not a number");
    memory.store32(kLarge + kHealthScale, std::bit_cast<std::uint32_t>(1.0f));

    ComputedHit scaled;
    scaled.total = 87;
    scaled.element = 21;
    scaled.scale = 1.0f;
    check(hit_amount(scaled).amount == 87 && hit_amount(scaled).element == 21, "a scale of 1 shows the hit as it is");
    scaled.scale = 0.7f;
    check(hit_amount(scaled).amount == 60 && hit_amount(scaled).element == 14,
        "a scale takes the health routine's truncated share");
    scaled.total = 1;
    scaled.element = 0;
    scaled.scale = 0.2f;
    check(hit_amount(scaled).amount == 1, "a part that was hit loses at least 1");

    check(player_of(memory, hunter) == 0, "this console's hunter is player 0");
    check(player_of(memory, 0u) == -1 && player_of(memory, kLarge) == -1, "no attacker, or a monster, is not known");
    namespace layout = mhfu::game::layout;
    constexpr std::uint32_t second = 0x09870000u, first = 0x09860000u;
    for (const std::uint32_t remote : {second, first}) memory.store32(remote + layout::kVtable, 0x09A60000u);
    memory.store32(0x09810500u + layout::kNodeEntity, second);
    memory.store32(0x09810600u + layout::kNodeEntity, first);
    memory.store32(0x09810400u + layout::kNodeOlder, 0x09810500u);
    memory.store32(0x09810500u + layout::kNodeOlder, 0x09810600u);
    check(player_of(memory, first) == 1 && player_of(memory, second) == 2,
        "the other hunters by the order of their entities, whatever the list's");
    memory.store32(0x09810400u + layout::kNodeOlder, 0u);

    ComputedHit weak;
    weak.monster = kLarge;
    weak.total = 150;
    weak.element = 30;
    weak.kind = Element::Ice;
    weak.affinity = Affinity::Positive;
    weak.hitzone = 0.45f;
    const Number number = number_for(weak, 2);
    check(number.amount == 150 && number.element == 30 && number.kind == Element::Ice && number.player == 2 &&
            number.object == kLarge && number.grade.fill == Fill::WeakPoint &&
            number.grade.affinity == Affinity::Positive,
        "the number carries the hit's amount, element, grade and player");
    weak.hitzone = 0.44f;
    weak.element = 0;
    check(number_for(weak, -1).grade.fill == Fill::Ordinary && number_for(weak, -1).kind == Element::None,
        "an ordinary part, and no element without a share");

    // A shot is its own place; a blade's hit uses the hand's bone, or none.
    memory.store32(kAttack, kShotClass);
    store_point(memory, kAttack + kShotPlace, 12.0f, 130.0f, -4.0f);
    weak.attack = kAttack;
    weak.attacker = hunter;
    const char *rule = nullptr;
    const std::optional<WorldPoint> shot = computed_contact(memory, weak, &rule);
    check(shot && shot->y == 130.0f && rule != nullptr && std::string(rule) == "shot",
        "a shot's hit is where the shot is, before the monster names it");
    weak.attacker = 0u;
    check(computed_contact(memory, weak) && computed_contact(memory, weak)->x == 12.0f, "also with no attacker known");
    memory.store32(kAttack, 0x09A70000u);
    check(!computed_contact(memory, weak), "another attack with no attacker has no contact point");
}

void test_pending_hits() {
    std::vector<PendingHit> pending;
    Number a = make_number(10, Grade{}, 0);
    a.object = kLarge;
    Number b = make_number(20, Grade{}, 0);
    b.object = kSmall;
    Number c = make_number(30, Grade{}, 0);
    c.object = kLarge;
    pending = {{a, 0.0f}, {b, 0.0f}, {c, 0.0f}};
    check(numbers_for_drop(pending, kLarge, std::nullopt, 0).empty() && pending.size() == 3u,
        "no drop shows nothing and keeps the hits for one");
    const std::vector<Number> taken = numbers_for_drop(pending, kLarge, make_number(25, Grade{}, 0), 4);
    check(taken.size() == 2u && taken[0].amount == 10 && taken[0].lane == 4 && taken[1].amount == 30 &&
            taken[1].lane == 5 && pending.size() == 1u && pending[0].number.object == kSmall,
        "a drop shows the monster's computed hits, each in a lane, and not the drop");
    Number drop = make_number(25, Grade{}, 0);
    drop.object = kLarge;
    const std::vector<Number> plain = numbers_for_drop(pending, kLarge, drop, 2);
    check(plain.size() == 1u && plain[0].amount == 25 && plain[0].lane == 2, "without them the drop is the number");

    age_pending(pending, kPendingSeconds * 0.5f);
    check(pending.size() == 1u, "a hit waits a while for its drop");
    age_pending(pending, -5.0f);
    check(pending.size() == 1u && pending[0].age == kPendingSeconds * 0.5f, "time does not run backwards");
    age_pending(pending, kPendingSeconds * 0.5f);
    check(pending.empty(), "and is dropped once kPendingSeconds have passed");

    Number mine = make_number(1, Grade{}, 0);
    mine.player = 0;
    Number theirs = make_number(2, Grade{}, 1);
    theirs.player = 1;
    Number unknown = make_number(3, Grade{}, 2);
    const std::vector<Number> all{mine, theirs, unknown};
    check(numbers_shown(all, Whose::Everyone).size() == 3u, "Everyone shows every hit");
    const std::vector<Number> only = numbers_shown(all, Whose::Mine);
    check(only.size() == 1u && only[0].amount == 1, "Mine shows only this console's hunter's");

    check(parse_poison(nullptr) && parse_poison("1") && !parse_poison("0"),
        "poison shows unless MHFU_DAMAGE_NUMBERS_POISON=0");
    const std::optional<Number> tick = poison_number(kLarge, 500, 488, 3);
    check(tick && tick->amount == 12 && tick->poison && tick->object == kLarge && tick->lane == 3 &&
            tick->player == -1 && tick->grade.affinity == Affinity::None && tick->element == 0,
        "a poison tick is its own number, with no mark and no element");
    check(!poison_number(kLarge, 500, 500, 0) && !poison_number(kLarge, 0, -12, 0),
        "a tick that takes nothing, or one on a monster already down, shows nothing");
    const NumberText purple = number_text(*tick, Breakdown::TotalAndElement);
    const Ink light = poison_ink();
    check(purple.main == "12" && purple.side.empty() && purple.main_ink.red == light.red &&
            purple.main_ink.blue == light.blue && light.blue > light.green && light.red > light.green &&
            light.green > 120,
        "it is light purple, and one number in every mode");
    check(numbers_shown({*tick, theirs}, Whose::Mine).size() == 1u, "Mine keeps poison and hides another's hit");

    // Lanes: only numbers on the same spot of the same monster stack.
    const auto at = [](std::uint32_t object, float x, bool anchored = true) {
        Number n = make_number(5, Grade{}, 7);
        n.object = object;
        n.at = {x, 100.0f, 0.0f};
        n.anchored = anchored;
        return n;
    };
    std::vector<Number> spots;
    remember_number(spots, at(kLarge, 0.0f));
    remember_number(spots, at(kLarge, 30.0f));
    remember_number(spots, at(kLarge, 400.0f));
    remember_number(spots, at(kSmall, 0.0f));
    remember_number(spots, at(kLarge, 10.0f));
    check(spots[0].lane == 0 && spots[1].lane == 1 && spots[4].lane == 2,
        "hits on the same spot stack, whatever lane they came with");
    check(spots[2].lane == 0 && spots[3].lane == 0, "a hit on another part or another monster starts its own stack");
    remember_number(spots, at(kLarge, 400.0f, false));
    check(spots.back().lane == 3, "a number without its point yet shares the monster's stack");
}

void stand_in_flag_check(psprecomp::Runtime &, psprecomp::AllegrexContext &ctx) {
    ctx.gpr[2] = 0u;
}

// Through the watched calls: the hit the game computed is the number when the
// health falls, also past the health that was left.
void test_computed_hits_are_the_numbers() {
    psprecomp::Runtime runtime;
    psprecomp::GuestMemory &memory = runtime.memory();
    stand_in_quest(memory);
    constexpr std::uint32_t hunter = 0x09820000u;
    memory.store32(kLarge + kHealthScale, std::bit_cast<std::uint32_t>(1.0f));
    memory.store32(kSmall + kHealthScale, std::bit_cast<std::uint32_t>(1.0f));
    memory.store16(kAttack + kAttackElements, kWaterBit);
    memory.store8(kAttack + kAttackMarks, kCriticalMark | 0x23u);
    runtime.register_function(kHitAdjust, &routine_takes_a1, "recomp_unit_stand_in_hit");
    runtime.register_function(kAttackFlagCheck, &stand_in_flag_check, "recomp_unit_stand_in_flag_check");
    auto hit_routine = [&runtime](std::uint32_t object, std::uint32_t amount) {
        runtime.cpu().gpr[4] = object;
        runtime.cpu().gpr[5] = amount;
        runtime.cpu().gpr[31] = 0u;
        runtime.cpu().pc = kHitAdjust;
        (void)runtime.invoke_chained_call(runtime.cpu());
    };
    auto computed = [&runtime](std::uint32_t monster, std::uint32_t attacker, int total, float element) {
        computed_registers(runtime.cpu(), monster, attacker, total, element, 0.7f);
        runtime.cpu().pc = kAttackFlagCheck;
        (void)runtime.invoke_chained_call(runtime.cpu());
    };

    damage_numbers_frame(true, &memory);
    computed(kLarge, hunter, 90, 25.5f);
    computed(kLarge, hunter, 60, 0.0f);
    damage_numbers_frame(true, &memory);
    check(damage_numbers().empty(), "a computed hit waits for the health to fall");
    hit_routine(kLarge, 150u);
    damage_numbers_frame(true, &memory);
    check(damage_numbers().size() == 2u && damage_numbers()[0].amount == 90 && damage_numbers()[0].element == 25 &&
            damage_numbers()[0].kind == Element::Water && damage_numbers()[0].player == 0 &&
            damage_numbers()[0].grade.fill == Fill::WeakPoint &&
            damage_numbers()[0].grade.affinity == Affinity::Positive && damage_numbers()[1].amount == 60 &&
            damage_numbers()[1].lane == damage_numbers()[0].lane + 1,
        "each computed hit is its own number when the health falls, with its share, grade and player");

    // A killing blow: 2000 computed, 1850 left.
    computed(kLarge, hunter, 2000, 0.0f);
    hit_routine(kLarge, 0u);
    damage_numbers_frame(true, &memory);
    check(damage_numbers().size() == 2u, "the monster's own update, with nothing taken off, shows nothing yet");
    hit_routine(kLarge, 1850u);
    damage_numbers_frame(true, &memory);
    check(damage_numbers().size() == 3u && damage_numbers().back().amount == 2000,
        "a killing blow shows the whole hit, not the health that was left");
    computed(kLarge, hunter, 30, 0.0f);
    hit_routine(kLarge, 0u);
    damage_numbers_frame(true, &memory);
    check(damage_numbers().size() == 3u, "a monster already down shows nothing");

    // Another hunter's hit, with Mine and Everyone.
    namespace layout = mhfu::game::layout;
    constexpr std::uint32_t remote = 0x09860000u;
    memory.store32(remote + layout::kVtable, 0x09A60000u);
    memory.store32(0x09810500u + layout::kNodeEntity, remote);
    memory.store32(0x09810400u + layout::kNodeOlder, 0x09810500u);
    memory.store16(kSmall + kMonsterHealth, 120u);
    damage_numbers_frame(true, &memory);
    computed(kSmall, remote, 15, 0.0f);
    hit_routine(kSmall, 15u);
    damage_numbers_frame(true, &memory, std::nullopt, Whose::Mine);
    check(std::none_of(
              damage_numbers().begin(), damage_numbers().end(), [](const Number &n) { return n.object == kSmall; }) &&
            !damage_numbers().empty(),
        "Mine hides another hunter's hit and keeps this hunter's");
    damage_numbers_frame(true, &memory, std::nullopt, Whose::Everyone);
    check(std::any_of(damage_numbers().begin(), damage_numbers().end(),
              [](const Number &n) { return n.object == kSmall && n.player == 1 && n.amount == 15; }),
        "Everyone shows it as player 1's");

    // A hit computed here and applied by another console arrives as a drop.
    computed(kSmall, hunter, 40, 0.0f);
    memory.store16(kSmall + kMonsterHealth, 70u);
    damage_numbers_frame(true, &memory);
    check(std::any_of(damage_numbers().begin(), damage_numbers().end(),
              [](const Number &n) { return n.object == kSmall && n.player == 0 && n.amount == 40; }),
        "a drop from the network shows the hit this console computed");

    // A hunter struck through the same routine is not a number.
    const std::size_t before = damage_numbers().size();
    computed(hunter, remote, 25, 0.0f);
    hit_routine(hunter, 25u);
    damage_numbers_frame(true, &memory);
    check(damage_numbers().size() == before, "a hunter's damage is not a number");

    damage_numbers_frame(false, &memory);
    memory.store32(0x09810400u + layout::kNodeOlder, 0u);
}

// The game's own point of contact: the collision pass's buffer, read from the
// damage routine's call, and used when it is on the monster.
void test_the_game_contact() {
    check(parse_game_contact(nullptr) && parse_game_contact("1") && !parse_game_contact("0"),
        "the game's point is used unless MHFU_DAMAGE_NUMBERS_GAME_CONTACT=0");
    psprecomp::GuestMemory memory(32u * 1024u * 1024u);
    stand_in_quest(memory);
    constexpr std::uint32_t hunter = 0x09820000u, sp = 0x09F80000u;
    memory.store32(kLarge + kHealthScale, std::bit_cast<std::uint32_t>(1.0f));
    store_point(memory, kLarge + kMonsterPosition, 0.0f, 0.0f, 0.0f);
    psprecomp::AllegrexContext ctx;
    computed_registers(ctx, kLarge, hunter, 50, 0.0f, 0.5f);
    ctx.gpr[29] = sp;
    check(kDamageFrame + kContactBuffer == 0x200u, "the point is 0x200 above the damage routine's stack pointer");
    store_point(memory, sp + kDamageFrame + kContactBuffer, 310.0f, 240.0f, 15.0f);
    const std::optional<WorldPoint> read = game_contact(memory, ctx);
    check(read && read->x == 310.0f && read->y == 240.0f && read->z == 15.0f, "the collision pass's point is read");
    std::optional<ComputedHit> hit = computed_hit(memory, ctx, kAttackFlagCheck);
    check(hit && hit->landed && hit->landed->x == 310.0f, "the computed hit carries it");
    memory.store32(sp + kDamageFrame + kContactBuffer + 4u, 0x7FC00000u);
    check(!game_contact(memory, ctx), "a point that is not a number is not read");
    ctx.gpr[29] = 0x00000100u;
    check(!game_contact(memory, ctx), "nor one outside RAM");
    ctx.gpr[29] = sp;
    store_point(memory, sp + kDamageFrame + kContactBuffer, 310.0f, 240.0f, 15.0f);

    // Without a skeleton the monster's position decides.
    float nearest = 0.0f;
    check(on_the_monster(memory, kLarge, {310.0f, 240.0f, 15.0f}, &nearest) && nearest == -1.0f,
        "without a skeleton, a point near the monster is on it");
    check(
        !on_the_monster(memory, kLarge, {kGameContactAcross + 10.0f, 0.0f, 0.0f}), "and one far across from it is not");
    store_skeleton(memory, kLarge, 0x09910000u, 7u,
        [](std::uint32_t n) { return WorldPoint{static_cast<float>(n) * 100.0f, 200.0f, 0.0f}; });
    memory.store32(0x09910000u + 7u * kBoneStride, 0u);
    check(on_the_monster(memory, kLarge, {310.0f, 240.0f, 15.0f}, &nearest) && nearest > 40.0f && nearest < 45.0f,
        "with a skeleton, a point near a bone is on it, its distance to the nearest given");
    check(!on_the_monster(memory, kLarge, {600.0f + kGameContactReach + 10.0f, 200.0f, 0.0f}),
        "and one beyond kGameContactReach of every bone is not");

    hit = computed_hit(memory, ctx, kAttackFlagCheck);
    const char *rule = nullptr;
    std::optional<WorldPoint> at = computed_contact(memory, *hit, &rule);
    check(at && at->x == 310.0f && rule != nullptr && std::string(rule) == "game",
        "the game's point comes first, where it put the spark");
    // A shot whose own place differs: still the game's point first.
    memory.store32(kAttack, kShotClass);
    store_point(memory, kAttack + kShotPlace, 12.0f, 130.0f, -4.0f);
    at = computed_contact(memory, *hit, &rule);
    check(at && at->x == 310.0f && std::string(rule) == "game", "also for a shot");
    at = computed_contact(memory, *hit, &rule, false);
    check(at && at->x == 12.0f && std::string(rule) == "shot", "turned off, the shot's own place again");
    ComputedHit off = *hit;
    off.landed = WorldPoint{5000.0f, 200.0f, 0.0f};
    at = computed_contact(memory, off, &rule);
    check(at && at->x == 12.0f && std::string(rule) == "shot", "a point off the monster falls back to the rules");
    off.landed.reset();
    at = computed_contact(memory, off, &rule);
    check(at && at->x == 12.0f && std::string(rule) == "shot", "and so does no point at all");
    memory.store32(kAttack, 0u);
}

int main() {
    test_sharpness_and_colour();
    test_the_drop();
    test_text_and_place();
    test_the_older_frame();
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
    test_the_contact_candidates();
    test_where_the_hit_landed();
    test_breakdown_and_marks();
    test_the_computed_hit();
    test_pending_hits();
    test_computed_hits_are_the_numbers();
    test_the_game_contact();
    if (failures != 0) {
        std::cerr << failures << " failure(s)\n";
        return 1;
    }
    std::cout << "damage number tests passed\n";
    return 0;
}
