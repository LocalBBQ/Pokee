#pragma once

// Damage numbers in ad hoc play (game/damage_numbers.hpp): each hit a console
// applies, sent to the other players so theirs show it as soon as it lands.
//
// Traced with MHFU_TRACE_HITS on two consoles in one quest (ULUS-10391):
// - The hit is computed (game_sub 0x09C43558) and applied (0x09AC7A08) on one
//   console only: a large monster's on the console in charge of it, whoever
//   attacked; a small monster's on the attacker's. The others never see the
//   breakdown.
// - They receive the new health 16 ms to more than 5 s later, often several
//   hits in one drop (50, 26 and 66 arrived as one 142), so a number from
//   the drop is late, plain and merged.
// - The quest's entities are at the same addresses on every console, so a
//   monster and an attacker are named by their address. They change from one
//   quest to the next.
//
// - A monster's hit on a small monster is worked out on every console, each
//   on its own copy of it (whose health was 54 on one and 33 on the other),
//   so sharing it showed it twice. Only a hunter's hit, and a poison tick,
//   is shared and taken in; another monster's reaches the others as before.
//
// - A small monster is not kept the same on every console: a hit landed on
//   one console's copy did not move the other's health (55 before and after)
//   and showed on the other screen where that copy was not. Only the hits on
//   a large monster (camera::hunt_monsters, the hunt icon's) are shared.
//
// So the console that applies a hit sends it, as a SharedHit, once a game
// frame in one datagram broadcast on kSyncPort through the ad hoc relay. A
// console that receives it shows the number at once (receive_shared_hits)
// and counts it against the health drop that follows, so the drop shows only
// what no hit it received explains, such as a hit from a player without
// this. A drop from the network waits kDropHold for the hits that explain it.
// The game's own sockets are datagram ports 10000 and 20001 and stream port
// 20001, and the client gives a socket on port 0 one from 0x8000 up, so
// kSyncPort is none of the game's. A player without this has no socket
// there, and the relay drops what is sent to it.
//
// A console sends while it plays a quest in an ad hoc group, with Damage
// numbers on or off: a player who shows none still lets the others see their
// hits. MHFU_DAMAGE_NUMBERS_SYNC=0 neither sends nor receives.
//
// Wire format, little-endian, no padding: "PKDN", u8 version (kSyncVersion),
// u8 count (1 to kMaxSharedPerDatagram), then count records of
// kSharedHitSize bytes:
//   u32 monster, u32 attacker (0 when not known), u16 amount, u16 element
//   share, i16 the monster's most health, u8 species, u8 element (Element),
//   u8 fill (Fill), u8 affinity (Affinity), u8 flags (1 poison, 2 a point
//   follows), u8 0, f32 x, f32 y, f32 z (zero without a point).

#include "game/damage_numbers.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace mhfu::game {

inline constexpr std::uint16_t kSyncPort = 30000u;
inline constexpr std::uint8_t kSyncVersion = 1u;
inline constexpr std::size_t kSyncHeaderSize = 6u;
inline constexpr std::size_t kSharedHitSize = 32u;
inline constexpr std::size_t kMaxSharedPerDatagram = 32u;
// How long a health drop from the network waits for the hits that explain it.
inline constexpr float kDropHold = 0.5f;
// How long a received hit can explain a drop: drops were seen 5.4 s late.
inline constexpr float kCreditSeconds = 15.0f;

// One hit as the console that applied it shows it.
struct SharedHit {
    std::uint32_t monster{};
    std::uint32_t attacker{};
    int amount{};
    int element{};
    int most{};
    std::uint8_t species{};
    Element kind{};
    Fill fill{Fill::Unknown};
    Affinity affinity{Affinity::None};
    bool poison{};
    std::optional<WorldPoint> at; // where it landed; none: on the body
};

// The datagrams for `hits`, kMaxSharedPerDatagram to each; none for none.
[[nodiscard]] std::vector<std::string> encode_hits(const std::vector<SharedHit> &hits);
// The hits in a datagram, or nothing when it is not one of ours, is of
// another version, or holds a value no hit has.
[[nodiscard]] std::optional<std::vector<SharedHit>> decode_hits(std::string_view datagram);

// MHFU_DAMAGE_NUMBERS_SYNC: unset or anything but "0" is on.
[[nodiscard]] bool parse_sync(const char *text) noexcept;
[[nodiscard]] bool sync_enabled() noexcept;

// The rest is kept with the numbers (damage_numbers.cpp).

// The large monsters of the quest, whose hits are shared: the host passes
// the hunt icon's (camera::hunt_monsters). Until it does, nothing is.
using LargeMonsters = std::vector<std::uint32_t> (*)(const psprecomp::GuestMemory &memory);
void set_large_monsters(LargeMonsters large) noexcept;

// What received hits took off a monster that its health has not shown yet.
struct Credit {
    std::uint32_t object{};
    int amount{};
    float age{};
};
// A health drop from the network, waiting for the hits that explain it.
struct HeldDrop {
    Number number;
    int health{}; // the monster's health after it
    float age{};
};
// One frame of `seconds` for the held drops: each takes what `credits`
// explain off its amount, oldest credit first, and a drop that leaves its
// monster at 0 health or less clears that monster's credits. Returns the
// drops whose rest is to be shown: at once without `sharing`, otherwise once
// kDropHold old; a drop explained whole is gone. `absorbed`, when given,
// gets each drop that something was taken off, with that amount.
[[nodiscard]] std::vector<HeldDrop> settle_drops(std::vector<HeldDrop> &held, std::vector<Credit> &credits,
    float seconds, bool sharing, std::vector<HeldDrop> *absorbed = nullptr);

// While sharing, the hit watch stays installed with Damage numbers off, each
// hit this console applies is kept for take_shared_hits, and a health drop
// from the network waits kDropHold. Set once a frame, before
// damage_numbers_frame; turning it off forgets what it kept.
void set_sharing(bool sharing) noexcept;
[[nodiscard]] bool sharing() noexcept;
// The hits applied since the last call, oldest first.
[[nodiscard]] std::vector<SharedHit> take_shared_hits();
// Hits another console applied. Each becomes a number now, on a monster of
// this quest with the same most health and species (others are left out),
// and is counted against the next drops in that monster's health.
void receive_shared_hits(const psprecomp::GuestMemory &memory, const std::vector<SharedHit> &hits);

} // namespace mhfu::game
