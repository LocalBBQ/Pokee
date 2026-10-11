#include "game/equipment_models.hpp"

#include "game/guest_ram.hpp"

namespace mhfu::game {

const char *sex_name(Sex sex) { return sex == Sex::Female ? "female" : "male"; }

std::uint32_t armor_data(std::uint8_t kind) {
    switch (kind) {
    case 0: return kChestData;
    case 1: return kArmsData;
    case 2: return kWaistData;
    case 3: return kLegsData;
    case 4: return kHeadData;
    default: return 0u;
    }
}

std::optional<std::uint16_t> armor_model(const Ram &ram, std::uint8_t kind, std::uint16_t id, Sex sex) {
    const std::uint32_t data = armor_data(kind);
    const std::uint32_t at = data + static_cast<std::uint32_t>(id) * kArmorRecord;
    if (data == 0u || !ram.contains(at, kArmorRecord)) return std::nullopt;
    return ram.load16(at + (sex == Sex::Female ? 2u : 0u));
}

bool wearable_by(const Ram &ram, std::uint8_t kind, std::uint16_t id, Sex sex) {
    const std::uint32_t data = armor_data(kind);
    const std::uint32_t at = data + static_cast<std::uint32_t>(id) * kArmorRecord;
    if (data == 0u || !ram.contains(at, kArmorRecord)) return false;
    return (ram.load8(at + 4u) & (1u << static_cast<unsigned>(sex))) != 0u;
}

} // namespace mhfu::game
