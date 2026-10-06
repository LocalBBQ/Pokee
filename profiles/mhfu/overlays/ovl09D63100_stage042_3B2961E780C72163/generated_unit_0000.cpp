#include "psprecomp/runtime.hpp"
#include "ovl09D63100_stage042_3B2961E780C72163_units.hpp"
#include <bit>
#include <cmath>
#include <cstdint>
#include <limits>

namespace psprecomp {
static const std::uint16_t kEntryIds_ovl09D63100_stage042_3B2961E780C72163_unit_0000[320] = {
    1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32,
    33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64,
    65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96,
    97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128,
    129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160,
    161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192,
    193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224,
    225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255, 256,
    257, 258, 259, 260, 261, 262, 263, 264, 265, 266, 267, 268, 269, 270, 271, 272, 273, 274, 275, 276, 277, 278, 279, 280, 281, 282, 283, 284, 285, 286, 287, 288,
    289, 290, 291, 292, 293, 294, 295, 296, 297, 298, 299, 300, 301, 302, 303, 304, 305, 306, 307, 308, 309, 310, 311, 312, 313, 314, 315, 316, 317, 318, 319, 320,
};
void ovl09D63100_stage042_3B2961E780C72163_unit_0000_entry(Runtime &rt, AllegrexContext &ctx, std::uint16_t direct_entry_id, GuestMemory::AotFastView &aot_mem) {
    std::uint32_t jump_target = 0u;
    std::uint32_t local_transfers = 0u;
    std::uint32_t local_pc = ctx.pc;
    std::uint32_t entry_id = direct_entry_id;
LOCAL_DISPATCH:
    {
    if (entry_id == 0u) {
        const std::uint32_t entry_delta = local_pc - 0x09D63140u;
        entry_id = (entry_delta < 1280u && (entry_delta & 3u) == 0u) ? kEntryIds_ovl09D63100_stage042_3B2961E780C72163_unit_0000[entry_delta >> 2u] : 0u;
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
    case 183u: goto L_09D63418;
    case 184u: goto L_09D6341C;
    case 185u: goto L_09D63420;
    case 186u: goto L_09D63424;
    case 187u: goto L_09D63428;
    case 188u: goto L_09D6342C;
    case 189u: goto L_09D63430;
    case 190u: goto L_09D63434;
    case 191u: goto L_09D63438;
    case 192u: goto L_09D6343C;
    case 193u: goto L_09D63440;
    case 194u: goto L_09D63444;
    case 195u: goto L_09D63448;
    case 196u: goto L_09D6344C;
    case 197u: goto L_09D63450;
    case 198u: goto L_09D63454;
    case 199u: goto L_09D63458;
    case 200u: goto L_09D6345C;
    case 201u: goto L_09D63460;
    case 202u: goto L_09D63464;
    case 203u: goto L_09D63468;
    case 204u: goto L_09D6346C;
    case 205u: goto L_09D63470;
    case 206u: goto L_09D63474;
    case 207u: goto L_09D63478;
    case 208u: goto L_09D6347C;
    case 209u: goto L_09D63480;
    case 210u: goto L_09D63484;
    case 211u: goto L_09D63488;
    case 212u: goto L_09D6348C;
    case 213u: goto L_09D63490;
    case 214u: goto L_09D63494;
    case 215u: goto L_09D63498;
    case 216u: goto L_09D6349C;
    case 217u: goto L_09D634A0;
    case 218u: goto L_09D634A4;
    case 219u: goto L_09D634A8;
    case 220u: goto L_09D634AC;
    case 221u: goto L_09D634B0;
    case 222u: goto L_09D634B4;
    case 223u: goto L_09D634B8;
    case 224u: goto L_09D634BC;
    case 225u: goto L_09D634C0;
    case 226u: goto L_09D634C4;
    case 227u: goto L_09D634C8;
    case 228u: goto L_09D634CC;
    case 229u: goto L_09D634D0;
    case 230u: goto L_09D634D4;
    case 231u: goto L_09D634D8;
    case 232u: goto L_09D634DC;
    case 233u: goto L_09D634E0;
    case 234u: goto L_09D634E4;
    case 235u: goto L_09D634E8;
    case 236u: goto L_09D634EC;
    case 237u: goto L_09D634F0;
    case 238u: goto L_09D634F4;
    case 239u: goto L_09D634F8;
    case 240u: goto L_09D634FC;
    case 241u: goto L_09D63500;
    case 242u: goto L_09D63504;
    case 243u: goto L_09D63508;
    case 244u: goto L_09D6350C;
    case 245u: goto L_09D63510;
    case 246u: goto L_09D63514;
    case 247u: goto L_09D63518;
    case 248u: goto L_09D6351C;
    case 249u: goto L_09D63520;
    case 250u: goto L_09D63524;
    case 251u: goto L_09D63528;
    case 252u: goto L_09D6352C;
    case 253u: goto L_09D63530;
    case 254u: goto L_09D63534;
    case 255u: goto L_09D63538;
    case 256u: goto L_09D6353C;
    case 257u: goto L_09D63540;
    case 258u: goto L_09D63544;
    case 259u: goto L_09D63548;
    case 260u: goto L_09D6354C;
    case 261u: goto L_09D63550;
    case 262u: goto L_09D63554;
    case 263u: goto L_09D63558;
    case 264u: goto L_09D6355C;
    case 265u: goto L_09D63560;
    case 266u: goto L_09D63564;
    case 267u: goto L_09D63568;
    case 268u: goto L_09D6356C;
    case 269u: goto L_09D63570;
    case 270u: goto L_09D63574;
    case 271u: goto L_09D63578;
    case 272u: goto L_09D6357C;
    case 273u: goto L_09D63580;
    case 274u: goto L_09D63584;
    case 275u: goto L_09D63588;
    case 276u: goto L_09D6358C;
    case 277u: goto L_09D63590;
    case 278u: goto L_09D63594;
    case 279u: goto L_09D63598;
    case 280u: goto L_09D6359C;
    case 281u: goto L_09D635A0;
    case 282u: goto L_09D635A4;
    case 283u: goto L_09D635A8;
    case 284u: goto L_09D635AC;
    case 285u: goto L_09D635B0;
    case 286u: goto L_09D635B4;
    case 287u: goto L_09D635B8;
    case 288u: goto L_09D635BC;
    case 289u: goto L_09D635C0;
    case 290u: goto L_09D635C4;
    case 291u: goto L_09D635C8;
    case 292u: goto L_09D635CC;
    case 293u: goto L_09D635D0;
    case 294u: goto L_09D635D4;
    case 295u: goto L_09D635D8;
    case 296u: goto L_09D635DC;
    case 297u: goto L_09D635E0;
    case 298u: goto L_09D635E4;
    case 299u: goto L_09D635E8;
    case 300u: goto L_09D635EC;
    case 301u: goto L_09D635F0;
    case 302u: goto L_09D635F4;
    case 303u: goto L_09D635F8;
    case 304u: goto L_09D635FC;
    case 305u: goto L_09D63600;
    case 306u: goto L_09D63604;
    case 307u: goto L_09D63608;
    case 308u: goto L_09D6360C;
    case 309u: goto L_09D63610;
    case 310u: goto L_09D63614;
    case 311u: goto L_09D63618;
    case 312u: goto L_09D6361C;
    case 313u: goto L_09D63620;
    case 314u: goto L_09D63624;
    case 315u: goto L_09D63628;
    case 316u: goto L_09D6362C;
    case 317u: goto L_09D63630;
    case 318u: goto L_09D63634;
    case 319u: goto L_09D63638;
    case 320u: goto L_09D6363C;
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
    ctx.gpr[2] = (ctx.gpr[2] + static_cast<std::uint32_t>(5632));
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
    ctx.gpr[2] = (ctx.gpr[2] + static_cast<std::uint32_t>(5632));
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
    goto L_09D63570;
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
    ctx.gpr[29] = (ctx.gpr[29] + static_cast<std::uint32_t>(-32));
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
    ctx.gpr[5] = (ctx.gpr[5] + static_cast<std::uint32_t>(14608));
    ctx.pc = 0x088C354Cu;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D6323Cu) goto L_09D6323C;
    return;
L_09D63238:
    ctx.gpr[5] = (ctx.gpr[5] + static_cast<std::uint32_t>(14608));
    goto L_09D6323C;
L_09D6323C:
    ctx.gpr[2] = (2469u << 16u);
    goto L_09D63240;
L_09D63240:
    ctx.gpr[4] = (aot_mem.aot_load32(ctx.gpr[2] + static_cast<std::uint32_t>(-3680)));
    goto L_09D63244;
L_09D63244:
    ctx.gpr[5] = (2518u << 16u);
    goto L_09D63248;
L_09D63248:
    ctx.gpr[31] = (0x09D63250u);
    ctx.gpr[5] = (ctx.gpr[5] + static_cast<std::uint32_t>(14688));
    ctx.pc = 0x088C354Cu;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D63250u) goto L_09D63250;
    return;
L_09D6324C:
    ctx.gpr[5] = (ctx.gpr[5] + static_cast<std::uint32_t>(14688));
    goto L_09D63250;
L_09D63250:
    ctx.gpr[2] = (2469u << 16u);
    goto L_09D63254;
L_09D63254:
    ctx.gpr[4] = (aot_mem.aot_load32(ctx.gpr[2] + static_cast<std::uint32_t>(-3680)));
    goto L_09D63258;
L_09D63258:
    ctx.gpr[6] = (2518u << 16u);
    goto L_09D6325C;
L_09D6325C:
    ctx.gpr[5] = (0u + 0u);
    goto L_09D63260;
L_09D63260:
    ctx.gpr[31] = (0x09D63268u);
    ctx.gpr[6] = (ctx.gpr[6] + static_cast<std::uint32_t>(14768));
    ctx.pc = 0x088C3640u;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D63268u) goto L_09D63268;
    return;
L_09D63264:
    ctx.gpr[6] = (ctx.gpr[6] + static_cast<std::uint32_t>(14768));
    goto L_09D63268;
L_09D63268:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(0), 0u);
    goto L_09D6326C;
L_09D6326C:
    ctx.gpr[2] = (2518u << 16u);
    goto L_09D63270;
L_09D63270:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(4), 0u);
    goto L_09D63274;
L_09D63274:
    ctx.gpr[2] = (ctx.gpr[2] + static_cast<std::uint32_t>(14976));
    goto L_09D63278;
L_09D63278:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(8), ctx.gpr[2]);
    goto L_09D6327C;
L_09D6327C:
    ctx.gpr[2] = (2469u << 16u);
    goto L_09D63280;
L_09D63280:
    ctx.gpr[7] = (2518u << 16u);
    goto L_09D63284;
L_09D63284:
    ctx.gpr[4] = (aot_mem.aot_load32(ctx.gpr[2] + static_cast<std::uint32_t>(-3680)));
    goto L_09D63288;
L_09D63288:
    ctx.gpr[5] = (0u + static_cast<std::uint32_t>(1));
    goto L_09D6328C;
L_09D6328C:
    ctx.gpr[6] = (0u + static_cast<std::uint32_t>(112));
    goto L_09D63290;
L_09D63290:
    ctx.gpr[7] = (ctx.gpr[7] + static_cast<std::uint32_t>(14800));
    goto L_09D63294;
L_09D63294:
    ctx.gpr[8] = (0u + static_cast<std::uint32_t>(7));
    goto L_09D63298;
L_09D63298:
    ctx.gpr[9] = (0u + static_cast<std::uint32_t>(4));
    goto L_09D6329C;
L_09D6329C:
    ctx.gpr[10] = (0u + 0u);
    goto L_09D632A0;
L_09D632A0:
    ctx.gpr[31] = (0x09D632A8u);
    ctx.gpr[11] = (0u + 0u);
    ctx.pc = 0x088C3C54u;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D632A8u) goto L_09D632A8;
    return;
L_09D632A4:
    ctx.gpr[11] = (0u + 0u);
    goto L_09D632A8;
L_09D632A8:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(0), 0u);
    goto L_09D632AC;
L_09D632AC:
    ctx.gpr[2] = (2518u << 16u);
    goto L_09D632B0;
L_09D632B0:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(4), 0u);
    goto L_09D632B4;
L_09D632B4:
    ctx.gpr[2] = (ctx.gpr[2] + static_cast<std::uint32_t>(14992));
    goto L_09D632B8;
L_09D632B8:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(8), ctx.gpr[2]);
    goto L_09D632BC;
L_09D632BC:
    ctx.gpr[2] = (2469u << 16u);
    goto L_09D632C0;
L_09D632C0:
    ctx.gpr[7] = (2518u << 16u);
    goto L_09D632C4;
L_09D632C4:
    ctx.gpr[4] = (aot_mem.aot_load32(ctx.gpr[2] + static_cast<std::uint32_t>(-3680)));
    goto L_09D632C8;
L_09D632C8:
    ctx.gpr[5] = (0u + static_cast<std::uint32_t>(1));
    goto L_09D632CC;
L_09D632CC:
    ctx.gpr[6] = (0u + static_cast<std::uint32_t>(112));
    goto L_09D632D0;
L_09D632D0:
    ctx.gpr[7] = (ctx.gpr[7] + static_cast<std::uint32_t>(14816));
    goto L_09D632D4;
L_09D632D4:
    ctx.gpr[8] = (0u + static_cast<std::uint32_t>(7));
    goto L_09D632D8;
L_09D632D8:
    ctx.gpr[9] = (0u + static_cast<std::uint32_t>(4));
    goto L_09D632DC;
L_09D632DC:
    ctx.gpr[10] = (0u + 0u);
    goto L_09D632E0;
L_09D632E0:
    ctx.gpr[31] = (0x09D632E8u);
    ctx.gpr[11] = (0u + 0u);
    ctx.pc = 0x088C3C54u;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D632E8u) goto L_09D632E8;
    return;
L_09D632E4:
    ctx.gpr[11] = (0u + 0u);
    goto L_09D632E8;
L_09D632E8:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(0), 0u);
    goto L_09D632EC;
L_09D632EC:
    ctx.gpr[2] = (2518u << 16u);
    goto L_09D632F0;
L_09D632F0:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(4), 0u);
    goto L_09D632F4;
L_09D632F4:
    ctx.gpr[2] = (ctx.gpr[2] + static_cast<std::uint32_t>(15008));
    goto L_09D632F8;
L_09D632F8:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(8), ctx.gpr[2]);
    goto L_09D632FC;
L_09D632FC:
    ctx.gpr[2] = (2469u << 16u);
    goto L_09D63300;
L_09D63300:
    ctx.gpr[7] = (2518u << 16u);
    goto L_09D63304;
L_09D63304:
    ctx.gpr[4] = (aot_mem.aot_load32(ctx.gpr[2] + static_cast<std::uint32_t>(-3680)));
    goto L_09D63308;
L_09D63308:
    ctx.gpr[5] = (0u + static_cast<std::uint32_t>(1));
    goto L_09D6330C;
L_09D6330C:
    ctx.gpr[6] = (0u + static_cast<std::uint32_t>(112));
    goto L_09D63310;
L_09D63310:
    ctx.gpr[7] = (ctx.gpr[7] + static_cast<std::uint32_t>(14832));
    goto L_09D63314;
L_09D63314:
    ctx.gpr[8] = (0u + static_cast<std::uint32_t>(7));
    goto L_09D63318;
L_09D63318:
    ctx.gpr[9] = (0u + static_cast<std::uint32_t>(4));
    goto L_09D6331C;
L_09D6331C:
    ctx.gpr[10] = (0u + 0u);
    goto L_09D63320;
L_09D63320:
    ctx.gpr[31] = (0x09D63328u);
    ctx.gpr[11] = (0u + 0u);
    ctx.pc = 0x088C3C54u;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D63328u) goto L_09D63328;
    return;
L_09D63324:
    ctx.gpr[11] = (0u + 0u);
    goto L_09D63328;
L_09D63328:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(0), 0u);
    goto L_09D6332C;
L_09D6332C:
    ctx.gpr[2] = (2518u << 16u);
    goto L_09D63330;
L_09D63330:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(4), 0u);
    goto L_09D63334;
L_09D63334:
    ctx.gpr[2] = (ctx.gpr[2] + static_cast<std::uint32_t>(15024));
    goto L_09D63338;
L_09D63338:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(8), ctx.gpr[2]);
    goto L_09D6333C;
L_09D6333C:
    ctx.gpr[2] = (2469u << 16u);
    goto L_09D63340;
L_09D63340:
    ctx.gpr[7] = (2518u << 16u);
    goto L_09D63344;
L_09D63344:
    ctx.gpr[4] = (aot_mem.aot_load32(ctx.gpr[2] + static_cast<std::uint32_t>(-3680)));
    goto L_09D63348;
L_09D63348:
    ctx.gpr[5] = (0u + static_cast<std::uint32_t>(1));
    goto L_09D6334C;
L_09D6334C:
    ctx.gpr[6] = (0u + static_cast<std::uint32_t>(112));
    goto L_09D63350;
L_09D63350:
    ctx.gpr[7] = (ctx.gpr[7] + static_cast<std::uint32_t>(14848));
    goto L_09D63354;
L_09D63354:
    ctx.gpr[8] = (0u + static_cast<std::uint32_t>(7));
    goto L_09D63358;
L_09D63358:
    ctx.gpr[9] = (0u + static_cast<std::uint32_t>(4));
    goto L_09D6335C;
L_09D6335C:
    ctx.gpr[10] = (0u + 0u);
    goto L_09D63360;
L_09D63360:
    ctx.gpr[31] = (0x09D63368u);
    ctx.gpr[11] = (0u + 0u);
    ctx.pc = 0x088C3C54u;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D63368u) goto L_09D63368;
    return;
L_09D63364:
    ctx.gpr[11] = (0u + 0u);
    goto L_09D63368;
L_09D63368:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(0), 0u);
    goto L_09D6336C;
L_09D6336C:
    ctx.gpr[2] = (2518u << 16u);
    goto L_09D63370;
L_09D63370:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(4), 0u);
    goto L_09D63374;
L_09D63374:
    ctx.gpr[2] = (ctx.gpr[2] + static_cast<std::uint32_t>(15040));
    goto L_09D63378;
L_09D63378:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(8), ctx.gpr[2]);
    goto L_09D6337C;
L_09D6337C:
    ctx.gpr[2] = (2469u << 16u);
    goto L_09D63380;
L_09D63380:
    ctx.gpr[7] = (2518u << 16u);
    goto L_09D63384;
L_09D63384:
    ctx.gpr[4] = (aot_mem.aot_load32(ctx.gpr[2] + static_cast<std::uint32_t>(-3680)));
    goto L_09D63388;
L_09D63388:
    ctx.gpr[5] = (0u + static_cast<std::uint32_t>(1));
    goto L_09D6338C;
L_09D6338C:
    ctx.gpr[6] = (0u + static_cast<std::uint32_t>(112));
    goto L_09D63390;
L_09D63390:
    ctx.gpr[7] = (ctx.gpr[7] + static_cast<std::uint32_t>(14864));
    goto L_09D63394;
L_09D63394:
    ctx.gpr[8] = (0u + static_cast<std::uint32_t>(7));
    goto L_09D63398;
L_09D63398:
    ctx.gpr[9] = (0u + static_cast<std::uint32_t>(4));
    goto L_09D6339C;
L_09D6339C:
    ctx.gpr[10] = (0u + 0u);
    goto L_09D633A0;
L_09D633A0:
    ctx.gpr[31] = (0x09D633A8u);
    ctx.gpr[11] = (0u + 0u);
    ctx.pc = 0x088C3C54u;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D633A8u) goto L_09D633A8;
    return;
L_09D633A4:
    ctx.gpr[11] = (0u + 0u);
    goto L_09D633A8;
L_09D633A8:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(0), 0u);
    goto L_09D633AC;
L_09D633AC:
    ctx.gpr[2] = (2518u << 16u);
    goto L_09D633B0;
L_09D633B0:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(4), 0u);
    goto L_09D633B4;
L_09D633B4:
    ctx.gpr[2] = (ctx.gpr[2] + static_cast<std::uint32_t>(15056));
    goto L_09D633B8;
L_09D633B8:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(8), ctx.gpr[2]);
    goto L_09D633BC;
L_09D633BC:
    ctx.gpr[2] = (2469u << 16u);
    goto L_09D633C0;
L_09D633C0:
    ctx.gpr[7] = (2518u << 16u);
    goto L_09D633C4;
L_09D633C4:
    ctx.gpr[4] = (aot_mem.aot_load32(ctx.gpr[2] + static_cast<std::uint32_t>(-3680)));
    goto L_09D633C8;
L_09D633C8:
    ctx.gpr[5] = (0u + static_cast<std::uint32_t>(1));
    goto L_09D633CC;
L_09D633CC:
    ctx.gpr[6] = (0u + static_cast<std::uint32_t>(112));
    goto L_09D633D0;
L_09D633D0:
    ctx.gpr[7] = (ctx.gpr[7] + static_cast<std::uint32_t>(14880));
    goto L_09D633D4;
L_09D633D4:
    ctx.gpr[8] = (0u + static_cast<std::uint32_t>(7));
    goto L_09D633D8;
L_09D633D8:
    ctx.gpr[9] = (0u + static_cast<std::uint32_t>(4));
    goto L_09D633DC;
L_09D633DC:
    ctx.gpr[10] = (0u + 0u);
    goto L_09D633E0;
L_09D633E0:
    ctx.gpr[31] = (0x09D633E8u);
    ctx.gpr[11] = (0u + 0u);
    ctx.pc = 0x088C3C54u;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D633E8u) goto L_09D633E8;
    return;
L_09D633E4:
    ctx.gpr[11] = (0u + 0u);
    goto L_09D633E8;
L_09D633E8:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(0), 0u);
    goto L_09D633EC;
L_09D633EC:
    ctx.gpr[2] = (2518u << 16u);
    goto L_09D633F0;
L_09D633F0:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(4), 0u);
    goto L_09D633F4;
L_09D633F4:
    ctx.gpr[2] = (ctx.gpr[2] + static_cast<std::uint32_t>(15072));
    goto L_09D633F8;
L_09D633F8:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(8), ctx.gpr[2]);
    goto L_09D633FC;
L_09D633FC:
    ctx.gpr[2] = (2469u << 16u);
    goto L_09D63400;
L_09D63400:
    ctx.gpr[7] = (2518u << 16u);
    goto L_09D63404;
L_09D63404:
    ctx.gpr[4] = (aot_mem.aot_load32(ctx.gpr[2] + static_cast<std::uint32_t>(-3680)));
    goto L_09D63408;
L_09D63408:
    ctx.gpr[5] = (0u + static_cast<std::uint32_t>(1));
    goto L_09D6340C;
L_09D6340C:
    ctx.gpr[6] = (0u + static_cast<std::uint32_t>(112));
    goto L_09D63410;
L_09D63410:
    ctx.gpr[7] = (ctx.gpr[7] + static_cast<std::uint32_t>(14896));
    goto L_09D63414;
L_09D63414:
    ctx.gpr[8] = (0u + static_cast<std::uint32_t>(7));
    goto L_09D63418;
L_09D63418:
    ctx.gpr[9] = (0u + static_cast<std::uint32_t>(4));
    goto L_09D6341C;
L_09D6341C:
    ctx.gpr[10] = (0u + 0u);
    goto L_09D63420;
L_09D63420:
    ctx.gpr[31] = (0x09D63428u);
    ctx.gpr[11] = (0u + 0u);
    ctx.pc = 0x088C3C54u;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D63428u) goto L_09D63428;
    return;
L_09D63424:
    ctx.gpr[11] = (0u + 0u);
    goto L_09D63428;
L_09D63428:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(0), 0u);
    goto L_09D6342C;
L_09D6342C:
    ctx.gpr[2] = (2518u << 16u);
    goto L_09D63430;
L_09D63430:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(4), 0u);
    goto L_09D63434;
L_09D63434:
    ctx.gpr[2] = (ctx.gpr[2] + static_cast<std::uint32_t>(15088));
    goto L_09D63438;
L_09D63438:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(8), ctx.gpr[2]);
    goto L_09D6343C;
L_09D6343C:
    ctx.gpr[2] = (2469u << 16u);
    goto L_09D63440;
L_09D63440:
    ctx.gpr[7] = (2518u << 16u);
    goto L_09D63444;
L_09D63444:
    ctx.gpr[4] = (aot_mem.aot_load32(ctx.gpr[2] + static_cast<std::uint32_t>(-3680)));
    goto L_09D63448;
L_09D63448:
    ctx.gpr[5] = (0u + static_cast<std::uint32_t>(1));
    goto L_09D6344C;
L_09D6344C:
    ctx.gpr[6] = (0u + static_cast<std::uint32_t>(112));
    goto L_09D63450;
L_09D63450:
    ctx.gpr[7] = (ctx.gpr[7] + static_cast<std::uint32_t>(14912));
    goto L_09D63454;
L_09D63454:
    ctx.gpr[8] = (0u + static_cast<std::uint32_t>(7));
    goto L_09D63458;
L_09D63458:
    ctx.gpr[9] = (0u + static_cast<std::uint32_t>(4));
    goto L_09D6345C;
L_09D6345C:
    ctx.gpr[10] = (0u + 0u);
    goto L_09D63460;
L_09D63460:
    ctx.gpr[31] = (0x09D63468u);
    ctx.gpr[11] = (0u + 0u);
    ctx.pc = 0x088C3C54u;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D63468u) goto L_09D63468;
    return;
L_09D63464:
    ctx.gpr[11] = (0u + 0u);
    goto L_09D63468;
L_09D63468:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(0), 0u);
    goto L_09D6346C;
L_09D6346C:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(4), 0u);
    goto L_09D63470;
L_09D63470:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(8), 0u);
    goto L_09D63474;
L_09D63474:
    ctx.gpr[2] = (2469u << 16u);
    goto L_09D63478;
L_09D63478:
    ctx.gpr[5] = (0u + static_cast<std::uint32_t>(1));
    goto L_09D6347C;
L_09D6347C:
    ctx.gpr[7] = (2518u << 16u);
    goto L_09D63480;
L_09D63480:
    ctx.gpr[4] = (aot_mem.aot_load32(ctx.gpr[2] + static_cast<std::uint32_t>(-3680)));
    goto L_09D63484;
L_09D63484:
    ctx.gpr[6] = (0u + static_cast<std::uint32_t>(72));
    goto L_09D63488;
L_09D63488:
    ctx.gpr[7] = (ctx.gpr[7] + static_cast<std::uint32_t>(14928));
    goto L_09D6348C;
L_09D6348C:
    ctx.gpr[8] = (0u + static_cast<std::uint32_t>(7));
    goto L_09D63490;
L_09D63490:
    ctx.gpr[9] = (ctx.gpr[5] + 0u);
    goto L_09D63494;
L_09D63494:
    ctx.gpr[10] = (0u + 0u);
    goto L_09D63498;
L_09D63498:
    ctx.gpr[31] = (0x09D634A0u);
    ctx.gpr[11] = (0u + static_cast<std::uint32_t>(90));
    ctx.pc = 0x088C3C54u;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D634A0u) goto L_09D634A0;
    return;
L_09D6349C:
    ctx.gpr[11] = (0u + static_cast<std::uint32_t>(90));
    goto L_09D634A0;
L_09D634A0:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(0), 0u);
    goto L_09D634A4;
L_09D634A4:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(4), 0u);
    goto L_09D634A8;
L_09D634A8:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(8), 0u);
    goto L_09D634AC;
L_09D634AC:
    ctx.gpr[2] = (2469u << 16u);
    goto L_09D634B0;
L_09D634B0:
    ctx.gpr[7] = (2518u << 16u);
    goto L_09D634B4;
L_09D634B4:
    ctx.gpr[4] = (aot_mem.aot_load32(ctx.gpr[2] + static_cast<std::uint32_t>(-3680)));
    goto L_09D634B8;
L_09D634B8:
    ctx.gpr[5] = (0u + static_cast<std::uint32_t>(1));
    goto L_09D634BC;
L_09D634BC:
    ctx.gpr[6] = (0u + static_cast<std::uint32_t>(72));
    goto L_09D634C0;
L_09D634C0:
    ctx.gpr[7] = (ctx.gpr[7] + static_cast<std::uint32_t>(14944));
    goto L_09D634C4;
L_09D634C4:
    ctx.gpr[8] = (0u + static_cast<std::uint32_t>(7));
    goto L_09D634C8;
L_09D634C8:
    ctx.gpr[9] = (0u + static_cast<std::uint32_t>(2));
    goto L_09D634CC;
L_09D634CC:
    ctx.gpr[10] = (0u + 0u);
    goto L_09D634D0;
L_09D634D0:
    ctx.gpr[31] = (0x09D634D8u);
    ctx.gpr[11] = (0u + static_cast<std::uint32_t>(32));
    ctx.pc = 0x088C3C54u;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D634D8u) goto L_09D634D8;
    return;
L_09D634D4:
    ctx.gpr[11] = (0u + static_cast<std::uint32_t>(32));
    goto L_09D634D8;
L_09D634D8:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(0), 0u);
    goto L_09D634DC;
L_09D634DC:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(4), 0u);
    goto L_09D634E0;
L_09D634E0:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(8), 0u);
    goto L_09D634E4;
L_09D634E4:
    ctx.gpr[2] = (2469u << 16u);
    goto L_09D634E8;
L_09D634E8:
    ctx.gpr[7] = (2518u << 16u);
    goto L_09D634EC;
L_09D634EC:
    ctx.gpr[4] = (aot_mem.aot_load32(ctx.gpr[2] + static_cast<std::uint32_t>(-3680)));
    goto L_09D634F0;
L_09D634F0:
    ctx.gpr[5] = (0u + static_cast<std::uint32_t>(1));
    goto L_09D634F4;
L_09D634F4:
    ctx.gpr[6] = (0u + static_cast<std::uint32_t>(80));
    goto L_09D634F8;
L_09D634F8:
    ctx.gpr[7] = (ctx.gpr[7] + static_cast<std::uint32_t>(14960));
    goto L_09D634FC;
L_09D634FC:
    ctx.gpr[8] = (0u + static_cast<std::uint32_t>(7));
    goto L_09D63500;
L_09D63500:
    ctx.gpr[9] = (0u + static_cast<std::uint32_t>(3));
    goto L_09D63504;
L_09D63504:
    ctx.gpr[10] = (0u + 0u);
    goto L_09D63508;
L_09D63508:
    ctx.gpr[31] = (0x09D63510u);
    ctx.gpr[11] = (0u + 0u);
    ctx.pc = 0x088C3C54u;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D63510u) goto L_09D63510;
    return;
L_09D6350C:
    ctx.gpr[11] = (0u + 0u);
    goto L_09D63510;
L_09D63510:
    ctx.gpr[31] = (0x09D63518u);
    ctx.gpr[4] = (ctx.gpr[16] + 0u);
    ctx.pc = 0x088CACCCu;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D63518u) goto L_09D63518;
    return;
L_09D63514:
    ctx.gpr[4] = (ctx.gpr[16] + 0u);
    goto L_09D63518;
L_09D63518:
    ctx.gpr[31] = (aot_mem.aot_load32(ctx.gpr[29] + static_cast<std::uint32_t>(28)));
    goto L_09D6351C;
L_09D6351C:
    ctx.gpr[16] = (aot_mem.aot_load32(ctx.gpr[29] + static_cast<std::uint32_t>(24)));
    goto L_09D63520;
L_09D63520:
    jump_target = ctx.gpr[31];
    ctx.gpr[29] = (ctx.gpr[29] + static_cast<std::uint32_t>(32));
    local_pc = jump_target;
    if (++local_transfers < 2048u) { entry_id = 0u; goto LOCAL_DISPATCH; }
    ctx.pc = jump_target;
    return;
L_09D63524:
    ctx.gpr[29] = (ctx.gpr[29] + static_cast<std::uint32_t>(32));
    goto L_09D63528;
L_09D63528:
    ctx.gpr[2] = (2518u << 16u);
    goto L_09D6352C;
L_09D6352C:
    jump_target = ctx.gpr[31];
    ctx.gpr[2] = (ctx.gpr[2] + static_cast<std::uint32_t>(14448));
    local_pc = jump_target;
    if (++local_transfers < 2048u) { entry_id = 0u; goto LOCAL_DISPATCH; }
    ctx.pc = jump_target;
    return;
L_09D63530:
    ctx.gpr[2] = (ctx.gpr[2] + static_cast<std::uint32_t>(14448));
    goto L_09D63534;
L_09D63534:
    // nop
    goto L_09D63538;
L_09D63538:
    ctx.gpr[2] = (ctx.gpr[5] & 255u);
    goto L_09D6353C;
L_09D6353C:
    ctx.gpr[3] = (ctx.gpr[2] << 2u);
    goto L_09D63540;
L_09D63540:
    ctx.gpr[2] = (2518u << 16u);
    goto L_09D63544;
L_09D63544:
    ctx.gpr[2] = (ctx.gpr[2] + static_cast<std::uint32_t>(14040));
    goto L_09D63548;
L_09D63548:
    ctx.gpr[2] = (ctx.gpr[2] + ctx.gpr[3]);
    goto L_09D6354C;
L_09D6354C:
    jump_target = ctx.gpr[31];
    ctx.gpr[2] = (aot_mem.aot_load32(ctx.gpr[2] + static_cast<std::uint32_t>(0)));
    local_pc = jump_target;
    if (++local_transfers < 2048u) { entry_id = 0u; goto LOCAL_DISPATCH; }
    ctx.pc = jump_target;
    return;
L_09D63550:
    ctx.gpr[2] = (aot_mem.aot_load32(ctx.gpr[2] + static_cast<std::uint32_t>(0)));
    goto L_09D63554;
L_09D63554:
    // nop
    goto L_09D63558;
L_09D63558:
    jump_target = ctx.gpr[31];
    ctx.gpr[2] = (0u + static_cast<std::uint32_t>(1));
    local_pc = jump_target;
    if (++local_transfers < 2048u) { entry_id = 0u; goto LOCAL_DISPATCH; }
    ctx.pc = jump_target;
    return;
L_09D6355C:
    ctx.gpr[2] = (0u + static_cast<std::uint32_t>(1));
    goto L_09D63560;
L_09D63560:
    ctx.gpr[2] = (2518u << 16u);
    goto L_09D63564;
L_09D63564:
    jump_target = ctx.gpr[31];
    ctx.gpr[2] = (ctx.gpr[2] + static_cast<std::uint32_t>(15104));
    local_pc = jump_target;
    if (++local_transfers < 2048u) { entry_id = 0u; goto LOCAL_DISPATCH; }
    ctx.pc = jump_target;
    return;
L_09D63568:
    ctx.gpr[2] = (ctx.gpr[2] + static_cast<std::uint32_t>(15104));
    goto L_09D6356C;
L_09D6356C:
    // nop
    goto L_09D63570;
L_09D63570:
    jump_target = ctx.gpr[31];
    // nop
    local_pc = jump_target;
    if (++local_transfers < 2048u) { entry_id = 0u; goto LOCAL_DISPATCH; }
    ctx.pc = jump_target;
    return;
L_09D63574:
    // nop
    goto L_09D63578;
L_09D63578:
    ctx.gpr[2] = (2518u << 16u);
    goto L_09D6357C;
L_09D6357C:
    jump_target = ctx.gpr[31];
    ctx.gpr[2] = (ctx.gpr[2] + static_cast<std::uint32_t>(14592));
    local_pc = jump_target;
    if (++local_transfers < 2048u) { entry_id = 0u; goto LOCAL_DISPATCH; }
    ctx.pc = jump_target;
    return;
L_09D63580:
    ctx.gpr[2] = (ctx.gpr[2] + static_cast<std::uint32_t>(14592));
    goto L_09D63584;
L_09D63584:
    // nop
    goto L_09D63588;
L_09D63588:
    jump_target = ctx.gpr[31];
    ctx.gpr[2] = (0u + static_cast<std::uint32_t>(1));
    local_pc = jump_target;
    if (++local_transfers < 2048u) { entry_id = 0u; goto LOCAL_DISPATCH; }
    ctx.pc = jump_target;
    return;
L_09D6358C:
    ctx.gpr[2] = (0u + static_cast<std::uint32_t>(1));
    goto L_09D63590;
L_09D63590:
    jump_target = ctx.gpr[31];
    ctx.gpr[2] = (0u + 0u);
    local_pc = jump_target;
    if (++local_transfers < 2048u) { entry_id = 0u; goto LOCAL_DISPATCH; }
    ctx.pc = jump_target;
    return;
L_09D63594:
    ctx.gpr[2] = (0u + 0u);
    goto L_09D63598;
L_09D63598:
    jump_target = ctx.gpr[31];
    ctx.gpr[2] = (0u + 0u);
    local_pc = jump_target;
    if (++local_transfers < 2048u) { entry_id = 0u; goto LOCAL_DISPATCH; }
    ctx.pc = jump_target;
    return;
L_09D6359C:
    ctx.gpr[2] = (0u + 0u);
    goto L_09D635A0;
L_09D635A0:
    ctx.gpr[29] = (ctx.gpr[29] + static_cast<std::uint32_t>(-16));
    goto L_09D635A4;
L_09D635A4:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(12), ctx.gpr[31]);
    goto L_09D635A8;
L_09D635A8:
    ctx.gpr[25] = (aot_mem.aot_load32(ctx.gpr[4] + static_cast<std::uint32_t>(0)));
    goto L_09D635AC;
L_09D635AC:
    ctx.gpr[25] = (aot_mem.aot_load32(ctx.gpr[25] + static_cast<std::uint32_t>(40)));
    goto L_09D635B0;
L_09D635B0:
    jump_target = ctx.gpr[25];
    ctx.gpr[31] = (0x09D635B8u);
    // nop
    ctx.pc = jump_target;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D635B8u) goto L_09D635B8;
    return;
L_09D635B4:
    // nop
    goto L_09D635B8;
L_09D635B8:
    ctx.gpr[31] = (aot_mem.aot_load32(ctx.gpr[29] + static_cast<std::uint32_t>(12)));
    goto L_09D635BC;
L_09D635BC:
    ctx.gpr[2] = (aot_mem.aot_load8(ctx.gpr[2] + static_cast<std::uint32_t>(63)));
    goto L_09D635C0;
L_09D635C0:
    jump_target = ctx.gpr[31];
    ctx.gpr[29] = (ctx.gpr[29] + static_cast<std::uint32_t>(16));
    local_pc = jump_target;
    if (++local_transfers < 2048u) { entry_id = 0u; goto LOCAL_DISPATCH; }
    ctx.pc = jump_target;
    return;
L_09D635C4:
    ctx.gpr[29] = (ctx.gpr[29] + static_cast<std::uint32_t>(16));
    goto L_09D635C8;
L_09D635C8:
    ctx.gpr[29] = (ctx.gpr[29] + static_cast<std::uint32_t>(-16));
    goto L_09D635CC;
L_09D635CC:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(12), ctx.gpr[31]);
    goto L_09D635D0;
L_09D635D0:
    ctx.gpr[25] = (aot_mem.aot_load32(ctx.gpr[4] + static_cast<std::uint32_t>(0)));
    goto L_09D635D4;
L_09D635D4:
    ctx.gpr[25] = (aot_mem.aot_load32(ctx.gpr[25] + static_cast<std::uint32_t>(40)));
    goto L_09D635D8;
L_09D635D8:
    jump_target = ctx.gpr[25];
    ctx.gpr[31] = (0x09D635E0u);
    // nop
    ctx.pc = jump_target;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D635E0u) goto L_09D635E0;
    return;
L_09D635DC:
    // nop
    goto L_09D635E0;
L_09D635E0:
    ctx.gpr[31] = (aot_mem.aot_load32(ctx.gpr[29] + static_cast<std::uint32_t>(12)));
    goto L_09D635E4;
L_09D635E4:
    ctx.gpr[2] = (aot_mem.aot_load32(ctx.gpr[2] + static_cast<std::uint32_t>(52)));
    goto L_09D635E8;
L_09D635E8:
    jump_target = ctx.gpr[31];
    ctx.gpr[29] = (ctx.gpr[29] + static_cast<std::uint32_t>(16));
    local_pc = jump_target;
    if (++local_transfers < 2048u) { entry_id = 0u; goto LOCAL_DISPATCH; }
    ctx.pc = jump_target;
    return;
L_09D635EC:
    ctx.gpr[29] = (ctx.gpr[29] + static_cast<std::uint32_t>(16));
    goto L_09D635F0;
L_09D635F0:
    ctx.gpr[29] = (ctx.gpr[29] + static_cast<std::uint32_t>(-16));
    goto L_09D635F4;
L_09D635F4:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(12), ctx.gpr[31]);
    goto L_09D635F8;
L_09D635F8:
    ctx.gpr[25] = (aot_mem.aot_load32(ctx.gpr[4] + static_cast<std::uint32_t>(0)));
    goto L_09D635FC;
L_09D635FC:
    ctx.gpr[25] = (aot_mem.aot_load32(ctx.gpr[25] + static_cast<std::uint32_t>(40)));
    goto L_09D63600;
L_09D63600:
    jump_target = ctx.gpr[25];
    ctx.gpr[31] = (0x09D63608u);
    // nop
    ctx.pc = jump_target;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D63608u) goto L_09D63608;
    return;
L_09D63604:
    // nop
    goto L_09D63608;
L_09D63608:
    ctx.gpr[31] = (aot_mem.aot_load32(ctx.gpr[29] + static_cast<std::uint32_t>(12)));
    goto L_09D6360C;
L_09D6360C:
    ctx.gpr[2] = (aot_mem.aot_load8(ctx.gpr[2] + static_cast<std::uint32_t>(64)));
    goto L_09D63610;
L_09D63610:
    jump_target = ctx.gpr[31];
    ctx.gpr[29] = (ctx.gpr[29] + static_cast<std::uint32_t>(16));
    local_pc = jump_target;
    if (++local_transfers < 2048u) { entry_id = 0u; goto LOCAL_DISPATCH; }
    ctx.pc = jump_target;
    return;
L_09D63614:
    ctx.gpr[29] = (ctx.gpr[29] + static_cast<std::uint32_t>(16));
    goto L_09D63618;
L_09D63618:
    ctx.gpr[29] = (ctx.gpr[29] + static_cast<std::uint32_t>(-16));
    goto L_09D6361C;
L_09D6361C:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(12), ctx.gpr[31]);
    goto L_09D63620;
L_09D63620:
    ctx.gpr[25] = (aot_mem.aot_load32(ctx.gpr[4] + static_cast<std::uint32_t>(0)));
    goto L_09D63624;
L_09D63624:
    ctx.gpr[25] = (aot_mem.aot_load32(ctx.gpr[25] + static_cast<std::uint32_t>(40)));
    goto L_09D63628;
L_09D63628:
    jump_target = ctx.gpr[25];
    ctx.gpr[31] = (0x09D63630u);
    // nop
    ctx.pc = jump_target;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D63630u) goto L_09D63630;
    return;
L_09D6362C:
    // nop
    goto L_09D63630;
L_09D63630:
    ctx.gpr[31] = (aot_mem.aot_load32(ctx.gpr[29] + static_cast<std::uint32_t>(12)));
    goto L_09D63634;
L_09D63634:
    ctx.gpr[2] = (aot_mem.aot_load32(ctx.gpr[2] + static_cast<std::uint32_t>(56)));
    goto L_09D63638;
L_09D63638:
    jump_target = ctx.gpr[31];
    ctx.gpr[29] = (ctx.gpr[29] + static_cast<std::uint32_t>(16));
    local_pc = jump_target;
    if (++local_transfers < 2048u) { entry_id = 0u; goto LOCAL_DISPATCH; }
    ctx.pc = jump_target;
    return;
L_09D6363C:
    ctx.gpr[29] = (ctx.gpr[29] + static_cast<std::uint32_t>(16));
    ctx.pc = 0x09D63640u; return;
}

void ovl09D63100_stage042_3B2961E780C72163_unit_0000(Runtime &rt, AllegrexContext &ctx) {
    auto aot_mem = rt.memory().aot_fast_view();
    ovl09D63100_stage042_3B2961E780C72163_unit_0000_entry(rt, ctx, 0u, aot_mem);
}

void register_ovl09D63100_stage042_3B2961E780C72163_unit_0(Runtime &runtime) {
    runtime.register_function(0x09D63140u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63144u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63148u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D6314Cu, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63150u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63154u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63158u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D6315Cu, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63160u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63164u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63168u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D6316Cu, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63170u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63174u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63178u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D6317Cu, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63180u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63184u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63188u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D6318Cu, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63190u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63194u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63198u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D6319Cu, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D631A0u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D631A4u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D631A8u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D631ACu, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D631B0u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D631B4u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D631B8u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D631BCu, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D631C0u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D631C4u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D631C8u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D631CCu, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D631D0u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D631D4u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D631D8u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D631DCu, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D631E0u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D631E4u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D631E8u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D631ECu, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D631F0u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D631F4u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D631F8u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D631FCu, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63200u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63204u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63208u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D6320Cu, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63210u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63214u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63218u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D6321Cu, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63220u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63224u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63228u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D6322Cu, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63230u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63234u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63238u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D6323Cu, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63240u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63244u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63248u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D6324Cu, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63250u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63254u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63258u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D6325Cu, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63260u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63264u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63268u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D6326Cu, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63270u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63274u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63278u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D6327Cu, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63280u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63284u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63288u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D6328Cu, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63290u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63294u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63298u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D6329Cu, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D632A0u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D632A4u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D632A8u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D632ACu, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D632B0u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D632B4u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D632B8u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D632BCu, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D632C0u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D632C4u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D632C8u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D632CCu, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D632D0u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D632D4u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D632D8u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D632DCu, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D632E0u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D632E4u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D632E8u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D632ECu, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D632F0u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D632F4u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D632F8u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D632FCu, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63300u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63304u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63308u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D6330Cu, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63310u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63314u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63318u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D6331Cu, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63320u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63324u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63328u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D6332Cu, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63330u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63334u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63338u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D6333Cu, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63340u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63344u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63348u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D6334Cu, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63350u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63354u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63358u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D6335Cu, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63360u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63364u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63368u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D6336Cu, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63370u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63374u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63378u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D6337Cu, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63380u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63384u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63388u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D6338Cu, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63390u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63394u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63398u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D6339Cu, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D633A0u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D633A4u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D633A8u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D633ACu, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D633B0u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D633B4u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D633B8u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D633BCu, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D633C0u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D633C4u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D633C8u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D633CCu, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D633D0u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D633D4u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D633D8u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D633DCu, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D633E0u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D633E4u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D633E8u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D633ECu, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D633F0u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D633F4u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D633F8u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D633FCu, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63400u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63404u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63408u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D6340Cu, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63410u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63414u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63418u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D6341Cu, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63420u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63424u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63428u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D6342Cu, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63430u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63434u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63438u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D6343Cu, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63440u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63444u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63448u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D6344Cu, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63450u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63454u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63458u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D6345Cu, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63460u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63464u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63468u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D6346Cu, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63470u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63474u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63478u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D6347Cu, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63480u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63484u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63488u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D6348Cu, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63490u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63494u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63498u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D6349Cu, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D634A0u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D634A4u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D634A8u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D634ACu, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D634B0u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D634B4u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D634B8u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D634BCu, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D634C0u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D634C4u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D634C8u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D634CCu, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D634D0u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D634D4u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D634D8u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D634DCu, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D634E0u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D634E4u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D634E8u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D634ECu, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D634F0u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D634F4u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D634F8u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D634FCu, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63500u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63504u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63508u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D6350Cu, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63510u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63514u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63518u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D6351Cu, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63520u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63524u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63528u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D6352Cu, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63530u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63534u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63538u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D6353Cu, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63540u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63544u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63548u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D6354Cu, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63550u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63554u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63558u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D6355Cu, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63560u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63564u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63568u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D6356Cu, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63570u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63574u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63578u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D6357Cu, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63580u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63584u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63588u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D6358Cu, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63590u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63594u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63598u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D6359Cu, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D635A0u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D635A4u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D635A8u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D635ACu, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D635B0u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D635B4u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D635B8u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D635BCu, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D635C0u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D635C4u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D635C8u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D635CCu, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D635D0u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D635D4u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D635D8u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D635DCu, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D635E0u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D635E4u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D635E8u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D635ECu, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D635F0u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D635F4u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D635F8u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D635FCu, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63600u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63604u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63608u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D6360Cu, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63610u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63614u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63618u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D6361Cu, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63620u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63624u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63628u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D6362Cu, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63630u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63634u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D63638u, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
    runtime.register_function(0x09D6363Cu, &ovl09D63100_stage042_3B2961E780C72163_unit_0000, "recomp_unit_ovl09D63100_stage042_3B2961E780C72163_unit_0000");
}
} // namespace psprecomp
