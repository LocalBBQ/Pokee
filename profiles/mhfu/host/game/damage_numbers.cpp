#include "game/damage_numbers.hpp"

#include "game/guest_layout.hpp"
#include "game/hit_trace.hpp"
#include "psprecomp/runtime.hpp"

#include <algorithm>
#include <bit>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
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

std::vector<HealthSeen> &health_seen() {
    static std::vector<HealthSeen> seen;
    return seen;
}

// Status ticks under way, by depth; `health` is unused.
std::vector<OpenHit> &open_ticks() {
    static std::vector<OpenHit> open;
    return open;
}

struct ListEntry {
    std::uint32_t entity{};
    std::uint8_t kind{}; // the node's type byte
};

// The quest entity list, newest first. Empty outside a quest.
std::vector<ListEntry> quest_entries(const psprecomp::GuestMemory &memory) {
    std::vector<ListEntry> entries;
    if (!layout::quest_running(memory)) return entries;
    layout::for_each_entity(
        memory, [&](const layout::EntityNode &node) { entries.push_back({node.entity, node.type}); });
    return entries;
}

// Hunters share a class. Only this console's own hunter is sure to carry the
// player node type, so an entity with a player's vtable is a hunter too.
std::vector<std::uint32_t> hunter_classes(const psprecomp::GuestMemory &memory, const std::vector<ListEntry> &entries) {
    std::vector<std::uint32_t> classes;
    for (const ListEntry &entry : entries)
        if (entry.kind == layout::kPlayerNode && layout::in_ram(entry.entity) && memory.contains(entry.entity, 4u))
            classes.push_back(memory.load32(entry.entity + layout::kVtable));
    return classes;
}

// Frames since start, for MHFU_TRACE_HITS.
std::atomic<std::uint64_t> &frame_count() {
    static std::atomic<std::uint64_t> count{0u};
    return count;
}

// MHFU_TRACE_HITS: one [dmg] line per drop a number is made from, whichever
// way it was found, on the ad hoc trace's clock. `how` is "routine" (the hit
// was applied on this console) or "health" (the drop was written elsewhere,
// from the network: several hits in one update arrive as one drop).
void trace_damage(const char *how, std::uint32_t object, int amount, int health, const char *note) {
    if (!hit_trace::enabled()) return;
    char line[160];
    std::snprintf(line, sizeof(line), "[dmg] t=%llums frame=%llu %s %08X -%d -> %d%s",
        static_cast<unsigned long long>(hit_trace::now_ms()),
        static_cast<unsigned long long>(frame_count().load(std::memory_order_relaxed)), how, object, amount, health,
        note);
    std::cout << line << std::endl;
}

bool &hook_installed() {
    static bool installed = false;
    return installed;
}

bool &observers_before() {
    static bool before = false;
    return before;
}

// This frame's perspective views, each with the vertices drawn through it.
struct PendingView {
    WorldCamera camera{};
    std::uint64_t vertices{};
};

struct PendingCamera {
    std::vector<PendingView> views;
    std::uint32_t vertices{}; // over every view; 0 when nothing was drawn
};

constexpr std::size_t kMaxViews = 16u;

PendingCamera &pending_camera() {
    static PendingCamera pending;
    return pending;
}

std::optional<WorldCamera> take_pending_camera() {
    PendingCamera &pending = pending_camera();
    if (pending.vertices == 0u || pending.views.empty()) {
        pending.views.clear();
        pending.vertices = 0u;
        return std::nullopt;
    }
    const PendingView *best = nullptr;
    const PendingView *best_centred = nullptr;
    for (const PendingView &view : pending.views) {
        const PendingView *&slot = camera_centred(view.camera) ? best_centred : best;
        if (slot == nullptr || view.vertices > slot->vertices) slot = &view;
    }
    const WorldCamera chosen = best != nullptr ? best->camera : best_centred->camera;
    pending.views.clear();
    pending.vertices = 0u;
    return chosen;
}

// MHFU_TRACE_HITS: why a number is or is not on the picture. A few lines for
// each number until it is placed, and one when it is.
void report_place(Number &number, const char *outcome, const WorldCamera *camera, const WorldPoint *at,
    const std::optional<PicturePoint> &point) {
    if (!hit_trace::enabled() || number.reports >= 4u || (number.placed && number.reports > 0u)) return;
    ++number.reports;
    char line[512];
    int used = std::snprintf(line, sizeof(line), "[hit] number %d on %08X: %s", number.amount, number.object, outcome);
    if (at != nullptr && used > 0 && used < static_cast<int>(sizeof(line)))
        used += std::snprintf(line + used, sizeof(line) - static_cast<std::size_t>(used), "; position %g %g %g",
            static_cast<double>(at->x), static_cast<double>(at->y), static_cast<double>(at->z));
    if (point && used > 0 && used < static_cast<int>(sizeof(line)))
        used += std::snprintf(line + used, sizeof(line) - static_cast<std::size_t>(used), "; picture %g %g",
            static_cast<double>(point->x), static_cast<double>(point->y));
    if (camera != nullptr && used > 0 && used < static_cast<int>(sizeof(line))) {
        const WorldCamera &c = *camera;
        std::snprintf(line + used, sizeof(line) - static_cast<std::size_t>(used),
            "; view t %g %g %g; projection %g %g %g %g %g; viewport %g %g %g %g origin %g %g",
            static_cast<double>(c.view[12]), static_cast<double>(c.view[13]), static_cast<double>(c.view[14]),
            static_cast<double>(c.projection[0]), static_cast<double>(c.projection[5]),
            static_cast<double>(c.projection[10]), static_cast<double>(c.projection[11]),
            static_cast<double>(c.projection[14]), static_cast<double>(c.x_scale), static_cast<double>(c.y_scale),
            static_cast<double>(c.x_offset), static_cast<double>(c.y_offset), static_cast<double>(c.origin_x),
            static_cast<double>(c.origin_y));
    }
    std::cout << line << std::endl;
}

void place_numbers(
    std::vector<Number> &live, const psprecomp::GuestMemory *memory, const std::optional<EyePose> &pose) {
    std::optional<WorldCamera> camera = take_pending_camera();
    if (camera && pose)
        if (const std::optional<std::array<float, 16>> view = look_at_view(pose->eye, pose->look_at))
            camera->view = *view;
    if (!camera || memory == nullptr) {
        for (Number &number : live)
            if (!number.placed) report_place(number, "no world camera this frame", nullptr, nullptr, std::nullopt);
        return;
    }
    for (Number &number : live) {
        if (number.object == 0u) continue;
        const std::optional<WorldPoint> at = monster_position(*memory, number.object);
        if (!at) {
            number.placed = false;
            report_place(number, "the monster's position cannot be read", &*camera, nullptr, std::nullopt);
            continue;
        }
        const std::optional<PicturePoint> picture =
            project_world(*camera, at->x, at->y + kBodyHeight, at->z);
        if (!picture) {
            number.placed = false;
            report_place(number, "the point is behind the camera or off the picture", &*camera, &*at,
                project_point(*camera, at->x, at->y + kBodyHeight, at->z));
            continue;
        }
        number.x = picture->x;
        number.y = picture->y;
        // How tall the body is on the picture, feet to the number's point;
        // the feet may be off the picture when the camera is close.
        const std::optional<PicturePoint> feet = project_point(*camera, at->x, at->y, at->z);
        number.body = feet ? std::fabs(feet->y - picture->y) : 0.0f;
        number.placed = true;
        report_place(number, "placed", &*camera, &*at, picture);
    }
}

void pre_hit(psprecomp::Runtime &runtime, psprecomp::AllegrexContext &ctx, std::uint32_t target_pc,
    std::uint32_t native_depth) {
    if (!watch_armed().load(std::memory_order_relaxed)) return;
    if (target_pc == kStatusTick) {
        const std::optional<int> health = monster_health(runtime.memory(), ctx.gpr[4]);
        if (!health) return;
        std::lock_guard lock(numbers_mutex());
        open_ticks().push_back(OpenHit{native_depth, ctx.gpr[4], *health});
        return;
    }
    if (target_pc != kHitAdjust) return;
    const std::uint32_t object = ctx.gpr[4];
    const std::optional<int> health = monster_health(runtime.memory(), object);
    if (hit_trace::enabled()) hit_trace::note_call(runtime.memory(), ctx, native_depth, health);
    if (!health) return;
    std::lock_guard lock(numbers_mutex());
    watch_hit_enter(open_hits(), target_pc, native_depth, object, *health);
}

void post_hit(psprecomp::Runtime &runtime, psprecomp::AllegrexContext &ctx, std::uint32_t target_pc,
    std::uint32_t native_depth) {
    if (!watch_armed().load(std::memory_order_relaxed)) return;
    if (target_pc == kStatusTick) {
        std::optional<OpenHit> ticked;
        {
            std::lock_guard lock(numbers_mutex());
            std::vector<OpenHit> &open = open_ticks();
            for (auto it = open.rbegin(); it != open.rend(); ++it) {
                if (it->depth != native_depth) continue;
                ticked = *it;
                open.erase(std::next(it).base());
                break;
            }
        }
        if (!ticked) return;
        if (const std::optional<int> health = monster_health(runtime.memory(), ticked->object)) {
            std::lock_guard lock(numbers_mutex());
            note_change(health_seen(), ticked->object, ticked->health, *health);
        }
        return;
    }
    if (target_pc != kHitAdjust) return;
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
    for (auto it = open_hits().rbegin(); it != open_hits().rend(); ++it) {
        if (it->depth != native_depth) continue;
        note_change(health_seen(), object, it->health, sample);
        break;
    }
    if (hit_trace::enabled()) {
        for (auto it = open_hits().rbegin(); it != open_hits().rend(); ++it) {
            if (it->depth != native_depth) continue;
            (void)hit_trace::log_return(ctx, native_depth, it->health, sample);
            break;
        }
    }
    // The routine also takes health off a hunter the monster hits.
    if (std::optional<Number> number =
            watch_hit_leave(open_hits(), target_pc, native_depth, sample, static_cast<int>(live_numbers().size()))) {
        const bool hunter = is_hunter(runtime.memory(), number->object);
        trace_damage("routine", number->object, number->amount, sample, hunter ? " hunter, not shown" : "");
        if (!hunter) remember_number(live_numbers(), *number);
    }
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

std::vector<Offset> stroke_offsets(float radius, int rings) {
    std::vector<Offset> offsets{{0.0f, 0.0f}};
    if (!(radius >= 0.25f) || rings <= 0) return offsets;
    constexpr int kPoints = 12;
    constexpr float kTurn = 6.28318530718f;
    for (int ring = 1; ring <= rings; ++ring) {
        const float r = radius * static_cast<float>(ring) / static_cast<float>(rings);
        for (int point = 0; point < kPoints; ++point) {
            const float angle = kTurn * static_cast<float>(point) / static_cast<float>(kPoints);
            offsets.push_back({r * std::cos(angle), r * std::sin(angle)});
        }
    }
    return offsets;
}

Stroke number_stroke(std::uint32_t thickness, float size, float unit_size) noexcept {
    if (thickness == 0u || !(size > 0.0f) || !(unit_size > 0.0f)) return {};
    // At least one pixel of outline however small the number is drawn.
    const float pixel = std::max(1.0f, size / unit_size);
    return {pixel * static_cast<float>(thickness), 0.5f * pixel * static_cast<float>(thickness - 1u)};
}

float stack_gap_px(float tuned_px, float size_scale, float glyph_px, float stroke_px, bool clear) noexcept {
    if (!(tuned_px > 0.0f)) tuned_px = 0.0f;
    if (!(size_scale > 0.0f)) size_scale = 1.0f;
    if (!(glyph_px > 0.0f)) glyph_px = 0.0f;
    if (!(stroke_px > 0.0f)) stroke_px = 0.0f;
    if (!clear) return tuned_px;
    return std::max(tuned_px * size_scale, glyph_px + 2.0f * stroke_px);
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
    // On the hit's point, never beside it: later hits sit above the earlier
    // ones, and the whole rises a little. Up close, smaller, so the number
    // keeps to the point rather than spreading over the body.
    anchor.scale = closeness_scale(number.body);
    anchor.x = number.x;
    anchor.y = number.y - (life * kRise + static_cast<float>(number.lane) * kStack) * anchor.scale;
    anchor.alpha = life < 0.70f ? 1.0f : (1.0f - life) / 0.30f;
    if (anchor.alpha < 0.0f) anchor.alpha = 0.0f;
    return anchor;
}

float closeness_scale(float body) noexcept {
    if (!std::isfinite(body) || body <= kShrinkFrom) return 1.0f;
    const float t = std::min((body - kShrinkFrom) / (kShrinkTo - kShrinkFrom), 1.0f);
    return 1.0f - t * (1.0f - kMinScale);
}

std::optional<std::array<float, 16>> look_at_view(const WorldPoint &eye, const WorldPoint &target) noexcept {
    float forward[3] = {target.x - eye.x, target.y - eye.y, target.z - eye.z};
    const float length = std::sqrt(forward[0] * forward[0] + forward[1] * forward[1] + forward[2] * forward[2]);
    if (!std::isfinite(length) || length < 1.0e-4f) return std::nullopt;
    for (float &f : forward) f /= length;
    // forward x (0, 1, 0)
    float right[3] = {-forward[2], 0.0f, forward[0]};
    const float level = std::sqrt(right[0] * right[0] + right[2] * right[2]);
    if (level < 1.0e-4f) return std::nullopt;
    right[0] /= level;
    right[2] /= level;
    const float back[3] = {-forward[0], -forward[1], -forward[2]};
    // back x right
    const float up[3] = {back[1] * right[2] - back[2] * right[1], back[2] * right[0] - back[0] * right[2],
        back[0] * right[1] - back[1] * right[0]};
    const float at[3] = {eye.x, eye.y, eye.z};
    std::array<float, 16> view{};
    for (std::size_t axis = 0; axis < 3u; ++axis) {
        view[axis * 4u + 0u] = right[axis];
        view[axis * 4u + 1u] = up[axis];
        view[axis * 4u + 2u] = back[axis];
    }
    view[12] = -(right[0] * at[0] + right[1] * at[1] + right[2] * at[2]);
    view[13] = -(up[0] * at[0] + up[1] * at[1] + up[2] * at[2]);
    view[14] = -(back[0] * at[0] + back[1] * at[1] + back[2] * at[2]);
    view[15] = 1.0f;
    return view;
}

std::optional<PicturePoint> project_world(const WorldCamera &camera, float x, float y, float z) noexcept {
    const std::optional<PicturePoint> point = project_point(camera, x, y, z);
    if (!point || point->x < 0.0f || point->x > 1.0f || point->y < 0.0f || point->y > 1.0f) return std::nullopt;
    return point;
}

std::optional<PicturePoint> project_point(const WorldCamera &camera, float x, float y, float z) noexcept {
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
    if (!std::isfinite(picture_x) || !std::isfinite(picture_y)) return std::nullopt;
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

bool camera_centred(const WorldCamera &camera) noexcept {
    constexpr float kNoTranslation = 1.0e-3f;
    return std::fabs(camera.view[12]) < kNoTranslation && std::fabs(camera.view[13]) < kNoTranslation &&
        std::fabs(camera.view[14]) < kNoTranslation;
}

void note_world_camera(const WorldCamera &camera, std::uint32_t vertices) {
    if (vertices == 0u || std::fabs(camera.projection[11]) < 1.0e-6f) return;
    std::lock_guard lock(numbers_mutex());
    PendingCamera &pending = pending_camera();
    pending.vertices += vertices;
    for (PendingView &view : pending.views) {
        if (view.camera.view == camera.view && view.camera.projection == camera.projection) {
            view.vertices += vertices;
            return;
        }
    }
    if (pending.views.size() < kMaxViews) pending.views.push_back({camera, vertices});
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

bool parse_health_watch(const char *text) noexcept {
    return text == nullptr || std::strcmp(text, "0") != 0;
}

bool health_watch_enabled() noexcept {
    static const bool on = parse_health_watch(std::getenv("MHFU_DAMAGE_NUMBERS_HEALTH_WATCH"));
    return on;
}

std::vector<HealthSeen> quest_health(const psprecomp::GuestMemory &memory) {
    std::vector<HealthSeen> found;
    const std::vector<ListEntry> entries = quest_entries(memory);
    const std::vector<std::uint32_t> hunters = hunter_classes(memory, entries);
    for (const ListEntry &entry : entries) {
        const std::uint32_t entity = entry.entity;
        if (entry.kind == layout::kPlayerNode || !layout::in_ram(entity) || (entity & 1u) != 0u ||
            !memory.contains(entity, 4u) || !memory.contains(entity + kMonsterMostHealth, 2u) ||
            !memory.contains(entity + kMonsterHealth, 2u))
            continue;
        const std::uint32_t vtable = memory.load32(entity + layout::kVtable);
        if (std::find(hunters.begin(), hunters.end(), vtable) != hunters.end()) continue;
        const int most = static_cast<std::int16_t>(memory.load16(entity + kMonsterMostHealth));
        if (most <= 0) continue; // a companion, or nothing with a health pool
        const bool repeated = std::any_of(
            found.begin(), found.end(), [entity](const HealthSeen &seen) { return seen.object == entity; });
        if (repeated) continue;
        found.push_back({entity, static_cast<std::int16_t>(memory.load16(entity + kMonsterHealth)), most, vtable,
            entry.kind});
    }
    return found;
}

bool is_hunter(const psprecomp::GuestMemory &memory, std::uint32_t object) {
    const std::vector<ListEntry> entries = quest_entries(memory);
    for (const ListEntry &entry : entries)
        if (entry.entity == object && entry.kind == layout::kPlayerNode) return true;
    if (!layout::in_ram(object) || !memory.contains(object, 4u)) return false;
    const std::vector<std::uint32_t> hunters = hunter_classes(memory, entries);
    return std::find(hunters.begin(), hunters.end(), memory.load32(object + layout::kVtable)) != hunters.end();
}

void note_change(std::vector<HealthSeen> &seen, std::uint32_t object, int before, int after) {
    for (HealthSeen &entry : seen)
        if (entry.object == object) entry.health += after - before;
}

std::vector<Number> watch_health(std::vector<HealthSeen> &seen, const std::vector<HealthSeen> &now, int lane) {
    std::vector<Number> numbers;
    for (const HealthSeen &reading : now) {
        const auto before = std::find_if(
            seen.begin(), seen.end(), [&reading](const HealthSeen &entry) { return entry.object == reading.object; });
        if (before == seen.end() || before->most != reading.most) continue;
        const std::optional<int> amount = damage_shown(before->health, reading.health);
        if (!amount) continue;
        Number number = make_number(*amount, Grade{}, lane++);
        number.object = reading.object;
        numbers.push_back(number);
    }
    seen = now;
    return numbers;
}

void damage_numbers_frame(bool enabled, const psprecomp::GuestMemory *memory, const std::optional<EyePose> &pose) {
    using Clock = std::chrono::steady_clock;
    static Clock::time_point previous = Clock::now();
    const Clock::time_point now = Clock::now();
    float seconds = std::chrono::duration<float>(now - previous).count();
    previous = now;
    if (seconds < 0.0f) seconds = 0.0f;
    if (seconds > 1.0f) seconds = 1.0f;
    frame_count().fetch_add(1u, std::memory_order_relaxed);

    // The trace watches the same routine, with the numbers themselves off.
    const bool watch = enabled || hit_trace::enabled();
    if (!watch) {
        // Off, and never turned on: the present path does not take the lock.
        if (!hook_installed()) return;
        remove_watch();
        std::lock_guard lock(numbers_mutex());
        open_hits().clear();
        open_ticks().clear();
        health_seen().clear();
        live_numbers().clear();
        shown_numbers().clear();
        pending_camera().vertices = 0u;
        pending_camera().views.clear();
        return;
    }
    install_watch();
    if (hit_trace::enabled() && memory != nullptr) hit_trace::frame(*memory, &monster_health);
    std::vector<HealthSeen> readings;
    const bool read_health = enabled && memory != nullptr && health_watch_enabled();
    if (read_health) readings = quest_health(*memory);
    std::lock_guard lock(numbers_mutex());
    live_numbers() = age_numbers(std::move(live_numbers()), seconds);
    if (read_health) {
        if (hit_trace::enabled())
            for (const HealthSeen &reading : readings)
                if (std::none_of(health_seen().begin(), health_seen().end(),
                        [&reading](const HealthSeen &seen) { return seen.object == reading.object; })) {
                    char line[160];
                    std::snprintf(line, sizeof(line),
                        "[hit] watching health of %08X: node type %u, vtable %08X, species %u, health %d/%d",
                        reading.object, static_cast<unsigned>(reading.kind), reading.vtable,
                        memory->contains(reading.object + layout::kMonsterId, 1u)
                            ? static_cast<unsigned>(memory->load8(reading.object + layout::kMonsterId))
                            : 0u,
                        reading.health, reading.most);
                    std::cout << line << std::endl;
                }
        for (const Number &number :
            watch_health(health_seen(), readings, static_cast<int>(live_numbers().size()))) {
            const auto reading = std::find_if(readings.begin(), readings.end(),
                [&number](const HealthSeen &r) { return r.object == number.object; });
            trace_damage("health", number.object, number.amount, reading != readings.end() ? reading->health : 0, "");
            remember_number(live_numbers(), number);
        }
    } else {
        health_seen().clear();
    }
    place_numbers(live_numbers(), memory, pose);
    if (enabled) shown_numbers() = live_numbers();
    else shown_numbers().clear();
}

const std::vector<Number> &damage_numbers() {
    return shown_numbers();
}

void set_shown_damage_numbers(std::vector<Number> numbers) {
    std::lock_guard lock(numbers_mutex());
    shown_numbers() = std::move(numbers);
}

} // namespace mhfu::game
