#include "frame_interpolation.hpp"

#include <algorithm>
#include <cmath>

namespace mhp3rd::gpu::interpolation {
namespace {

constexpr float kPi = 3.14159265358979f;

float length3(const float *v) noexcept { return std::sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]); }

} // namespace

DrawSummary summarize(const DrawCall &call) {
    DrawSummary draw{};
    draw.vertex_address = call.vertex_address;
    draw.index_address = call.index_address;
    draw.vertex_type = call.vertex_type;
    draw.texture_address = call.texture.enabled ? call.texture.address : 0u;
    draw.count = call.primitive_count;
    draw.primitive = call.primitive;
    draw.target = call.target.color_address;
    draw.perspective = !call.through && !call.clear_mode && !is_orthographic(call.projection);
    // Vertex type bits 9..10 give the weight format; ge_state.cpp's
    // decode_vertices skins exactly the transformed vertices that have one.
    draw.skinned = ((call.vertex_type >> 9u) & 3u) != 0u && !call.through;
    draw.world = call.world;
    draw.view = call.view;
    draw.projection = call.projection;
    return draw;
}

void mark_eligible(std::vector<DrawSummary> &draws, std::uint32_t displayed) noexcept {
    for (DrawSummary &draw : draws) draw.eligible = draw.perspective && draw.target == displayed;
}

bool is_orthographic(const Matrix &projection) noexcept {
    // A perspective projection copies -z into w: row 3 is (0, 0, -1, 0).
    return std::fabs(projection[11]) < 1e-6f;
}

std::size_t Matcher::KeyHash::operator()(const Key &key) const noexcept {
    std::uint64_t hash = 1469598103934665603ull;
    const auto mix = [&](std::uint32_t value) {
        hash ^= value;
        hash *= 1099511628211ull;
    };
    mix(key.vertex_address);
    mix(key.index_address);
    mix(key.vertex_type);
    mix(key.texture_address);
    mix(key.count);
    mix(key.primitive);
    return static_cast<std::size_t>(hash);
}

Matcher::Key Matcher::key_of(const DrawSummary &draw) noexcept {
    return Key{draw.vertex_address, draw.index_address, draw.vertex_type, draw.texture_address, draw.count,
               static_cast<std::uint8_t>(draw.primitive)};
}

const Matching &Matcher::match(const std::vector<DrawSummary> &older, const std::vector<DrawSummary> &newer,
                               const CutThresholds &thresholds) {
    Matching &out = result_;
    out = Matching{};
    out.newer_of.assign(older.size(), -1);

    for (auto &[key, slot] : slots_) {
        slot.newer.clear();
        slot.used = 0u;
    }
    for (std::size_t i = 0; i < newer.size(); ++i) {
        if (!newer[i].eligible) continue;
        ++out.eligible_newer;
        slots_[key_of(newer[i])].newer.push_back(static_cast<std::int32_t>(i));
    }
    for (std::size_t i = 0; i < older.size(); ++i) {
        if (!older[i].eligible) continue;
        ++out.eligible_older;
        const auto found = slots_.find(key_of(older[i]));
        if (found == slots_.end()) continue;
        Slot &slot = found->second;
        if (slot.used >= slot.newer.size()) {
            ++slot.used;
            continue;
        }
        out.newer_of[i] = slot.newer[slot.used++];
        ++out.matched;
    }
    // Keys neither frame used lately would pile up over a long session.
    if (slots_.size() > 8192u) slots_.clear();

    // How the camera moved: this game keeps the view matrix almost fixed and
    // puts the camera's rotation into every world matrix, so the camera shows
    // in how each matched draw moved in eye space (view times world). Most of
    // a scene stands still, so the median over the matched draws is the
    // camera's turn and move, and characters moving on their own do not
    // count. A sample of up to 256 pairs spread over the frame is enough.
    std::vector<float> turns;
    std::vector<float> moves;
    const std::size_t stride = std::max<std::size_t>(1u, out.matched / 256u);
    std::size_t seen = 0u;
    for (std::size_t i = 0; i < older.size(); ++i) {
        const std::int32_t partner = out.newer_of[i];
        if (partner < 0 || seen++ % stride != 0u) continue;
        const DrawSummary &from = older[i];
        const DrawSummary &to = newer[static_cast<std::size_t>(partner)];
        const Matrix before = multiply(from.view, from.world);
        const Matrix after = multiply(to.view, to.world);
        turns.push_back(rotation_angle_degrees(before, after));
        const float dx = after[12] - before[12], dy = after[13] - before[13], dz = after[14] - before[14];
        moves.push_back(std::sqrt(dx * dx + dy * dy + dz * dz));
    }
    if (!turns.empty()) {
        const auto median = [](std::vector<float> &values) {
            const auto middle = values.begin() + static_cast<std::ptrdiff_t>(values.size() / 2u);
            std::nth_element(values.begin(), middle, values.end());
            return *middle;
        };
        out.camera_found = true;
        out.camera_angle_degrees = median(turns);
        out.camera_distance = median(moves);
    }

    float max_angle = thresholds.max_camera_angle_degrees;
    float max_distance = thresholds.max_camera_distance;
    if (previous_blended_) {
        max_angle = std::clamp(previous_angle_degrees_ * thresholds.continuous_growth +
                                   thresholds.continuous_angle_margin_degrees,
                               max_angle, std::max(max_angle, thresholds.max_continuous_angle_degrees));
        max_distance = std::clamp(previous_distance_ * thresholds.continuous_growth +
                                      thresholds.continuous_distance_margin,
                                  max_distance, std::max(max_distance, thresholds.max_continuous_distance));
    }
    if (out.eligible_newer == 0u || out.matched == 0u) {
        out.cut = "nothing to blend";
    } else if (static_cast<float>(out.matched) <
               thresholds.min_matched_fraction * static_cast<float>(std::max(out.eligible_newer, out.eligible_older))) {
        out.cut = "few draws match";
    } else if (out.camera_found && out.camera_angle_degrees > max_angle) {
        out.cut = "camera turned";
    } else if (out.camera_found && out.camera_distance > max_distance) {
        out.cut = "camera moved";
    }
    out.continued = out.cut == nullptr && out.camera_found &&
                    (out.camera_angle_degrees > thresholds.max_camera_angle_degrees ||
                     out.camera_distance > thresholds.max_camera_distance);
    previous_blended_ = out.cut == nullptr;
    previous_angle_degrees_ = out.camera_found ? out.camera_angle_degrees : 0.0f;
    previous_distance_ = out.camera_found ? out.camera_distance : 0.0f;
    return out;
}

Matrix blend_affine(const Matrix &a, const Matrix &b, float t) noexcept {
    Matrix out = blend_linear(a, b, t);
    for (std::uint32_t column = 0; column < 3u; ++column) {
        float *axis = out.data() + column * 4u;
        const float length = length3(axis);
        if (length < 1e-12f) continue;
        const float wanted = length3(a.data() + column * 4u) * (1.0f - t) + length3(b.data() + column * 4u) * t;
        const float scale = wanted / length;
        for (std::uint32_t row = 0; row < 3u; ++row) axis[row] *= scale;
    }
    return out;
}

Matrix multiply(const Matrix &a, const Matrix &b) noexcept {
    Matrix out{};
    for (std::uint32_t column = 0; column < 4u; ++column) {
        for (std::uint32_t row = 0; row < 4u; ++row) {
            float sum = 0.0f;
            for (std::uint32_t k = 0; k < 4u; ++k) sum += a[k * 4u + row] * b[column * 4u + k];
            out[column * 4u + row] = sum;
        }
    }
    return out;
}

Matrix blend_linear(const Matrix &a, const Matrix &b, float t) noexcept {
    Matrix out{};
    for (std::size_t i = 0; i < out.size(); ++i) out[i] = a[i] + (b[i] - a[i]) * t;
    return out;
}

float rotation_angle_degrees(const Matrix &a, const Matrix &b) noexcept {
    // trace(Ra^T Rb) = 1 + 2 cos(angle) for two rotations; the columns are
    // normalised first so a uniform scale does not read as a turn.
    float trace = 0.0f;
    for (std::uint32_t column = 0; column < 3u; ++column) {
        const float *x = a.data() + column * 4u;
        const float *y = b.data() + column * 4u;
        const float lengths = length3(x) * length3(y);
        if (lengths < 1e-12f) return 180.0f;
        trace += (x[0] * y[0] + x[1] * y[1] + x[2] * y[2]) / lengths;
    }
    const float cosine = std::clamp((trace - 1.0f) * 0.5f, -1.0f, 1.0f);
    return std::acos(cosine) * 180.0f / kPi;
}

} // namespace mhp3rd::gpu::interpolation
