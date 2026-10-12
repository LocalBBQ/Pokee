#include "camera/hunt_icon.hpp"

#include "gpu/texture_decode.hpp"

#include <vector>

namespace mhfu::camera {
namespace {

// The buffer the icon draw samples. GE texture address 0x0919BD00 with stride
// 0x100 and size word 0x0908 (256 wide, 512 tall), swizzled CLUT8. The palette
// is 8888 at 0x091AE510, mask 0xFF (format word 0x00FF03). The address is the
// buffer that draw points at, not an immediate in the resident image.
constexpr std::uint32_t kSheetAddress = 0x0919BD00u;
constexpr std::uint32_t kClutAddress = 0x091AE510u;
constexpr std::uint32_t kSheetWidth = 256u;
constexpr std::uint32_t kSheetHeight = 512u;

gpu::TextureState sheet_texture() {
    gpu::TextureState texture;
    texture.address = kSheetAddress;
    texture.buffer_width = kSheetWidth;
    texture.width = static_cast<std::uint16_t>(kSheetWidth);
    texture.height = static_cast<std::uint16_t>(kSheetHeight);
    texture.format = gpu::TextureFormat::Clut8;
    texture.swizzled = true;
    texture.clut_address = kClutAddress;
    texture.clut_format = 3u;
    texture.clut_mask = 0xFFu;
    return texture;
}

// A sample of the texels and the palette, so a quest that fills the buffer
// decodes once instead of every frame.
std::uint64_t sample_hash(const psprecomp::GuestMemory &memory) {
    const std::uint8_t *texels =
        memory.raw_pointer(kSheetAddress, static_cast<std::size_t>(kSheetWidth) * kSheetHeight);
    const std::uint8_t *clut = memory.raw_pointer(kClutAddress, 1024u);
    if (texels == nullptr || clut == nullptr) return 0u;
    std::uint64_t hash = 14695981039346656037ull;
    const auto mix = [&](std::uint8_t byte) {
        hash ^= byte;
        hash *= 1099511628211ull;
    };
    for (std::size_t i = 0; i < static_cast<std::size_t>(kSheetWidth) * kSheetHeight; i += 64u) mix(texels[i]);
    for (std::size_t i = 0; i < 1024u; i += 4u) mix(clut[i]);
    return hash == 0u ? 1u : hash;
}

bool varied(const std::vector<std::uint32_t> &pixels) {
    if (pixels.size() < 8u) return false;
    const std::uint32_t first = pixels[0];
    unsigned different = 0u;
    for (std::size_t i = 0; i < pixels.size(); i += 97u)
        if (pixels[i] != first && ++different >= 8u) return true;
    return false;
}

struct Cache {
    bool ready{};
    std::uint64_t hash{};
    std::uint64_t generation{};
    std::vector<std::uint32_t> pixels;
};

Cache &cache() {
    static Cache value;
    return value;
}

} // namespace

void refresh_hunt_icons(const psprecomp::GuestMemory &memory) {
    Cache &kept = cache();
    const std::uint64_t hash = sample_hash(memory);
    if (hash == 0u) {
        kept.ready = false;
        kept.hash = 0u;
        return;
    }
    if (hash == kept.hash) return;
    kept.hash = hash;
    std::vector<std::uint32_t> pixels;
    if (!gpu::decode_texture(memory, sheet_texture(), pixels) || !varied(pixels)) {
        kept.ready = false;
        return;
    }
    kept.pixels = std::move(pixels);
    kept.ready = true;
    if (++kept.generation == 0u) kept.generation = 1u;
}

HuntIconSheet hunt_icon_sheet() {
    const Cache &kept = cache();
    HuntIconSheet sheet;
    if (!kept.ready || kept.pixels.empty()) return sheet;
    sheet.ready = true;
    sheet.width = kSheetWidth;
    sheet.height = kSheetHeight;
    sheet.pixels = kept.pixels.data();
    sheet.generation = kept.generation;
    return sheet;
}

} // namespace mhfu::camera
