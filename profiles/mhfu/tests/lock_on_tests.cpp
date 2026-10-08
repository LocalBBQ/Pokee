// Lock-on (camera/lock_on.hpp) without game code or game data: a stand-in
// for the quest overlay's header, the entity list and the camera, and the
// real camera driver dispatching the camera update.
#include "camera/camera_input.hpp"
#include "camera/game_camera.hpp"
#include "camera/lock_on.hpp"
#include "input/bindings.hpp"
#include "input/presets.hpp"
#include "settings/settings.hpp"
#include "psprecomp/runtime.hpp"

#include <bit>
#include <cmath>
#include <cstring>
#include <iostream>
#include <vector>

namespace mhfu::settings {
Settings &current() {
    static Settings settings;
    return settings;
}
}

namespace {
using namespace mhfu::camera;
namespace game = mhfu::camera::lock_on_layout;

constexpr std::uint32_t helper = 0x08886E50u;
constexpr std::uint32_t camera_address = 0x08900000u;
constexpr std::uint32_t parent_address = camera_address - 0xA0u;
constexpr std::uint32_t preset_address = 0x08902000u;
constexpr std::uint32_t monster_a = 0x09000000u;
constexpr std::uint32_t monster_b = 0x09001000u;
constexpr std::uint32_t companion = 0x09002000u;
constexpr std::uint32_t player = 0x09003000u;
constexpr std::uint32_t small = 0x09004000u;
constexpr std::uint32_t manager = 0x09100000u;
constexpr std::uint32_t large_vtable = 0x09300000u;
constexpr std::uint32_t small_vtable = 0x09300100u;
constexpr std::uint32_t node_b = 0x09200000u;
constexpr std::uint32_t node_a = 0x09200040u;
constexpr std::uint32_t node_small = 0x09200080u;
constexpr std::uint32_t node_companion = 0x092000C0u;
constexpr std::uint32_t node_player = 0x09200100u;
constexpr std::uint32_t node_again = 0x09200140u;
int failures{};

void check(bool condition, const char *message) {
    if (!condition) {
        ++failures;
        std::cerr << "FAIL: " << message << '\n';
    }
}

void original(psprecomp::Runtime &, psprecomp::AllegrexContext &ctx) {
    ctx.pc = ctx.gpr[31];
}

void write_float(psprecomp::GuestMemory &memory, std::uint32_t address, float value) {
    memory.store32(address, std::bit_cast<std::uint32_t>(value));
}

void place(psprecomp::GuestMemory &memory, std::uint32_t monster, float x, float z, std::int16_t health = 1000,
    std::int16_t most = 1000) {
    write_float(memory, monster + game::kPosition, x);
    write_float(memory, monster + game::kPosition + 4u, 0.0f);
    write_float(memory, monster + game::kPosition + 8u, z);
    memory.store16(monster + game::kHealth, static_cast<std::uint16_t>(health));
    memory.store16(monster + game::kMostHealth, static_cast<std::uint16_t>(most));
}

// Newest node first, each +4 the next older one, as the game's insert leaves it.
void link(psprecomp::GuestMemory &memory, std::uint32_t node, std::uint32_t entity, std::uint32_t older,
    std::uint8_t type) {
    memory.store32(node + game::kNodeOlder, older);
    memory.store32(node + game::kNodeEntity, entity);
    memory.store8(node + game::kNodeType, type);
}

// The quest overlay's header, as the game has it in the task slot.
void start_quest(psprecomp::GuestMemory &memory) {
    memory.store32(game::kTaskSlot, 0x336F574Du);
    memory.store32(game::kTaskSlot + 8u, game::kTaskSlot);
    const char name[] = "game_task.ovl";
    for (std::uint32_t i = 0; i < sizeof(name); ++i)
        memory.store8(game::kTaskSlot + 32u + i, static_cast<std::uint8_t>(name[i]));
}

struct Fixture {
    // The quest overlay and its data sit above 32 MiB, as on a PSP with 64.
    psprecomp::Runtime runtime{64u * 1024u * 1024u};
    psprecomp::AllegrexContext ctx{};
    Fixture() {
        mhfu::settings::current() = {};
        mhfu::settings::current().analog_camera = false;
        runtime.register_generated_unit(29u, 0x08886000u, 0x1000u, &original, nullptr);
        runtime.register_function(helper, &original, "recomp_unit_test");
        auto &memory = runtime.memory();
        for (const CodeWord &word : game_code_signature()) memory.store32(word.address, word.word);
        check(prepare_game_camera(runtime, &original), "the matching game code is accepted");
        memory.store32(camera_address + 0x70u, preset_address);
        memory.store16(camera_address + 0x80u, 0u);
        memory.store16(camera_address + 0x82u, 0u);
        memory.store8(camera_address + 0x91u, 0xFFu);
        write_float(memory, preset_address + 4u, -40.0f);
        write_float(memory, preset_address + 8u, 490.0f);
        write_float(memory, preset_address + 0x10u, 190.0f);
        write_float(memory, camera_address + 4u, 150.0f);
        // The hunter at the origin, the camera looking along -z (yaw 0).
        write_float(memory, camera_address + 0x10u, 0.0f);
        write_float(memory, camera_address + 0x14u, 190.0f);
        write_float(memory, camera_address + 0x18u, 0.0f);
        ctx.gpr[4] = parent_address;
        ctx.gpr[31] = 0x09C00000u;
        start_quest(memory);
        memory.store32(game::kEntityList, manager);
        link(memory, node_b, monster_b, node_a, 0u);
        link(memory, node_a, monster_a, node_small, 0u);
        link(memory, node_small, small, node_companion, 0u);
        link(memory, node_companion, companion, node_player, 0u);
        link(memory, node_player, player, 0u, game::kPlayerNode);
        link(memory, node_again, monster_b, node_b, 0u); // the same monster, inserted twice
        memory.store32(manager + game::kListHead, node_again);
        memory.store32(large_vtable + game::kVtableMethod, game::kEmOverlay);
        memory.store32(small_vtable + game::kVtableMethod, game::kTaskSlot);
        place(memory, monster_a, 1000.0f, 0.0f);        // to the right, 90 degrees off the view
        place(memory, monster_b, 0.0f, -3000.0f);       // straight ahead, farther
        memory.store32(monster_a + game::kVtable, large_vtable);
        memory.store32(monster_b + game::kVtable, large_vtable);
        place(memory, small, 200.0f, -200.0f); // health, but a small monster's vtable
        memory.store32(small + game::kVtable, small_vtable);
        place(memory, companion, 0.0f, -100.0f, 50, 0); // no most health: not a large monster
        memory.store32(companion + game::kVtable, large_vtable);
        place(memory, player, 0.0f, -50.0f); // a player node, even with health
        memory.store32(player + game::kVtable, large_vtable);
    }
    void update() {
        check(runtime.invoke_isolated_aot(helper, ctx), "the camera update dispatches");
    }
    void frame(float stick_x = 0.0f) {
        set_rate(Source::Stick, stick_x, 0.0f);
        game_camera_frame(runtime);
        advance(1.0f / 30.0f, game_camera_degrees_per_second());
        update();
    }
    std::uint16_t yaw() { return runtime.memory().load16(camera_address + 0x82u); }
};

void test_geometry() {
    check(yaw_towards({0, 0, 0}, {0, 0, -100}) == 0u, "looking along -z is yaw 0");
    check(yaw_towards({0, 0, 0}, {100, 0, 0}) == 49152u, "looking along +x is three quarters of a turn");
    check(yaw_towards({0, 0, 0}, {0, 0, 100}) == 32768u, "looking along +z is half a turn");
    check(yaw_difference(65000u, 500u) == 1036, "differences go the short way round");
    check(yaw_difference(500u, 65000u) == -1036, "and both ways");
    std::uint16_t yaw = 0u;
    int largest = 0;
    for (int i = 0; i < 60; ++i) {
        const std::uint16_t next = lock_on_ease_yaw(yaw, 30000u);
        largest = std::max(largest, std::abs(yaw_difference(yaw, next)));
        yaw = next;
    }
    check(yaw == 30000u, "easing reaches the wanted yaw");
    check(largest <= 2048, "and never turns more than 11.25 degrees in one update");
    check(lock_on_ease_yaw(100u, 120u) == 120u, "a small rest is closed at once");
    check(lock_on_ease_yaw(1000u, 20000u, 0.0f) == 1000u, "with the monster on top of the hunter the yaw holds");
    check(yaw_difference(0u, lock_on_ease_yaw(0u, 32868u, 1.0f, 1)) > 0,
        "near half a turn, the turn keeps the way it was going");
    check(yaw_difference(0u, lock_on_ease_yaw(0u, 32868u)) < 0, "and otherwise takes the short way");
}

void test_monsters_and_pick() {
    Fixture f;
    auto &memory = f.runtime.memory();
    std::vector<LockMonster> found = lock_on_monsters(memory);
    check(found.size() == 2u, "a small monster, a companion and a player are not tracked");
    memory.store32(node_a + game::kNodeOlder, node_b);
    check(lock_on_monsters(memory).size() == 2u, "a cyclic list is walked once");
    memory.store32(node_a + game::kNodeOlder, node_small);
    const Vec3 origin{0.0f, 190.0f, 0.0f};
    check(lock_on_pick(found, origin, 0u, std::nullopt, {}) == monster_b,
        "a tap picks the monster in view over a nearer one out of view");
    check(lock_on_pick(found, origin, 49152u, std::nullopt, {}) == monster_a, "or the one the camera looks at");
    check(lock_on_pick(found, origin, 32768u, std::nullopt, {}) == monster_a, "with none in view, the nearest");
    check(lock_on_pick(found, origin, 0u, monster_a, {monster_a}) == monster_b, "a second tap moves to the next one");
    check(!lock_on_pick(found, origin, 0u, monster_b, {monster_a, monster_b}), "after the last, a tap lets go");
    check(lock_on_pick(found, origin, 0u, monster_b, {monster_b}) == monster_a, "cycling wraps round the table");
    place(memory, monster_a, 1000.0f, 0.0f, 0, 1000);
    check(lock_on_monsters(memory).size() == 1u, "a dead monster cannot be locked");
    place(memory, monster_b, 0.0f, -3000.0f, 1000, 0);
    check(lock_on_monsters(memory).size() == 1u && lock_on_monsters(memory)[0].address == small,
        "with no marked monster left, the healthy small one remains");
    memory.store8(game::kTaskSlot + 32u, 'l');
    place(memory, monster_b, 0.0f, -3000.0f);
    check(lock_on_monsters(memory).empty(), "and nothing outside a quest");
}

void test_lock_follows_and_lets_go() {
    Fixture f;
    auto &memory = f.runtime.memory();
    f.frame();
    check(!game_camera_driving(), "nothing is driven before a tap, with the analog camera off");
    lock_on_tap();
    f.frame();
    check(lock_on_status().locked && lock_on_status().target == monster_b, "a tap locks onto the monster in view");
    check(game_camera_driving(), "the second stick is kept from the game while locked");
    // The monster walks round to the hunter's right; the camera follows.
    place(memory, monster_b, 3000.0f, 0.0f);
    for (int i = 0; i < 40; ++i) f.frame();
    check(std::abs(yaw_difference(f.yaw(), 49152u)) < 64, "the camera turns to keep the monster ahead");
    check(memory.load16(camera_address + 0x80u) == f.yaw(), "the game's target yaw is kept with it");
    // A push of the stick is the player taking the camera back.
    f.frame(1.0f);
    f.frame(1.0f);
    check(!lock_on_status().locked, "turning by hand lets go");
    f.frame();
    check(!game_camera_driving(), "and the stick goes back to the game");

    lock_on_tap();
    f.frame();
    check(lock_on_status().target == monster_a, "locked again, onto the nearer monster now in view");
    place(memory, monster_a, 1000.0f, 0.0f, 0, 1000);
    f.frame();
    check(!lock_on_status().locked, "a monster killed lets go");
    lock_on_tap();
    f.frame();
    check(lock_on_status().target == monster_b, "the next tap takes the one that is left");
    place(memory, monster_b, 8000.0f, 0.0f);
    f.frame();
    check(!lock_on_status().locked, "one farther than the release distance lets go");
    place(memory, monster_b, 0.0f, -3000.0f, 0, 1000);

    place(memory, monster_a, 1000.0f, 0.0f);
    lock_on_tap();
    f.frame();
    check(lock_on_status().locked, "locked once more");
    memory.store16(camera_address + 0x84u, 0x0080u); // the D-pad's turn
    f.frame();
    memory.store16(camera_address + 0x84u, 0u);
    check(!lock_on_status().locked, "a camera command of the game's lets go");

    lock_on_tap();
    f.frame();
    lock_on_tap();
    f.frame();
    check(!lock_on_status().locked, "with one monster a second tap lets go");
}

void test_unmarked_monsters_still_lock() {
    Fixture f;
    auto &memory = f.runtime.memory();
    // The quest that loaded em75 kept game_task methods on every list entity.
    memory.store32(large_vtable + game::kVtableMethod, game::kTaskSlot);
    std::vector<LockMonster> found = lock_on_monsters(memory);
    check(found.size() == 3u, "with nothing marked large, a healthy small monster is included");
    memory.store16(node_a + game::kNodeClass, 34u);
    found = lock_on_monsters(memory);
    check(found.size() == 1u && found[0].address == monster_a,
        "a class id of 34 keeps every unmarked monster out");
}

void test_tap_waits_for_the_monster() {
    Fixture f;
    auto &memory = f.runtime.memory();
    place(memory, monster_a, 1000.0f, 0.0f, 0, 1000);
    place(memory, monster_b, 0.0f, -3000.0f, 0, 1000);
    place(memory, small, 200.0f, -200.0f, 0, 1000);
    lock_on_tap();
    f.frame();
    check(!lock_on_status().locked, "a tap before the monster exists does not lock yet");
    place(memory, monster_b, 0.0f, -3000.0f);
    f.frame();
    check(lock_on_status().locked && lock_on_status().target == monster_b,
        "and locks when the monster appears a moment later");

    lock_on_release("");
    place(memory, monster_b, 0.0f, -3000.0f, 0, 1000);
    lock_on_tap();
    for (int i = 0; i < 120; ++i) f.frame();
    place(memory, monster_b, 0.0f, -3000.0f);
    f.frame();
    check(!lock_on_status().locked, "a tap that found nothing gives up after a few seconds");
}

void test_off_changes_nothing() {
    Fixture f;
    mhfu::settings::current().lock_on = false;
    auto &memory = f.runtime.memory();
    const auto *p = memory.raw_pointer(camera_address, 0x100u);
    const std::vector<std::uint8_t> before(p, p + 0x100u);
    lock_on_tap();
    for (int i = 0; i < 5; ++i) f.frame();
    check(!lock_on_status().locked, "with the setting off a tap does nothing");
    const auto *q = memory.raw_pointer(camera_address, 0x100u);
    check(std::vector<std::uint8_t>(q, q + 0x100u) == before, "and the camera is left as the game made it");
}

LockOnView picture_camera() {
    LockOnView camera;
    camera.view[0] = camera.view[5] = camera.view[10] = camera.view[15] = 1.0f;
    camera.projection[0] = camera.projection[5] = camera.projection[10] = 1.0f;
    camera.projection[11] = -1.0f;
    camera.x_scale = 240.0f;
    camera.x_offset = 240.0f;
    camera.y_scale = -136.0f;
    camera.y_offset = 136.0f;
    return camera;
}

void test_marker_sits_on_the_drawn_view() {
    lock_on_reset();
    const LockOnView camera = picture_camera();
    const auto centre = lock_on_project(camera, {0.0f, 0.0f, -10.0f});
    check(centre && (*centre)[0] == 0.5f && (*centre)[1] == 0.5f,
        "a point in front of the camera lands in the middle of the picture");
    const auto high = lock_on_project(camera, {0.0f, 10.0f, -10.0f});
    check(high && (*high)[1] == 0.0f, "a point above the camera lands at the top");
    check(!lock_on_project(camera, {0.0f, 0.0f, 10.0f}), "a point behind the camera is not marked");
    check(!lock_on_project(camera, {100.0f, 0.0f, -10.0f}), "a point off the picture is not marked");
    LockOnView unscaled = camera;
    unscaled.x_scale = 0.0f;
    check(!lock_on_project(unscaled, {0.0f, 0.0f, -10.0f}), "a draw with no viewport is not a camera");

    Fixture f;
    f.frame();
    lock_on_tap();
    f.frame();
    check(lock_on_status().locked, "locked, so the marker has a monster");
    const std::uint32_t target = lock_on_status().target;
    // The ring is drawn a little above the feet. At y -100 that point is on
    // the camera's axis, in the middle of the picture.
    place(f.runtime.memory(), target, 0.0f, -10.0f);
    write_float(f.runtime.memory(), target + game::kPosition + 4u, -100.0f);
    LockOnView hud = camera;
    hud.projection[11] = 0.0f;
    lock_on_note_view(hud, 5000u);
    lock_on_note_view(camera, 20u);
    LockOnView smaller = camera;
    smaller.x_offset = 0.0f;
    lock_on_note_view(smaller, 3u);
    const auto at = lock_on_marker(f.runtime.memory());
    check(at && (*at)[0] == 0.5f && (*at)[1] == 0.5f,
        "the ring sits on the monster in the view the world was drawn with, not the HUD");
    check(!lock_on_marker(f.runtime.memory()), "that view is used for one frame");
    lock_on_reset();
}

void test_binds() {
    using namespace mhfu::input;
    TapDetector tap;
    check(!tap.update(true, false), "nothing while held");
    check(tap.update(false, false), "a tap when let go alone");
    check(!tap.update(true, false) && !tap.update(true, true) && !tap.update(false, false),
        "no tap when another input was pressed during the hold (R3 + Left)");
    check(!tap.update(true, true) && !tap.update(false, false), "nor when one was held as it went down (L3 + R3)");
    for (std::size_t i = 0; i < kPresets; ++i) {
        const Layout &l = layout(static_cast<Preset>(i));
        const Slots &pad_slots = l.pad[static_cast<std::size_t>(Action::LockOn)];
        check(!pad_slots[0].empty(), "every preset binds lock-on on the pad");
        check(!l.keys[static_cast<std::size_t>(Action::LockOn)][0].empty(), "and on the keyboard");
        for (std::size_t a = 0; a < kActions; ++a)
            check(conflicts(l.pad, static_cast<Action>(a)).empty(), "no preset has a conflict on the pad");
        for (std::size_t a = 0; a < kActions; ++a)
            check(conflicts(l.keys, static_cast<Action>(a)).empty(), "nor on the keyboard");
    }
    const Layout &d = layout(Preset::Default);
    check(d.pad[static_cast<std::size_t>(Action::LockOn)][0] == single(pad(PadInput::RightStick)),
        "R3 locks on in the default preset");
    check(acts_on_release(Action::LockOn) && !acts_on_release(Action::Screenshot), "only lock-on acts on release");
    check(
        layout(Preset::LeftHanded).pad[static_cast<std::size_t>(Action::LockOn)][0] == single(pad(PadInput::LeftStick)),
        "and L3 in the left-handed one");
    check(d.keys[static_cast<std::size_t>(Action::LockOn)][0] == single(mouse_button(2)) &&
            d.keys[static_cast<std::size_t>(Action::LockOn)][1] == single(from_name("T")),
        "the middle mouse button and T lock on from the keyboard");
    {
        // A tap's chord inside a longer one is no conflict either way; the
        // same chord inside a longer one for anything else still is.
        Bindings b{};
        const Chord r3 = single(pad(PadInput::RightStick));
        const Chord shot = chord(pad(PadInput::RightStick), pad(PadInput::DpadLeft));
        b[static_cast<std::size_t>(Action::LockOn)] = {r3};
        b[static_cast<std::size_t>(Action::Screenshot)] = {shot};
        check(conflicts(b, Action::LockOn).empty() && conflicts(b, Action::Screenshot).empty(),
            "R3 for lock-on beside R3 + D-pad left for a screenshot is no conflict");
        b[static_cast<std::size_t>(Action::HideHud)] = {r3};
        check(conflicts(b, Action::HideHud).size() == 2u, "R3 for Hide HUD clashes with lock-on and the screenshot");
    }
}

} // namespace

int main() {
    test_geometry();
    test_monsters_and_pick();
    test_unmarked_monsters_still_lock();
    test_lock_follows_and_lets_go();
    test_tap_waits_for_the_monster();
    test_off_changes_nothing();
    test_marker_sits_on_the_drawn_view();
    test_binds();
    std::cout << (failures ? "FAIL" : "PASS") << ": lock-on (" << failures << " failures)\n";
    return failures ? 1 : 0;
}
