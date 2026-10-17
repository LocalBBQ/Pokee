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

// MHFU_TRACE_GATHER: the item a node's mark shows and its icon, once a node.
void trace_art(const psprecomp::GuestMemory &memory, const GatherNode &node, const GatherArt &art) {
    if (!trace_on()) return;
    static std::uint32_t last = 0u;
    if (node.address == last) return;
    last = node.address;
    if (art.kind != GatherArt::Kind::Item) {
        std::cout << "[gather] node id " << node.id << " shows no item" << std::endl;
        return;
    }
    std::cout << "[gather] node id " << node.id << " shows item " << art.item;
    if (const std::optional<ItemArt> icon = item_art(memory, art.item))
        std::cout << " at " << icon->u << "," << icon->v << " " << icon->width << "x" << icon->height << " tint "
                  << static_cast<int>(icon->red) << "," << static_cast<int>(icon->green) << ","
                  << static_cast<int>(icon->blue);
    else
        std::cout << ", which has no icon";
    std::cout << std::endl;
    // The whole list: weight, item, and the item's icon and tint numbers.
    if (const std::optional<std::vector<GatherDrop>> drops = gather_drops(memory, node.id)) {
        std::cout << "[gather]   list:";
        for (const GatherDrop &drop : *drops) {
            std::cout << ' ' << drop.weight << 'x' << drop.item;
            if (const std::optional<std::array<std::uint8_t, 2>> look = item_icon_and_tint(memory, drop.item))
                std::cout << "(icon " << static_cast<int>((*look)[0]) << " tint " << static_cast<int>((*look)[1])
                          << ')';
        }
        std::cout << std::endl;
    }
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
constexpr std::uint32_t kSubSlot = 0x09C19100u;
constexpr std::uint32_t kOverlayMagic = 0x336F574Du; // "MWo3"
constexpr char kQuestOverlay[] = "game_task.ovl";
constexpr char kVillageOverlay[] = "lobby_task.ovl";
constexpr char kSubOverlay[] = "game_sub.ovl";

namespace {

template <std::size_t N>
bool overlay_loaded(const psprecomp::GuestMemory &memory, std::uint32_t slot, const char (&name)[N]) {
    if (memory.raw_pointer(slot, 64u) == nullptr || memory.load32(slot) != kOverlayMagic ||
        memory.load32(slot + 8u) != slot)
        return false;
    for (std::size_t i = 0; i < N; ++i)
        if (memory.load8(slot + 32u + static_cast<std::uint32_t>(i)) != static_cast<std::uint8_t>(name[i]))
            return false;
    return true;
}

// Every word mapped and matching. A mapped word that differs is logged once
// per `logged` flag.
bool words_match(const psprecomp::GuestMemory &memory, std::span<const GatherWord> words, bool &logged) {
    bool ready = true;
    bool differs = false;
    for (const GatherWord &word : words) {
        if (memory.raw_pointer(word.address, 4u) == nullptr) {
            ready = false;
            continue;
        }
        if (memory.load32(word.address) != word.instruction) differs = true;
    }
    if (differs && !logged) {
        logged = true;
        for (const GatherWord &word : words)
            if (memory.raw_pointer(word.address, 4u) != nullptr && memory.load32(word.address) != word.instruction)
                std::cout << "[gather] game code differs at 0x" << std::hex << word.address << ": 0x"
                          << memory.load32(word.address) << ", expected 0x" << word.instruction << std::dec
                          << std::endl;
    }
    return ready && !differs;
}

} // namespace

bool gather_code_matches(const psprecomp::GuestMemory &memory) {
    if (!overlay_loaded(memory, kTaskSlot, kQuestOverlay)) return false;
    return words_match(memory, kWords, code_logged());
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

// MHFU_TRACE_GATHER: the record of a body that comes into reach, past the
// position and radius, to find which of its words names the monster.
void trace_carve(const psprecomp::GuestMemory &memory, std::uint32_t record) {
    if (!trace_on()) return;
    static std::uint32_t last = 0u;
    if (record == last) return;
    last = record;
    std::cout << "[gather] carve 0x" << std::hex << record << " +16:";
    for (std::uint32_t offset = 16u; offset < kCarveStride; offset += 2u)
        std::cout << ' ' << memory.load16(record + offset);
    std::cout << std::dec << std::endl;
}

// game_task 0x09AA6B18, the spot search, and the item handler's bait case
// that asks it.
constexpr GatherWord kSpotWords[] = {
    {0x09AA6B34u, 0x3C0209A5u}, // lui v0,0x09A5
    {0x09AA6B38u, 0x8C42F1A0u}, // lw v0,-3680(v0): the pointer at 0x09A4F1A0
    {0x09AA6B44u, 0x8C440000u}, // lw a0,0(v0): the stage object
    {0x09AA6B4Cu, 0x8C990000u}, // lw t9,0(a0): its vtable
    {0x09AA6B50u, 0x8F390028u}, // lw t9,40(t9): method +40, the spot table
    {0x09AA6B5Cu, 0x8C500010u}, // lw s0,16(v0): the spot list
    {0x09AA6B88u, 0x8043003Eu}, // lb v1,62(v0): the spot count
    {0x09AA6B94u, 0xC6840204u}, // lwc1 f4,516(s4): the hunter's height
    {0x09AA6B98u, 0x3C024248u}, // lui v0,0x4248: 50 above and below the spot
    {0x09AA6C0Cu, 0xC6010010u}, // lwc1 f1,16(s0): the spot's radius
    {0x09AA6C4Cu, 0x96020002u}, // lhu v0,2(s0): the type it returns
    {0x09AA6C5Cu, 0x26100018u}, // addiu s0,s0,24: 24 bytes a spot
    {0x09AA7ABCu, 0x2402008Fu}, // addiu v0,zero,143: the last bait
    {0x09AA7B10u, 0x24020088u}, // addiu v0,zero,136: the first bait
    {0x09AA7FA4u, 0x0E6A9AC6u}, // jal 0x09AA6B18: a bait asks the spot search
    {0x09AA7FB0u, 0x24020017u}, // addiu v0,zero,23: and fishes at type 23
};

constexpr std::uint32_t kSpotMethod = 40u;
constexpr std::uint32_t kSpotList = 16u;
constexpr std::uint32_t kSpotCount = 62u;
constexpr std::uint32_t kSpotStride = 24u;
constexpr std::uint32_t kSpotType = 2u;
constexpr std::uint32_t kSpotRadius = 16u;
constexpr float kSpotReach = 50.0f;

bool &spot_logged() {
    static bool logged = false;
    return logged;
}

// MHFU_TRACE_GATHER: the fishing spot in reach, once each time it changes.
void trace_fishing(const std::optional<FishingSpot> &spot) {
    if (!trace_on()) return;
    static std::uint32_t last = 1u;
    const std::uint32_t address = spot ? spot->address : 0u;
    if (address == last) return;
    last = address;
    if (!spot) {
        std::cout << "[gather] fishing none" << std::endl;
        return;
    }
    std::cout << "[gather] fishing 0x" << std::hex << spot->address << std::dec << " at " << spot->x << ","
              << spot->y << "," << spot->z << std::endl;
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
            trace_carve(memory, record);
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

std::span<const GatherWord> fishing_checked_words() {
    return kSpotWords;
}

std::optional<std::uint32_t> spot_table(const psprecomp::GuestMemory &memory) {
    if (!in_ram(kCarveTable, 4u, memory)) return std::nullopt;
    const std::uint32_t quest = memory.load32(kCarveTable);
    if (!in_ram(quest, 4u, memory)) return std::nullopt;
    const std::uint32_t stage = memory.load32(quest);
    if (!in_ram(stage, 4u, memory)) return std::nullopt;
    const std::uint32_t vtable = memory.load32(stage);
    if (!in_ram(vtable + kSpotMethod, 4u, memory)) return std::nullopt;
    const std::uint32_t method = memory.load32(vtable + kSpotMethod);
    if (!in_ram(method, 12u, memory)) return std::nullopt;
    // lui v0,hi; jr ra; addiu v0,v0,lo
    const std::uint32_t high = memory.load32(method);
    const std::uint32_t low = memory.load32(method + 8u);
    if ((high & 0xFFFF0000u) != 0x3C020000u || memory.load32(method + 4u) != 0x03E00008u ||
        (low & 0xFFFF0000u) != 0x24420000u)
        return std::nullopt;
    const std::uint32_t table = (high << 16u) + static_cast<std::uint32_t>(static_cast<std::int16_t>(low & 0xFFFFu));
    if (!in_ram(table, kSpotCount + 1u, memory)) return std::nullopt;
    return table;
}

std::optional<GatherSpot> spot_from_table(const psprecomp::GuestMemory &memory, std::uint32_t table, float hunter_x,
    float hunter_y, float hunter_z) {
    if (!in_ram(table, kSpotCount + 1u, memory)) return std::nullopt;
    std::uint32_t spot = memory.load32(table + kSpotList);
    const auto count = static_cast<std::int8_t>(memory.load8(table + kSpotCount));
    if (spot == 0u || count <= 0) return std::nullopt;
    for (std::int8_t n = 0; n < count; ++n, spot += kSpotStride) {
        if (!in_ram(spot, kSpotStride, memory)) return std::nullopt;
        const float x = load_float(memory, spot + 4u);
        const float y = load_float(memory, spot + 8u);
        const float z = load_float(memory, spot + 12u);
        // 0x09AA6BB4: below the spot minus 50, or a height that is not a
        // number, is skipped; so is the spot plus 50 and above.
        if (!(hunter_y >= y - kSpotReach) || !(hunter_y < y + kSpotReach)) continue;
        const float dx = hunter_x - x;
        const float dz = hunter_z - z;
        if (!(std::sqrt(dx * dx + dz * dz) <= load_float(memory, spot + kSpotRadius))) continue;
        return GatherSpot{spot, memory.load16(spot + kSpotType), x, y, z};
    }
    return std::nullopt;
}

namespace {

std::optional<FishingSpot> read_fishing(const psprecomp::GuestMemory &memory) {
    if (!gather_code_matches(memory) || !words_match(memory, kSpotWords, spot_logged())) return std::nullopt;
    const std::optional<std::uint32_t> hunter = followed_hunter(memory);
    if (!hunter || !in_ram(*hunter + kHunterPosition, 12u, memory)) return std::nullopt;
    const std::optional<std::uint32_t> table = spot_table(memory);
    if (!table) return std::nullopt;
    const std::optional<GatherSpot> spot = spot_from_table(memory, *table, load_float(memory, *hunter + kHunterPosition),
        load_float(memory, *hunter + kHunterPosition + 4u), load_float(memory, *hunter + kHunterPosition + 8u));
    // The item handler takes the search's first spot and fishes only at 23.
    if (!spot || spot->type != kFishingSpotType) return std::nullopt;
    return FishingSpot{spot->address, spot->x, spot->y, spot->z};
}

} // namespace

std::optional<FishingSpot> fishing_spot(const psprecomp::GuestMemory &memory) {
    const std::optional<FishingSpot> spot = read_fishing(memory);
    trace_fishing(spot);
    return spot;
}

namespace {

// lobby_task 0x09AAF290 keeps the point in reach, 0x09AAD350 starts it on □,
// and game_sub counts the uses left on a farm spot.
constexpr GatherWord kFarmWords[] = {
    {0x09AAF3ACu, 0xAE510554u}, // sw s1,1364(s2): the point in reach
    {0x09AAD4CCu, 0x962401E6u}, // lhu a0,486(s1): the local hunter only
    {0x09AAD4F8u, 0x3C0309B2u}, // lui v1,0x09B2: the village state object
    {0x09AAD4FCu, 0x8C702E70u}, // lw s0,11888(v1): the pointer at 0x09B22E70
    {0x09AAD7B4u, 0x8E040000u}, // lw a0,0(s0): its state
    {0x09AAD7CCu, 0x8603001Au}, // lh v1,26(s0): and the halfword at +26
    {0x09AADA40u, 0x8E230484u}, // lw v1,1156(s1): the buttons new this read
    {0x09AADA44u, 0x30638000u}, // andi v1,v1,0x8000: □ starts the point
    {0x09AADA54u, 0x96430002u}, // lhu v1,2(s2): the point's type
    {0x09AADA64u, 0x3C0409B1u}, // lui a0,0x09B1
    {0x09AADA6Cu, 0x2484F300u}, // addiu a0,a0,-3328: the table at 0x09B0F300
    {0x09B0F33Cu, 0x09AAE08Cu}, // type 15: mining
    {0x09B0F340u, 0x09AAE118u}, // type 16: bugs
    {0x09AAE094u, 0x0E70A57Eu}, // jal 0x09C295F8: mining uses left
    {0x09AAE120u, 0x0E70A592u}, // jal 0x09C29648: bug uses left
    {0x09C29604u, 0x3421A238u}, // ori at,at,0xA238: the farm in GameSys
    {0x09C2960Cu, 0x90620002u}, // lbu v0,2(v1): the mining spot count
    {0x09C29624u, 0x94420008u}, // lhu v0,8(v0): its uses left
    {0x09C29654u, 0x3421A238u}, // ori at,at,0xA238
    {0x09C2965Cu, 0x90620003u}, // lbu v0,3(v1): the bug spot count
    {0x09C29674u, 0x94420030u}, // lhu v0,48(v0): its uses left
    {0x08886E80u, 0x3C03089Du}, // lui v1,0x89d: the base of the hunter list
    {0x08886EF8u, 0x8E130A84u}, // lw s3,2692(s0): the hunter the camera follows
};

constexpr std::uint32_t kVillageState = 0x09B22E70u;
constexpr std::uint32_t kVillageBlock = 26u;
constexpr std::uint32_t kPointInReach = 1364u;
constexpr std::uint32_t kPointType = 2u;
constexpr std::uint32_t kPointIndex = 22u;
constexpr std::uint32_t kHunterSlot = 486u;
constexpr std::uint32_t kLocalSlot = 40u;
constexpr std::uint32_t kFarm = 0x6A238u;
constexpr std::uint32_t kFarmStride = 8u;

bool &farm_logged() {
    static bool logged = false;
    return logged;
}

// MHFU_TRACE_GATHER: the farm spot in reach, once each time it changes.
void trace_farm(const std::optional<FarmPoint> &point) {
    if (!trace_on()) return;
    static std::uint32_t last = 1u;
    static std::uint16_t last_uses = 0u;
    const std::uint32_t address = point ? point->address : 0u;
    const std::uint16_t uses = point ? point->uses : 0u;
    if (address == last && uses == last_uses) return;
    last = address;
    last_uses = uses;
    if (!point) {
        std::cout << "[gather] farm none" << std::endl;
        return;
    }
    std::cout << "[gather] farm 0x" << std::hex << point->address << std::dec << " type " << point->type << " index "
              << point->index << " uses " << point->uses << std::endl;
}

std::optional<FarmPoint> read_farm(const psprecomp::GuestMemory &memory) {
    if (!farm_code_matches(memory) || !in_ram(kHunterList, 4u, memory) || !in_ram(kGameSysPointer, 4u, memory) ||
        !in_ram(kVillageState, 4u, memory))
        return std::nullopt;
    const std::uint32_t list = memory.load32(kHunterList);
    if (!in_ram(list + kFollowedHunter, 4u, memory)) return std::nullopt;
    const std::uint32_t hunter = memory.load32(list + kFollowedHunter);
    const std::uint32_t gamesys = memory.load32(kGameSysPointer);
    const std::uint32_t state = memory.load32(kVillageState);
    if (!in_ram(hunter, kPointInReach + 4u, memory) || !in_ram(gamesys + kLocalSlot, 1u, memory) ||
        !in_ram(gamesys + kFarm, 4u, memory) || !in_ram(state, kVillageBlock + 2u, memory))
        return std::nullopt;
    if (memory.load16(hunter + kHunterSlot) != memory.load8(gamesys + kLocalSlot)) return std::nullopt;
    // Another state is a menu, a talk or a gather already running: the
    // handler does not start a point then.
    if (memory.load32(state) != 0u || memory.load16(state + kVillageBlock) != 0u) return std::nullopt;
    const std::uint32_t point = memory.load32(hunter + kPointInReach);
    if (!in_ram(point, kPointIndex + 2u, memory)) return std::nullopt;
    const std::uint16_t type = memory.load16(point + kPointType);
    if (type != kFarmMining && type != kFarmBugs) return std::nullopt;
    const std::uint16_t index = memory.load16(point + kPointIndex);
    const std::uint32_t farm = gamesys + kFarm;
    // 0x09C295F8 and 0x09C29648: an index past the count has no uses.
    const std::uint8_t count = memory.load8(farm + (type == kFarmMining ? 2u : 3u));
    if (index > count) return std::nullopt;
    const std::uint32_t uses_at = farm + (type == kFarmMining ? 8u : 48u) + index * kFarmStride;
    if (!in_ram(uses_at, 2u, memory)) return std::nullopt;
    const std::uint16_t uses = memory.load16(uses_at);
    if (uses == 0u) return std::nullopt;
    return FarmPoint{point, type, index, uses};
}

} // namespace

std::span<const GatherWord> farm_checked_words() {
    return kFarmWords;
}

bool farm_code_matches(const psprecomp::GuestMemory &memory) {
    if (!overlay_loaded(memory, kTaskSlot, kVillageOverlay) || !overlay_loaded(memory, kSubSlot, kSubOverlay))
        return false;
    return words_match(memory, kFarmWords, farm_logged());
}

std::optional<FarmPoint> farm_point(const psprecomp::GuestMemory &memory) {
    const std::optional<FarmPoint> point = read_farm(memory);
    trace_farm(point);
    return point;
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

GatherGlyph gather_glyph(std::uint16_t kind) noexcept {
    if (kind == 3u) return GatherGlyph::Mining;
    if (kind == 4u) return GatherGlyph::BugNet;
    return GatherGlyph::Plant;
}

std::optional<GatherTarget> gather_target(const psprecomp::GuestMemory &memory) {
    // The circle handler tries the gather list first and carves only when it
    // misses, so a node in range takes the mark. Fishing is the item action,
    // so a fishing spot is marked only when ○ has nothing to do there.
    if (const std::optional<GatherNode> node = gather_node(memory))
        {
        GatherArt art;
        if (gather_art_code_matches(memory)) art = gather_art_for(memory, node->kind, node->id);
        trace_art(memory, *node, art);
        return GatherTarget{node->address, node->x, node->y, node->z, gather_glyph(node->kind), art};
    }
    if (const std::optional<CarveBody> body = carve_body(memory))
        return GatherTarget{body->address, body->x, body->y, body->z, GatherGlyph::Carve};
    if (const std::optional<FishingSpot> spot = fishing_spot(memory)) {
        GatherArt art;
        if (gather_art_code_matches(memory)) art = GatherArt::of_item(kBaitItem);
        return GatherTarget{spot->address, spot->x, spot->y, spot->z, GatherGlyph::Fishing, art};
    }
    return std::nullopt;
}

void GatherMotion::step(const std::optional<GatherTarget> &target) {
    if (!target) {
        alpha_ = std::max(0.0f, alpha_ - kFadeStep);
        if (alpha_ <= 0.0f) live_ = false;
        return;
    }
    const float dx = target->x - x_;
    const float dy = target->y - y_;
    const float dz = target->z - z_;
    // Nothing on screen, or too far to glide: the mark starts on the target.
    if (!live_ || dx * dx + dy * dy + dz * dz > kJump * kJump) {
        x_ = target->x;
        y_ = target->y;
        z_ = target->z;
        if (!live_) alpha_ = 0.0f;
    } else if (dx * dx + dy * dy + dz * dz <= kArrive * kArrive) {
        x_ = target->x;
        y_ = target->y;
        z_ = target->z;
    } else {
        x_ += dx * kGlide;
        y_ += dy * kGlide;
        z_ += dz * kGlide;
    }
    live_ = true;
    glyph_ = target->glyph;
    art_ = target->art;
    alpha_ = std::min(1.0f, alpha_ + kFadeStep);
}

std::optional<GatherMotion::Shown> GatherMotion::shown() const {
    if (!live_ || alpha_ <= 0.0f) return std::nullopt;
    return Shown{x_, y_, z_, alpha_, glyph_, art_};
}

std::optional<GatherMark> gather_mark(
    const psprecomp::GuestMemory &memory, const std::optional<GatherPose> &pose, GatherMotion *motion) {
    Noted &slot = noted();
    const GatherPicture picture = slot.picture;
    const std::uint32_t vertices = std::exchange(slot.vertices, 0u);
    slot.picture = {};
    if (!settings::current().gather_icon || vertices == 0u || !pose) {
        if (motion != nullptr) motion->reset();
        return std::nullopt;
    }
    const std::optional<GatherTarget> target = gather_target(memory);
    std::optional<std::array<float, 3>> point;
    GatherMark mark;
    if (motion != nullptr) {
        motion->step(target);
        const std::optional<GatherMotion::Shown> shown = motion->shown();
        if (!shown) {
            motion->set_last_at(std::nullopt);
            return std::nullopt;
        }
        point = std::array<float, 3>{shown->x, shown->y, shown->z};
        mark.alpha = shown->alpha;
        mark.glyph = shown->glyph;
        mark.art = shown->art;
    } else if (target) {
        point = std::array<float, 3>{target->x, target->y, target->z};
        mark.glyph = target->glyph;
        mark.art = target->art;
    }
    if (!point) return std::nullopt;
    mark.world = *point;
    const auto project = [&]() -> std::optional<std::array<float, 2>> {
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
        const std::optional<PicturePoint> at =
            project_world(camera, (*point)[0], (*point)[1] + kGatherLift, (*point)[2]);
        if (!at) return std::nullopt;
        return std::array<float, 2>{at->x, at->y};
    };
    const std::optional<std::array<float, 2>> at = project();
    if (motion != nullptr) {
        mark.older = motion->last_at();
        motion->set_last_at(at);
    }
    if (!at) return std::nullopt;
    mark.at = *at;
    return mark;
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
        // The farm's mining outcrops and bug thickets, in the village.
        if (farm_point(memory)) square_ready = true;
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
