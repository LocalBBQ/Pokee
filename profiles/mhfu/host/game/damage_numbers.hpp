#pragma once

// Damage numbers drawn by the port over the game. Off, nothing here runs and
// the game is unchanged. On, a number is the health a hit removed.
//
// The hit is game_task.ovl at 0x09AC7A08 (ULUS-10391). The monster is a0 and
// its current health is the signed halfword at +0x2E4. That routine subtracts
// one halfword and adds another, then clamps to the maximum at +0x41E. The
// status tick at 0x09AD2698 writes the same health field on its own countdown
// and is not a hit.
//
// Colour follows the hitzone rule used by the later games: orange when the
// effective hitzone is 45 or more, gray otherwise. A blade multiplies the
// hitzone by its sharpness; a gun uses the hitzone alone. Element is already
// inside the one health drop, so it does not change the colour. A critical
// hit is a separate mark. Until the part's hitzone is known the number is
// white, because the size of the drop is not a hitzone.
//
// The number is drawn on the monster. Its position is the three floats at
// +0x200, the same field lock-on reads, a little above the feet. The frame's
// 3D view and projection, the ones the world was drawn with, put that point
// on the picture. A hit with no place on screen is not drawn. The hit routine
// does not carry the point of contact, so this body point is the nearest
// place known; the number stays on it and does not step aside. Several hits
// stack upward from it. The closer the camera, the taller the monster on the
// picture and the smaller the number is drawn, so it keeps to that point
// rather than spreading over the body.
//
// In ad hoc play the hit routine runs only on the console that applies the
// hit: for a large monster the one in charge of it, for a small monster the
// attacker's. Every other console receives the new health from the network
// and writes it without that routine. So each frame the health of every
// monster on the quest entity list is read too, and a drop that the routine
// did not make is shown as well: every player sees every hit. A call of the
// status tick, when it is reached through the same watch, moves the health
// the frame compares against, so a poison tick on this console stays hidden;
// one received from another console cannot be told from a hit.
// MHFU_DAMAGE_NUMBERS_HEALTH_WATCH=0 turns that reading off, for comparing.

#include "game/guest_layout.hpp"
#include "psprecomp/guest_memory.hpp"

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace mhfu::game {

inline constexpr std::uint32_t kHitAdjust = 0x09AC7A08u;
inline constexpr std::uint32_t kStatusTick = 0x09AD2698u;
// An entity's health and world position (game/guest_layout.hpp).
inline constexpr std::uint32_t kMonsterHealth = layout::kHealth;
inline constexpr std::uint32_t kMonsterPosition = layout::kPosition;
// Above the feet, where a number sits on the body.
inline constexpr float kBodyHeight = 100.0f;
inline constexpr int kWeakPointHitzone = 45;
inline constexpr float kNumberSeconds = 1.60f;
inline constexpr std::size_t kMaxOnScreen = 8u;
// How far a number rises over its life, and how far each later hit sits
// above the one before, as fractions of the picture's height.
inline constexpr float kRise = 0.025f;
inline constexpr float kStack = 0.018f;
// The number shrinks as the monster grows on the picture: full size while
// kBodyHeight covers up to kShrinkFrom of the picture's height, down to
// kMinScale when it covers kShrinkTo or more.
inline constexpr float kShrinkFrom = 0.30f;
inline constexpr float kShrinkTo = 0.70f;
inline constexpr float kMinScale = 0.55f;

enum class Sharpness { Red, Orange, Yellow, Green, Blue, White, Purple };
enum class Fill { Unknown, WeakPoint, Ordinary };
enum class Affinity { None, Positive, Negative };

struct Grade {
    Fill fill{Fill::Unknown};
    Affinity affinity{Affinity::None};
};

struct Ink {
    std::uint8_t red{255};
    std::uint8_t green{255};
    std::uint8_t blue{255};
};

[[nodiscard]] float sharpness_modifier(Sharpness sharpness) noexcept;
// `hitzone` is the raw part value. `gunner` skips the sharpness multiplier.
[[nodiscard]] Grade grade_hit(int hitzone, Sharpness sharpness, bool gunner, Affinity affinity) noexcept;
[[nodiscard]] Ink damage_number_ink(Fill fill) noexcept;

// How much current health fell. Nothing for a heal, a miss, or a monster that
// was already at zero.
[[nodiscard]] std::optional<int> damage_shown(int health_before, int health_after) noexcept;

struct Number {
    int amount{};
    Grade grade{};
    float age{};
    int lane{};
    std::uint32_t object{}; // the monster the hit landed on
    float x{};              // its place on the picture, once known
    float y{};
    float body{};           // kBodyHeight on the picture, 0..1 of its height
    bool placed{};
    std::uint8_t reports{}; // MHFU_TRACE_HITS lines about its placing so far
};

// The view and projection a transformed draw was made with, and the GE
// viewport that maps its normalised device coordinates onto the 480×272 screen.
struct WorldCamera {
    std::array<float, 16> view{};
    std::array<float, 16> projection{};
    float x_scale{};
    float y_scale{};
    float x_offset{};
    float y_offset{};
    float origin_x{};
    float origin_y{};
};

struct WorldPoint {
    float x{};
    float y{};
    float z{};
};

// The camera's eye and the point it looks at, in world space, from the game's
// camera structure (camera/game_camera.hpp). This game draws the world through
// a GE view without translation, so the view the numbers project with is built
// from these whenever they are known.
struct EyePose {
    WorldPoint eye;
    WorldPoint look_at;
};
// A view looking from `eye` at `target`, y up, column major like the GE's:
// rows right, up and back, right being forward x up. Nothing when the two
// points coincide or the camera looks straight up or down.
[[nodiscard]] std::optional<std::array<float, 16>> look_at_view(const WorldPoint &eye, const WorldPoint &target) noexcept;

struct PicturePoint {
    float x{}; // 0..1 across the game picture
    float y{}; // 0..1 down it
};

struct Anchor {
    float x{}; // 0..1 across the game picture, from the left
    float y{}; // 0..1 down the game picture, from the top
    float alpha{};
    float scale{1.0f}; // of the number's full size
};

// Where a number's text is drawn again to give it weight, in pixels from its
// place: the centre, and `rings` rings out to `radius`, each of 12 points so
// no gap shows between the copies. Only the centre for a radius under a
// quarter pixel.
struct Offset {
    float x{};
    float y{};
};
[[nodiscard]] std::vector<Offset> stroke_offsets(float radius, int rings);
// The thickness setting (1..settings::kMaxDamageNumberThickness) as pixels,
// for a number drawn `size` pixels tall where `unit_size` is 1 pixel of
// stroke: how far the dark outline reaches past the digits, and how far the
// digits themselves are thickened. Thickness 1 is a one-pixel outline and
// the font's own strokes.
struct Stroke {
    float outline{};
    float weight{};
};
[[nodiscard]] Stroke number_stroke(std::uint32_t thickness, float size, float unit_size) noexcept;
// Screen pixels between the centres of two stacked hits. `tuned_px` is the gap
// at the default size and thickness (kStack of the picture, already scaled for
// how close the camera is). With `clear` false, that gap is returned as it is.
// With `clear` true (a larger, smaller or thicker number), the gap is at least
// the glyph plus the stroke past it on both sides, and it still scales with
// the size.
[[nodiscard]] float stack_gap_px(
    float tuned_px, float size_scale, float glyph_px, float stroke_px, bool clear) noexcept;

[[nodiscard]] Number make_number(int amount, Grade grade, int lane) noexcept;
[[nodiscard]] std::string damage_number_text(const Number &number);
[[nodiscard]] std::vector<Number> age_numbers(std::vector<Number> numbers, float seconds);
[[nodiscard]] Anchor number_anchor(const Number &number) noexcept;
// The size of a number for a monster whose kBodyHeight covers `body` of the
// picture's height: 1 when it is far, down to kMinScale when it is close.
[[nodiscard]] float closeness_scale(float body) noexcept;

// Where a world point lands on the picture. Nothing when it is behind the
// camera or off the picture.
[[nodiscard]] std::optional<PicturePoint> project_world(const WorldCamera &camera, float x, float y, float z) noexcept;
// The same, for a point that may lie off the picture: only one behind the
// camera has no place. For measuring a monster that fills the view.
[[nodiscard]] std::optional<PicturePoint> project_point(const WorldCamera &camera, float x, float y, float z) noexcept;
// The monster's position, when those three floats are mapped.
[[nodiscard]] std::optional<WorldPoint> monster_position(const psprecomp::GuestMemory &memory, std::uint32_t object);

// The renderer reports each transformed draw. Draws with the same view and
// projection are counted together, and the view with the most vertices is
// the world camera for the numbers. A view without translation is centred on
// the camera (the sky, a background), which on Akantor's map drew more than
// anything else and put every monster off the picture, so such a view is
// chosen only when there is no other. An orthographic draw (the HUD) is
// ignored. `take_world_camera` returns the choice and clears the frame's
// views, so the next frame starts again.
[[nodiscard]] bool camera_centred(const WorldCamera &camera) noexcept;
void note_world_camera(const WorldCamera &camera, std::uint32_t vertices);
[[nodiscard]] std::optional<WorldCamera> take_world_camera();
// True while damage numbers are on, so a draw can skip the note when they are off.
[[nodiscard]] bool damage_numbers_watching() noexcept;
void remember_number(std::vector<Number> &live, Number number);

struct OpenHit {
    std::uint32_t depth{};
    std::uint32_t object{};
    int health{};
};

void watch_hit_enter(std::vector<OpenHit> &open, std::uint32_t pc, std::uint32_t depth, std::uint32_t object,
    int health);
[[nodiscard]] std::optional<std::uint32_t> open_object(
    const std::vector<OpenHit> &open, std::uint32_t pc, std::uint32_t depth);
// `lane` is how many numbers are already showing, so a second hit steps aside.
[[nodiscard]] std::optional<Number> watch_hit_leave(
    std::vector<OpenHit> &open, std::uint32_t pc, std::uint32_t depth, int health, int lane);

// The signed health at object+kMonsterHealth, when that halfword is mapped.
[[nodiscard]] std::optional<int> monster_health(const psprecomp::GuestMemory &memory, std::uint32_t object);

// A monster's health as last known, and its most health, which tells a new
// monster in the same place in memory from the old one.
inline constexpr std::uint32_t kMonsterMostHealth = layout::kMostHealth;
struct HealthSeen {
    std::uint32_t object{};
    int health{};
    int most{};
    std::uint32_t vtable{}; // for MHFU_TRACE_HITS
    std::uint8_t kind{};    // the list node's type byte
};

// MHFU_DAMAGE_NUMBERS_HEALTH_WATCH: unset or anything but "0" is on.
[[nodiscard]] bool parse_health_watch(const char *text) noexcept;
[[nodiscard]] bool health_watch_enabled() noexcept;

// Every entity on the quest entity list that is not a hunter and has a health
// pool, small monsters included, with its health now. Empty outside a quest.
// A hunter is a player node, or an entity with a player node's vtable: the
// other players' hunters, which are on the list too, and whose health falls
// when the monster hits them.
[[nodiscard]] std::vector<HealthSeen> quest_health(const psprecomp::GuestMemory &memory);
// The same rule for one entity. The hit routine also takes health off a
// hunter a monster hits, and that drop is not shown either.
[[nodiscard]] bool is_hunter(const psprecomp::GuestMemory &memory, std::uint32_t object);
// A watched call took `object`'s health from `before` to `after`: the health
// the next frame compares against moves by that much, so the drop is not
// shown twice. Only by that much: the monster's own update calls the routine
// every frame, and a drop written from the network just before it must still
// show. A monster not seen yet is left for the frame to add.
void note_change(std::vector<HealthSeen> &seen, std::uint32_t object, int before, int after);
// One frame: a fall in health since `seen`, on a monster with the same most
// health, is a hit applied elsewhere and becomes a number, the first in
// `lane` and each next one a lane above. A monster seen for the first time,
// healed, or with another most health shows nothing. `seen` becomes `now`.
[[nodiscard]] std::vector<Number> watch_health(
    std::vector<HealthSeen> &seen, const std::vector<HealthSeen> &now, int lane);

// Once a flip, before the interface draws. Installs the hit watch while
// `enabled` (or while MHFU_TRACE_HITS is set; game/hit_trace.hpp) and drops
// it otherwise. Direct chained calls stay on their fast path either way.
// Numbers are shown only while `enabled`.
// `pose` is the game's camera this frame, when its update ran.
void damage_numbers_frame(bool enabled, const psprecomp::GuestMemory *memory = nullptr,
    const std::optional<EyePose> &pose = std::nullopt);
// What the interface draws this frame. Empty while the setting is off.
[[nodiscard]] const std::vector<Number> &damage_numbers();
void set_shown_damage_numbers(std::vector<Number> numbers);

} // namespace mhfu::game
