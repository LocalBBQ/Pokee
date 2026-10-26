#include "game/button_glyphs.hpp"

#include "gpu/texture_decode.hpp"
#include "gpu/texture_pack.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdlib>
#include <future>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

namespace mhfu::game {
namespace {

struct Decoded {
    bool ok{};
    std::uint32_t width{};
    std::uint32_t height{};
    std::vector<std::uint32_t> pixels;
};

struct Cache {
    bool noted{};
    gpu::TextureState texture{};
    // The pack's image for the sheet, as the renderer last chose it, and the
    // one the pixels are from (empty: the game's texture).
    std::filesystem::path image;
    std::filesystem::path shown_image;
    std::future<Decoded> decoding;
    std::filesystem::path decoding_image;
    std::uint64_t texture_key{};
    bool ready{};
    std::uint32_t width{};
    std::uint32_t height{};
    std::uint64_t generation{};
    std::vector<std::uint32_t> pixels;
};

Cache &cache() {
    static Cache value;
    return value;
}

void publish(Cache &kept, std::vector<std::uint32_t> pixels, std::uint32_t width, std::uint32_t height) {
    kept.pixels = std::move(pixels);
    kept.width = width;
    kept.height = height;
    kept.ready = true;
    if (++kept.generation == 0u) kept.generation = 1u;
}

} // namespace

std::optional<ButtonGlyph> button_glyph_for(std::uint32_t button) noexcept {
    switch (button) {
    case 0x2000u:
        return ButtonGlyph::Circle;
    case 0x4000u:
        return ButtonGlyph::Cross;
    case 0x1000u:
        return ButtonGlyph::Triangle;
    case 0x8000u:
        return ButtonGlyph::Square;
    default:
        return std::nullopt;
    }
}

// MHFU_TRACE_GATHER: each palette the sheet is drawn with, and each sheet
// decoded from the game's texture, saved as button_glyphs_<n>.png in the
// working directory, a few of each.
namespace {

bool trace_on() {
    static const bool on = std::getenv("MHFU_TRACE_GATHER") != nullptr;
    return on;
}

void trace_state(const gpu::TextureState &texture) {
    static std::vector<std::array<std::uint32_t, 6>> seen;
    const std::array<std::uint32_t, 6> state{texture.clut_address, texture.clut_format, texture.clut_shift,
        texture.clut_mask, texture.clut_offset, static_cast<std::uint32_t>(texture.format)};
    if (seen.size() >= 32u || std::find(seen.begin(), seen.end(), state) != seen.end()) return;
    seen.push_back(state);
    std::cout << "[button glyphs] sheet drawn with fmt=" << state[5] << " clut=0x" << std::hex << state[0]
              << " clutfmt=" << state[1] << std::dec << " shift=" << state[2] << " mask=0x" << std::hex << state[3]
              << " offset=0x" << state[4] << std::dec << " swizzled=" << texture.swizzled << std::endl;
}

void trace_sheet(const std::vector<std::uint32_t> &pixels, const gpu::TextureState &texture) {
    static unsigned saved = 0u;
    if (saved >= 8u) return;
    const std::filesystem::path file =
        std::filesystem::current_path() / ("button_glyphs_" + std::to_string(saved++) + ".png");
    const bool written = gpu::write_texture_pack_png(file, pixels.data(), kQuestBarSide, kQuestBarSide);
    std::cout << "[button glyphs] the game's sheet, clut=0x" << std::hex << texture.clut_address << std::dec
              << " shift=" << texture.clut_shift << " offset=" << texture.clut_offset << ", "
              << (written ? "saved as " : "not saved as ") << file.string() << std::endl;
}

} // namespace

void note_quest_bar(const gpu::TextureState &texture) {
    if (!texture.enabled || texture.address != kQuestBarAddress || texture.width != kQuestBarSide ||
        texture.height != kQuestBarSide)
        return;
    if (trace_on()) trace_state(texture);
    Cache &kept = cache();
    kept.texture = texture;
    kept.noted = true;
}

void note_quest_bar_image(const std::filesystem::path &file) {
    cache().image = file;
}

void refresh_button_glyphs(const psprecomp::GuestMemory &memory) {
    Cache &kept = cache();
    // A decode that has finished: shown if it is still the image the pack
    // gives the sheet.
    if (kept.decoding.valid() && kept.decoding.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
        Decoded done = kept.decoding.get();
        if (done.ok && kept.decoding_image == kept.image) {
            kept.shown_image = kept.decoding_image;
            publish(kept, std::move(done.pixels), done.width, done.height);
        } else if (!done.ok) {
            std::cerr << "[button glyphs] cannot read " << kept.decoding_image.string() << "; the game's own are drawn"
                      << std::endl;
            kept.image.clear();
        }
    }
    if (!kept.image.empty()) {
        if (kept.image != kept.shown_image && !kept.decoding.valid()) {
            kept.decoding_image = kept.image;
            kept.decoding = std::async(std::launch::async, [file = kept.image] {
                Decoded out;
                out.ok = gpu::decode_texture_pack_image(file, out.pixels, out.width, out.height);
                return out;
            });
        }
        return;
    }
    // No pack image: the game's own sheet, through the palette it was drawn with.
    if (!kept.noted) return;
    const std::uint64_t key = gpu::texture_key(memory, kept.texture);
    if (kept.ready && kept.shown_image.empty() && key == kept.texture_key) return;
    std::vector<std::uint32_t> pixels;
    if (!gpu::decode_texture(memory, kept.texture, pixels) ||
        pixels.size() != static_cast<std::size_t>(kQuestBarSide) * kQuestBarSide)
        return;
    kept.texture_key = key;
    kept.shown_image.clear();
    if (trace_on()) trace_sheet(pixels, kept.texture);
    publish(kept, std::move(pixels), kQuestBarSide, kQuestBarSide);
}

ButtonGlyphSheet button_glyph_sheet() {
    const Cache &kept = cache();
    ButtonGlyphSheet sheet;
    if (!kept.ready || kept.pixels.empty()) return sheet;
    sheet.ready = true;
    sheet.width = kept.width;
    sheet.height = kept.height;
    sheet.pixels = kept.pixels.data();
    sheet.generation = kept.generation;
    return sheet;
}

GlyphCell button_glyph_cell(ButtonGlyph glyph) noexcept {
    constexpr float kCell = static_cast<float>(kButtonGlyphSide) / static_cast<float>(kQuestBarSide);
    const float u = static_cast<float>(static_cast<unsigned>(glyph)) * kCell;
    return {u, 0.0f, u + kCell, kCell};
}

} // namespace mhfu::game
