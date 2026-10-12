#include "game/gather_icon.hpp"

#include "game/damage_numbers.hpp"
#include "settings/settings.hpp"

#include <algorithm>
#include <bit>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <utility>

namespace mhfu::game {
namespace {

// game_task 0x09A6F490 and the two calls it makes to find the list.
constexpr GatherWord kWords[] = {
    {0x09A6F490u, 0x27BDFFF0u}, // addiu sp,sp,-16: the search
    {0x09A6F4ACu, 0x3C0208A6u}, // lui v0,0x08A6: the gather object
    {0x09A6F4B0u, 0x8C442D9Cu}, // lw a0,11676(v0): the pointer at 0x08A62D9C
    {0x09A6F4B4u, 0x9645029Au}, // lhu a1,666(s2): the hunter's stage
    {0x09A6F4D0u, 0x3C03BF80u}, // lui v1,0xBF80: the list ends where x is -1
    {0x09A6F4E8u, 0x3C0342C8u}, // lui v1,0x42C8: 100 above the node
    {0x09A6F4F0u, 0x3C034348u}, // lui v1,0x4348: 200 below the node
    {0x08869570u, 0x27BDFFF0u}, // addiu sp,sp,-16: the area lookup
    {0x08869580u, 0x8C900060u}, // lw s0,96(a0): the area table
    {0x088695A8u, 0x9065AF10u}, // lbu a1,-20720(v1): the area byte at GameSys+0x6AF10
    {0x088C722Cu, 0x3C0209CEu}, // lui v0,0x09CE: the stage-id table
    {0x088C7234u, 0x24424808u}, // addiu v0,v0,0x4808: the table at 0x09CE4808
    {0x08886E80u, 0x3C03089Du}, // lui v1,0x89d: the base of the hunter list
    {0x08886EF8u, 0x8E130A84u}, // lw s3,2692(s0): the hunter the camera follows
    {0x09AA4E1Cu, 0x96440324u}, // lhu a0,804(s2): the hunter's current action
};

constexpr std::uint32_t kGatherObject = 0x08A62D9Cu;
constexpr std::uint32_t kAreas = 96u;
constexpr std::uint32_t kListBias = 80u;
constexpr std::uint32_t kGameSysPointer = 0x089CC558u;
constexpr std::uint32_t kAreaByte = 0x6AF10u;
constexpr std::uint32_t kStageTable = 0x09CE4808u;
constexpr std::uint32_t kHunterList = 0x089CBD04u;
constexpr std::uint32_t kFollowedHunter = 2692u;
constexpr std::uint32_t kHunterPosition = 0x200u;
constexpr std::uint32_t kHunterStage = 0x29Au;
constexpr std::uint32_t kHunterAction = 804u;
constexpr std::uint32_t kNodeStride = 24u;
constexpr std::uint32_t kNodeRadius = 12u;
constexpr std::uint32_t kNodeId = 16u;
constexpr std::uint32_t kNodeKind = 20u;
constexpr std::uint32_t kSentinel = 0xBF800000u; // -1, where the list ends
constexpr std::uint32_t kMaxNodes = 64u;
constexpr std::uint32_t kMaxStages = 256u;
constexpr float kBelow = 200.0f;
constexpr float kAbove = 100.0f;
constexpr std::uint32_t kRam = 0x08800000u;

struct Noted {
    GatherPicture picture{};
    std::uint32_t vertices{};
};

Noted &noted() {
    static Noted value;
    return value;
}

bool trace_on() {
    static const bool on = std::getenv("MHFU_TRACE_GATHER") != nullptr;
    return on;
}

float load_float(const psprecomp::GuestMemory &memory, std::uint32_t address) {
    return std::bit_cast<float>(memory.load32(address));
}

bool in_ram(std::uint32_t address, std::uint32_t bytes, const psprecomp::GuestMemory &memory) {
    return address >= kRam && memory.raw_pointer(address, bytes) != nullptr;
}

bool &code_logged() {
    static bool logged = false;
    return logged;
}

void trace_node(const std::optional<GatherNode> &node) {
    if (!trace_on()) return;
    static std::uint32_t last = 1u; // not an address, so the first result is printed
    const std::uint32_t address = node ? node->address : 0u;
    if (address == last) return;
    last = address;
    if (!node) {
        std::cout << "[gather] none" << std::endl;
        return;
    }
    std::cout << "[gather] node 0x" << std::hex << node->address << std::dec << " id " << node->id << " kind "
              << node->kind << " at " << node->x << "," << node->y << "," << node->z << std::endl;
}

} // namespace

std::span<const GatherWord> gather_checked_words() {
    return kWords;
}

GatherShape gather_mark_shape(std::uint32_t size_percent, std::uint32_t thickness) noexcept {
    const std::uint32_t size = std::clamp(size_percent, settings::kMinGatherIconSize, settings::kMaxGatherIconSize);
    const std::uint32_t weight = std::clamp(thickness, 1u, settings::kMaxGatherIconThickness);
    const float scale = static_cast<float>(size) / 100.0f;
    GatherShape shape;
    shape.radius = kGatherDisc * scale;
    shape.glyph = kGatherGlyph * scale;
    shape.ring = scale * static_cast<float>(weight);
    shape.weight = 0.5f * scale * static_cast<float>(weight - 1u);
    return shape;
}

// The search lives in game_task.ovl. Until that overlay is loaded the words
// are not the search, and that is a quiet miss rather than a code change.
constexpr std::uint32_t kTaskSlot = 0x09A5F300u;
constexpr std::uint32_t kOverlayMagic = 0x336F574Du; // "MWo3"
constexpr char kQuestOverlay[] = "game_task.ovl";

bool quest_overlay(const psprecomp::GuestMemory &memory) {
    if (memory.raw_pointer(kTaskSlot, 64u) == nullptr || memory.load32(kTaskSlot) != kOverlayMagic ||
        memory.load32(kTaskSlot + 8u) != kTaskSlot)
        return false;
    for (std::size_t i = 0; i < sizeof(kQuestOverlay); ++i)
        if (memory.load8(kTaskSlot + 32u + static_cast<std::uint32_t>(i)) != static_cast<std::uint8_t>(kQuestOverlay[i]))
            return false;
    return true;
}

bool gather_code_matches(const psprecomp::GuestMemory &memory) {
    if (!quest_overlay(memory)) return false;
    bool ready = true;
    bool differs = false;
    for (const GatherWord &word : kWords) {
        if (memory.raw_pointer(word.address, 4u) == nullptr) {
            ready = false;
            continue;
        }
        if (memory.load32(word.address) != word.instruction) differs = true;
    }
    if (differs && !code_logged()) {
        code_logged() = true;
        for (const GatherWord &word : kWords)
            if (memory.raw_pointer(word.address, 4u) != nullptr && memory.load32(word.address) != word.instruction)
                std::cout << "[gather] game code differs at 0x" << std::hex << word.address << ": 0x"
                          << memory.load32(word.address) << ", expected 0x" << word.instruction << std::dec
                          << std::endl;
    }
    return ready && !differs;
}

std::optional<GatherNode> gather_from_list(const psprecomp::GuestMemory &memory, std::uint32_t list, float hunter_x,
    float hunter_y, float hunter_z) {
    if (!std::isfinite(hunter_x) || !std::isfinite(hunter_y) || !std::isfinite(hunter_z)) return std::nullopt;
    for (std::uint32_t n = 0; n < kMaxNodes; ++n) {
        if (!in_ram(list, kNodeStride, memory)) return std::nullopt;
        const std::uint32_t x_bits = memory.load32(list);
        if (x_bits == kSentinel) return std::nullopt;
        const float x = std::bit_cast<float>(x_bits);
        const float y = load_float(memory, list + 4u);
        const float z = load_float(memory, list + 8u);
        const float radius = load_float(memory, list + kNodeRadius);
        if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(z) || !std::isfinite(radius)) return std::nullopt;
        const float dx = hunter_x - x;
        const float dz = hunter_z - z;
        // 0x09A6F4E8: at or above the node minus 200, and below the node plus 100.
        if (hunter_y >= y - kBelow && hunter_y < y + kAbove && dx * dx + dz * dz <= radius * radius)
            return GatherNode{list, x, y, z, memory.load16(list + kNodeId), memory.load16(list + kNodeKind)};
        list += kNodeStride;
    }
    return std::nullopt;
}

std::optional<std::uint32_t> gather_list_for(const psprecomp::GuestMemory &memory, std::uint16_t stage) {
    if (!in_ram(kGameSysPointer, 4u, memory) || !in_ram(kGatherObject, 4u, memory)) return std::nullopt;
    const std::uint32_t gamesys = memory.load32(kGameSysPointer);
    const std::uint32_t object = memory.load32(kGatherObject);
    if (!in_ram(gamesys + kAreaByte, 1u, memory) || !in_ram(object + kAreas, 4u, memory) ||
        !in_ram(object + kListBias, 4u, memory))
        return std::nullopt;
    const std::uint32_t entry = kStageTable + static_cast<std::uint32_t>(memory.load8(gamesys + kAreaByte)) * 8u;
    if (!in_ram(entry, 8u, memory)) return std::nullopt;
    const auto count = static_cast<std::int32_t>(memory.load32(entry));
    const std::uint32_t ids = memory.load32(entry + 4u);
    if (count <= 0 || !in_ram(ids, 2u, memory)) return std::nullopt;
    const std::uint32_t limit = std::min(static_cast<std::uint32_t>(count), kMaxStages);
    std::optional<std::uint32_t> index;
    for (std::uint32_t i = 0; i < limit; ++i) {
        if (!in_ram(ids + i * 2u, 2u, memory)) return std::nullopt;
        if (memory.load16(ids + i * 2u) == stage) {
            index = i;
            break;
        }
    }
    if (!index) return std::nullopt;
    const std::uint32_t areas = memory.load32(object + kAreas);
    if (!in_ram(areas + *index * 4u, 4u, memory)) return std::nullopt;
    const std::uint32_t base = memory.load32(areas + *index * 4u);
    if (base == 0u) return std::nullopt;
    const std::uint32_t list = base + memory.load32(object + kListBias);
    if (!in_ram(list, kNodeStride, memory)) return std::nullopt;
    return list;
}

namespace {

std::optional<std::uint32_t> followed_hunter(const psprecomp::GuestMemory &memory) {
    if (!gather_code_matches(memory) || !in_ram(kHunterList, 4u, memory)) return std::nullopt;
    const std::uint32_t list = memory.load32(kHunterList);
    if (!in_ram(list + kFollowedHunter, 4u, memory)) return std::nullopt;
    const std::uint32_t hunter = memory.load32(list + kFollowedHunter);
    if (hunter < kRam) return std::nullopt;
    return hunter;
}

std::optional<GatherNode> read_node(const psprecomp::GuestMemory &memory) {
    const std::optional<std::uint32_t> hunter = followed_hunter(memory);
    if (!hunter) return std::nullopt;
    // Position at +0x200 through the stage halfword at +0x29A.
    if (!in_ram(*hunter + kHunterPosition, kHunterStage + 2u - kHunterPosition, memory)) return std::nullopt;
    const std::uint16_t stage = memory.load16(*hunter + kHunterStage);
    const std::optional<std::uint32_t> nodes = gather_list_for(memory, stage);
    if (!nodes) return std::nullopt;
    return gather_from_list(memory, *nodes, load_float(memory, *hunter + kHunterPosition),
        load_float(memory, *hunter + kHunterPosition + 4u), load_float(memory, *hunter + kHunterPosition + 8u));
}

std::optional<std::uint16_t> hunter_action(const psprecomp::GuestMemory &memory) {
    const std::optional<std::uint32_t> hunter = followed_hunter(memory);
    if (!hunter || !in_ram(*hunter + kHunterAction, 2u, memory)) return std::nullopt;
    return memory.load16(*hunter + kHunterAction);
}

} // namespace

std::optional<GatherNode> gather_node(const psprecomp::GuestMemory &memory) {
    const std::optional<GatherNode> node = read_node(memory);
    trace_node(node);
    return node;
}

void gather_note_draw(const GatherPicture &picture, std::uint32_t vertices) {
    if (vertices == 0u || std::fabs(picture.projection[11]) < 1.0e-6f) return;
    Noted &slot = noted();
    if (vertices <= slot.vertices) return;
    slot.picture = picture;
    slot.vertices = vertices;
}

std::optional<std::array<float, 2>> gather_mark(
    const psprecomp::GuestMemory &memory, const std::optional<GatherPose> &pose) {
    Noted &slot = noted();
    const GatherPicture picture = slot.picture;
    const std::uint32_t vertices = std::exchange(slot.vertices, 0u);
    slot.picture = {};
    if (!settings::current().gather_icon || vertices == 0u || !pose) return std::nullopt;
    const std::optional<GatherNode> node = gather_node(memory);
    if (!node) return std::nullopt;
    const WorldPoint eye{pose->eye_x, pose->eye_y, pose->eye_z};
    const WorldPoint look{pose->look_x, pose->look_y, pose->look_z};
    const std::optional<std::array<float, 16>> view = look_at_view(eye, look);
    if (!view || picture.x_scale == 0.0f || picture.y_scale == 0.0f) return std::nullopt;
    WorldCamera camera;
    camera.view = *view;
    camera.projection = picture.projection;
    camera.x_scale = picture.x_scale;
    camera.y_scale = picture.y_scale;
    camera.x_offset = picture.x_offset;
    camera.y_offset = picture.y_offset;
    camera.origin_x = picture.origin_x;
    camera.origin_y = picture.origin_y;
    const std::optional<PicturePoint> at = project_world(camera, node->x, node->y + kGatherLift, node->z);
    if (!at) return std::nullopt;
    return std::array<float, 2>{at->x, at->y};
}

std::uint32_t GatherRepeat::apply(std::uint32_t buttons, bool ready, std::optional<std::uint16_t> action) {
    constexpr std::uint32_t kCircle = 0x2000u; // ○, B on an Xbox pad: gather
    constexpr std::uint32_t kLead = 0x0100u;   // L, held with ○ for the item bar
    // L+○ scrolls the item bar. Leave it, and leave a gap that is already running.
    if ((buttons & kLead) != 0u) return buttons;
    const bool down = (buttons & kCircle) != 0u;
    if (!down || !ready || !action) {
        input_down_ = down;
        phase_ = Phase::Idle;
        return buttons;
    }
    // ○ was already down on the read before this point was in range, so
    // the game has no new press until ○ comes up and goes down again.
    if (phase_ == Phase::Idle) {
        free_action_ = *action;
        gap_ = 0;
        phase_ = input_down_ ? Phase::Gap : Phase::Down;
    }
    input_down_ = true;
    if (phase_ == Phase::Down) {
        if (*action == free_action_) return buttons;
        phase_ = Phase::Busy;
        return buttons & ~kCircle;
    }
    if (phase_ == Phase::Busy) {
        if (*action != free_action_) return buttons & ~kCircle;
        phase_ = Phase::Gap;
        gap_ = 0;
    }
    if (*action != free_action_) {
        phase_ = Phase::Busy;
        return buttons & ~kCircle;
    }
    if (gap_ < kGatherHoldGap) {
        ++gap_;
        return buttons & ~kCircle;
    }
    phase_ = Phase::Down;
    free_action_ = *action;
    if (trace_on()) std::cout << "[gather] hold" << std::endl;
    return buttons;
}

std::uint32_t gather_hold(std::uint32_t buttons, const psprecomp::GuestMemory &memory) {
    static GatherRepeat repeat;
    if (!settings::current().gather_hold) {
        repeat.reset();
        return buttons;
    }
    bool ready = false;
    std::optional<std::uint16_t> action;
    constexpr std::uint32_t kCircle = 0x2000u;
    constexpr std::uint32_t kLead = 0x0100u;
    if ((buttons & kLead) == 0u && (buttons & kCircle) != 0u) {
        ready = gather_node(memory).has_value();
        if (ready) action = hunter_action(memory);
    }
    return repeat.apply(buttons, ready, action);
}

} // namespace mhfu::game
