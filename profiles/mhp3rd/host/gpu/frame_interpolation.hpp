#pragma once

#include "ge_state.hpp"

#include <array>
#include <cstdint>
#include <unordered_map>
#include <vector>

// Frame interpolation: the game simulates at 30 frames per second, and the
// renderer presents extra frames between two of them by drawing the older
// frame's draws with their transforms blended towards the newer frame's.
// This part is independent of the backend: it recognises a draw of one frame
// in the next, decides whether two frames may be blended at all, and blends
// matrices. The renderer records the draws and replays them.
namespace mhp3rd::gpu::interpolation {

using Matrix = std::array<float, 16>;  // column-major, as in DrawCall

// What matching and blending need to know about one draw.
struct DrawSummary {
    // Identity: the same object drawn in two frames reads the same vertices
    // and indices from the same addresses with the same vertex type and
    // texture. Draws with the same identity (instances of one mesh) pair up
    // in the order the frame draws them.
    std::uint32_t vertex_address{};
    std::uint32_t index_address{};
    std::uint32_t vertex_type{};
    std::uint32_t texture_address{};
    std::uint32_t count{};
    PrimitiveType primitive{};
    std::uint32_t target{};  // framebuffer address drawn into
    // Transformed through a perspective projection, and not a clear.
    bool perspective{};
    // A perspective draw into the framebuffer the game showed. Only these are
    // blended; 2D and interface draws, orthographic ones, clears and
    // render-to-texture passes are shown as the frame drew them.
    bool eligible{};
    bool skinned{};  // the vertices were blended by bone matrices
    Matrix world{};
    Matrix view{};
    Matrix projection{};
};

// Fills the fields of a summary that come from the draw itself; `eligible`
// is decided once the frame's displayed framebuffer is known.
DrawSummary summarize(const DrawCall &call);
// Marks the perspective draws into `displayed` eligible.
void mark_eligible(std::vector<DrawSummary> &draws, std::uint32_t displayed) noexcept;

// True for a projection without perspective division.
[[nodiscard]] bool is_orthographic(const Matrix &projection) noexcept;

// How two consecutive frames' draws correspond.
struct Matching {
    // For each draw of the older frame, the index of the same draw in the
    // newer frame, or -1. Only eligible draws are matched.
    std::vector<std::int32_t> newer_of;
    std::uint32_t eligible_older{};
    std::uint32_t eligible_newer{};
    std::uint32_t matched{};
    // The camera's turn and move between the frames: the median over the
    // matched draws of how far each turned and moved in eye space.
    bool camera_found{};
    float camera_angle_degrees{};
    float camera_distance{};
    // The reason the two frames must not be blended, or null.
    const char *cut{};
    // Blended only because the motion continues the previous pair's: past
    // the plain limits of CutThresholds.
    bool continued{};
};

// Thresholds for telling a camera cut or a scene change from motion.
struct CutThresholds {
    float min_matched_fraction{0.5f};   // of the newer frame's eligible draws
    float max_camera_angle_degrees{30.0f};
    float max_camera_distance{200.0f};  // world units the camera moves in one frame
    // Motion that continues the previous pair's is not a cut even past the
    // limits above: the analog camera turns up to 720 degrees a second, 24
    // degrees a game frame in yaw alone, and more on a diagonal, with the eye
    // orbiting its look-at point. After a blended pair the limits become the
    // previous pair's turn and move times `continuous_growth` plus a margin,
    // never more than the `max_continuous_*` values. A cut in a cutscene
    // jumps from standing still or slow motion, so it still trips the limits.
    float continuous_growth{1.5f};
    float continuous_angle_margin_degrees{6.0f};
    float continuous_distance_margin{60.0f};
    float max_continuous_angle_degrees{75.0f};
    float max_continuous_distance{800.0f};
};

class Matcher {
public:
    // Pairs `older`'s eligible draws with `newer`'s and decides whether the
    // frames may be blended.
    // Each call is taken to follow the previous one (the pair before shares
    // `older` with this pair's newer frame): a pair that was blended lets the
    // next one continue its motion.
    const Matching &match(const std::vector<DrawSummary> &older, const std::vector<DrawSummary> &newer,
                          const CutThresholds &thresholds);
    // Forgets the previous pair, when the next call does not follow it.
    void forget_motion() noexcept { previous_blended_ = false; }

private:
    struct Key {
        std::uint32_t vertex_address, index_address, vertex_type, texture_address, count;
        std::uint8_t primitive;
        bool operator==(const Key &) const = default;
    };
    struct KeyHash {
        std::size_t operator()(const Key &key) const noexcept;
    };
    struct Slot {
        std::vector<std::int32_t> newer;  // the newer frame's draws with this key, in order
        std::uint32_t used{};             // how many the older frame has consumed
    };
    static Key key_of(const DrawSummary &draw) noexcept;

    std::unordered_map<Key, Slot, KeyHash> slots_;
    Matching result_;
    // The previous pair's camera motion, when that pair was blended.
    bool previous_blended_{};
    float previous_angle_degrees_{};
    float previous_distance_{};
};

// Blends two affine transforms: each basis vector keeps a length between the
// two lengths while its direction turns (a normalised blend, close to a
// rotation's slerp for the few degrees one game frame turns), and the
// translation moves linearly. `t` = 0 gives `a`, 1 gives `b`.
[[nodiscard]] Matrix blend_affine(const Matrix &a, const Matrix &b, float t) noexcept;
// a * b for column-major matrices.
[[nodiscard]] Matrix multiply(const Matrix &a, const Matrix &b) noexcept;
// Element-wise linear blend, for projections.
[[nodiscard]] Matrix blend_linear(const Matrix &a, const Matrix &b, float t) noexcept;

// The angle between the rotations of two transforms.
[[nodiscard]] float rotation_angle_degrees(const Matrix &a, const Matrix &b) noexcept;

} // namespace mhp3rd::gpu::interpolation
