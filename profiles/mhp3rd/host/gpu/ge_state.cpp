#include "ge_state.hpp"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>

namespace mhp3rd::gpu {
namespace {

// Display list opcodes (command = word >> 24).
enum Command : std::uint32_t {
    kNop = 0x00,
    kVertexAddress = 0x01,
    kIndexAddress = 0x02,
    kPrimitive = 0x04,
    kBezier = 0x05,
    kSpline = 0x06,
    kBoundingBox = 0x07,
    kJump = 0x08,
    kConditionalJump = 0x09,
    kCall = 0x0A,
    kReturn = 0x0B,
    kEnd = 0x0C,
    kSignal = 0x0E,
    kFinish = 0x0F,
    kBase = 0x10,
    kVertexType = 0x12,
    kOffsetAddress = 0x13,
    kOrigin = 0x14,
    kLightingEnable = 0x17,
    kCullFaceEnable = 0x1D,
    kTextureMapEnable = 0x1E,
    kAlphaBlendEnable = 0x21,
    kAlphaTestEnable = 0x22,
    kDepthTestEnable = 0x23,
    kBoneMatrixNumber = 0x2A,
    kBoneMatrixData = 0x2B,
    kWorldMatrixNumber = 0x3A,
    kWorldMatrixData = 0x3B,
    kViewMatrixNumber = 0x3C,
    kViewMatrixData = 0x3D,
    kProjMatrixNumber = 0x3E,
    kProjMatrixData = 0x3F,
    kTexGenMatrixNumber = 0x40,
    kTexGenMatrixData = 0x41,
    kViewportXScale = 0x42,
    kViewportYScale = 0x43,
    kViewportZScale = 0x44,
    kViewportXCenter = 0x45,
    kViewportYCenter = 0x46,
    kViewportZCenter = 0x47,
    kTexScaleU = 0x48,
    kTexScaleV = 0x49,
    kTexOffsetU = 0x4A,
    kTexOffsetV = 0x4B,
    kOffsetX = 0x4C,
    kOffsetY = 0x4D,
    // Read off the game rather than recalled: 0x53 carries the material update
    // mask (3 and 7), 0x55 and 0x56 carry colours, 0x58 carries an alpha (0 and
    // 0xFF) and 0x5B a float. That fixes the run as update, emissive, ambient,
    // diffuse, specular, alpha — and an unlit draw takes ambient and alpha.
    kMaterialAmbient = 0x55,
    kMaterialAlpha = 0x58,
    kCull = 0x9B,
    kFrameBufferPointer = 0x9C,
    kFrameBufferWidth = 0x9D,
    kDepthBufferPointer = 0x9E,
    kDepthBufferWidth = 0x9F,
    kTextureAddress0 = 0xA0,
    kTextureBufferWidth0 = 0xA8,
    kClutAddress = 0xB0,
    kClutAddressUpper = 0xB1,
    kTextureSize0 = 0xB8,
    kTextureMode = 0xC2,
    kTextureFormat = 0xC3,
    kLoadClut = 0xC4,
    kClutFormat = 0xC5,
    kTextureFilter = 0xC6,
    kTextureWrap = 0xC7,
    kTextureFunction = 0xC9,
    kTextureFlush = 0xCB,
    kFrameBufferPixelFormat = 0xD2,
    kClearMode = 0xD3,
    kScissor1 = 0xD4,
    kScissor2 = 0xD5,
    kMinZ = 0xD6,
    kMaxZ = 0xD7,
    kAlphaTest = 0xDB,
    kDepthTest = 0xDE,
    kBlendMode = 0xDF,
    kBlendFixedA = 0xE0,
    kBlendFixedB = 0xE1,
    kDepthWriteDisable = 0xE7,
};

// GE pointers carry their high byte in the companion width register; when that
// byte is zero the address is an offset inside VRAM.
std::uint32_t resolve_ge_address(std::uint32_t address) {
    return (address & 0xFF000000u) == 0u ? (address | 0x04000000u) : address;
}

float decode_float24(std::uint32_t data) {
    const std::uint32_t bits = data << 8u;
    float value{};
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}

std::uint32_t expand_color(std::uint32_t value, std::uint32_t format) {
    switch (format) {
    case 4u: {  // 5650
        const std::uint32_t r = (value & 0x1Fu) * 255u / 31u;
        const std::uint32_t g = ((value >> 5u) & 0x3Fu) * 255u / 63u;
        const std::uint32_t b = ((value >> 11u) & 0x1Fu) * 255u / 31u;
        return 0xFF000000u | (b << 16u) | (g << 8u) | r;
    }
    case 5u: {  // 5551
        const std::uint32_t r = (value & 0x1Fu) * 255u / 31u;
        const std::uint32_t g = ((value >> 5u) & 0x1Fu) * 255u / 31u;
        const std::uint32_t b = ((value >> 10u) & 0x1Fu) * 255u / 31u;
        const std::uint32_t a = ((value >> 15u) & 1u) * 255u;
        return (a << 24u) | (b << 16u) | (g << 8u) | r;
    }
    case 6u: {  // 4444
        const std::uint32_t r = (value & 0xFu) * 17u;
        const std::uint32_t g = ((value >> 4u) & 0xFu) * 17u;
        const std::uint32_t b = ((value >> 8u) & 0xFu) * 17u;
        const std::uint32_t a = ((value >> 12u) & 0xFu) * 17u;
        return (a << 24u) | (b << 16u) | (g << 8u) | r;
    }
    default:
        return value;  // 8888
    }
}

std::uint32_t align_up(std::uint32_t value, std::uint32_t alignment) {
    return (value + alignment - 1u) & ~(alignment - 1u);
}

void identity(std::array<float, 16> &matrix) {
    matrix.fill(0.0f);
    matrix[0] = matrix[5] = matrix[10] = matrix[15] = 1.0f;
}

} // namespace

std::uint32_t decode_vertices(const GuestMemory &memory, std::uint32_t address, std::uint32_t vertex_type,
                              std::uint32_t count, std::vector<Vertex> &out, const float *bone_matrices) {
    // Field order is weights, texcoords, color, normal, position; every field is
    // aligned to its component size and the vertex to the largest of them.
    const std::uint32_t texcoord_type = vertex_type & 3u;
    const std::uint32_t color_type = (vertex_type >> 2u) & 7u;
    const std::uint32_t normal_type = (vertex_type >> 5u) & 3u;
    const std::uint32_t position_type = (vertex_type >> 7u) & 3u;
    const std::uint32_t weight_type = (vertex_type >> 9u) & 3u;
    const std::uint32_t weight_count = ((vertex_type >> 14u) & 7u) + 1u;
    const std::uint32_t morph_count = ((vertex_type >> 18u) & 7u) + 1u;
    const bool through = (vertex_type & (1u << 23u)) != 0u;

    static constexpr std::uint32_t kComponentSize[4] = {0u, 1u, 2u, 4u};
    static constexpr std::uint32_t kColorSize[8] = {0u, 0u, 0u, 0u, 2u, 2u, 2u, 4u};

    std::uint32_t offset = 0u;
    std::uint32_t biggest = 1u;
    const auto place = [&](std::uint32_t component, std::uint32_t components) {
        if (component == 0u) return std::uint32_t{0xFFFFFFFFu};
        offset = align_up(offset, component);
        biggest = std::max(biggest, component);
        const std::uint32_t at = offset;
        offset += component * components;
        return at;
    };

    const std::uint32_t weight_offset = place(kComponentSize[weight_type], weight_count);
    const bool skinned = weight_type != 0u && bone_matrices != nullptr && !through;
    const std::uint32_t texcoord_offset = place(kComponentSize[texcoord_type], 2u);
    const std::uint32_t color_component = kColorSize[color_type];
    const std::uint32_t color_offset = place(color_component, 1u);
    const std::uint32_t normal_offset = place(kComponentSize[normal_type], 3u);
    const std::uint32_t position_offset = place(kComponentSize[position_type], 3u);
    const std::uint32_t stride = align_up(offset, biggest) * morph_count;
    if (stride == 0u) return 0u;

    // Resolve the whole vertex run once and read it directly; a run that is not
    // contiguous in host memory falls back to checked loads.
    const std::uint8_t *data =
        count != 0u ? memory.raw_pointer(address, static_cast<std::size_t>(count) * stride) : nullptr;
    const auto load8 = [&](std::uint32_t at) -> std::uint8_t {
        return data != nullptr ? data[at - address] : memory.load8(at);
    };
    const auto load16 = [&](std::uint32_t at) -> std::uint16_t {
        if (data == nullptr) return memory.load16(at);
        std::uint16_t value{};
        std::memcpy(&value, data + (at - address), sizeof(value));
        return value;
    };
    const auto load32 = [&](std::uint32_t at) -> std::uint32_t {
        if (data == nullptr) return memory.load32(at);
        std::uint32_t value{};
        std::memcpy(&value, data + (at - address), sizeof(value));
        return value;
    };

    const auto read_unsigned = [&](std::uint32_t at, std::uint32_t type) -> float {
        switch (type) {
        case 1u: return static_cast<float>(load8(at));
        case 2u: return static_cast<float>(load16(at));
        case 3u: {
            const std::uint32_t bits = load32(at);
            float value{};
            std::memcpy(&value, &bits, sizeof(value));
            return value;
        }
        default: return 0.0f;
        }
    };

    const auto read_component = [&](std::uint32_t at, std::uint32_t type) -> float {
        switch (type) {
        case 1u: return static_cast<float>(static_cast<std::int8_t>(load8(at)));
        case 2u: return static_cast<float>(static_cast<std::int16_t>(load16(at)));
        case 3u: {
            const std::uint32_t bits = load32(at);
            float value{};
            std::memcpy(&value, &bits, sizeof(value));
            return value;
        }
        default: return 0.0f;
        }
    };

    out.clear();
    out.reserve(count);
    for (std::uint32_t i = 0; i < count; ++i) {
        const std::uint32_t base = address + i * stride;
        Vertex vertex{};
        if (texcoord_offset != 0xFFFFFFFFu) {
            // Texture coordinates are unsigned; reading them signed wrapped the
            // upper half of every 8- and 16-bit UV range to negative values.
            const std::uint32_t component = kComponentSize[texcoord_type];
            const float scale = texcoord_type == 1u ? 1.0f / 128.0f
                                                    : (texcoord_type == 2u && !through ? 1.0f / 32768.0f : 1.0f);
            vertex.texcoord[0] = read_unsigned(base + texcoord_offset, texcoord_type) * scale;
            vertex.texcoord[1] = read_unsigned(base + texcoord_offset + component, texcoord_type) * scale;
        }
        if (color_offset != 0xFFFFFFFFu) {
            const std::uint32_t raw = color_component == 2u ? load16(base + color_offset)
                                                            : load32(base + color_offset);
            vertex.color = expand_color(raw, color_type);
        }
        if (normal_offset != 0xFFFFFFFFu) {
            const std::uint32_t component = kComponentSize[normal_type];
            const float scale = normal_type == 1u ? 1.0f / 128.0f
                                                  : (normal_type == 2u ? 1.0f / 32768.0f : 1.0f);
            for (std::uint32_t axis = 0; axis < 3u; ++axis)
                vertex.normal[axis] = read_component(base + normal_offset + axis * component, normal_type) * scale;
        }
        if (position_offset != 0xFFFFFFFFu) {
            const std::uint32_t component = kComponentSize[position_type];
            // Screen-space vertices keep their integer units; transformed ones
            // are normalized by their component size (128 / 32768, not 127 /
            // 32767 — the GE divides by the magnitude of the sign bit).
            const float scale = through ? 1.0f
                                        : (position_type == 1u ? 1.0f / 128.0f
                                                               : (position_type == 2u ? 1.0f / 32768.0f : 1.0f));
            for (std::uint32_t axis = 0; axis < 3u; ++axis) {
                float value = read_component(base + position_offset + axis * component, position_type);
                // Through-mode Z is unsigned in the 16-bit case.
                if (through && axis == 2u && position_type == 2u)
                    value = static_cast<float>(load16(base + position_offset + axis * component));
                vertex.position[axis] = value * scale;
            }
        }
        if (skinned) {
            // A skinned vertex is stored in its bones' space: the GE blends it
            // by its weights through the bone matrices, and the result is what
            // the world matrix then sees. Without this the raw positions stay
            // inside the unit cube they were normalised into and the whole model
            // collapses onto a couple of pixels.
            const std::uint32_t component = kComponentSize[weight_type];
            const float weight_scale = weight_type == 1u ? 1.0f / 128.0f
                                                         : (weight_type == 2u ? 1.0f / 32768.0f : 1.0f);
            std::array<float, 3> position{};
            std::array<float, 3> normal{};
            for (std::uint32_t bone = 0; bone < weight_count && bone < 8u; ++bone) {
                const float weight =
                    read_unsigned(base + weight_offset + bone * component, weight_type) * weight_scale;
                if (weight == 0.0f) continue;
                // PSP 3x4 matrices are three basis rows plus a translation row,
                // used as a row vector: p' = p * M.
                const float *m = bone_matrices + bone * 12u;
                for (std::uint32_t axis = 0; axis < 3u; ++axis) {
                    position[axis] += weight * (vertex.position[0] * m[axis] + vertex.position[1] * m[3u + axis] +
                                                vertex.position[2] * m[6u + axis] + m[9u + axis]);
                    normal[axis] += weight * (vertex.normal[0] * m[axis] + vertex.normal[1] * m[3u + axis] +
                                              vertex.normal[2] * m[6u + axis]);
                }
            }
            vertex.position[0] = position[0];
            vertex.position[1] = position[1];
            vertex.position[2] = position[2];
            vertex.normal = normal;
        }
        out.push_back(vertex);
    }
    return stride;
}

void GeState::handle_command(const GuestMemory &memory, std::uint32_t command, std::uint32_t data) {
    registers_[command] = data;
    // Which of 0x53..0x5C actually carries the colour the guest sets is worth
    // reading off the game rather than recalling: MHP3RD_TRACE_MATERIAL reports
    // every distinct value each of them is given.
    if (static const bool trace = std::getenv("MHP3RD_TRACE_MATERIAL") != nullptr;
        trace && command >= 0x53u && command <= 0x5Cu) {
        static std::map<std::uint32_t, std::map<std::uint32_t, std::uint64_t>> seen;
        auto &values = seen[command];
        if (++values[data] == 1u && values.size() <= 24u)
            std::cout << "[material] cmd=0x" << std::hex << command << " value=0x" << data << std::dec << "\n";
    }
    // MHP3RD_TRACE_LIGHTING does the same for the registers around that run
    // that lighting and fog may live in: the enables after 0x17, 0x50..0x52,
    // 0x5D..0x9A and 0xC8..0xD0. Each value is also shown as a 24-bit float,
    // since several of them carry one.
    if (static const bool trace = std::getenv("MHP3RD_TRACE_LIGHTING") != nullptr;
        trace && ((command >= 0x18u && command <= 0x20u) || (command >= 0x50u && command <= 0x52u) ||
                  (command >= 0x5Du && command <= 0x9Au) || (command >= 0xC8u && command <= 0xD0u))) {
        static std::map<std::uint32_t, std::map<std::uint32_t, std::uint64_t>> seen;
        auto &values = seen[command];
        if (++values[data] == 1u && values.size() <= 16u)
            std::cout << "[lighting] cmd=0x" << std::hex << command << " value=0x" << data << std::dec
                      << " float=" << decode_float24(data) << "\n";
    }
    switch (command) {
    // Addresses in a display list are relative: BASE supplies four high bits and
    // OFFSET_ADDR is added on top of them. Conflating the two — and letting
    // OFFSET_ADDR overwrite BASE — produced pointers into nowhere.
    case kVertexAddress: vertex_address_ = relative_address(data); break;
    case kIndexAddress: index_address_ = relative_address(data); break;
    case kBase: base_extended_ = (data & 0x000F0000u) << 8u; break;
    case kVertexType: vertex_type_ = data; break;
    case kOffsetAddress: offset_address_ = data << 8u; break;
    case kOrigin: break;  // handled in execute(), where the list pc is known

    case kCullFaceEnable: culling_enabled_ = (data & 1u) != 0u; break;
    case kCull: cull_clockwise_ = (data & 1u) != 0u; break;
    case kTextureMapEnable: texture_.enabled = (data & 1u) != 0u; break;
    case kLightingEnable: lighting_enabled_ = (data & 1u) != 0u; break;
    case kAlphaBlendEnable: blend_.enabled = (data & 1u) != 0u; break;
    case kAlphaTestEnable: alpha_test_.enabled = (data & 1u) != 0u; break;
    case kDepthTestEnable: depth_.test_enabled = (data & 1u) != 0u; break;
    case kDepthWriteDisable: depth_.write_enabled = (data & 1u) == 0u; break;
    case kDepthTest: depth_.function = data & 7u; break;
    case kMinZ: depth_.range_near = static_cast<std::uint16_t>(data); break;
    case kMaxZ: depth_.range_far = static_cast<std::uint16_t>(data); break;

    case kAlphaTest:
        alpha_test_.function = data & 7u;
        alpha_test_.reference = (data >> 8u) & 0xFFu;
        alpha_test_.mask = (data >> 16u) & 0xFFu;
        break;
    case kBlendMode:
        blend_.source_factor = data & 0xFu;
        blend_.destination_factor = (data >> 4u) & 0xFu;
        blend_.equation = (data >> 8u) & 0xFu;
        break;
    case kBlendFixedA: blend_.fixed_source = data; break;
    case kBlendFixedB: blend_.fixed_destination = data; break;

    case kFrameBufferPointer:
        target_.color_address = resolve_ge_address((target_.color_address & 0xFF000000u) | data);
        break;
    case kFrameBufferWidth:
        target_.color_stride = data & 0xFFFFu;
        target_.color_address =
            resolve_ge_address((target_.color_address & 0x00FFFFFFu) | ((data << 8u) & 0xFF000000u));
        break;
    case kDepthBufferPointer:
        target_.depth_address = resolve_ge_address((target_.depth_address & 0xFF000000u) | data);
        break;
    case kDepthBufferWidth:
        target_.depth_stride = data & 0xFFFFu;
        target_.depth_address =
            resolve_ge_address((target_.depth_address & 0x00FFFFFFu) | ((data << 8u) & 0xFF000000u));
        break;
    case kFrameBufferPixelFormat: target_.color_format = data & 3u; break;

    case kTextureAddress0: texture_.address = resolve_ge_address((texture_.address & 0xFF000000u) | data); break;
    case kTextureBufferWidth0:
        texture_.buffer_width = data & 0xFFFFu;
        texture_.address = resolve_ge_address((texture_.address & 0x00FFFFFFu) | ((data << 8u) & 0xFF000000u));
        break;
    case kTextureSize0:
        texture_.width = static_cast<std::uint16_t>(1u << (data & 0xFu));
        texture_.height = static_cast<std::uint16_t>(1u << ((data >> 8u) & 0xFu));
        break;
    case kTextureFormat: texture_.format = static_cast<TextureFormat>(data & 0xFu); break;
    case kTextureMode: texture_.swizzled = (data & 1u) != 0u; break;
    case kClutAddress: texture_.clut_address = resolve_ge_address((texture_.clut_address & 0xFF000000u) | data); break;
    case kClutAddressUpper:
        texture_.clut_address = resolve_ge_address((texture_.clut_address & 0x00FFFFFFu) | ((data << 8u) & 0xFF000000u));
        break;
    case kClutFormat:
        texture_.clut_format = data & 3u;
        texture_.clut_shift = (data >> 2u) & 0x1Fu;
        texture_.clut_mask = (data >> 8u) & 0xFFu;
        texture_.clut_offset = (data >> 16u) & 0x1Fu;
        break;
    case kTextureFilter:
        texture_.min_filter = data & 7u;
        texture_.mag_filter = (data >> 8u) & 1u;
        break;
    case kTextureWrap:
        texture_.wrap_s = data & 1u;
        texture_.wrap_t = (data >> 8u) & 1u;
        break;
    case kTextureFunction:
        texture_.function = data & 7u;
        texture_.alpha_from_texture = ((data >> 8u) & 1u) != 0u;
        break;
    case kTexScaleU: texture_.scale_u = decode_float24(data); break;
    case kTexScaleV: texture_.scale_v = decode_float24(data); break;
    case kTexOffsetU: texture_.offset_u = decode_float24(data); break;
    case kTexOffsetV: texture_.offset_v = decode_float24(data); break;

    case kViewportXScale: viewport_.x_scale = decode_float24(data); break;
    case kViewportYScale: viewport_.y_scale = decode_float24(data); break;
    case kViewportZScale: viewport_.z_scale = decode_float24(data); break;
    case kViewportXCenter: viewport_.x_offset = decode_float24(data); break;
    case kViewportYCenter: viewport_.y_offset = decode_float24(data); break;
    case kViewportZCenter: viewport_.z_offset = decode_float24(data); break;
    case kScissor1:
        viewport_.scissor_x1 = data & 0x3FFu;
        viewport_.scissor_y1 = (data >> 10u) & 0x3FFu;
        break;
    case kScissor2:
        viewport_.scissor_x2 = data & 0x3FFu;
        viewport_.scissor_y2 = (data >> 10u) & 0x3FFu;
        break;
    // The offset is an unsigned 16-bit value with 4 fractional bits; the bits
    // above it are not part of it.
    case kOffsetX: viewport_.offset_x = static_cast<float>(data & 0xFFFFu) / 16.0f; break;
    case kOffsetY: viewport_.offset_y = static_cast<float>(data & 0xFFFFu) / 16.0f; break;

    case kMaterialAmbient: material_color_ = (material_color_ & 0xFF000000u) | (data & 0x00FFFFFFu); break;
    case kMaterialAlpha: material_color_ = (material_color_ & 0x00FFFFFFu) | ((data & 0xFFu) << 24u); break;

    case kWorldMatrixNumber: world_write_index_ = data & 0xFu; break;
    case kViewMatrixNumber: view_write_index_ = data & 0xFu; break;
    case kProjMatrixNumber: projection_write_index_ = data & 0xFu; break;
    case kTexGenMatrixNumber: texture_write_index_ = data & 0xFu; break;
    case kBoneMatrixNumber: bone_write_index_ = data & 0x7Fu; break;

    case kWorldMatrixData:
    case kViewMatrixData:
    case kTexGenMatrixData:
    case kProjMatrixData:
    case kBoneMatrixData: {
        // 3x4 matrices arrive as 12 values, three basis rows then a translation
        // row; the projection matrix has all 16. Expand the 3x4 forms into the
        // column-major 4x4 the shader multiplies as M * v.
        const float value = decode_float24(data);
        const auto store_3x4 = [&](std::array<float, 16> &matrix, std::uint32_t &index) {
            if (index < 12u) {
                matrix[(index / 3u) * 4u + (index % 3u)] = value;
                matrix[3] = matrix[7] = matrix[11] = 0.0f;
                matrix[15] = 1.0f;
            }
            ++index;
        };
        if (command == kProjMatrixData) {
            if (projection_write_index_ < 16u) projection_[projection_write_index_] = value;
            ++projection_write_index_;
        } else if (command == kWorldMatrixData) {
            store_3x4(world_, world_write_index_);
        } else if (command == kViewMatrixData) {
            store_3x4(view_, view_write_index_);
        } else if (command == kTexGenMatrixData) {
            store_3x4(texture_matrix_, texture_write_index_);
        } else {
            if (bone_write_index_ < bone_matrices_.size()) bone_matrices_[bone_write_index_] = value;
            ++bone_write_index_;
        }
        break;
    }

    case kClearMode:
        // Bit 0 enables clear mode; bits 8..10 select which buffers it writes
        // (color, alpha/stencil, depth).
        clear_mode_ = (data & 1u) != 0u;
        clear_flags_ = (data >> 8u) & 7u;
        break;

    case kNop:
    case kTextureFlush:
    case kLoadClut:
        break;

    default:
        ++unhandled_commands_;
        break;
    }
}

void GeState::draw_primitive(const GuestMemory &memory, std::uint32_t data) {
    const std::uint32_t count = data & 0xFFFFu;
    const auto primitive = static_cast<PrimitiveType>((data >> 16u) & 7u);
    if (count == 0u || vertex_address_ == 0u) return;

    DrawCall call{};
    call.primitive = primitive;
    call.through = (vertex_type_ & (1u << 23u)) != 0u;
    call.texture = texture_;
    call.target = target_;
    call.blend = blend_;
    call.depth = depth_;
    call.alpha_test = alpha_test_;
    call.viewport = viewport_;
    call.culling_enabled = culling_enabled_;
    call.cull_clockwise = cull_clockwise_;
    call.clear_mode = clear_mode_;
    call.clear_flags = clear_flags_;
    call.vertex_type = vertex_type_;
    call.material_color = material_color_;
    call.lighting_enabled = lighting_enabled_;
    call.world = world_;
    call.view = view_;
    call.projection = projection_;
    call.texture_matrix = texture_matrix_;

    const std::uint32_t index_type = (vertex_type_ >> 11u) & 3u;
    std::uint32_t vertex_count = count;
    std::uint32_t first_vertex = 0u;
    if (index_type != 0u && index_address_ != 0u) {
        // A list that points its indices outside RAM is malformed; skip the draw
        // rather than faulting the whole runtime on it.
        if (!memory.contains(index_address_, count * (index_type == 1u ? 1u : index_type == 2u ? 2u : 4u))) return;
        call.indices.reserve(count);
        std::uint32_t lowest = 0xFFFFFFFFu;
        std::uint32_t highest = 0u;
        for (std::uint32_t i = 0; i < count; ++i) {
            std::uint32_t index = 0u;
            if (index_type == 1u) index = memory.load8(index_address_ + i);
            else if (index_type == 2u) index = memory.load16(index_address_ + i * 2u);
            else index = memory.load32(index_address_ + i * 4u);
            lowest = std::min(lowest, index);
            highest = std::max(highest, index);
            call.indices.push_back(static_cast<std::uint16_t>(index));
        }
        // Games draw a mesh as many indexed prims into one shared vertex
        // buffer. Decode only the vertices this prim references, not the
        // whole buffer from vertex 0, and rebase its indices onto them.
        if (count != 0u) {
            first_vertex = lowest;
            for (std::uint16_t &index : call.indices) index = static_cast<std::uint16_t>(index - lowest);
        }
        vertex_count = count != 0u ? highest - lowest + 1u : 0u;
    }

    const std::uint32_t probe = decode_vertices(memory, vertex_address_, vertex_type_, 0u, call.vertices);
    if (probe == 0u) return;
    const std::uint32_t first_address = vertex_address_ + first_vertex * probe;
    if (!memory.contains(first_address, static_cast<std::size_t>(probe) * vertex_count)) return;
    const std::uint32_t stride =
        decode_vertices(memory, first_address, vertex_type_, vertex_count, call.vertices, bone_matrices_.data());
    if (stride == 0u || call.vertices.empty()) return;
    // A prim leaves VADDR/IADDR alone but advances the pointer it consumed, so
    // a run of prims can share one setup. An indexed prim consumes indices, not
    // vertices: advancing the vertex pointer instead walked it off the mesh and
    // every prim after the first read its vertices from the wrong place.
    if (index_type != 0u && index_address_ != 0u)
        index_address_ += count * (index_type == 1u ? 1u : index_type == 2u ? 2u : 4u);
    else
        vertex_address_ += stride * count;

    // Which register values a lit draw is actually made with: one line per
    // distinct combination of vertex type, enables and material registers,
    // printed with every non-zero register lighting may read. Light positions
    // and colours are left out of the key because the game animates them.
    if (static const bool trace = std::getenv("MHP3RD_TRACE_LIGHTING") != nullptr; trace && lighting_enabled_) {
        static std::map<std::vector<std::uint32_t>, std::uint64_t> seen;
        std::vector<std::uint32_t> key{vertex_type_};
        for (std::uint32_t command = 0x18u; command <= 0x1Fu; ++command) key.push_back(registers_[command]);
        for (std::uint32_t command = 0x50u; command <= 0x5Eu; ++command) key.push_back(registers_[command]);
        if (++seen[key] == 1u && seen.size() <= 256u) {
            std::cout << "[lit-draw] vtype=0x" << std::hex << vertex_type_ << " verts=" << std::dec
                      << call.vertices.size() << " n0=(" << call.vertices[0].normal[0] << ","
                      << call.vertices[0].normal[1] << "," << call.vertices[0].normal[2] << ") c0=0x" << std::hex
                      << call.vertices[0].color << " regs";
            for (std::uint32_t command = 0x18u; command <= 0x1Fu; ++command)
                std::cout << " " << command << ":" << registers_[command];
            for (std::uint32_t command = 0x50u; command <= 0x9Au; ++command)
                if (registers_[command] != 0u) std::cout << " " << command << ":" << registers_[command];
            std::cout << std::dec << "\n";
        }
    }

    ++draw_count_;
    vertex_count_ += call.vertices.size();
    if (draw_sink_) draw_sink_(call);
}

std::uint32_t GeState::execute(const GuestMemory &memory, std::uint32_t pc, std::uint32_t stall, bool &finished) {
    finished = false;
    if (world_[15] == 0.0f) {
        identity(world_);
        identity(view_);
        identity(projection_);
        identity(texture_matrix_);
    }

    for (std::uint32_t steps = 0; steps < 2'000'000u; ++steps) {
        if (stall != 0u && pc == stall) return pc;
        if (!memory.contains(pc, 4u)) return pc;
        const std::uint32_t word = memory.load32(pc);
        const std::uint32_t command = word >> 24u;
        const std::uint32_t data = word & 0x00FFFFFFu;
        pc += 4u;

        switch (command) {
        case kJump:
            pc = relative_address(data) & 0x0FFFFFFCu;
            continue;
        case kOrigin:
            // ORIGIN makes later relative addresses count from this command.
            offset_address_ = pc - 4u;
            continue;
        case kConditionalJump:
            // The bounding-box test is not evaluated; taking the jump would skip
            // geometry, so fall through to the next command instead.
            continue;
        case kCall:
            call_stack_.push_back(pc);
            pc = relative_address(data) & 0x0FFFFFFCu;
            continue;
        case kReturn:
            if (!call_stack_.empty()) {
                pc = call_stack_.back();
                call_stack_.pop_back();
            }
            continue;
        case kEnd:
            finished = true;
            return pc;
        case kFinish:
            if (signal_sink_) signal_sink_(0x10000u | (data & 0xFFFFu), pc);
            continue;
        case kSignal:
            if (signal_sink_) signal_sink_(data & 0xFFFFu, pc);
            continue;
        case kPrimitive:
            draw_primitive(memory, data);
            continue;
        case kBezier:
        case kSpline:
            // Curved surfaces are not tessellated yet.
            ++unhandled_commands_;
            continue;
        default:
            handle_command(memory, command, data);
            continue;
        }
    }
    return pc;
}

} // namespace mhp3rd::gpu
