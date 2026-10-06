#include "psprecomp/runtime.hpp"
#include "ovl09D63100_stage003_532E6ED590D365B3_units.hpp"
#include <bit>
#include <cmath>
#include <cstdint>
#include <limits>

namespace psprecomp {
static const std::uint16_t kEntryIds_ovl09D63100_stage003_532E6ED590D365B3_unit_0000[126] = {
    1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32,
    33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64,
    65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96,
    97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126,
};
void ovl09D63100_stage003_532E6ED590D365B3_unit_0000_entry(Runtime &rt, AllegrexContext &ctx, std::uint16_t direct_entry_id, GuestMemory::AotFastView &aot_mem) {
    std::uint32_t jump_target = 0u;
    std::uint32_t local_transfers = 0u;
    std::uint32_t local_pc = ctx.pc;
    std::uint32_t entry_id = direct_entry_id;
LOCAL_DISPATCH:
    {
    if (entry_id == 0u) {
        const std::uint32_t entry_delta = local_pc - 0x09D63140u;
        entry_id = (entry_delta < 504u && (entry_delta & 3u) == 0u) ? kEntryIds_ovl09D63100_stage003_532E6ED590D365B3_unit_0000[entry_delta >> 2u] : 0u;
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
    ctx.gpr[2] = (ctx.gpr[2] + static_cast<std::uint32_t>(-1700));
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
    ctx.gpr[2] = (ctx.gpr[2] + static_cast<std::uint32_t>(-1700));
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
    goto L_09D63268;
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
    ctx.gpr[29] = (ctx.gpr[29] + static_cast<std::uint32_t>(-16));
    goto L_09D6321C;
L_09D6321C:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(12), ctx.gpr[31]);
    goto L_09D63220;
L_09D63220:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(8), ctx.gpr[16]);
    goto L_09D63224;
L_09D63224:
    ctx.gpr[16] = (ctx.gpr[4] + 0u);
    goto L_09D63228;
L_09D63228:
    ctx.gpr[2] = (2469u << 16u);
    goto L_09D6322C;
L_09D6322C:
    ctx.gpr[5] = (2518u << 16u);
    goto L_09D63230;
L_09D63230:
    ctx.gpr[4] = (aot_mem.aot_load32(ctx.gpr[2] + static_cast<std::uint32_t>(-3680)));
    goto L_09D63234;
L_09D63234:
    ctx.gpr[31] = (0x09D6323Cu);
    ctx.gpr[5] = (ctx.gpr[5] + static_cast<std::uint32_t>(13808));
    ctx.pc = 0x088C435Cu;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D6323Cu) goto L_09D6323C;
    return;
L_09D63238:
    ctx.gpr[5] = (ctx.gpr[5] + static_cast<std::uint32_t>(13808));
    goto L_09D6323C;
L_09D6323C:
    ctx.gpr[31] = (0x09D63244u);
    ctx.gpr[4] = (ctx.gpr[16] + 0u);
    ctx.pc = 0x088CACCCu;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D63244u) goto L_09D63244;
    return;
L_09D63240:
    ctx.gpr[4] = (ctx.gpr[16] + 0u);
    goto L_09D63244;
L_09D63244:
    ctx.gpr[31] = (aot_mem.aot_load32(ctx.gpr[29] + static_cast<std::uint32_t>(12)));
    goto L_09D63248;
L_09D63248:
    ctx.gpr[16] = (aot_mem.aot_load32(ctx.gpr[29] + static_cast<std::uint32_t>(8)));
    goto L_09D6324C;
L_09D6324C:
    jump_target = ctx.gpr[31];
    ctx.gpr[29] = (ctx.gpr[29] + static_cast<std::uint32_t>(16));
    local_pc = jump_target;
    if (++local_transfers < 2048u) { entry_id = 0u; goto LOCAL_DISPATCH; }
    ctx.pc = jump_target;
    return;
L_09D63250:
    ctx.gpr[29] = (ctx.gpr[29] + static_cast<std::uint32_t>(16));
    goto L_09D63254;
L_09D63254:
    // nop
    goto L_09D63258;
L_09D63258:
    ctx.gpr[2] = (2518u << 16u);
    goto L_09D6325C;
L_09D6325C:
    jump_target = ctx.gpr[31];
    ctx.gpr[2] = (ctx.gpr[2] + static_cast<std::uint32_t>(13488));
    local_pc = jump_target;
    if (++local_transfers < 2048u) { entry_id = 0u; goto LOCAL_DISPATCH; }
    ctx.pc = jump_target;
    return;
L_09D63260:
    ctx.gpr[2] = (ctx.gpr[2] + static_cast<std::uint32_t>(13488));
    goto L_09D63264;
L_09D63264:
    // nop
    goto L_09D63268;
L_09D63268:
    jump_target = ctx.gpr[31];
    // nop
    local_pc = jump_target;
    if (++local_transfers < 2048u) { entry_id = 0u; goto LOCAL_DISPATCH; }
    ctx.pc = jump_target;
    return;
L_09D6326C:
    // nop
    goto L_09D63270;
L_09D63270:
    ctx.gpr[2] = (2518u << 16u);
    goto L_09D63274;
L_09D63274:
    jump_target = ctx.gpr[31];
    ctx.gpr[2] = (ctx.gpr[2] + static_cast<std::uint32_t>(13792));
    local_pc = jump_target;
    if (++local_transfers < 2048u) { entry_id = 0u; goto LOCAL_DISPATCH; }
    ctx.pc = jump_target;
    return;
L_09D63278:
    ctx.gpr[2] = (ctx.gpr[2] + static_cast<std::uint32_t>(13792));
    goto L_09D6327C;
L_09D6327C:
    // nop
    goto L_09D63280;
L_09D63280:
    jump_target = ctx.gpr[31];
    ctx.gpr[2] = (0u + static_cast<std::uint32_t>(1));
    local_pc = jump_target;
    if (++local_transfers < 2048u) { entry_id = 0u; goto LOCAL_DISPATCH; }
    ctx.pc = jump_target;
    return;
L_09D63284:
    ctx.gpr[2] = (0u + static_cast<std::uint32_t>(1));
    goto L_09D63288;
L_09D63288:
    jump_target = ctx.gpr[31];
    ctx.gpr[2] = (0u + static_cast<std::uint32_t>(1));
    local_pc = jump_target;
    if (++local_transfers < 2048u) { entry_id = 0u; goto LOCAL_DISPATCH; }
    ctx.pc = jump_target;
    return;
L_09D6328C:
    ctx.gpr[2] = (0u + static_cast<std::uint32_t>(1));
    goto L_09D63290;
L_09D63290:
    jump_target = ctx.gpr[31];
    ctx.gpr[2] = (0u + static_cast<std::uint32_t>(1));
    local_pc = jump_target;
    if (++local_transfers < 2048u) { entry_id = 0u; goto LOCAL_DISPATCH; }
    ctx.pc = jump_target;
    return;
L_09D63294:
    ctx.gpr[2] = (0u + static_cast<std::uint32_t>(1));
    goto L_09D63298;
L_09D63298:
    ctx.gpr[29] = (ctx.gpr[29] + static_cast<std::uint32_t>(-16));
    goto L_09D6329C;
L_09D6329C:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(12), ctx.gpr[31]);
    goto L_09D632A0;
L_09D632A0:
    ctx.gpr[25] = (aot_mem.aot_load32(ctx.gpr[4] + static_cast<std::uint32_t>(0)));
    goto L_09D632A4;
L_09D632A4:
    ctx.gpr[25] = (aot_mem.aot_load32(ctx.gpr[25] + static_cast<std::uint32_t>(40)));
    goto L_09D632A8;
L_09D632A8:
    jump_target = ctx.gpr[25];
    ctx.gpr[31] = (0x09D632B0u);
    // nop
    ctx.pc = jump_target;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D632B0u) goto L_09D632B0;
    return;
L_09D632AC:
    // nop
    goto L_09D632B0;
L_09D632B0:
    ctx.gpr[31] = (aot_mem.aot_load32(ctx.gpr[29] + static_cast<std::uint32_t>(12)));
    goto L_09D632B4;
L_09D632B4:
    ctx.gpr[2] = (aot_mem.aot_load8(ctx.gpr[2] + static_cast<std::uint32_t>(63)));
    goto L_09D632B8;
L_09D632B8:
    jump_target = ctx.gpr[31];
    ctx.gpr[29] = (ctx.gpr[29] + static_cast<std::uint32_t>(16));
    local_pc = jump_target;
    if (++local_transfers < 2048u) { entry_id = 0u; goto LOCAL_DISPATCH; }
    ctx.pc = jump_target;
    return;
L_09D632BC:
    ctx.gpr[29] = (ctx.gpr[29] + static_cast<std::uint32_t>(16));
    goto L_09D632C0;
L_09D632C0:
    ctx.gpr[29] = (ctx.gpr[29] + static_cast<std::uint32_t>(-16));
    goto L_09D632C4;
L_09D632C4:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(12), ctx.gpr[31]);
    goto L_09D632C8;
L_09D632C8:
    ctx.gpr[25] = (aot_mem.aot_load32(ctx.gpr[4] + static_cast<std::uint32_t>(0)));
    goto L_09D632CC;
L_09D632CC:
    ctx.gpr[25] = (aot_mem.aot_load32(ctx.gpr[25] + static_cast<std::uint32_t>(40)));
    goto L_09D632D0;
L_09D632D0:
    jump_target = ctx.gpr[25];
    ctx.gpr[31] = (0x09D632D8u);
    // nop
    ctx.pc = jump_target;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D632D8u) goto L_09D632D8;
    return;
L_09D632D4:
    // nop
    goto L_09D632D8;
L_09D632D8:
    ctx.gpr[31] = (aot_mem.aot_load32(ctx.gpr[29] + static_cast<std::uint32_t>(12)));
    goto L_09D632DC;
L_09D632DC:
    ctx.gpr[2] = (aot_mem.aot_load32(ctx.gpr[2] + static_cast<std::uint32_t>(52)));
    goto L_09D632E0;
L_09D632E0:
    jump_target = ctx.gpr[31];
    ctx.gpr[29] = (ctx.gpr[29] + static_cast<std::uint32_t>(16));
    local_pc = jump_target;
    if (++local_transfers < 2048u) { entry_id = 0u; goto LOCAL_DISPATCH; }
    ctx.pc = jump_target;
    return;
L_09D632E4:
    ctx.gpr[29] = (ctx.gpr[29] + static_cast<std::uint32_t>(16));
    goto L_09D632E8;
L_09D632E8:
    ctx.gpr[29] = (ctx.gpr[29] + static_cast<std::uint32_t>(-16));
    goto L_09D632EC;
L_09D632EC:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(12), ctx.gpr[31]);
    goto L_09D632F0;
L_09D632F0:
    ctx.gpr[25] = (aot_mem.aot_load32(ctx.gpr[4] + static_cast<std::uint32_t>(0)));
    goto L_09D632F4;
L_09D632F4:
    ctx.gpr[25] = (aot_mem.aot_load32(ctx.gpr[25] + static_cast<std::uint32_t>(40)));
    goto L_09D632F8;
L_09D632F8:
    jump_target = ctx.gpr[25];
    ctx.gpr[31] = (0x09D63300u);
    // nop
    ctx.pc = jump_target;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D63300u) goto L_09D63300;
    return;
L_09D632FC:
    // nop
    goto L_09D63300;
L_09D63300:
    ctx.gpr[31] = (aot_mem.aot_load32(ctx.gpr[29] + static_cast<std::uint32_t>(12)));
    goto L_09D63304;
L_09D63304:
    ctx.gpr[2] = (aot_mem.aot_load8(ctx.gpr[2] + static_cast<std::uint32_t>(64)));
    goto L_09D63308;
L_09D63308:
    jump_target = ctx.gpr[31];
    ctx.gpr[29] = (ctx.gpr[29] + static_cast<std::uint32_t>(16));
    local_pc = jump_target;
    if (++local_transfers < 2048u) { entry_id = 0u; goto LOCAL_DISPATCH; }
    ctx.pc = jump_target;
    return;
L_09D6330C:
    ctx.gpr[29] = (ctx.gpr[29] + static_cast<std::uint32_t>(16));
    goto L_09D63310;
L_09D63310:
    ctx.gpr[29] = (ctx.gpr[29] + static_cast<std::uint32_t>(-16));
    goto L_09D63314;
L_09D63314:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(12), ctx.gpr[31]);
    goto L_09D63318;
L_09D63318:
    ctx.gpr[25] = (aot_mem.aot_load32(ctx.gpr[4] + static_cast<std::uint32_t>(0)));
    goto L_09D6331C;
L_09D6331C:
    ctx.gpr[25] = (aot_mem.aot_load32(ctx.gpr[25] + static_cast<std::uint32_t>(40)));
    goto L_09D63320;
L_09D63320:
    jump_target = ctx.gpr[25];
    ctx.gpr[31] = (0x09D63328u);
    // nop
    ctx.pc = jump_target;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D63328u) goto L_09D63328;
    return;
L_09D63324:
    // nop
    goto L_09D63328;
L_09D63328:
    ctx.gpr[31] = (aot_mem.aot_load32(ctx.gpr[29] + static_cast<std::uint32_t>(12)));
    goto L_09D6332C;
L_09D6332C:
    ctx.gpr[2] = (aot_mem.aot_load32(ctx.gpr[2] + static_cast<std::uint32_t>(56)));
    goto L_09D63330;
L_09D63330:
    jump_target = ctx.gpr[31];
    ctx.gpr[29] = (ctx.gpr[29] + static_cast<std::uint32_t>(16));
    local_pc = jump_target;
    if (++local_transfers < 2048u) { entry_id = 0u; goto LOCAL_DISPATCH; }
    ctx.pc = jump_target;
    return;
L_09D63334:
    ctx.gpr[29] = (ctx.gpr[29] + static_cast<std::uint32_t>(16));
    ctx.pc = 0x09D63338u; return;
}

void ovl09D63100_stage003_532E6ED590D365B3_unit_0000(Runtime &rt, AllegrexContext &ctx) {
    auto aot_mem = rt.memory().aot_fast_view();
    ovl09D63100_stage003_532E6ED590D365B3_unit_0000_entry(rt, ctx, 0u, aot_mem);
}

void register_ovl09D63100_stage003_532E6ED590D365B3_unit_0(Runtime &runtime) {
    runtime.register_function(0x09D63140u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D63144u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D63148u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D6314Cu, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D63150u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D63154u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D63158u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D6315Cu, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D63160u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D63164u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D63168u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D6316Cu, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D63170u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D63174u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D63178u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D6317Cu, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D63180u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D63184u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D63188u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D6318Cu, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D63190u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D63194u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D63198u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D6319Cu, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D631A0u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D631A4u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D631A8u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D631ACu, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D631B0u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D631B4u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D631B8u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D631BCu, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D631C0u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D631C4u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D631C8u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D631CCu, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D631D0u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D631D4u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D631D8u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D631DCu, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D631E0u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D631E4u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D631E8u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D631ECu, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D631F0u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D631F4u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D631F8u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D631FCu, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D63200u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D63204u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D63208u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D6320Cu, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D63210u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D63214u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D63218u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D6321Cu, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D63220u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D63224u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D63228u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D6322Cu, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D63230u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D63234u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D63238u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D6323Cu, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D63240u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D63244u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D63248u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D6324Cu, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D63250u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D63254u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D63258u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D6325Cu, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D63260u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D63264u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D63268u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D6326Cu, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D63270u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D63274u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D63278u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D6327Cu, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D63280u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D63284u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D63288u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D6328Cu, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D63290u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D63294u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D63298u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D6329Cu, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D632A0u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D632A4u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D632A8u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D632ACu, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D632B0u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D632B4u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D632B8u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D632BCu, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D632C0u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D632C4u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D632C8u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D632CCu, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D632D0u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D632D4u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D632D8u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D632DCu, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D632E0u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D632E4u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D632E8u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D632ECu, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D632F0u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D632F4u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D632F8u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D632FCu, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D63300u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D63304u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D63308u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D6330Cu, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D63310u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D63314u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D63318u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D6331Cu, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D63320u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D63324u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D63328u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D6332Cu, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D63330u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
    runtime.register_function(0x09D63334u, &ovl09D63100_stage003_532E6ED590D365B3_unit_0000, "recomp_unit_ovl09D63100_stage003_532E6ED590D365B3_unit_0000");
}
} // namespace psprecomp
