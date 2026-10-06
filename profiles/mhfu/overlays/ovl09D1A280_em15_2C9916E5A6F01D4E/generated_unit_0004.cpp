#include "psprecomp/runtime.hpp"
#include "ovl09D1A280_em15_2C9916E5A6F01D4E_units.hpp"
#include <bit>
#include <cmath>
#include <cstdint>
#include <limits>

namespace psprecomp {
static const std::uint16_t kEntryIds_ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004[56] = {
    1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32,
    33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56,
};
void ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004_entry(Runtime &rt, AllegrexContext &ctx, std::uint16_t direct_entry_id, GuestMemory::AotFastView &aot_mem) {
    std::uint32_t jump_target = 0u;
    std::uint32_t local_transfers = 0u;
    std::uint32_t local_pc = ctx.pc;
    std::uint32_t entry_id = direct_entry_id;
LOCAL_DISPATCH:
    {
    if (entry_id == 0u) {
        const std::uint32_t entry_delta = local_pc - 0x09D2A2C0u;
        entry_id = (entry_delta < 224u && (entry_delta & 3u) == 0u) ? kEntryIds_ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004[entry_delta >> 2u] : 0u;
    }
    switch (entry_id) {
    case 1u: goto L_09D2A2C0;
    case 2u: goto L_09D2A2C4;
    case 3u: goto L_09D2A2C8;
    case 4u: goto L_09D2A2CC;
    case 5u: goto L_09D2A2D0;
    case 6u: goto L_09D2A2D4;
    case 7u: goto L_09D2A2D8;
    case 8u: goto L_09D2A2DC;
    case 9u: goto L_09D2A2E0;
    case 10u: goto L_09D2A2E4;
    case 11u: goto L_09D2A2E8;
    case 12u: goto L_09D2A2EC;
    case 13u: goto L_09D2A2F0;
    case 14u: goto L_09D2A2F4;
    case 15u: goto L_09D2A2F8;
    case 16u: goto L_09D2A2FC;
    case 17u: goto L_09D2A300;
    case 18u: goto L_09D2A304;
    case 19u: goto L_09D2A308;
    case 20u: goto L_09D2A30C;
    case 21u: goto L_09D2A310;
    case 22u: goto L_09D2A314;
    case 23u: goto L_09D2A318;
    case 24u: goto L_09D2A31C;
    case 25u: goto L_09D2A320;
    case 26u: goto L_09D2A324;
    case 27u: goto L_09D2A328;
    case 28u: goto L_09D2A32C;
    case 29u: goto L_09D2A330;
    case 30u: goto L_09D2A334;
    case 31u: goto L_09D2A338;
    case 32u: goto L_09D2A33C;
    case 33u: goto L_09D2A340;
    case 34u: goto L_09D2A344;
    case 35u: goto L_09D2A348;
    case 36u: goto L_09D2A34C;
    case 37u: goto L_09D2A350;
    case 38u: goto L_09D2A354;
    case 39u: goto L_09D2A358;
    case 40u: goto L_09D2A35C;
    case 41u: goto L_09D2A360;
    case 42u: goto L_09D2A364;
    case 43u: goto L_09D2A368;
    case 44u: goto L_09D2A36C;
    case 45u: goto L_09D2A370;
    case 46u: goto L_09D2A374;
    case 47u: goto L_09D2A378;
    case 48u: goto L_09D2A37C;
    case 49u: goto L_09D2A380;
    case 50u: goto L_09D2A384;
    case 51u: goto L_09D2A388;
    case 52u: goto L_09D2A38C;
    case 53u: goto L_09D2A390;
    case 54u: goto L_09D2A394;
    case 55u: goto L_09D2A398;
    case 56u: goto L_09D2A39C;
    default:
        if (local_transfers == 0u) rt.unsupported(ctx.pc, 0u, "invalid internal function entry");
        else ctx.pc = local_pc;
        return;
    }
    }
L_09D2A2C0:
    aot_mem.aot_store32(ctx.gpr[16] + static_cast<std::uint32_t>(120), std::bit_cast<std::uint32_t>(ctx.fpr[2]));
    goto L_09D2A2C4;
L_09D2A2C4:
    aot_mem.aot_store32(ctx.gpr[16] + static_cast<std::uint32_t>(124), std::bit_cast<std::uint32_t>(ctx.fpr[1]));
    goto L_09D2A2C8;
L_09D2A2C8:
    aot_mem.aot_store32(ctx.gpr[16] + static_cast<std::uint32_t>(128), std::bit_cast<std::uint32_t>(ctx.fpr[0]));
    goto L_09D2A2CC;
L_09D2A2CC:
    ctx.gpr[31] = (aot_mem.aot_load32(ctx.gpr[29] + static_cast<std::uint32_t>(12)));
    goto L_09D2A2D0;
L_09D2A2D0:
    ctx.gpr[16] = (aot_mem.aot_load32(ctx.gpr[29] + static_cast<std::uint32_t>(8)));
    goto L_09D2A2D4;
L_09D2A2D4:
    jump_target = ctx.gpr[31];
    ctx.gpr[29] = (ctx.gpr[29] + static_cast<std::uint32_t>(32));
    local_pc = jump_target;
    if (++local_transfers < 2048u) { entry_id = 0u; goto LOCAL_DISPATCH; }
    ctx.pc = jump_target;
    return;
L_09D2A2D8:
    ctx.gpr[29] = (ctx.gpr[29] + static_cast<std::uint32_t>(32));
    goto L_09D2A2DC;
L_09D2A2DC:
    // nop
    goto L_09D2A2E0;
L_09D2A2E0:
    ctx.gpr[29] = (ctx.gpr[29] + static_cast<std::uint32_t>(-32));
    goto L_09D2A2E4;
L_09D2A2E4:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(12), ctx.gpr[31]);
    goto L_09D2A2E8;
L_09D2A2E8:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(8), ctx.gpr[16]);
    goto L_09D2A2EC;
L_09D2A2EC:
    aot_mem.aot_store8(ctx.gpr[4] + static_cast<std::uint32_t>(20), static_cast<std::uint8_t>(0u));
    goto L_09D2A2F0;
L_09D2A2F0:
    ctx.gpr[2] = (2515u << 16u);
    goto L_09D2A2F4;
L_09D2A2F4:
    ctx.fpr[1] = std::bit_cast<float>(aot_mem.aot_load32(ctx.gpr[2] + static_cast<std::uint32_t>(5440)));
    goto L_09D2A2F8;
L_09D2A2F8:
    ctx.gpr[16] = (ctx.gpr[4] + 0u);
    goto L_09D2A2FC;
L_09D2A2FC:
    ctx.gpr[4] = (ctx.gpr[29] + static_cast<std::uint32_t>(20));
    goto L_09D2A300;
L_09D2A300:
    ctx.gpr[2] = (2515u << 16u);
    goto L_09D2A304;
L_09D2A304:
    ctx.fpr[0] = std::bit_cast<float>(aot_mem.aot_load32(ctx.gpr[2] + static_cast<std::uint32_t>(5444)));
    goto L_09D2A308;
L_09D2A308:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(20), std::bit_cast<std::uint32_t>(ctx.fpr[1]));
    goto L_09D2A30C;
L_09D2A30C:
    ctx.gpr[2] = (2515u << 16u);
    goto L_09D2A310;
L_09D2A310:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(24), std::bit_cast<std::uint32_t>(ctx.fpr[0]));
    goto L_09D2A314;
L_09D2A314:
    ctx.fpr[0] = std::bit_cast<float>(aot_mem.aot_load32(ctx.gpr[2] + static_cast<std::uint32_t>(5448)));
    goto L_09D2A318;
L_09D2A318:
    ctx.gpr[31] = (0x09D2A320u);
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(28), std::bit_cast<std::uint32_t>(ctx.fpr[0]));
    ctx.pc = 0x0890C4F8u;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D2A320u) goto L_09D2A320;
    return;
L_09D2A31C:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(28), std::bit_cast<std::uint32_t>(ctx.fpr[0]));
    goto L_09D2A320;
L_09D2A320:
    if (ctx.gpr[2] == 0u) {
    ctx.gpr[31] = (aot_mem.aot_load32(ctx.gpr[29] + static_cast<std::uint32_t>(12)));
        goto L_09D2A344;
    }
    goto L_09D2A328;
L_09D2A324:
    ctx.gpr[31] = (aot_mem.aot_load32(ctx.gpr[29] + static_cast<std::uint32_t>(12)));
    goto L_09D2A328;
L_09D2A328:
    ctx.fpr[2] = std::bit_cast<float>(aot_mem.aot_load32(ctx.gpr[29] + static_cast<std::uint32_t>(20)));
    goto L_09D2A32C;
L_09D2A32C:
    ctx.fpr[1] = std::bit_cast<float>(aot_mem.aot_load32(ctx.gpr[29] + static_cast<std::uint32_t>(24)));
    goto L_09D2A330;
L_09D2A330:
    ctx.fpr[0] = std::bit_cast<float>(aot_mem.aot_load32(ctx.gpr[29] + static_cast<std::uint32_t>(28)));
    goto L_09D2A334;
L_09D2A334:
    aot_mem.aot_store32(ctx.gpr[16] + static_cast<std::uint32_t>(120), std::bit_cast<std::uint32_t>(ctx.fpr[2]));
    goto L_09D2A338;
L_09D2A338:
    aot_mem.aot_store32(ctx.gpr[16] + static_cast<std::uint32_t>(124), std::bit_cast<std::uint32_t>(ctx.fpr[1]));
    goto L_09D2A33C;
L_09D2A33C:
    aot_mem.aot_store32(ctx.gpr[16] + static_cast<std::uint32_t>(128), std::bit_cast<std::uint32_t>(ctx.fpr[0]));
    goto L_09D2A340;
L_09D2A340:
    ctx.gpr[31] = (aot_mem.aot_load32(ctx.gpr[29] + static_cast<std::uint32_t>(12)));
    goto L_09D2A344;
L_09D2A344:
    ctx.gpr[16] = (aot_mem.aot_load32(ctx.gpr[29] + static_cast<std::uint32_t>(8)));
    goto L_09D2A348;
L_09D2A348:
    jump_target = ctx.gpr[31];
    ctx.gpr[29] = (ctx.gpr[29] + static_cast<std::uint32_t>(32));
    local_pc = jump_target;
    if (++local_transfers < 2048u) { entry_id = 0u; goto LOCAL_DISPATCH; }
    ctx.pc = jump_target;
    return;
L_09D2A34C:
    ctx.gpr[29] = (ctx.gpr[29] + static_cast<std::uint32_t>(32));
    goto L_09D2A350;
L_09D2A350:
    ctx.gpr[25] = (aot_mem.aot_load32(ctx.gpr[4] + static_cast<std::uint32_t>(0)));
    goto L_09D2A354;
L_09D2A354:
    ctx.gpr[25] = (aot_mem.aot_load32(ctx.gpr[25] + static_cast<std::uint32_t>(36)));
    goto L_09D2A358;
L_09D2A358:
    jump_target = ctx.gpr[25];
    // nop
    local_pc = jump_target;
    if (++local_transfers < 2048u) { entry_id = 0u; goto LOCAL_DISPATCH; }
    ctx.pc = jump_target;
    return;
L_09D2A35C:
    // nop
    goto L_09D2A360;
L_09D2A360:
    ctx.gpr[25] = (aot_mem.aot_load32(ctx.gpr[4] + static_cast<std::uint32_t>(0)));
    goto L_09D2A364;
L_09D2A364:
    ctx.gpr[25] = (aot_mem.aot_load32(ctx.gpr[25] + static_cast<std::uint32_t>(40)));
    goto L_09D2A368;
L_09D2A368:
    jump_target = ctx.gpr[25];
    // nop
    local_pc = jump_target;
    if (++local_transfers < 2048u) { entry_id = 0u; goto LOCAL_DISPATCH; }
    ctx.pc = jump_target;
    return;
L_09D2A36C:
    // nop
    goto L_09D2A370;
L_09D2A370:
    ctx.gpr[25] = (aot_mem.aot_load32(ctx.gpr[4] + static_cast<std::uint32_t>(0)));
    goto L_09D2A374;
L_09D2A374:
    ctx.gpr[25] = (aot_mem.aot_load32(ctx.gpr[25] + static_cast<std::uint32_t>(40)));
    goto L_09D2A378;
L_09D2A378:
    jump_target = ctx.gpr[25];
    // nop
    local_pc = jump_target;
    if (++local_transfers < 2048u) { entry_id = 0u; goto LOCAL_DISPATCH; }
    ctx.pc = jump_target;
    return;
L_09D2A37C:
    // nop
    goto L_09D2A380;
L_09D2A380:
    ctx.gpr[25] = (aot_mem.aot_load32(ctx.gpr[4] + static_cast<std::uint32_t>(0)));
    goto L_09D2A384;
L_09D2A384:
    ctx.gpr[25] = (aot_mem.aot_load32(ctx.gpr[25] + static_cast<std::uint32_t>(40)));
    goto L_09D2A388;
L_09D2A388:
    jump_target = ctx.gpr[25];
    // nop
    local_pc = jump_target;
    if (++local_transfers < 2048u) { entry_id = 0u; goto LOCAL_DISPATCH; }
    ctx.pc = jump_target;
    return;
L_09D2A38C:
    // nop
    goto L_09D2A390;
L_09D2A390:
    // nop
    ctx.pc = 0x09B673F8u; (void)rt.invoke_chained_call(ctx, &aot_mem); return;
L_09D2A394:
    // nop
    goto L_09D2A398;
L_09D2A398:
    jump_target = ctx.gpr[31];
    // nop
    local_pc = jump_target;
    if (++local_transfers < 2048u) { entry_id = 0u; goto LOCAL_DISPATCH; }
    ctx.pc = jump_target;
    return;
L_09D2A39C:
    // nop
    ctx.pc = 0x09D2A3A0u; return;
}

void ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004(Runtime &rt, AllegrexContext &ctx) {
    auto aot_mem = rt.memory().aot_fast_view();
    ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004_entry(rt, ctx, 0u, aot_mem);
}

void register_ovl09D1A280_em15_2C9916E5A6F01D4E_unit_4(Runtime &runtime) {
    runtime.register_function(0x09D2A2C0u, &ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004, "recomp_unit_ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004");
    runtime.register_function(0x09D2A2C4u, &ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004, "recomp_unit_ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004");
    runtime.register_function(0x09D2A2C8u, &ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004, "recomp_unit_ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004");
    runtime.register_function(0x09D2A2CCu, &ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004, "recomp_unit_ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004");
    runtime.register_function(0x09D2A2D0u, &ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004, "recomp_unit_ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004");
    runtime.register_function(0x09D2A2D4u, &ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004, "recomp_unit_ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004");
    runtime.register_function(0x09D2A2D8u, &ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004, "recomp_unit_ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004");
    runtime.register_function(0x09D2A2DCu, &ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004, "recomp_unit_ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004");
    runtime.register_function(0x09D2A2E0u, &ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004, "recomp_unit_ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004");
    runtime.register_function(0x09D2A2E4u, &ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004, "recomp_unit_ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004");
    runtime.register_function(0x09D2A2E8u, &ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004, "recomp_unit_ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004");
    runtime.register_function(0x09D2A2ECu, &ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004, "recomp_unit_ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004");
    runtime.register_function(0x09D2A2F0u, &ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004, "recomp_unit_ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004");
    runtime.register_function(0x09D2A2F4u, &ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004, "recomp_unit_ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004");
    runtime.register_function(0x09D2A2F8u, &ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004, "recomp_unit_ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004");
    runtime.register_function(0x09D2A2FCu, &ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004, "recomp_unit_ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004");
    runtime.register_function(0x09D2A300u, &ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004, "recomp_unit_ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004");
    runtime.register_function(0x09D2A304u, &ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004, "recomp_unit_ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004");
    runtime.register_function(0x09D2A308u, &ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004, "recomp_unit_ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004");
    runtime.register_function(0x09D2A30Cu, &ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004, "recomp_unit_ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004");
    runtime.register_function(0x09D2A310u, &ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004, "recomp_unit_ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004");
    runtime.register_function(0x09D2A314u, &ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004, "recomp_unit_ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004");
    runtime.register_function(0x09D2A318u, &ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004, "recomp_unit_ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004");
    runtime.register_function(0x09D2A31Cu, &ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004, "recomp_unit_ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004");
    runtime.register_function(0x09D2A320u, &ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004, "recomp_unit_ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004");
    runtime.register_function(0x09D2A324u, &ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004, "recomp_unit_ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004");
    runtime.register_function(0x09D2A328u, &ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004, "recomp_unit_ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004");
    runtime.register_function(0x09D2A32Cu, &ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004, "recomp_unit_ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004");
    runtime.register_function(0x09D2A330u, &ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004, "recomp_unit_ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004");
    runtime.register_function(0x09D2A334u, &ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004, "recomp_unit_ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004");
    runtime.register_function(0x09D2A338u, &ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004, "recomp_unit_ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004");
    runtime.register_function(0x09D2A33Cu, &ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004, "recomp_unit_ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004");
    runtime.register_function(0x09D2A340u, &ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004, "recomp_unit_ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004");
    runtime.register_function(0x09D2A344u, &ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004, "recomp_unit_ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004");
    runtime.register_function(0x09D2A348u, &ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004, "recomp_unit_ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004");
    runtime.register_function(0x09D2A34Cu, &ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004, "recomp_unit_ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004");
    runtime.register_function(0x09D2A350u, &ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004, "recomp_unit_ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004");
    runtime.register_function(0x09D2A354u, &ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004, "recomp_unit_ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004");
    runtime.register_function(0x09D2A358u, &ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004, "recomp_unit_ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004");
    runtime.register_function(0x09D2A35Cu, &ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004, "recomp_unit_ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004");
    runtime.register_function(0x09D2A360u, &ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004, "recomp_unit_ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004");
    runtime.register_function(0x09D2A364u, &ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004, "recomp_unit_ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004");
    runtime.register_function(0x09D2A368u, &ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004, "recomp_unit_ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004");
    runtime.register_function(0x09D2A36Cu, &ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004, "recomp_unit_ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004");
    runtime.register_function(0x09D2A370u, &ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004, "recomp_unit_ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004");
    runtime.register_function(0x09D2A374u, &ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004, "recomp_unit_ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004");
    runtime.register_function(0x09D2A378u, &ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004, "recomp_unit_ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004");
    runtime.register_function(0x09D2A37Cu, &ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004, "recomp_unit_ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004");
    runtime.register_function(0x09D2A380u, &ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004, "recomp_unit_ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004");
    runtime.register_function(0x09D2A384u, &ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004, "recomp_unit_ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004");
    runtime.register_function(0x09D2A388u, &ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004, "recomp_unit_ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004");
    runtime.register_function(0x09D2A38Cu, &ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004, "recomp_unit_ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004");
    runtime.register_function(0x09D2A390u, &ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004, "recomp_unit_ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004");
    runtime.register_function(0x09D2A394u, &ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004, "recomp_unit_ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004");
    runtime.register_function(0x09D2A398u, &ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004, "recomp_unit_ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004");
    runtime.register_function(0x09D2A39Cu, &ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004, "recomp_unit_ovl09D1A280_em15_2C9916E5A6F01D4E_unit_0004");
}
} // namespace psprecomp
