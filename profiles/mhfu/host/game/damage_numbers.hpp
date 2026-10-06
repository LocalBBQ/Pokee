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
// on the picture. A hit with no place on screen is not drawn.

#include "psprecomp/guest_memory.hpp"

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace mhfu::game {

inline constexpr std::uint32_t kHitAdjust = 0x09AC7A08u;
inline constexpr std::uint32_t kStatusTick = 0x09AD2698u;
inline constexpr std::uint32_t kMonsterHealth = 0x2E4u;
// World position, three floats, y up. Lock-on reads the same field.
inline constexpr std::uint32_t kMonsterPosition = 0x200u;
// Above the feet, where a number sits on the body.
inline constexpr float kBodyHeight = 100.0f;
inline constexpr int kWeakPointHitzone = 45;
inline constexpr float kNumberSeconds = 1.60f;
inline constexpr std::size_t kMaxOnScreen = 8u;

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
    bool placed{};
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

struct PicturePoint {
    float x{}; // 0..1 across the game picture
    float y{}; // 0..1 down it
};

struct Anchor {
    float x{}; // 0..1 across the game picture, from the left
    float y{}; // 0..1 down the game picture, from the top
    float alpha{};
};

[[nodiscard]] Number make_number(int amount, Grade grade, int lane) noexcept;
[[nodiscard]] std::string damage_number_text(const Number &number);
[[nodiscard]] std::vector<Number> age_numbers(std::vector<Number> numbers, float seconds);
[[nodiscard]] Anchor number_anchor(const Number &number) noexcept;

// Where a world point lands on the picture. Nothing when it is behind the
// camera or off the picture.
[[nodiscard]] std::optional<PicturePoint> project_world(const WorldCamera &camera, float x, float y, float z) noexcept;
// The monster's position, when those three floats are mapped.
[[nodiscard]] std::optional<WorldPoint> monster_position(const psprecomp::GuestMemory &memory, std::uint32_t object);

// The renderer reports each transformed draw. The perspective draw with the
// most vertices is the world camera for the numbers. An orthographic draw
// (the HUD) is ignored. `take_world_camera` returns that draw and clears it,
// so the next frame starts again.
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

// Once a flip, before the interface draws. Installs the hit watch while
// `enabled` and drops it otherwise. Direct chained calls stay on their fast
// path either way.
void damage_numbers_frame(bool enabled, const psprecomp::GuestMemory *memory = nullptr);
// What the interface draws this frame. Empty while the setting is off.
[[nodiscard]] const std::vector<Number> &damage_numbers();
void set_shown_damage_numbers(std::vector<Number> numbers);

} // namespace mhfu::game
