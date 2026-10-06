#include "psprecomp/runtime.hpp"
#include "ovl09D63100_stage058_20B88A4CF23F7FEE_units.hpp"
#include <bit>
#include <cmath>
#include <cstdint>
#include <limits>

namespace psprecomp {
static const std::uint16_t kEntryIds_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000[182] = {
    1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32,
    33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64,
    65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96,
    97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128,
    129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160,
    161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182,
};
void ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000_entry(Runtime &rt, AllegrexContext &ctx, std::uint16_t direct_entry_id, GuestMemory::AotFastView &aot_mem) {
    std::uint32_t jump_target = 0u;
    std::uint32_t local_transfers = 0u;
    std::uint32_t local_pc = ctx.pc;
    std::uint32_t entry_id = direct_entry_id;
LOCAL_DISPATCH:
    {
    if (entry_id == 0u) {
        const std::uint32_t entry_delta = local_pc - 0x09D63140u;
        entry_id = (entry_delta < 728u && (entry_delta & 3u) == 0u) ? kEntryIds_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000[entry_delta >> 2u] : 0u;
    }
    switch (entry_id) {
    case 1u: goto L_09D63140;
    case 2u: goto L_09D63144;
    case 3u: goto L_09D63148;
    case 4u: goto L_09D6314C;
    case 5u: goto L_09D63150;
    case 6u: goto L_09D63154;
    case 7u: goto L_09D63158;
    case 8u: goto L_09D6315C;
    case 9u: goto L_09D63160;
    case 10u: goto L_09D63164;
    case 11u: goto L_09D63168;
    case 12u: goto L_09D6316C;
    case 13u: goto L_09D63170;
    case 14u: goto L_09D63174;
    case 15u: goto L_09D63178;
    case 16u: goto L_09D6317C;
    case 17u: goto L_09D63180;
    case 18u: goto L_09D63184;
    case 19u: goto L_09D63188;
    case 20u: goto L_09D6318C;
    case 21u: goto L_09D63190;
    case 22u: goto L_09D63194;
    case 23u: goto L_09D63198;
    case 24u: goto L_09D6319C;
    case 25u: goto L_09D631A0;
    case 26u: goto L_09D631A4;
    case 27u: goto L_09D631A8;
    case 28u: goto L_09D631AC;
    case 29u: goto L_09D631B0;
    case 30u: goto L_09D631B4;
    case 31u: goto L_09D631B8;
    case 32u: goto L_09D631BC;
    case 33u: goto L_09D631C0;
    case 34u: goto L_09D631C4;
    case 35u: goto L_09D631C8;
    case 36u: goto L_09D631CC;
    case 37u: goto L_09D631D0;
    case 38u: goto L_09D631D4;
    case 39u: goto L_09D631D8;
    case 40u: goto L_09D631DC;
    case 41u: goto L_09D631E0;
    case 42u: goto L_09D631E4;
    case 43u: goto L_09D631E8;
    case 44u: goto L_09D631EC;
    case 45u: goto L_09D631F0;
    case 46u: goto L_09D631F4;
    case 47u: goto L_09D631F8;
    case 48u: goto L_09D631FC;
    case 49u: goto L_09D63200;
    case 50u: goto L_09D63204;
    case 51u: goto L_09D63208;
    case 52u: goto L_09D6320C;
    case 53u: goto L_09D63210;
    case 54u: goto L_09D63214;
    case 55u: goto L_09D63218;
    case 56u: goto L_09D6321C;
    case 57u: goto L_09D63220;
    case 58u: goto L_09D63224;
    case 59u: goto L_09D63228;
    case 60u: goto L_09D6322C;
    case 61u: goto L_09D63230;
    case 62u: goto L_09D63234;
    case 63u: goto L_09D63238;
    case 64u: goto L_09D6323C;
    case 65u: goto L_09D63240;
    case 66u: goto L_09D63244;
    case 67u: goto L_09D63248;
    case 68u: goto L_09D6324C;
    case 69u: goto L_09D63250;
    case 70u: goto L_09D63254;
    case 71u: goto L_09D63258;
    case 72u: goto L_09D6325C;
    case 73u: goto L_09D63260;
    case 74u: goto L_09D63264;
    case 75u: goto L_09D63268;
    case 76u: goto L_09D6326C;
    case 77u: goto L_09D63270;
    case 78u: goto L_09D63274;
    case 79u: goto L_09D63278;
    case 80u: goto L_09D6327C;
    case 81u: goto L_09D63280;
    case 82u: goto L_09D63284;
    case 83u: goto L_09D63288;
    case 84u: goto L_09D6328C;
    case 85u: goto L_09D63290;
    case 86u: goto L_09D63294;
    case 87u: goto L_09D63298;
    case 88u: goto L_09D6329C;
    case 89u: goto L_09D632A0;
    case 90u: goto L_09D632A4;
    case 91u: goto L_09D632A8;
    case 92u: goto L_09D632AC;
    case 93u: goto L_09D632B0;
    case 94u: goto L_09D632B4;
    case 95u: goto L_09D632B8;
    case 96u: goto L_09D632BC;
    case 97u: goto L_09D632C0;
    case 98u: goto L_09D632C4;
    case 99u: goto L_09D632C8;
    case 100u: goto L_09D632CC;
    case 101u: goto L_09D632D0;
    case 102u: goto L_09D632D4;
    case 103u: goto L_09D632D8;
    case 104u: goto L_09D632DC;
    case 105u: goto L_09D632E0;
    case 106u: goto L_09D632E4;
    case 107u: goto L_09D632E8;
    case 108u: goto L_09D632EC;
    case 109u: goto L_09D632F0;
    case 110u: goto L_09D632F4;
    case 111u: goto L_09D632F8;
    case 112u: goto L_09D632FC;
    case 113u: goto L_09D63300;
    case 114u: goto L_09D63304;
    case 115u: goto L_09D63308;
    case 116u: goto L_09D6330C;
    case 117u: goto L_09D63310;
    case 118u: goto L_09D63314;
    case 119u: goto L_09D63318;
    case 120u: goto L_09D6331C;
    case 121u: goto L_09D63320;
    case 122u: goto L_09D63324;
    case 123u: goto L_09D63328;
    case 124u: goto L_09D6332C;
    case 125u: goto L_09D63330;
    case 126u: goto L_09D63334;
    case 127u: goto L_09D63338;
    case 128u: goto L_09D6333C;
    case 129u: goto L_09D63340;
    case 130u: goto L_09D63344;
    case 131u: goto L_09D63348;
    case 132u: goto L_09D6334C;
    case 133u: goto L_09D63350;
    case 134u: goto L_09D63354;
    case 135u: goto L_09D63358;
    case 136u: goto L_09D6335C;
    case 137u: goto L_09D63360;
    case 138u: goto L_09D63364;
    case 139u: goto L_09D63368;
    case 140u: goto L_09D6336C;
    case 141u: goto L_09D63370;
    case 142u: goto L_09D63374;
    case 143u: goto L_09D63378;
    case 144u: goto L_09D6337C;
    case 145u: goto L_09D63380;
    case 146u: goto L_09D63384;
    case 147u: goto L_09D63388;
    case 148u: goto L_09D6338C;
    case 149u: goto L_09D63390;
    case 150u: goto L_09D63394;
    case 151u: goto L_09D63398;
    case 152u: goto L_09D6339C;
    case 153u: goto L_09D633A0;
    case 154u: goto L_09D633A4;
    case 155u: goto L_09D633A8;
    case 156u: goto L_09D633AC;
    case 157u: goto L_09D633B0;
    case 158u: goto L_09D633B4;
    case 159u: goto L_09D633B8;
    case 160u: goto L_09D633BC;
    case 161u: goto L_09D633C0;
    case 162u: goto L_09D633C4;
    case 163u: goto L_09D633C8;
    case 164u: goto L_09D633CC;
    case 165u: goto L_09D633D0;
    case 166u: goto L_09D633D4;
    case 167u: goto L_09D633D8;
    case 168u: goto L_09D633DC;
    case 169u: goto L_09D633E0;
    case 170u: goto L_09D633E4;
    case 171u: goto L_09D633E8;
    case 172u: goto L_09D633EC;
    case 173u: goto L_09D633F0;
    case 174u: goto L_09D633F4;
    case 175u: goto L_09D633F8;
    case 176u: goto L_09D633FC;
    case 177u: goto L_09D63400;
    case 178u: goto L_09D63404;
    case 179u: goto L_09D63408;
    case 180u: goto L_09D6340C;
    case 181u: goto L_09D63410;
    case 182u: goto L_09D63414;
    default:
        if (local_transfers == 0u) rt.unsupported(ctx.pc, 0u, "invalid internal function entry");
        else ctx.pc = local_pc;
        return;
    }
    }
L_09D63140:
    // nop
    goto L_09D63144;
L_09D63144:
    // nop
    goto L_09D63148;
L_09D63148:
    // nop
    goto L_09D6314C;
L_09D6314C:
    // nop
    goto L_09D63150;
L_09D63150:
    // nop
    goto L_09D63154;
L_09D63154:
    // nop
    goto L_09D63158;
L_09D63158:
    // nop
    goto L_09D6315C;
L_09D6315C:
    // nop
    goto L_09D63160;
L_09D63160:
    // nop
    goto L_09D63164;
L_09D63164:
    // nop
    goto L_09D63168;
L_09D63168:
    // nop
    goto L_09D6316C;
L_09D6316C:
    // nop
    goto L_09D63170;
L_09D63170:
    // nop
    goto L_09D63174;
L_09D63174:
    // nop
    goto L_09D63178;
L_09D63178:
    // nop
    goto L_09D6317C;
L_09D6317C:
    // nop
    goto L_09D63180;
L_09D63180:
    ctx.gpr[29] = (ctx.gpr[29] + static_cast<std::uint32_t>(-16));
    goto L_09D63184;
L_09D63184:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(12), ctx.gpr[31]);
    goto L_09D63188;
L_09D63188:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(8), ctx.gpr[16]);
    goto L_09D6318C;
L_09D6318C:
    ctx.gpr[31] = (0x09D63194u);
    ctx.gpr[16] = (ctx.gpr[4] + 0u);
    ctx.pc = 0x088C7814u;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D63194u) goto L_09D63194;
    return;
L_09D63190:
    ctx.gpr[16] = (ctx.gpr[4] + 0u);
    goto L_09D63194;
L_09D63194:
    ctx.gpr[2] = (2204u << 16u);
    goto L_09D63198;
L_09D63198:
    ctx.gpr[2] = (ctx.gpr[2] + static_cast<std::uint32_t>(8640));
    goto L_09D6319C;
L_09D6319C:
    aot_mem.aot_store32(ctx.gpr[16] + static_cast<std::uint32_t>(0), ctx.gpr[2]);
    goto L_09D631A0;
L_09D631A0:
    ctx.gpr[2] = (ctx.gpr[16] + 0u);
    goto L_09D631A4;
L_09D631A4:
    ctx.gpr[31] = (aot_mem.aot_load32(ctx.gpr[29] + static_cast<std::uint32_t>(12)));
    goto L_09D631A8;
L_09D631A8:
    ctx.gpr[16] = (aot_mem.aot_load32(ctx.gpr[29] + static_cast<std::uint32_t>(8)));
    goto L_09D631AC;
L_09D631AC:
    jump_target = ctx.gpr[31];
    ctx.gpr[29] = (ctx.gpr[29] + static_cast<std::uint32_t>(16));
    local_pc = jump_target;
    if (++local_transfers < 2048u) { entry_id = 0u; goto LOCAL_DISPATCH; }
    ctx.pc = jump_target;
    return;
L_09D631B0:
    ctx.gpr[29] = (ctx.gpr[29] + static_cast<std::uint32_t>(16));
    goto L_09D631B4;
L_09D631B4:
    // nop
    goto L_09D631B8;
L_09D631B8:
    ctx.gpr[29] = (ctx.gpr[29] + static_cast<std::uint32_t>(-16));
    goto L_09D631BC;
L_09D631BC:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(12), ctx.gpr[31]);
    goto L_09D631C0;
L_09D631C0:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(8), ctx.gpr[17]);
    goto L_09D631C4;
L_09D631C4:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(4), ctx.gpr[16]);
    goto L_09D631C8;
L_09D631C8:
    ctx.gpr[17] = (ctx.gpr[4] + 0u);
    goto L_09D631CC;
L_09D631CC:
    { const bool branch_taken = ctx.gpr[17] == 0u;
    ctx.gpr[16] = (ctx.gpr[5] + 0u);
      if (branch_taken) {
          goto L_09D631FC;
      }
      goto L_09D631D4;
    }
L_09D631D0:
    ctx.gpr[16] = (ctx.gpr[5] + 0u);
    goto L_09D631D4;
L_09D631D4:
    ctx.gpr[2] = (2204u << 16u);
    goto L_09D631D8;
L_09D631D8:
    ctx.gpr[2] = (ctx.gpr[2] + static_cast<std::uint32_t>(8640));
    goto L_09D631DC;
L_09D631DC:
    aot_mem.aot_store32(ctx.gpr[17] + static_cast<std::uint32_t>(0), ctx.gpr[2]);
    goto L_09D631E0;
L_09D631E0:
    ctx.gpr[31] = (0x09D631E8u);
    ctx.gpr[5] = (0u + 0u);
    ctx.pc = 0x088C789Cu;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D631E8u) goto L_09D631E8;
    return;
L_09D631E4:
    ctx.gpr[5] = (0u + 0u);
    goto L_09D631E8;
L_09D631E8:
    ctx.gpr[2] = (static_cast<std::uint32_t>(static_cast<std::int32_t>(static_cast<std::int16_t>(ctx.gpr[16]))));
    goto L_09D631EC;
L_09D631EC:
    if (static_cast<std::int32_t>(ctx.gpr[2]) <= 0) {
    ctx.gpr[2] = (ctx.gpr[17] + 0u);
        goto L_09D63200;
    }
    goto L_09D631F4;
L_09D631F0:
    ctx.gpr[2] = (ctx.gpr[17] + 0u);
    goto L_09D631F4;
L_09D631F4:
    ctx.gpr[31] = (0x09D631FCu);
    ctx.gpr[4] = (ctx.gpr[17] + 0u);
    goto L_09D63348;
L_09D631F8:
    ctx.gpr[4] = (ctx.gpr[17] + 0u);
    goto L_09D631FC;
L_09D631FC:
    ctx.gpr[2] = (ctx.gpr[17] + 0u);
    goto L_09D63200;
L_09D63200:
    ctx.gpr[31] = (aot_mem.aot_load32(ctx.gpr[29] + static_cast<std::uint32_t>(12)));
    goto L_09D63204;
L_09D63204:
    ctx.gpr[17] = (aot_mem.aot_load32(ctx.gpr[29] + static_cast<std::uint32_t>(8)));
    goto L_09D63208;
L_09D63208:
    ctx.gpr[16] = (aot_mem.aot_load32(ctx.gpr[29] + static_cast<std::uint32_t>(4)));
    goto L_09D6320C;
L_09D6320C:
    jump_target = ctx.gpr[31];
    ctx.gpr[29] = (ctx.gpr[29] + static_cast<std::uint32_t>(16));
    local_pc = jump_target;
    if (++local_transfers < 2048u) { entry_id = 0u; goto LOCAL_DISPATCH; }
    ctx.pc = jump_target;
    return;
L_09D63210:
    ctx.gpr[29] = (ctx.gpr[29] + static_cast<std::uint32_t>(16));
    goto L_09D63214;
L_09D63214:
    // nop
    goto L_09D63218;
L_09D63218:
    ctx.gpr[29] = (ctx.gpr[29] + static_cast<std::uint32_t>(-64));
    goto L_09D6321C;
L_09D6321C:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(28), ctx.gpr[31]);
    goto L_09D63220;
L_09D63220:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(24), ctx.gpr[16]);
    goto L_09D63224;
L_09D63224:
    ctx.gpr[16] = (ctx.gpr[4] + 0u);
    goto L_09D63228;
L_09D63228:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(48), 0u);
    goto L_09D6322C;
L_09D6322C:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(52), 0u);
    goto L_09D63230;
L_09D63230:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(56), 0u);
    goto L_09D63234;
L_09D63234:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(60), 0u);
    goto L_09D63238;
L_09D63238:
    ctx.gpr[5] = (ctx.gpr[29] + static_cast<std::uint32_t>(60));
    goto L_09D6323C;
L_09D6323C:
    ctx.gpr[4] = (ctx.gpr[29] + static_cast<std::uint32_t>(56));
    goto L_09D63240;
L_09D63240:
    ctx.gpr[3] = (ctx.gpr[29] + static_cast<std::uint32_t>(52));
    goto L_09D63244;
L_09D63244:
    ctx.gpr[2] = (ctx.gpr[29] + static_cast<std::uint32_t>(48));
    goto L_09D63248;
L_09D63248:
    ctx.gpr[9] = (ctx.gpr[29] + static_cast<std::uint32_t>(32));
    goto L_09D6324C;
L_09D6324C:
    ctx.set_vfpu_scalar_bits_ct<0u>(aot_mem.aot_load32(ctx.gpr[5] + static_cast<std::uint32_t>(0)));
    goto L_09D63250;
L_09D63250:
    ctx.set_vfpu_scalar_bits_ct<32u>(aot_mem.aot_load32(ctx.gpr[4] + static_cast<std::uint32_t>(0)));
    goto L_09D63254;
L_09D63254:
    ctx.set_vfpu_scalar_bits_ct<64u>(aot_mem.aot_load32(ctx.gpr[3] + static_cast<std::uint32_t>(0)));
    goto L_09D63258;
L_09D63258:
    ctx.set_vfpu_scalar_bits_ct<96u>(aot_mem.aot_load32(ctx.gpr[2] + static_cast<std::uint32_t>(0)));
    goto L_09D6325C;
L_09D6325C:
    { float vfpu_value[4]{}; ctx.read_vfpu_vector_ct<0u, 4u>(vfpu_value);
      const std::uint32_t vfpu_address = ctx.gpr[9] + static_cast<std::uint32_t>(0);
      aot_mem.aot_store32(vfpu_address + 0u, std::bit_cast<std::uint32_t>(vfpu_value[0]));
      aot_mem.aot_store32(vfpu_address + 4u, std::bit_cast<std::uint32_t>(vfpu_value[1]));
      aot_mem.aot_store32(vfpu_address + 8u, std::bit_cast<std::uint32_t>(vfpu_value[2]));
      aot_mem.aot_store32(vfpu_address + 12u, std::bit_cast<std::uint32_t>(vfpu_value[3])); }
    goto L_09D63260;
L_09D63260:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(0), 0u);
    goto L_09D63264;
L_09D63264:
    ctx.gpr[2] = (2518u << 16u);
    goto L_09D63268;
L_09D63268:
    ctx.gpr[2] = (ctx.gpr[2] + static_cast<std::uint32_t>(14048));
    goto L_09D6326C;
L_09D6326C:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(4), ctx.gpr[2]);
    goto L_09D63270;
L_09D63270:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(8), 0u);
    goto L_09D63274;
L_09D63274:
    ctx.gpr[2] = (2469u << 16u);
    goto L_09D63278;
L_09D63278:
    ctx.gpr[4] = (aot_mem.aot_load32(ctx.gpr[2] + static_cast<std::uint32_t>(-3680)));
    goto L_09D6327C;
L_09D6327C:
    ctx.gpr[5] = (0u + static_cast<std::uint32_t>(3));
    goto L_09D63280;
L_09D63280:
    ctx.gpr[6] = (0u + static_cast<std::uint32_t>(6));
    goto L_09D63284;
L_09D63284:
    ctx.gpr[7] = (0u + static_cast<std::uint32_t>(72));
    goto L_09D63288;
L_09D63288:
    ctx.gpr[8] = (0u + static_cast<std::uint32_t>(11));
    goto L_09D6328C;
L_09D6328C:
    ctx.gpr[10] = (0u + static_cast<std::uint32_t>(180));
    goto L_09D63290;
L_09D63290:
    ctx.gpr[31] = (0x09D63298u);
    ctx.gpr[11] = (0u + static_cast<std::uint32_t>(128));
    ctx.pc = 0x088C4200u;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D63298u) goto L_09D63298;
    return;
L_09D63294:
    ctx.gpr[11] = (0u + static_cast<std::uint32_t>(128));
    goto L_09D63298;
L_09D63298:
    ctx.gpr[2] = (2469u << 16u);
    goto L_09D6329C;
L_09D6329C:
    ctx.gpr[4] = (aot_mem.aot_load32(ctx.gpr[2] + static_cast<std::uint32_t>(-3680)));
    goto L_09D632A0;
L_09D632A0:
    ctx.gpr[6] = (2518u << 16u);
    goto L_09D632A4;
L_09D632A4:
    ctx.gpr[5] = (0u + 0u);
    goto L_09D632A8;
L_09D632A8:
    ctx.gpr[6] = (ctx.gpr[6] + static_cast<std::uint32_t>(15472));
    goto L_09D632AC;
L_09D632AC:
    ctx.gpr[7] = (0u + 0u);
    goto L_09D632B0;
L_09D632B0:
    ctx.gpr[8] = (0u + 0u);
    goto L_09D632B4;
L_09D632B4:
    ctx.gpr[9] = (0u + 0u);
    goto L_09D632B8;
L_09D632B8:
    ctx.gpr[10] = (0u + 0u);
    goto L_09D632BC;
L_09D632BC:
    ctx.gpr[31] = (0x09D632C4u);
    ctx.gpr[11] = (0u + static_cast<std::uint32_t>(1));
    ctx.pc = 0x088C493Cu;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D632C4u) goto L_09D632C4;
    return;
L_09D632C0:
    ctx.gpr[11] = (0u + static_cast<std::uint32_t>(1));
    goto L_09D632C4;
L_09D632C4:
    ctx.gpr[2] = (2469u << 16u);
    goto L_09D632C8;
L_09D632C8:
    ctx.gpr[4] = (aot_mem.aot_load32(ctx.gpr[2] + static_cast<std::uint32_t>(-3680)));
    goto L_09D632CC;
L_09D632CC:
    ctx.gpr[6] = (2518u << 16u);
    goto L_09D632D0;
L_09D632D0:
    ctx.gpr[10] = (0u + static_cast<std::uint32_t>(1));
    goto L_09D632D4;
L_09D632D4:
    ctx.gpr[5] = (0u + 0u);
    goto L_09D632D8;
L_09D632D8:
    ctx.gpr[6] = (ctx.gpr[6] + static_cast<std::uint32_t>(15488));
    goto L_09D632DC;
L_09D632DC:
    ctx.gpr[7] = (0u + static_cast<std::uint32_t>(512));
    goto L_09D632E0;
L_09D632E0:
    ctx.gpr[8] = (0u + 0u);
    goto L_09D632E4;
L_09D632E4:
    ctx.gpr[9] = (0u + 0u);
    goto L_09D632E8;
L_09D632E8:
    ctx.gpr[31] = (0x09D632F0u);
    ctx.gpr[11] = (ctx.gpr[10] + 0u);
    ctx.pc = 0x088C493Cu;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D632F0u) goto L_09D632F0;
    return;
L_09D632EC:
    ctx.gpr[11] = (ctx.gpr[10] + 0u);
    goto L_09D632F0;
L_09D632F0:
    ctx.gpr[2] = (2469u << 16u);
    goto L_09D632F4;
L_09D632F4:
    ctx.gpr[4] = (aot_mem.aot_load32(ctx.gpr[2] + static_cast<std::uint32_t>(-3680)));
    goto L_09D632F8;
L_09D632F8:
    ctx.gpr[6] = (2518u << 16u);
    goto L_09D632FC;
L_09D632FC:
    ctx.gpr[5] = (0u + 0u);
    goto L_09D63300;
L_09D63300:
    ctx.gpr[6] = (ctx.gpr[6] + static_cast<std::uint32_t>(15504));
    goto L_09D63304;
L_09D63304:
    ctx.gpr[7] = (0u + 0u);
    goto L_09D63308;
L_09D63308:
    ctx.gpr[8] = (0u + static_cast<std::uint32_t>(256));
    goto L_09D6330C;
L_09D6330C:
    ctx.gpr[9] = (0u + 0u);
    goto L_09D63310;
L_09D63310:
    ctx.gpr[10] = (0u + static_cast<std::uint32_t>(2));
    goto L_09D63314;
L_09D63314:
    ctx.gpr[31] = (0x09D6331Cu);
    ctx.gpr[11] = (0u + static_cast<std::uint32_t>(1));
    ctx.pc = 0x088C493Cu;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D6331Cu) goto L_09D6331C;
    return;
L_09D63318:
    ctx.gpr[11] = (0u + static_cast<std::uint32_t>(1));
    goto L_09D6331C;
L_09D6331C:
    ctx.gpr[31] = (0x09D63324u);
    ctx.gpr[4] = (ctx.gpr[16] + 0u);
    ctx.pc = 0x088CACCCu;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D63324u) goto L_09D63324;
    return;
L_09D63320:
    ctx.gpr[4] = (ctx.gpr[16] + 0u);
    goto L_09D63324;
L_09D63324:
    ctx.gpr[31] = (aot_mem.aot_load32(ctx.gpr[29] + static_cast<std::uint32_t>(28)));
    goto L_09D63328;
L_09D63328:
    ctx.gpr[16] = (aot_mem.aot_load32(ctx.gpr[29] + static_cast<std::uint32_t>(24)));
    goto L_09D6332C;
L_09D6332C:
    jump_target = ctx.gpr[31];
    ctx.gpr[29] = (ctx.gpr[29] + static_cast<std::uint32_t>(64));
    local_pc = jump_target;
    if (++local_transfers < 2048u) { entry_id = 0u; goto LOCAL_DISPATCH; }
    ctx.pc = jump_target;
    return;
L_09D63330:
    ctx.gpr[29] = (ctx.gpr[29] + static_cast<std::uint32_t>(64));
    goto L_09D63334;
L_09D63334:
    // nop
    goto L_09D63338;
L_09D63338:
    ctx.gpr[2] = (2518u << 16u);
    goto L_09D6333C;
L_09D6333C:
    jump_target = ctx.gpr[31];
    ctx.gpr[2] = (ctx.gpr[2] + static_cast<std::uint32_t>(13872));
    local_pc = jump_target;
    if (++local_transfers < 2048u) { entry_id = 0u; goto LOCAL_DISPATCH; }
    ctx.pc = jump_target;
    return;
L_09D63340:
    ctx.gpr[2] = (ctx.gpr[2] + static_cast<std::uint32_t>(13872));
    goto L_09D63344;
L_09D63344:
    // nop
    goto L_09D63348;
L_09D63348:
    jump_target = ctx.gpr[31];
    // nop
    local_pc = jump_target;
    if (++local_transfers < 2048u) { entry_id = 0u; goto LOCAL_DISPATCH; }
    ctx.pc = jump_target;
    return;
L_09D6334C:
    // nop
    goto L_09D63350;
L_09D63350:
    ctx.gpr[2] = (2518u << 16u);
    goto L_09D63354;
L_09D63354:
    jump_target = ctx.gpr[31];
    ctx.gpr[2] = (ctx.gpr[2] + static_cast<std::uint32_t>(14024));
    local_pc = jump_target;
    if (++local_transfers < 2048u) { entry_id = 0u; goto LOCAL_DISPATCH; }
    ctx.pc = jump_target;
    return;
L_09D63358:
    ctx.gpr[2] = (ctx.gpr[2] + static_cast<std::uint32_t>(14024));
    goto L_09D6335C;
L_09D6335C:
    // nop
    goto L_09D63360;
L_09D63360:
    jump_target = ctx.gpr[31];
    ctx.gpr[2] = (0u + static_cast<std::uint32_t>(1));
    local_pc = jump_target;
    if (++local_transfers < 2048u) { entry_id = 0u; goto LOCAL_DISPATCH; }
    ctx.pc = jump_target;
    return;
L_09D63364:
    ctx.gpr[2] = (0u + static_cast<std::uint32_t>(1));
    goto L_09D63368;
L_09D63368:
    jump_target = ctx.gpr[31];
    ctx.gpr[2] = (0u + 0u);
    local_pc = jump_target;
    if (++local_transfers < 2048u) { entry_id = 0u; goto LOCAL_DISPATCH; }
    ctx.pc = jump_target;
    return;
L_09D6336C:
    ctx.gpr[2] = (0u + 0u);
    goto L_09D63370;
L_09D63370:
    jump_target = ctx.gpr[31];
    ctx.gpr[2] = (0u + 0u);
    local_pc = jump_target;
    if (++local_transfers < 2048u) { entry_id = 0u; goto LOCAL_DISPATCH; }
    ctx.pc = jump_target;
    return;
L_09D63374:
    ctx.gpr[2] = (0u + 0u);
    goto L_09D63378;
L_09D63378:
    ctx.gpr[29] = (ctx.gpr[29] + static_cast<std::uint32_t>(-16));
    goto L_09D6337C;
L_09D6337C:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(12), ctx.gpr[31]);
    goto L_09D63380;
L_09D63380:
    ctx.gpr[25] = (aot_mem.aot_load32(ctx.gpr[4] + static_cast<std::uint32_t>(0)));
    goto L_09D63384;
L_09D63384:
    ctx.gpr[25] = (aot_mem.aot_load32(ctx.gpr[25] + static_cast<std::uint32_t>(40)));
    goto L_09D63388;
L_09D63388:
    jump_target = ctx.gpr[25];
    ctx.gpr[31] = (0x09D63390u);
    // nop
    ctx.pc = jump_target;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D63390u) goto L_09D63390;
    return;
L_09D6338C:
    // nop
    goto L_09D63390;
L_09D63390:
    ctx.gpr[31] = (aot_mem.aot_load32(ctx.gpr[29] + static_cast<std::uint32_t>(12)));
    goto L_09D63394;
L_09D63394:
    ctx.gpr[2] = (aot_mem.aot_load8(ctx.gpr[2] + static_cast<std::uint32_t>(63)));
    goto L_09D63398;
L_09D63398:
    jump_target = ctx.gpr[31];
    ctx.gpr[29] = (ctx.gpr[29] + static_cast<std::uint32_t>(16));
    local_pc = jump_target;
    if (++local_transfers < 2048u) { entry_id = 0u; goto LOCAL_DISPATCH; }
    ctx.pc = jump_target;
    return;
L_09D6339C:
    ctx.gpr[29] = (ctx.gpr[29] + static_cast<std::uint32_t>(16));
    goto L_09D633A0;
L_09D633A0:
    ctx.gpr[29] = (ctx.gpr[29] + static_cast<std::uint32_t>(-16));
    goto L_09D633A4;
L_09D633A4:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(12), ctx.gpr[31]);
    goto L_09D633A8;
L_09D633A8:
    ctx.gpr[25] = (aot_mem.aot_load32(ctx.gpr[4] + static_cast<std::uint32_t>(0)));
    goto L_09D633AC;
L_09D633AC:
    ctx.gpr[25] = (aot_mem.aot_load32(ctx.gpr[25] + static_cast<std::uint32_t>(40)));
    goto L_09D633B0;
L_09D633B0:
    jump_target = ctx.gpr[25];
    ctx.gpr[31] = (0x09D633B8u);
    // nop
    ctx.pc = jump_target;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D633B8u) goto L_09D633B8;
    return;
L_09D633B4:
    // nop
    goto L_09D633B8;
L_09D633B8:
    ctx.gpr[31] = (aot_mem.aot_load32(ctx.gpr[29] + static_cast<std::uint32_t>(12)));
    goto L_09D633BC;
L_09D633BC:
    ctx.gpr[2] = (aot_mem.aot_load32(ctx.gpr[2] + static_cast<std::uint32_t>(52)));
    goto L_09D633C0;
L_09D633C0:
    jump_target = ctx.gpr[31];
    ctx.gpr[29] = (ctx.gpr[29] + static_cast<std::uint32_t>(16));
    local_pc = jump_target;
    if (++local_transfers < 2048u) { entry_id = 0u; goto LOCAL_DISPATCH; }
    ctx.pc = jump_target;
    return;
L_09D633C4:
    ctx.gpr[29] = (ctx.gpr[29] + static_cast<std::uint32_t>(16));
    goto L_09D633C8;
L_09D633C8:
    ctx.gpr[29] = (ctx.gpr[29] + static_cast<std::uint32_t>(-16));
    goto L_09D633CC;
L_09D633CC:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(12), ctx.gpr[31]);
    goto L_09D633D0;
L_09D633D0:
    ctx.gpr[25] = (aot_mem.aot_load32(ctx.gpr[4] + static_cast<std::uint32_t>(0)));
    goto L_09D633D4;
L_09D633D4:
    ctx.gpr[25] = (aot_mem.aot_load32(ctx.gpr[25] + static_cast<std::uint32_t>(40)));
    goto L_09D633D8;
L_09D633D8:
    jump_target = ctx.gpr[25];
    ctx.gpr[31] = (0x09D633E0u);
    // nop
    ctx.pc = jump_target;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D633E0u) goto L_09D633E0;
    return;
L_09D633DC:
    // nop
    goto L_09D633E0;
L_09D633E0:
    ctx.gpr[31] = (aot_mem.aot_load32(ctx.gpr[29] + static_cast<std::uint32_t>(12)));
    goto L_09D633E4;
L_09D633E4:
    ctx.gpr[2] = (aot_mem.aot_load8(ctx.gpr[2] + static_cast<std::uint32_t>(64)));
    goto L_09D633E8;
L_09D633E8:
    jump_target = ctx.gpr[31];
    ctx.gpr[29] = (ctx.gpr[29] + static_cast<std::uint32_t>(16));
    local_pc = jump_target;
    if (++local_transfers < 2048u) { entry_id = 0u; goto LOCAL_DISPATCH; }
    ctx.pc = jump_target;
    return;
L_09D633EC:
    ctx.gpr[29] = (ctx.gpr[29] + static_cast<std::uint32_t>(16));
    goto L_09D633F0;
L_09D633F0:
    ctx.gpr[29] = (ctx.gpr[29] + static_cast<std::uint32_t>(-16));
    goto L_09D633F4;
L_09D633F4:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(12), ctx.gpr[31]);
    goto L_09D633F8;
L_09D633F8:
    ctx.gpr[25] = (aot_mem.aot_load32(ctx.gpr[4] + static_cast<std::uint32_t>(0)));
    goto L_09D633FC;
L_09D633FC:
    ctx.gpr[25] = (aot_mem.aot_load32(ctx.gpr[25] + static_cast<std::uint32_t>(40)));
    goto L_09D63400;
L_09D63400:
    jump_target = ctx.gpr[25];
    ctx.gpr[31] = (0x09D63408u);
    // nop
    ctx.pc = jump_target;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D63408u) goto L_09D63408;
    return;
L_09D63404:
    // nop
    goto L_09D63408;
L_09D63408:
    ctx.gpr[31] = (aot_mem.aot_load32(ctx.gpr[29] + static_cast<std::uint32_t>(12)));
    goto L_09D6340C;
L_09D6340C:
    ctx.gpr[2] = (aot_mem.aot_load32(ctx.gpr[2] + static_cast<std::uint32_t>(56)));
    goto L_09D63410;
L_09D63410:
    jump_target = ctx.gpr[31];
    ctx.gpr[29] = (ctx.gpr[29] + static_cast<std::uint32_t>(16));
    local_pc = jump_target;
    if (++local_transfers < 2048u) { entry_id = 0u; goto LOCAL_DISPATCH; }
    ctx.pc = jump_target;
    return;
L_09D63414:
    ctx.gpr[29] = (ctx.gpr[29] + static_cast<std::uint32_t>(16));
    ctx.pc = 0x09D63418u; return;
}

void ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000(Runtime &rt, AllegrexContext &ctx) {
    auto aot_mem = rt.memory().aot_fast_view();
    ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000_entry(rt, ctx, 0u, aot_mem);
}

void register_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0(Runtime &runtime) {
    runtime.register_function(0x09D63140u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D63144u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D63148u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D6314Cu, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D63150u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D63154u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D63158u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D6315Cu, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D63160u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D63164u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D63168u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D6316Cu, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D63170u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D63174u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D63178u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D6317Cu, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D63180u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D63184u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D63188u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D6318Cu, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D63190u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D63194u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D63198u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D6319Cu, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D631A0u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D631A4u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D631A8u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D631ACu, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D631B0u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D631B4u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D631B8u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D631BCu, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D631C0u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D631C4u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D631C8u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D631CCu, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D631D0u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D631D4u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D631D8u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D631DCu, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D631E0u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D631E4u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D631E8u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D631ECu, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D631F0u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D631F4u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D631F8u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D631FCu, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D63200u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D63204u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D63208u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D6320Cu, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D63210u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D63214u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D63218u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D6321Cu, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D63220u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D63224u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D63228u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D6322Cu, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D63230u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D63234u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D63238u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D6323Cu, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D63240u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D63244u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D63248u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D6324Cu, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D63250u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D63254u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D63258u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D6325Cu, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D63260u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D63264u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D63268u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D6326Cu, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D63270u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D63274u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D63278u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D6327Cu, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D63280u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D63284u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D63288u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D6328Cu, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D63290u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D63294u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D63298u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D6329Cu, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D632A0u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D632A4u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D632A8u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D632ACu, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D632B0u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D632B4u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D632B8u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D632BCu, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D632C0u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D632C4u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D632C8u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D632CCu, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D632D0u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D632D4u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D632D8u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D632DCu, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D632E0u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D632E4u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D632E8u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D632ECu, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D632F0u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D632F4u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D632F8u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D632FCu, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D63300u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D63304u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D63308u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D6330Cu, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D63310u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D63314u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D63318u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D6331Cu, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D63320u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D63324u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D63328u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D6332Cu, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D63330u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D63334u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D63338u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D6333Cu, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D63340u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D63344u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D63348u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D6334Cu, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D63350u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D63354u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D63358u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D6335Cu, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D63360u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D63364u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D63368u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D6336Cu, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D63370u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D63374u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D63378u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D6337Cu, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D63380u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D63384u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D63388u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D6338Cu, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D63390u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D63394u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D63398u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D6339Cu, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D633A0u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D633A4u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D633A8u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D633ACu, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D633B0u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D633B4u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D633B8u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D633BCu, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D633C0u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D633C4u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D633C8u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D633CCu, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D633D0u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D633D4u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D633D8u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D633DCu, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D633E0u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D633E4u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D633E8u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D633ECu, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D633F0u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D633F4u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D633F8u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D633FCu, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D63400u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D63404u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D63408u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D6340Cu, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D63410u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
    runtime.register_function(0x09D63414u, &ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000, "recomp_unit_ovl09D63100_stage058_20B88A4CF23F7FEE_unit_0000");
}
} // namespace psprecomp
