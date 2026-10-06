#include "psprecomp/runtime.hpp"
#include "ovl09D1A280_em21_9E5C67A047FDEEE5_units.hpp"
#include <bit>
#include <cmath>
#include <cstdint>
#include <limits>

namespace psprecomp {
static const std::uint16_t kEntryIds_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004[528] = {
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
    321, 322, 323, 324, 325, 326, 327, 328, 329, 330, 331, 332, 333, 334, 335, 336, 337, 338, 339, 340, 341, 342, 343, 344, 345, 346, 347, 348, 349, 350, 351, 352,
    353, 354, 355, 356, 357, 358, 359, 360, 361, 362, 363, 364, 365, 366, 367, 368, 369, 370, 371, 372, 373, 374, 375, 376, 377, 378, 379, 380, 381, 382, 383, 384,
    385, 386, 387, 388, 389, 390, 391, 392, 393, 394, 395, 396, 397, 398, 399, 400, 401, 402, 403, 404, 405, 406, 407, 408, 409, 410, 411, 412, 413, 414, 415, 416,
    417, 418, 419, 420, 421, 422, 423, 424, 425, 426, 427, 428, 429, 430, 431, 432, 433, 434, 435, 436, 437, 438, 439, 440, 441, 442, 443, 444, 445, 446, 447, 448,
    449, 450, 451, 452, 453, 454, 455, 456, 457, 458, 459, 460, 461, 462, 463, 464, 465, 466, 467, 468, 469, 470, 471, 472, 473, 474, 475, 476, 477, 478, 479, 480,
    481, 482, 483, 484, 485, 486, 487, 488, 489, 490, 491, 492, 493, 494, 495, 496, 497, 498, 499, 500, 501, 502, 503, 504, 505, 506, 507, 508, 509, 510, 511, 512,
    513, 514, 515, 516, 517, 518, 519, 520, 521, 522, 523, 524, 525, 526, 527, 528,
};
void ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004_entry(Runtime &rt, AllegrexContext &ctx, std::uint16_t direct_entry_id, GuestMemory::AotFastView &aot_mem) {
    std::uint32_t jump_target = 0u;
    std::uint32_t local_transfers = 0u;
    std::uint32_t local_pc = ctx.pc;
    std::uint32_t entry_id = direct_entry_id;
LOCAL_DISPATCH:
    {
    if (entry_id == 0u) {
        const std::uint32_t entry_delta = local_pc - 0x09D2A2C0u;
        entry_id = (entry_delta < 2112u && (entry_delta & 3u) == 0u) ? kEntryIds_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004[entry_delta >> 2u] : 0u;
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
    case 57u: goto L_09D2A3A0;
    case 58u: goto L_09D2A3A4;
    case 59u: goto L_09D2A3A8;
    case 60u: goto L_09D2A3AC;
    case 61u: goto L_09D2A3B0;
    case 62u: goto L_09D2A3B4;
    case 63u: goto L_09D2A3B8;
    case 64u: goto L_09D2A3BC;
    case 65u: goto L_09D2A3C0;
    case 66u: goto L_09D2A3C4;
    case 67u: goto L_09D2A3C8;
    case 68u: goto L_09D2A3CC;
    case 69u: goto L_09D2A3D0;
    case 70u: goto L_09D2A3D4;
    case 71u: goto L_09D2A3D8;
    case 72u: goto L_09D2A3DC;
    case 73u: goto L_09D2A3E0;
    case 74u: goto L_09D2A3E4;
    case 75u: goto L_09D2A3E8;
    case 76u: goto L_09D2A3EC;
    case 77u: goto L_09D2A3F0;
    case 78u: goto L_09D2A3F4;
    case 79u: goto L_09D2A3F8;
    case 80u: goto L_09D2A3FC;
    case 81u: goto L_09D2A400;
    case 82u: goto L_09D2A404;
    case 83u: goto L_09D2A408;
    case 84u: goto L_09D2A40C;
    case 85u: goto L_09D2A410;
    case 86u: goto L_09D2A414;
    case 87u: goto L_09D2A418;
    case 88u: goto L_09D2A41C;
    case 89u: goto L_09D2A420;
    case 90u: goto L_09D2A424;
    case 91u: goto L_09D2A428;
    case 92u: goto L_09D2A42C;
    case 93u: goto L_09D2A430;
    case 94u: goto L_09D2A434;
    case 95u: goto L_09D2A438;
    case 96u: goto L_09D2A43C;
    case 97u: goto L_09D2A440;
    case 98u: goto L_09D2A444;
    case 99u: goto L_09D2A448;
    case 100u: goto L_09D2A44C;
    case 101u: goto L_09D2A450;
    case 102u: goto L_09D2A454;
    case 103u: goto L_09D2A458;
    case 104u: goto L_09D2A45C;
    case 105u: goto L_09D2A460;
    case 106u: goto L_09D2A464;
    case 107u: goto L_09D2A468;
    case 108u: goto L_09D2A46C;
    case 109u: goto L_09D2A470;
    case 110u: goto L_09D2A474;
    case 111u: goto L_09D2A478;
    case 112u: goto L_09D2A47C;
    case 113u: goto L_09D2A480;
    case 114u: goto L_09D2A484;
    case 115u: goto L_09D2A488;
    case 116u: goto L_09D2A48C;
    case 117u: goto L_09D2A490;
    case 118u: goto L_09D2A494;
    case 119u: goto L_09D2A498;
    case 120u: goto L_09D2A49C;
    case 121u: goto L_09D2A4A0;
    case 122u: goto L_09D2A4A4;
    case 123u: goto L_09D2A4A8;
    case 124u: goto L_09D2A4AC;
    case 125u: goto L_09D2A4B0;
    case 126u: goto L_09D2A4B4;
    case 127u: goto L_09D2A4B8;
    case 128u: goto L_09D2A4BC;
    case 129u: goto L_09D2A4C0;
    case 130u: goto L_09D2A4C4;
    case 131u: goto L_09D2A4C8;
    case 132u: goto L_09D2A4CC;
    case 133u: goto L_09D2A4D0;
    case 134u: goto L_09D2A4D4;
    case 135u: goto L_09D2A4D8;
    case 136u: goto L_09D2A4DC;
    case 137u: goto L_09D2A4E0;
    case 138u: goto L_09D2A4E4;
    case 139u: goto L_09D2A4E8;
    case 140u: goto L_09D2A4EC;
    case 141u: goto L_09D2A4F0;
    case 142u: goto L_09D2A4F4;
    case 143u: goto L_09D2A4F8;
    case 144u: goto L_09D2A4FC;
    case 145u: goto L_09D2A500;
    case 146u: goto L_09D2A504;
    case 147u: goto L_09D2A508;
    case 148u: goto L_09D2A50C;
    case 149u: goto L_09D2A510;
    case 150u: goto L_09D2A514;
    case 151u: goto L_09D2A518;
    case 152u: goto L_09D2A51C;
    case 153u: goto L_09D2A520;
    case 154u: goto L_09D2A524;
    case 155u: goto L_09D2A528;
    case 156u: goto L_09D2A52C;
    case 157u: goto L_09D2A530;
    case 158u: goto L_09D2A534;
    case 159u: goto L_09D2A538;
    case 160u: goto L_09D2A53C;
    case 161u: goto L_09D2A540;
    case 162u: goto L_09D2A544;
    case 163u: goto L_09D2A548;
    case 164u: goto L_09D2A54C;
    case 165u: goto L_09D2A550;
    case 166u: goto L_09D2A554;
    case 167u: goto L_09D2A558;
    case 168u: goto L_09D2A55C;
    case 169u: goto L_09D2A560;
    case 170u: goto L_09D2A564;
    case 171u: goto L_09D2A568;
    case 172u: goto L_09D2A56C;
    case 173u: goto L_09D2A570;
    case 174u: goto L_09D2A574;
    case 175u: goto L_09D2A578;
    case 176u: goto L_09D2A57C;
    case 177u: goto L_09D2A580;
    case 178u: goto L_09D2A584;
    case 179u: goto L_09D2A588;
    case 180u: goto L_09D2A58C;
    case 181u: goto L_09D2A590;
    case 182u: goto L_09D2A594;
    case 183u: goto L_09D2A598;
    case 184u: goto L_09D2A59C;
    case 185u: goto L_09D2A5A0;
    case 186u: goto L_09D2A5A4;
    case 187u: goto L_09D2A5A8;
    case 188u: goto L_09D2A5AC;
    case 189u: goto L_09D2A5B0;
    case 190u: goto L_09D2A5B4;
    case 191u: goto L_09D2A5B8;
    case 192u: goto L_09D2A5BC;
    case 193u: goto L_09D2A5C0;
    case 194u: goto L_09D2A5C4;
    case 195u: goto L_09D2A5C8;
    case 196u: goto L_09D2A5CC;
    case 197u: goto L_09D2A5D0;
    case 198u: goto L_09D2A5D4;
    case 199u: goto L_09D2A5D8;
    case 200u: goto L_09D2A5DC;
    case 201u: goto L_09D2A5E0;
    case 202u: goto L_09D2A5E4;
    case 203u: goto L_09D2A5E8;
    case 204u: goto L_09D2A5EC;
    case 205u: goto L_09D2A5F0;
    case 206u: goto L_09D2A5F4;
    case 207u: goto L_09D2A5F8;
    case 208u: goto L_09D2A5FC;
    case 209u: goto L_09D2A600;
    case 210u: goto L_09D2A604;
    case 211u: goto L_09D2A608;
    case 212u: goto L_09D2A60C;
    case 213u: goto L_09D2A610;
    case 214u: goto L_09D2A614;
    case 215u: goto L_09D2A618;
    case 216u: goto L_09D2A61C;
    case 217u: goto L_09D2A620;
    case 218u: goto L_09D2A624;
    case 219u: goto L_09D2A628;
    case 220u: goto L_09D2A62C;
    case 221u: goto L_09D2A630;
    case 222u: goto L_09D2A634;
    case 223u: goto L_09D2A638;
    case 224u: goto L_09D2A63C;
    case 225u: goto L_09D2A640;
    case 226u: goto L_09D2A644;
    case 227u: goto L_09D2A648;
    case 228u: goto L_09D2A64C;
    case 229u: goto L_09D2A650;
    case 230u: goto L_09D2A654;
    case 231u: goto L_09D2A658;
    case 232u: goto L_09D2A65C;
    case 233u: goto L_09D2A660;
    case 234u: goto L_09D2A664;
    case 235u: goto L_09D2A668;
    case 236u: goto L_09D2A66C;
    case 237u: goto L_09D2A670;
    case 238u: goto L_09D2A674;
    case 239u: goto L_09D2A678;
    case 240u: goto L_09D2A67C;
    case 241u: goto L_09D2A680;
    case 242u: goto L_09D2A684;
    case 243u: goto L_09D2A688;
    case 244u: goto L_09D2A68C;
    case 245u: goto L_09D2A690;
    case 246u: goto L_09D2A694;
    case 247u: goto L_09D2A698;
    case 248u: goto L_09D2A69C;
    case 249u: goto L_09D2A6A0;
    case 250u: goto L_09D2A6A4;
    case 251u: goto L_09D2A6A8;
    case 252u: goto L_09D2A6AC;
    case 253u: goto L_09D2A6B0;
    case 254u: goto L_09D2A6B4;
    case 255u: goto L_09D2A6B8;
    case 256u: goto L_09D2A6BC;
    case 257u: goto L_09D2A6C0;
    case 258u: goto L_09D2A6C4;
    case 259u: goto L_09D2A6C8;
    case 260u: goto L_09D2A6CC;
    case 261u: goto L_09D2A6D0;
    case 262u: goto L_09D2A6D4;
    case 263u: goto L_09D2A6D8;
    case 264u: goto L_09D2A6DC;
    case 265u: goto L_09D2A6E0;
    case 266u: goto L_09D2A6E4;
    case 267u: goto L_09D2A6E8;
    case 268u: goto L_09D2A6EC;
    case 269u: goto L_09D2A6F0;
    case 270u: goto L_09D2A6F4;
    case 271u: goto L_09D2A6F8;
    case 272u: goto L_09D2A6FC;
    case 273u: goto L_09D2A700;
    case 274u: goto L_09D2A704;
    case 275u: goto L_09D2A708;
    case 276u: goto L_09D2A70C;
    case 277u: goto L_09D2A710;
    case 278u: goto L_09D2A714;
    case 279u: goto L_09D2A718;
    case 280u: goto L_09D2A71C;
    case 281u: goto L_09D2A720;
    case 282u: goto L_09D2A724;
    case 283u: goto L_09D2A728;
    case 284u: goto L_09D2A72C;
    case 285u: goto L_09D2A730;
    case 286u: goto L_09D2A734;
    case 287u: goto L_09D2A738;
    case 288u: goto L_09D2A73C;
    case 289u: goto L_09D2A740;
    case 290u: goto L_09D2A744;
    case 291u: goto L_09D2A748;
    case 292u: goto L_09D2A74C;
    case 293u: goto L_09D2A750;
    case 294u: goto L_09D2A754;
    case 295u: goto L_09D2A758;
    case 296u: goto L_09D2A75C;
    case 297u: goto L_09D2A760;
    case 298u: goto L_09D2A764;
    case 299u: goto L_09D2A768;
    case 300u: goto L_09D2A76C;
    case 301u: goto L_09D2A770;
    case 302u: goto L_09D2A774;
    case 303u: goto L_09D2A778;
    case 304u: goto L_09D2A77C;
    case 305u: goto L_09D2A780;
    case 306u: goto L_09D2A784;
    case 307u: goto L_09D2A788;
    case 308u: goto L_09D2A78C;
    case 309u: goto L_09D2A790;
    case 310u: goto L_09D2A794;
    case 311u: goto L_09D2A798;
    case 312u: goto L_09D2A79C;
    case 313u: goto L_09D2A7A0;
    case 314u: goto L_09D2A7A4;
    case 315u: goto L_09D2A7A8;
    case 316u: goto L_09D2A7AC;
    case 317u: goto L_09D2A7B0;
    case 318u: goto L_09D2A7B4;
    case 319u: goto L_09D2A7B8;
    case 320u: goto L_09D2A7BC;
    case 321u: goto L_09D2A7C0;
    case 322u: goto L_09D2A7C4;
    case 323u: goto L_09D2A7C8;
    case 324u: goto L_09D2A7CC;
    case 325u: goto L_09D2A7D0;
    case 326u: goto L_09D2A7D4;
    case 327u: goto L_09D2A7D8;
    case 328u: goto L_09D2A7DC;
    case 329u: goto L_09D2A7E0;
    case 330u: goto L_09D2A7E4;
    case 331u: goto L_09D2A7E8;
    case 332u: goto L_09D2A7EC;
    case 333u: goto L_09D2A7F0;
    case 334u: goto L_09D2A7F4;
    case 335u: goto L_09D2A7F8;
    case 336u: goto L_09D2A7FC;
    case 337u: goto L_09D2A800;
    case 338u: goto L_09D2A804;
    case 339u: goto L_09D2A808;
    case 340u: goto L_09D2A80C;
    case 341u: goto L_09D2A810;
    case 342u: goto L_09D2A814;
    case 343u: goto L_09D2A818;
    case 344u: goto L_09D2A81C;
    case 345u: goto L_09D2A820;
    case 346u: goto L_09D2A824;
    case 347u: goto L_09D2A828;
    case 348u: goto L_09D2A82C;
    case 349u: goto L_09D2A830;
    case 350u: goto L_09D2A834;
    case 351u: goto L_09D2A838;
    case 352u: goto L_09D2A83C;
    case 353u: goto L_09D2A840;
    case 354u: goto L_09D2A844;
    case 355u: goto L_09D2A848;
    case 356u: goto L_09D2A84C;
    case 357u: goto L_09D2A850;
    case 358u: goto L_09D2A854;
    case 359u: goto L_09D2A858;
    case 360u: goto L_09D2A85C;
    case 361u: goto L_09D2A860;
    case 362u: goto L_09D2A864;
    case 363u: goto L_09D2A868;
    case 364u: goto L_09D2A86C;
    case 365u: goto L_09D2A870;
    case 366u: goto L_09D2A874;
    case 367u: goto L_09D2A878;
    case 368u: goto L_09D2A87C;
    case 369u: goto L_09D2A880;
    case 370u: goto L_09D2A884;
    case 371u: goto L_09D2A888;
    case 372u: goto L_09D2A88C;
    case 373u: goto L_09D2A890;
    case 374u: goto L_09D2A894;
    case 375u: goto L_09D2A898;
    case 376u: goto L_09D2A89C;
    case 377u: goto L_09D2A8A0;
    case 378u: goto L_09D2A8A4;
    case 379u: goto L_09D2A8A8;
    case 380u: goto L_09D2A8AC;
    case 381u: goto L_09D2A8B0;
    case 382u: goto L_09D2A8B4;
    case 383u: goto L_09D2A8B8;
    case 384u: goto L_09D2A8BC;
    case 385u: goto L_09D2A8C0;
    case 386u: goto L_09D2A8C4;
    case 387u: goto L_09D2A8C8;
    case 388u: goto L_09D2A8CC;
    case 389u: goto L_09D2A8D0;
    case 390u: goto L_09D2A8D4;
    case 391u: goto L_09D2A8D8;
    case 392u: goto L_09D2A8DC;
    case 393u: goto L_09D2A8E0;
    case 394u: goto L_09D2A8E4;
    case 395u: goto L_09D2A8E8;
    case 396u: goto L_09D2A8EC;
    case 397u: goto L_09D2A8F0;
    case 398u: goto L_09D2A8F4;
    case 399u: goto L_09D2A8F8;
    case 400u: goto L_09D2A8FC;
    case 401u: goto L_09D2A900;
    case 402u: goto L_09D2A904;
    case 403u: goto L_09D2A908;
    case 404u: goto L_09D2A90C;
    case 405u: goto L_09D2A910;
    case 406u: goto L_09D2A914;
    case 407u: goto L_09D2A918;
    case 408u: goto L_09D2A91C;
    case 409u: goto L_09D2A920;
    case 410u: goto L_09D2A924;
    case 411u: goto L_09D2A928;
    case 412u: goto L_09D2A92C;
    case 413u: goto L_09D2A930;
    case 414u: goto L_09D2A934;
    case 415u: goto L_09D2A938;
    case 416u: goto L_09D2A93C;
    case 417u: goto L_09D2A940;
    case 418u: goto L_09D2A944;
    case 419u: goto L_09D2A948;
    case 420u: goto L_09D2A94C;
    case 421u: goto L_09D2A950;
    case 422u: goto L_09D2A954;
    case 423u: goto L_09D2A958;
    case 424u: goto L_09D2A95C;
    case 425u: goto L_09D2A960;
    case 426u: goto L_09D2A964;
    case 427u: goto L_09D2A968;
    case 428u: goto L_09D2A96C;
    case 429u: goto L_09D2A970;
    case 430u: goto L_09D2A974;
    case 431u: goto L_09D2A978;
    case 432u: goto L_09D2A97C;
    case 433u: goto L_09D2A980;
    case 434u: goto L_09D2A984;
    case 435u: goto L_09D2A988;
    case 436u: goto L_09D2A98C;
    case 437u: goto L_09D2A990;
    case 438u: goto L_09D2A994;
    case 439u: goto L_09D2A998;
    case 440u: goto L_09D2A99C;
    case 441u: goto L_09D2A9A0;
    case 442u: goto L_09D2A9A4;
    case 443u: goto L_09D2A9A8;
    case 444u: goto L_09D2A9AC;
    case 445u: goto L_09D2A9B0;
    case 446u: goto L_09D2A9B4;
    case 447u: goto L_09D2A9B8;
    case 448u: goto L_09D2A9BC;
    case 449u: goto L_09D2A9C0;
    case 450u: goto L_09D2A9C4;
    case 451u: goto L_09D2A9C8;
    case 452u: goto L_09D2A9CC;
    case 453u: goto L_09D2A9D0;
    case 454u: goto L_09D2A9D4;
    case 455u: goto L_09D2A9D8;
    case 456u: goto L_09D2A9DC;
    case 457u: goto L_09D2A9E0;
    case 458u: goto L_09D2A9E4;
    case 459u: goto L_09D2A9E8;
    case 460u: goto L_09D2A9EC;
    case 461u: goto L_09D2A9F0;
    case 462u: goto L_09D2A9F4;
    case 463u: goto L_09D2A9F8;
    case 464u: goto L_09D2A9FC;
    case 465u: goto L_09D2AA00;
    case 466u: goto L_09D2AA04;
    case 467u: goto L_09D2AA08;
    case 468u: goto L_09D2AA0C;
    case 469u: goto L_09D2AA10;
    case 470u: goto L_09D2AA14;
    case 471u: goto L_09D2AA18;
    case 472u: goto L_09D2AA1C;
    case 473u: goto L_09D2AA20;
    case 474u: goto L_09D2AA24;
    case 475u: goto L_09D2AA28;
    case 476u: goto L_09D2AA2C;
    case 477u: goto L_09D2AA30;
    case 478u: goto L_09D2AA34;
    case 479u: goto L_09D2AA38;
    case 480u: goto L_09D2AA3C;
    case 481u: goto L_09D2AA40;
    case 482u: goto L_09D2AA44;
    case 483u: goto L_09D2AA48;
    case 484u: goto L_09D2AA4C;
    case 485u: goto L_09D2AA50;
    case 486u: goto L_09D2AA54;
    case 487u: goto L_09D2AA58;
    case 488u: goto L_09D2AA5C;
    case 489u: goto L_09D2AA60;
    case 490u: goto L_09D2AA64;
    case 491u: goto L_09D2AA68;
    case 492u: goto L_09D2AA6C;
    case 493u: goto L_09D2AA70;
    case 494u: goto L_09D2AA74;
    case 495u: goto L_09D2AA78;
    case 496u: goto L_09D2AA7C;
    case 497u: goto L_09D2AA80;
    case 498u: goto L_09D2AA84;
    case 499u: goto L_09D2AA88;
    case 500u: goto L_09D2AA8C;
    case 501u: goto L_09D2AA90;
    case 502u: goto L_09D2AA94;
    case 503u: goto L_09D2AA98;
    case 504u: goto L_09D2AA9C;
    case 505u: goto L_09D2AAA0;
    case 506u: goto L_09D2AAA4;
    case 507u: goto L_09D2AAA8;
    case 508u: goto L_09D2AAAC;
    case 509u: goto L_09D2AAB0;
    case 510u: goto L_09D2AAB4;
    case 511u: goto L_09D2AAB8;
    case 512u: goto L_09D2AABC;
    case 513u: goto L_09D2AAC0;
    case 514u: goto L_09D2AAC4;
    case 515u: goto L_09D2AAC8;
    case 516u: goto L_09D2AACC;
    case 517u: goto L_09D2AAD0;
    case 518u: goto L_09D2AAD4;
    case 519u: goto L_09D2AAD8;
    case 520u: goto L_09D2AADC;
    case 521u: goto L_09D2AAE0;
    case 522u: goto L_09D2AAE4;
    case 523u: goto L_09D2AAE8;
    case 524u: goto L_09D2AAEC;
    case 525u: goto L_09D2AAF0;
    case 526u: goto L_09D2AAF4;
    case 527u: goto L_09D2AAF8;
    case 528u: goto L_09D2AAFC;
    default:
        if (local_transfers == 0u) rt.unsupported(ctx.pc, 0u, "invalid internal function entry");
        else ctx.pc = local_pc;
        return;
    }
    }
L_09D2A2C0:
    ctx.gpr[7] = (0u + static_cast<std::uint32_t>(35));
    goto L_09D2A2C4;
L_09D2A2C4:
    ctx.gpr[4] = (ctx.gpr[17] + 0u);
    goto L_09D2A2C8;
L_09D2A2C8:
    ctx.gpr[5] = (0u + static_cast<std::uint32_t>(4));
    goto L_09D2A2CC;
L_09D2A2CC:
    ctx.gpr[6] = (0u + static_cast<std::uint32_t>(22));
    goto L_09D2A2D0;
L_09D2A2D0:
    ctx.gpr[31] = (0x09D2A2D8u);
    ctx.gpr[7] = (0u + static_cast<std::uint32_t>(35));
    ctx.pc = 0x09ACB620u;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D2A2D8u) goto L_09D2A2D8;
    return;
L_09D2A2D4:
    ctx.gpr[7] = (0u + static_cast<std::uint32_t>(35));
    goto L_09D2A2D8;
L_09D2A2D8:
    { const bool branch_taken = 0u == 0u;
    // nop
      if (branch_taken) {
          goto L_09D2A634;
      }
      goto L_09D2A2E0;
    }
L_09D2A2DC:
    // nop
    goto L_09D2A2E0;
L_09D2A2E0:
    ctx.gpr[5] = (0u + static_cast<std::uint32_t>(4));
    goto L_09D2A2E4;
L_09D2A2E4:
    ctx.gpr[6] = (0u + static_cast<std::uint32_t>(22));
    goto L_09D2A2E8;
L_09D2A2E8:
    ctx.gpr[31] = (0x09D2A2F0u);
    ctx.gpr[7] = (0u + 0u);
    ctx.pc = 0x09ACB620u;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D2A2F0u) goto L_09D2A2F0;
    return;
L_09D2A2EC:
    ctx.gpr[7] = (0u + 0u);
    goto L_09D2A2F0;
L_09D2A2F0:
    ctx.gpr[4] = (ctx.gpr[17] + 0u);
    goto L_09D2A2F4;
L_09D2A2F4:
    ctx.gpr[5] = (0u + static_cast<std::uint32_t>(134));
    goto L_09D2A2F8;
L_09D2A2F8:
    ctx.gpr[6] = (0u + static_cast<std::uint32_t>(23));
    goto L_09D2A2FC;
L_09D2A2FC:
    ctx.gpr[31] = (0x09D2A304u);
    ctx.gpr[7] = (0u + 0u);
    ctx.pc = 0x09ACB620u;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D2A304u) goto L_09D2A304;
    return;
L_09D2A300:
    ctx.gpr[7] = (0u + 0u);
    goto L_09D2A304;
L_09D2A304:
    ctx.gpr[4] = (ctx.gpr[17] + 0u);
    goto L_09D2A308;
L_09D2A308:
    ctx.gpr[5] = (0u + static_cast<std::uint32_t>(134));
    goto L_09D2A30C;
L_09D2A30C:
    ctx.gpr[6] = (0u + static_cast<std::uint32_t>(22));
    goto L_09D2A310;
L_09D2A310:
    ctx.gpr[31] = (0x09D2A318u);
    ctx.gpr[7] = (0u + static_cast<std::uint32_t>(35));
    ctx.pc = 0x09ACB620u;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D2A318u) goto L_09D2A318;
    return;
L_09D2A314:
    ctx.gpr[7] = (0u + static_cast<std::uint32_t>(35));
    goto L_09D2A318;
L_09D2A318:
    { const bool branch_taken = 0u == 0u;
    // nop
      if (branch_taken) {
          goto L_09D2A634;
      }
      goto L_09D2A320;
    }
L_09D2A31C:
    // nop
    goto L_09D2A320;
L_09D2A320:
    ctx.gpr[5] = (0u + static_cast<std::uint32_t>(4));
    goto L_09D2A324;
L_09D2A324:
    ctx.gpr[6] = (0u + static_cast<std::uint32_t>(23));
    goto L_09D2A328;
L_09D2A328:
    ctx.gpr[31] = (0x09D2A330u);
    ctx.gpr[7] = (0u + 0u);
    ctx.pc = 0x09ACB620u;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D2A330u) goto L_09D2A330;
    return;
L_09D2A32C:
    ctx.gpr[7] = (0u + 0u);
    goto L_09D2A330;
L_09D2A330:
    ctx.gpr[4] = (ctx.gpr[17] + 0u);
    goto L_09D2A334;
L_09D2A334:
    ctx.gpr[5] = (0u + static_cast<std::uint32_t>(32));
    goto L_09D2A338;
L_09D2A338:
    ctx.gpr[6] = (0u + static_cast<std::uint32_t>(22));
    goto L_09D2A33C;
L_09D2A33C:
    ctx.gpr[31] = (0x09D2A344u);
    ctx.gpr[7] = (0u + 0u);
    ctx.pc = 0x09ACB620u;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D2A344u) goto L_09D2A344;
    return;
L_09D2A340:
    ctx.gpr[7] = (0u + 0u);
    goto L_09D2A344;
L_09D2A344:
    ctx.gpr[4] = (ctx.gpr[17] + 0u);
    goto L_09D2A348;
L_09D2A348:
    ctx.gpr[5] = (0u + static_cast<std::uint32_t>(110));
    goto L_09D2A34C;
L_09D2A34C:
    ctx.gpr[6] = (0u + static_cast<std::uint32_t>(21));
    goto L_09D2A350;
L_09D2A350:
    ctx.gpr[31] = (0x09D2A358u);
    ctx.gpr[7] = (0u + static_cast<std::uint32_t>(35));
    ctx.pc = 0x09ACB620u;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D2A358u) goto L_09D2A358;
    return;
L_09D2A354:
    ctx.gpr[7] = (0u + static_cast<std::uint32_t>(35));
    goto L_09D2A358;
L_09D2A358:
    { const bool branch_taken = 0u == 0u;
    // nop
      if (branch_taken) {
          goto L_09D2A634;
      }
      goto L_09D2A360;
    }
L_09D2A35C:
    // nop
    goto L_09D2A360;
L_09D2A360:
    ctx.gpr[5] = (0u + static_cast<std::uint32_t>(36));
    goto L_09D2A364;
L_09D2A364:
    ctx.gpr[6] = (0u + static_cast<std::uint32_t>(23));
    goto L_09D2A368;
L_09D2A368:
    ctx.gpr[31] = (0x09D2A370u);
    ctx.gpr[7] = (0u + 0u);
    ctx.pc = 0x09ACB620u;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D2A370u) goto L_09D2A370;
    return;
L_09D2A36C:
    ctx.gpr[7] = (0u + 0u);
    goto L_09D2A370;
L_09D2A370:
    ctx.gpr[4] = (ctx.gpr[17] + 0u);
    goto L_09D2A374;
L_09D2A374:
    ctx.gpr[5] = (0u + static_cast<std::uint32_t>(18));
    goto L_09D2A378;
L_09D2A378:
    ctx.gpr[6] = (0u + static_cast<std::uint32_t>(22));
    goto L_09D2A37C;
L_09D2A37C:
    ctx.gpr[31] = (0x09D2A384u);
    ctx.gpr[7] = (0u + static_cast<std::uint32_t>(34));
    ctx.pc = 0x09ACB620u;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D2A384u) goto L_09D2A384;
    return;
L_09D2A380:
    ctx.gpr[7] = (0u + static_cast<std::uint32_t>(34));
    goto L_09D2A384;
L_09D2A384:
    ctx.gpr[4] = (ctx.gpr[17] + 0u);
    goto L_09D2A388;
L_09D2A388:
    ctx.gpr[5] = (0u + static_cast<std::uint32_t>(70));
    goto L_09D2A38C;
L_09D2A38C:
    ctx.gpr[6] = (0u + static_cast<std::uint32_t>(22));
    goto L_09D2A390;
L_09D2A390:
    ctx.gpr[31] = (0x09D2A398u);
    ctx.gpr[7] = (0u + static_cast<std::uint32_t>(34));
    ctx.pc = 0x09ACB620u;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D2A398u) goto L_09D2A398;
    return;
L_09D2A394:
    ctx.gpr[7] = (0u + static_cast<std::uint32_t>(34));
    goto L_09D2A398;
L_09D2A398:
    { const bool branch_taken = 0u == 0u;
    // nop
      if (branch_taken) {
          goto L_09D2A634;
      }
      goto L_09D2A3A0;
    }
L_09D2A39C:
    // nop
    goto L_09D2A3A0;
L_09D2A3A0:
    ctx.gpr[5] = (0u + static_cast<std::uint32_t>(10));
    goto L_09D2A3A4;
L_09D2A3A4:
    ctx.gpr[6] = (0u + static_cast<std::uint32_t>(6));
    goto L_09D2A3A8;
L_09D2A3A8:
    ctx.gpr[31] = (0x09D2A3B0u);
    ctx.gpr[7] = (0u + static_cast<std::uint32_t>(20));
    ctx.pc = 0x09ACB620u;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D2A3B0u) goto L_09D2A3B0;
    return;
L_09D2A3AC:
    ctx.gpr[7] = (0u + static_cast<std::uint32_t>(20));
    goto L_09D2A3B0;
L_09D2A3B0:
    ctx.gpr[4] = (ctx.gpr[17] + 0u);
    goto L_09D2A3B4;
L_09D2A3B4:
    ctx.gpr[5] = (0u + static_cast<std::uint32_t>(4));
    goto L_09D2A3B8;
L_09D2A3B8:
    ctx.gpr[6] = (0u + static_cast<std::uint32_t>(31));
    goto L_09D2A3BC;
L_09D2A3BC:
    ctx.gpr[31] = (0x09D2A3C4u);
    ctx.gpr[7] = (0u + static_cast<std::uint32_t>(35));
    ctx.pc = 0x09ACB620u;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D2A3C4u) goto L_09D2A3C4;
    return;
L_09D2A3C0:
    ctx.gpr[7] = (0u + static_cast<std::uint32_t>(35));
    goto L_09D2A3C4;
L_09D2A3C4:
    ctx.gpr[4] = (ctx.gpr[17] + 0u);
    goto L_09D2A3C8;
L_09D2A3C8:
    ctx.gpr[5] = (0u + static_cast<std::uint32_t>(30));
    goto L_09D2A3CC;
L_09D2A3CC:
    ctx.gpr[6] = (0u + static_cast<std::uint32_t>(18));
    goto L_09D2A3D0;
L_09D2A3D0:
    ctx.gpr[31] = (0x09D2A3D8u);
    ctx.gpr[7] = (0u + static_cast<std::uint32_t>(20));
    ctx.pc = 0x09ACB620u;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D2A3D8u) goto L_09D2A3D8;
    return;
L_09D2A3D4:
    ctx.gpr[7] = (0u + static_cast<std::uint32_t>(20));
    goto L_09D2A3D8;
L_09D2A3D8:
    ctx.gpr[4] = (ctx.gpr[17] + 0u);
    goto L_09D2A3DC;
L_09D2A3DC:
    ctx.gpr[5] = (0u + static_cast<std::uint32_t>(30));
    goto L_09D2A3E0;
L_09D2A3E0:
    ctx.gpr[6] = (0u + static_cast<std::uint32_t>(21));
    goto L_09D2A3E4;
L_09D2A3E4:
    ctx.gpr[31] = (0x09D2A3ECu);
    ctx.gpr[7] = (0u + 0u);
    ctx.pc = 0x09ACB620u;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D2A3ECu) goto L_09D2A3EC;
    return;
L_09D2A3E8:
    ctx.gpr[7] = (0u + 0u);
    goto L_09D2A3EC;
L_09D2A3EC:
    ctx.gpr[4] = (ctx.gpr[17] + 0u);
    goto L_09D2A3F0;
L_09D2A3F0:
    ctx.gpr[5] = (0u + static_cast<std::uint32_t>(30));
    goto L_09D2A3F4;
L_09D2A3F4:
    ctx.gpr[6] = (0u + static_cast<std::uint32_t>(20));
    goto L_09D2A3F8;
L_09D2A3F8:
    ctx.gpr[31] = (0x09D2A400u);
    ctx.gpr[7] = (0u + 0u);
    ctx.pc = 0x09ACB620u;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D2A400u) goto L_09D2A400;
    return;
L_09D2A3FC:
    ctx.gpr[7] = (0u + 0u);
    goto L_09D2A400;
L_09D2A400:
    { const bool branch_taken = 0u == 0u;
    // nop
      if (branch_taken) {
          goto L_09D2A634;
      }
      goto L_09D2A408;
    }
L_09D2A404:
    // nop
    goto L_09D2A408;
L_09D2A408:
    ctx.gpr[5] = (0u + static_cast<std::uint32_t>(4));
    goto L_09D2A40C;
L_09D2A40C:
    ctx.gpr[6] = (0u + static_cast<std::uint32_t>(52));
    goto L_09D2A410;
L_09D2A410:
    ctx.gpr[31] = (0x09D2A418u);
    ctx.gpr[7] = (0u + static_cast<std::uint32_t>(35));
    ctx.pc = 0x09ACB620u;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D2A418u) goto L_09D2A418;
    return;
L_09D2A414:
    ctx.gpr[7] = (0u + static_cast<std::uint32_t>(35));
    goto L_09D2A418;
L_09D2A418:
    ctx.gpr[4] = (ctx.gpr[17] + 0u);
    goto L_09D2A41C;
L_09D2A41C:
    ctx.gpr[5] = (0u + static_cast<std::uint32_t>(30));
    goto L_09D2A420;
L_09D2A420:
    ctx.gpr[6] = (0u + static_cast<std::uint32_t>(48));
    goto L_09D2A424;
L_09D2A424:
    ctx.gpr[31] = (0x09D2A42Cu);
    ctx.gpr[7] = (0u + 0u);
    ctx.pc = 0x09ACB620u;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D2A42Cu) goto L_09D2A42C;
    return;
L_09D2A428:
    ctx.gpr[7] = (0u + 0u);
    goto L_09D2A42C;
L_09D2A42C:
    { const bool branch_taken = 0u == 0u;
    // nop
      if (branch_taken) {
          goto L_09D2A634;
      }
      goto L_09D2A434;
    }
L_09D2A430:
    // nop
    goto L_09D2A434;
L_09D2A434:
    ctx.gpr[5] = (0u + static_cast<std::uint32_t>(4));
    goto L_09D2A438;
L_09D2A438:
    ctx.gpr[6] = (0u + static_cast<std::uint32_t>(52));
    goto L_09D2A43C;
L_09D2A43C:
    ctx.gpr[31] = (0x09D2A444u);
    ctx.gpr[7] = (0u + static_cast<std::uint32_t>(35));
    ctx.pc = 0x09ACB620u;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D2A444u) goto L_09D2A444;
    return;
L_09D2A440:
    ctx.gpr[7] = (0u + static_cast<std::uint32_t>(35));
    goto L_09D2A444;
L_09D2A444:
    ctx.gpr[4] = (ctx.gpr[17] + 0u);
    goto L_09D2A448;
L_09D2A448:
    ctx.gpr[5] = (0u + static_cast<std::uint32_t>(30));
    goto L_09D2A44C;
L_09D2A44C:
    ctx.gpr[6] = (0u + static_cast<std::uint32_t>(48));
    goto L_09D2A450;
L_09D2A450:
    ctx.gpr[31] = (0x09D2A458u);
    ctx.gpr[7] = (0u + 0u);
    ctx.pc = 0x09ACB620u;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D2A458u) goto L_09D2A458;
    return;
L_09D2A454:
    ctx.gpr[7] = (0u + 0u);
    goto L_09D2A458;
L_09D2A458:
    { const bool branch_taken = 0u == 0u;
    // nop
      if (branch_taken) {
          goto L_09D2A634;
      }
      goto L_09D2A460;
    }
L_09D2A45C:
    // nop
    goto L_09D2A460;
L_09D2A460:
    ctx.gpr[5] = (0u + static_cast<std::uint32_t>(4));
    goto L_09D2A464;
L_09D2A464:
    ctx.gpr[6] = (0u + static_cast<std::uint32_t>(18));
    goto L_09D2A468;
L_09D2A468:
    ctx.gpr[31] = (0x09D2A470u);
    ctx.gpr[7] = (0u + static_cast<std::uint32_t>(26));
    ctx.pc = 0x09ACB620u;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D2A470u) goto L_09D2A470;
    return;
L_09D2A46C:
    ctx.gpr[7] = (0u + static_cast<std::uint32_t>(26));
    goto L_09D2A470;
L_09D2A470:
    ctx.gpr[4] = (ctx.gpr[17] + 0u);
    goto L_09D2A474;
L_09D2A474:
    ctx.gpr[5] = (0u + static_cast<std::uint32_t>(10));
    goto L_09D2A478;
L_09D2A478:
    ctx.gpr[6] = (0u + static_cast<std::uint32_t>(3));
    goto L_09D2A47C;
L_09D2A47C:
    ctx.gpr[31] = (0x09D2A484u);
    ctx.gpr[7] = (0u + static_cast<std::uint32_t>(26));
    ctx.pc = 0x09ACB620u;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D2A484u) goto L_09D2A484;
    return;
L_09D2A480:
    ctx.gpr[7] = (0u + static_cast<std::uint32_t>(26));
    goto L_09D2A484;
L_09D2A484:
    ctx.gpr[4] = (ctx.gpr[17] + 0u);
    goto L_09D2A488;
L_09D2A488:
    ctx.gpr[5] = (0u + static_cast<std::uint32_t>(112));
    goto L_09D2A48C;
L_09D2A48C:
    ctx.gpr[6] = (0u + static_cast<std::uint32_t>(4));
    goto L_09D2A490;
L_09D2A490:
    ctx.gpr[31] = (0x09D2A498u);
    ctx.gpr[7] = (0u + static_cast<std::uint32_t>(20));
    ctx.pc = 0x09ACB620u;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D2A498u) goto L_09D2A498;
    return;
L_09D2A494:
    ctx.gpr[7] = (0u + static_cast<std::uint32_t>(20));
    goto L_09D2A498;
L_09D2A498:
    ctx.gpr[4] = (ctx.gpr[17] + 0u);
    goto L_09D2A49C;
L_09D2A49C:
    ctx.gpr[5] = (0u + static_cast<std::uint32_t>(216));
    goto L_09D2A4A0;
L_09D2A4A0:
    ctx.gpr[6] = (0u + static_cast<std::uint32_t>(1));
    goto L_09D2A4A4;
L_09D2A4A4:
    ctx.gpr[31] = (0x09D2A4ACu);
    ctx.gpr[7] = (0u + static_cast<std::uint32_t>(26));
    ctx.pc = 0x09ACB620u;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D2A4ACu) goto L_09D2A4AC;
    return;
L_09D2A4A8:
    ctx.gpr[7] = (0u + static_cast<std::uint32_t>(26));
    goto L_09D2A4AC;
L_09D2A4AC:
    ctx.gpr[4] = (ctx.gpr[17] + 0u);
    goto L_09D2A4B0;
L_09D2A4B0:
    ctx.gpr[5] = (0u + static_cast<std::uint32_t>(264));
    goto L_09D2A4B4;
L_09D2A4B4:
    ctx.gpr[6] = (0u + static_cast<std::uint32_t>(3));
    goto L_09D2A4B8;
L_09D2A4B8:
    ctx.gpr[31] = (0x09D2A4C0u);
    ctx.gpr[7] = (0u + static_cast<std::uint32_t>(20));
    ctx.pc = 0x09ACB620u;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D2A4C0u) goto L_09D2A4C0;
    return;
L_09D2A4BC:
    ctx.gpr[7] = (0u + static_cast<std::uint32_t>(20));
    goto L_09D2A4C0;
L_09D2A4C0:
    ctx.gpr[4] = (ctx.gpr[17] + 0u);
    goto L_09D2A4C4;
L_09D2A4C4:
    ctx.gpr[5] = (0u + static_cast<std::uint32_t>(56));
    goto L_09D2A4C8;
L_09D2A4C8:
    ctx.gpr[6] = (0u + static_cast<std::uint32_t>(21));
    goto L_09D2A4CC;
L_09D2A4CC:
    ctx.gpr[31] = (0x09D2A4D4u);
    ctx.gpr[7] = (0u + 0u);
    ctx.pc = 0x09ACB620u;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D2A4D4u) goto L_09D2A4D4;
    return;
L_09D2A4D0:
    ctx.gpr[7] = (0u + 0u);
    goto L_09D2A4D4;
L_09D2A4D4:
    ctx.gpr[4] = (ctx.gpr[17] + 0u);
    goto L_09D2A4D8;
L_09D2A4D8:
    ctx.gpr[5] = (0u + static_cast<std::uint32_t>(56));
    goto L_09D2A4DC;
L_09D2A4DC:
    ctx.gpr[6] = (0u + static_cast<std::uint32_t>(8));
    goto L_09D2A4E0;
L_09D2A4E0:
    ctx.gpr[31] = (0x09D2A4E8u);
    ctx.gpr[7] = (0u + 0u);
    ctx.pc = 0x09ACB620u;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D2A4E8u) goto L_09D2A4E8;
    return;
L_09D2A4E4:
    ctx.gpr[7] = (0u + 0u);
    goto L_09D2A4E8;
L_09D2A4E8:
    ctx.gpr[4] = (ctx.gpr[17] + 0u);
    goto L_09D2A4EC;
L_09D2A4EC:
    ctx.gpr[5] = (0u + static_cast<std::uint32_t>(4));
    goto L_09D2A4F0;
L_09D2A4F0:
    ctx.gpr[6] = (0u + static_cast<std::uint32_t>(31));
    goto L_09D2A4F4;
L_09D2A4F4:
    ctx.gpr[31] = (0x09D2A4FCu);
    ctx.gpr[7] = (0u + static_cast<std::uint32_t>(35));
    ctx.pc = 0x09ACB620u;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D2A4FCu) goto L_09D2A4FC;
    return;
L_09D2A4F8:
    ctx.gpr[7] = (0u + static_cast<std::uint32_t>(35));
    goto L_09D2A4FC;
L_09D2A4FC:
    ctx.gpr[4] = (ctx.gpr[17] + 0u);
    goto L_09D2A500;
L_09D2A500:
    ctx.gpr[5] = (0u + static_cast<std::uint32_t>(84));
    goto L_09D2A504;
L_09D2A504:
    ctx.gpr[6] = (0u + static_cast<std::uint32_t>(22));
    goto L_09D2A508;
L_09D2A508:
    ctx.gpr[31] = (0x09D2A510u);
    ctx.gpr[7] = (0u + 0u);
    ctx.pc = 0x09ACB620u;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D2A510u) goto L_09D2A510;
    return;
L_09D2A50C:
    ctx.gpr[7] = (0u + 0u);
    goto L_09D2A510;
L_09D2A510:
    { const bool branch_taken = 0u == 0u;
    // nop
      if (branch_taken) {
          goto L_09D2A634;
      }
      goto L_09D2A518;
    }
L_09D2A514:
    // nop
    goto L_09D2A518;
L_09D2A518:
    ctx.gpr[5] = (0u + static_cast<std::uint32_t>(4));
    goto L_09D2A51C;
L_09D2A51C:
    ctx.gpr[6] = (0u + static_cast<std::uint32_t>(87));
    goto L_09D2A520;
L_09D2A520:
    ctx.gpr[31] = (0x09D2A528u);
    ctx.gpr[7] = (0u + static_cast<std::uint32_t>(35));
    ctx.pc = 0x09ACB620u;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D2A528u) goto L_09D2A528;
    return;
L_09D2A524:
    ctx.gpr[7] = (0u + static_cast<std::uint32_t>(35));
    goto L_09D2A528;
L_09D2A528:
    ctx.gpr[4] = (ctx.gpr[17] + 0u);
    goto L_09D2A52C;
L_09D2A52C:
    ctx.gpr[5] = (0u + static_cast<std::uint32_t>(290));
    goto L_09D2A530;
L_09D2A530:
    ctx.gpr[6] = (0u + static_cast<std::uint32_t>(85));
    goto L_09D2A534;
L_09D2A534:
    ctx.gpr[31] = (0x09D2A53Cu);
    ctx.gpr[7] = (0u + static_cast<std::uint32_t>(35));
    ctx.pc = 0x09ACB620u;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D2A53Cu) goto L_09D2A53C;
    return;
L_09D2A538:
    ctx.gpr[7] = (0u + static_cast<std::uint32_t>(35));
    goto L_09D2A53C;
L_09D2A53C:
    { const bool branch_taken = 0u == 0u;
    // nop
      if (branch_taken) {
          goto L_09D2A634;
      }
      goto L_09D2A544;
    }
L_09D2A540:
    // nop
    goto L_09D2A544;
L_09D2A544:
    ctx.gpr[5] = (0u + static_cast<std::uint32_t>(26));
    goto L_09D2A548;
L_09D2A548:
    ctx.gpr[6] = (0u + static_cast<std::uint32_t>(83));
    goto L_09D2A54C;
L_09D2A54C:
    ctx.gpr[31] = (0x09D2A554u);
    ctx.gpr[7] = (0u + static_cast<std::uint32_t>(35));
    ctx.pc = 0x09ACB620u;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D2A554u) goto L_09D2A554;
    return;
L_09D2A550:
    ctx.gpr[7] = (0u + static_cast<std::uint32_t>(35));
    goto L_09D2A554;
L_09D2A554:
    ctx.gpr[4] = (ctx.gpr[17] + 0u);
    goto L_09D2A558;
L_09D2A558:
    ctx.gpr[5] = (0u + static_cast<std::uint32_t>(120));
    goto L_09D2A55C;
L_09D2A55C:
    ctx.gpr[6] = (0u + static_cast<std::uint32_t>(82));
    goto L_09D2A560;
L_09D2A560:
    ctx.gpr[31] = (0x09D2A568u);
    ctx.gpr[7] = (0u + static_cast<std::uint32_t>(35));
    ctx.pc = 0x09ACB620u;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D2A568u) goto L_09D2A568;
    return;
L_09D2A564:
    ctx.gpr[7] = (0u + static_cast<std::uint32_t>(35));
    goto L_09D2A568;
L_09D2A568:
    { const bool branch_taken = 0u == 0u;
    // nop
      if (branch_taken) {
          goto L_09D2A634;
      }
      goto L_09D2A570;
    }
L_09D2A56C:
    // nop
    goto L_09D2A570;
L_09D2A570:
    ctx.gpr[5] = (0u + static_cast<std::uint32_t>(88));
    goto L_09D2A574;
L_09D2A574:
    ctx.gpr[6] = (0u + static_cast<std::uint32_t>(81));
    goto L_09D2A578;
L_09D2A578:
    ctx.gpr[31] = (0x09D2A580u);
    ctx.gpr[7] = (0u + static_cast<std::uint32_t>(35));
    ctx.pc = 0x09ACB620u;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D2A580u) goto L_09D2A580;
    return;
L_09D2A57C:
    ctx.gpr[7] = (0u + static_cast<std::uint32_t>(35));
    goto L_09D2A580;
L_09D2A580:
    ctx.gpr[4] = (ctx.gpr[17] + 0u);
    goto L_09D2A584;
L_09D2A584:
    ctx.gpr[5] = (0u + static_cast<std::uint32_t>(116));
    goto L_09D2A588;
L_09D2A588:
    ctx.gpr[6] = (0u + static_cast<std::uint32_t>(24));
    goto L_09D2A58C;
L_09D2A58C:
    ctx.gpr[31] = (0x09D2A594u);
    ctx.gpr[7] = (0u + static_cast<std::uint32_t>(35));
    ctx.pc = 0x09ACB620u;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D2A594u) goto L_09D2A594;
    return;
L_09D2A590:
    ctx.gpr[7] = (0u + static_cast<std::uint32_t>(35));
    goto L_09D2A594;
L_09D2A594:
    ctx.gpr[4] = (ctx.gpr[17] + 0u);
    goto L_09D2A598;
L_09D2A598:
    ctx.gpr[5] = (0u + static_cast<std::uint32_t>(4));
    goto L_09D2A59C;
L_09D2A59C:
    ctx.gpr[6] = (0u + static_cast<std::uint32_t>(40));
    goto L_09D2A5A0;
L_09D2A5A0:
    ctx.gpr[31] = (0x09D2A5A8u);
    ctx.gpr[7] = (0u + static_cast<std::uint32_t>(35));
    ctx.pc = 0x09ACB620u;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D2A5A8u) goto L_09D2A5A8;
    return;
L_09D2A5A4:
    ctx.gpr[7] = (0u + static_cast<std::uint32_t>(35));
    goto L_09D2A5A8;
L_09D2A5A8:
    ctx.gpr[4] = (ctx.gpr[17] + 0u);
    goto L_09D2A5AC;
L_09D2A5AC:
    ctx.gpr[5] = (0u + static_cast<std::uint32_t>(4));
    goto L_09D2A5B0;
L_09D2A5B0:
    ctx.gpr[6] = (0u + static_cast<std::uint32_t>(12));
    goto L_09D2A5B4;
L_09D2A5B4:
    ctx.gpr[31] = (0x09D2A5BCu);
    ctx.gpr[7] = (0u + static_cast<std::uint32_t>(6));
    ctx.pc = 0x09ACB620u;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D2A5BCu) goto L_09D2A5BC;
    return;
L_09D2A5B8:
    ctx.gpr[7] = (0u + static_cast<std::uint32_t>(6));
    goto L_09D2A5BC;
L_09D2A5BC:
    ctx.gpr[6] = (0u + static_cast<std::uint32_t>(12));
    goto L_09D2A5C0;
L_09D2A5C0:
    ctx.gpr[4] = (ctx.gpr[17] + 0u);
    goto L_09D2A5C4;
L_09D2A5C4:
    ctx.gpr[5] = (0u + static_cast<std::uint32_t>(8));
    goto L_09D2A5C8;
L_09D2A5C8:
    ctx.gpr[31] = (0x09D2A5D0u);
    ctx.gpr[7] = (ctx.gpr[6] + 0u);
    ctx.pc = 0x09ACB620u;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D2A5D0u) goto L_09D2A5D0;
    return;
L_09D2A5CC:
    ctx.gpr[7] = (ctx.gpr[6] + 0u);
    goto L_09D2A5D0;
L_09D2A5D0:
    ctx.gpr[4] = (ctx.gpr[17] + 0u);
    goto L_09D2A5D4;
L_09D2A5D4:
    ctx.gpr[5] = (0u + static_cast<std::uint32_t>(56));
    goto L_09D2A5D8;
L_09D2A5D8:
    ctx.gpr[6] = (0u + static_cast<std::uint32_t>(2));
    goto L_09D2A5DC;
L_09D2A5DC:
    ctx.gpr[31] = (0x09D2A5E4u);
    ctx.gpr[7] = (0u + static_cast<std::uint32_t>(20));
    ctx.pc = 0x09ACB620u;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D2A5E4u) goto L_09D2A5E4;
    return;
L_09D2A5E0:
    ctx.gpr[7] = (0u + static_cast<std::uint32_t>(20));
    goto L_09D2A5E4;
L_09D2A5E4:
    ctx.gpr[4] = (ctx.gpr[17] + 0u);
    goto L_09D2A5E8;
L_09D2A5E8:
    ctx.gpr[5] = (0u + static_cast<std::uint32_t>(62));
    goto L_09D2A5EC;
L_09D2A5EC:
    ctx.gpr[6] = (0u + static_cast<std::uint32_t>(4));
    goto L_09D2A5F0;
L_09D2A5F0:
    ctx.gpr[31] = (0x09D2A5F8u);
    ctx.gpr[7] = (0u + static_cast<std::uint32_t>(26));
    ctx.pc = 0x09ACB620u;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D2A5F8u) goto L_09D2A5F8;
    return;
L_09D2A5F4:
    ctx.gpr[7] = (0u + static_cast<std::uint32_t>(26));
    goto L_09D2A5F8;
L_09D2A5F8:
    ctx.gpr[4] = (ctx.gpr[17] + 0u);
    goto L_09D2A5FC;
L_09D2A5FC:
    ctx.gpr[5] = (0u + static_cast<std::uint32_t>(246));
    goto L_09D2A600;
L_09D2A600:
    ctx.gpr[6] = (0u + static_cast<std::uint32_t>(3));
    goto L_09D2A604;
L_09D2A604:
    ctx.gpr[31] = (0x09D2A60Cu);
    ctx.gpr[7] = (0u + static_cast<std::uint32_t>(20));
    ctx.pc = 0x09ACB620u;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D2A60Cu) goto L_09D2A60C;
    return;
L_09D2A608:
    ctx.gpr[7] = (0u + static_cast<std::uint32_t>(20));
    goto L_09D2A60C;
L_09D2A60C:
    ctx.gpr[4] = (ctx.gpr[17] + 0u);
    goto L_09D2A610;
L_09D2A610:
    ctx.gpr[5] = (0u + static_cast<std::uint32_t>(286));
    goto L_09D2A614;
L_09D2A614:
    ctx.gpr[6] = (0u + static_cast<std::uint32_t>(3));
    goto L_09D2A618;
L_09D2A618:
    ctx.gpr[31] = (0x09D2A620u);
    ctx.gpr[7] = (0u + static_cast<std::uint32_t>(26));
    ctx.pc = 0x09ACB620u;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D2A620u) goto L_09D2A620;
    return;
L_09D2A61C:
    ctx.gpr[7] = (0u + static_cast<std::uint32_t>(26));
    goto L_09D2A620;
L_09D2A620:
    ctx.gpr[4] = (ctx.gpr[17] + 0u);
    goto L_09D2A624;
L_09D2A624:
    ctx.gpr[5] = (0u + static_cast<std::uint32_t>(244));
    goto L_09D2A628;
L_09D2A628:
    ctx.gpr[6] = (0u + static_cast<std::uint32_t>(12));
    goto L_09D2A62C;
L_09D2A62C:
    ctx.gpr[31] = (0x09D2A634u);
    ctx.gpr[7] = (0u + 0u);
    ctx.pc = 0x09ACB620u;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D2A634u) goto L_09D2A634;
    return;
L_09D2A630:
    ctx.gpr[7] = (0u + 0u);
    goto L_09D2A634;
L_09D2A634:
    ctx.gpr[31] = (aot_mem.aot_load32(ctx.gpr[29] + static_cast<std::uint32_t>(12)));
    goto L_09D2A638;
L_09D2A638:
    ctx.gpr[17] = (aot_mem.aot_load32(ctx.gpr[29] + static_cast<std::uint32_t>(8)));
    goto L_09D2A63C;
L_09D2A63C:
    ctx.gpr[16] = (aot_mem.aot_load32(ctx.gpr[29] + static_cast<std::uint32_t>(4)));
    goto L_09D2A640;
L_09D2A640:
    jump_target = ctx.gpr[31];
    ctx.gpr[29] = (ctx.gpr[29] + static_cast<std::uint32_t>(16));
    local_pc = jump_target;
    if (++local_transfers < 2048u) { entry_id = 0u; goto LOCAL_DISPATCH; }
    ctx.pc = jump_target;
    return;
L_09D2A644:
    ctx.gpr[29] = (ctx.gpr[29] + static_cast<std::uint32_t>(16));
    goto L_09D2A648;
L_09D2A648:
    ctx.gpr[29] = (ctx.gpr[29] + static_cast<std::uint32_t>(-16));
    goto L_09D2A64C;
L_09D2A64C:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(12), ctx.gpr[31]);
    goto L_09D2A650;
L_09D2A650:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(8), ctx.gpr[16]);
    goto L_09D2A654;
L_09D2A654:
    ctx.gpr[31] = (0x09D2A65Cu);
    ctx.gpr[16] = (ctx.gpr[4] + 0u);
    ctx.pc = 0x09B67178u;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D2A65Cu) goto L_09D2A65C;
    return;
L_09D2A658:
    ctx.gpr[16] = (ctx.gpr[4] + 0u);
    goto L_09D2A65C;
L_09D2A65C:
    ctx.gpr[2] = (2204u << 16u);
    goto L_09D2A660;
L_09D2A660:
    ctx.gpr[2] = (ctx.gpr[2] + static_cast<std::uint32_t>(-5200));
    goto L_09D2A664;
L_09D2A664:
    aot_mem.aot_store32(ctx.gpr[16] + static_cast<std::uint32_t>(0), ctx.gpr[2]);
    goto L_09D2A668;
L_09D2A668:
    ctx.gpr[2] = (ctx.gpr[16] + 0u);
    goto L_09D2A66C;
L_09D2A66C:
    ctx.gpr[31] = (aot_mem.aot_load32(ctx.gpr[29] + static_cast<std::uint32_t>(12)));
    goto L_09D2A670;
L_09D2A670:
    ctx.gpr[16] = (aot_mem.aot_load32(ctx.gpr[29] + static_cast<std::uint32_t>(8)));
    goto L_09D2A674;
L_09D2A674:
    jump_target = ctx.gpr[31];
    ctx.gpr[29] = (ctx.gpr[29] + static_cast<std::uint32_t>(16));
    local_pc = jump_target;
    if (++local_transfers < 2048u) { entry_id = 0u; goto LOCAL_DISPATCH; }
    ctx.pc = jump_target;
    return;
L_09D2A678:
    ctx.gpr[29] = (ctx.gpr[29] + static_cast<std::uint32_t>(16));
    goto L_09D2A67C;
L_09D2A67C:
    // nop
    goto L_09D2A680;
L_09D2A680:
    ctx.gpr[29] = (ctx.gpr[29] + static_cast<std::uint32_t>(-16));
    goto L_09D2A684;
L_09D2A684:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(12), ctx.gpr[31]);
    goto L_09D2A688;
L_09D2A688:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(8), ctx.gpr[17]);
    goto L_09D2A68C;
L_09D2A68C:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(4), ctx.gpr[16]);
    goto L_09D2A690;
L_09D2A690:
    ctx.gpr[17] = (ctx.gpr[4] + 0u);
    goto L_09D2A694;
L_09D2A694:
    { const bool branch_taken = ctx.gpr[17] == 0u;
    ctx.gpr[16] = (ctx.gpr[5] + 0u);
      if (branch_taken) {
          goto L_09D2A6C4;
      }
      goto L_09D2A69C;
    }
L_09D2A698:
    ctx.gpr[16] = (ctx.gpr[5] + 0u);
    goto L_09D2A69C;
L_09D2A69C:
    ctx.gpr[2] = (2204u << 16u);
    goto L_09D2A6A0;
L_09D2A6A0:
    ctx.gpr[2] = (ctx.gpr[2] + static_cast<std::uint32_t>(-5200));
    goto L_09D2A6A4;
L_09D2A6A4:
    aot_mem.aot_store32(ctx.gpr[17] + static_cast<std::uint32_t>(0), ctx.gpr[2]);
    goto L_09D2A6A8;
L_09D2A6A8:
    ctx.gpr[31] = (0x09D2A6B0u);
    ctx.gpr[5] = (0u + 0u);
    ctx.pc = 0x09B67210u;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D2A6B0u) goto L_09D2A6B0;
    return;
L_09D2A6AC:
    ctx.gpr[5] = (0u + 0u);
    goto L_09D2A6B0;
L_09D2A6B0:
    ctx.gpr[2] = (static_cast<std::uint32_t>(static_cast<std::int32_t>(static_cast<std::int16_t>(ctx.gpr[16]))));
    goto L_09D2A6B4;
L_09D2A6B4:
    if (static_cast<std::int32_t>(ctx.gpr[2]) <= 0) {
    ctx.gpr[2] = (ctx.gpr[17] + 0u);
        goto L_09D2A6C8;
    }
    goto L_09D2A6BC;
L_09D2A6B8:
    ctx.gpr[2] = (ctx.gpr[17] + 0u);
    goto L_09D2A6BC;
L_09D2A6BC:
    ctx.gpr[31] = (0x09D2A6C4u);
    ctx.gpr[4] = (ctx.gpr[17] + 0u);
    ctx.pc = 0x09B67260u;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D2A6C4u) goto L_09D2A6C4;
    return;
L_09D2A6C0:
    ctx.gpr[4] = (ctx.gpr[17] + 0u);
    goto L_09D2A6C4;
L_09D2A6C4:
    ctx.gpr[2] = (ctx.gpr[17] + 0u);
    goto L_09D2A6C8;
L_09D2A6C8:
    ctx.gpr[31] = (aot_mem.aot_load32(ctx.gpr[29] + static_cast<std::uint32_t>(12)));
    goto L_09D2A6CC;
L_09D2A6CC:
    ctx.gpr[17] = (aot_mem.aot_load32(ctx.gpr[29] + static_cast<std::uint32_t>(8)));
    goto L_09D2A6D0;
L_09D2A6D0:
    ctx.gpr[16] = (aot_mem.aot_load32(ctx.gpr[29] + static_cast<std::uint32_t>(4)));
    goto L_09D2A6D4;
L_09D2A6D4:
    jump_target = ctx.gpr[31];
    ctx.gpr[29] = (ctx.gpr[29] + static_cast<std::uint32_t>(16));
    local_pc = jump_target;
    if (++local_transfers < 2048u) { entry_id = 0u; goto LOCAL_DISPATCH; }
    ctx.pc = jump_target;
    return;
L_09D2A6D8:
    ctx.gpr[29] = (ctx.gpr[29] + static_cast<std::uint32_t>(16));
    goto L_09D2A6DC;
L_09D2A6DC:
    // nop
    goto L_09D2A6E0;
L_09D2A6E0:
    ctx.gpr[3] = (0u + static_cast<std::uint32_t>(20));
    goto L_09D2A6E4;
L_09D2A6E4:
    aot_mem.aot_store8(ctx.gpr[4] + static_cast<std::uint32_t>(12), static_cast<std::uint8_t>(ctx.gpr[3]));
    goto L_09D2A6E8;
L_09D2A6E8:
    aot_mem.aot_store8(ctx.gpr[4] + static_cast<std::uint32_t>(13), static_cast<std::uint8_t>(ctx.gpr[6]));
    goto L_09D2A6EC;
L_09D2A6EC:
    ctx.gpr[6] = (aot_mem.aot_load16(ctx.gpr[5] + static_cast<std::uint32_t>(486)));
    goto L_09D2A6F0;
L_09D2A6F0:
    ctx.gpr[3] = (2498u << 16u);
    goto L_09D2A6F4;
L_09D2A6F4:
    aot_mem.aot_store8(ctx.gpr[4] + static_cast<std::uint32_t>(21), static_cast<std::uint8_t>(ctx.gpr[6]));
    goto L_09D2A6F8;
L_09D2A6F8:
    ctx.gpr[6] = (aot_mem.aot_load8(ctx.gpr[5] + static_cast<std::uint32_t>(489)));
    goto L_09D2A6FC;
L_09D2A6FC:
    aot_mem.aot_store8(ctx.gpr[4] + static_cast<std::uint32_t>(15), static_cast<std::uint8_t>(ctx.gpr[6]));
    goto L_09D2A700;
L_09D2A700:
    ctx.gpr[6] = (aot_mem.aot_load16(ctx.gpr[5] + static_cast<std::uint32_t>(804)));
    goto L_09D2A704;
L_09D2A704:
    aot_mem.aot_store16(ctx.gpr[4] + static_cast<std::uint32_t>(194), static_cast<std::uint16_t>(ctx.gpr[6]));
    goto L_09D2A708;
L_09D2A708:
    aot_mem.aot_store32(ctx.gpr[4] + static_cast<std::uint32_t>(16), ctx.gpr[5]);
    goto L_09D2A70C;
L_09D2A70C:
    ctx.gpr[6] = (aot_mem.aot_load32(ctx.gpr[5] + static_cast<std::uint32_t>(500)));
    goto L_09D2A710;
L_09D2A710:
    aot_mem.aot_store16(ctx.gpr[4] + static_cast<std::uint32_t>(102), static_cast<std::uint16_t>(ctx.gpr[6]));
    goto L_09D2A714;
L_09D2A714:
    ctx.gpr[6] = (aot_mem.aot_load32(ctx.gpr[5] + static_cast<std::uint32_t>(496)));
    goto L_09D2A718;
L_09D2A718:
    aot_mem.aot_store32(ctx.gpr[4] + static_cast<std::uint32_t>(144), ctx.gpr[6]);
    goto L_09D2A71C;
L_09D2A71C:
    ctx.gpr[6] = (aot_mem.aot_load32(ctx.gpr[5] + static_cast<std::uint32_t>(500)));
    goto L_09D2A720;
L_09D2A720:
    aot_mem.aot_store32(ctx.gpr[4] + static_cast<std::uint32_t>(148), ctx.gpr[6]);
    goto L_09D2A724;
L_09D2A724:
    ctx.gpr[6] = (aot_mem.aot_load32(ctx.gpr[5] + static_cast<std::uint32_t>(504)));
    goto L_09D2A728;
L_09D2A728:
    aot_mem.aot_store32(ctx.gpr[4] + static_cast<std::uint32_t>(152), ctx.gpr[6]);
    goto L_09D2A72C;
L_09D2A72C:
    aot_mem.aot_store8(ctx.gpr[5] + static_cast<std::uint32_t>(829), static_cast<std::uint8_t>(0u));
    goto L_09D2A730;
L_09D2A730:
    aot_mem.aot_store8(ctx.gpr[5] + static_cast<std::uint32_t>(1016), static_cast<std::uint8_t>(0u));
    goto L_09D2A734;
L_09D2A734:
    ctx.gpr[5] = (aot_mem.aot_load32(ctx.gpr[3] + static_cast<std::uint32_t>(-28464)));
    goto L_09D2A738;
L_09D2A738:
    ctx.gpr[3] = (aot_mem.aot_load32(ctx.gpr[5] + static_cast<std::uint32_t>(20524)));
    goto L_09D2A73C;
L_09D2A73C:
    if (ctx.gpr[3] != 0u) {
    aot_mem.aot_store32(ctx.gpr[4] + static_cast<std::uint32_t>(4), ctx.gpr[3]);
        goto L_09D2A74C;
    }
    goto L_09D2A744;
L_09D2A740:
    aot_mem.aot_store32(ctx.gpr[4] + static_cast<std::uint32_t>(4), ctx.gpr[3]);
    goto L_09D2A744;
L_09D2A744:
    { const bool branch_taken = 0u == 0u;
    aot_mem.aot_store32(ctx.gpr[5] + static_cast<std::uint32_t>(20524), ctx.gpr[4]);
      if (branch_taken) {
          goto L_09D2A75C;
      }
      goto L_09D2A74C;
    }
L_09D2A748:
    aot_mem.aot_store32(ctx.gpr[5] + static_cast<std::uint32_t>(20524), ctx.gpr[4]);
    goto L_09D2A74C;
L_09D2A74C:
    aot_mem.aot_store32(ctx.gpr[4] + static_cast<std::uint32_t>(8), 0u);
    goto L_09D2A750;
L_09D2A750:
    ctx.gpr[3] = (aot_mem.aot_load32(ctx.gpr[5] + static_cast<std::uint32_t>(20524)));
    goto L_09D2A754;
L_09D2A754:
    aot_mem.aot_store32(ctx.gpr[3] + static_cast<std::uint32_t>(8), ctx.gpr[4]);
    goto L_09D2A758;
L_09D2A758:
    aot_mem.aot_store32(ctx.gpr[5] + static_cast<std::uint32_t>(20524), ctx.gpr[4]);
    goto L_09D2A75C;
L_09D2A75C:
    aot_mem.aot_store32(ctx.gpr[5] + static_cast<std::uint32_t>(20528), ctx.gpr[4]);
    goto L_09D2A760;
L_09D2A760:
    ctx.gpr[3] = (aot_mem.aot_load32(ctx.gpr[5] + static_cast<std::uint32_t>(20532)));
    goto L_09D2A764;
L_09D2A764:
    ctx.gpr[3] = (ctx.gpr[3] + static_cast<std::uint32_t>(1));
    goto L_09D2A768;
L_09D2A768:
    jump_target = ctx.gpr[31];
    aot_mem.aot_store32(ctx.gpr[5] + static_cast<std::uint32_t>(20532), ctx.gpr[3]);
    local_pc = jump_target;
    if (++local_transfers < 2048u) { entry_id = 0u; goto LOCAL_DISPATCH; }
    ctx.pc = jump_target;
    return;
L_09D2A76C:
    aot_mem.aot_store32(ctx.gpr[5] + static_cast<std::uint32_t>(20532), ctx.gpr[3]);
    goto L_09D2A770;
L_09D2A770:
    jump_target = ctx.gpr[31];
    // nop
    local_pc = jump_target;
    if (++local_transfers < 2048u) { entry_id = 0u; goto LOCAL_DISPATCH; }
    ctx.pc = jump_target;
    return;
L_09D2A774:
    // nop
    goto L_09D2A778;
L_09D2A778:
    ctx.gpr[29] = (ctx.gpr[29] + static_cast<std::uint32_t>(-48));
    goto L_09D2A77C;
L_09D2A77C:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(12), ctx.gpr[31]);
    goto L_09D2A780;
L_09D2A780:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(8), ctx.gpr[17]);
    goto L_09D2A784;
L_09D2A784:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(4), ctx.gpr[16]);
    goto L_09D2A788;
L_09D2A788:
    ctx.gpr[17] = (ctx.gpr[4] + 0u);
    goto L_09D2A78C;
L_09D2A78C:
    ctx.gpr[2] = (2497u << 16u);
    goto L_09D2A790;
L_09D2A790:
    ctx.gpr[4] = (aot_mem.aot_load8(ctx.gpr[4] + static_cast<std::uint32_t>(21)));
    goto L_09D2A794;
L_09D2A794:
    { const bool branch_taken = static_cast<std::int32_t>(ctx.gpr[4]) < 0;
    ctx.gpr[3] = (aot_mem.aot_load32(ctx.gpr[2] + static_cast<std::uint32_t>(8880)));
      if (branch_taken) {
          goto L_09D2A7A8;
      }
      goto L_09D2A79C;
    }
L_09D2A798:
    ctx.gpr[3] = (aot_mem.aot_load32(ctx.gpr[2] + static_cast<std::uint32_t>(8880)));
    goto L_09D2A79C;
L_09D2A79C:
    ctx.gpr[2] = (static_cast<std::int32_t>(ctx.gpr[4]) < 20 ? 1u : 0u);
    goto L_09D2A7A0;
L_09D2A7A0:
    if (ctx.gpr[2] != 0u) {
    ctx.gpr[2] = (ctx.gpr[4] << 2u);
        goto L_09D2A7B0;
    }
    goto L_09D2A7A8;
L_09D2A7A4:
    ctx.gpr[2] = (ctx.gpr[4] << 2u);
    goto L_09D2A7A8;
L_09D2A7A8:
    { const bool branch_taken = 0u == 0u;
    ctx.gpr[16] = (0u + 0u);
      if (branch_taken) {
          goto L_09D2A7B8;
      }
      goto L_09D2A7B0;
    }
L_09D2A7AC:
    ctx.gpr[16] = (0u + 0u);
    goto L_09D2A7B0;
L_09D2A7B0:
    ctx.gpr[2] = (ctx.gpr[2] + ctx.gpr[3]);
    goto L_09D2A7B4;
L_09D2A7B4:
    ctx.gpr[16] = (aot_mem.aot_load32(ctx.gpr[2] + static_cast<std::uint32_t>(4640)));
    goto L_09D2A7B8;
L_09D2A7B8:
    ctx.gpr[4] = (ctx.gpr[17] + 0u);
    goto L_09D2A7BC;
L_09D2A7BC:
    ctx.gpr[31] = (0x09D2A7C4u);
    ctx.gpr[5] = (0u + static_cast<std::uint32_t>(32));
    ctx.pc = 0x09B67738u;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D2A7C4u) goto L_09D2A7C4;
    return;
L_09D2A7C0:
    ctx.gpr[5] = (0u + static_cast<std::uint32_t>(32));
    goto L_09D2A7C4;
L_09D2A7C4:
    ctx.gpr[6] = (aot_mem.aot_load8(ctx.gpr[17] + static_cast<std::uint32_t>(13)));
    goto L_09D2A7C8;
L_09D2A7C8:
    ctx.gpr[7] = (2515u << 16u);
    goto L_09D2A7CC;
L_09D2A7CC:
    ctx.gpr[4] = (ctx.gpr[17] + 0u);
    goto L_09D2A7D0;
L_09D2A7D0:
    ctx.gpr[5] = (ctx.gpr[16] + 0u);
    goto L_09D2A7D4;
L_09D2A7D4:
    ctx.gpr[31] = (0x09D2A7DCu);
    ctx.gpr[7] = (ctx.gpr[7] + static_cast<std::uint32_t>(-6760));
    ctx.pc = 0x09B675C0u;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D2A7DCu) goto L_09D2A7DC;
    return;
L_09D2A7D8:
    ctx.gpr[7] = (ctx.gpr[7] + static_cast<std::uint32_t>(-6760));
    goto L_09D2A7DC;
L_09D2A7DC:
    ctx.gpr[3] = (aot_mem.aot_load8(ctx.gpr[17] + static_cast<std::uint32_t>(38)));
    goto L_09D2A7E0;
L_09D2A7E0:
    ctx.gpr[2] = (2515u << 16u);
    goto L_09D2A7E4;
L_09D2A7E4:
    ctx.gpr[2] = (ctx.gpr[2] + static_cast<std::uint32_t>(-7456));
    goto L_09D2A7E8;
L_09D2A7E8:
    ctx.gpr[3] = (ctx.gpr[3] << 2u);
    goto L_09D2A7EC;
L_09D2A7EC:
    ctx.gpr[2] = (ctx.gpr[2] + ctx.gpr[3]);
    goto L_09D2A7F0;
L_09D2A7F0:
    ctx.gpr[2] = (aot_mem.aot_load32(ctx.gpr[2] + static_cast<std::uint32_t>(0)));
    goto L_09D2A7F4;
L_09D2A7F4:
    aot_mem.aot_store32(ctx.gpr[17] + static_cast<std::uint32_t>(52), ctx.gpr[2]);
    goto L_09D2A7F8;
L_09D2A7F8:
    aot_mem.aot_store32(ctx.gpr[17] + static_cast<std::uint32_t>(56), 0u);
    goto L_09D2A7FC;
L_09D2A7FC:
    ctx.gpr[2] = (aot_mem.aot_load16(ctx.gpr[16] + static_cast<std::uint32_t>(666)));
    goto L_09D2A800;
L_09D2A800:
    aot_mem.aot_store16(ctx.gpr[17] + static_cast<std::uint32_t>(24), static_cast<std::uint16_t>(ctx.gpr[2]));
    goto L_09D2A804;
L_09D2A804:
    ctx.gpr[2] = (aot_mem.aot_load8(ctx.gpr[17] + static_cast<std::uint32_t>(28)));
    goto L_09D2A808;
L_09D2A808:
    if (ctx.gpr[2] == 0u) {
    ctx.gpr[4] = (ctx.gpr[17] + 0u);
        goto L_09D2A8C8;
    }
    goto L_09D2A810;
L_09D2A80C:
    ctx.gpr[4] = (ctx.gpr[17] + 0u);
    goto L_09D2A810;
L_09D2A810:
    ctx.gpr[4] = (static_cast<std::uint32_t>(static_cast<std::int32_t>(static_cast<std::int16_t>(aot_mem.aot_load16(ctx.gpr[16] + static_cast<std::uint32_t>(820))))));
    goto L_09D2A814;
L_09D2A814:
    ctx.gpr[2] = (0u + static_cast<std::uint32_t>(100));
    goto L_09D2A818;
L_09D2A818:
    ctx.gpr[3] = (ctx.gpr[29] + static_cast<std::uint32_t>(40));
    goto L_09D2A81C;
L_09D2A81C:
    { const std::int32_t dividend = static_cast<std::int32_t>(ctx.gpr[4]); const std::int32_t divisor = static_cast<std::int32_t>(ctx.gpr[2]); if (divisor == 0) { ctx.lo = dividend >= 0 ? 0xFFFFFFFFu : 1u; ctx.hi = static_cast<std::uint32_t>(dividend); } else if (dividend == static_cast<std::int32_t>(0x80000000u) && divisor == -1) { ctx.lo = 0x80000000u; ctx.hi = 0u; } else { ctx.lo = static_cast<std::uint32_t>(dividend / divisor); ctx.hi = static_cast<std::uint32_t>(dividend % divisor); } }
    goto L_09D2A820;
L_09D2A820:
    ctx.gpr[4] = (ctx.hi);
    goto L_09D2A824;
L_09D2A824:
    ctx.fpr[0] = std::bit_cast<float>(ctx.gpr[4]);
    goto L_09D2A828;
L_09D2A828:
    ctx.fpr[0] = static_cast<float>(static_cast<std::int32_t>(std::bit_cast<std::uint32_t>(ctx.fpr[0])));
    goto L_09D2A82C;
L_09D2A82C:
    ctx.gpr[2] = (ctx.gpr[29] + static_cast<std::uint32_t>(44));
    goto L_09D2A830;
L_09D2A830:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(44), std::bit_cast<std::uint32_t>(ctx.fpr[0]));
    goto L_09D2A834;
L_09D2A834:
    ctx.set_vfpu_scalar_bits_ct<0u>(aot_mem.aot_load32(ctx.gpr[2] + static_cast<std::uint32_t>(0)));
    goto L_09D2A838;
L_09D2A838:
    { float vfpu_s[4]{}, vfpu_d[4]{};
      ctx.read_vfpu_vector_with_source_prefix_ct<0u, 1u, 0u>(vfpu_s);
      for (std::uint32_t i = 0; i < 1u; ++i) vfpu_d[i] = std::fabs(vfpu_s[i]);
      ctx.write_vfpu_vector_with_destination_prefix_ct<1u, 1u>(vfpu_d); }
    goto L_09D2A83C;
L_09D2A83C:
    aot_mem.aot_store32(ctx.gpr[3] + static_cast<std::uint32_t>(0), ctx.vfpu_scalar_bits_ct<1u>());
    goto L_09D2A840;
L_09D2A840:
    ctx.fpr[0] = std::bit_cast<float>(aot_mem.aot_load32(ctx.gpr[29] + static_cast<std::uint32_t>(40)));
    goto L_09D2A844;
L_09D2A844:
    ctx.gpr[2] = (aot_mem.aot_load16(ctx.gpr[16] + static_cast<std::uint32_t>(812)));
    goto L_09D2A848;
L_09D2A848:
    ctx.fpr[0] = std::bit_cast<float>(ctx.fpu_float_to_word_ct<1u>(ctx.fpr[0]));
    goto L_09D2A84C;
L_09D2A84C:
    { const bool branch_taken = static_cast<std::int32_t>(ctx.gpr[2]) < 0;
    ctx.gpr[4] = (std::bit_cast<std::uint32_t>(ctx.fpr[0]));
      if (branch_taken) {
          goto L_09D2A860;
      }
      goto L_09D2A854;
    }
L_09D2A850:
    ctx.gpr[4] = (std::bit_cast<std::uint32_t>(ctx.fpr[0]));
    goto L_09D2A854;
L_09D2A854:
    ctx.fpr[0] = std::bit_cast<float>(ctx.gpr[2]);
    goto L_09D2A858;
L_09D2A858:
    { const bool branch_taken = 0u == 0u;
    ctx.fpr[0] = static_cast<float>(static_cast<std::int32_t>(std::bit_cast<std::uint32_t>(ctx.fpr[0])));
      if (branch_taken) {
          goto L_09D2A878;
      }
      goto L_09D2A860;
    }
L_09D2A85C:
    ctx.fpr[0] = static_cast<float>(static_cast<std::int32_t>(std::bit_cast<std::uint32_t>(ctx.fpr[0])));
    goto L_09D2A860;
L_09D2A860:
    ctx.gpr[3] = (ctx.gpr[2] >> 1u);
    goto L_09D2A864;
L_09D2A864:
    ctx.gpr[2] = (ctx.gpr[2] & 1u);
    goto L_09D2A868;
L_09D2A868:
    ctx.gpr[3] = (ctx.gpr[3] | ctx.gpr[2]);
    goto L_09D2A86C;
L_09D2A86C:
    ctx.fpr[0] = std::bit_cast<float>(ctx.gpr[3]);
    goto L_09D2A870;
L_09D2A870:
    ctx.fpr[0] = static_cast<float>(static_cast<std::int32_t>(std::bit_cast<std::uint32_t>(ctx.fpr[0])));
    goto L_09D2A874;
L_09D2A874:
    ctx.fpr[0] = ctx.fpr[0] + ctx.fpr[0];
    goto L_09D2A878;
L_09D2A878:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(36), std::bit_cast<std::uint32_t>(ctx.fpr[0]));
    goto L_09D2A87C;
L_09D2A87C:
    ctx.gpr[2] = (ctx.gpr[29] + static_cast<std::uint32_t>(36));
    goto L_09D2A880;
L_09D2A880:
    ctx.gpr[3] = (ctx.gpr[29] + static_cast<std::uint32_t>(32));
    goto L_09D2A884;
L_09D2A884:
    ctx.set_vfpu_scalar_bits_ct<0u>(aot_mem.aot_load32(ctx.gpr[2] + static_cast<std::uint32_t>(0)));
    goto L_09D2A888;
L_09D2A888:
    { float vfpu_s[4]{}, vfpu_d[4]{};
      ctx.read_vfpu_vector_with_source_prefix_ct<0u, 1u, 0u>(vfpu_s);
      for (std::uint32_t i = 0; i < 1u; ++i) vfpu_d[i] = std::fabs(vfpu_s[i]);
      ctx.write_vfpu_vector_with_destination_prefix_ct<1u, 1u>(vfpu_d); }
    goto L_09D2A88C;
L_09D2A88C:
    aot_mem.aot_store32(ctx.gpr[3] + static_cast<std::uint32_t>(0), ctx.vfpu_scalar_bits_ct<1u>());
    goto L_09D2A890;
L_09D2A890:
    ctx.fpr[0] = std::bit_cast<float>(aot_mem.aot_load32(ctx.gpr[29] + static_cast<std::uint32_t>(32)));
    goto L_09D2A894;
L_09D2A894:
    ctx.gpr[2] = (aot_mem.aot_load8(ctx.gpr[17] + static_cast<std::uint32_t>(28)));
    goto L_09D2A898;
L_09D2A898:
    ctx.fpr[0] = std::bit_cast<float>(ctx.fpu_float_to_word_ct<1u>(ctx.fpr[0]));
    goto L_09D2A89C;
L_09D2A89C:
    ctx.gpr[3] = (std::bit_cast<std::uint32_t>(ctx.fpr[0]));
    goto L_09D2A8A0;
L_09D2A8A0:
    ctx.gpr[3] = (ctx.gpr[4] - ctx.gpr[3]);
    goto L_09D2A8A4;
L_09D2A8A4:
    ctx.gpr[3] = (ctx.gpr[3] + static_cast<std::uint32_t>(-1));
    goto L_09D2A8A8;
L_09D2A8A8:
    ctx.gpr[3] = (ctx.gpr[3] & 255u);
    goto L_09D2A8AC;
L_09D2A8AC:
    ctx.gpr[2] = (ctx.gpr[2] + ctx.gpr[3]);
    goto L_09D2A8B0;
L_09D2A8B0:
    aot_mem.aot_store8(ctx.gpr[17] + static_cast<std::uint32_t>(28), static_cast<std::uint8_t>(ctx.gpr[2]));
    goto L_09D2A8B4;
L_09D2A8B4:
    ctx.gpr[2] = (static_cast<std::uint32_t>(static_cast<std::int32_t>(static_cast<std::int8_t>(ctx.gpr[2]))));
    goto L_09D2A8B8;
L_09D2A8B8:
    { const bool branch_taken = static_cast<std::int32_t>(ctx.gpr[2]) >= 0;
    // nop
      if (branch_taken) {
          goto L_09D2A8C4;
      }
      goto L_09D2A8C0;
    }
L_09D2A8BC:
    // nop
    goto L_09D2A8C0;
L_09D2A8C0:
    aot_mem.aot_store8(ctx.gpr[17] + static_cast<std::uint32_t>(28), static_cast<std::uint8_t>(0u));
    goto L_09D2A8C4;
L_09D2A8C4:
    ctx.gpr[4] = (ctx.gpr[17] + 0u);
    goto L_09D2A8C8;
L_09D2A8C8:
    ctx.gpr[31] = (0x09D2A8D0u);
    // nop
    ctx.pc = 0x09B672B8u;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D2A8D0u) goto L_09D2A8D0;
    return;
L_09D2A8CC:
    // nop
    goto L_09D2A8D0;
L_09D2A8D0:
    ctx.gpr[25] = (aot_mem.aot_load32(ctx.gpr[17] + static_cast<std::uint32_t>(0)));
    goto L_09D2A8D4;
L_09D2A8D4:
    ctx.gpr[25] = (aot_mem.aot_load32(ctx.gpr[25] + static_cast<std::uint32_t>(36)));
    goto L_09D2A8D8;
L_09D2A8D8:
    jump_target = ctx.gpr[25];
    ctx.gpr[31] = (0x09D2A8E0u);
    ctx.gpr[4] = (ctx.gpr[17] + 0u);
    ctx.pc = jump_target;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D2A8E0u) goto L_09D2A8E0;
    return;
L_09D2A8DC:
    ctx.gpr[4] = (ctx.gpr[17] + 0u);
    goto L_09D2A8E0;
L_09D2A8E0:
    ctx.gpr[2] = (2515u << 16u);
    goto L_09D2A8E4;
L_09D2A8E4:
    ctx.fpr[1] = std::bit_cast<float>(aot_mem.aot_load32(ctx.gpr[2] + static_cast<std::uint32_t>(-6752)));
    goto L_09D2A8E8;
L_09D2A8E8:
    ctx.gpr[4] = (ctx.gpr[29] + static_cast<std::uint32_t>(20));
    goto L_09D2A8EC;
L_09D2A8EC:
    ctx.gpr[2] = (2515u << 16u);
    goto L_09D2A8F0;
L_09D2A8F0:
    ctx.fpr[0] = std::bit_cast<float>(aot_mem.aot_load32(ctx.gpr[2] + static_cast<std::uint32_t>(-6748)));
    goto L_09D2A8F4;
L_09D2A8F4:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(20), std::bit_cast<std::uint32_t>(ctx.fpr[1]));
    goto L_09D2A8F8;
L_09D2A8F8:
    ctx.gpr[2] = (2515u << 16u);
    goto L_09D2A8FC;
L_09D2A8FC:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(24), std::bit_cast<std::uint32_t>(ctx.fpr[0]));
    goto L_09D2A900;
L_09D2A900:
    ctx.fpr[0] = std::bit_cast<float>(aot_mem.aot_load32(ctx.gpr[2] + static_cast<std::uint32_t>(-6744)));
    goto L_09D2A904;
L_09D2A904:
    ctx.gpr[31] = (0x09D2A90Cu);
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(28), std::bit_cast<std::uint32_t>(ctx.fpr[0]));
    ctx.pc = 0x0890C4F8u;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D2A90Cu) goto L_09D2A90C;
    return;
L_09D2A908:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(28), std::bit_cast<std::uint32_t>(ctx.fpr[0]));
    goto L_09D2A90C;
L_09D2A90C:
    if (ctx.gpr[2] == 0u) {
    ctx.gpr[31] = (aot_mem.aot_load32(ctx.gpr[29] + static_cast<std::uint32_t>(12)));
        goto L_09D2A930;
    }
    goto L_09D2A914;
L_09D2A910:
    ctx.gpr[31] = (aot_mem.aot_load32(ctx.gpr[29] + static_cast<std::uint32_t>(12)));
    goto L_09D2A914;
L_09D2A914:
    ctx.fpr[2] = std::bit_cast<float>(aot_mem.aot_load32(ctx.gpr[29] + static_cast<std::uint32_t>(20)));
    goto L_09D2A918;
L_09D2A918:
    ctx.fpr[1] = std::bit_cast<float>(aot_mem.aot_load32(ctx.gpr[29] + static_cast<std::uint32_t>(24)));
    goto L_09D2A91C;
L_09D2A91C:
    ctx.fpr[0] = std::bit_cast<float>(aot_mem.aot_load32(ctx.gpr[29] + static_cast<std::uint32_t>(28)));
    goto L_09D2A920;
L_09D2A920:
    aot_mem.aot_store32(ctx.gpr[17] + static_cast<std::uint32_t>(120), std::bit_cast<std::uint32_t>(ctx.fpr[2]));
    goto L_09D2A924;
L_09D2A924:
    aot_mem.aot_store32(ctx.gpr[17] + static_cast<std::uint32_t>(124), std::bit_cast<std::uint32_t>(ctx.fpr[1]));
    goto L_09D2A928;
L_09D2A928:
    aot_mem.aot_store32(ctx.gpr[17] + static_cast<std::uint32_t>(128), std::bit_cast<std::uint32_t>(ctx.fpr[0]));
    goto L_09D2A92C;
L_09D2A92C:
    ctx.gpr[31] = (aot_mem.aot_load32(ctx.gpr[29] + static_cast<std::uint32_t>(12)));
    goto L_09D2A930;
L_09D2A930:
    ctx.gpr[17] = (aot_mem.aot_load32(ctx.gpr[29] + static_cast<std::uint32_t>(8)));
    goto L_09D2A934;
L_09D2A934:
    ctx.gpr[16] = (aot_mem.aot_load32(ctx.gpr[29] + static_cast<std::uint32_t>(4)));
    goto L_09D2A938;
L_09D2A938:
    jump_target = ctx.gpr[31];
    ctx.gpr[29] = (ctx.gpr[29] + static_cast<std::uint32_t>(48));
    local_pc = jump_target;
    if (++local_transfers < 2048u) { entry_id = 0u; goto LOCAL_DISPATCH; }
    ctx.pc = jump_target;
    return;
L_09D2A93C:
    ctx.gpr[29] = (ctx.gpr[29] + static_cast<std::uint32_t>(48));
    goto L_09D2A940;
L_09D2A940:
    ctx.gpr[29] = (ctx.gpr[29] + static_cast<std::uint32_t>(-32));
    goto L_09D2A944;
L_09D2A944:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(12), ctx.gpr[31]);
    goto L_09D2A948;
L_09D2A948:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(8), ctx.gpr[16]);
    goto L_09D2A94C;
L_09D2A94C:
    ctx.gpr[5] = (aot_mem.aot_load8(ctx.gpr[4] + static_cast<std::uint32_t>(21)));
    goto L_09D2A950;
L_09D2A950:
    ctx.gpr[16] = (ctx.gpr[4] + 0u);
    goto L_09D2A954;
L_09D2A954:
    ctx.gpr[3] = (2497u << 16u);
    goto L_09D2A958;
L_09D2A958:
    { const bool branch_taken = static_cast<std::int32_t>(ctx.gpr[5]) < 0;
    ctx.gpr[4] = (aot_mem.aot_load32(ctx.gpr[3] + static_cast<std::uint32_t>(8880)));
      if (branch_taken) {
          goto L_09D2A96C;
      }
      goto L_09D2A960;
    }
L_09D2A95C:
    ctx.gpr[4] = (aot_mem.aot_load32(ctx.gpr[3] + static_cast<std::uint32_t>(8880)));
    goto L_09D2A960;
L_09D2A960:
    ctx.gpr[3] = (static_cast<std::int32_t>(ctx.gpr[5]) < 20 ? 1u : 0u);
    goto L_09D2A964;
L_09D2A964:
    if (ctx.gpr[3] != 0u) {
    ctx.gpr[3] = (ctx.gpr[5] << 2u);
        goto L_09D2A974;
    }
    goto L_09D2A96C;
L_09D2A968:
    ctx.gpr[3] = (ctx.gpr[5] << 2u);
    goto L_09D2A96C;
L_09D2A96C:
    { const bool branch_taken = 0u == 0u;
    ctx.gpr[5] = (0u + 0u);
      if (branch_taken) {
          goto L_09D2A97C;
      }
      goto L_09D2A974;
    }
L_09D2A970:
    ctx.gpr[5] = (0u + 0u);
    goto L_09D2A974;
L_09D2A974:
    ctx.gpr[3] = (ctx.gpr[3] + ctx.gpr[4]);
    goto L_09D2A978;
L_09D2A978:
    ctx.gpr[5] = (aot_mem.aot_load32(ctx.gpr[3] + static_cast<std::uint32_t>(4640)));
    goto L_09D2A97C;
L_09D2A97C:
    ctx.gpr[4] = (aot_mem.aot_load8(ctx.gpr[16] + static_cast<std::uint32_t>(13)));
    goto L_09D2A980;
L_09D2A980:
    ctx.gpr[3] = (0u + static_cast<std::uint32_t>(24));
    goto L_09D2A984;
L_09D2A984:
    if (ctx.gpr[4] == ctx.gpr[3]) {
    ctx.gpr[3] = (0u + static_cast<std::uint32_t>(99));
        goto L_09D2A9BC;
    }
    goto L_09D2A98C;
L_09D2A988:
    ctx.gpr[3] = (0u + static_cast<std::uint32_t>(99));
    goto L_09D2A98C;
L_09D2A98C:
    ctx.gpr[3] = (0u + static_cast<std::uint32_t>(23));
    goto L_09D2A990;
L_09D2A990:
    { const bool branch_taken = ctx.gpr[4] == ctx.gpr[3];
    // nop
      if (branch_taken) {
          goto L_09D2A9B8;
      }
      goto L_09D2A998;
    }
L_09D2A994:
    // nop
    goto L_09D2A998;
L_09D2A998:
    ctx.gpr[3] = (0u + static_cast<std::uint32_t>(19));
    goto L_09D2A99C;
L_09D2A99C:
    { const bool branch_taken = ctx.gpr[4] == ctx.gpr[3];
    // nop
      if (branch_taken) {
          goto L_09D2A9B8;
      }
      goto L_09D2A9A4;
    }
L_09D2A9A0:
    // nop
    goto L_09D2A9A4;
L_09D2A9A4:
    ctx.gpr[3] = (0u + static_cast<std::uint32_t>(14));
    goto L_09D2A9A8;
L_09D2A9A8:
    { const bool branch_taken = ctx.gpr[4] == ctx.gpr[3];
    // nop
      if (branch_taken) {
          goto L_09D2A9B8;
      }
      goto L_09D2A9B0;
    }
L_09D2A9AC:
    // nop
    goto L_09D2A9B0;
L_09D2A9B0:
    { const bool branch_taken = 0u == 0u;
    ctx.gpr[4] = (aot_mem.aot_load16(ctx.gpr[5] + static_cast<std::uint32_t>(804)));
      if (branch_taken) {
          goto L_09D2A9C4;
      }
      goto L_09D2A9B8;
    }
L_09D2A9B4:
    ctx.gpr[4] = (aot_mem.aot_load16(ctx.gpr[5] + static_cast<std::uint32_t>(804)));
    goto L_09D2A9B8;
L_09D2A9B8:
    ctx.gpr[3] = (0u + static_cast<std::uint32_t>(99));
    goto L_09D2A9BC;
L_09D2A9BC:
    aot_mem.aot_store8(ctx.gpr[16] + static_cast<std::uint32_t>(29), static_cast<std::uint8_t>(ctx.gpr[3]));
    goto L_09D2A9C0;
L_09D2A9C0:
    ctx.gpr[4] = (aot_mem.aot_load16(ctx.gpr[5] + static_cast<std::uint32_t>(804)));
    goto L_09D2A9C4;
L_09D2A9C4:
    ctx.gpr[3] = (static_cast<std::uint32_t>(static_cast<std::int32_t>(static_cast<std::int16_t>(aot_mem.aot_load16(ctx.gpr[16] + static_cast<std::uint32_t>(194))))));
    goto L_09D2A9C8;
L_09D2A9C8:
    if (ctx.gpr[4] != ctx.gpr[3]) {
    aot_mem.aot_store8(ctx.gpr[16] + static_cast<std::uint32_t>(20), static_cast<std::uint8_t>(0u));
        goto L_09D2A9E0;
    }
    goto L_09D2A9D0;
L_09D2A9CC:
    aot_mem.aot_store8(ctx.gpr[16] + static_cast<std::uint32_t>(20), static_cast<std::uint8_t>(0u));
    goto L_09D2A9D0;
L_09D2A9D0:
    ctx.gpr[3] = (aot_mem.aot_load8(ctx.gpr[16] + static_cast<std::uint32_t>(20)));
    goto L_09D2A9D4;
L_09D2A9D4:
    if (ctx.gpr[3] != 0u) {
    ctx.gpr[31] = (aot_mem.aot_load32(ctx.gpr[29] + static_cast<std::uint32_t>(12)));
        goto L_09D2AA30;
    }
    goto L_09D2A9DC;
L_09D2A9D8:
    ctx.gpr[31] = (aot_mem.aot_load32(ctx.gpr[29] + static_cast<std::uint32_t>(12)));
    goto L_09D2A9DC;
L_09D2A9DC:
    aot_mem.aot_store8(ctx.gpr[16] + static_cast<std::uint32_t>(20), static_cast<std::uint8_t>(0u));
    goto L_09D2A9E0;
L_09D2A9E0:
    ctx.gpr[2] = (2515u << 16u);
    goto L_09D2A9E4;
L_09D2A9E4:
    ctx.fpr[1] = std::bit_cast<float>(aot_mem.aot_load32(ctx.gpr[2] + static_cast<std::uint32_t>(-6736)));
    goto L_09D2A9E8;
L_09D2A9E8:
    ctx.gpr[4] = (ctx.gpr[29] + static_cast<std::uint32_t>(20));
    goto L_09D2A9EC;
L_09D2A9EC:
    ctx.gpr[2] = (2515u << 16u);
    goto L_09D2A9F0;
L_09D2A9F0:
    ctx.fpr[0] = std::bit_cast<float>(aot_mem.aot_load32(ctx.gpr[2] + static_cast<std::uint32_t>(-6732)));
    goto L_09D2A9F4;
L_09D2A9F4:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(20), std::bit_cast<std::uint32_t>(ctx.fpr[1]));
    goto L_09D2A9F8;
L_09D2A9F8:
    ctx.gpr[2] = (2515u << 16u);
    goto L_09D2A9FC;
L_09D2A9FC:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(24), std::bit_cast<std::uint32_t>(ctx.fpr[0]));
    goto L_09D2AA00;
L_09D2AA00:
    ctx.fpr[0] = std::bit_cast<float>(aot_mem.aot_load32(ctx.gpr[2] + static_cast<std::uint32_t>(-6728)));
    goto L_09D2AA04;
L_09D2AA04:
    ctx.gpr[31] = (0x09D2AA0Cu);
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(28), std::bit_cast<std::uint32_t>(ctx.fpr[0]));
    ctx.pc = 0x0890C4F8u;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D2AA0Cu) goto L_09D2AA0C;
    return;
L_09D2AA08:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(28), std::bit_cast<std::uint32_t>(ctx.fpr[0]));
    goto L_09D2AA0C;
L_09D2AA0C:
    { const bool branch_taken = ctx.gpr[2] == 0u;
    // nop
      if (branch_taken) {
          goto L_09D2AA2C;
      }
      goto L_09D2AA14;
    }
L_09D2AA10:
    // nop
    goto L_09D2AA14;
L_09D2AA14:
    ctx.fpr[2] = std::bit_cast<float>(aot_mem.aot_load32(ctx.gpr[29] + static_cast<std::uint32_t>(20)));
    goto L_09D2AA18;
L_09D2AA18:
    ctx.fpr[1] = std::bit_cast<float>(aot_mem.aot_load32(ctx.gpr[29] + static_cast<std::uint32_t>(24)));
    goto L_09D2AA1C;
L_09D2AA1C:
    ctx.fpr[0] = std::bit_cast<float>(aot_mem.aot_load32(ctx.gpr[29] + static_cast<std::uint32_t>(28)));
    goto L_09D2AA20;
L_09D2AA20:
    aot_mem.aot_store32(ctx.gpr[16] + static_cast<std::uint32_t>(120), std::bit_cast<std::uint32_t>(ctx.fpr[2]));
    goto L_09D2AA24;
L_09D2AA24:
    aot_mem.aot_store32(ctx.gpr[16] + static_cast<std::uint32_t>(124), std::bit_cast<std::uint32_t>(ctx.fpr[1]));
    goto L_09D2AA28;
L_09D2AA28:
    aot_mem.aot_store32(ctx.gpr[16] + static_cast<std::uint32_t>(128), std::bit_cast<std::uint32_t>(ctx.fpr[0]));
    goto L_09D2AA2C;
L_09D2AA2C:
    ctx.gpr[31] = (aot_mem.aot_load32(ctx.gpr[29] + static_cast<std::uint32_t>(12)));
    goto L_09D2AA30;
L_09D2AA30:
    ctx.gpr[16] = (aot_mem.aot_load32(ctx.gpr[29] + static_cast<std::uint32_t>(8)));
    goto L_09D2AA34;
L_09D2AA34:
    jump_target = ctx.gpr[31];
    ctx.gpr[29] = (ctx.gpr[29] + static_cast<std::uint32_t>(32));
    local_pc = jump_target;
    if (++local_transfers < 2048u) { entry_id = 0u; goto LOCAL_DISPATCH; }
    ctx.pc = jump_target;
    return;
L_09D2AA38:
    ctx.gpr[29] = (ctx.gpr[29] + static_cast<std::uint32_t>(32));
    goto L_09D2AA3C;
L_09D2AA3C:
    // nop
    goto L_09D2AA40;
L_09D2AA40:
    ctx.gpr[29] = (ctx.gpr[29] + static_cast<std::uint32_t>(-32));
    goto L_09D2AA44;
L_09D2AA44:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(12), ctx.gpr[31]);
    goto L_09D2AA48;
L_09D2AA48:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(8), ctx.gpr[16]);
    goto L_09D2AA4C;
L_09D2AA4C:
    aot_mem.aot_store8(ctx.gpr[4] + static_cast<std::uint32_t>(20), static_cast<std::uint8_t>(0u));
    goto L_09D2AA50;
L_09D2AA50:
    ctx.gpr[2] = (2515u << 16u);
    goto L_09D2AA54;
L_09D2AA54:
    ctx.fpr[1] = std::bit_cast<float>(aot_mem.aot_load32(ctx.gpr[2] + static_cast<std::uint32_t>(-6720)));
    goto L_09D2AA58;
L_09D2AA58:
    ctx.gpr[16] = (ctx.gpr[4] + 0u);
    goto L_09D2AA5C;
L_09D2AA5C:
    ctx.gpr[4] = (ctx.gpr[29] + static_cast<std::uint32_t>(20));
    goto L_09D2AA60;
L_09D2AA60:
    ctx.gpr[2] = (2515u << 16u);
    goto L_09D2AA64;
L_09D2AA64:
    ctx.fpr[0] = std::bit_cast<float>(aot_mem.aot_load32(ctx.gpr[2] + static_cast<std::uint32_t>(-6716)));
    goto L_09D2AA68;
L_09D2AA68:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(20), std::bit_cast<std::uint32_t>(ctx.fpr[1]));
    goto L_09D2AA6C;
L_09D2AA6C:
    ctx.gpr[2] = (2515u << 16u);
    goto L_09D2AA70;
L_09D2AA70:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(24), std::bit_cast<std::uint32_t>(ctx.fpr[0]));
    goto L_09D2AA74;
L_09D2AA74:
    ctx.fpr[0] = std::bit_cast<float>(aot_mem.aot_load32(ctx.gpr[2] + static_cast<std::uint32_t>(-6712)));
    goto L_09D2AA78;
L_09D2AA78:
    ctx.gpr[31] = (0x09D2AA80u);
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(28), std::bit_cast<std::uint32_t>(ctx.fpr[0]));
    ctx.pc = 0x0890C4F8u;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09D2AA80u) goto L_09D2AA80;
    return;
L_09D2AA7C:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(28), std::bit_cast<std::uint32_t>(ctx.fpr[0]));
    goto L_09D2AA80;
L_09D2AA80:
    if (ctx.gpr[2] == 0u) {
    ctx.gpr[31] = (aot_mem.aot_load32(ctx.gpr[29] + static_cast<std::uint32_t>(12)));
        goto L_09D2AAA4;
    }
    goto L_09D2AA88;
L_09D2AA84:
    ctx.gpr[31] = (aot_mem.aot_load32(ctx.gpr[29] + static_cast<std::uint32_t>(12)));
    goto L_09D2AA88;
L_09D2AA88:
    ctx.fpr[2] = std::bit_cast<float>(aot_mem.aot_load32(ctx.gpr[29] + static_cast<std::uint32_t>(20)));
    goto L_09D2AA8C;
L_09D2AA8C:
    ctx.fpr[1] = std::bit_cast<float>(aot_mem.aot_load32(ctx.gpr[29] + static_cast<std::uint32_t>(24)));
    goto L_09D2AA90;
L_09D2AA90:
    ctx.fpr[0] = std::bit_cast<float>(aot_mem.aot_load32(ctx.gpr[29] + static_cast<std::uint32_t>(28)));
    goto L_09D2AA94;
L_09D2AA94:
    aot_mem.aot_store32(ctx.gpr[16] + static_cast<std::uint32_t>(120), std::bit_cast<std::uint32_t>(ctx.fpr[2]));
    goto L_09D2AA98;
L_09D2AA98:
    aot_mem.aot_store32(ctx.gpr[16] + static_cast<std::uint32_t>(124), std::bit_cast<std::uint32_t>(ctx.fpr[1]));
    goto L_09D2AA9C;
L_09D2AA9C:
    aot_mem.aot_store32(ctx.gpr[16] + static_cast<std::uint32_t>(128), std::bit_cast<std::uint32_t>(ctx.fpr[0]));
    goto L_09D2AAA0;
L_09D2AAA0:
    ctx.gpr[31] = (aot_mem.aot_load32(ctx.gpr[29] + static_cast<std::uint32_t>(12)));
    goto L_09D2AAA4;
L_09D2AAA4:
    ctx.gpr[16] = (aot_mem.aot_load32(ctx.gpr[29] + static_cast<std::uint32_t>(8)));
    goto L_09D2AAA8;
L_09D2AAA8:
    jump_target = ctx.gpr[31];
    ctx.gpr[29] = (ctx.gpr[29] + static_cast<std::uint32_t>(32));
    local_pc = jump_target;
    if (++local_transfers < 2048u) { entry_id = 0u; goto LOCAL_DISPATCH; }
    ctx.pc = jump_target;
    return;
L_09D2AAAC:
    ctx.gpr[29] = (ctx.gpr[29] + static_cast<std::uint32_t>(32));
    goto L_09D2AAB0;
L_09D2AAB0:
    ctx.gpr[25] = (aot_mem.aot_load32(ctx.gpr[4] + static_cast<std::uint32_t>(0)));
    goto L_09D2AAB4;
L_09D2AAB4:
    ctx.gpr[25] = (aot_mem.aot_load32(ctx.gpr[25] + static_cast<std::uint32_t>(36)));
    goto L_09D2AAB8;
L_09D2AAB8:
    jump_target = ctx.gpr[25];
    // nop
    local_pc = jump_target;
    if (++local_transfers < 2048u) { entry_id = 0u; goto LOCAL_DISPATCH; }
    ctx.pc = jump_target;
    return;
L_09D2AABC:
    // nop
    goto L_09D2AAC0;
L_09D2AAC0:
    ctx.gpr[25] = (aot_mem.aot_load32(ctx.gpr[4] + static_cast<std::uint32_t>(0)));
    goto L_09D2AAC4;
L_09D2AAC4:
    ctx.gpr[25] = (aot_mem.aot_load32(ctx.gpr[25] + static_cast<std::uint32_t>(40)));
    goto L_09D2AAC8;
L_09D2AAC8:
    jump_target = ctx.gpr[25];
    // nop
    local_pc = jump_target;
    if (++local_transfers < 2048u) { entry_id = 0u; goto LOCAL_DISPATCH; }
    ctx.pc = jump_target;
    return;
L_09D2AACC:
    // nop
    goto L_09D2AAD0;
L_09D2AAD0:
    ctx.gpr[25] = (aot_mem.aot_load32(ctx.gpr[4] + static_cast<std::uint32_t>(0)));
    goto L_09D2AAD4;
L_09D2AAD4:
    ctx.gpr[25] = (aot_mem.aot_load32(ctx.gpr[25] + static_cast<std::uint32_t>(40)));
    goto L_09D2AAD8;
L_09D2AAD8:
    jump_target = ctx.gpr[25];
    // nop
    local_pc = jump_target;
    if (++local_transfers < 2048u) { entry_id = 0u; goto LOCAL_DISPATCH; }
    ctx.pc = jump_target;
    return;
L_09D2AADC:
    // nop
    goto L_09D2AAE0;
L_09D2AAE0:
    ctx.gpr[25] = (aot_mem.aot_load32(ctx.gpr[4] + static_cast<std::uint32_t>(0)));
    goto L_09D2AAE4;
L_09D2AAE4:
    ctx.gpr[25] = (aot_mem.aot_load32(ctx.gpr[25] + static_cast<std::uint32_t>(40)));
    goto L_09D2AAE8;
L_09D2AAE8:
    jump_target = ctx.gpr[25];
    // nop
    local_pc = jump_target;
    if (++local_transfers < 2048u) { entry_id = 0u; goto LOCAL_DISPATCH; }
    ctx.pc = jump_target;
    return;
L_09D2AAEC:
    // nop
    goto L_09D2AAF0;
L_09D2AAF0:
    // nop
    ctx.pc = 0x09B673F8u; (void)rt.invoke_chained_call(ctx, &aot_mem); return;
L_09D2AAF4:
    // nop
    goto L_09D2AAF8;
L_09D2AAF8:
    jump_target = ctx.gpr[31];
    // nop
    local_pc = jump_target;
    if (++local_transfers < 2048u) { entry_id = 0u; goto LOCAL_DISPATCH; }
    ctx.pc = jump_target;
    return;
L_09D2AAFC:
    // nop
    ctx.pc = 0x09D2AB00u; return;
}

void ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004(Runtime &rt, AllegrexContext &ctx) {
    auto aot_mem = rt.memory().aot_fast_view();
    ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004_entry(rt, ctx, 0u, aot_mem);
}

void register_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_4(Runtime &runtime) {
    runtime.register_function(0x09D2A2C0u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A2C4u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A2C8u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A2CCu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A2D0u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A2D4u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A2D8u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A2DCu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A2E0u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A2E4u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A2E8u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A2ECu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A2F0u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A2F4u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A2F8u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A2FCu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A300u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A304u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A308u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A30Cu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A310u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A314u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A318u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A31Cu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A320u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A324u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A328u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A32Cu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A330u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A334u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A338u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A33Cu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A340u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A344u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A348u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A34Cu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A350u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A354u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A358u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A35Cu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A360u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A364u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A368u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A36Cu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A370u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A374u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A378u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A37Cu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A380u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A384u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A388u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A38Cu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A390u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A394u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A398u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A39Cu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A3A0u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A3A4u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A3A8u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A3ACu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A3B0u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A3B4u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A3B8u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A3BCu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A3C0u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A3C4u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A3C8u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A3CCu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A3D0u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A3D4u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A3D8u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A3DCu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A3E0u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A3E4u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A3E8u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A3ECu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A3F0u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A3F4u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A3F8u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A3FCu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A400u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A404u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A408u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A40Cu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A410u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A414u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A418u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A41Cu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A420u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A424u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A428u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A42Cu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A430u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A434u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A438u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A43Cu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A440u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A444u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A448u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A44Cu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A450u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A454u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A458u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A45Cu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A460u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A464u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A468u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A46Cu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A470u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A474u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A478u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A47Cu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A480u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A484u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A488u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A48Cu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A490u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A494u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A498u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A49Cu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A4A0u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A4A4u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A4A8u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A4ACu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A4B0u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A4B4u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A4B8u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A4BCu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A4C0u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A4C4u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A4C8u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A4CCu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A4D0u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A4D4u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A4D8u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A4DCu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A4E0u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A4E4u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A4E8u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A4ECu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A4F0u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A4F4u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A4F8u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A4FCu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A500u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A504u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A508u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A50Cu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A510u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A514u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A518u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A51Cu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A520u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A524u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A528u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A52Cu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A530u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A534u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A538u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A53Cu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A540u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A544u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A548u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A54Cu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A550u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A554u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A558u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A55Cu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A560u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A564u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A568u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A56Cu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A570u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A574u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A578u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A57Cu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A580u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A584u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A588u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A58Cu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A590u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A594u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A598u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A59Cu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A5A0u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A5A4u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A5A8u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A5ACu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A5B0u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A5B4u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A5B8u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A5BCu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A5C0u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A5C4u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A5C8u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A5CCu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A5D0u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A5D4u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A5D8u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A5DCu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A5E0u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A5E4u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A5E8u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A5ECu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A5F0u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A5F4u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A5F8u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A5FCu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A600u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A604u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A608u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A60Cu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A610u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A614u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A618u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A61Cu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A620u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A624u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A628u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A62Cu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A630u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A634u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A638u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A63Cu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A640u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A644u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A648u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A64Cu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A650u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A654u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A658u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A65Cu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A660u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A664u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A668u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A66Cu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A670u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A674u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A678u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A67Cu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A680u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A684u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A688u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A68Cu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A690u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A694u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A698u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A69Cu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A6A0u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A6A4u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A6A8u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A6ACu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A6B0u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A6B4u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A6B8u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A6BCu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A6C0u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A6C4u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A6C8u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A6CCu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A6D0u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A6D4u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A6D8u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A6DCu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A6E0u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A6E4u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A6E8u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A6ECu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A6F0u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A6F4u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A6F8u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A6FCu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A700u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A704u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A708u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A70Cu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A710u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A714u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A718u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A71Cu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A720u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A724u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A728u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A72Cu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A730u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A734u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A738u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A73Cu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A740u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A744u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A748u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A74Cu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A750u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A754u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A758u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A75Cu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A760u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A764u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A768u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A76Cu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A770u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A774u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A778u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A77Cu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A780u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A784u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A788u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A78Cu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A790u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A794u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A798u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A79Cu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A7A0u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A7A4u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A7A8u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A7ACu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A7B0u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A7B4u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A7B8u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A7BCu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A7C0u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A7C4u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A7C8u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A7CCu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A7D0u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A7D4u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A7D8u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A7DCu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A7E0u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A7E4u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A7E8u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A7ECu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A7F0u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A7F4u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A7F8u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A7FCu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A800u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A804u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A808u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A80Cu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A810u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A814u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A818u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A81Cu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A820u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A824u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A828u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A82Cu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A830u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A834u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A838u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A83Cu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A840u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A844u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A848u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A84Cu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A850u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A854u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A858u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A85Cu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A860u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A864u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A868u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A86Cu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A870u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A874u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A878u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A87Cu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A880u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A884u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A888u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A88Cu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A890u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A894u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A898u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A89Cu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A8A0u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A8A4u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A8A8u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A8ACu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A8B0u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A8B4u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A8B8u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A8BCu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A8C0u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A8C4u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A8C8u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A8CCu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A8D0u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A8D4u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A8D8u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A8DCu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A8E0u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A8E4u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A8E8u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A8ECu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A8F0u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A8F4u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A8F8u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A8FCu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A900u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A904u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A908u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A90Cu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A910u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A914u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A918u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A91Cu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A920u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A924u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A928u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A92Cu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A930u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A934u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A938u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A93Cu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A940u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A944u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A948u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A94Cu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A950u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A954u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A958u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A95Cu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A960u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A964u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A968u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A96Cu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A970u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A974u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A978u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A97Cu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A980u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A984u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A988u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A98Cu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A990u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A994u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A998u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A99Cu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A9A0u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A9A4u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A9A8u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A9ACu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A9B0u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A9B4u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A9B8u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A9BCu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A9C0u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A9C4u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A9C8u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A9CCu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A9D0u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A9D4u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A9D8u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A9DCu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A9E0u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A9E4u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A9E8u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A9ECu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A9F0u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A9F4u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A9F8u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2A9FCu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2AA00u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2AA04u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2AA08u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2AA0Cu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2AA10u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2AA14u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2AA18u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2AA1Cu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2AA20u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2AA24u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2AA28u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2AA2Cu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2AA30u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2AA34u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2AA38u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2AA3Cu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2AA40u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2AA44u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2AA48u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2AA4Cu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2AA50u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2AA54u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2AA58u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2AA5Cu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2AA60u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2AA64u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2AA68u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2AA6Cu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2AA70u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2AA74u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2AA78u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2AA7Cu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2AA80u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2AA84u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2AA88u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2AA8Cu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2AA90u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2AA94u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2AA98u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2AA9Cu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2AAA0u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2AAA4u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2AAA8u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2AAACu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2AAB0u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2AAB4u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2AAB8u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2AABCu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2AAC0u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2AAC4u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2AAC8u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2AACCu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2AAD0u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2AAD4u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2AAD8u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2AADCu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2AAE0u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2AAE4u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2AAE8u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2AAECu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2AAF0u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2AAF4u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2AAF8u, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
    runtime.register_function(0x09D2AAFCu, &ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004, "recomp_unit_ovl09D1A280_em21_9E5C67A047FDEEE5_unit_0004");
}
} // namespace psprecomp
