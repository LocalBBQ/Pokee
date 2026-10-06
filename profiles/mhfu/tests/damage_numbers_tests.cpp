// Damage numbers: the hitzone colour, the health drop, and the hit watch.
// No game data. Guest memory is a stand-in.
#include "game/damage_numbers.hpp"

#include "psprecomp/runtime.hpp"

#include <bit>
#include <iostream>

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
    check(first.x == 0.40f && first.y == 0.55f && first.alpha == 1.0f, "a number starts on the monster");
    Number left = on;
    left.lane = 1;
    Number right = on;
    right.lane = 2;
    check(number_anchor(left).x < first.x && number_anchor(right).x > first.x,
        "the next hits step a little to either side of the monster");
    Number risen = on;
    risen.age = kNumberSeconds * 0.5f;
    check(number_anchor(risen).y < first.y && number_anchor(risen).alpha == 1.0f, "a number rises and holds its ink");
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
        "the perspective draw with the most vertices is the world camera, once");
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

} // namespace

int main() {
    test_sharpness_and_colour();
    test_the_drop();
    test_text_and_place();
    test_the_hit_watch();
    test_health_halfword();
    test_on_the_monster();
    test_the_setting_does_not_slow_direct_chains();
    if (failures != 0) {
        std::cerr << failures << " failure(s)\n";
        return 1;
    }
    std::cout << "damage number tests passed\n";
    return 0;
}
