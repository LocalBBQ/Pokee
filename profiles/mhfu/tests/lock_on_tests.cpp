// Lock-on (camera/lock_on.hpp) without game code or game data: a stand-in
// for the quest overlay's header, the entity list and the camera, and the
// real camera driver dispatching the camera update.
#include "camera/camera_input.hpp"
#include "camera/game_camera.hpp"
#include "camera/hunt_icon.hpp"
#include "camera/lock_on.hpp"
#include "input/bindings.hpp"
#include "input/presets.hpp"
#include "settings/settings.hpp"
#include "psprecomp/runtime.hpp"

#include <bit>
#include <cmath>
#include <cstring>
#include <iostream>
#include <utility>
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

constexpr std::uint32_t hunter_list = 0x089CBD04u;
constexpr std::uint32_t hunters = 0x09400000u;
constexpr std::uint32_t hunter = 0x09410000u;
// The game's L recenter with the camera already near behind the hunter: the
// target becomes the facing plus half a turn and the snap byte is set and
// cleared in the same update.
bool game_recenters{};
// L while the camera is not yet that close: the target is written behind the
// hunter, the filtered yaw steps a quarter of the way there, and the snap
// stays set.
bool game_pulls_recenter{};
// Set when the stand-in places the eye behind the hunter inside the update.
// The recenter check can do that without the button bit, and so can the
// facing step that runs when camera +0x91 is not negative.
bool game_placed_recenter_eye{};

void write_float(psprecomp::GuestMemory &memory, std::uint32_t address, float value);

// The pad sample the update writes back onto the hunter list, undoing a clear
// made before the update. 0x09A6EE48 can also make the recenter check return 1
// without that bit.
std::uint16_t pad_recenter = 0u;
bool overlay_recenters = false;

void place_behind_eye(psprecomp::GuestMemory &memory) {
    const auto behind = static_cast<std::uint16_t>(memory.load32(hunter + 500u) + 0x8000u);
    const auto current = memory.load16(camera_address + 0x82u);
    const int step = static_cast<std::int16_t>(static_cast<std::uint16_t>(behind - current)) >> 2;
    memory.store16(camera_address + 0x80u, behind);
    memory.store16(camera_address + 0x82u, static_cast<std::uint16_t>(current + step));
    memory.store8(camera_address + 0x8Eu, 1u);
    write_float(memory, camera_address, 50.0f);
    memory.store8(camera_address + 0x8Du, 20u);
    game_placed_recenter_eye = true;
}

// The game's recenter check. Returns 1 when the button bit survived, or when
// the overlay says to recenter without it.
void recenter_stand_in(psprecomp::Runtime &runtime, psprecomp::AllegrexContext &ctx) {
    auto &memory = runtime.memory();
    const auto camera = ctx.gpr[4] + 0xA0u;
    const bool bits = (memory.load16(camera + 0x86u) & 0x200u) != 0u && (memory.load32(hunter + 1152u) & 0x200u) != 0u;
    ctx.gpr[2] = (bits || overlay_recenters) ? 1u : 0u;
    ctx.pc = ctx.gpr[31];
}

void original(psprecomp::Runtime &runtime, psprecomp::AllegrexContext &ctx) {
    auto &memory = runtime.memory();
    const auto behind = static_cast<std::uint16_t>(memory.load32(hunter + 500u) + 0x8000u);
    if (std::exchange(game_recenters, false)) {
        memory.store16(camera_address + 0x80u, behind);
        memory.store8(camera_address + 0x8Eu, 0u);
    }
    if (std::exchange(game_pulls_recenter, false)) {
        const auto current = memory.load16(camera_address + 0x82u);
        const int step = static_cast<std::int16_t>(static_cast<std::uint16_t>(behind - current)) >> 2;
        memory.store16(camera_address + 0x80u, behind);
        memory.store16(camera_address + 0x82u, static_cast<std::uint16_t>(current + step));
        memory.store8(camera_address + 0x8Eu, 1u);
        // The eye is placed from that stepped yaw, and +0x8D starts at 20.
        write_float(memory, camera_address, 50.0f);
        memory.store8(camera_address + 0x8Du, 20u);
    }
    // The update refreshes the list halfword, then copies it onto the camera,
    // then asks 0x0888E494. A clear made before this does not survive the refresh.
    const auto list = memory.load32(hunter_list);
    if (list >= 0x08800000u) {
        if (pad_recenter != 0u) memory.store16(list + 0xA72u, pad_recenter);
        memory.store16(camera_address + 0x86u, memory.load16(list + 0xA72u));
    }
    psprecomp::AllegrexContext gate = ctx;
    gate.gpr[4] = parent_address;
    gate.gpr[31] = 0x088874ECu;
    gate.gpr[2] = 0u;
    gate.pc = 0x0888E494u;
    if (auto check = runtime.function_at(0x0888E494u)) check(runtime, gate);
    if (gate.gpr[2] != 0u) {
        place_behind_eye(memory);
    } else if (static_cast<std::int8_t>(memory.load8(camera_address + 0x91u)) >= 0) {
        // 0x0888755C: the check returned 0, and the gate is not negative, so
        // the update still steps toward where the hunter is facing.
        place_behind_eye(memory);
    }
    ctx.pc = ctx.gpr[31];
}

void write_float(psprecomp::GuestMemory &memory, std::uint32_t address, float value) {
    memory.store32(address, std::bit_cast<std::uint32_t>(value));
}

float read_float(const psprecomp::GuestMemory &memory, std::uint32_t address) {
    return std::bit_cast<float>(memory.load32(address));
}

void place(psprecomp::GuestMemory &memory, std::uint32_t monster, float x, float z, std::int16_t health = 1000,
    std::int16_t most = 1000) {
    write_float(memory, monster + game::kPosition, x);
    write_float(memory, monster + game::kPosition + 4u, 0.0f);
    write_float(memory, monster + game::kPosition + 8u, z);
    memory.store16(monster + game::kHealth, static_cast<std::uint16_t>(health));
    memory.store16(monster + game::kMostHealth, static_cast<std::uint16_t>(most));
}

// GameSys and the stage the quest has loaded. Zone 21 is this area; 39 is not.
constexpr std::uint32_t game_sys = 0x09800000u;
constexpr std::uint16_t zone_here = 21u;
constexpr std::uint16_t zone_away = 39u;

void set_loaded_stage(psprecomp::GuestMemory &memory, std::uint16_t stage) {
    memory.store32(game::kGameSysPointer, game_sys);
    memory.store16(game_sys + game::kGameSysStage, stage);
}

void set_stage(psprecomp::GuestMemory &memory, std::uint32_t monster, std::uint16_t stage) {
    memory.store16(monster + game::kStageId, stage);
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
        // The default pad binds Focus/Follow Camera to LT. These tests press L,
        // or a bind they set themselves, so they start with that button empty.
        const auto focus = static_cast<std::size_t>(mhfu::input::Action::FocusCamera);
        mhfu::settings::current().controls.keys[focus] = {};
        mhfu::settings::current().controls.pad[focus] = {};
        set_focus_camera_held(false);
        pad_recenter = 0u;
        overlay_recenters = false;
        runtime.register_generated_unit(29u, 0x08886000u, 0x1000u, &original, nullptr);
        runtime.register_function(helper, &original, "recomp_unit_test");
        runtime.register_function(0x0888E494u, &recenter_stand_in, "recomp_unit_test_recenter");
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
        // The followed hunter, at the origin, for the camera tests.
        memory.store32(hunter_list, hunters);
        memory.store32(hunters + 2692u, hunter);
        memory.store16(hunters + 0xA72u, 0u);
        place(memory, hunter, 0.0f, 0.0f);
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
    check(lock_on_monsters(memory).empty(),
        "a dead large monster still keeps a small one from taking the lock");
    place(memory, monster_a, 1000.0f, 0.0f, 1000, 0);
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
    // The right stick pauses tracking and hides the reticle. The monster, and
    // its portrait, stay.
    check(lock_on_marker_wanted(), "follow shows the reticle while tracking");
    f.frame(1.0f);
    check(lock_on_status().locked && lock_on_status().target == monster_b && !game_camera_driving() &&
            !lock_on_marker_wanted(),
        "the right stick turns follow off, hides the reticle, and keeps the monster");
    f.frame(1.0f);
    check(lock_on_status().locked && !game_camera_driving() && !lock_on_marker_wanted(),
        "holding the stick does not drop the selection or bring the reticle back");
    lock_on_release("");
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
    memory.store8(camera_address + 0x8Eu, 1u);
    f.frame();
    check(!lock_on_status().locked && memory.load8(camera_address + 0x8Eu) == 1u,
        "recentring with L lets go and stays the game's recenter");

    lock_on_tap();
    f.frame();
    lock_on_tap();
    f.frame();
    check(!lock_on_status().locked, "with one monster a second tap lets go");
}

void test_monster_in_another_area() {
    Fixture f;
    auto &memory = f.runtime.memory();
    place(memory, monster_b, 8095.14f, 5898.45f);
    write_float(memory, monster_b + game::kPosition + 4u, 1000.0f);
    check(lock_on_monsters(memory).size() == 2u, "with no loaded stage, a far hold is still a target");
    set_loaded_stage(memory, zone_here);
    set_stage(memory, monster_a, zone_here);
    set_stage(memory, monster_b, zone_here);
    place(memory, monster_b, 200.0f, -200.0f);
    write_float(memory, monster_b + game::kPosition + 4u, 1000.0f);
    check(lock_on_monsters(memory).size() == 2u, "the same zone is this area, even at the hold height");
    place(memory, monster_b, 0.0f, -3000.0f);
    set_stage(memory, monster_b, zone_away);
    const std::vector<LockMonster> away = lock_on_monsters(memory);
    check(away.size() == 1u && away[0].address == monster_a, "a different zone is not a target, even close by");
    set_stage(memory, monster_b, zone_here);
    place(memory, monster_b, 0.0f, -3000.0f);
    lock_on_tap();
    f.frame();
    check(lock_on_status().target == monster_b, "a tap locks the monster in this area");
    set_stage(memory, monster_b, zone_away);
    f.frame();
    check(!lock_on_status().locked, "the lock lets go when the monster leaves for another area");
    lock_on_tap();
    f.frame();
    check(lock_on_status().target == monster_a && lock_on_status().locked,
        "the next tap takes the monster that is still here");
    place(memory, monster_a, 1000.0f, 0.0f, 0, 1000);
    f.frame();
    check(!lock_on_status().locked, "the lock lets go when the monster dies");
    lock_on_tap();
    f.frame();
    check(!lock_on_status().locked && !lock_on_marker_wanted(),
        "with no large monster in this area, R3 locks nothing and shows no reticle");
}

void test_hunt_portrait_keeps_the_id_in_another_area() {
    Fixture f;
    auto &memory = f.runtime.memory();
    memory.store8(monster_a + game::kMonsterId, 11u);
    memory.store8(monster_b + game::kMonsterId, 1u);
    memory.store8(small + game::kMonsterId, 5u);
    set_loaded_stage(memory, zone_here);
    set_stage(memory, monster_a, zone_here);
    set_stage(memory, monster_b, zone_away);
    const std::vector<HuntMonster> hunt = hunt_monsters(memory);
    check(hunt.size() == 2u, "the card keeps the large monster that is in another area");
    const HuntMonster *here = nullptr;
    const HuntMonster *away = nullptr;
    for (const HuntMonster &monster : hunt) {
        if (monster.address == monster_a) here = &monster;
        if (monster.address == monster_b) away = &monster;
    }
    check(here != nullptr && here->id == 11u && !here->other_area, "the monster here keeps its id");
    check(away != nullptr && away->id == 1u && away->other_area, "the monster in another area keeps its id");
    check(lock_on_monsters(memory).size() == 1u && lock_on_monsters(memory)[0].address == monster_a,
        "lock-on still will not select the monster in another area");

    const std::vector<HuntPortrait> placed = place_hunt_portraits(hunt, monster_a, true);
    check(placed.size() == 2u, "one card each");
    check(placed[0].other_area && !placed[0].locked && !placed[0].dead && placed[0].id == 1u,
        "the corner icon stays the first monster on the list, even in another area");
    check(!placed[1].other_area && placed[1].locked && placed[1].id == 11u,
        "the next icon is the monster here, and the brackets follow the lock");
    check(placed[0].x < placed[1].x && placed[0].y == placed[1].y, "the cards sit in a row");
    check(placed[0].width == kHuntCardWidth && placed[0].height == kHuntCardHeight &&
            placed[1].width == kHuntCardWidth && placed[1].height == kHuntCardHeight,
        "each card is three quarters of the first portrait");
    mhfu::settings::current().hunt_portrait_size = 200u;
    const std::vector<HuntPortrait> large = place_hunt_portraits(hunt, monster_a, true);
    check(large.size() == 2u && large[0].width == kHuntCardWidth * 2.0f && large[0].height == kHuntCardHeight * 2.0f &&
            large[1].x + large[1].width <= 480.0f,
        "twice the usual size still fits the row on the picture");
    mhfu::settings::current().hunt_portrait_size = 50u;
    const std::vector<HuntPortrait> half = place_hunt_portraits(hunt, monster_a, true);
    check(half[0].width == kHuntCardWidth * 0.5f && half[0].height == kHuntCardHeight * 0.5f,
        "half the usual size is half the card");
    mhfu::settings::current().hunt_portrait_size = 100u;
    check(placed[0].x >= 0.0f && placed[1].x + placed[1].width <= 480.0f && placed[0].y >= 0.0f &&
            placed[0].y + placed[0].height <= 272.0f,
        "the row stays on the 480 by 272 picture");
    const std::optional<HuntIconCell> rathalos = hunt_icon_cell(11u);
    const std::optional<HuntIconCell> rathian = hunt_icon_cell(1u);
    const std::optional<HuntIconCell> tigrex = hunt_icon_cell(0x4Bu);
    check(rathalos && rathalos->u == 0x90u && rathalos->v == 0x24u, "Rathalos is the icon at u 144, v 36");
    check(rathian && rathian->u == 0xB4u && rathian->v == 0x24u, "Rathian is the icon beside it");
    check(tigrex && tigrex->u == 0u && tigrex->v == 0xB4u, "Tigrex is the icon at the start of its row");
    check(!hunt_icon_cell(0u), "an id the sheet does not carry has no icon");
    bool inside = true;
    unsigned cells = 0u;
    for (unsigned id = 0u; id < 256u; ++id) {
        const std::optional<HuntIconCell> cell = hunt_icon_cell(static_cast<std::uint8_t>(id));
        if (!cell) continue;
        ++cells;
        if (cell->u + kHuntIconTexels > 256u || cell->v + kHuntIconTexels > 512u) inside = false;
    }
    check(cells == 77u && inside, "every icon cell is 36 texels and stays inside the 256 by 512 sheet");

    const std::vector<HuntPortrait> idle = place_hunt_portraits(hunt, monster_a, false);
    check(!idle[0].locked && !idle[1].locked, "with nothing locked, neither card has brackets");

    const std::vector<HuntMonster> returned_order = hunt;
    set_stage(memory, monster_b, zone_here);
    const std::vector<HuntMonster> returned = hunt_monsters(memory);
    check(returned.size() == returned_order.size() && returned[0].address == monster_b && !returned[0].other_area &&
            returned[1].address == monster_a,
        "coming back to this area does not move the icon");

    set_stage(memory, monster_a, zone_away);
    set_stage(memory, monster_b, zone_away);
    const std::vector<HuntMonster> both_away = hunt_monsters(memory);
    check(both_away.size() == 2u && both_away[0].address == monster_b && both_away[0].other_area &&
            both_away[1].address == monster_a && both_away[1].other_area,
        "with both large monsters in another area, a small monster does not take the card");
    check(lock_on_monsters(memory).empty(),
        "and lock-on has nothing to select, so R3 does not put a reticle on the small one");
    set_stage(memory, monster_a, zone_here);
    set_stage(memory, monster_b, zone_here);

    const std::vector<HuntPortrait> before_death = place_hunt_portraits(hunt_monsters(memory), 0u, false);
    place(memory, monster_b, 0.0f, -3000.0f, 0, 1000);
    const std::vector<HuntMonster> after_death = hunt_monsters(memory);
    const HuntMonster *corpse = nullptr;
    for (const HuntMonster &monster : after_death)
        if (monster.address == monster_b) corpse = &monster;
    check(corpse != nullptr && corpse->dead && !corpse->other_area, "a dead monster stays on the icon, marked dead");
    check(lock_on_monsters(memory).size() == 1u && lock_on_monsters(memory)[0].address == monster_a,
        "and lock-on will not select it");
    const std::vector<HuntPortrait> dead_row = place_hunt_portraits(after_death, monster_b, true);
    check(dead_row.size() == before_death.size() && dead_row[0].dead && !dead_row[0].locked && dead_row[0].id == 1u &&
            dead_row[1].x == before_death[1].x && !dead_row[1].dead,
        "dying greys that icon in place and does not move the other one");

    const std::vector<HuntMonster> before_shuffle = hunt_monsters(memory);
    const std::vector<HuntPortrait> before_places = place_hunt_portraits(before_shuffle, 0u, false);
    memory.store32(manager + game::kListHead, node_a);
    memory.store32(node_a + game::kNodeOlder, node_b);
    memory.store32(node_b + game::kNodeOlder, node_small);
    const std::vector<HuntMonster> shuffled = hunt_monsters(memory);
    check(shuffled.size() == before_shuffle.size() && shuffled[0].address == before_shuffle[0].address &&
            shuffled[1].address == before_shuffle[1].address,
        "reinserting a monster at the head of the list does not move its icon");
    const std::vector<HuntPortrait> after_places = place_hunt_portraits(shuffled, 0u, false);
    check(after_places.size() == before_places.size() && after_places[0].x == before_places[0].x &&
            after_places[1].x == before_places[1].x && after_places[0].id == before_places[0].id,
        "and the cards stay in the places they first took");

    memory.store32(companion + game::kVtable, large_vtable);
    place(memory, companion, 50.0f, -50.0f);
    memory.store8(companion + game::kMonsterId, 7u);
    memory.store32(manager + game::kListHead, node_companion);
    memory.store32(node_companion + game::kNodeOlder, node_a);
    const std::vector<HuntMonster> with_new = hunt_monsters(memory);
    check(with_new.size() == 3u && with_new[0].address == before_shuffle[0].address &&
            with_new[1].address == before_shuffle[1].address && with_new[2].address == companion,
        "a monster that joins later takes the next icon, not the first");

    memory.store8(game::kTaskSlot + 32u, 'l');
    check(hunt_monsters(memory).empty(), "leaving the quest clears the icon order");
    start_quest(memory);
    const std::vector<HuntMonster> fresh = hunt_monsters(memory);
    check(fresh.size() == 3u && fresh[0].address == companion && fresh[1].address == monster_a &&
            fresh[2].address == monster_b,
        "a new quest takes the list's order again");
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

void test_focus_turns_when_facing_away() {
    Fixture f;
    mhfu::settings::current().lock_on_style = mhfu::settings::LockOnStyle::Focus;
    auto &memory = f.runtime.memory();
    memory.store32(hunter_list, hunters);
    memory.store32(hunters + 2692u, hunter);
    memory.store32(hunter + 500u, 0x00008000u); // facing +z, so behind is yaw 0, where the camera looks
    lock_on_tap();
    f.frame();
    check(lock_on_status().locked && lock_on_status().target == monster_b, "focus selects the monster ahead");
    for (int i = 0; i < 5; ++i) f.frame();
    check(memory.load16(camera_address + 0x80u) == 0u && f.yaw() == 0u,
        "a target already behind the hunter is put back after each update without L");
    place(memory, monster_b, 0.0f, 3000.0f); // now straight behind the camera

    game_recenters = true;
    f.frame();
    f.frame();
    check(f.yaw() != 0u, "a recenter the game dropped still starts the turn");
    for (int i = 0; i < 40; ++i) f.frame();
    check(std::abs(yaw_difference(f.yaw(), 32768u)) < 64, "L turns the camera a half turn to the monster behind it");
    check(!game_camera_driving(), "and hands it back on arrival");

    const std::uint16_t landed = f.yaw();
    for (int i = 0; i < 10; ++i) f.frame();
    check(f.yaw() == landed && memory.load16(camera_address + 0x80u) == landed,
        "updates without L leave the target alone");
    mhfu::settings::current().lock_on_style = mhfu::settings::LockOnStyle::Follow;
}

void test_focus_turn_lets_go_promptly() {
    Fixture f;
    mhfu::settings::current().lock_on_style = mhfu::settings::LockOnStyle::Focus;
    auto &memory = f.runtime.memory();
    lock_on_tap();
    f.frame();
    place(memory, monster_b, 3000.0f, 0.0f);
    memory.store8(camera_address + 0x8Eu, 1u);
    f.frame();
    int updates = 1;
    while (game_camera_driving() && updates < 60) {
        f.frame();
        ++updates;
    }
    check(updates <= 15, "a quarter turn lets go of the camera within half a second");
    check(f.yaw() == 49152u, "and lands exactly on the monster");

    memory.store8(camera_address + 0x8Eu, 1u);
    updates = 0;
    float angle = 0.0f;
    do {
        angle += 0.14f;
        place(memory, monster_b, 3000.0f * std::cos(angle), 3000.0f * std::sin(angle));
        f.frame();
        ++updates;
    } while (game_camera_driving() && updates < 120);
    check(updates <= 31, "a monster circling the hunter is given up on after a second");
    check(lock_on_status().locked, "and stays selected");
    mhfu::settings::current().lock_on_style = mhfu::settings::LockOnStyle::Follow;
}

void test_focus_centers_a_monster_already_on_screen() {
    Fixture f;
    mhfu::settings::current().lock_on_style = mhfu::settings::LockOnStyle::Focus;
    auto &memory = f.runtime.memory();
    lock_on_tap();
    f.frame();
    // About 15 degrees off the camera's look, inside the picture.
    place(memory, monster_b, 800.0f, -3000.0f);
    const std::uint16_t before = f.yaw();
    const std::uint16_t wanted = yaw_towards({0.0f, 190.0f, 0.0f}, {800.0f, 100.0f, -3000.0f});
    memory.store8(camera_address + 0x8Eu, 1u);
    f.frame();
    const std::uint16_t stepped = f.yaw();
    check(stepped != before && std::abs(yaw_difference(stepped, wanted)) < std::abs(yaw_difference(before, wanted)),
        "a monster already on screen starts an eased turn toward it");
    check(std::abs(yaw_difference(before, stepped)) < std::abs(yaw_difference(before, wanted)),
        "the first update does not snap onto the monster");
    for (int i = 0; i < 40; ++i) f.frame();
    check(f.yaw() == wanted, "the tap lands on the monster");
    check(!game_camera_driving(), "and then leaves the camera");
    check(lock_on_status().locked, "the selection stays");

    // The double-tap window has closed. This L is a new tap, and the frame
    // does not jump behind the hunter.
    memory.store32(hunter_list, hunters);
    memory.store32(hunters + 2692u, hunter);
    memory.store32(hunter + 500u, 0x00001000u);
    const std::uint16_t landed = f.yaw();
    game_recenters = true;
    f.frame();
    check(f.yaw() == landed && memory.load16(camera_address + 0x80u) == landed,
        "L's recenter is put back, so the camera does not jump behind the hunter");
    mhfu::settings::current().lock_on_style = mhfu::settings::LockOnStyle::Follow;
}

void test_focus_turns_on_l_and_keeps_the_selection() {
    Fixture f;
    mhfu::settings::current().lock_on_style = mhfu::settings::LockOnStyle::Focus;
    auto &memory = f.runtime.memory();
    lock_on_tap();
    f.frame();
    check(lock_on_status().locked && lock_on_status().target == monster_b, "focus selects the monster in view");
    check(!game_camera_driving(), "and does not take the camera while it only holds the selection");
    place(memory, monster_b, 3000.0f, 0.0f);
    const std::uint16_t held = f.yaw();
    for (int i = 0; i < 40; ++i) f.frame();
    check(f.yaw() == held && lock_on_status().locked, "the camera stays put as the monster moves");

    memory.store8(camera_address + 0x8Eu, 1u);
    for (int i = 0; i < 40; ++i) f.frame();
    check(memory.load8(camera_address + 0x8Eu) == 0u, "L's recenter is taken");
    check(std::abs(yaw_difference(f.yaw(), 49152u)) < 64, "and the camera turns to the selected monster");
    check(lock_on_status().locked && lock_on_status().target == monster_b, "the selection stays");
    check(!game_camera_driving(), "the turn has arrived, so the camera is the player's again");
    place(memory, monster_b, 0.0f, 3000.0f);
    const std::uint16_t landed = f.yaw();
    for (int i = 0; i < 10; ++i) f.frame();
    check(f.yaw() == landed && lock_on_status().locked, "after the turn the camera does not follow");

    place(memory, monster_b, 0.0f, 3000.0f);
    memory.store8(camera_address + 0x8Eu, 1u);
    f.frame();
    f.frame(1.0f);
    f.frame(1.0f);
    check(lock_on_status().locked, "turning by hand during the turn keeps the monster selected");
    const std::uint16_t stopped = f.yaw();
    place(memory, monster_b, 0.0f, -3000.0f);
    for (int i = 0; i < 10; ++i) f.frame();
    check(f.yaw() == stopped, "and the turn does not resume");

    lock_on_release("");
    const std::uint16_t before = f.yaw();
    memory.store8(camera_address + 0x8Eu, 1u);
    f.frame();
    check(memory.load8(camera_address + 0x8Eu) == 1u && f.yaw() == before,
        "with nothing selected, L still recenters behind the hunter");
    lock_on_tap();
    f.frame();
    const std::uint32_t first = lock_on_status().target;
    lock_on_tap();
    f.frame();
    check(lock_on_status().locked && lock_on_status().target != first, "another tap moves to the next monster");
    lock_on_tap();
    f.frame();
    check(!lock_on_status().locked, "after the last one, a tap lets go");
}

void test_focus_keeps_the_picture_when_the_hunter_is_aside() {
    Fixture f;
    mhfu::settings::current().lock_on_style = mhfu::settings::LockOnStyle::Focus;
    auto &memory = f.runtime.memory();
    memory.store32(hunter_list, hunters);
    memory.store32(hunters + 2692u, hunter);
    // Behind the hunter is a quarter turn off the camera, so the hunter is
    // off at the side of the picture. The monster is straight ahead.
    memory.store32(hunter + 500u, 0x0000C000u);
    lock_on_tap();
    f.frame();
    check(lock_on_status().locked, "focus selects the monster ahead");
    write_float(memory, camera_address, 0.0f);
    game_pulls_recenter = true;
    f.frame();
    check(f.yaw() == 0u && memory.load16(camera_address + 0x80u) == 0u,
        "L's quarter-step behind the hunter is put back in the same update");
    check(read_float(memory, camera_address) == 0.0f, "and the eye it placed is put back with it");
    check(memory.load8(camera_address + 0x8Eu) == 0u && memory.load8(camera_address + 0x8Du) == 0u,
        "the snap and its countdown are dropped");
    for (int i = 0; i < 10; ++i) f.frame();
    check(f.yaw() == 0u && !game_camera_driving(), "a monster the camera already faces is left on it");
    // Past the double-tap window, so the next L focuses instead of recentering.
    for (int i = 0; i < 12; ++i) f.frame();

    place(memory, monster_b, 3000.0f, 0.0f);
    game_pulls_recenter = true;
    f.frame();
    check(f.yaw() == 0u && read_float(memory, camera_address) == 0.0f,
        "off the picture, that same frame still shows the camera where it was");
    f.frame();
    check(f.yaw() != 0u && game_camera_driving(), "and the turn starts from there on the next update");
    mhfu::settings::current().lock_on_style = mhfu::settings::LockOnStyle::Follow;
}

void hold_recenter(psprecomp::GuestMemory &memory) {
    // L is 0x100 on the hunter's button word. The overlay recenters without
    // the R bit, which is what a tap of L does.
    pad_recenter = 0x100u;
    overlay_recenters = true;
    memory.store16(hunters + 0xA72u, 0x100u);
    memory.store32(hunter + 1152u, 0x100u);
}

void release_recenter(psprecomp::GuestMemory &memory) {
    pad_recenter = 0u;
    overlay_recenters = false;
    memory.store16(hunters + 0xA72u, 0u);
    memory.store16(camera_address + 0x86u, 0u);
    memory.store32(hunter + 1152u, 0u);
}

void test_focus_l_does_not_place_the_eye_behind_the_hunter() {
    Fixture f;
    mhfu::settings::current().lock_on_style = mhfu::settings::LockOnStyle::Focus;
    auto &memory = f.runtime.memory();
    // Behind the hunter is a quarter turn off, so the hunter stands at the
    // side of the picture. The monster is already in view, about 15° off.
    memory.store32(hunter + 500u, 0x0000C000u);
    place(memory, monster_b, 800.0f, -3000.0f);
    lock_on_tap();
    f.frame();
    check(lock_on_status().locked, "focus selects the monster in view");
    const std::uint16_t wanted = yaw_towards({0.0f, 190.0f, 0.0f}, {800.0f, 100.0f, -3000.0f});
    const std::uint16_t behind = static_cast<std::uint16_t>(0x0000C000u + 0x8000u);
    write_float(memory, camera_address, 0.0f);
    // Non-negative, this gate steps the yaw toward the hunter's facing even
    // when the recenter check returns 0.
    memory.store8(camera_address + 0x91u, 0u);
    hold_recenter(memory);
    game_placed_recenter_eye = false;
    f.frame();
    check(game_placed_recenter_eye && read_float(memory, camera_address) == 0.0f,
        "L's eye placement is put back, and the focus turn is the only move");
    check((memory.load32(hunter + 1152u) & 0x100u) != 0u, "L stays down");
    check(std::abs(yaw_difference(f.yaw(), wanted)) < std::abs(yaw_difference(0u, wanted)),
        "the same update turns toward the monster");
    check(std::abs(yaw_difference(f.yaw(), wanted)) < std::abs(yaw_difference(f.yaw(), behind)),
        "and not toward where the hunter is facing");
    std::uint16_t previous = f.yaw();
    for (int i = 0; i < 3; ++i) {
        game_placed_recenter_eye = false;
        f.frame();
        check(std::abs(yaw_difference(f.yaw(), wanted)) < std::abs(yaw_difference(f.yaw(), behind)),
            "holding L does not swing the camera behind the hunter");
        check(std::abs(yaw_difference(f.yaw(), wanted)) < std::abs(yaw_difference(previous, wanted)),
            "holding L keeps the turn toward the monster");
        previous = f.yaw();
    }
    // Idle frames are not a Focus turn. A non-negative gate would keep stepping
    // toward the hunter, so put back the negative gate the fixture started with.
    memory.store8(camera_address + 0x91u, 0xFFu);

    release_recenter(memory);
    f.frame();
    hold_recenter(memory);
    game_placed_recenter_eye = false;
    f.frame();
    check(game_placed_recenter_eye && read_float(memory, camera_address) == 50.0f,
        "a second tap, after L was released, still recenters behind the hunter");
    check(lock_on_status().locked, "and the monster stays selected");

    release_recenter(memory);
    memory.store8(camera_address + 0x8Eu, 0u);
    memory.store8(camera_address + 0x8Du, 0u);
    f.frame();
    for (int i = 0; i < 12; ++i) f.frame();
    write_float(memory, camera_address, 0.0f);
    memory.store8(camera_address + 0x91u, 0u);
    hold_recenter(memory);
    game_placed_recenter_eye = false;
    const std::uint16_t parked = f.yaw();
    f.frame();
    check(std::abs(yaw_difference(f.yaw(), wanted)) < std::abs(yaw_difference(f.yaw(), behind)),
        "a later tap centers on the monster again");
    check(std::abs(yaw_difference(f.yaw(), wanted)) < std::abs(yaw_difference(parked, wanted)),
        "and that tap moves toward the monster");

    release_recenter(memory);
    lock_on_release("");
    hold_recenter(memory);
    game_placed_recenter_eye = false;
    f.frame();
    check(game_placed_recenter_eye, "with nothing selected, L still recenters behind the hunter");
    mhfu::settings::current().lock_on_style = mhfu::settings::LockOnStyle::Follow;
}

void test_r_does_not_turn_toward_the_monster() {
    Fixture f;
    mhfu::settings::current().lock_on_style = mhfu::settings::LockOnStyle::Focus;
    auto &memory = f.runtime.memory();
    memory.store32(hunter_list, hunters);
    memory.store32(hunters + 2692u, hunter);
    memory.store32(hunter + 500u, 0x0000C000u);
    place(memory, monster_b, 800.0f, -3000.0f);
    lock_on_tap();
    f.frame();
    check(lock_on_status().locked && !lock_on_marker_wanted(), "R3 selects the monster and leaves the reticle off");
    const std::uint16_t before = f.yaw();
    const std::uint16_t wanted = yaw_towards({0.0f, 190.0f, 0.0f}, {800.0f, 100.0f, -3000.0f});
    pad_recenter = 0x200u;
    overlay_recenters = false;
    memory.store16(hunters + 0xA72u, 0x200u);
    memory.store32(hunter + 1152u, 0x200u);
    memory.store8(camera_address + 0x91u, 0xFFu);
    f.frame();
    check(lock_on_status().locked, "R leaves the monster selected");
    check(std::abs(yaw_difference(f.yaw(), wanted)) >= std::abs(yaw_difference(before, wanted)),
        "R does not turn the camera toward the monster");
    check(!lock_on_marker_wanted(), "R leaves the reticle off");
    for (int i = 0; i < 9; ++i) f.frame();
    check(!lock_on_marker_wanted() && lock_on_status().locked, "holding R does not bring the reticle up");
    release_recenter(memory);
    mhfu::settings::current().lock_on_style = mhfu::settings::LockOnStyle::Follow;
    lock_on_tap();
    f.frame();
    check(lock_on_status().locked, "follow selects the monster");
    pad_recenter = 0x200u;
    memory.store16(hunters + 0xA72u, 0x200u);
    memory.store32(hunter + 1152u, 0x200u);
    f.frame();
    f.frame();
    check(lock_on_status().locked, "R does not drop the follow lock");
    release_recenter(memory);
}

void test_focus_camera_bind_replaces_l() {
    using namespace mhfu::input;
    Fixture f;
    mhfu::settings::current().lock_on_style = mhfu::settings::LockOnStyle::Focus;
    auto &memory = f.runtime.memory();
    memory.store32(hunter_list, hunters);
    memory.store32(hunters + 2692u, hunter);
    memory.store32(hunter + 500u, 0x0000C000u);
    place(memory, monster_b, 800.0f, -3000.0f);
    lock_on_tap();
    f.frame();
    const std::uint16_t wanted = yaw_towards({0.0f, 190.0f, 0.0f}, {800.0f, 100.0f, -3000.0f});
    const std::uint16_t behind = static_cast<std::uint16_t>(0x0000C000u + 0x8000u);
    auto &keys = mhfu::settings::current().controls.keys[static_cast<std::size_t>(Action::FocusCamera)];
    keys[0] = single(from_name("F"));
    set_focus_camera_held(true);
    memory.store8(camera_address + 0x91u, 0xFFu);
    const std::uint16_t before = f.yaw();
    f.frame();
    check(std::abs(yaw_difference(f.yaw(), wanted)) < std::abs(yaw_difference(before, wanted)),
        "the Focus camera bind centers on the monster without L");
    check(lock_on_marker_wanted() && lock_on_status().locked, "that press shows the reticle and keeps the selection");
    set_focus_camera_held(false);
    for (int i = 0; i < 12; ++i) f.frame();
    check(!lock_on_marker_wanted() && lock_on_status().locked,
        "the reticle expires after the bind, and the selection stays");

    memory.store8(camera_address + 0x8Eu, 1u);
    memory.store8(camera_address + 0x91u, 0xFFu);
    hold_recenter(memory);
    const std::uint16_t parked = f.yaw();
    f.frame();
    check(std::abs(yaw_difference(f.yaw(), behind)) < std::abs(yaw_difference(parked, behind)),
        "with Focus camera bound, L recenters behind the hunter");
    check(std::abs(yaw_difference(f.yaw(), wanted)) > std::abs(yaw_difference(parked, wanted)),
        "and that recenter does not turn toward the monster");
    check(lock_on_status().locked, "the monster stays selected");
    release_recenter(memory);
    memory.store8(camera_address + 0x8Eu, 0u);
    memory.store8(camera_address + 0x8Du, 0u);
    for (int i = 0; i < 12; ++i) f.frame();
    // The snap byte is still clear, so the game marks the recenter inside the
    // update. That pose has to stay behind the hunter.
    memory.store16(camera_address + 0x80u, 0u);
    memory.store16(camera_address + 0x82u, 0u);
    write_float(memory, camera_address, 0.0f);
    memory.store8(camera_address + 0x91u, 0xFFu);
    hold_recenter(memory);
    game_placed_recenter_eye = false;
    f.frame();
    check(game_placed_recenter_eye && read_float(memory, camera_address) == 50.0f,
        "L's recenter stays behind the hunter when Focus camera is bound elsewhere");
    check(std::abs(yaw_difference(f.yaw(), behind)) < std::abs(yaw_difference(0u, behind)),
        "and that press moves the camera behind the hunter");
    const std::uint16_t after_l = f.yaw();
    release_recenter(memory);
    f.frame();
    check(std::abs(yaw_difference(f.yaw(), wanted)) >= std::abs(yaw_difference(after_l, wanted)),
        "letting go of L does not then aim at the monster");
    memory.store8(camera_address + 0x8Eu, 0u);
    memory.store8(camera_address + 0x8Du, 0u);

    set_focus_camera_held(true);
    pad_recenter = 0x200u;
    overlay_recenters = false;
    memory.store16(hunters + 0xA72u, 0x200u);
    memory.store32(hunter + 1152u, 0x200u);
    memory.store8(camera_address + 0x91u, 0xFFu);
    const std::uint16_t aside = f.yaw();
    f.frame();
    check(std::abs(yaw_difference(f.yaw(), wanted)) < std::abs(yaw_difference(aside, wanted)),
        "Focus camera bound to a press the game reads as R still aims at the monster");
    check(memory.load32(hunter + 500u) == 0x0000C000u, "the hunter's facing is put back");
    check(lock_on_status().locked, "and the monster stays selected");
    set_focus_camera_held(false);
    release_recenter(memory);
    keys[0] = {};
}

void test_focus_double_tap_recenters_behind_the_hunter() {
    Fixture f;
    mhfu::settings::current().lock_on_style = mhfu::settings::LockOnStyle::Focus;
    auto &memory = f.runtime.memory();
    memory.store32(hunter_list, hunters);
    memory.store32(hunters + 2692u, hunter);
    memory.store32(hunter + 500u, 0x00008000u); // behind the hunter is yaw 0, where the camera started
    lock_on_tap();
    f.frame();
    place(memory, monster_b, 3000.0f, 0.0f); // a quarter turn off the camera
    memory.store8(camera_address + 0x8Eu, 1u);
    f.frame();
    check(game_camera_driving(), "the first tap starts the turn");
    const std::uint16_t mid = f.yaw();
    memory.store8(camera_address + 0x8Eu, 1u);
    game_pulls_recenter = true;
    f.frame();
    check(lock_on_status().locked, "a second L keeps the monster selected");
    check(std::abs(yaw_difference(f.yaw(), 0u)) < std::abs(yaw_difference(mid, 0u)),
        "and steps the camera back behind the hunter");
    check(read_float(memory, camera_address) == 50.0f, "the eye that recenter placed is left there");
    const std::uint16_t behind = f.yaw();
    for (int i = 0; i < 5; ++i) f.frame();
    check(f.yaw() == behind, "the turn does not resume while that recenter is the game's");

    memory.store8(camera_address + 0x8Eu, 0u);
    memory.store8(camera_address + 0x8Du, 0u);
    f.frame();
    for (int i = 0; i < 12; ++i) f.frame();
    memory.store8(camera_address + 0x8Eu, 1u);
    f.frame();
    check(game_camera_driving() && std::abs(yaw_difference(f.yaw(), 49152u)) < std::abs(yaw_difference(behind, 49152u)),
        "after that window, L centers on the monster again");
    mhfu::settings::current().lock_on_style = mhfu::settings::LockOnStyle::Follow;
}

void test_focus_later_l_keeps_the_turn() {
    Fixture f;
    mhfu::settings::current().lock_on_style = mhfu::settings::LockOnStyle::Focus;
    auto &memory = f.runtime.memory();
    memory.store32(hunter_list, hunters);
    memory.store32(hunters + 2692u, hunter);
    memory.store32(hunter + 500u, 0x00008000u); // behind the hunter is yaw 0
    lock_on_tap();
    f.frame();
    // Far enough that the turn is still going after the double-tap window.
    place(memory, monster_b, 0.0f, 3000.0f);
    memory.store8(camera_address + 0x8Eu, 1u);
    f.frame();
    for (int i = 0; i < 12; ++i) f.frame();
    check(game_camera_driving(), "the turn is still on its way after the double-tap window");
    const std::uint16_t mid = f.yaw();
    game_pulls_recenter = true;
    f.frame();
    check(game_camera_driving() && lock_on_status().locked, "a later L does not let the camera go");
    check(std::abs(yaw_difference(f.yaw(), 32768u)) < std::abs(yaw_difference(mid, 32768u)),
        "and the camera keeps moving toward the monster");
    check(read_float(memory, camera_address) == 0.0f, "the recenter behind the hunter is put back");
    mhfu::settings::current().lock_on_style = mhfu::settings::LockOnStyle::Follow;
}

void test_focus_reticle_and_close_turn() {
    Fixture f;
    mhfu::settings::current().lock_on_style = mhfu::settings::LockOnStyle::Focus;
    auto &memory = f.runtime.memory();
    // Beside the hunter, inside the distance where Follow holds its yaw.
    place(memory, monster_b, 150.0f, 0.0f);
    lock_on_tap();
    f.frame();
    check(lock_on_status().locked && lock_on_status().target == monster_b, "focus selects the monster beside the hunter");
    check(!lock_on_marker_wanted(), "R3 selects without a reticle");
    for (int i = 0; i < 10; ++i) f.frame();
    check(!lock_on_marker_wanted() && lock_on_status().locked, "the selection stays unmarked");
    lock_on_tap();
    f.frame();
    check(lock_on_status().target == monster_a && !lock_on_marker_wanted(), "another selection leaves the reticle off");

    memory.store8(camera_address + 0x8Eu, 1u);
    f.frame();
    check(lock_on_marker_wanted(), "L shows the reticle");
    for (int i = 0; i < 9; ++i) f.frame();
    check(lock_on_marker_wanted(), "it stays for a third of a second after L");
    f.frame();
    check(!lock_on_marker_wanted(), "then the reticle expires");
    check(!lock_on_marker(memory), "and none is drawn");
    check(lock_on_status().locked, "the selection stays");
    for (int i = 0; i < 60; ++i) f.frame();
    check(std::abs(yaw_difference(f.yaw(), 49152u)) < 64, "a monster that close is still turned to");
    check(!lock_on_marker_wanted() && lock_on_status().locked, "the reticle stays gone after the turn");

    memory.store8(camera_address + 0x8Eu, 1u);
    f.frame();
    check(lock_on_marker_wanted(), "another L shows the reticle again");
    for (int i = 0; i < 10; ++i) f.frame();
    check(!lock_on_marker_wanted() && lock_on_status().locked, "and that one expires too");

    mhfu::settings::current().lock_on_style = mhfu::settings::LockOnStyle::Follow;
    check(lock_on_marker_wanted(), "follow shows the reticle for the whole selection");
}

void test_focus_tilts_toward_a_monster_above() {
    Fixture f;
    mhfu::settings::current().lock_on_style = mhfu::settings::LockOnStyle::Focus;
    auto &memory = f.runtime.memory();
    place(memory, monster_b, 0.0f, -3000.0f);
    // About 30 degrees above the look-at point. Follow would stop 10 degrees
    // up from the preset; this tap goes most of the way to the monster.
    write_float(memory, monster_b + game::kPosition + 4u, 1822.0f);
    lock_on_tap();
    f.frame();
    const float eye = read_float(memory, camera_address + 4u);
    memory.store8(camera_address + 0x8Eu, 1u);
    f.frame();
    for (int i = 0; i < 20; ++i) f.frame();
    check(read_float(memory, camera_address + 4u) < eye - 100.0f,
        "a monster above the hunter pulls the camera up past the old ten-degree cap");
    check(!game_camera_driving(), "the tap still ends");
    check(lock_on_status().locked, "and the monster stays selected");
    mhfu::settings::current().lock_on_style = mhfu::settings::LockOnStyle::Follow;
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

// The three instructions the marker requires before it reads the camera object:
// the pointer store, the view's addiu, and the projection's addiu.
void allow_camera_object(psprecomp::GuestMemory &memory) {
    memory.store32(0x08812954u, 0xAC50BD04u);
    memory.store32(0x0881439Cu, 0x26640C80u);
    memory.store32(0x088145D8u, 0x26050C00u);
}

void store_matrix(psprecomp::GuestMemory &memory, std::uint32_t address, const std::array<float, 16> &matrix) {
    for (std::uint32_t i = 0; i < 16u; ++i) write_float(memory, address + i * 4u, matrix[i]);
}

void test_marker_tracks_the_camera_object() {
    Fixture f;
    f.frame();
    lock_on_tap();
    f.frame();
    check(lock_on_status().locked, "locked, so the marker has a monster");
    const std::uint32_t target = lock_on_status().target;
    auto &memory = f.runtime.memory();
    // Far from the origin. A view with no translation leaves this off the picture.
    const float eye_x = 1000.0f;
    const float eye_y = 200.0f;
    const float eye_z = -400.0f;
    place(memory, target, eye_x, eye_z - 10.0f);
    write_float(memory, target + game::kPosition + 4u, eye_y - 100.0f);
    const LockOnView drawn = picture_camera();
    lock_on_note_view(drawn, 100u);
    check(!lock_on_marker(memory), "the draw's view has no translation, so the monster is not on that picture");

    constexpr std::uint32_t object = 0x09400000u;
    allow_camera_object(memory);
    memory.store32(0x089CBD04u, object);
    LockOnView tracked = picture_camera();
    tracked.view[12] = -eye_x;
    tracked.view[13] = -eye_y;
    tracked.view[14] = -eye_z;
    store_matrix(memory, object + 0xC80u, tracked.view);
    store_matrix(memory, object + 0xC00u, tracked.projection);
    lock_on_note_view(drawn, 100u);
    const auto at = lock_on_marker(memory);
    check(at && (*at)[0] == 0.5f && (*at)[1] == 0.5f,
        "the ring sits on the monster through the camera object's view and projection");

    LockOnView shifted = drawn;
    shifted.x_offset = 0.0f;
    lock_on_note_view(shifted, 100u);
    const auto edge = lock_on_marker(memory);
    check(edge && (*edge)[0] == 0.0f, "the world draw still supplies the viewport");

    const auto unprompted = lock_on_marker(memory);
    check(unprompted && (*unprompted)[0] == 0.5f && (*unprompted)[1] == 0.5f,
        "with no draw noted, the full picture is the viewport");

    place(memory, target, eye_x, eye_z + 10.0f);
    write_float(memory, target + game::kPosition + 4u, eye_y - 100.0f);
    check(!lock_on_marker(memory), "a monster behind that camera is not marked");

    place(memory, target, eye_x + 10000.0f, eye_z - 10.0f);
    write_float(memory, target + game::kPosition + 4u, eye_y - 100.0f);
    check(!lock_on_marker(memory), "a monster off that picture is not marked");

    write_float(memory, object + 0xC00u + 11u * 4u, 0.0f);
    place(memory, target, 0.0f, -10.0f);
    write_float(memory, target + game::kPosition + 4u, -100.0f);
    lock_on_note_view(drawn, 100u);
    const auto fallback = lock_on_marker(memory);
    check(fallback && (*fallback)[0] == 0.5f && (*fallback)[1] == 0.5f,
        "a projection that was not built falls back to the draw");

    memory.store32(0x089CBD04u, 0u);
    lock_on_note_view(drawn, 100u);
    const auto missing = lock_on_marker(memory);
    check(missing && (*missing)[0] == 0.5f, "a missing camera object falls back to the draw");
    lock_on_reset();
}

void test_marker_follows_the_camera_eye() {
    Fixture f;
    f.frame();
    lock_on_tap();
    f.frame();
    check(lock_on_status().locked, "locked, so the eye can place a reticle");
    const std::uint32_t target = lock_on_status().target;
    auto &memory = f.runtime.memory();
    const float eye_x = 1000.0f;
    const float eye_y = 200.0f;
    const float eye_z = -400.0f;
    place(memory, target, eye_x, eye_z - 10.0f);
    write_float(memory, target + game::kPosition + 4u, eye_y - 100.0f);
    lock_on_note_view(picture_camera(), 100u);
    check(!lock_on_marker(memory), "the uploaded view has no translation, so the monster is off that picture");

    write_float(memory, camera_address + 0u, eye_x);
    write_float(memory, camera_address + 4u, eye_y);
    write_float(memory, camera_address + 8u, eye_z);
    write_float(memory, camera_address + 0x10u, eye_x);
    write_float(memory, camera_address + 0x14u, eye_y);
    write_float(memory, camera_address + 0x18u, eye_z - 100.0f);
    lock_on_note_view(picture_camera(), 100u);
    const auto at = lock_on_marker(memory);
    check(at && std::fabs((*at)[0] - 0.5f) < 0.02f && std::fabs((*at)[1] - 0.5f) < 0.02f,
        "the reticle sits on the monster through the camera's eye and look-at");
    // Back near the origin, in front of a view that has no translation. The
    // eye at x 1000 cannot see it. That draw must not put a reticle up.
    place(memory, target, 0.0f, -10.0f);
    write_float(memory, target + game::kPosition + 4u, -100.0f);
    lock_on_note_view(picture_camera(), 100u);
    check(!lock_on_marker(memory), "a monster the camera cannot see draws no reticle");
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
    check(d.pad[static_cast<std::size_t>(Action::LockOn)][0] == single(pad(PadInput::LeftTrigger)),
        "LT locks on in the default preset");
    check(d.pad[static_cast<std::size_t>(Action::FocusCamera)][0] == single(pad(PadInput::LeftTrigger)),
        "and that tap starts focus and follow");
    check(d.pad[static_cast<std::size_t>(Action::L)][0] == single(pad(PadInput::LeftShoulder)) &&
            d.pad[static_cast<std::size_t>(Action::L)][1].empty(),
        "LB is L: the camera and item use");
    check(acts_on_release(Action::LockOn) && !acts_on_release(Action::Screenshot), "only lock-on acts on release");
    check(layout(Preset::LeftHanded).pad[static_cast<std::size_t>(Action::LockOn)][0] ==
                single(pad(PadInput::RightTrigger)) &&
            layout(Preset::LeftHanded).pad[static_cast<std::size_t>(Action::L)][0] ==
                single(pad(PadInput::RightShoulder)),
        "left-handed puts lock-on on RT and L on RB");
    check(layout(Preset::Modern).pad[static_cast<std::size_t>(Action::LockOn)][0] == single(pad(PadInput::RightStick)),
        "Modern keeps lock-on on R3, since LT guards");
    check(d.keys[static_cast<std::size_t>(Action::LockOn)][0] == single(mouse_button(2)) &&
            d.keys[static_cast<std::size_t>(Action::LockOn)][1] == single(from_name("T")),
        "the middle mouse button and T lock on from the keyboard");
    check(buttons_of(Action::FocusCamera) == 0u && group_of(Action::FocusCamera) == ActionGroup::Camera &&
            !acts_on_release(Action::FocusCamera),
        "Focus camera is a camera bind and presses no PSP button");
    {
        const Slots &lock = d.pad[static_cast<std::size_t>(Action::LockOn)];
        const Chord shot = d.pad[static_cast<std::size_t>(Action::Screenshot)][0];
        check(!extends_lock_on(lock, shot), "the screenshot chord is the stick click, not LT");
        check(!extends_lock_on(lock, chord(pad(PadInput::LeftStick), pad(PadInput::RightStick))),
            "L3 + R3 is the menu, not the lock-on press");
        check(!extends_lock_on(lock, single(pad(PadInput::RightShoulder))),
            "holding R, as for a sprint, still leaves the lock-on press");
        check(!extends_lock_on(lock, single(pad(PadInput::LeftTrigger))), "LT on its own is the lock-on press");
        check(extends_lock_on(lock, chord(pad(PadInput::LeftTrigger), pad(PadInput::South))),
            "a longer chord that contains LT does not also lock on");
        check(!extends_lock_on(lock, chord(pad(PadInput::RightShoulder), pad(PadInput::South))),
            "two other buttons, with no lock-on input, are not that chord");
    }
    for (std::size_t i = 0; i < kPresets; ++i) {
        const Layout &l = layout(static_cast<Preset>(i));
        const auto slot = static_cast<std::size_t>(Action::FocusCamera);
        check(l.keys[slot][0].empty(), "no keyboard preset binds Focus camera, so that tap stays L");
        if (static_cast<Preset>(i) == Preset::Modern) {
            check(l.pad[slot][0].empty(), "Modern leaves Focus camera unbound, so L does it");
            continue;
        }
        const PadInput trigger =
            static_cast<Preset>(i) == Preset::LeftHanded ? PadInput::RightTrigger : PadInput::LeftTrigger;
        check(l.pad[slot][0] == single(pad(trigger)), "the other presets start focus and follow on the lock-on trigger");
    }
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

void test_shared_trigger_initiates_focus_and_follow() {
    using namespace mhfu::input;
    Fixture f;
    auto &memory = f.runtime.memory();
    const auto lock = static_cast<std::size_t>(Action::LockOn);
    const auto focus = static_cast<std::size_t>(Action::FocusCamera);
    const Chord trigger = single(pad(PadInput::LeftTrigger));
    mhfu::settings::current().controls.pad[lock][0] = trigger;
    mhfu::settings::current().controls.pad[focus][0] = trigger;
    place(memory, monster_b, 800.0f, -3000.0f);
    mhfu::settings::current().lock_on_style = mhfu::settings::LockOnStyle::Follow;
    lock_on_tap();
    f.frame();
    check(lock_on_status().locked && lock_on_status().target == monster_b && game_camera_driving(),
        "LT locks on and follow starts");
    f.frame(1.0f);
    check(lock_on_status().locked && !game_camera_driving(), "the stick pauses follow");
    lock_on_tap();
    f.frame();
    check(lock_on_status().target == monster_b && game_camera_driving() && lock_on_marker_wanted(),
        "LT turns follow back on without moving to the next monster");
    lock_on_reset();
    mhfu::settings::current().lock_on_style = mhfu::settings::LockOnStyle::Focus;
    const std::uint16_t wanted = yaw_towards({0.0f, 190.0f, 0.0f}, {800.0f, 100.0f, -3000.0f});
    lock_on_tap();
    f.frame();
    check(lock_on_status().target == monster_b &&
            std::abs(yaw_difference(f.yaw(), wanted)) < std::abs(yaw_difference(0u, wanted)),
        "the same tap starts a focus turn, without L");
    for (int i = 0; i < 15; ++i) f.frame();
    lock_on_tap();
    f.frame();
    check(lock_on_status().target == monster_b, "another LT centers again instead of moving to the next monster");
    mhfu::settings::current().controls.pad[focus][0] = {};
}

void test_follow_toggle_uses_the_focus_camera_bind() {
    using namespace mhfu::input;
    Fixture f;
    auto &memory = f.runtime.memory();
    auto &keys = mhfu::settings::current().controls.keys[static_cast<std::size_t>(Action::FocusCamera)];
    keys[0] = single(from_name("F"));
    set_focus_camera_held(true);
    f.frame();
    set_focus_camera_held(false);
    f.frame();
    check(!lock_on_status().locked, "with nothing selected, the focus button does not lock on");
    lock_on_tap();
    f.frame();
    check(lock_on_status().locked && game_camera_driving(), "follow starts with the camera on the monster");
    place(memory, monster_b, 3000.0f, 0.0f);
    f.frame();
    const std::uint16_t moving = f.yaw();
    check(moving != 0u, "the camera has started toward the monster");
    set_focus_camera_held(true);
    f.frame();
    const std::uint16_t paused = f.yaw();
    check(lock_on_status().locked && !lock_on_marker_wanted() && !game_camera_driving(),
        "the focus button turns follow off, hides the reticle, and keeps the monster");
    for (int i = 0; i < 5; ++i) f.frame();
    check(f.yaw() == paused && lock_on_status().locked, "holding the button does not turn follow back on");
    f.frame(1.0f);
    f.frame(1.0f);
    check(lock_on_status().locked && !lock_on_marker_wanted() && f.yaw() == paused,
        "looking around while follow is off does not let go or bring the reticle back");
    set_focus_camera_held(false);
    f.frame();
    set_focus_camera_held(true);
    f.frame();
    set_focus_camera_held(false);
    for (int i = 0; i < 20; ++i) f.frame();
    check(lock_on_status().locked && lock_on_marker_wanted() &&
            std::abs(yaw_difference(f.yaw(), 49152u)) < std::abs(yaw_difference(paused, 49152u)),
        "the same button turns follow back on");
    f.frame(1.0f);
    check(lock_on_status().locked && !lock_on_marker_wanted() && !game_camera_driving(),
        "the right stick turns tracking off and the reticle with it");
    set_focus_camera_held(true);
    f.frame();
    set_focus_camera_held(false);
    check(lock_on_status().locked && lock_on_marker_wanted() && game_camera_driving(),
        "the same button turns tracking and the reticle back on");
    keys[0] = {};
}

int main() {
    test_geometry();
    test_monsters_and_pick();
    test_monster_in_another_area();
    test_hunt_portrait_keeps_the_id_in_another_area();
    test_unmarked_monsters_still_lock();
    test_lock_follows_and_lets_go();
    test_shared_trigger_initiates_focus_and_follow();
    test_follow_toggle_uses_the_focus_camera_bind();
    test_focus_turns_on_l_and_keeps_the_selection();
    test_focus_keeps_the_picture_when_the_hunter_is_aside();
    test_focus_l_does_not_place_the_eye_behind_the_hunter();
    test_r_does_not_turn_toward_the_monster();
    test_focus_camera_bind_replaces_l();
    test_focus_double_tap_recenters_behind_the_hunter();
    test_focus_later_l_keeps_the_turn();
    test_focus_reticle_and_close_turn();
    test_focus_turn_lets_go_promptly();
    test_focus_centers_a_monster_already_on_screen();
    test_focus_tilts_toward_a_monster_above();
    test_focus_turns_when_facing_away();
    test_tap_waits_for_the_monster();
    test_off_changes_nothing();
    test_marker_sits_on_the_drawn_view();
    test_marker_tracks_the_camera_object();
    test_marker_follows_the_camera_eye();
    test_binds();
    std::cout << (failures ? "FAIL" : "PASS") << ": lock-on (" << failures << " failures)\n";
    return failures ? 1 : 0;
}
