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

// 0x0886CEA8. The circle handler asks it only after the gather search misses.
constexpr std::uint32_t kCarveTable = 0x09A4F1A0u;
constexpr std::uint32_t kCarveStride = 32u;
constexpr std::uint32_t kCarveSlots = 20u;
constexpr std::uint32_t kCarveFirst = 4u;
constexpr std::uint32_t kCarveEmpty = 0xFFFFu;
constexpr std::uint32_t kCarveId = 20u;
constexpr std::uint32_t kCarveStage = 24u;
constexpr std::uint32_t kCarveBlocked = 1164u; // bit 8: the handler leaves the body
constexpr std::uint32_t kCarveBlockBit = 0x100u;

bool body_in_reach(const psprecomp::GuestMemory &memory, std::uint32_t record, float hunter_x, float hunter_y,
    float hunter_z) {
    const float x = load_float(memory, record);
    const float y = load_float(memory, record + 4u);
    const float z = load_float(memory, record + 8u);
    const float radius = load_float(memory, record + 12u);
    if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(z) || !std::isfinite(radius)) return false;
    const float dx = hunter_x - x;
    const float dz = hunter_z - z;
    return hunter_y >= y - kBelow && hunter_y < y + kAbove && dx * dx + dz * dz <= radius * radius;
}

} // namespace

std::optional<CarveBody> carve_body(const psprecomp::GuestMemory &memory) {
    if (!gather_code_matches(memory)) return std::nullopt;
    const std::optional<std::uint32_t> hunter = followed_hunter(memory);
    if (!hunter || !in_ram(*hunter + kHunterPosition, kHunterStage + 2u - kHunterPosition, memory) ||
        !in_ram(*hunter + kCarveBlocked, 4u, memory))
        return std::nullopt;
    if ((memory.load32(*hunter + kCarveBlocked) & kCarveBlockBit) != 0u) return std::nullopt;
    if (!in_ram(kCarveTable, 4u, memory)) return std::nullopt;
    const std::uint32_t table = memory.load32(kCarveTable);
    if (table < kRam || !in_ram(table + kCarveFirst, kCarveStride, memory)) return std::nullopt;
    const float hunter_x = load_float(memory, *hunter + kHunterPosition);
    const float hunter_y = load_float(memory, *hunter + kHunterPosition + 4u);
    const float hunter_z = load_float(memory, *hunter + kHunterPosition + 8u);
    if (!std::isfinite(hunter_x) || !std::isfinite(hunter_y) || !std::isfinite(hunter_z)) return std::nullopt;
    const std::uint16_t stage = memory.load16(*hunter + kHunterStage);
    std::uint32_t record = table + kCarveFirst;
    for (std::uint32_t n = 0; n < kCarveSlots; ++n) {
        if (!in_ram(record, kCarveStride, memory)) return std::nullopt;
        const bool empty = memory.load16(record + 16u) == kCarveEmpty;
        const bool here = memory.load16(record + kCarveStage) == stage;
        if (!empty && here) {
            if (!body_in_reach(memory, record, hunter_x, hunter_y, hunter_z)) {
                record += kCarveStride;
                continue;
            }
            if (memory.load16(record + kCarveId) == kCarveEmpty) return std::nullopt;
            return CarveBody{
                record, load_float(memory, record), load_float(memory, record + 4u), load_float(memory, record + 8u)};
        }
        record += kCarveStride;
    }
    return std::nullopt;
}

bool carve_in_reach(const psprecomp::GuestMemory &memory) {
    return carve_body(memory).has_value();
}

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
    // The circle handler tries the gather list first and carves only when it
    // misses, so a node in range takes the mark.
    std::optional<std::array<float, 3>> point;
    if (const std::optional<GatherNode> node = gather_node(memory))
        point = std::array<float, 3>{node->x, node->y, node->z};
    else if (const std::optional<CarveBody> body = carve_body(memory))
        point = std::array<float, 3>{body->x, body->y, body->z};
    if (!point) return std::nullopt;
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
    const std::optional<PicturePoint> at = project_world(camera, (*point)[0], (*point)[1] + kGatherLift, (*point)[2]);
    if (!at) return std::nullopt;
    return std::array<float, 2>{at->x, at->y};
}

std::uint32_t GatherRepeat::apply(std::uint32_t buttons, bool ready, std::uint32_t button) {
    constexpr std::uint32_t kLead = 0x0100u; // L, held with ○ or □ for the item bar
    constexpr unsigned kPulse = kGatherPulseDown + kGatherPulseUp;
    centres_ = false;
    // L with ○ or □ before a hold starts scrolls the item bar. Once the hold
    // has started, L is another button and must not open that bar.
    if ((buttons & kLead) != 0u && !holding_ && !latched_) return buttons;
    // △+○ and the other chords are attacks, not a gather, even beside a point
    // or a body. The pad goes through, and a hold already running ends.
    if ((buttons & kGatherChord) != 0u) {
        holding_ = false;
        latched_ = false;
        input_down_ = (buttons & button_) != 0u;
        input_button_ = button_;
        return buttons;
    }
    // A hold already in progress keeps its own button. A new hold takes the
    // caller's, so mining can pulse □ while carving pulses ○.
    if (holding_) button = button_;
    else button_ = button;
    const bool down = (buttons & button) != 0u;
    // A second button can drop the gather button from the resolved pad for a
    // few reads. The hold continues until the pad is clear, and the game is
    // shown the gather pulse rather than that button.
    if (down) latched_ = true;
    else if ((buttons & ~button) == 0u) latched_ = false;
    if (!ready || !latched_) {
        if (!ready) latched_ = down;
        holding_ = false;
        input_down_ = down;
        input_button_ = button;
        return buttons;
    }
    // The gather button already down when the hold begins has been seen by the
    // game, so the pulse starts on its up part and the next read down is a new
    // press. Only the same button counts: a held ○ must not skip the first □.
    if (!holding_) {
        holding_ = true;
        step_ = (input_down_ && input_button_ == button) ? kGatherPulseDown : 0u;
    }
    input_down_ = true;
    input_button_ = button;
    const bool show = step_ < kGatherPulseDown;
    if (step_ == 0u && trace_on()) std::cout << "[gather] hold" << std::endl;
    step_ = (step_ + 1u) % kPulse;
    // The game sees the gather button as this pulse wants it, and nothing else.
    // The bit is forced on: the resolved pad may not contain it while another
    // button is down. A stick push is stopped by the caller.
    centres_ = true;
    return show ? button : 0u;
}

namespace {
bool centres_stick = false;
} // namespace

bool gather_hold_centres_stick() { return centres_stick; }

std::uint32_t gather_hold(std::uint32_t buttons, const psprecomp::GuestMemory &memory) {
    static GatherRepeat repeat;
    centres_stick = false;
    if (!settings::current().gather_hold) {
        repeat.reset();
        return buttons;
    }
    constexpr std::uint32_t kCircle = 0x2000u; // ○, B: plants and carving
    constexpr std::uint32_t kSquare = 0x8000u; // □, X: mining and a bug net
    constexpr std::uint32_t kLead = 0x0100u;
    bool circle_ready = false;
    bool square_ready = false;
    const bool look = ((buttons & kLead) == 0u && ((buttons & kCircle) != 0u || (buttons & kSquare) != 0u)) ||
        repeat.underway();
    if (look) {
        if (const std::optional<GatherNode> node = gather_node(memory)) {
            if (gather_uses_square(node->kind)) square_ready = true;
            else circle_ready = true;
        }
        if (carve_in_reach(memory)) circle_ready = true;
    }
    // Even away from a point the held gather button is still tracked, so walking
    // onto one with it already down starts on the up part of the pulse.
    std::uint32_t button = kCircle;
    bool ready = false;
    if (repeat.underway()) {
        button = repeat.button();
        ready = button == kSquare ? square_ready : circle_ready;
    } else if ((buttons & kLead) == 0u && (buttons & kCircle) != 0u && circle_ready) {
        button = kCircle;
        ready = true;
    } else if ((buttons & kLead) == 0u && (buttons & kSquare) != 0u && square_ready) {
        button = kSquare;
        ready = true;
    } else if ((buttons & kLead) == 0u && (buttons & kSquare) != 0u) {
        button = kSquare;
    } else if ((buttons & kLead) == 0u && (buttons & kCircle) != 0u) {
        button = kCircle;
    } else if (!repeat.underway()) {
        (void)repeat.apply(buttons, false, repeat.button());
        return buttons;
    }
    const std::uint32_t shown = repeat.apply(buttons, ready, button);
    centres_stick = repeat.centres_stick();
    return shown;
}

} // namespace mhfu::game
