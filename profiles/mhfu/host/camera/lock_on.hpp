#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace psprecomp {
class GuestMemory;
}

// Lock-on (#163): a tap of the Lock on bind turns the quest camera towards a
// large monster and keeps it there while the monster moves.
//
// It is not a camera of its own. The follow camera's driver (game_camera.hpp)
// asks it, on every update of the game's ordinary follow camera, which yaw
// and pitch the camera should move towards, and moves the game's own angles
// there the way the analog camera does. The game keeps everything else: its
// collision checks, its presets for every other mode, and the hunter's
// movement, which stays relative to the camera as it always is.
//
// ULUS-10391, read from this executable (see the profile README):
// - While game_task.ovl is loaded at 0x09A5F300, a manager pointer sits at
//   0x09D19860. Its list of quest entities starts at manager+0x19030. Each
//   node points at the next-older node at +4, at the entity at +12, and
//   keeps a type byte at +36 (1 is a player).
// - The entity's position is three floats at +0x200. Its health and most
//   health are s16 at +0x2E4 and +0x41E. A companion's most health is 0.
// - A large monster's vtable is at +0. The word at vtable+8 is a method in
//   the em overlay slot (0x09D1A280..0x09D63100). Small monsters' methods
//   stay in game_task, so they are left out.
// - The camera looks from its eye at +0x0 of the camera structure to its
//   look-at point at +0x10. Its yaw, in 65536ths of a turn, puts the eye in
//   the direction (sin yaw, cos yaw) from the look-at point, so the camera
//   looks along -(sin yaw, cos yaw).
namespace mhfu::camera {

struct Vec3 {
    float x{};
    float y{};
    float z{};
};

// What lock-on reads in the game. Exposed for tests, which build a stand-in.
namespace lock_on_layout {
inline constexpr std::uint32_t kTaskSlot = 0x09A5F300u; // game_task.ovl while a quest runs
// The word at this address is the entity manager. Its list head is the
// newest node; +4 of a node is the next older one, +12 the entity, +36 a
// type byte. Type 1 is a player. The insert is game_sub 0x09C57CA0.
inline constexpr std::uint32_t kEntityList = 0x09D19860u;
inline constexpr std::uint32_t kListHead = 0x19030u;
inline constexpr std::uint32_t kNodeOlder = 0x4u;
inline constexpr std::uint32_t kNodeEntity = 0xCu;
inline constexpr std::uint32_t kNodeType = 0x24u;
inline constexpr std::uint8_t kPlayerNode = 1u;
inline constexpr std::uint32_t kMaxMonsters = 64u; // a bound on a corrupt or cyclic list
// Object +0 is the vtable. Its word at +8 is a method. For a large monster
// that method lies in the em overlay slot; a small monster's does not.
inline constexpr std::uint32_t kVtable = 0x0u;
inline constexpr std::uint32_t kVtableMethod = 0x8u;
inline constexpr std::uint32_t kEmOverlay = 0x09D1A280u;
inline constexpr std::uint32_t kEmOverlayEnd = 0x09D63100u;
inline constexpr std::uint32_t kPosition = 0x200u;
inline constexpr std::uint32_t kHealth = 0x2E4u;
inline constexpr std::uint32_t kMostHealth = 0x41Eu;
inline constexpr std::uint32_t kExtent = 0x420u;
} // namespace lock_on_layout

struct LockMonster {
    std::uint32_t address{};
    std::uint32_t slot{};
    Vec3 position;
};

// The large monsters that can be locked onto: on the quest entity list while
// a quest runs, not a player, with a health pool, and with a vtable method
// in the large-monster overlay.
[[nodiscard]] std::vector<LockMonster> lock_on_monsters(const psprecomp::GuestMemory &memory);

// The camera's yaw, in its units, that looks from `from` towards `to`.
[[nodiscard]] std::uint16_t yaw_towards(const Vec3 &from, const Vec3 &to);
// `to` minus `from`, the short way round: -32768..32767.
[[nodiscard]] int yaw_difference(std::uint16_t from, std::uint16_t to);

// What a tap does, given the monsters there are now, the camera (its look-at
// point and yaw), the target locked now, if any, and the ones this lock has
// already gone through. Returns the monster to lock onto, or nothing to let
// go. With nothing locked: the nearest monster within kInView of where the
// camera looks, else the nearest one. Locked: the next monster in the list
// this lock has not been on yet; after the last, nothing.
inline constexpr float kInView = 60.0f; // degrees either side of where the camera looks
[[nodiscard]] std::optional<std::uint32_t> lock_on_pick(const std::vector<LockMonster> &monsters, const Vec3 &look_at,
    std::uint16_t camera_yaw, std::optional<std::uint32_t> current, const std::vector<std::uint32_t> &visited);

// One step of the camera's yaw towards `wanted`: a share of what is left,
// neither too small to finish nor so large that the turn jumps. `weight`
// (0..1) scales the step, down to none; `sign` (+1, -1, 0 for none) is the
// way the previous step went, kept while the rest is near half a turn, where
// the short way round flips from one update to the next.
[[nodiscard]] std::uint16_t lock_on_ease_yaw(
    std::uint16_t current, std::uint16_t wanted, float weight = 1.0f, int sign = 0);

// --- The running lock -------------------------------------------------------

// A tap of the bind, from the flip.
void lock_on_tap();

// What the follow camera's driver hands over on each update, and what it
// gets back: the yaw to put the camera at, and the pitch (degrees, the eye
// above the look-at point) when lock-on tilts it too.
struct LockOnCamera {
    std::uint32_t address{}; // the camera structure (s1)
    std::uint16_t yaw{};     // the camera's yaw now
    Vec3 look_at;
    // The player turned the camera by hand this update, in degrees, or gave
    // the game a camera command (the D-pad's turn or tilt, recentring).
    float manual_yaw{};
    bool command{};
    // The camera's pitch now, from the game's eye offset, in degrees.
    float pitch{};
    bool player_pitch{}; // the player has tilted the camera by hand
};
struct LockOnAim {
    std::uint16_t yaw{};
    std::optional<float> pitch;
};
// Nothing when there is no lock, or it has just been let go.
[[nodiscard]] std::optional<LockOnAim> lock_on_update(const psprecomp::GuestMemory &memory, const LockOnCamera &camera);

// While the free camera flies (camera/free_camera.hpp) a lock is held but
// does not turn the game's camera, and taps wait.
void lock_on_suspend(bool suspended);

// Once a flip: lets a lock go whose monster is no longer there (killed,
// captured, gone to another area, the quest over), and a tap that no camera
// update took (the village, a cutscene).
void lock_on_frame(const psprecomp::GuestMemory &memory);

// A tap is waiting for a camera update, or a lock is on: the camera driver
// must be in place for either.
[[nodiscard]] bool lock_on_wanted();

struct LockOnStatus {
    bool locked{};
    std::uint32_t target{};
};
[[nodiscard]] LockOnStatus lock_on_status();

// The locked monster's place on the game's picture, each coordinate 0..1
// from the top left, from the game's own view and projection; nothing when
// there is no lock or the monster is behind the camera. For the marker.
[[nodiscard]] std::optional<std::array<float, 2>> lock_on_marker(const psprecomp::GuestMemory &memory);

// Lets go, for tests and when the setting goes off. `why` goes to the
// trace (MHFU_TRACE_LOCK_ON) and, when not empty, to a note on screen.
void lock_on_release(const char *why);
void lock_on_reset();

// Why the last tap did nothing, once, for a note on screen: "" when there is
// none to show.
[[nodiscard]] std::string lock_on_take_note();

} // namespace mhfu::camera
