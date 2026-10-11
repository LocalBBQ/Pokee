#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace psprecomp {
class GuestMemory;
}

// Lock-on (#163): a tap of the Lock on bind selects a large monster. Follow
// keeps the quest camera on it. Focus leaves the camera where it is until L
// is tapped, then turns it toward the selection and stops.
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
// - When an entity's vtable has a method in the em overlay slot, or its list
//   node carries class id 32, 34 or 36, only those entities are locked.
//   A quest whose monsters keep a game_task vtable (seen with em75 loaded)
//   still locks: any other non-player with a health pool is then eligible.
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
// game_sub 0x09C899E0 stores this halfword. em overlays pass 32, 34 or 36.
inline constexpr std::uint32_t kNodeClass = 58u;
inline constexpr std::uint32_t kMaxMonsters = 64u; // a bound on a corrupt or cyclic list
// Object +0 is the vtable. A large monster's methods in the first slots lie
// in the em overlay. The word at +8 is the one those overlays replace.
inline constexpr std::uint32_t kVtable = 0x0u;
inline constexpr std::uint32_t kVtableMethod = 0x8u;
inline constexpr std::uint32_t kVtableSpan = 64u;
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

// What a tap can lock onto: on the quest entity list while a quest runs, not
// a player, and alive with a health pool. When any entry is marked large
// (an em-overlay method, or class id 32, 34 or 36), only those.
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
// the short way round flips from one update to the next. `share` is the part
// of the rest taken each update.
[[nodiscard]] std::uint16_t lock_on_ease_yaw(
    std::uint16_t current, std::uint16_t wanted, float weight = 1.0f, int sign = 0, float share = 0.2f);

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
    // L's recenter, taken for a Focus turn instead of the game's placement
    // behind the hunter. Follow leaves it as a command.
    bool snap{};
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

// The camera driver should keep the second stick and write the yaw: Follow
// while a monster is selected, Focus only during the turn L started.
[[nodiscard]] bool lock_on_drives_camera();
// The reticle is shown: Follow while a monster is selected, Focus only while
// L's turn is on its way, so it goes once the camera has arrived.
[[nodiscard]] bool lock_on_marker_wanted();

// The view a transformed draw was made with, and the GE viewport that maps
// its normalised device coordinates onto the 480×272 screen. Column major,
// as the GE holds the matrices.
struct LockOnView {
    std::array<float, 16> view{};
    std::array<float, 16> projection{};
    float x_scale{};
    float y_scale{};
    float x_offset{};
    float y_offset{};
    float origin_x{};
    float origin_y{};
};

// One transformed draw. An orthographic draw is ignored. The perspective
// draw with the most vertices supplies the projection and viewport the marker
// maps through. The view that draw uploaded has no translation, so the marker
// replaces it with one built from the follow camera's eye and look-at.
void lock_on_note_view(const LockOnView &view, std::uint32_t vertices);

// Where a world point lands on the picture, each coordinate 0..1 from the
// top left. Nothing when it is behind the camera or off the picture.
[[nodiscard]] std::optional<std::array<float, 2>> lock_on_project(const LockOnView &view, const Vec3 &world);

// The locked monster's place on the picture, a little above its feet.
// The view a draw uploads has no translation, so the monster is placed with
// a view built from the follow camera's eye (+0) and look-at (+0x10), through
// the projection and viewport of the world draw. Nothing when there is no
// lock, that camera has not updated, or the monster is behind the camera or
// off the picture. Takes the viewport noted since the last call, so a frame
// asks once.
[[nodiscard]] std::optional<std::array<float, 2>> lock_on_marker(const psprecomp::GuestMemory &memory);

// Lets go, for tests and when the setting goes off. `why` goes to the
// trace (MHFU_TRACE_LOCK_ON) and, when not empty, to a note on screen.
void lock_on_release(const char *why);
void lock_on_reset();

// Why the last tap did nothing, once, for a note on screen: "" when there is
// none to show.
[[nodiscard]] std::string lock_on_take_note();

} // namespace mhfu::camera
