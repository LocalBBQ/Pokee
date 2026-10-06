#include "game/damage_numbers.hpp"

#include "psprecomp/runtime.hpp"

#include <algorithm>
#include <bit>
#include <atomic>
#include <chrono>
#include <cmath>
#include <iterator>
#include <mutex>

namespace mhfu::game {
namespace {

std::mutex &numbers_mutex() {
    static std::mutex mutex;
    return mutex;
}

std::atomic<bool> &watch_armed() {
    static std::atomic<bool> armed{false};
    return armed;
}

std::vector<OpenHit> &open_hits() {
    static std::vector<OpenHit> open;
    return open;
}

std::vector<Number> &live_numbers() {
    static std::vector<Number> live;
    return live;
}

std::vector<Number> &shown_numbers() {
    static std::vector<Number> shown;
    return shown;
}

bool &hook_installed() {
    static bool installed = false;
    return installed;
}

bool &observers_before() {
    static bool before = false;
    return before;
}

struct PendingCamera {
    WorldCamera camera{};
    std::uint32_t vertices{};
};

PendingCamera &pending_camera() {
    static PendingCamera pending;
    return pending;
}

std::optional<WorldCamera> take_pending_camera() {
    PendingCamera &pending = pending_camera();
    if (pending.vertices == 0u) return std::nullopt;
    pending.vertices = 0u;
    return pending.camera;
}

void place_numbers(std::vector<Number> &live, const psprecomp::GuestMemory *memory) {
    const std::optional<WorldCamera> camera = take_pending_camera();
    if (!camera || memory == nullptr) return;
    for (Number &number : live) {
        if (number.object == 0u) continue;
        const std::optional<WorldPoint> at = monster_position(*memory, number.object);
        if (!at) {
            number.placed = false;
            continue;
        }
        const std::optional<PicturePoint> picture =
            project_world(*camera, at->x, at->y + kBodyHeight, at->z);
        if (!picture) {
            number.placed = false;
            continue;
        }
        number.x = picture->x;
        number.y = picture->y;
        number.placed = true;
    }
}

void pre_hit(psprecomp::Runtime &runtime, psprecomp::AllegrexContext &ctx, std::uint32_t target_pc,
    std::uint32_t native_depth) {
    if (!watch_armed().load(std::memory_order_relaxed) || target_pc != kHitAdjust) return;
    const std::uint32_t object = ctx.gpr[4];
    const std::optional<int> health = monster_health(runtime.memory(), object);
    if (!health) return;
    std::lock_guard lock(numbers_mutex());
    watch_hit_enter(open_hits(), target_pc, native_depth, object, *health);
}

void post_hit(psprecomp::Runtime &runtime, psprecomp::AllegrexContext &, std::uint32_t target_pc,
    std::uint32_t native_depth) {
    if (!watch_armed().load(std::memory_order_relaxed) || target_pc != kHitAdjust) return;
    std::uint32_t object = 0u;
    {
        std::lock_guard lock(numbers_mutex());
        const std::optional<std::uint32_t> found = open_object(open_hits(), target_pc, native_depth);
        if (!found) return;
        object = *found;
    }
    const std::optional<int> health = monster_health(runtime.memory(), object);
    std::lock_guard lock(numbers_mutex());
    int sample = 0;
    bool have = false;
    if (health) {
        sample = *health;
        have = true;
    } else {
        for (const OpenHit &hit : open_hits()) {
            if (hit.depth == native_depth && hit.object == object) {
                sample = hit.health;
                have = true;
                break;
            }
        }
    }
    if (!have) return;
    if (std::optional<Number> number =
            watch_hit_leave(open_hits(), target_pc, native_depth, sample, static_cast<int>(live_numbers().size())))
        remember_number(live_numbers(), *number);
}

void install_watch() {
    if (hook_installed()) return;
    // The setter also marks every direct chain as observed, which sends those
    // calls down the slow path for the rest of the process. This watch only
    // needs invoke_chained_call, which is how 0x09AC7A08 is reached, so the
    // direct-chain flag goes back to what it was.
    observers_before() = psprecomp::g_runtime_chain_observers_active;
    psprecomp::set_runtime_pre_chained_call_hook(&pre_hit);
    psprecomp::set_runtime_post_chained_call_hook(&post_hit);
    if (!observers_before()) psprecomp::g_runtime_chain_observers_active = false;
    hook_installed() = true;
    watch_armed().store(true, std::memory_order_relaxed);
}

void remove_watch() {
    if (!hook_installed()) return;
    watch_armed().store(false, std::memory_order_relaxed);
    psprecomp::set_runtime_pre_chained_call_hook(nullptr);
    psprecomp::set_runtime_post_chained_call_hook(nullptr);
    psprecomp::g_runtime_chain_observers_active = observers_before();
    hook_installed() = false;
}

} // namespace

float sharpness_modifier(Sharpness sharpness) noexcept {
    switch (sharpness) {
    case Sharpness::Red: return 0.50f;
    case Sharpness::Orange: return 0.75f;
    case Sharpness::Yellow: return 1.00f;
    case Sharpness::Green: return 1.05f;
    case Sharpness::Blue: return 1.20f;
    case Sharpness::White: return 1.32f;
    case Sharpness::Purple: return 1.45f;
    }
    return 1.00f;
}

Grade grade_hit(int hitzone, Sharpness sharpness, bool gunner, Affinity affinity) noexcept {
    if (hitzone < 0) hitzone = 0;
    const float effective = gunner ? static_cast<float>(hitzone)
                                   : static_cast<float>(hitzone) * sharpness_modifier(sharpness);
    Grade grade;
    grade.fill = effective >= static_cast<float>(kWeakPointHitzone) ? Fill::WeakPoint : Fill::Ordinary;
    grade.affinity = affinity;
    return grade;
}

Ink damage_number_ink(Fill fill) noexcept {
    switch (fill) {
    case Fill::WeakPoint: return Ink{255, 148, 32};
    case Fill::Ordinary: return Ink{168, 168, 168};
    case Fill::Unknown: return Ink{255, 255, 255};
    }
    return Ink{255, 255, 255};
}

std::optional<int> damage_shown(int health_before, int health_after) noexcept {
    if (health_before <= 0 || health_after >= health_before) return std::nullopt;
    return health_before - health_after;
}

Number make_number(int amount, Grade grade, int lane) noexcept {
    Number number;
    number.amount = amount;
    number.grade = grade;
    number.lane = lane;
    return number;
}

std::string damage_number_text(const Number &number) {
    std::string text = std::to_string(number.amount);
    if (number.grade.affinity == Affinity::Positive) text += '!';
    else if (number.grade.affinity == Affinity::Negative) text += '~';
    return text;
}

std::vector<Number> age_numbers(std::vector<Number> numbers, float seconds) {
    if (seconds < 0.0f) seconds = 0.0f;
    std::vector<Number> kept;
    kept.reserve(numbers.size());
    for (Number &number : numbers) {
        number.age += seconds;
        if (number.age < kNumberSeconds) kept.push_back(number);
    }
    return kept;
}

Anchor number_anchor(const Number &number) noexcept {
    Anchor anchor;
    if (!number.placed) return anchor;
    const float life = number.age / kNumberSeconds;
    const int slot = number.lane % 5;
    const int row = number.lane / 5;
    // A few pixels either side of the monster, so two hits do not share a point,
    // and a short rise that stays on the body.
    static constexpr float kStep[] = {0.0f, -1.0f, 1.0f, -2.0f, 2.0f};
    anchor.x = number.x + kStep[slot] * 0.012f;
    anchor.y = number.y - life * 0.025f - static_cast<float>(row) * 0.012f;
    anchor.alpha = life < 0.70f ? 1.0f : (1.0f - life) / 0.30f;
    if (anchor.alpha < 0.0f) anchor.alpha = 0.0f;
    return anchor;
}

std::optional<PicturePoint> project_world(const WorldCamera &camera, float x, float y, float z) noexcept {
    if (camera.x_scale == 0.0f || camera.y_scale == 0.0f) return std::nullopt;
    const float point[4] = {x, y, z, 1.0f};
    float eye[4]{};
    float clip[4]{};
    for (int row = 0; row < 4; ++row)
        for (int col = 0; col < 4; ++col)
            eye[row] += camera.view[static_cast<std::size_t>(col * 4 + row)] * point[col];
    for (int row = 0; row < 4; ++row)
        for (int col = 0; col < 4; ++col)
            clip[row] += camera.projection[static_cast<std::size_t>(col * 4 + row)] * eye[col];
    if (!std::isfinite(clip[0]) || !std::isfinite(clip[1]) || !std::isfinite(clip[3]) || clip[3] <= 1.0e-4f)
        return std::nullopt;
    const float ndc_x = clip[0] / clip[3];
    const float ndc_y = clip[1] / clip[3];
    // The GE viewport: screen = ndc * scale + offset - origin, in PSP pixels.
    const float pixel_x = ndc_x * camera.x_scale + camera.x_offset - camera.origin_x;
    const float pixel_y = ndc_y * camera.y_scale + camera.y_offset - camera.origin_y;
    const float picture_x = pixel_x / 480.0f;
    const float picture_y = pixel_y / 272.0f;
    if (!std::isfinite(picture_x) || !std::isfinite(picture_y) || picture_x < 0.0f || picture_x > 1.0f ||
        picture_y < 0.0f || picture_y > 1.0f)
        return std::nullopt;
    return PicturePoint{picture_x, picture_y};
}

std::optional<WorldPoint> monster_position(const psprecomp::GuestMemory &memory, std::uint32_t object) {
    if (object < psprecomp::GuestMemory::kPhysicalBase || (object & 1u) != 0u) return std::nullopt;
    if (memory.raw_pointer(object + kMonsterPosition, 12u) == nullptr) return std::nullopt;
    WorldPoint point;
    point.x = std::bit_cast<float>(memory.load32(object + kMonsterPosition));
    point.y = std::bit_cast<float>(memory.load32(object + kMonsterPosition + 4u));
    point.z = std::bit_cast<float>(memory.load32(object + kMonsterPosition + 8u));
    if (!std::isfinite(point.x) || !std::isfinite(point.y) || !std::isfinite(point.z)) return std::nullopt;
    return point;
}

void note_world_camera(const WorldCamera &camera, std::uint32_t vertices) {
    if (vertices == 0u || std::fabs(camera.projection[11]) < 1.0e-6f) return;
    std::lock_guard lock(numbers_mutex());
    PendingCamera &pending = pending_camera();
    if (vertices <= pending.vertices) return;
    pending.camera = camera;
    pending.vertices = vertices;
}

std::optional<WorldCamera> take_world_camera() {
    std::lock_guard lock(numbers_mutex());
    return take_pending_camera();
}

bool damage_numbers_watching() noexcept {
    return watch_armed().load(std::memory_order_relaxed);
}

void remember_number(std::vector<Number> &live, Number number) {
    live.push_back(number);
    if (live.size() > kMaxOnScreen)
        live.erase(live.begin(), live.begin() + static_cast<std::ptrdiff_t>(live.size() - kMaxOnScreen));
}

void watch_hit_enter(std::vector<OpenHit> &open, std::uint32_t pc, std::uint32_t depth, std::uint32_t object,
    int health) {
    if (pc != kHitAdjust) return;
    open.push_back(OpenHit{depth, object, health});
}

std::optional<std::uint32_t> open_object(const std::vector<OpenHit> &open, std::uint32_t pc, std::uint32_t depth) {
    if (pc != kHitAdjust) return std::nullopt;
    for (auto it = open.rbegin(); it != open.rend(); ++it)
        if (it->depth == depth) return it->object;
    return std::nullopt;
}

std::optional<Number> watch_hit_leave(
    std::vector<OpenHit> &open, std::uint32_t pc, std::uint32_t depth, int health, int lane) {
    if (pc != kHitAdjust) return std::nullopt;
    for (auto it = open.rbegin(); it != open.rend(); ++it) {
        if (it->depth != depth) continue;
        const int before = it->health;
        const std::uint32_t object = it->object;
        open.erase(std::next(it).base());
        const std::optional<int> amount = damage_shown(before, health);
        if (!amount) return std::nullopt;
        // The part's hitzone is not in this routine's arguments. White until
        // a later read of the part supplies grade_hit.
        Number number = make_number(*amount, Grade{}, lane);
        number.object = object;
        return number;
    }
    return std::nullopt;
}

std::optional<int> monster_health(const psprecomp::GuestMemory &memory, std::uint32_t object) {
    if (object < psprecomp::GuestMemory::kPhysicalBase || (object & 1u) != 0u) return std::nullopt;
    const std::uint32_t address = object + kMonsterHealth;
    if (memory.raw_pointer(address, 2u) == nullptr) return std::nullopt;
    return static_cast<int>(static_cast<std::int16_t>(memory.load16(address)));
}

void damage_numbers_frame(bool enabled, const psprecomp::GuestMemory *memory) {
    using Clock = std::chrono::steady_clock;
    static Clock::time_point previous = Clock::now();
    const Clock::time_point now = Clock::now();
    float seconds = std::chrono::duration<float>(now - previous).count();
    previous = now;
    if (seconds < 0.0f) seconds = 0.0f;
    if (seconds > 1.0f) seconds = 1.0f;

    if (!enabled) {
        // Off, and never turned on: the present path does not take the lock.
        if (!hook_installed()) return;
        remove_watch();
        std::lock_guard lock(numbers_mutex());
        open_hits().clear();
        live_numbers().clear();
        shown_numbers().clear();
        pending_camera().vertices = 0u;
        return;
    }
    install_watch();
    std::lock_guard lock(numbers_mutex());
    live_numbers() = age_numbers(std::move(live_numbers()), seconds);
    place_numbers(live_numbers(), memory);
    shown_numbers() = live_numbers();
}

const std::vector<Number> &damage_numbers() {
    return shown_numbers();
}

void set_shown_damage_numbers(std::vector<Number> numbers) {
    std::lock_guard lock(numbers_mutex());
    shown_numbers() = std::move(numbers);
}

} // namespace mhfu::game
