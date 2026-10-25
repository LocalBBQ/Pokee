#include "game/damage_sync.hpp"

#include "adhoc/protocol.hpp"

#include <algorithm>
#include <bit>
#include <cmath>
#include <cstdlib>
#include <cstring>

namespace mhfu::game {
namespace {

namespace wire = adhoc::wire;

constexpr char kMagic[4] = {'P', 'K', 'D', 'N'};
constexpr std::uint8_t kPoisonFlag = 1u;
constexpr std::uint8_t kPointFlag = 2u;

void put_float(std::string &out, float value) {
    wire::put32(out, std::bit_cast<std::uint32_t>(value));
}

float get_float(const char *data) {
    return std::bit_cast<float>(wire::get32(data));
}

void put_hit(std::string &out, const SharedHit &hit) {
    wire::put32(out, hit.monster);
    wire::put32(out, hit.attacker);
    wire::put16(out, static_cast<std::uint16_t>(std::clamp(hit.amount, 0, 0xFFFF)));
    wire::put16(out, static_cast<std::uint16_t>(std::clamp(hit.element, 0, 0xFFFF)));
    wire::put16(out, static_cast<std::uint16_t>(static_cast<std::int16_t>(hit.most)));
    wire::put8(out, hit.species);
    wire::put8(out, static_cast<std::uint8_t>(hit.kind));
    wire::put8(out, static_cast<std::uint8_t>(hit.fill));
    wire::put8(out, static_cast<std::uint8_t>(hit.affinity));
    wire::put8(out, static_cast<std::uint8_t>((hit.poison ? kPoisonFlag : 0u) | (hit.at ? kPointFlag : 0u)));
    wire::put8(out, 0u);
    const WorldPoint at = hit.at.value_or(WorldPoint{});
    put_float(out, at.x);
    put_float(out, at.y);
    put_float(out, at.z);
}

std::optional<SharedHit> get_hit(const char *data) {
    SharedHit hit;
    hit.monster = wire::get32(data);
    hit.attacker = wire::get32(data + 4);
    hit.amount = wire::get16(data + 8);
    hit.element = wire::get16(data + 10);
    hit.most = static_cast<std::int16_t>(wire::get16(data + 12));
    hit.species = static_cast<std::uint8_t>(data[14]);
    const auto kind = static_cast<std::uint8_t>(data[15]);
    const auto fill = static_cast<std::uint8_t>(data[16]);
    const auto affinity = static_cast<std::uint8_t>(data[17]);
    const auto flags = static_cast<std::uint8_t>(data[18]);
    if (kind > static_cast<std::uint8_t>(Element::Dragon) || fill > static_cast<std::uint8_t>(Fill::Ordinary) ||
        affinity > static_cast<std::uint8_t>(Affinity::Negative) || (flags & ~(kPoisonFlag | kPointFlag)) != 0u ||
        data[19] != 0 || hit.amount <= 0 || hit.element > hit.amount || hit.most <= 0)
        return std::nullopt;
    hit.kind = static_cast<Element>(kind);
    hit.fill = static_cast<Fill>(fill);
    hit.affinity = static_cast<Affinity>(affinity);
    hit.poison = (flags & kPoisonFlag) != 0u;
    if ((flags & kPointFlag) != 0u) {
        const WorldPoint at{get_float(data + 20), get_float(data + 24), get_float(data + 28)};
        if (!std::isfinite(at.x) || !std::isfinite(at.y) || !std::isfinite(at.z)) return std::nullopt;
        hit.at = at;
    }
    return hit;
}

} // namespace

std::vector<std::string> encode_hits(const std::vector<SharedHit> &hits) {
    std::vector<std::string> datagrams;
    for (std::size_t first = 0; first < hits.size(); first += kMaxSharedPerDatagram) {
        const std::size_t count = std::min(kMaxSharedPerDatagram, hits.size() - first);
        std::string out(kMagic, sizeof(kMagic));
        wire::put8(out, kSyncVersion);
        wire::put8(out, static_cast<std::uint8_t>(count));
        for (std::size_t i = 0; i < count; ++i) put_hit(out, hits[first + i]);
        datagrams.push_back(std::move(out));
    }
    return datagrams;
}

std::optional<std::vector<SharedHit>> decode_hits(std::string_view datagram) {
    if (datagram.size() < kSyncHeaderSize || std::memcmp(datagram.data(), kMagic, sizeof(kMagic)) != 0 ||
        static_cast<std::uint8_t>(datagram[4]) != kSyncVersion)
        return std::nullopt;
    const std::size_t count = static_cast<std::uint8_t>(datagram[5]);
    if (count == 0u || count > kMaxSharedPerDatagram || datagram.size() != kSyncHeaderSize + count * kSharedHitSize)
        return std::nullopt;
    std::vector<SharedHit> hits;
    for (std::size_t i = 0; i < count; ++i) {
        const std::optional<SharedHit> hit = get_hit(datagram.data() + kSyncHeaderSize + i * kSharedHitSize);
        if (!hit) return std::nullopt;
        hits.push_back(*hit);
    }
    return hits;
}

bool parse_sync(const char *text) noexcept {
    return text == nullptr || std::strcmp(text, "0") != 0;
}

bool sync_enabled() noexcept {
    static const bool enabled = parse_sync(std::getenv("MHFU_DAMAGE_NUMBERS_SYNC"));
    return enabled;
}

} // namespace mhfu::game
