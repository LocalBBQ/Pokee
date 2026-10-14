#include "camera/lock_on.hpp"

#include "camera/game_camera.hpp"
#include "camera/hunt_icon.hpp"
#include "settings/settings.hpp"

#include "psprecomp/runtime.hpp"

#include <algorithm>
#include <bit>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <string_view>
#include <utility>

namespace mhfu::camera {
namespace {

using namespace lock_on_layout;

constexpr float kPi = 3.14159265358979323846f;
constexpr float kUnitsPerTurn = 65536.0f;
constexpr float kDegrees = 180.0f / kPi;

// The overlay header: "MWo3", its load address at +8, its name at +32.
constexpr std::uint32_t kOverlayMagic = 0x336F574Du;
constexpr std::string_view kQuestOverlay = "game_task.ovl";

// Farther than this from the camera's look-at point, horizontally, a lock
// lets go: well past what the camera can frame. Areas are a few thousand
// units across.
constexpr float kFarthest = 6000.0f;
// Where on the monster the camera aims: a little above its feet, which is
// where the game keeps its position.
constexpr float kAimHeight = 100.0f;
// Turning by hand lets go once this many degrees have built up, with what
// came before fading by kManualFade each update, so a stray nudge of the
// mouse does not.
constexpr float kManualRelease = 6.0f;
constexpr float kManualFade = 0.7f;
// How the yaw follows: a share of the rest each update (30 a second), at
// least kSmallestStep and at most kLargestStep units.
constexpr float kYawShare = 0.2f;
constexpr int kSmallestStep = 48;  // about a quarter of a degree
constexpr int kLargestStep = 2048; // 11.25 degrees
// And the pitch: the eye rises or sinks by half the monster's elevation as
// seen from the look-at point, at most kPitchUp or kPitchDown from where it
// was at the lock, a share of the rest each update.
constexpr float kPitchFollow = 0.5f;
constexpr float kPitchUp = 10.0f;
constexpr float kPitchDown = 20.0f;
constexpr float kPitchShare = 0.12f;
constexpr float kPitchLowest = -30.0f;
constexpr float kPitchHighest = 60.0f;
// Nearer than kNear to the camera's look-at point, horizontally, the
// monster is on top of the hunter and its direction means little: the yaw
// holds. Beyond, the turn comes in over kNearBlend.
constexpr float kNear = 300.0f;
constexpr float kNearBlend = 400.0f;
// Past this much of a turn to go, the turn keeps the way it was going.
constexpr int kHalfTurnBand = 28672; // 157.5 degrees
// A first tap that no camera update took in this many flips is dropped.
constexpr unsigned kTapFrames = 6u;
// A tap as a zone finishes loading finds nothing: the monster is inserted a
// moment later. The tap keeps looking for this many follow-camera updates.
constexpr unsigned kSeekUpdates = 90u;
// A Focus tap takes a larger share of the rest, closes the last
// kFocusArrival units at once, and gives up after kFocusUpdates (one
// second) if the monster keeps the rest above that by moving. Off the
// picture the step is at most kFocusSweep; inside it, kLargestStep, so a
// monster already on screen is centered rather than jumped to.
constexpr float kFocusShare = 0.3f;
constexpr int kFocusArrival = 364; // 2 degrees
constexpr int kFocusSweep = 4096;  // 22.5 degrees
constexpr unsigned kFocusUpdates = 30u;
// How far a Focus tap tilts toward the monster's elevation. The result is
// the camera's own pitch limits, not a band around the angle L was pressed at.
constexpr float kFocusPitchFollow = 0.85f;
// A second L within this many camera updates (a third of a second) of the
// tap that started the turn is the game's recenter behind the hunter.
constexpr unsigned kDoubleTap = 10u;
// Focus shows the reticle when the focus button is pressed, for this many
// camera updates, a third of a second, and then it goes. Selecting with R3
// leaves it off. The selection stays.
constexpr unsigned kMarkerLife = 10u;
// The picture's half width, from the projection's vertical field of view
// (camera/game_aspect.cpp) at the game's 480 by 272.
constexpr float kVerticalFov = 0.872222f;
constexpr float kPictureAspect = 480.0f / 272.0f;

struct State {
    bool tap{};
    unsigned tap_age{};
    // The shared lock-on trigger asked for a Focus turn. The camera update
    // takes it; a hold of that trigger is not itself the focus button.
    bool shared_focus{};
    bool seeking{};
    unsigned seek_left{};
    bool locked{};
    // Follow: tracking is off, and the reticle with it. The selection stays.
    // Focus/Follow Camera turns both on again. The right stick only turns them off.
    bool follow_paused{};
    bool focusing{}; // Focus: L's turn is in progress; the selection stays after
    unsigned focus_left{};
    unsigned double_tap_left{}; // updates left in which a second L recenters behind
    bool yielding{};            // that second L: the game's recenter owns the camera
    bool reconciled{}; // Focus: L's turn has landed
    unsigned marker_left{}; // Focus: updates left before the reticle expires
    bool marker_expired{};
    std::uint32_t target{};
    std::vector<std::uint32_t> visited;
    float manual{};
    int turning{}; // the way the last step went: +1, -1 or 0
    bool pitch_known{};
    float base_pitch{};
    float pitch{};
    std::uint64_t updates{};
    bool trace_walk{}; // MHFU_TRACE_LOCK_ON: list the next walk's candidates
    // Monster icons, in the order each one was first shown this quest. The
    // entity list puts the newest node first, and the game reinserts a
    // monster when it changes area, which would move its icon.
    std::vector<std::uint32_t> icon_order;
    // The perspective draw with the most vertices since the marker last asked.
    LockOnView view{};
    std::uint32_t view_vertices{};
};
State state;

bool trace_on() {
    static const bool on = std::getenv("MHFU_TRACE_LOCK_ON") != nullptr;
    return on;
}

float load_float(const psprecomp::GuestMemory &memory, std::uint32_t address) {
    return std::bit_cast<float>(memory.load32(address));
}

Vec3 load_vec(const psprecomp::GuestMemory &memory, std::uint32_t address) {
    return {load_float(memory, address), load_float(memory, address + 4u), load_float(memory, address + 8u)};
}

bool focus_style();

bool finite(const Vec3 &v) {
    return std::isfinite(v.x) && std::isfinite(v.y) && std::isfinite(v.z);
}

float horizontal(const Vec3 &from, const Vec3 &to) {
    return std::hypot(to.x - from.x, to.z - from.z);
}

// The stage the quest has loaded, or nothing when GameSys is not up yet.
std::optional<std::uint16_t> loaded_stage(const psprecomp::GuestMemory &memory) {
    if (memory.raw_pointer(kGameSysPointer, 4u) == nullptr) return std::nullopt;
    const std::uint32_t sys = memory.load32(kGameSysPointer);
    if (sys < 0x08800000u || memory.raw_pointer(sys + kGameSysStage, 2u) == nullptr) return std::nullopt;
    return memory.load16(sys + kGameSysStage);
}

// Another zone is the object's stage id not matching the loaded stage, the
// same compare 0x08813630 makes. Where it stands does not count. With no
// loaded stage the zone is unknown, so the monster stays selectable.
bool in_another_zone(const psprecomp::GuestMemory &memory, std::uint32_t entity) {
    const std::optional<std::uint16_t> loaded = loaded_stage(memory);
    if (!loaded) return false;
    return memory.load16(entity + kStageId) != *loaded;
}

bool on_quest(const psprecomp::GuestMemory &memory) {
    if (memory.raw_pointer(kTaskSlot, 64u) == nullptr || memory.load32(kTaskSlot) != kOverlayMagic ||
        memory.load32(kTaskSlot + 8u) != kTaskSlot)
        return false;
    for (std::size_t i = 0; i < kQuestOverlay.size(); ++i)
        if (memory.load8(kTaskSlot + 32u + static_cast<std::uint32_t>(i)) !=
            static_cast<std::uint8_t>(kQuestOverlay[i]))
            return false;
    return memory.load8(kTaskSlot + 32u + static_cast<std::uint32_t>(kQuestOverlay.size())) == 0u;
}

const LockMonster *find(const std::vector<LockMonster> &monsters, std::uint32_t address) {
    for (const LockMonster &m : monsters)
        if (m.address == address) return &m;
    return nullptr;
}

void lock(std::uint32_t target, const char *how) {
    state.locked = true;
    state.follow_paused = false;
    state.target = target;
    state.reconciled = false;
    // Focus leaves the reticle off until the focus button. Follow keeps it
    // for the whole selection.
    state.marker_expired = focus_style();
    state.marker_left = 0u;
    state.manual = 0.0f;
    state.turning = 0;
    state.visited.push_back(target);
    std::cout << "[lockon] " << how << " the monster at 0x" << std::hex << target << std::dec << std::endl;
}

bool em_method(const psprecomp::GuestMemory &memory, std::uint32_t vtable) {
    if (vtable < 0x08800000u || memory.raw_pointer(vtable, kVtableSpan) == nullptr) return false;
    for (std::uint32_t off = kVtableMethod; off < kVtableSpan; off += 4u) {
        const std::uint32_t method = memory.load32(vtable + off);
        if (method >= kEmOverlay && method < kEmOverlayEnd) return true;
    }
    return false;
}

bool marked_large(std::uint16_t class_id) {
    return class_id == 32u || class_id == 34u || class_id == 36u;
}

bool focus_style() {
    return mhfu::settings::current().lock_on_style == mhfu::settings::LockOnStyle::Focus;
}

// Lock on and Focus/Follow Camera on the same chord: one tap does both.
bool trigger_initiates_camera() {
    const input::Layout &layout = settings::current().controls;
    const auto focus = static_cast<std::size_t>(input::Action::FocusCamera);
    const auto lock = static_cast<std::size_t>(input::Action::LockOn);
    for (const bool pad : {false, true}) {
        const input::Bindings &table = pad ? layout.pad : layout.keys;
        for (const input::Chord &mine : table[lock]) {
            if (mine.empty()) continue;
            for (const input::Chord &theirs : table[focus])
                if (!theirs.empty() && mine == theirs) return true;
        }
    }
    return false;
}

} // namespace

namespace {

struct Candidate {
    LockMonster monster;
    bool marked{};
    bool other_area{};
    bool dead{};
    std::uint8_t id{};
};

// Species the hunt icon draws that are not large monsters. A small monster
// can share an em method or a class id with its large relative (a Ceanataur
// with a Shogun), and a quest that marks nothing large would otherwise lock
// it. These stay off the lock and off the icon. An id that is not here,
// including one with no icon, can still be large.
bool small_species(std::uint8_t id) {
    switch (id) {
    case 0x03: // Kelbi
    case 0x04: // Mosswine
    case 0x05: // Bullfango
    case 0x09: // Felyne
    case 0x0C: // Aptonoth
    case 0x0D: // Genprey
    case 0x10: // Velociprey
    case 0x13: // Vespoid
    case 0x17: // Melynx
    case 0x18: // Hornetaur
    case 0x19: // Apceros
    case 0x1E: // Ioprey
    case 0x22: // Cephalos
    case 0x23: // Giaprey
    case 0x39: // Shakalaka
    case 0x3D: // Blango
    case 0x3E: // Conga
    case 0x3F: // Remobra
    case 0x42: // Hermitaur
    case 0x45: // Anteka
    case 0x46: // Popo
    case 0x49: // Ceanataur
        return true;
    default: return false;
    }
}

// Large monsters on the quest list, including one in another zone. When
// `trace` is set, the walk is printed and the flag cleared,
// and only once the list can actually be read.
std::vector<Candidate> collect_monsters(const psprecomp::GuestMemory &memory, bool *trace_flag) {
    std::vector<Candidate> found;
    if (!on_quest(memory) || memory.raw_pointer(kEntityList, 4u) == nullptr) return found;
    const std::uint32_t manager = memory.load32(kEntityList);
    if (manager < 0x08800000u || memory.raw_pointer(manager + kListHead, 4u) == nullptr) return found;
    const bool trace = trace_flag != nullptr && *trace_flag;
    if (trace_flag != nullptr) *trace_flag = false;
    std::uint32_t node = memory.load32(manager + kListHead);
    std::uint32_t seen[kMaxMonsters]{};
    for (std::uint32_t n = 0; n < kMaxMonsters && node >= 0x08800000u; ++n) {
        if (memory.raw_pointer(node, kNodeClass + 2u) == nullptr) break;
        bool repeated = false;
        for (std::uint32_t earlier = 0; earlier < n; ++earlier)
            if (seen[earlier] == node) repeated = true;
        if (repeated) break;
        seen[n] = node;
        const std::uint32_t older = memory.load32(node + kNodeOlder);
        const std::uint8_t kind = memory.load8(node + kNodeType);
        const std::uint16_t class_id = memory.load16(node + kNodeClass);
        const std::uint32_t at = memory.load32(node + kNodeEntity);
        node = older;
        if (kind == kPlayerNode) continue;
        if (at < 0x08800000u || memory.raw_pointer(at, kExtent) == nullptr) continue;
        bool seen_entity = false;
        for (const Candidate &already : found)
            if (already.monster.address == at) seen_entity = true;
        if (seen_entity) continue;
        const auto most = static_cast<std::int16_t>(memory.load16(at + kMostHealth));
        const auto health = static_cast<std::int16_t>(memory.load16(at + kHealth));
        if (most <= 0) continue; // a companion, or not a monster with a health pool
        const std::uint8_t id = memory.load8(at + kMonsterId);
        if (small_species(id)) continue;
        const bool dead = health <= 0;
        const Vec3 position = load_vec(memory, at + kPosition);
        if (!finite(position)) continue;
        const bool other_area = in_another_zone(memory, at);
        const std::uint32_t vtable = memory.load32(at + kVtable);
        const bool em = em_method(memory, vtable);
        const bool marked = em || marked_large(class_id);
        if (trace) {
            const std::uint32_t method = vtable >= 0x08800000u && memory.raw_pointer(vtable + kVtableMethod, 4u)
                ? memory.load32(vtable + kVtableMethod)
                : 0u;
            std::cout << "[lockon] candidate 0x" << std::hex << at << std::dec << " type "
                      << static_cast<unsigned>(kind) << " class " << class_id << " vtable 0x" << std::hex << vtable
                      << " method 0x" << method << std::dec << " em " << (em ? 1 : 0) << " id "
                      << static_cast<unsigned>(id) << " health " << health << "/" << most
                      << " at " << position.x << "," << position.y << "," << position.z
                      << (other_area ? " other-area" : "") << (dead ? " dead" : "") << std::endl;
        }
        found.push_back({LockMonster{at, 0u, position}, marked, other_area, dead, id});
    }
    if (trace) {
        bool any_marked = false;
        unsigned lockable = 0;
        for (const Candidate &candidate : found) {
            if (candidate.other_area || candidate.dead) continue;
            ++lockable;
            if (candidate.marked) any_marked = true;
        }
        std::cout << "[lockon] " << lockable << " candidates, " << (any_marked ? "some" : "none")
                  << " marked large" << std::endl;
    }
    return found;
}

// `present` keeps a monster in another zone, and a dead one, for the
// icon. Whether anything is marked large is decided on the whole quest, so
// a large monster elsewhere or dead still keeps a small one out. Lock-on
// then has nothing to select, and the reticle stays off.
std::vector<Candidate> select_monsters(const std::vector<Candidate> &found, bool present) {
    bool any_marked = false;
    for (const Candidate &candidate : found)
        if (candidate.marked) any_marked = true;
    std::vector<Candidate> out;
    for (const Candidate &candidate : found) {
        if (!present && (candidate.other_area || candidate.dead)) continue;
        if (any_marked && !candidate.marked) continue;
        out.push_back(candidate);
    }
    return out;
}

} // namespace

std::vector<LockMonster> lock_on_monsters(const psprecomp::GuestMemory &memory) {
    std::vector<LockMonster> out;
    for (const Candidate &candidate : select_monsters(collect_monsters(memory, &state.trace_walk), false)) {
        out.push_back(candidate.monster);
        out.back().slot = static_cast<std::uint32_t>(out.size() - 1u);
    }
    return out;
}

std::vector<HuntMonster> hunt_monsters(const psprecomp::GuestMemory &memory) {
    if (!on_quest(memory)) {
        state.icon_order.clear();
        return {};
    }
    std::vector<HuntMonster> found;
    for (const Candidate &candidate : select_monsters(collect_monsters(memory, nullptr), true))
        found.push_back(HuntMonster{candidate.monster.address, candidate.id, candidate.other_area, candidate.dead});
    for (const HuntMonster &monster : found) {
        bool known = false;
        for (const std::uint32_t address : state.icon_order)
            if (address == monster.address) known = true;
        if (!known) state.icon_order.push_back(monster.address);
    }
    std::vector<HuntMonster> out;
    for (const std::uint32_t address : state.icon_order)
        for (const HuntMonster &monster : found)
            if (monster.address == address) out.push_back(monster);
    return out;
}

namespace {

const char *why_target_left(const psprecomp::GuestMemory &memory, std::uint32_t address) {
    for (const HuntMonster &monster : hunt_monsters(memory)) {
        if (monster.address != address) continue;
        if (monster.dead) return "Lock-on: the monster is dead";
        if (monster.other_area) return "Lock-on: the monster is in another area";
    }
    return "Lock-on: the monster is gone";
}

} // namespace

std::vector<HuntPortrait> place_hunt_portraits(
    const std::vector<HuntMonster> &monsters, std::uint32_t locked_target, bool locked) {
    // Bottom left of the 480×272 picture, in the order each monster was first
    // shown this quest. A later reshuffle of the entity list, leaving the
    // area, or dying, keeps that place. The player's size is a percent of
    // the usual icon.
    const float scale = static_cast<float>(std::clamp(settings::current().hunt_portrait_size,
                            settings::kMinHuntPortraitSize, settings::kMaxHuntPortraitSize)) /
        100.0f;
    const float kWidth = kHuntCardWidth * scale;
    const float kHeight = kHuntCardHeight * scale;
    constexpr float kMargin = 6.0f;
    const float kGap = 4.0f * scale;
    std::vector<HuntPortrait> out;
    for (std::size_t i = 0; i < monsters.size(); ++i) {
        const float x = kMargin + static_cast<float>(i) * (kWidth + kGap);
        if (x + kWidth > 480.0f - kMargin) break;
        const HuntMonster &monster = monsters[i];
        out.push_back(HuntPortrait{monster.id, monster.other_area, monster.dead,
            locked && !monster.other_area && !monster.dead && monster.address == locked_target, x,
            272.0f - kMargin - kHeight, kWidth, kHeight});
    }
    return out;
}

namespace {
// Why a tap found nothing, from the same walk as lock_on_monsters. One line.
void explain_miss(const psprecomp::GuestMemory &memory) {
    if (!on_quest(memory)) {
        std::cout << "[lockon] no large monster to lock onto: not in a quest" << std::endl;
        return;
    }
    if (memory.raw_pointer(kEntityList, 4u) == nullptr) {
        std::cout << "[lockon] no large monster to lock onto: entity list unreadable" << std::endl;
        return;
    }
    const std::uint32_t manager = memory.load32(kEntityList);
    if (manager < 0x08800000u || memory.raw_pointer(manager + kListHead, 4u) == nullptr) {
        std::cout << "[lockon] no large monster to lock onto: no entity manager at 0x" << std::hex << manager
                  << std::dec << std::endl;
        return;
    }
    unsigned nodes = 0;
    unsigned players = 0;
    unsigned unwell = 0;
    unsigned unplaced = 0;
    std::uint32_t sample = 0;
    std::uint32_t sample_method = 0;
    std::uint8_t sample_type = 0;
    int sample_health = 0;
    int sample_most = 0;
    std::uint32_t node = memory.load32(manager + kListHead);
    std::uint32_t seen[kMaxMonsters]{};
    for (std::uint32_t n = 0; n < kMaxMonsters && node >= 0x08800000u; ++n) {
        if (memory.raw_pointer(node, kNodeType + 1u) == nullptr) break;
        bool repeated = false;
        for (std::uint32_t earlier = 0; earlier < n; ++earlier)
            if (seen[earlier] == node) repeated = true;
        if (repeated) break;
        seen[n] = node;
        const std::uint8_t kind = memory.load8(node + kNodeType);
        const std::uint32_t at = memory.load32(node + kNodeEntity);
        node = memory.load32(node + kNodeOlder);
        ++nodes;
        if (kind == kPlayerNode) {
            ++players;
            continue;
        }
        if (at < 0x08800000u || memory.raw_pointer(at, kExtent) == nullptr) continue;
        const std::uint32_t vtable = memory.load32(at + kVtable);
        const std::uint32_t method =
            vtable >= 0x08800000u && memory.raw_pointer(vtable + kVtableMethod, 4u) != nullptr
                ? memory.load32(vtable + kVtableMethod)
                : 0u;
        const auto most = static_cast<std::int16_t>(memory.load16(at + kMostHealth));
        const auto health = static_cast<std::int16_t>(memory.load16(at + kHealth));
        if (sample == 0) {
            sample = at;
            sample_method = method;
            sample_type = kind;
            sample_health = health;
            sample_most = most;
        }
        if (most <= 0 || health <= 0) {
            ++unwell;
            continue;
        }
        if (!finite(load_vec(memory, at + kPosition))) ++unplaced;
    }
    std::cout << "[lockon] no large monster to lock onto: " << nodes << " entities, " << players << " players, "
              << unwell << " without health, " << unplaced << " with no position";
    if (sample != 0)
        std::cout << "; first at 0x" << std::hex << sample << " type " << std::dec
                  << static_cast<unsigned>(sample_type) << " method 0x" << std::hex << sample_method << std::dec
                  << " health " << sample_health << "/" << sample_most;
    std::cout << std::endl;
}

} // namespace

std::uint16_t yaw_towards(const Vec3 &from, const Vec3 &to) {
    // The camera looks along -(sin yaw, cos yaw); the eye sits the other way.
    const float angle = std::atan2(-(to.x - from.x), -(to.z - from.z));
    const long units = std::lround(angle / (2.0f * kPi) * kUnitsPerTurn);
    return static_cast<std::uint16_t>(units & 0xFFFF);
}

int yaw_difference(std::uint16_t from, std::uint16_t to) {
    return static_cast<std::int16_t>(static_cast<std::uint16_t>(to - from));
}

std::optional<std::uint32_t> lock_on_pick(const std::vector<LockMonster> &monsters, const Vec3 &look_at,
    std::uint16_t camera_yaw, std::optional<std::uint32_t> current, const std::vector<std::uint32_t> &visited) {
    if (monsters.empty()) return std::nullopt;
    if (!current) {
        const LockMonster *best = nullptr;
        bool best_in_view = false;
        float best_distance = 0.0f;
        for (const LockMonster &m : monsters) {
            const int off = yaw_difference(camera_yaw, yaw_towards(look_at, m.position));
            const bool in_view = std::abs(static_cast<float>(off)) * 360.0f / kUnitsPerTurn <= kInView;
            const float distance = horizontal(look_at, m.position);
            if (best == nullptr || (in_view && !best_in_view) ||
                (in_view == best_in_view && distance < best_distance)) {
                best = &m;
                best_in_view = in_view;
                best_distance = distance;
            }
        }
        return best->address;
    }
    // The next one in the table after the current one, round the table,
    // that this lock has not been on.
    std::uint32_t from = 0u;
    for (const LockMonster &m : monsters)
        if (m.address == *current) from = m.slot;
    for (std::uint32_t step = 1; step <= kMaxMonsters; ++step) {
        const std::uint32_t slot = (from + step) % kMaxMonsters;
        for (const LockMonster &m : monsters)
            if (m.slot == slot && m.address != *current &&
                std::find(visited.begin(), visited.end(), m.address) == visited.end())
                return m.address;
    }
    return std::nullopt;
}

int screen_half_yaw() {
    const float half = std::atan(kPictureAspect * std::tan(kVerticalFov * 0.5f));
    return static_cast<int>(std::lround(half / (2.0f * kPi) * kUnitsPerTurn));
}

bool lock_on_monster_in_view(const psprecomp::GuestMemory &memory, std::uint16_t yaw, const Vec3 &look_at) {
    if (!state.locked) return false;
    const LockMonster *target = find(lock_on_monsters(memory), state.target);
    if (target == nullptr || !finite(target->position)) return false;
    const Vec3 aim{target->position.x, target->position.y + kAimHeight, target->position.z};
    return std::abs(yaw_difference(yaw, yaw_towards(look_at, aim))) <= screen_half_yaw();
}

std::uint16_t lock_on_ease_yaw(
    std::uint16_t current, std::uint16_t wanted, float weight, int sign, float share_of_rest, int largest_step) {
    weight = std::clamp(weight, 0.0f, 1.0f);
    if (weight <= 0.0f) return current;
    int rest = yaw_difference(current, wanted);
    if (std::abs(rest) <= kSmallestStep) return wanted;
    if (std::abs(rest) > kHalfTurnBand && sign != 0 && (rest > 0) != (sign > 0))
        rest += sign > 0 ? 65536 : -65536; // the long way, as before
    const int way = rest > 0 ? 1 : -1;
    const float share = static_cast<float>(std::abs(rest)) * share_of_rest * weight;
    const int step = std::clamp(static_cast<int>(std::lround(share)), kSmallestStep,
        std::max(kSmallestStep, static_cast<int>(static_cast<float>(largest_step) * weight)));
    return static_cast<std::uint16_t>(current + way * std::min(step, std::abs(rest)));
}

void lock_on_tap() {
    state.tap = true;
    state.tap_age = 0u;
    state.trace_walk = trace_on();
}

bool lock_on_trigger_initiates_camera() {
    return trigger_initiates_camera();
}

bool lock_on_take_focus_press() {
    return std::exchange(state.shared_focus, false);
}

std::optional<LockOnAim> lock_on_update(const psprecomp::GuestMemory &memory, const LockOnCamera &camera) {
    ++state.updates;
    const bool focus = focus_style();
    if (state.double_tap_left > 0u) --state.double_tap_left;
    // Only the L that starts a turn. A second one inside the double-tap
    // window is claimed before this runs, and left to the game. One after
    // that window, while the turn is still moving, must not restart it.
    if (focus && camera.snap && state.locked && !state.focusing && !state.yielding) {
        state.focusing = true;
        state.focus_left = kFocusUpdates;
        state.double_tap_left = kDoubleTap;
        state.reconciled = false;
        state.turning = 0;
        state.manual = 0.0f;
        state.pitch_known = true;
        state.base_pitch = camera.pitch;
        state.pitch = camera.pitch;
    }
    const bool tapped = std::exchange(state.tap, false);
    const bool retry = !tapped && state.seeking && !state.locked;
    if (!tapped && !state.locked && !retry) return std::nullopt;
    const std::vector<LockMonster> monsters = lock_on_monsters(memory);
    if (tapped || retry) {
        const std::optional<std::uint32_t> current = state.locked ? std::optional(state.target) : std::nullopt;
        if (tapped && !state.locked) state.visited.clear();
        const std::optional<std::uint32_t> next =
            lock_on_pick(monsters, camera.look_at, camera.yaw, current, state.visited);
        if (!next) {
            if (state.locked) {
                lock_on_release("");
            } else if (tapped) {
                state.seeking = true;
                state.seek_left = kSeekUpdates;
                explain_miss(memory);
            } else if (state.seek_left > 0u) {
                --state.seek_left;
            } else {
                state.seeking = false;
            }
            return std::nullopt;
        }
        state.seeking = false;
        const bool first = !state.locked;
        lock(*next, first ? "locked onto" : "moved to");
        if (first) {
            state.pitch_known = true;
            state.base_pitch = camera.pitch;
            state.pitch = camera.pitch;
            // The same tap that selects also starts a Focus turn, when that
            // button is the focus button. Follow starts on its own.
            if (focus && trigger_initiates_camera()) {
                state.focusing = true;
                state.focus_left = kFocusUpdates;
                state.double_tap_left = kDoubleTap;
                state.reconciled = false;
                state.turning = 0;
                state.manual = 0.0f;
                state.marker_expired = false;
                state.marker_left = kMarkerLife;
            }
        }
    }
    const LockMonster *target = find(monsters, state.target);
    if (target == nullptr) {
        lock_on_release(why_target_left(memory, state.target));
        return std::nullopt;
    }
    if (horizontal(camera.look_at, target->position) > kFarthest) {
        lock_on_release("Lock-on: the monster is too far");
        return std::nullopt;
    }
    // The right stick pauses Follow and takes the reticle with it. The monster
    // stays selected, portrait included. The Focus/Follow Camera button turns
    // tracking, and the reticle, on again. Another push does not.
    if (!focus && camera.stick) {
        if (!state.follow_paused) {
            state.follow_paused = true;
            state.manual = 0.0f;
            state.turning = 0;
            std::cout << "[lockon] follow off" << std::endl;
        }
        return std::nullopt;
    }
    // Follow, with tracking turned off. The selection stays, and turning the
    // camera by hand does not let go: that is how the player looks around
    // until the Focus/Follow Camera button turns tracking on again.
    if (!focus && state.follow_paused) return std::nullopt;
    // Focus holds the selection without turning. A stick nudge then is the
    // player's camera, not a reason to drop the monster. So is the game's
    // recenter after a quick second L.
    if (focus && (!state.focusing || state.yielding)) return std::nullopt;
    // The player takes the camera back: a camera command of the game's, or
    // turning it by hand. Focus keeps the selection and only stops the turn.
    state.manual = state.manual * kManualFade + std::fabs(camera.manual_yaw);
    if (camera.command || state.manual > kManualRelease) {
        std::cout << "[lockon] the player took the camera back ("
                  << (camera.command ? "a camera command" : "turned by hand") << ")" << std::endl;
        if (focus) state.focusing = false;
        else
            lock_on_release("");
        return std::nullopt;
    }
    const Vec3 aim{target->position.x, target->position.y + kAimHeight, target->position.z};
    const std::uint16_t wanted = yaw_towards(camera.look_at, aim);
    // Follow holds the yaw while the monster is on top of the hunter, where
    // its direction jumps from one update to the next. A Focus turn is one
    // the player asked for and ends on arrival, so it turns at any distance,
    // including when the monster is already on screen.
    const float near = focus ? 1.0f : (horizontal(camera.look_at, aim) - kNear) / kNearBlend;
    const int sweep = std::abs(yaw_difference(camera.yaw, wanted)) > screen_half_yaw() ? kFocusSweep : kLargestStep;
    LockOnAim out{lock_on_ease_yaw(camera.yaw, wanted, near, state.turning, focus ? kFocusShare : kYawShare,
                     focus ? sweep : kLargestStep),
        std::nullopt};
    const bool arrived = focus && std::abs(yaw_difference(out.yaw, wanted)) <= kFocusArrival;
    if (arrived) {
        out.yaw = wanted;
        state.reconciled = true;
    }
    const int step = yaw_difference(camera.yaw, out.yaw);
    state.turning = step > 0 ? 1 : (step < 0 ? -1 : 0);
    if (focus && state.focus_left > 0u) --state.focus_left;
    if (focus && (arrived || state.focus_left == 0u)) state.focusing = false;
    if (!camera.player_pitch && state.pitch_known) {
        const float distance = std::max(horizontal(camera.look_at, aim), 1.0f);
        const float elevation = std::atan2(aim.y - camera.look_at.y, distance) * kDegrees;
        // Follow stays near the angle the lock began at. A Focus tap looks
        // toward the monster, the way MH4U's target cam adjusts for height.
        const float goal = focus ? std::clamp(-elevation * kFocusPitchFollow, kPitchLowest, kPitchHighest)
                                 : std::clamp(std::clamp(state.base_pitch - elevation * kPitchFollow,
                                                   state.base_pitch - kPitchDown, state.base_pitch + kPitchUp),
                                       kPitchLowest, kPitchHighest);
        state.pitch += (goal - state.pitch) * (focus ? kFocusShare : kPitchShare);
        if (arrived) state.pitch = goal;
        out.pitch = state.pitch;
    } else {
        state.pitch_known = false;
    }
    if (trace_on())
        std::cout << "[lockon] update " << state.updates << " target 0x" << std::hex << state.target << std::dec
                  << " at " << target->position.x << "," << target->position.y << "," << target->position.z
                  << " look-at " << camera.look_at.x << "," << camera.look_at.y << "," << camera.look_at.z << " yaw "
                  << camera.yaw << " -> " << out.yaw << " (wanted " << wanted << ") pitch " << camera.pitch << " -> "
                  << (out.pitch ? *out.pitch : camera.pitch) << " manual " << state.manual << std::endl;
    return out;
}

void lock_on_frame(const psprecomp::GuestMemory &memory) {
    // A tap while locked moves on or lets go, which needs no camera: done
    // here, so it counts even while the follow camera does not run (a
    // grab, an aim).
    // The trigger is both buttons. While locked, a Focus tap centers instead
    // of moving on, and a Follow tap while tracking is paused turns it on
    // again instead of moving on. An ordinary Follow tap still moves on.
    if (state.tap && state.locked && trigger_initiates_camera()) {
        if (focus_style()) {
            state.tap = false;
            state.shared_focus = true;
        } else if (state.follow_paused) {
            state.tap = false;
            state.follow_paused = false;
            state.manual = 0.0f;
            state.turning = 0;
            std::cout << "[lockon] follow on" << std::endl;
        }
    }
    if (state.tap && state.locked) {
        state.tap = false;
        const std::vector<LockMonster> monsters = lock_on_monsters(memory);
        const std::optional<std::uint32_t> next =
            lock_on_pick(monsters, {}, 0u, std::optional(state.target), state.visited);
        if (next)
            lock(*next, "moved to");
        else
            lock_on_release("");
    }
    if (state.tap && ++state.tap_age > kTapFrames) {
        // No follow camera took it: the village, a cutscene, aiming.
        state.tap = false;
        if (!state.locked) std::cout << "[lockon] tap with no follow camera to turn" << std::endl;
    }
    if (focus_style() && state.marker_left > 0u && --state.marker_left == 0u) state.marker_expired = true;
    if (!state.locked) return;
    const std::vector<LockMonster> monsters = lock_on_monsters(memory);
    if (find(monsters, state.target) == nullptr)
        lock_on_release(why_target_left(memory, state.target));
}

bool lock_on_wanted() {
    return state.tap || state.locked || state.seeking;
}

LockOnStatus lock_on_status() {
    return {state.locked, state.locked ? state.target : 0u};
}

bool lock_on_focus_double_tap() {
    return state.double_tap_left > 0u && !state.yielding;
}

void lock_on_accept_double_tap() {
    state.focusing = false;
    state.focus_left = 0u;
    state.double_tap_left = 0u;
    state.yielding = true;
    state.reconciled = false;
    state.turning = 0;
    state.manual = 0.0f;
}

bool lock_on_yielding_recenter() {
    return state.yielding;
}

void lock_on_finish_yield() {
    state.yielding = false;
}

void lock_on_toggle_follow() {
    if (focus_style() || !state.locked) return;
    state.follow_paused = !state.follow_paused;
    state.manual = 0.0f;
    state.turning = 0;
    std::cout << "[lockon] follow " << (state.follow_paused ? "off" : "on") << std::endl;
}

bool lock_on_drives_camera() {
    // Follow stays on the monster unless the Focus camera button turned that
    // off. Focus eases only for a turn it started itself.
    return state.locked && !state.yielding && (focus_style() ? state.focusing : !state.follow_paused);
}

std::optional<std::uint16_t> lock_on_focus_aim_yaw(const psprecomp::GuestMemory &memory, const Vec3 &look_at) {
    if (!focus_style() || !state.locked || state.yielding) return std::nullopt;
    const std::vector<LockMonster> monsters = lock_on_monsters(memory);
    const LockMonster *target = find(monsters, state.target);
    if (target == nullptr) return std::nullopt;
    const Vec3 aim{target->position.x, target->position.y + kAimHeight, target->position.z};
    return yaw_towards(look_at, aim);
}

void lock_on_focus_reticle() {
    if (!focus_style() || !state.locked) return;
    state.marker_expired = false;
    state.marker_left = kMarkerLife;
}

bool lock_on_marker_wanted() {
    if (!state.locked) return false;
    if (!focus_style() && state.follow_paused) return false;
    if (focus_style() && state.marker_expired) return false;
    return true;
}

void lock_on_note_view(const LockOnView &view, std::uint32_t vertices) {
    if (vertices == 0u || std::fabs(view.projection[11]) < 1.0e-6f) return;
    if (vertices <= state.view_vertices) return;
    state.view = view;
    state.view_vertices = vertices;
}

namespace {
// The marker used to read Portable 3rd's camera object at 0x08A2F958 (view
// +0xF50, projection +0xFD0). ULUS-10391 stores the object at 0x089CBD04.
// 0x088144B4 writes the projection at +0xC00 (x scale at +0, y scale at +0x14,
// m[11] = -1 at +0x2C). 0x08814268 then writes the view, with its translation,
// at +0xC80. The view a draw uploads has no translation, so a world position
// projected through it misses the picture.
constexpr std::uint32_t kCameraPointer = 0x089CBD04u;
constexpr std::uint32_t kProjection = 0xC00u;
constexpr std::uint32_t kViewMatrix = 0xC80u;

bool marker_instructions(const psprecomp::GuestMemory &memory) {
    const auto word = [&](std::uint32_t address, std::uint32_t expect) {
        return memory.raw_pointer(address, 4u) != nullptr && memory.load32(address) == expect;
    };
    return word(0x08812954u, 0xAC50BD04u) && // sw s0,-0x42fc(v0): the pointer at 0x089CBD04
           word(0x0881439Cu, 0x26640C80u) && // addiu a0,s3,0xC80: the view
           word(0x088145D8u, 0x26050C00u);   // addiu a1,s0,0xC00: the projection
}

void load_matrix(const psprecomp::GuestMemory &memory, std::uint32_t address, std::array<float, 16> &out) {
    for (std::uint32_t i = 0; i < 16u; ++i) out[i] = load_float(memory, address + i * 4u);
}

// A view from the follow camera's eye (+0) to its look-at point (+0x10).
// Column major, as the GE holds matrices. Nothing when the two points
// coincide or the camera looks straight up or down.
std::optional<std::array<float, 16>> view_from_pose(const CameraPose &pose) {
    float forward[3] = {pose.look_at[0] - pose.eye[0], pose.look_at[1] - pose.eye[1], pose.look_at[2] - pose.eye[2]};
    const float length = std::sqrt(forward[0] * forward[0] + forward[1] * forward[1] + forward[2] * forward[2]);
    if (!std::isfinite(length) || length < 1.0e-4f) return std::nullopt;
    for (float &f : forward) f /= length;
    float right[3] = {-forward[2], 0.0f, forward[0]};
    const float level = std::sqrt(right[0] * right[0] + right[2] * right[2]);
    if (level < 1.0e-4f) return std::nullopt;
    right[0] /= level;
    right[2] /= level;
    const float back[3] = {-forward[0], -forward[1], -forward[2]};
    const float up[3] = {back[1] * right[2] - back[2] * right[1], back[2] * right[0] - back[0] * right[2],
        back[0] * right[1] - back[1] * right[0]};
    std::array<float, 16> view{};
    for (std::size_t axis = 0; axis < 3u; ++axis) {
        view[axis * 4u + 0u] = right[axis];
        view[axis * 4u + 1u] = up[axis];
        view[axis * 4u + 2u] = back[axis];
    }
    view[12] = -(right[0] * pose.eye[0] + right[1] * pose.eye[1] + right[2] * pose.eye[2]);
    view[13] = -(up[0] * pose.eye[0] + up[1] * pose.eye[1] + up[2] * pose.eye[2]);
    view[14] = -(back[0] * pose.eye[0] + back[1] * pose.eye[1] + back[2] * pose.eye[2]);
    view[15] = 1.0f;
    return view;
}

// The world draw's projection and viewport, with a view built from the
// follow camera. The view that draw uploaded has no translation, so the
// monster's world position misses the picture through it.
std::optional<LockOnView> posed_view(const psprecomp::GuestMemory &memory, const LockOnView &noted) {
    if (noted.x_scale == 0.0f || noted.y_scale == 0.0f) return std::nullopt;
    const std::optional<CameraPose> pose = game_camera_pose(memory);
    if (!pose) return std::nullopt;
    const std::optional<std::array<float, 16>> view = view_from_pose(*pose);
    if (!view) return std::nullopt;
    LockOnView out = noted;
    out.view = *view;
    return out;
}

// The camera object's view and projection, mapped by the viewport of the
// world draw when one was noted, otherwise by the full 480×272 picture.
std::optional<LockOnView> camera_object_view(const psprecomp::GuestMemory &memory, const LockOnView &noted) {
    if (!marker_instructions(memory) || memory.raw_pointer(kCameraPointer, 4u) == nullptr) return std::nullopt;
    const std::uint32_t camera = memory.load32(kCameraPointer);
    if (camera < 0x08800000u ||
        memory.raw_pointer(camera + kProjection, kViewMatrix - kProjection + 64u) == nullptr)
        return std::nullopt;
    LockOnView view;
    load_matrix(memory, camera + kViewMatrix, view.view);
    load_matrix(memory, camera + kProjection, view.projection);
    if (std::fabs(view.projection[11]) < 1.0e-6f) return std::nullopt;
    if (noted.x_scale != 0.0f && noted.y_scale != 0.0f) {
        view.x_scale = noted.x_scale;
        view.y_scale = noted.y_scale;
        view.x_offset = noted.x_offset;
        view.y_offset = noted.y_offset;
        view.origin_x = noted.origin_x;
        view.origin_y = noted.origin_y;
    } else {
        view.x_scale = 240.0f;
        view.x_offset = 240.0f;
        view.y_scale = -136.0f;
        view.y_offset = 136.0f;
    }
    return view;
}
} // namespace

std::optional<std::array<float, 2>> lock_on_project(const LockOnView &view, const Vec3 &world) {
    if (view.x_scale == 0.0f || view.y_scale == 0.0f) return std::nullopt;
    const float point[4] = {world.x, world.y, world.z, 1.0f};
    float eye[4]{};
    float clip[4]{};
    for (int row = 0; row < 4; ++row)
        for (int col = 0; col < 4; ++col)
            eye[row] += view.view[static_cast<std::size_t>(col * 4 + row)] * point[col];
    for (int row = 0; row < 4; ++row)
        for (int col = 0; col < 4; ++col)
            clip[row] += view.projection[static_cast<std::size_t>(col * 4 + row)] * eye[col];
    if (!std::isfinite(clip[0]) || !std::isfinite(clip[1]) || !std::isfinite(clip[3]) || clip[3] <= 1.0e-4f)
        return std::nullopt;
    const float ndc_x = clip[0] / clip[3];
    const float ndc_y = clip[1] / clip[3];
    // The GE viewport: screen = ndc * scale + offset - origin, in PSP pixels.
    // A negative y scale already points device y down the screen.
    const float pixel_x = ndc_x * view.x_scale + view.x_offset - view.origin_x;
    const float pixel_y = ndc_y * view.y_scale + view.y_offset - view.origin_y;
    const float picture_x = pixel_x / 480.0f;
    const float picture_y = pixel_y / 272.0f;
    if (!std::isfinite(picture_x) || !std::isfinite(picture_y) || picture_x < 0.0f || picture_x > 1.0f ||
        picture_y < 0.0f || picture_y > 1.0f)
        return std::nullopt;
    return std::array<float, 2>{picture_x, picture_y};
}

std::optional<std::array<float, 2>> lock_on_marker(const psprecomp::GuestMemory &memory) {
    const LockOnView noted = state.view;
    const std::uint32_t vertices = std::exchange(state.view_vertices, 0u);
    state.view = {};
    if (!lock_on_marker_wanted() || memory.raw_pointer(state.target, kExtent) == nullptr) return std::nullopt;
    const Vec3 feet = load_vec(memory, state.target + kPosition);
    if (!finite(feet)) return std::nullopt;
    const Vec3 aim{feet.x, feet.y + kAimHeight, feet.z};
    // The follow camera's eye and look-at. When that pose is known, it is the
    // only view: a monster it cannot see stays unmarked. The view a draw
    // uploaded has no translation, so a point near the world origin can land
    // on the picture even though the camera is looking somewhere else.
    if (const std::optional<LockOnView> posed = posed_view(memory, noted)) return lock_on_project(*posed, aim);
    if (vertices != 0u)
        if (const std::optional<std::array<float, 2>> at = lock_on_project(noted, aim)) return at;
    if (const std::optional<LockOnView> tracked = camera_object_view(memory, noted))
        return lock_on_project(*tracked, aim);
    return std::nullopt;
}

void lock_on_release(const char *why) {
    state.seeking = false;
    if (state.locked) {
        std::cout << "[lockon] let go of the monster at 0x" << std::hex << state.target << std::dec;
        if (why != nullptr && *why != '\0') std::cout << ": " << why;
        std::cout << std::endl;
    }
    state.locked = false;
    state.follow_paused = false;
    state.focusing = false;
    state.focus_left = 0u;
    state.double_tap_left = 0u;
    state.yielding = false;
    state.reconciled = false;
    state.marker_left = 0u;
    state.marker_expired = false;
    state.target = 0u;
    state.manual = 0.0f;
    state.turning = 0;
    state.pitch_known = false;
}

void lock_on_reset() {
    state = State{};
}

namespace {

// Species id, then the icon's top-left texel. The coordinate pair is the one
// the icon draw stores in a vertex: the low 16 bits are u and the high 16
// bits are v, and the opposite corner is 36 texels on from both. Subspecies
// that share a picture share a cell.
struct IconCell {
    std::uint8_t id;
    std::uint16_t u;
    std::uint16_t v;
};

constexpr IconCell kIconCells[] = {
    {0x01, 0x00B4, 0x0024}, // Rathian
    {0x03, 0x00D8, 0x0000}, // Kelbi
    {0x04, 0x0024, 0x00D8}, // Mosswine
    {0x05, 0x0048, 0x006C}, // Bullfango
    {0x06, 0x0000, 0x0024}, // Yian Kut-Ku
    {0x07, 0x0000, 0x006C}, // Lao-Shan Lung
    {0x08, 0x0048, 0x0024}, // Cephadrome
    {0x09, 0x00D8, 0x00B4}, // Felyne
    {0x0B, 0x0090, 0x0024}, // Rathalos
    {0x0C, 0x00D8, 0x0090}, // Aptonoth
    {0x0D, 0x0024, 0x0000}, // Genprey
    {0x0E, 0x0090, 0x0048}, // Diablos
    {0x0F, 0x0000, 0x0048}, // Khezu
    {0x10, 0x0000, 0x0000}, // Velociprey
    {0x11, 0x0024, 0x0048}, // Gravios
    {0x13, 0x00D8, 0x0024}, // Vespoid
    {0x14, 0x0024, 0x0024}, // Gypceros
    {0x15, 0x006C, 0x0024}, // Plesioth
    {0x16, 0x0048, 0x0048}, // Basarios
    {0x17, 0x0000, 0x00D8}, // Melynx
    {0x18, 0x00D8, 0x0048}, // Hornetaur
    {0x19, 0x00D8, 0x006C}, // Apceros
    {0x1A, 0x006C, 0x0048}, // Monoblos
    {0x1B, 0x006C, 0x0000}, // Velocidrome
    {0x1C, 0x0090, 0x0000}, // Gendrome
    {0x1E, 0x0048, 0x0000}, // Ioprey
    {0x1F, 0x00B4, 0x0000}, // Iodrome
    {0x21, 0x00B4, 0x0048}, // Kirin
    {0x22, 0x0048, 0x0024}, // Cephalos
    {0x23, 0x0048, 0x00D8}, // Giaprey
    {0x25, 0x00B4, 0x0024}, // Pink Rathian
    {0x26, 0x0000, 0x0024}, // Blue Yian Kut-Ku
    {0x27, 0x0024, 0x0024}, // Purple Gypceros
    {0x28, 0x0024, 0x006C}, // One-Eared Yian Garuga
    {0x29, 0x0090, 0x0024}, // Silver Rathalos
    {0x2A, 0x00B4, 0x0024}, // Gold Rathian
    {0x2B, 0x0090, 0x0048}, // Black Diablos
    {0x2C, 0x006C, 0x0048}, // White Monoblos
    {0x2D, 0x0000, 0x0048}, // Red Khezu
    {0x2E, 0x006C, 0x0024}, // Green Plesioth
    {0x2F, 0x0024, 0x0048}, // Black Gravios
    {0x30, 0x0000, 0x0090}, // Daimyo Hermitaur
    {0x31, 0x0090, 0x0024}, // Azure Rathalos
    {0x32, 0x0000, 0x006C}, // Ashen Lao-Shan Lung
    {0x33, 0x006C, 0x006C}, // Blangonga
    {0x34, 0x0048, 0x00B4}, // Congalala
    {0x35, 0x00B4, 0x006C}, // Rajang
    {0x36, 0x0048, 0x0090}, // Kushala Daora
    {0x37, 0x0090, 0x00FC}, // Shen Gaoren
    {0x38, 0x0090, 0x006C}, // Great Thunderbug
    {0x39, 0x006C, 0x00FC}, // Shakalaka
    {0x3B, 0x006C, 0x0090}, // Chameleos
    {0x3C, 0x0048, 0x0090}, // Rusted Kushala Daora
    {0x3D, 0x0090, 0x00B4}, // Blango
    {0x3E, 0x00B4, 0x00B4}, // Conga
    {0x3F, 0x0090, 0x00D8}, // Remobra
    {0x40, 0x0090, 0x0090}, // Lunastra
    {0x41, 0x00B4, 0x0090}, // Teostra
    {0x42, 0x0000, 0x0090}, // Hermitaur
    {0x43, 0x0024, 0x0090}, // Shogun Ceanataur
    {0x44, 0x0048, 0x006C}, // Bulldrome
    {0x45, 0x00B4, 0x00D8}, // Anteka
    {0x46, 0x00D8, 0x00D8}, // Popo
    {0x48, 0x0048, 0x00FC}, // Yama Tsukami
    {0x49, 0x0024, 0x0090}, // Ceanataur
    {0x4B, 0x0000, 0x00B4}, // Tigrex
    {0x4D, 0x006C, 0x00D8}, // Giadrome
    {0x4E, 0x0024, 0x006C}, // Yian Garuga
    {0x50, 0x00D8, 0x0024}, // Vespoid Queen
    {0x51, 0x00B4, 0x00FC}, // Nargacuga
    {0x52, 0x0024, 0x00FC}, // Hypnocatrice
    {0x53, 0x0000, 0x00FC}, // Lavasioth
    {0x54, 0x006C, 0x006C}, // Copper Blangonga
    {0x55, 0x0048, 0x00B4}, // Emerald Congalala
    {0x56, 0x0000, 0x0090}, // Plum Daimyo Hermitaur
    {0x57, 0x0024, 0x0090}, // Terra Shogun Ceanataur
    {0x59, 0x00B4, 0x006C}, // Furious Rajang
};

} // namespace

std::optional<HuntIconCell> hunt_icon_cell(std::uint8_t id) {
    for (const IconCell &cell : kIconCells)
        if (cell.id == id) return HuntIconCell{cell.u, cell.v};
    return std::nullopt;
}

} // namespace mhfu::camera
