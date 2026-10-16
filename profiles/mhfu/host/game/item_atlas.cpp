#include "game/item_atlas.hpp"

#include "gpu/texture_decode.hpp"

#include <vector>

namespace mhfu::game {
namespace {

constexpr std::uint32_t kSide = 256u;

struct Cache {
    bool noted{};
    gpu::TextureState texture{};
    bool ready{};
    std::uint64_t key{};
    std::uint64_t generation{};
    std::vector<std::uint32_t> pixels;
};

Cache &cache() {
    static Cache value;
    return value;
}

} // namespace

void note_item_atlas(const gpu::TextureState &texture) {
    if (!texture.enabled || texture.address != kItemAtlasAddress || texture.width != kSide ||
        texture.height != kSide)
        return;
    Cache &kept = cache();
    kept.texture = texture;
    kept.noted = true;
}

void refresh_item_atlas(const psprecomp::GuestMemory &memory) {
    Cache &kept = cache();
    if (!kept.noted) return;
    const std::uint64_t key = gpu::texture_key(memory, kept.texture);
    if (kept.ready && key == kept.key) return;
    std::vector<std::uint32_t> pixels;
    if (!gpu::decode_texture(memory, kept.texture, pixels) || pixels.size() != static_cast<std::size_t>(kSide) * kSide) {
        kept.ready = false;
        return;
    }
    kept.key = key;
    kept.pixels = std::move(pixels);
    kept.ready = true;
    if (++kept.generation == 0u) kept.generation = 1u;
}

ItemAtlas item_atlas() {
    const Cache &kept = cache();
    ItemAtlas atlas;
    if (!kept.ready || kept.pixels.empty()) return atlas;
    atlas.ready = true;
    atlas.width = kSide;
    atlas.height = kSide;
    atlas.pixels = kept.pixels.data();
    atlas.generation = kept.generation;
    return atlas;
}

} // namespace mhfu::game
