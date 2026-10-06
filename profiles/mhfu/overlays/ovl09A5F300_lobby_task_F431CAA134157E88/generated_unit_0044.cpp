#include "psprecomp/runtime.hpp"
#include "ovl09A5F300_lobby_task_F431CAA134157E88_units.hpp"
#include <bit>
#include <cmath>
#include <cstdint>
#include <limits>

namespace psprecomp {
static const std::uint16_t kEntryIds_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044[864] = {
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
    513, 514, 515, 516, 517, 518, 519, 520, 521, 522, 523, 524, 525, 526, 527, 528, 529, 530, 531, 532, 533, 534, 535, 536, 537, 538, 539, 540, 541, 542, 543, 544,
    545, 546, 547, 548, 549, 550, 551, 552, 553, 554, 555, 556, 557, 558, 559, 560, 561, 562, 563, 564, 565, 566, 567, 568, 569, 570, 571, 572, 573, 574, 575, 576,
    577, 578, 579, 580, 581, 582, 583, 584, 585, 586, 587, 588, 589, 590, 591, 592, 593, 594, 595, 596, 597, 598, 599, 600, 601, 602, 603, 604, 605, 606, 607, 608,
    609, 610, 611, 612, 613, 614, 615, 616, 617, 618, 619, 620, 621, 622, 623, 624, 625, 626, 627, 628, 629, 630, 631, 632, 633, 634, 635, 636, 637, 638, 639, 640,
    641, 642, 643, 644, 645, 646, 647, 648, 649, 650, 651, 652, 653, 654, 655, 656, 657, 658, 659, 660, 661, 662, 663, 664, 665, 666, 667, 668, 669, 670, 671, 672,
    673, 674, 675, 676, 677, 678, 679, 680, 681, 682, 683, 684, 685, 686, 687, 688, 689, 690, 691, 692, 693, 694, 695, 696, 697, 698, 699, 700, 701, 702, 703, 704,
    705, 706, 707, 708, 709, 710, 711, 712, 713, 714, 715, 716, 717, 718, 719, 720, 721, 722, 723, 724, 725, 726, 727, 728, 729, 730, 731, 732, 733, 734, 735, 736,
    737, 738, 739, 740, 741, 742, 743, 744, 745, 746, 747, 748, 749, 750, 751, 752, 753, 754, 755, 756, 757, 758, 759, 760, 761, 762, 763, 764, 765, 766, 767, 768,
    769, 770, 771, 772, 773, 774, 775, 776, 777, 778, 779, 780, 781, 782, 783, 784, 785, 786, 787, 788, 789, 790, 791, 792, 793, 794, 795, 796, 797, 798, 799, 800,
    801, 802, 803, 804, 805, 806, 807, 808, 809, 810, 811, 812, 813, 814, 815, 816, 817, 818, 819, 820, 821, 822, 823, 824, 825, 826, 827, 828, 829, 830, 831, 832,
    833, 834, 835, 836, 837, 838, 839, 840, 841, 842, 843, 844, 845, 846, 847, 848, 849, 850, 851, 852, 853, 854, 855, 856, 857, 858, 859, 860, 861, 862, 863, 864,
};
void ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044_entry(Runtime &rt, AllegrexContext &ctx, std::uint16_t direct_entry_id, GuestMemory::AotFastView &aot_mem) {
    std::uint32_t jump_target = 0u;
    std::uint32_t local_transfers = 0u;
    std::uint32_t local_pc = ctx.pc;
    std::uint32_t entry_id = direct_entry_id;
LOCAL_DISPATCH:
    {
    if (entry_id == 0u) {
        const std::uint32_t entry_delta = local_pc - 0x09B0F300u;
        entry_id = (entry_delta < 3456u && (entry_delta & 3u) == 0u) ? kEntryIds_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044[entry_delta >> 2u] : 0u;
    }
    switch (entry_id) {
    case 1u: goto L_09B0F300;
    case 2u: goto L_09B0F304;
    case 3u: goto L_09B0F308;
    case 4u: goto L_09B0F30C;
    case 5u: goto L_09B0F310;
    case 6u: goto L_09B0F314;
    case 7u: goto L_09B0F318;
    case 8u: goto L_09B0F31C;
    case 9u: goto L_09B0F320;
    case 10u: goto L_09B0F324;
    case 11u: goto L_09B0F328;
    case 12u: goto L_09B0F32C;
    case 13u: goto L_09B0F330;
    case 14u: goto L_09B0F334;
    case 15u: goto L_09B0F338;
    case 16u: goto L_09B0F33C;
    case 17u: goto L_09B0F340;
    case 18u: goto L_09B0F344;
    case 19u: goto L_09B0F348;
    case 20u: goto L_09B0F34C;
    case 21u: goto L_09B0F350;
    case 22u: goto L_09B0F354;
    case 23u: goto L_09B0F358;
    case 24u: goto L_09B0F35C;
    case 25u: goto L_09B0F360;
    case 26u: goto L_09B0F364;
    case 27u: goto L_09B0F368;
    case 28u: goto L_09B0F36C;
    case 29u: goto L_09B0F370;
    case 30u: goto L_09B0F374;
    case 31u: goto L_09B0F378;
    case 32u: goto L_09B0F37C;
    case 33u: goto L_09B0F380;
    case 34u: goto L_09B0F384;
    case 35u: goto L_09B0F388;
    case 36u: goto L_09B0F38C;
    case 37u: goto L_09B0F390;
    case 38u: goto L_09B0F394;
    case 39u: goto L_09B0F398;
    case 40u: goto L_09B0F39C;
    case 41u: goto L_09B0F3A0;
    case 42u: goto L_09B0F3A4;
    case 43u: goto L_09B0F3A8;
    case 44u: goto L_09B0F3AC;
    case 45u: goto L_09B0F3B0;
    case 46u: goto L_09B0F3B4;
    case 47u: goto L_09B0F3B8;
    case 48u: goto L_09B0F3BC;
    case 49u: goto L_09B0F3C0;
    case 50u: goto L_09B0F3C4;
    case 51u: goto L_09B0F3C8;
    case 52u: goto L_09B0F3CC;
    case 53u: goto L_09B0F3D0;
    case 54u: goto L_09B0F3D4;
    case 55u: goto L_09B0F3D8;
    case 56u: goto L_09B0F3DC;
    case 57u: goto L_09B0F3E0;
    case 58u: goto L_09B0F3E4;
    case 59u: goto L_09B0F3E8;
    case 60u: goto L_09B0F3EC;
    case 61u: goto L_09B0F3F0;
    case 62u: goto L_09B0F3F4;
    case 63u: goto L_09B0F3F8;
    case 64u: goto L_09B0F3FC;
    case 65u: goto L_09B0F400;
    case 66u: goto L_09B0F404;
    case 67u: goto L_09B0F408;
    case 68u: goto L_09B0F40C;
    case 69u: goto L_09B0F410;
    case 70u: goto L_09B0F414;
    case 71u: goto L_09B0F418;
    case 72u: goto L_09B0F41C;
    case 73u: goto L_09B0F420;
    case 74u: goto L_09B0F424;
    case 75u: goto L_09B0F428;
    case 76u: goto L_09B0F42C;
    case 77u: goto L_09B0F430;
    case 78u: goto L_09B0F434;
    case 79u: goto L_09B0F438;
    case 80u: goto L_09B0F43C;
    case 81u: goto L_09B0F440;
    case 82u: goto L_09B0F444;
    case 83u: goto L_09B0F448;
    case 84u: goto L_09B0F44C;
    case 85u: goto L_09B0F450;
    case 86u: goto L_09B0F454;
    case 87u: goto L_09B0F458;
    case 88u: goto L_09B0F45C;
    case 89u: goto L_09B0F460;
    case 90u: goto L_09B0F464;
    case 91u: goto L_09B0F468;
    case 92u: goto L_09B0F46C;
    case 93u: goto L_09B0F470;
    case 94u: goto L_09B0F474;
    case 95u: goto L_09B0F478;
    case 96u: goto L_09B0F47C;
    case 97u: goto L_09B0F480;
    case 98u: goto L_09B0F484;
    case 99u: goto L_09B0F488;
    case 100u: goto L_09B0F48C;
    case 101u: goto L_09B0F490;
    case 102u: goto L_09B0F494;
    case 103u: goto L_09B0F498;
    case 104u: goto L_09B0F49C;
    case 105u: goto L_09B0F4A0;
    case 106u: goto L_09B0F4A4;
    case 107u: goto L_09B0F4A8;
    case 108u: goto L_09B0F4AC;
    case 109u: goto L_09B0F4B0;
    case 110u: goto L_09B0F4B4;
    case 111u: goto L_09B0F4B8;
    case 112u: goto L_09B0F4BC;
    case 113u: goto L_09B0F4C0;
    case 114u: goto L_09B0F4C4;
    case 115u: goto L_09B0F4C8;
    case 116u: goto L_09B0F4CC;
    case 117u: goto L_09B0F4D0;
    case 118u: goto L_09B0F4D4;
    case 119u: goto L_09B0F4D8;
    case 120u: goto L_09B0F4DC;
    case 121u: goto L_09B0F4E0;
    case 122u: goto L_09B0F4E4;
    case 123u: goto L_09B0F4E8;
    case 124u: goto L_09B0F4EC;
    case 125u: goto L_09B0F4F0;
    case 126u: goto L_09B0F4F4;
    case 127u: goto L_09B0F4F8;
    case 128u: goto L_09B0F4FC;
    case 129u: goto L_09B0F500;
    case 130u: goto L_09B0F504;
    case 131u: goto L_09B0F508;
    case 132u: goto L_09B0F50C;
    case 133u: goto L_09B0F510;
    case 134u: goto L_09B0F514;
    case 135u: goto L_09B0F518;
    case 136u: goto L_09B0F51C;
    case 137u: goto L_09B0F520;
    case 138u: goto L_09B0F524;
    case 139u: goto L_09B0F528;
    case 140u: goto L_09B0F52C;
    case 141u: goto L_09B0F530;
    case 142u: goto L_09B0F534;
    case 143u: goto L_09B0F538;
    case 144u: goto L_09B0F53C;
    case 145u: goto L_09B0F540;
    case 146u: goto L_09B0F544;
    case 147u: goto L_09B0F548;
    case 148u: goto L_09B0F54C;
    case 149u: goto L_09B0F550;
    case 150u: goto L_09B0F554;
    case 151u: goto L_09B0F558;
    case 152u: goto L_09B0F55C;
    case 153u: goto L_09B0F560;
    case 154u: goto L_09B0F564;
    case 155u: goto L_09B0F568;
    case 156u: goto L_09B0F56C;
    case 157u: goto L_09B0F570;
    case 158u: goto L_09B0F574;
    case 159u: goto L_09B0F578;
    case 160u: goto L_09B0F57C;
    case 161u: goto L_09B0F580;
    case 162u: goto L_09B0F584;
    case 163u: goto L_09B0F588;
    case 164u: goto L_09B0F58C;
    case 165u: goto L_09B0F590;
    case 166u: goto L_09B0F594;
    case 167u: goto L_09B0F598;
    case 168u: goto L_09B0F59C;
    case 169u: goto L_09B0F5A0;
    case 170u: goto L_09B0F5A4;
    case 171u: goto L_09B0F5A8;
    case 172u: goto L_09B0F5AC;
    case 173u: goto L_09B0F5B0;
    case 174u: goto L_09B0F5B4;
    case 175u: goto L_09B0F5B8;
    case 176u: goto L_09B0F5BC;
    case 177u: goto L_09B0F5C0;
    case 178u: goto L_09B0F5C4;
    case 179u: goto L_09B0F5C8;
    case 180u: goto L_09B0F5CC;
    case 181u: goto L_09B0F5D0;
    case 182u: goto L_09B0F5D4;
    case 183u: goto L_09B0F5D8;
    case 184u: goto L_09B0F5DC;
    case 185u: goto L_09B0F5E0;
    case 186u: goto L_09B0F5E4;
    case 187u: goto L_09B0F5E8;
    case 188u: goto L_09B0F5EC;
    case 189u: goto L_09B0F5F0;
    case 190u: goto L_09B0F5F4;
    case 191u: goto L_09B0F5F8;
    case 192u: goto L_09B0F5FC;
    case 193u: goto L_09B0F600;
    case 194u: goto L_09B0F604;
    case 195u: goto L_09B0F608;
    case 196u: goto L_09B0F60C;
    case 197u: goto L_09B0F610;
    case 198u: goto L_09B0F614;
    case 199u: goto L_09B0F618;
    case 200u: goto L_09B0F61C;
    case 201u: goto L_09B0F620;
    case 202u: goto L_09B0F624;
    case 203u: goto L_09B0F628;
    case 204u: goto L_09B0F62C;
    case 205u: goto L_09B0F630;
    case 206u: goto L_09B0F634;
    case 207u: goto L_09B0F638;
    case 208u: goto L_09B0F63C;
    case 209u: goto L_09B0F640;
    case 210u: goto L_09B0F644;
    case 211u: goto L_09B0F648;
    case 212u: goto L_09B0F64C;
    case 213u: goto L_09B0F650;
    case 214u: goto L_09B0F654;
    case 215u: goto L_09B0F658;
    case 216u: goto L_09B0F65C;
    case 217u: goto L_09B0F660;
    case 218u: goto L_09B0F664;
    case 219u: goto L_09B0F668;
    case 220u: goto L_09B0F66C;
    case 221u: goto L_09B0F670;
    case 222u: goto L_09B0F674;
    case 223u: goto L_09B0F678;
    case 224u: goto L_09B0F67C;
    case 225u: goto L_09B0F680;
    case 226u: goto L_09B0F684;
    case 227u: goto L_09B0F688;
    case 228u: goto L_09B0F68C;
    case 229u: goto L_09B0F690;
    case 230u: goto L_09B0F694;
    case 231u: goto L_09B0F698;
    case 232u: goto L_09B0F69C;
    case 233u: goto L_09B0F6A0;
    case 234u: goto L_09B0F6A4;
    case 235u: goto L_09B0F6A8;
    case 236u: goto L_09B0F6AC;
    case 237u: goto L_09B0F6B0;
    case 238u: goto L_09B0F6B4;
    case 239u: goto L_09B0F6B8;
    case 240u: goto L_09B0F6BC;
    case 241u: goto L_09B0F6C0;
    case 242u: goto L_09B0F6C4;
    case 243u: goto L_09B0F6C8;
    case 244u: goto L_09B0F6CC;
    case 245u: goto L_09B0F6D0;
    case 246u: goto L_09B0F6D4;
    case 247u: goto L_09B0F6D8;
    case 248u: goto L_09B0F6DC;
    case 249u: goto L_09B0F6E0;
    case 250u: goto L_09B0F6E4;
    case 251u: goto L_09B0F6E8;
    case 252u: goto L_09B0F6EC;
    case 253u: goto L_09B0F6F0;
    case 254u: goto L_09B0F6F4;
    case 255u: goto L_09B0F6F8;
    case 256u: goto L_09B0F6FC;
    case 257u: goto L_09B0F700;
    case 258u: goto L_09B0F704;
    case 259u: goto L_09B0F708;
    case 260u: goto L_09B0F70C;
    case 261u: goto L_09B0F710;
    case 262u: goto L_09B0F714;
    case 263u: goto L_09B0F718;
    case 264u: goto L_09B0F71C;
    case 265u: goto L_09B0F720;
    case 266u: goto L_09B0F724;
    case 267u: goto L_09B0F728;
    case 268u: goto L_09B0F72C;
    case 269u: goto L_09B0F730;
    case 270u: goto L_09B0F734;
    case 271u: goto L_09B0F738;
    case 272u: goto L_09B0F73C;
    case 273u: goto L_09B0F740;
    case 274u: goto L_09B0F744;
    case 275u: goto L_09B0F748;
    case 276u: goto L_09B0F74C;
    case 277u: goto L_09B0F750;
    case 278u: goto L_09B0F754;
    case 279u: goto L_09B0F758;
    case 280u: goto L_09B0F75C;
    case 281u: goto L_09B0F760;
    case 282u: goto L_09B0F764;
    case 283u: goto L_09B0F768;
    case 284u: goto L_09B0F76C;
    case 285u: goto L_09B0F770;
    case 286u: goto L_09B0F774;
    case 287u: goto L_09B0F778;
    case 288u: goto L_09B0F77C;
    case 289u: goto L_09B0F780;
    case 290u: goto L_09B0F784;
    case 291u: goto L_09B0F788;
    case 292u: goto L_09B0F78C;
    case 293u: goto L_09B0F790;
    case 294u: goto L_09B0F794;
    case 295u: goto L_09B0F798;
    case 296u: goto L_09B0F79C;
    case 297u: goto L_09B0F7A0;
    case 298u: goto L_09B0F7A4;
    case 299u: goto L_09B0F7A8;
    case 300u: goto L_09B0F7AC;
    case 301u: goto L_09B0F7B0;
    case 302u: goto L_09B0F7B4;
    case 303u: goto L_09B0F7B8;
    case 304u: goto L_09B0F7BC;
    case 305u: goto L_09B0F7C0;
    case 306u: goto L_09B0F7C4;
    case 307u: goto L_09B0F7C8;
    case 308u: goto L_09B0F7CC;
    case 309u: goto L_09B0F7D0;
    case 310u: goto L_09B0F7D4;
    case 311u: goto L_09B0F7D8;
    case 312u: goto L_09B0F7DC;
    case 313u: goto L_09B0F7E0;
    case 314u: goto L_09B0F7E4;
    case 315u: goto L_09B0F7E8;
    case 316u: goto L_09B0F7EC;
    case 317u: goto L_09B0F7F0;
    case 318u: goto L_09B0F7F4;
    case 319u: goto L_09B0F7F8;
    case 320u: goto L_09B0F7FC;
    case 321u: goto L_09B0F800;
    case 322u: goto L_09B0F804;
    case 323u: goto L_09B0F808;
    case 324u: goto L_09B0F80C;
    case 325u: goto L_09B0F810;
    case 326u: goto L_09B0F814;
    case 327u: goto L_09B0F818;
    case 328u: goto L_09B0F81C;
    case 329u: goto L_09B0F820;
    case 330u: goto L_09B0F824;
    case 331u: goto L_09B0F828;
    case 332u: goto L_09B0F82C;
    case 333u: goto L_09B0F830;
    case 334u: goto L_09B0F834;
    case 335u: goto L_09B0F838;
    case 336u: goto L_09B0F83C;
    case 337u: goto L_09B0F840;
    case 338u: goto L_09B0F844;
    case 339u: goto L_09B0F848;
    case 340u: goto L_09B0F84C;
    case 341u: goto L_09B0F850;
    case 342u: goto L_09B0F854;
    case 343u: goto L_09B0F858;
    case 344u: goto L_09B0F85C;
    case 345u: goto L_09B0F860;
    case 346u: goto L_09B0F864;
    case 347u: goto L_09B0F868;
    case 348u: goto L_09B0F86C;
    case 349u: goto L_09B0F870;
    case 350u: goto L_09B0F874;
    case 351u: goto L_09B0F878;
    case 352u: goto L_09B0F87C;
    case 353u: goto L_09B0F880;
    case 354u: goto L_09B0F884;
    case 355u: goto L_09B0F888;
    case 356u: goto L_09B0F88C;
    case 357u: goto L_09B0F890;
    case 358u: goto L_09B0F894;
    case 359u: goto L_09B0F898;
    case 360u: goto L_09B0F89C;
    case 361u: goto L_09B0F8A0;
    case 362u: goto L_09B0F8A4;
    case 363u: goto L_09B0F8A8;
    case 364u: goto L_09B0F8AC;
    case 365u: goto L_09B0F8B0;
    case 366u: goto L_09B0F8B4;
    case 367u: goto L_09B0F8B8;
    case 368u: goto L_09B0F8BC;
    case 369u: goto L_09B0F8C0;
    case 370u: goto L_09B0F8C4;
    case 371u: goto L_09B0F8C8;
    case 372u: goto L_09B0F8CC;
    case 373u: goto L_09B0F8D0;
    case 374u: goto L_09B0F8D4;
    case 375u: goto L_09B0F8D8;
    case 376u: goto L_09B0F8DC;
    case 377u: goto L_09B0F8E0;
    case 378u: goto L_09B0F8E4;
    case 379u: goto L_09B0F8E8;
    case 380u: goto L_09B0F8EC;
    case 381u: goto L_09B0F8F0;
    case 382u: goto L_09B0F8F4;
    case 383u: goto L_09B0F8F8;
    case 384u: goto L_09B0F8FC;
    case 385u: goto L_09B0F900;
    case 386u: goto L_09B0F904;
    case 387u: goto L_09B0F908;
    case 388u: goto L_09B0F90C;
    case 389u: goto L_09B0F910;
    case 390u: goto L_09B0F914;
    case 391u: goto L_09B0F918;
    case 392u: goto L_09B0F91C;
    case 393u: goto L_09B0F920;
    case 394u: goto L_09B0F924;
    case 395u: goto L_09B0F928;
    case 396u: goto L_09B0F92C;
    case 397u: goto L_09B0F930;
    case 398u: goto L_09B0F934;
    case 399u: goto L_09B0F938;
    case 400u: goto L_09B0F93C;
    case 401u: goto L_09B0F940;
    case 402u: goto L_09B0F944;
    case 403u: goto L_09B0F948;
    case 404u: goto L_09B0F94C;
    case 405u: goto L_09B0F950;
    case 406u: goto L_09B0F954;
    case 407u: goto L_09B0F958;
    case 408u: goto L_09B0F95C;
    case 409u: goto L_09B0F960;
    case 410u: goto L_09B0F964;
    case 411u: goto L_09B0F968;
    case 412u: goto L_09B0F96C;
    case 413u: goto L_09B0F970;
    case 414u: goto L_09B0F974;
    case 415u: goto L_09B0F978;
    case 416u: goto L_09B0F97C;
    case 417u: goto L_09B0F980;
    case 418u: goto L_09B0F984;
    case 419u: goto L_09B0F988;
    case 420u: goto L_09B0F98C;
    case 421u: goto L_09B0F990;
    case 422u: goto L_09B0F994;
    case 423u: goto L_09B0F998;
    case 424u: goto L_09B0F99C;
    case 425u: goto L_09B0F9A0;
    case 426u: goto L_09B0F9A4;
    case 427u: goto L_09B0F9A8;
    case 428u: goto L_09B0F9AC;
    case 429u: goto L_09B0F9B0;
    case 430u: goto L_09B0F9B4;
    case 431u: goto L_09B0F9B8;
    case 432u: goto L_09B0F9BC;
    case 433u: goto L_09B0F9C0;
    case 434u: goto L_09B0F9C4;
    case 435u: goto L_09B0F9C8;
    case 436u: goto L_09B0F9CC;
    case 437u: goto L_09B0F9D0;
    case 438u: goto L_09B0F9D4;
    case 439u: goto L_09B0F9D8;
    case 440u: goto L_09B0F9DC;
    case 441u: goto L_09B0F9E0;
    case 442u: goto L_09B0F9E4;
    case 443u: goto L_09B0F9E8;
    case 444u: goto L_09B0F9EC;
    case 445u: goto L_09B0F9F0;
    case 446u: goto L_09B0F9F4;
    case 447u: goto L_09B0F9F8;
    case 448u: goto L_09B0F9FC;
    case 449u: goto L_09B0FA00;
    case 450u: goto L_09B0FA04;
    case 451u: goto L_09B0FA08;
    case 452u: goto L_09B0FA0C;
    case 453u: goto L_09B0FA10;
    case 454u: goto L_09B0FA14;
    case 455u: goto L_09B0FA18;
    case 456u: goto L_09B0FA1C;
    case 457u: goto L_09B0FA20;
    case 458u: goto L_09B0FA24;
    case 459u: goto L_09B0FA28;
    case 460u: goto L_09B0FA2C;
    case 461u: goto L_09B0FA30;
    case 462u: goto L_09B0FA34;
    case 463u: goto L_09B0FA38;
    case 464u: goto L_09B0FA3C;
    case 465u: goto L_09B0FA40;
    case 466u: goto L_09B0FA44;
    case 467u: goto L_09B0FA48;
    case 468u: goto L_09B0FA4C;
    case 469u: goto L_09B0FA50;
    case 470u: goto L_09B0FA54;
    case 471u: goto L_09B0FA58;
    case 472u: goto L_09B0FA5C;
    case 473u: goto L_09B0FA60;
    case 474u: goto L_09B0FA64;
    case 475u: goto L_09B0FA68;
    case 476u: goto L_09B0FA6C;
    case 477u: goto L_09B0FA70;
    case 478u: goto L_09B0FA74;
    case 479u: goto L_09B0FA78;
    case 480u: goto L_09B0FA7C;
    case 481u: goto L_09B0FA80;
    case 482u: goto L_09B0FA84;
    case 483u: goto L_09B0FA88;
    case 484u: goto L_09B0FA8C;
    case 485u: goto L_09B0FA90;
    case 486u: goto L_09B0FA94;
    case 487u: goto L_09B0FA98;
    case 488u: goto L_09B0FA9C;
    case 489u: goto L_09B0FAA0;
    case 490u: goto L_09B0FAA4;
    case 491u: goto L_09B0FAA8;
    case 492u: goto L_09B0FAAC;
    case 493u: goto L_09B0FAB0;
    case 494u: goto L_09B0FAB4;
    case 495u: goto L_09B0FAB8;
    case 496u: goto L_09B0FABC;
    case 497u: goto L_09B0FAC0;
    case 498u: goto L_09B0FAC4;
    case 499u: goto L_09B0FAC8;
    case 500u: goto L_09B0FACC;
    case 501u: goto L_09B0FAD0;
    case 502u: goto L_09B0FAD4;
    case 503u: goto L_09B0FAD8;
    case 504u: goto L_09B0FADC;
    case 505u: goto L_09B0FAE0;
    case 506u: goto L_09B0FAE4;
    case 507u: goto L_09B0FAE8;
    case 508u: goto L_09B0FAEC;
    case 509u: goto L_09B0FAF0;
    case 510u: goto L_09B0FAF4;
    case 511u: goto L_09B0FAF8;
    case 512u: goto L_09B0FAFC;
    case 513u: goto L_09B0FB00;
    case 514u: goto L_09B0FB04;
    case 515u: goto L_09B0FB08;
    case 516u: goto L_09B0FB0C;
    case 517u: goto L_09B0FB10;
    case 518u: goto L_09B0FB14;
    case 519u: goto L_09B0FB18;
    case 520u: goto L_09B0FB1C;
    case 521u: goto L_09B0FB20;
    case 522u: goto L_09B0FB24;
    case 523u: goto L_09B0FB28;
    case 524u: goto L_09B0FB2C;
    case 525u: goto L_09B0FB30;
    case 526u: goto L_09B0FB34;
    case 527u: goto L_09B0FB38;
    case 528u: goto L_09B0FB3C;
    case 529u: goto L_09B0FB40;
    case 530u: goto L_09B0FB44;
    case 531u: goto L_09B0FB48;
    case 532u: goto L_09B0FB4C;
    case 533u: goto L_09B0FB50;
    case 534u: goto L_09B0FB54;
    case 535u: goto L_09B0FB58;
    case 536u: goto L_09B0FB5C;
    case 537u: goto L_09B0FB60;
    case 538u: goto L_09B0FB64;
    case 539u: goto L_09B0FB68;
    case 540u: goto L_09B0FB6C;
    case 541u: goto L_09B0FB70;
    case 542u: goto L_09B0FB74;
    case 543u: goto L_09B0FB78;
    case 544u: goto L_09B0FB7C;
    case 545u: goto L_09B0FB80;
    case 546u: goto L_09B0FB84;
    case 547u: goto L_09B0FB88;
    case 548u: goto L_09B0FB8C;
    case 549u: goto L_09B0FB90;
    case 550u: goto L_09B0FB94;
    case 551u: goto L_09B0FB98;
    case 552u: goto L_09B0FB9C;
    case 553u: goto L_09B0FBA0;
    case 554u: goto L_09B0FBA4;
    case 555u: goto L_09B0FBA8;
    case 556u: goto L_09B0FBAC;
    case 557u: goto L_09B0FBB0;
    case 558u: goto L_09B0FBB4;
    case 559u: goto L_09B0FBB8;
    case 560u: goto L_09B0FBBC;
    case 561u: goto L_09B0FBC0;
    case 562u: goto L_09B0FBC4;
    case 563u: goto L_09B0FBC8;
    case 564u: goto L_09B0FBCC;
    case 565u: goto L_09B0FBD0;
    case 566u: goto L_09B0FBD4;
    case 567u: goto L_09B0FBD8;
    case 568u: goto L_09B0FBDC;
    case 569u: goto L_09B0FBE0;
    case 570u: goto L_09B0FBE4;
    case 571u: goto L_09B0FBE8;
    case 572u: goto L_09B0FBEC;
    case 573u: goto L_09B0FBF0;
    case 574u: goto L_09B0FBF4;
    case 575u: goto L_09B0FBF8;
    case 576u: goto L_09B0FBFC;
    case 577u: goto L_09B0FC00;
    case 578u: goto L_09B0FC04;
    case 579u: goto L_09B0FC08;
    case 580u: goto L_09B0FC0C;
    case 581u: goto L_09B0FC10;
    case 582u: goto L_09B0FC14;
    case 583u: goto L_09B0FC18;
    case 584u: goto L_09B0FC1C;
    case 585u: goto L_09B0FC20;
    case 586u: goto L_09B0FC24;
    case 587u: goto L_09B0FC28;
    case 588u: goto L_09B0FC2C;
    case 589u: goto L_09B0FC30;
    case 590u: goto L_09B0FC34;
    case 591u: goto L_09B0FC38;
    case 592u: goto L_09B0FC3C;
    case 593u: goto L_09B0FC40;
    case 594u: goto L_09B0FC44;
    case 595u: goto L_09B0FC48;
    case 596u: goto L_09B0FC4C;
    case 597u: goto L_09B0FC50;
    case 598u: goto L_09B0FC54;
    case 599u: goto L_09B0FC58;
    case 600u: goto L_09B0FC5C;
    case 601u: goto L_09B0FC60;
    case 602u: goto L_09B0FC64;
    case 603u: goto L_09B0FC68;
    case 604u: goto L_09B0FC6C;
    case 605u: goto L_09B0FC70;
    case 606u: goto L_09B0FC74;
    case 607u: goto L_09B0FC78;
    case 608u: goto L_09B0FC7C;
    case 609u: goto L_09B0FC80;
    case 610u: goto L_09B0FC84;
    case 611u: goto L_09B0FC88;
    case 612u: goto L_09B0FC8C;
    case 613u: goto L_09B0FC90;
    case 614u: goto L_09B0FC94;
    case 615u: goto L_09B0FC98;
    case 616u: goto L_09B0FC9C;
    case 617u: goto L_09B0FCA0;
    case 618u: goto L_09B0FCA4;
    case 619u: goto L_09B0FCA8;
    case 620u: goto L_09B0FCAC;
    case 621u: goto L_09B0FCB0;
    case 622u: goto L_09B0FCB4;
    case 623u: goto L_09B0FCB8;
    case 624u: goto L_09B0FCBC;
    case 625u: goto L_09B0FCC0;
    case 626u: goto L_09B0FCC4;
    case 627u: goto L_09B0FCC8;
    case 628u: goto L_09B0FCCC;
    case 629u: goto L_09B0FCD0;
    case 630u: goto L_09B0FCD4;
    case 631u: goto L_09B0FCD8;
    case 632u: goto L_09B0FCDC;
    case 633u: goto L_09B0FCE0;
    case 634u: goto L_09B0FCE4;
    case 635u: goto L_09B0FCE8;
    case 636u: goto L_09B0FCEC;
    case 637u: goto L_09B0FCF0;
    case 638u: goto L_09B0FCF4;
    case 639u: goto L_09B0FCF8;
    case 640u: goto L_09B0FCFC;
    case 641u: goto L_09B0FD00;
    case 642u: goto L_09B0FD04;
    case 643u: goto L_09B0FD08;
    case 644u: goto L_09B0FD0C;
    case 645u: goto L_09B0FD10;
    case 646u: goto L_09B0FD14;
    case 647u: goto L_09B0FD18;
    case 648u: goto L_09B0FD1C;
    case 649u: goto L_09B0FD20;
    case 650u: goto L_09B0FD24;
    case 651u: goto L_09B0FD28;
    case 652u: goto L_09B0FD2C;
    case 653u: goto L_09B0FD30;
    case 654u: goto L_09B0FD34;
    case 655u: goto L_09B0FD38;
    case 656u: goto L_09B0FD3C;
    case 657u: goto L_09B0FD40;
    case 658u: goto L_09B0FD44;
    case 659u: goto L_09B0FD48;
    case 660u: goto L_09B0FD4C;
    case 661u: goto L_09B0FD50;
    case 662u: goto L_09B0FD54;
    case 663u: goto L_09B0FD58;
    case 664u: goto L_09B0FD5C;
    case 665u: goto L_09B0FD60;
    case 666u: goto L_09B0FD64;
    case 667u: goto L_09B0FD68;
    case 668u: goto L_09B0FD6C;
    case 669u: goto L_09B0FD70;
    case 670u: goto L_09B0FD74;
    case 671u: goto L_09B0FD78;
    case 672u: goto L_09B0FD7C;
    case 673u: goto L_09B0FD80;
    case 674u: goto L_09B0FD84;
    case 675u: goto L_09B0FD88;
    case 676u: goto L_09B0FD8C;
    case 677u: goto L_09B0FD90;
    case 678u: goto L_09B0FD94;
    case 679u: goto L_09B0FD98;
    case 680u: goto L_09B0FD9C;
    case 681u: goto L_09B0FDA0;
    case 682u: goto L_09B0FDA4;
    case 683u: goto L_09B0FDA8;
    case 684u: goto L_09B0FDAC;
    case 685u: goto L_09B0FDB0;
    case 686u: goto L_09B0FDB4;
    case 687u: goto L_09B0FDB8;
    case 688u: goto L_09B0FDBC;
    case 689u: goto L_09B0FDC0;
    case 690u: goto L_09B0FDC4;
    case 691u: goto L_09B0FDC8;
    case 692u: goto L_09B0FDCC;
    case 693u: goto L_09B0FDD0;
    case 694u: goto L_09B0FDD4;
    case 695u: goto L_09B0FDD8;
    case 696u: goto L_09B0FDDC;
    case 697u: goto L_09B0FDE0;
    case 698u: goto L_09B0FDE4;
    case 699u: goto L_09B0FDE8;
    case 700u: goto L_09B0FDEC;
    case 701u: goto L_09B0FDF0;
    case 702u: goto L_09B0FDF4;
    case 703u: goto L_09B0FDF8;
    case 704u: goto L_09B0FDFC;
    case 705u: goto L_09B0FE00;
    case 706u: goto L_09B0FE04;
    case 707u: goto L_09B0FE08;
    case 708u: goto L_09B0FE0C;
    case 709u: goto L_09B0FE10;
    case 710u: goto L_09B0FE14;
    case 711u: goto L_09B0FE18;
    case 712u: goto L_09B0FE1C;
    case 713u: goto L_09B0FE20;
    case 714u: goto L_09B0FE24;
    case 715u: goto L_09B0FE28;
    case 716u: goto L_09B0FE2C;
    case 717u: goto L_09B0FE30;
    case 718u: goto L_09B0FE34;
    case 719u: goto L_09B0FE38;
    case 720u: goto L_09B0FE3C;
    case 721u: goto L_09B0FE40;
    case 722u: goto L_09B0FE44;
    case 723u: goto L_09B0FE48;
    case 724u: goto L_09B0FE4C;
    case 725u: goto L_09B0FE50;
    case 726u: goto L_09B0FE54;
    case 727u: goto L_09B0FE58;
    case 728u: goto L_09B0FE5C;
    case 729u: goto L_09B0FE60;
    case 730u: goto L_09B0FE64;
    case 731u: goto L_09B0FE68;
    case 732u: goto L_09B0FE6C;
    case 733u: goto L_09B0FE70;
    case 734u: goto L_09B0FE74;
    case 735u: goto L_09B0FE78;
    case 736u: goto L_09B0FE7C;
    case 737u: goto L_09B0FE80;
    case 738u: goto L_09B0FE84;
    case 739u: goto L_09B0FE88;
    case 740u: goto L_09B0FE8C;
    case 741u: goto L_09B0FE90;
    case 742u: goto L_09B0FE94;
    case 743u: goto L_09B0FE98;
    case 744u: goto L_09B0FE9C;
    case 745u: goto L_09B0FEA0;
    case 746u: goto L_09B0FEA4;
    case 747u: goto L_09B0FEA8;
    case 748u: goto L_09B0FEAC;
    case 749u: goto L_09B0FEB0;
    case 750u: goto L_09B0FEB4;
    case 751u: goto L_09B0FEB8;
    case 752u: goto L_09B0FEBC;
    case 753u: goto L_09B0FEC0;
    case 754u: goto L_09B0FEC4;
    case 755u: goto L_09B0FEC8;
    case 756u: goto L_09B0FECC;
    case 757u: goto L_09B0FED0;
    case 758u: goto L_09B0FED4;
    case 759u: goto L_09B0FED8;
    case 760u: goto L_09B0FEDC;
    case 761u: goto L_09B0FEE0;
    case 762u: goto L_09B0FEE4;
    case 763u: goto L_09B0FEE8;
    case 764u: goto L_09B0FEEC;
    case 765u: goto L_09B0FEF0;
    case 766u: goto L_09B0FEF4;
    case 767u: goto L_09B0FEF8;
    case 768u: goto L_09B0FEFC;
    case 769u: goto L_09B0FF00;
    case 770u: goto L_09B0FF04;
    case 771u: goto L_09B0FF08;
    case 772u: goto L_09B0FF0C;
    case 773u: goto L_09B0FF10;
    case 774u: goto L_09B0FF14;
    case 775u: goto L_09B0FF18;
    case 776u: goto L_09B0FF1C;
    case 777u: goto L_09B0FF20;
    case 778u: goto L_09B0FF24;
    case 779u: goto L_09B0FF28;
    case 780u: goto L_09B0FF2C;
    case 781u: goto L_09B0FF30;
    case 782u: goto L_09B0FF34;
    case 783u: goto L_09B0FF38;
    case 784u: goto L_09B0FF3C;
    case 785u: goto L_09B0FF40;
    case 786u: goto L_09B0FF44;
    case 787u: goto L_09B0FF48;
    case 788u: goto L_09B0FF4C;
    case 789u: goto L_09B0FF50;
    case 790u: goto L_09B0FF54;
    case 791u: goto L_09B0FF58;
    case 792u: goto L_09B0FF5C;
    case 793u: goto L_09B0FF60;
    case 794u: goto L_09B0FF64;
    case 795u: goto L_09B0FF68;
    case 796u: goto L_09B0FF6C;
    case 797u: goto L_09B0FF70;
    case 798u: goto L_09B0FF74;
    case 799u: goto L_09B0FF78;
    case 800u: goto L_09B0FF7C;
    case 801u: goto L_09B0FF80;
    case 802u: goto L_09B0FF84;
    case 803u: goto L_09B0FF88;
    case 804u: goto L_09B0FF8C;
    case 805u: goto L_09B0FF90;
    case 806u: goto L_09B0FF94;
    case 807u: goto L_09B0FF98;
    case 808u: goto L_09B0FF9C;
    case 809u: goto L_09B0FFA0;
    case 810u: goto L_09B0FFA4;
    case 811u: goto L_09B0FFA8;
    case 812u: goto L_09B0FFAC;
    case 813u: goto L_09B0FFB0;
    case 814u: goto L_09B0FFB4;
    case 815u: goto L_09B0FFB8;
    case 816u: goto L_09B0FFBC;
    case 817u: goto L_09B0FFC0;
    case 818u: goto L_09B0FFC4;
    case 819u: goto L_09B0FFC8;
    case 820u: goto L_09B0FFCC;
    case 821u: goto L_09B0FFD0;
    case 822u: goto L_09B0FFD4;
    case 823u: goto L_09B0FFD8;
    case 824u: goto L_09B0FFDC;
    case 825u: goto L_09B0FFE0;
    case 826u: goto L_09B0FFE4;
    case 827u: goto L_09B0FFE8;
    case 828u: goto L_09B0FFEC;
    case 829u: goto L_09B0FFF0;
    case 830u: goto L_09B0FFF4;
    case 831u: goto L_09B0FFF8;
    case 832u: goto L_09B0FFFC;
    case 833u: goto L_09B10000;
    case 834u: goto L_09B10004;
    case 835u: goto L_09B10008;
    case 836u: goto L_09B1000C;
    case 837u: goto L_09B10010;
    case 838u: goto L_09B10014;
    case 839u: goto L_09B10018;
    case 840u: goto L_09B1001C;
    case 841u: goto L_09B10020;
    case 842u: goto L_09B10024;
    case 843u: goto L_09B10028;
    case 844u: goto L_09B1002C;
    case 845u: goto L_09B10030;
    case 846u: goto L_09B10034;
    case 847u: goto L_09B10038;
    case 848u: goto L_09B1003C;
    case 849u: goto L_09B10040;
    case 850u: goto L_09B10044;
    case 851u: goto L_09B10048;
    case 852u: goto L_09B1004C;
    case 853u: goto L_09B10050;
    case 854u: goto L_09B10054;
    case 855u: goto L_09B10058;
    case 856u: goto L_09B1005C;
    case 857u: goto L_09B10060;
    case 858u: goto L_09B10064;
    case 859u: goto L_09B10068;
    case 860u: goto L_09B1006C;
    case 861u: goto L_09B10070;
    case 862u: goto L_09B10074;
    case 863u: goto L_09B10078;
    case 864u: goto L_09B1007C;
    default:
        if (local_transfers == 0u) rt.unsupported(ctx.pc, 0u, "invalid internal function entry");
        else ctx.pc = local_pc;
        return;
    }
    }
L_09B0F300:
    rt.unsupported(0x09B0F304u, 0x09AADA80u, "control flow in delay slot"); return;
L_09B0F304:
    rt.unsupported(0x09B0F308u, 0x09AADA88u, "control flow in delay slot"); return;
L_09B0F308:
    rt.unsupported(0x09B0F30Cu, 0x09AADFE0u, "control flow in delay slot"); return;
L_09B0F30C:
    rt.unsupported(0x09B0F310u, 0x09AADA80u, "control flow in delay slot"); return;
L_09B0F310:
    rt.unsupported(0x09B0F314u, 0x09AADD84u, "control flow in delay slot"); return;
L_09B0F314:
    rt.unsupported(0x09B0F318u, 0x09AADF2Cu, "control flow in delay slot"); return;
L_09B0F318:
    rt.unsupported(0x09B0F31Cu, 0x09AADF50u, "control flow in delay slot"); return;
L_09B0F31C:
    rt.unsupported(0x09B0F320u, 0x09AADDC8u, "control flow in delay slot"); return;
L_09B0F320:
    rt.unsupported(0x09B0F324u, 0x09AADEF0u, "control flow in delay slot"); return;
L_09B0F324:
    rt.unsupported(0x09B0F328u, 0x09AAE034u, "control flow in delay slot"); return;
L_09B0F328:
    rt.unsupported(0x09B0F32Cu, 0x09AADF74u, "control flow in delay slot"); return;
L_09B0F32C:
    rt.unsupported(0x09B0F330u, 0x09AADF94u, "control flow in delay slot"); return;
L_09B0F330:
    rt.unsupported(0x09B0F334u, 0x09AADFA8u, "control flow in delay slot"); return;
L_09B0F334:
    rt.unsupported(0x09B0F338u, 0x09AAE054u, "control flow in delay slot"); return;
L_09B0F338:
    rt.unsupported(0x09B0F33Cu, 0x09AAE08Cu, "control flow in delay slot"); return;
L_09B0F33C:
    rt.unsupported(0x09B0F340u, 0x09AAE118u, "control flow in delay slot"); return;
L_09B0F340:
    rt.unsupported(0x09B0F344u, 0x09AAE178u, "control flow in delay slot"); return;
L_09B0F344:
    rt.unsupported(0x09B0F348u, 0x09AAE1D8u, "control flow in delay slot"); return;
L_09B0F348:
    rt.unsupported(0x09B0F34Cu, 0x09AAE234u, "control flow in delay slot"); return;
L_09B0F34C:
    rt.unsupported(0x09B0F350u, 0x09AAE294u, "control flow in delay slot"); return;
L_09B0F350:
    rt.unsupported(0x09B0F354u, 0x09AAE304u, "control flow in delay slot"); return;
L_09B0F354:
    rt.unsupported(0x09B0F358u, 0x09AAFE00u, "control flow in delay slot"); return;
L_09B0F358:
    rt.unsupported(0x09B0F35Cu, 0x09AAFE00u, "control flow in delay slot"); return;
L_09B0F35C:
    rt.unsupported(0x09B0F360u, 0x09AAFD28u, "control flow in delay slot"); return;
L_09B0F360:
    rt.unsupported(0x09B0F364u, 0x09AAFD38u, "control flow in delay slot"); return;
L_09B0F364:
    rt.unsupported(0x09B0F368u, 0x09AAFD48u, "control flow in delay slot"); return;
L_09B0F368:
    rt.unsupported(0x09B0F36Cu, 0x09AAFD58u, "control flow in delay slot"); return;
L_09B0F36C:
    rt.unsupported(0x09B0F370u, 0x09AAFE00u, "control flow in delay slot"); return;
L_09B0F370:
    rt.unsupported(0x09B0F374u, 0x09AAFD68u, "control flow in delay slot"); return;
L_09B0F374:
    rt.unsupported(0x09B0F378u, 0x09AAFD78u, "control flow in delay slot"); return;
L_09B0F378:
    rt.unsupported(0x09B0F37Cu, 0x09AAFD88u, "control flow in delay slot"); return;
L_09B0F37C:
    rt.unsupported(0x09B0F380u, 0x09AAFD98u, "control flow in delay slot"); return;
L_09B0F380:
    rt.unsupported(0x09B0F384u, 0x09AAFDA8u, "control flow in delay slot"); return;
L_09B0F384:
    rt.unsupported(0x09B0F388u, 0x09AAFE00u, "control flow in delay slot"); return;
L_09B0F388:
    rt.unsupported(0x09B0F38Cu, 0x09AAFDB8u, "control flow in delay slot"); return;
L_09B0F38C:
    rt.unsupported(0x09B0F390u, 0x09AAFDC8u, "control flow in delay slot"); return;
L_09B0F390:
    rt.unsupported(0x09B0F394u, 0x09AAFDD8u, "control flow in delay slot"); return;
L_09B0F394:
    rt.unsupported(0x09B0F398u, 0x09AAFDE8u, "control flow in delay slot"); return;
L_09B0F398:
    rt.unsupported(0x09B0F39Cu, 0x09AAFDF8u, "control flow in delay slot"); return;
L_09B0F39C:
    rt.unsupported(0x09B0F3A0u, 0x09AB0954u, "control flow in delay slot"); return;
L_09B0F3A0:
    rt.unsupported(0x09B0F3A4u, 0x09AB0954u, "control flow in delay slot"); return;
L_09B0F3A4:
    rt.unsupported(0x09B0F3A8u, 0x09AB0954u, "control flow in delay slot"); return;
L_09B0F3A8:
    rt.unsupported(0x09B0F3ACu, 0x09AB0954u, "control flow in delay slot"); return;
L_09B0F3AC:
    rt.unsupported(0x09B0F3B0u, 0x09AB0954u, "control flow in delay slot"); return;
L_09B0F3B0:
    rt.unsupported(0x09B0F3B4u, 0x09AB095Cu, "control flow in delay slot"); return;
L_09B0F3B4:
    rt.unsupported(0x09B0F3B8u, 0x09AB095Cu, "control flow in delay slot"); return;
L_09B0F3B8:
    rt.unsupported(0x09B0F3BCu, 0x09AB095Cu, "control flow in delay slot"); return;
L_09B0F3BC:
    rt.unsupported(0x09B0F3C0u, 0x09AB095Cu, "control flow in delay slot"); return;
L_09B0F3C0:
    rt.unsupported(0x09B0F3C4u, 0x09AB095Cu, "control flow in delay slot"); return;
L_09B0F3C4:
    rt.unsupported(0x09B0F3C8u, 0x09AB095Cu, "control flow in delay slot"); return;
L_09B0F3C8:
    rt.unsupported(0x09B0F3CCu, 0x09AB095Cu, "control flow in delay slot"); return;
L_09B0F3CC:
    rt.unsupported(0x09B0F3D0u, 0x09AB0954u, "control flow in delay slot"); return;
L_09B0F3D0:
    rt.unsupported(0x09B0F3D4u, 0x09AB095Cu, "control flow in delay slot"); return;
L_09B0F3D4:
    rt.unsupported(0x09B0F3D8u, 0x09AB095Cu, "control flow in delay slot"); return;
L_09B0F3D8:
    rt.unsupported(0x09B0F3DCu, 0x09AB0970u, "control flow in delay slot"); return;
L_09B0F3DC:
    rt.unsupported(0x09B0F3E0u, 0x09AB0994u, "control flow in delay slot"); return;
L_09B0F3E0:
    rt.unsupported(0x09B0F3E4u, 0x09AB09B8u, "control flow in delay slot"); return;
L_09B0F3E4:
    rt.unsupported(0x09B0F3E8u, 0x09AB0994u, "control flow in delay slot"); return;
L_09B0F3E8:
    rt.unsupported(0x09B0F3ECu, 0x09AB0994u, "control flow in delay slot"); return;
L_09B0F3EC:
    rt.unsupported(0x09B0F3F0u, 0x09AB0994u, "control flow in delay slot"); return;
L_09B0F3F0:
    rt.unsupported(0x09B0F3F4u, 0x09AB0994u, "control flow in delay slot"); return;
L_09B0F3F4:
    rt.unsupported(0x09B0F3F8u, 0x09AB0994u, "control flow in delay slot"); return;
L_09B0F3F8:
    rt.unsupported(0x09B0F3FCu, 0x09AB0994u, "control flow in delay slot"); return;
L_09B0F3FC:
    rt.unsupported(0x09B0F400u, 0x09AB0994u, "control flow in delay slot"); return;
L_09B0F400:
    rt.unsupported(0x09B0F404u, 0x09AB0994u, "control flow in delay slot"); return;
L_09B0F404:
    rt.unsupported(0x09B0F408u, 0x09AB0994u, "control flow in delay slot"); return;
L_09B0F408:
    rt.unsupported(0x09B0F40Cu, 0x09AB0994u, "control flow in delay slot"); return;
L_09B0F40C:
    rt.unsupported(0x09B0F410u, 0x09AB13E4u, "control flow in delay slot"); return;
L_09B0F410:
    rt.unsupported(0x09B0F414u, 0x09AB13E4u, "control flow in delay slot"); return;
L_09B0F414:
    rt.unsupported(0x09B0F418u, 0x09AB13E4u, "control flow in delay slot"); return;
L_09B0F418:
    rt.unsupported(0x09B0F41Cu, 0x09AB13E4u, "control flow in delay slot"); return;
L_09B0F41C:
    rt.unsupported(0x09B0F420u, 0x09AB13E4u, "control flow in delay slot"); return;
L_09B0F420:
    rt.unsupported(0x09B0F424u, 0x09AB13ECu, "control flow in delay slot"); return;
L_09B0F424:
    rt.unsupported(0x09B0F428u, 0x09AB13ECu, "control flow in delay slot"); return;
L_09B0F428:
    rt.unsupported(0x09B0F42Cu, 0x09AB13ECu, "control flow in delay slot"); return;
L_09B0F42C:
    rt.unsupported(0x09B0F430u, 0x09AB13ECu, "control flow in delay slot"); return;
L_09B0F430:
    rt.unsupported(0x09B0F434u, 0x09AB13ECu, "control flow in delay slot"); return;
L_09B0F434:
    rt.unsupported(0x09B0F438u, 0x09AB13ECu, "control flow in delay slot"); return;
L_09B0F438:
    rt.unsupported(0x09B0F43Cu, 0x09AB13ECu, "control flow in delay slot"); return;
L_09B0F43C:
    rt.unsupported(0x09B0F440u, 0x09AB13E4u, "control flow in delay slot"); return;
L_09B0F440:
    rt.unsupported(0x09B0F444u, 0x09AB13ECu, "control flow in delay slot"); return;
L_09B0F444:
    rt.unsupported(0x09B0F448u, 0x09AB13ECu, "control flow in delay slot"); return;
L_09B0F448:
    rt.unsupported(0x09B0F44Cu, 0x09AB1400u, "control flow in delay slot"); return;
L_09B0F44C:
    rt.unsupported(0x09B0F450u, 0x09AB1424u, "control flow in delay slot"); return;
L_09B0F450:
    rt.unsupported(0x09B0F454u, 0x09AB1448u, "control flow in delay slot"); return;
L_09B0F454:
    rt.unsupported(0x09B0F458u, 0x09AB1424u, "control flow in delay slot"); return;
L_09B0F458:
    rt.unsupported(0x09B0F45Cu, 0x09AB1424u, "control flow in delay slot"); return;
L_09B0F45C:
    rt.unsupported(0x09B0F460u, 0x09AB1424u, "control flow in delay slot"); return;
L_09B0F460:
    rt.unsupported(0x09B0F464u, 0x09AB1424u, "control flow in delay slot"); return;
L_09B0F464:
    rt.unsupported(0x09B0F468u, 0x09AB1424u, "control flow in delay slot"); return;
L_09B0F468:
    rt.unsupported(0x09B0F46Cu, 0x09AB1424u, "control flow in delay slot"); return;
L_09B0F46C:
    rt.unsupported(0x09B0F470u, 0x09AB1424u, "control flow in delay slot"); return;
L_09B0F470:
    rt.unsupported(0x09B0F474u, 0x09AB1424u, "control flow in delay slot"); return;
L_09B0F474:
    rt.unsupported(0x09B0F478u, 0x09AB1424u, "control flow in delay slot"); return;
L_09B0F478:
    rt.unsupported(0x09B0F47Cu, 0x09AB1424u, "control flow in delay slot"); return;
L_09B0F47C:
    rt.unsupported(0x09B0F480u, 0x09AB1B8Cu, "control flow in delay slot"); return;
L_09B0F480:
    rt.unsupported(0x09B0F484u, 0x09AB1B9Cu, "control flow in delay slot"); return;
L_09B0F484:
    rt.unsupported(0x09B0F488u, 0x09AB1BACu, "control flow in delay slot"); return;
L_09B0F488:
    rt.unsupported(0x09B0F48Cu, 0x09AB1BBCu, "control flow in delay slot"); return;
L_09B0F48C:
    rt.unsupported(0x09B0F490u, 0x09AB1BCCu, "control flow in delay slot"); return;
L_09B0F490:
    rt.unsupported(0x09B0F494u, 0x09AB1BDCu, "control flow in delay slot"); return;
L_09B0F494:
    rt.unsupported(0x09B0F498u, 0x09AB1BECu, "control flow in delay slot"); return;
L_09B0F498:
    rt.unsupported(0x09B0F49Cu, 0x09AB1BFCu, "control flow in delay slot"); return;
L_09B0F49C:
    rt.unsupported(0x09B0F4A0u, 0x09AB1C0Cu, "control flow in delay slot"); return;
L_09B0F4A0:
    // nop
    ctx.pc = 0x06AC7030u; (void)rt.invoke_chained_call(ctx, &aot_mem); return;
L_09B0F4A4:
    // nop
    goto L_09B0F4A8;
L_09B0F4A8:
    rt.unsupported(0x09B0F4ACu, 0x09AB2274u, "control flow in delay slot"); return;
L_09B0F4AC:
    rt.unsupported(0x09B0F4B0u, 0x09AB2284u, "control flow in delay slot"); return;
L_09B0F4B0:
    rt.unsupported(0x09B0F4B4u, 0x09AB2294u, "control flow in delay slot"); return;
L_09B0F4B4:
    rt.unsupported(0x09B0F4B8u, 0x09AB22A4u, "control flow in delay slot"); return;
L_09B0F4B8:
    rt.unsupported(0x09B0F4BCu, 0x09AB22B4u, "control flow in delay slot"); return;
L_09B0F4BC:
    rt.unsupported(0x09B0F4C0u, 0x09AB22C4u, "control flow in delay slot"); return;
L_09B0F4C0:
    rt.unsupported(0x09B0F4C4u, 0x09AB22D4u, "control flow in delay slot"); return;
L_09B0F4C4:
    rt.unsupported(0x09B0F4C8u, 0x09AB22E4u, "control flow in delay slot"); return;
L_09B0F4C8:
    rt.unsupported(0x09B0F4CCu, 0x09AB22F4u, "control flow in delay slot"); return;
L_09B0F4CC:
    rt.unsupported(0x09B0F4D0u, 0x09AB2304u, "control flow in delay slot"); return;
L_09B0F4D0:
    rt.unsupported(0x09B0F4D4u, 0x09AB2314u, "control flow in delay slot"); return;
L_09B0F4D4:
    rt.unsupported(0x09B0F4D8u, 0x09AB2324u, "control flow in delay slot"); return;
L_09B0F4D8:
    rt.unsupported(0x09B0F4DCu, 0x09AB2334u, "control flow in delay slot"); return;
L_09B0F4DC:
    rt.unsupported(0x09B0F4E0u, 0x09AB2344u, "control flow in delay slot"); return;
L_09B0F4E0:
    rt.unsupported(0x09B0F4E4u, 0x09AB2354u, "control flow in delay slot"); return;
L_09B0F4E4:
    rt.unsupported(0x09B0F4E8u, 0x09AB2364u, "control flow in delay slot"); return;
L_09B0F4E8:
    rt.unsupported(0x09B0F4ECu, 0x09AB2374u, "control flow in delay slot"); return;
L_09B0F4EC:
    rt.unsupported(0x09B0F4F0u, 0x09AB2384u, "control flow in delay slot"); return;
L_09B0F4F0:
    rt.unsupported(0x09B0F4F4u, 0x09AB2394u, "control flow in delay slot"); return;
L_09B0F4F4:
    rt.unsupported(0x09B0F4F8u, 0x09AB23A4u, "control flow in delay slot"); return;
L_09B0F4F8:
    rt.unsupported(0x09B0F4FCu, 0x09AB23B4u, "control flow in delay slot"); return;
L_09B0F4FC:
    rt.unsupported(0x09B0F500u, 0x09AB23C4u, "control flow in delay slot"); return;
L_09B0F500:
    rt.unsupported(0x09B0F504u, 0x09AB23D4u, "control flow in delay slot"); return;
L_09B0F504:
    rt.unsupported(0x09B0F508u, 0x09AB23E4u, "control flow in delay slot"); return;
L_09B0F508:
    // nop
    ctx.pc = 0x06AC8F90u; (void)rt.invoke_chained_call(ctx, &aot_mem); return;
L_09B0F50C:
    // nop
    goto L_09B0F510;
L_09B0F510:
    rt.unsupported(0x09B0F514u, 0x09AB552Cu, "control flow in delay slot"); return;
L_09B0F514:
    rt.unsupported(0x09B0F518u, 0x09AB552Cu, "control flow in delay slot"); return;
L_09B0F518:
    rt.unsupported(0x09B0F51Cu, 0x09AB552Cu, "control flow in delay slot"); return;
L_09B0F51C:
    rt.unsupported(0x09B0F520u, 0x09AB552Cu, "control flow in delay slot"); return;
L_09B0F520:
    rt.unsupported(0x09B0F524u, 0x09AB55FCu, "control flow in delay slot"); return;
L_09B0F524:
    rt.unsupported(0x09B0F528u, 0x09AB55FCu, "control flow in delay slot"); return;
L_09B0F528:
    // nop
    ctx.pc = 0x06AD57F0u; (void)rt.invoke_chained_call(ctx, &aot_mem); return;
L_09B0F52C:
    // nop
    goto L_09B0F530;
L_09B0F530:
    rt.unsupported(0x09B0F534u, 0x09AB6474u, "control flow in delay slot"); return;
L_09B0F534:
    rt.unsupported(0x09B0F538u, 0x09AB6354u, "control flow in delay slot"); return;
L_09B0F538:
    rt.unsupported(0x09B0F53Cu, 0x09AB6354u, "control flow in delay slot"); return;
L_09B0F53C:
    rt.unsupported(0x09B0F540u, 0x09AB6354u, "control flow in delay slot"); return;
L_09B0F540:
    rt.unsupported(0x09B0F544u, 0x09AB636Cu, "control flow in delay slot"); return;
L_09B0F544:
    rt.unsupported(0x09B0F548u, 0x09AB636Cu, "control flow in delay slot"); return;
L_09B0F548:
    // nop
    ctx.pc = 0x06AD8DB0u; (void)rt.invoke_chained_call(ctx, &aot_mem); return;
L_09B0F54C:
    // nop
    goto L_09B0F550;
L_09B0F550:
    rt.unsupported(0x09B0F554u, 0x09AB5EA8u, "control flow in delay slot"); return;
L_09B0F554:
    rt.unsupported(0x09B0F558u, 0x09AB5EA8u, "control flow in delay slot"); return;
L_09B0F558:
    rt.unsupported(0x09B0F55Cu, 0x09AB5EA8u, "control flow in delay slot"); return;
L_09B0F55C:
    rt.unsupported(0x09B0F560u, 0x09AB5EA8u, "control flow in delay slot"); return;
L_09B0F560:
    rt.unsupported(0x09B0F564u, 0x09AB5FD4u, "control flow in delay slot"); return;
L_09B0F564:
    rt.unsupported(0x09B0F568u, 0x09AB5FD4u, "control flow in delay slot"); return;
L_09B0F568:
    // nop
    ctx.pc = 0x06AD7F50u; (void)rt.invoke_chained_call(ctx, &aot_mem); return;
L_09B0F56C:
    // nop
    goto L_09B0F570;
L_09B0F570:
    rt.unsupported(0x09B0F574u, 0x09AB75D0u, "control flow in delay slot"); return;
L_09B0F574:
    rt.unsupported(0x09B0F578u, 0x09AB767Cu, "control flow in delay slot"); return;
L_09B0F578:
    rt.unsupported(0x09B0F57Cu, 0x09AB79E0u, "control flow in delay slot"); return;
L_09B0F57C:
    rt.unsupported(0x09B0F580u, 0x09AB7E00u, "control flow in delay slot"); return;
L_09B0F580:
    rt.unsupported(0x09B0F584u, 0x09AB7EDCu, "control flow in delay slot"); return;
L_09B0F584:
    rt.unsupported(0x09B0F588u, 0x09AB7EDCu, "control flow in delay slot"); return;
L_09B0F588:
    // nop
    ctx.pc = 0x06ADFB70u; (void)rt.invoke_chained_call(ctx, &aot_mem); return;
L_09B0F58C:
    // nop
    goto L_09B0F590;
L_09B0F590:
    rt.unsupported(0x09B0F594u, 0x09AB8378u, "control flow in delay slot"); return;
L_09B0F594:
    rt.unsupported(0x09B0F598u, 0x09AB83A0u, "control flow in delay slot"); return;
L_09B0F598:
    rt.unsupported(0x09B0F59Cu, 0x09AB83A0u, "control flow in delay slot"); return;
L_09B0F59C:
    rt.unsupported(0x09B0F5A0u, 0x09AB83A0u, "control flow in delay slot"); return;
L_09B0F5A0:
    rt.unsupported(0x09B0F5A4u, 0x09AB8378u, "control flow in delay slot"); return;
L_09B0F5A4:
    rt.unsupported(0x09B0F5A8u, 0x09AB83A0u, "control flow in delay slot"); return;
L_09B0F5A8:
    rt.unsupported(0x09B0F5ACu, 0x09AB83A0u, "control flow in delay slot"); return;
L_09B0F5AC:
    rt.unsupported(0x09B0F5B0u, 0x09AB83A0u, "control flow in delay slot"); return;
L_09B0F5B0:
    rt.unsupported(0x09B0F5B4u, 0x09AB83A0u, "control flow in delay slot"); return;
L_09B0F5B4:
    rt.unsupported(0x09B0F5B8u, 0x09AB83A0u, "control flow in delay slot"); return;
L_09B0F5B8:
    // nop
    ctx.pc = 0x06AE0E80u; (void)rt.invoke_chained_call(ctx, &aot_mem); return;
L_09B0F5BC:
    // nop
    goto L_09B0F5C0;
L_09B0F5C0:
    rt.unsupported(0x09B0F5C4u, 0x09AB8D04u, "control flow in delay slot"); return;
L_09B0F5C4:
    rt.unsupported(0x09B0F5C8u, 0x09AB8D68u, "control flow in delay slot"); return;
L_09B0F5C8:
    rt.unsupported(0x09B0F5CCu, 0x09AB8DE8u, "control flow in delay slot"); return;
L_09B0F5CC:
    rt.unsupported(0x09B0F5D0u, 0x09AB8D2Cu, "control flow in delay slot"); return;
L_09B0F5D0:
    rt.unsupported(0x09B0F5D4u, 0x09AB8D2Cu, "control flow in delay slot"); return;
L_09B0F5D4:
    rt.unsupported(0x09B0F5D8u, 0x09AB8DE8u, "control flow in delay slot"); return;
L_09B0F5D8:
    rt.unsupported(0x09B0F5DCu, 0x09AB8DE8u, "control flow in delay slot"); return;
L_09B0F5DC:
    rt.unsupported(0x09B0F5E0u, 0x09AB8DA0u, "control flow in delay slot"); return;
L_09B0F5E0:
    // nop
    ctx.pc = 0x06AE3680u; (void)rt.invoke_chained_call(ctx, &aot_mem); return;
L_09B0F5E4:
    // nop
    goto L_09B0F5E8;
L_09B0F5E8:
    rt.unsupported(0x09B0F5ECu, 0x09ABB560u, "control flow in delay slot"); return;
L_09B0F5EC:
    rt.unsupported(0x09B0F5F0u, 0x09ABB560u, "control flow in delay slot"); return;
L_09B0F5F0:
    rt.unsupported(0x09B0F5F4u, 0x09ABB560u, "control flow in delay slot"); return;
L_09B0F5F4:
    rt.unsupported(0x09B0F5F8u, 0x09ABB560u, "control flow in delay slot"); return;
L_09B0F5F8:
    rt.unsupported(0x09B0F5FCu, 0x09ABB560u, "control flow in delay slot"); return;
L_09B0F5FC:
    rt.unsupported(0x09B0F600u, 0x09ABB564u, "control flow in delay slot"); return;
L_09B0F600:
    rt.unsupported(0x09B0F604u, 0x09ABB288u, "control flow in delay slot"); return;
L_09B0F604:
    rt.unsupported(0x09B0F608u, 0x09ABB2DCu, "control flow in delay slot"); return;
L_09B0F608:
    rt.unsupported(0x09B0F60Cu, 0x09ABB330u, "control flow in delay slot"); return;
L_09B0F60C:
    rt.unsupported(0x09B0F610u, 0x09ABB370u, "control flow in delay slot"); return;
L_09B0F610:
    rt.unsupported(0x09B0F614u, 0x09ABB39Cu, "control flow in delay slot"); return;
L_09B0F614:
    rt.unsupported(0x09B0F618u, 0x09ABB3DCu, "control flow in delay slot"); return;
L_09B0F618:
    rt.unsupported(0x09B0F61Cu, 0x09ABB418u, "control flow in delay slot"); return;
L_09B0F61C:
    rt.unsupported(0x09B0F620u, 0x09ABB444u, "control flow in delay slot"); return;
L_09B0F620:
    rt.unsupported(0x09B0F624u, 0x09ABB4A4u, "control flow in delay slot"); return;
L_09B0F624:
    rt.unsupported(0x09B0F628u, 0x09ABB504u, "control flow in delay slot"); return;
L_09B0F628:
    // nop
    ctx.pc = 0x06AED410u; (void)rt.invoke_chained_call(ctx, &aot_mem); return;
L_09B0F62C:
    // nop
    goto L_09B0F630;
L_09B0F630:
    rt.unsupported(0x09B0F634u, 0x09ABB138u, "control flow in delay slot"); return;
L_09B0F634:
    rt.unsupported(0x09B0F638u, 0x09ABB16Cu, "control flow in delay slot"); return;
L_09B0F638:
    rt.unsupported(0x09B0F63Cu, 0x09ABB190u, "control flow in delay slot"); return;
L_09B0F63C:
    rt.unsupported(0x09B0F640u, 0x09ABB1D0u, "control flow in delay slot"); return;
L_09B0F640:
    rt.unsupported(0x09B0F644u, 0x09ABB210u, "control flow in delay slot"); return;
L_09B0F644:
    rt.unsupported(0x09B0F648u, 0x09ABB564u, "control flow in delay slot"); return;
L_09B0F648:
    rt.unsupported(0x09B0F64Cu, 0x09ABB244u, "control flow in delay slot"); return;
L_09B0F64C:
    rt.unsupported(0x09B0F650u, 0x09ABB244u, "control flow in delay slot"); return;
L_09B0F650:
    rt.unsupported(0x09B0F654u, 0x09ABB244u, "control flow in delay slot"); return;
L_09B0F654:
    rt.unsupported(0x09B0F658u, 0x09ABB244u, "control flow in delay slot"); return;
L_09B0F658:
    rt.unsupported(0x09B0F65Cu, 0x09ABB244u, "control flow in delay slot"); return;
L_09B0F65C:
    rt.unsupported(0x09B0F660u, 0x09ABB244u, "control flow in delay slot"); return;
L_09B0F660:
    rt.unsupported(0x09B0F664u, 0x09ABB24Cu, "control flow in delay slot"); return;
L_09B0F664:
    rt.unsupported(0x09B0F668u, 0x09ABB24Cu, "control flow in delay slot"); return;
L_09B0F668:
    rt.unsupported(0x09B0F66Cu, 0x09ABB244u, "control flow in delay slot"); return;
L_09B0F66C:
    rt.unsupported(0x09B0F670u, 0x09ABB244u, "control flow in delay slot"); return;
L_09B0F670:
    // nop
    ctx.pc = 0x06AEC910u; (void)rt.invoke_chained_call(ctx, &aot_mem); return;
L_09B0F674:
    // nop
    goto L_09B0F678;
L_09B0F678:
    rt.unsupported(0x09B0F67Cu, 0x09ABB070u, "control flow in delay slot"); return;
L_09B0F67C:
    rt.unsupported(0x09B0F680u, 0x09ABB070u, "control flow in delay slot"); return;
L_09B0F680:
    rt.unsupported(0x09B0F684u, 0x09ABB070u, "control flow in delay slot"); return;
L_09B0F684:
    rt.unsupported(0x09B0F688u, 0x09ABB070u, "control flow in delay slot"); return;
L_09B0F688:
    rt.unsupported(0x09B0F68Cu, 0x09ABB070u, "control flow in delay slot"); return;
L_09B0F68C:
    rt.unsupported(0x09B0F690u, 0x09ABB080u, "control flow in delay slot"); return;
L_09B0F690:
    rt.unsupported(0x09B0F694u, 0x09ABB070u, "control flow in delay slot"); return;
L_09B0F694:
    rt.unsupported(0x09B0F698u, 0x09ABB070u, "control flow in delay slot"); return;
L_09B0F698:
    rt.unsupported(0x09B0F69Cu, 0x09ABB070u, "control flow in delay slot"); return;
L_09B0F69C:
    rt.unsupported(0x09B0F6A0u, 0x09ABB070u, "control flow in delay slot"); return;
L_09B0F6A0:
    rt.unsupported(0x09B0F6A4u, 0x09ABB070u, "control flow in delay slot"); return;
L_09B0F6A4:
    rt.unsupported(0x09B0F6A8u, 0x09ABB070u, "control flow in delay slot"); return;
L_09B0F6A8:
    rt.unsupported(0x09B0F6ACu, 0x09ABB078u, "control flow in delay slot"); return;
L_09B0F6AC:
    rt.unsupported(0x09B0F6B0u, 0x09ABB078u, "control flow in delay slot"); return;
L_09B0F6B0:
    rt.unsupported(0x09B0F6B4u, 0x09ABB070u, "control flow in delay slot"); return;
L_09B0F6B4:
    rt.unsupported(0x09B0F6B8u, 0x09ABB070u, "control flow in delay slot"); return;
L_09B0F6B8:
    // nop
    ctx.pc = 0x06AEC1C0u; (void)rt.invoke_chained_call(ctx, &aot_mem); return;
L_09B0F6BC:
    // nop
    goto L_09B0F6C0;
L_09B0F6C0:
    rt.unsupported(0x09B0F6C4u, 0x09ABBC14u, "control flow in delay slot"); return;
L_09B0F6C4:
    rt.unsupported(0x09B0F6C8u, 0x09ABBC1Cu, "control flow in delay slot"); return;
L_09B0F6C8:
    rt.unsupported(0x09B0F6CCu, 0x09ABBC24u, "control flow in delay slot"); return;
L_09B0F6CC:
    rt.unsupported(0x09B0F6D0u, 0x09ABBC08u, "control flow in delay slot"); return;
L_09B0F6D0:
    rt.unsupported(0x09B0F6D4u, 0x09ABBC08u, "control flow in delay slot"); return;
L_09B0F6D4:
    rt.unsupported(0x09B0F6D8u, 0x09ABBC2Cu, "control flow in delay slot"); return;
L_09B0F6D8:
    rt.unsupported(0x09B0F6DCu, 0x09ABBC34u, "control flow in delay slot"); return;
L_09B0F6DC:
    rt.unsupported(0x09B0F6E0u, 0x09ABBC3Cu, "control flow in delay slot"); return;
L_09B0F6E0:
    rt.unsupported(0x09B0F6E4u, 0x09ABBC44u, "control flow in delay slot"); return;
L_09B0F6E4:
    rt.unsupported(0x09B0F6E8u, 0x09ABBC4Cu, "control flow in delay slot"); return;
L_09B0F6E8:
    rt.unsupported(0x09B0F6ECu, 0x09ABBC54u, "control flow in delay slot"); return;
L_09B0F6EC:
    rt.unsupported(0x09B0F6F0u, 0x09ABD03Cu, "control flow in delay slot"); return;
L_09B0F6F0:
    rt.unsupported(0x09B0F6F4u, 0x09ABD044u, "control flow in delay slot"); return;
L_09B0F6F4:
    rt.unsupported(0x09B0F6F8u, 0x09ABD04Cu, "control flow in delay slot"); return;
L_09B0F6F8:
    rt.unsupported(0x09B0F6FCu, 0x09ABD054u, "control flow in delay slot"); return;
L_09B0F6FC:
    rt.unsupported(0x09B0F700u, 0x09ABD05Cu, "control flow in delay slot"); return;
L_09B0F700:
    rt.unsupported(0x09B0F704u, 0x09ABD064u, "control flow in delay slot"); return;
L_09B0F704:
    rt.unsupported(0x09B0F708u, 0x09ABEC1Cu, "control flow in delay slot"); return;
L_09B0F708:
    rt.unsupported(0x09B0F70Cu, 0x09ABEC0Cu, "control flow in delay slot"); return;
L_09B0F70C:
    rt.unsupported(0x09B0F710u, 0x09ABEC2Cu, "control flow in delay slot"); return;
L_09B0F710:
    rt.unsupported(0x09B0F714u, 0x09ABEC2Cu, "control flow in delay slot"); return;
L_09B0F714:
    rt.unsupported(0x09B0F718u, 0x09ABEBFCu, "control flow in delay slot"); return;
L_09B0F718:
    rt.unsupported(0x09B0F71Cu, 0x09ABEC3Cu, "control flow in delay slot"); return;
L_09B0F71C:
    rt.unsupported(0x09B0F720u, 0x09ABEC6Cu, "control flow in delay slot"); return;
L_09B0F720:
    rt.unsupported(0x09B0F724u, 0x09ABEC4Cu, "control flow in delay slot"); return;
L_09B0F724:
    rt.unsupported(0x09B0F728u, 0x09ABEC5Cu, "control flow in delay slot"); return;
L_09B0F728:
    // nop
    ctx.pc = 0x06AFB170u; (void)rt.invoke_chained_call(ctx, &aot_mem); return;
L_09B0F72C:
    // nop
    goto L_09B0F730;
L_09B0F730:
    rt.unsupported(0x09B0F734u, 0x09ABEBC0u, "control flow in delay slot"); return;
L_09B0F734:
    rt.unsupported(0x09B0F738u, 0x09ABEBC0u, "control flow in delay slot"); return;
L_09B0F738:
    rt.unsupported(0x09B0F73Cu, 0x09ABEBC0u, "control flow in delay slot"); return;
L_09B0F73C:
    rt.unsupported(0x09B0F740u, 0x09ABEBC0u, "control flow in delay slot"); return;
L_09B0F740:
    rt.unsupported(0x09B0F744u, 0x09ABEBC0u, "control flow in delay slot"); return;
L_09B0F744:
    rt.unsupported(0x09B0F748u, 0x09ABEE6Cu, "control flow in delay slot"); return;
L_09B0F748:
    rt.unsupported(0x09B0F74Cu, 0x09ABEE7Cu, "control flow in delay slot"); return;
L_09B0F74C:
    rt.unsupported(0x09B0F750u, 0x09ABEDD8u, "control flow in delay slot"); return;
L_09B0F750:
    rt.unsupported(0x09B0F754u, 0x09ABEDECu, "control flow in delay slot"); return;
L_09B0F754:
    rt.unsupported(0x09B0F758u, 0x09ABEDFCu, "control flow in delay slot"); return;
L_09B0F758:
    rt.unsupported(0x09B0F75Cu, 0x09ABEDD8u, "control flow in delay slot"); return;
L_09B0F75C:
    rt.unsupported(0x09B0F760u, 0x09ABEDD8u, "control flow in delay slot"); return;
L_09B0F760:
    rt.unsupported(0x09B0F764u, 0x09ABEDD8u, "control flow in delay slot"); return;
L_09B0F764:
    rt.unsupported(0x09B0F768u, 0x09ABEE0Cu, "control flow in delay slot"); return;
L_09B0F768:
    rt.unsupported(0x09B0F76Cu, 0x09ABEE1Cu, "control flow in delay slot"); return;
L_09B0F76C:
    rt.unsupported(0x09B0F770u, 0x09ABEE2Cu, "control flow in delay slot"); return;
L_09B0F770:
    rt.unsupported(0x09B0F774u, 0x09ABEE3Cu, "control flow in delay slot"); return;
L_09B0F774:
    rt.unsupported(0x09B0F778u, 0x09ABEE4Cu, "control flow in delay slot"); return;
L_09B0F778:
    rt.unsupported(0x09B0F77Cu, 0x09ABEE5Cu, "control flow in delay slot"); return;
L_09B0F77C:
    rt.unsupported(0x09B0F780u, 0x09ABEE8Cu, "control flow in delay slot"); return;
L_09B0F780:
    // nop
    ctx.pc = 0x06AFBA30u; (void)rt.invoke_chained_call(ctx, &aot_mem); return;
L_09B0F784:
    // nop
    goto L_09B0F788;
L_09B0F788:
    rt.unsupported(0x09B0F78Cu, 0x09ABFBB8u, "control flow in delay slot"); return;
L_09B0F78C:
    rt.unsupported(0x09B0F790u, 0x09ABFC0Cu, "control flow in delay slot"); return;
L_09B0F790:
    rt.unsupported(0x09B0F794u, 0x09ABFC38u, "control flow in delay slot"); return;
L_09B0F794:
    rt.unsupported(0x09B0F798u, 0x09ABFC78u, "control flow in delay slot"); return;
L_09B0F798:
    rt.unsupported(0x09B0F79Cu, 0x09ABFCA8u, "control flow in delay slot"); return;
L_09B0F79C:
    rt.unsupported(0x09B0F7A0u, 0x09ABFF30u, "control flow in delay slot"); return;
L_09B0F7A0:
    rt.unsupported(0x09B0F7A4u, 0x09ABFF70u, "control flow in delay slot"); return;
L_09B0F7A4:
    rt.unsupported(0x09B0F7A8u, 0x09ABFF9Cu, "control flow in delay slot"); return;
L_09B0F7A8:
    rt.unsupported(0x09B0F7ACu, 0x09ABFFD8u, "control flow in delay slot"); return;
L_09B0F7AC:
    rt.unsupported(0x09B0F7B0u, 0x09AC0014u, "control flow in delay slot"); return;
L_09B0F7B0:
    rt.unsupported(0x09B0F7B4u, 0x09AC0054u, "control flow in delay slot"); return;
L_09B0F7B4:
    rt.unsupported(0x09B0F7B8u, 0x09AC0C60u, "control flow in delay slot"); return;
L_09B0F7B8:
    rt.unsupported(0x09B0F7BCu, 0x09AC0CB0u, "control flow in delay slot"); return;
L_09B0F7BC:
    rt.unsupported(0x09B0F7C0u, 0x09AC0D04u, "control flow in delay slot"); return;
L_09B0F7C0:
    rt.unsupported(0x09B0F7C4u, 0x09AC0D40u, "control flow in delay slot"); return;
L_09B0F7C4:
    rt.unsupported(0x09B0F7C8u, 0x09AC0D84u, "control flow in delay slot"); return;
L_09B0F7C8:
    rt.unsupported(0x09B0F7CCu, 0x09AC0DB4u, "control flow in delay slot"); return;
L_09B0F7CC:
    rt.unsupported(0x09B0F7D0u, 0x09AC0DF4u, "control flow in delay slot"); return;
L_09B0F7D0:
    // nop
    ctx.pc = 0x06B037D0u; (void)rt.invoke_chained_call(ctx, &aot_mem); return;
L_09B0F7D4:
    // nop
    goto L_09B0F7D8;
L_09B0F7D8:
    rt.unsupported(0x09B0F7DCu, 0x09AC2F40u, "control flow in delay slot"); return;
L_09B0F7DC:
    rt.unsupported(0x09B0F7E0u, 0x09AC2F60u, "control flow in delay slot"); return;
L_09B0F7E0:
    rt.unsupported(0x09B0F7E4u, 0x09AC2F78u, "control flow in delay slot"); return;
L_09B0F7E4:
    rt.unsupported(0x09B0F7E8u, 0x09AC2FB8u, "control flow in delay slot"); return;
L_09B0F7E8:
    rt.unsupported(0x09B0F7ECu, 0x09AC3280u, "control flow in delay slot"); return;
L_09B0F7EC:
    rt.unsupported(0x09B0F7F0u, 0x09AC32D0u, "control flow in delay slot"); return;
L_09B0F7F0:
    rt.unsupported(0x09B0F7F4u, 0x09AC334Cu, "control flow in delay slot"); return;
L_09B0F7F4:
    rt.unsupported(0x09B0F7F8u, 0x09AC3388u, "control flow in delay slot"); return;
L_09B0F7F8:
    rt.unsupported(0x09B0F7FCu, 0x09AC33E8u, "control flow in delay slot"); return;
L_09B0F7FC:
    rt.unsupported(0x09B0F800u, 0x09AC34C0u, "control flow in delay slot"); return;
L_09B0F800:
    rt.unsupported(0x09B0F804u, 0x09AC3530u, "control flow in delay slot"); return;
L_09B0F804:
    rt.unsupported(0x09B0F808u, 0x09AC3578u, "control flow in delay slot"); return;
L_09B0F808:
    rt.unsupported(0x09B0F80Cu, 0x09AC35B8u, "control flow in delay slot"); return;
L_09B0F80C:
    rt.unsupported(0x09B0F810u, 0x09AC35ECu, "control flow in delay slot"); return;
L_09B0F810:
    rt.unsupported(0x09B0F814u, 0x09AC3614u, "control flow in delay slot"); return;
L_09B0F814:
    rt.unsupported(0x09B0F818u, 0x09AC3724u, "control flow in delay slot"); return;
L_09B0F818:
    rt.unsupported(0x09B0F81Cu, 0x09AC3724u, "control flow in delay slot"); return;
L_09B0F81C:
    rt.unsupported(0x09B0F820u, 0x09AC3724u, "control flow in delay slot"); return;
L_09B0F820:
    rt.unsupported(0x09B0F824u, 0x09AC3724u, "control flow in delay slot"); return;
L_09B0F824:
    rt.unsupported(0x09B0F828u, 0x09AC3724u, "control flow in delay slot"); return;
L_09B0F828:
    rt.unsupported(0x09B0F82Cu, 0x09AC3724u, "control flow in delay slot"); return;
L_09B0F82C:
    rt.unsupported(0x09B0F830u, 0x09AC3724u, "control flow in delay slot"); return;
L_09B0F830:
    rt.unsupported(0x09B0F834u, 0x09AC3724u, "control flow in delay slot"); return;
L_09B0F834:
    rt.unsupported(0x09B0F838u, 0x09AC3724u, "control flow in delay slot"); return;
L_09B0F838:
    rt.unsupported(0x09B0F83Cu, 0x09AC3634u, "control flow in delay slot"); return;
L_09B0F83C:
    rt.unsupported(0x09B0F840u, 0x09AC36D0u, "control flow in delay slot"); return;
L_09B0F840:
    rt.unsupported(0x09B0F844u, 0x09AC32D0u, "control flow in delay slot"); return;
L_09B0F844:
    rt.unsupported(0x09B0F848u, 0x09AC334Cu, "control flow in delay slot"); return;
L_09B0F848:
    rt.unsupported(0x09B0F84Cu, 0x09AC3388u, "control flow in delay slot"); return;
L_09B0F84C:
    rt.unsupported(0x09B0F850u, 0x09AC33E8u, "control flow in delay slot"); return;
L_09B0F850:
    // nop
    ctx.pc = 0x06B0CFA0u; (void)rt.invoke_chained_call(ctx, &aot_mem); return;
L_09B0F854:
    // nop
    goto L_09B0F858;
L_09B0F858:
    rt.unsupported(0x09B0F85Cu, 0x09AC385Cu, "control flow in delay slot"); return;
L_09B0F85C:
    rt.unsupported(0x09B0F860u, 0x09AC3880u, "control flow in delay slot"); return;
L_09B0F860:
    rt.unsupported(0x09B0F864u, 0x09AC38B4u, "control flow in delay slot"); return;
L_09B0F864:
    rt.unsupported(0x09B0F868u, 0x09AC392Cu, "control flow in delay slot"); return;
L_09B0F868:
    rt.unsupported(0x09B0F86Cu, 0x09AC398Cu, "control flow in delay slot"); return;
L_09B0F86C:
    rt.unsupported(0x09B0F870u, 0x09AC39D8u, "control flow in delay slot"); return;
L_09B0F870:
    rt.unsupported(0x09B0F874u, 0x09AC39F0u, "control flow in delay slot"); return;
L_09B0F874:
    rt.unsupported(0x09B0F878u, 0x09AC3A10u, "control flow in delay slot"); return;
L_09B0F878:
    rt.unsupported(0x09B0F87Cu, 0x09AC3A58u, "control flow in delay slot"); return;
L_09B0F87C:
    rt.unsupported(0x09B0F880u, 0x09AC3D08u, "control flow in delay slot"); return;
L_09B0F880:
    rt.unsupported(0x09B0F884u, 0x09AC3D30u, "control flow in delay slot"); return;
L_09B0F884:
    rt.unsupported(0x09B0F888u, 0x09AC3D4Cu, "control flow in delay slot"); return;
L_09B0F888:
    rt.unsupported(0x09B0F88Cu, 0x09AC3D94u, "control flow in delay slot"); return;
L_09B0F88C:
    rt.unsupported(0x09B0F890u, 0x09AC4048u, "control flow in delay slot"); return;
L_09B0F890:
    rt.unsupported(0x09B0F894u, 0x09AC4070u, "control flow in delay slot"); return;
L_09B0F894:
    rt.unsupported(0x09B0F898u, 0x09AC409Cu, "control flow in delay slot"); return;
L_09B0F898:
    rt.unsupported(0x09B0F89Cu, 0x09AC40E4u, "control flow in delay slot"); return;
L_09B0F89C:
    rt.unsupported(0x09B0F8A0u, 0x09AC4394u, "control flow in delay slot"); return;
L_09B0F8A0:
    rt.unsupported(0x09B0F8A4u, 0x09AC43BCu, "control flow in delay slot"); return;
L_09B0F8A4:
    rt.unsupported(0x09B0F8A8u, 0x09AC4A28u, "control flow in delay slot"); return;
L_09B0F8A8:
    rt.unsupported(0x09B0F8ACu, 0x09AC4A28u, "control flow in delay slot"); return;
L_09B0F8AC:
    rt.unsupported(0x09B0F8B0u, 0x09AC4A28u, "control flow in delay slot"); return;
L_09B0F8B0:
    rt.unsupported(0x09B0F8B4u, 0x09AC4A28u, "control flow in delay slot"); return;
L_09B0F8B4:
    rt.unsupported(0x09B0F8B8u, 0x09AC43D8u, "control flow in delay slot"); return;
L_09B0F8B8:
    rt.unsupported(0x09B0F8BCu, 0x09AC4420u, "control flow in delay slot"); return;
L_09B0F8BC:
    rt.unsupported(0x09B0F8C0u, 0x09AC46D0u, "control flow in delay slot"); return;
L_09B0F8C0:
    rt.unsupported(0x09B0F8C4u, 0x09AC4944u, "control flow in delay slot"); return;
L_09B0F8C4:
    rt.unsupported(0x09B0F8C8u, 0x09AC497Cu, "control flow in delay slot"); return;
L_09B0F8C8:
    rt.unsupported(0x09B0F8CCu, 0x09AC49A4u, "control flow in delay slot"); return;
L_09B0F8CC:
    rt.unsupported(0x09B0F8D0u, 0x09AC49F4u, "control flow in delay slot"); return;
L_09B0F8D0:
    rt.unsupported(0x09B0F8D4u, 0x09AC4A1Cu, "control flow in delay slot"); return;
L_09B0F8D4:
    rt.unsupported(0x09B0F8D8u, 0x09AC5260u, "control flow in delay slot"); return;
L_09B0F8D8:
    rt.unsupported(0x09B0F8DCu, 0x09AC52D0u, "control flow in delay slot"); return;
L_09B0F8DC:
    rt.unsupported(0x09B0F8E0u, 0x09AC5324u, "control flow in delay slot"); return;
L_09B0F8E0:
    rt.unsupported(0x09B0F8E4u, 0x09AC5374u, "control flow in delay slot"); return;
L_09B0F8E4:
    rt.unsupported(0x09B0F8E8u, 0x09AC53B8u, "control flow in delay slot"); return;
L_09B0F8E8:
    rt.unsupported(0x09B0F8ECu, 0x09AC56D0u, "control flow in delay slot"); return;
L_09B0F8EC:
    rt.unsupported(0x09B0F8F0u, 0x09AC5744u, "control flow in delay slot"); return;
L_09B0F8F0:
    rt.unsupported(0x09B0F8F4u, 0x09AC5830u, "control flow in delay slot"); return;
L_09B0F8F4:
    rt.unsupported(0x09B0F8F8u, 0x09AC5884u, "control flow in delay slot"); return;
L_09B0F8F8:
    rt.unsupported(0x09B0F8FCu, 0x09AC58CCu, "control flow in delay slot"); return;
L_09B0F8FC:
    rt.unsupported(0x09B0F900u, 0x09AC5BA0u, "control flow in delay slot"); return;
L_09B0F900:
    rt.unsupported(0x09B0F904u, 0x09AC5BD0u, "control flow in delay slot"); return;
L_09B0F904:
    rt.unsupported(0x09B0F908u, 0x09AC607Cu, "control flow in delay slot"); return;
L_09B0F908:
    rt.unsupported(0x09B0F90Cu, 0x09AC60ACu, "control flow in delay slot"); return;
L_09B0F90C:
    rt.unsupported(0x09B0F910u, 0x09AC6364u, "control flow in delay slot"); return;
L_09B0F910:
    rt.unsupported(0x09B0F914u, 0x09AC6390u, "control flow in delay slot"); return;
L_09B0F914:
    rt.unsupported(0x09B0F918u, 0x09AC66E8u, "control flow in delay slot"); return;
L_09B0F918:
    rt.unsupported(0x09B0F91Cu, 0x09AC6724u, "control flow in delay slot"); return;
L_09B0F91C:
    rt.unsupported(0x09B0F920u, 0x09AC6EF4u, "control flow in delay slot"); return;
L_09B0F920:
    rt.unsupported(0x09B0F924u, 0x09AC6EF4u, "control flow in delay slot"); return;
L_09B0F924:
    rt.unsupported(0x09B0F928u, 0x09AC6734u, "control flow in delay slot"); return;
L_09B0F928:
    // nop
    ctx.pc = 0x06B19CD0u; (void)rt.invoke_chained_call(ctx, &aot_mem); return;
L_09B0F92C:
    // nop
    goto L_09B0F930;
L_09B0F930:
    rt.unsupported(0x09B0F934u, 0x09ACB880u, "control flow in delay slot"); return;
L_09B0F934:
    rt.unsupported(0x09B0F938u, 0x09ACB8C0u, "control flow in delay slot"); return;
L_09B0F938:
    rt.unsupported(0x09B0F93Cu, 0x09ACB8FCu, "control flow in delay slot"); return;
L_09B0F93C:
    rt.unsupported(0x09B0F940u, 0x09ACB938u, "control flow in delay slot"); return;
L_09B0F940:
    rt.unsupported(0x09B0F944u, 0x09ACB990u, "control flow in delay slot"); return;
L_09B0F944:
    rt.unsupported(0x09B0F948u, 0x09ACBF3Cu, "control flow in delay slot"); return;
L_09B0F948:
    rt.unsupported(0x09B0F94Cu, 0x09ACBF4Cu, "control flow in delay slot"); return;
L_09B0F94C:
    rt.unsupported(0x09B0F950u, 0x09ACBF5Cu, "control flow in delay slot"); return;
L_09B0F950:
    rt.unsupported(0x09B0F954u, 0x09ACBF6Cu, "control flow in delay slot"); return;
L_09B0F954:
    rt.unsupported(0x09B0F958u, 0x09ACBF7Cu, "control flow in delay slot"); return;
L_09B0F958:
    rt.unsupported(0x09B0F95Cu, 0x09ACBF8Cu, "control flow in delay slot"); return;
L_09B0F95C:
    rt.unsupported(0x09B0F960u, 0x09ACBF9Cu, "control flow in delay slot"); return;
L_09B0F960:
    rt.unsupported(0x09B0F964u, 0x09ACBFACu, "control flow in delay slot"); return;
L_09B0F964:
    rt.unsupported(0x09B0F968u, 0x09ACBFBCu, "control flow in delay slot"); return;
L_09B0F968:
    rt.unsupported(0x09B0F96Cu, 0x09ACBFCCu, "control flow in delay slot"); return;
L_09B0F96C:
    rt.unsupported(0x09B0F970u, 0x09ACBFDCu, "control flow in delay slot"); return;
L_09B0F970:
    rt.unsupported(0x09B0F974u, 0x09ACBFECu, "control flow in delay slot"); return;
L_09B0F974:
    rt.unsupported(0x09B0F978u, 0x09ACBFFCu, "control flow in delay slot"); return;
L_09B0F978:
    rt.unsupported(0x09B0F97Cu, 0x09ACC00Cu, "control flow in delay slot"); return;
L_09B0F97C:
    rt.unsupported(0x09B0F980u, 0x09ACC01Cu, "control flow in delay slot"); return;
L_09B0F980:
    rt.unsupported(0x09B0F984u, 0x09ACC02Cu, "control flow in delay slot"); return;
L_09B0F984:
    rt.unsupported(0x09B0F988u, 0x09ACC03Cu, "control flow in delay slot"); return;
L_09B0F988:
    rt.unsupported(0x09B0F98Cu, 0x09ACC04Cu, "control flow in delay slot"); return;
L_09B0F98C:
    rt.unsupported(0x09B0F990u, 0x09ACC4F0u, "control flow in delay slot"); return;
L_09B0F990:
    rt.unsupported(0x09B0F994u, 0x09ACC528u, "control flow in delay slot"); return;
L_09B0F994:
    rt.unsupported(0x09B0F998u, 0x09ACC550u, "control flow in delay slot"); return;
L_09B0F998:
    rt.unsupported(0x09B0F99Cu, 0x09ACC5B0u, "control flow in delay slot"); return;
L_09B0F99C:
    rt.unsupported(0x09B0F9A0u, 0x09ACC61Cu, "control flow in delay slot"); return;
L_09B0F9A0:
    rt.unsupported(0x09B0F9A4u, 0x09ACC678u, "control flow in delay slot"); return;
L_09B0F9A4:
    rt.unsupported(0x09B0F9A8u, 0x09ACC6E4u, "control flow in delay slot"); return;
L_09B0F9A8:
    rt.unsupported(0x09B0F9ACu, 0x09ACC740u, "control flow in delay slot"); return;
L_09B0F9AC:
    rt.unsupported(0x09B0F9B0u, 0x09ACC7ACu, "control flow in delay slot"); return;
L_09B0F9B0:
    // nop
    ctx.pc = 0x06B31EB0u; (void)rt.invoke_chained_call(ctx, &aot_mem); return;
L_09B0F9B4:
    // nop
    goto L_09B0F9B8;
L_09B0F9B8:
    rt.unsupported(0x09B0F9BCu, 0x09ACE604u, "control flow in delay slot"); return;
L_09B0F9BC:
    rt.unsupported(0x09B0F9C0u, 0x09ACE620u, "control flow in delay slot"); return;
L_09B0F9C0:
    rt.unsupported(0x09B0F9C4u, 0x09ACE644u, "control flow in delay slot"); return;
L_09B0F9C4:
    rt.unsupported(0x09B0F9C8u, 0x09ACE660u, "control flow in delay slot"); return;
L_09B0F9C8:
    rt.unsupported(0x09B0F9CCu, 0x09ACE68Cu, "control flow in delay slot"); return;
L_09B0F9CC:
    rt.unsupported(0x09B0F9D0u, 0x09ACE708u, "control flow in delay slot"); return;
L_09B0F9D0:
    rt.unsupported(0x09B0F9D4u, 0x09ACE72Cu, "control flow in delay slot"); return;
L_09B0F9D4:
    rt.unsupported(0x09B0F9D8u, 0x09ACE748u, "control flow in delay slot"); return;
L_09B0F9D8:
    rt.unsupported(0x09B0F9DCu, 0x09ACE7ECu, "control flow in delay slot"); return;
L_09B0F9DC:
    rt.unsupported(0x09B0F9E0u, 0x09ACE808u, "control flow in delay slot"); return;
L_09B0F9E0:
    rt.unsupported(0x09B0F9E4u, 0x09ACE834u, "control flow in delay slot"); return;
L_09B0F9E4:
    rt.unsupported(0x09B0F9E8u, 0x09ACF8B4u, "control flow in delay slot"); return;
L_09B0F9E8:
    rt.unsupported(0x09B0F9ECu, 0x09ACF8C0u, "control flow in delay slot"); return;
L_09B0F9EC:
    rt.unsupported(0x09B0F9F0u, 0x09ACF8ECu, "control flow in delay slot"); return;
L_09B0F9F0:
    rt.unsupported(0x09B0F9F4u, 0x09ACF918u, "control flow in delay slot"); return;
L_09B0F9F4:
    rt.unsupported(0x09B0F9F8u, 0x09ACF93Cu, "control flow in delay slot"); return;
L_09B0F9F8:
    rt.unsupported(0x09B0F9FCu, 0x09ACF93Cu, "control flow in delay slot"); return;
L_09B0F9FC:
    rt.unsupported(0x09B0FA00u, 0x09ACFF40u, "control flow in delay slot"); return;
L_09B0FA00:
    rt.unsupported(0x09B0FA04u, 0x09ACF948u, "control flow in delay slot"); return;
L_09B0FA04:
    rt.unsupported(0x09B0FA08u, 0x09ACF954u, "control flow in delay slot"); return;
L_09B0FA08:
    rt.unsupported(0x09B0FA0Cu, 0x09ACF97Cu, "control flow in delay slot"); return;
L_09B0FA0C:
    rt.unsupported(0x09B0FA10u, 0x09ACF9A0u, "control flow in delay slot"); return;
L_09B0FA10:
    rt.unsupported(0x09B0FA14u, 0x09ACF9C4u, "control flow in delay slot"); return;
L_09B0FA14:
    rt.unsupported(0x09B0FA18u, 0x09ACFF40u, "control flow in delay slot"); return;
L_09B0FA18:
    rt.unsupported(0x09B0FA1Cu, 0x09ACF9D0u, "control flow in delay slot"); return;
L_09B0FA1C:
    rt.unsupported(0x09B0FA20u, 0x09ACF9DCu, "control flow in delay slot"); return;
L_09B0FA20:
    rt.unsupported(0x09B0FA24u, 0x09ACF8B4u, "control flow in delay slot"); return;
L_09B0FA24:
    rt.unsupported(0x09B0FA28u, 0x09ACF8B4u, "control flow in delay slot"); return;
L_09B0FA28:
    // nop
    ctx.pc = 0x06B3E2D0u; (void)rt.invoke_chained_call(ctx, &aot_mem); return;
L_09B0FA2C:
    // nop
    goto L_09B0FA30;
L_09B0FA30:
    rt.unsupported(0x09B0FA34u, 0x09ACEFA8u, "control flow in delay slot"); return;
L_09B0FA34:
    rt.unsupported(0x09B0FA38u, 0x09ACEFD4u, "control flow in delay slot"); return;
L_09B0FA38:
    rt.unsupported(0x09B0FA3Cu, 0x09ACF000u, "control flow in delay slot"); return;
L_09B0FA3C:
    rt.unsupported(0x09B0FA40u, 0x09ACF050u, "control flow in delay slot"); return;
L_09B0FA40:
    rt.unsupported(0x09B0FA44u, 0x09ACF050u, "control flow in delay slot"); return;
L_09B0FA44:
    rt.unsupported(0x09B0FA48u, 0x09ACF0C0u, "control flow in delay slot"); return;
L_09B0FA48:
    rt.unsupported(0x09B0FA4Cu, 0x09ACF0CCu, "control flow in delay slot"); return;
L_09B0FA4C:
    rt.unsupported(0x09B0FA50u, 0x09ACF100u, "control flow in delay slot"); return;
L_09B0FA50:
    rt.unsupported(0x09B0FA54u, 0x09ACF124u, "control flow in delay slot"); return;
L_09B0FA54:
    rt.unsupported(0x09B0FA58u, 0x09ACF188u, "control flow in delay slot"); return;
L_09B0FA58:
    rt.unsupported(0x09B0FA5Cu, 0x09ACF1ECu, "control flow in delay slot"); return;
L_09B0FA5C:
    rt.unsupported(0x09B0FA60u, 0x09ACF258u, "control flow in delay slot"); return;
L_09B0FA60:
    rt.unsupported(0x09B0FA64u, 0x09ACF28Cu, "control flow in delay slot"); return;
L_09B0FA64:
    rt.unsupported(0x09B0FA68u, 0x09ACF298u, "control flow in delay slot"); return;
L_09B0FA68:
    rt.unsupported(0x09B0FA6Cu, 0x09ACEF9Cu, "control flow in delay slot"); return;
L_09B0FA6C:
    rt.unsupported(0x09B0FA70u, 0x09ACEF9Cu, "control flow in delay slot"); return;
L_09B0FA70:
    rt.unsupported(0x09B0FA74u, 0x09ACF2C0u, "control flow in delay slot"); return;
L_09B0FA74:
    rt.unsupported(0x09B0FA78u, 0x09ACFF90u, "control flow in delay slot"); return;
L_09B0FA78:
    rt.unsupported(0x09B0FA7Cu, 0x09ACFFC0u, "control flow in delay slot"); return;
L_09B0FA7C:
    rt.unsupported(0x09B0FA80u, 0x09AD0210u, "control flow in delay slot"); return;
L_09B0FA80:
    rt.unsupported(0x09B0FA84u, 0x09AD0454u, "control flow in delay slot"); return;
L_09B0FA84:
    rt.unsupported(0x09B0FA88u, 0x09AD068Cu, "control flow in delay slot"); return;
L_09B0FA88:
    rt.unsupported(0x09B0FA8Cu, 0x09AD06D8u, "control flow in delay slot"); return;
L_09B0FA8C:
    rt.unsupported(0x09B0FA90u, 0x09AD0920u, "control flow in delay slot"); return;
L_09B0FA90:
    rt.unsupported(0x09B0FA94u, 0x09AD0978u, "control flow in delay slot"); return;
L_09B0FA94:
    rt.unsupported(0x09B0FA98u, 0x09AD0BBCu, "control flow in delay slot"); return;
L_09B0FA98:
    rt.unsupported(0x09B0FA9Cu, 0x09AD0DF4u, "control flow in delay slot"); return;
L_09B0FA9C:
    rt.unsupported(0x09B0FAA0u, 0x09AD102Cu, "control flow in delay slot"); return;
L_09B0FAA0:
    rt.unsupported(0x09B0FAA4u, 0x09AD1258u, "control flow in delay slot"); return;
L_09B0FAA4:
    rt.unsupported(0x09B0FAA8u, 0x09AD1484u, "control flow in delay slot"); return;
L_09B0FAA8:
    rt.unsupported(0x09B0FAACu, 0x09AD16B0u, "control flow in delay slot"); return;
L_09B0FAAC:
    rt.unsupported(0x09B0FAB0u, 0x09AD18E8u, "control flow in delay slot"); return;
L_09B0FAB0:
    rt.unsupported(0x09B0FAB4u, 0x09AD1B58u, "control flow in delay slot"); return;
L_09B0FAB4:
    rt.unsupported(0x09B0FAB8u, 0x09AD1B8Cu, "control flow in delay slot"); return;
L_09B0FAB8:
    // nop
    ctx.pc = 0x06B46E30u; (void)rt.invoke_chained_call(ctx, &aot_mem); return;
L_09B0FABC:
    // nop
    goto L_09B0FAC0;
L_09B0FAC0:
    rt.unsupported(0x09B0FAC4u, 0x09AD1F24u, "control flow in delay slot"); return;
L_09B0FAC4:
    rt.unsupported(0x09B0FAC8u, 0x09AD1D60u, "control flow in delay slot"); return;
L_09B0FAC8:
    rt.unsupported(0x09B0FACCu, 0x09AD1E70u, "control flow in delay slot"); return;
L_09B0FACC:
    rt.unsupported(0x09B0FAD0u, 0x09AD1F24u, "control flow in delay slot"); return;
L_09B0FAD0:
    rt.unsupported(0x09B0FAD4u, 0x09AD1D60u, "control flow in delay slot"); return;
L_09B0FAD4:
    rt.unsupported(0x09B0FAD8u, 0x09AD1E70u, "control flow in delay slot"); return;
L_09B0FAD8:
    rt.unsupported(0x09B0FADCu, 0x09AD1F24u, "control flow in delay slot"); return;
L_09B0FADC:
    rt.unsupported(0x09B0FAE0u, 0x09AD1D60u, "control flow in delay slot"); return;
L_09B0FAE0:
    rt.unsupported(0x09B0FAE4u, 0x09AD1EBCu, "control flow in delay slot"); return;
L_09B0FAE4:
    rt.unsupported(0x09B0FAE8u, 0x09AD1F0Cu, "control flow in delay slot"); return;
L_09B0FAE8:
    rt.unsupported(0x09B0FAECu, 0x09AD1F24u, "control flow in delay slot"); return;
L_09B0FAEC:
    rt.unsupported(0x09B0FAF0u, 0x09AD1C40u, "control flow in delay slot"); return;
L_09B0FAF0:
    rt.unsupported(0x09B0FAF4u, 0x09AD1C70u, "control flow in delay slot"); return;
L_09B0FAF4:
    rt.unsupported(0x09B0FAF8u, 0x09AD1F24u, "control flow in delay slot"); return;
L_09B0FAF8:
    rt.unsupported(0x09B0FAFCu, 0x09AD1CBCu, "control flow in delay slot"); return;
L_09B0FAFC:
    rt.unsupported(0x09B0FB00u, 0x09AD1C70u, "control flow in delay slot"); return;
L_09B0FB00:
    rt.unsupported(0x09B0FB04u, 0x09AD1F24u, "control flow in delay slot"); return;
L_09B0FB04:
    rt.unsupported(0x09B0FB08u, 0x09AD1CBCu, "control flow in delay slot"); return;
L_09B0FB08:
    rt.unsupported(0x09B0FB0Cu, 0x09AD1C70u, "control flow in delay slot"); return;
L_09B0FB0C:
    rt.unsupported(0x09B0FB10u, 0x09AD1F24u, "control flow in delay slot"); return;
L_09B0FB10:
    // nop
    ctx.pc = 0x06B47C90u; (void)rt.invoke_chained_call(ctx, &aot_mem); return;
L_09B0FB14:
    // nop
    goto L_09B0FB18;
L_09B0FB18:
    rt.unsupported(0x09B0FB1Cu, 0x09AD38F0u, "control flow in delay slot"); return;
L_09B0FB1C:
    rt.unsupported(0x09B0FB20u, 0x09AD3948u, "control flow in delay slot"); return;
L_09B0FB20:
    rt.unsupported(0x09B0FB24u, 0x09AD3984u, "control flow in delay slot"); return;
L_09B0FB24:
    rt.unsupported(0x09B0FB28u, 0x09AD39D0u, "control flow in delay slot"); return;
L_09B0FB28:
    rt.unsupported(0x09B0FB2Cu, 0x09AD39F4u, "control flow in delay slot"); return;
L_09B0FB2C:
    rt.unsupported(0x09B0FB30u, 0x09AD3A24u, "control flow in delay slot"); return;
L_09B0FB30:
    rt.unsupported(0x09B0FB34u, 0x09AD3A78u, "control flow in delay slot"); return;
L_09B0FB34:
    rt.unsupported(0x09B0FB38u, 0x09AD3AE8u, "control flow in delay slot"); return;
L_09B0FB38:
    rt.unsupported(0x09B0FB3Cu, 0x09AD3B24u, "control flow in delay slot"); return;
L_09B0FB3C:
    rt.unsupported(0x09B0FB40u, 0x09AD3B88u, "control flow in delay slot"); return;
L_09B0FB40:
    rt.unsupported(0x09B0FB44u, 0x09AD3BF4u, "control flow in delay slot"); return;
L_09B0FB44:
    rt.unsupported(0x09B0FB48u, 0x09AD3C30u, "control flow in delay slot"); return;
L_09B0FB48:
    rt.unsupported(0x09B0FB4Cu, 0x09AD3CE0u, "control flow in delay slot"); return;
L_09B0FB4C:
    rt.unsupported(0x09B0FB50u, 0x09AD3FF0u, "control flow in delay slot"); return;
L_09B0FB50:
    rt.unsupported(0x09B0FB54u, 0x09AD3AE8u, "control flow in delay slot"); return;
L_09B0FB54:
    rt.unsupported(0x09B0FB58u, 0x09AD3B24u, "control flow in delay slot"); return;
L_09B0FB58:
    rt.unsupported(0x09B0FB5Cu, 0x09AD3EF0u, "control flow in delay slot"); return;
L_09B0FB5C:
    rt.unsupported(0x09B0FB60u, 0x09AD3FF0u, "control flow in delay slot"); return;
L_09B0FB60:
    rt.unsupported(0x09B0FB64u, 0x09AD3FF0u, "control flow in delay slot"); return;
L_09B0FB64:
    rt.unsupported(0x09B0FB68u, 0x09AD39D0u, "control flow in delay slot"); return;
L_09B0FB68:
    rt.unsupported(0x09B0FB6Cu, 0x09AD39F4u, "control flow in delay slot"); return;
L_09B0FB6C:
    rt.unsupported(0x09B0FB70u, 0x09AD3A3Cu, "control flow in delay slot"); return;
L_09B0FB70:
    rt.unsupported(0x09B0FB74u, 0x09AD3EB0u, "control flow in delay slot"); return;
L_09B0FB74:
    rt.unsupported(0x09B0FB78u, 0x09AD3F4Cu, "control flow in delay slot"); return;
L_09B0FB78:
    // nop
    ctx.pc = 0x06B4FD30u; (void)rt.invoke_chained_call(ctx, &aot_mem); return;
L_09B0FB7C:
    // nop
    goto L_09B0FB80;
L_09B0FB80:
    rt.unsupported(0x09B0FB84u, 0x09AD4488u, "control flow in delay slot"); return;
L_09B0FB84:
    rt.unsupported(0x09B0FB88u, 0x09AD46A4u, "control flow in delay slot"); return;
L_09B0FB88:
    rt.unsupported(0x09B0FB8Cu, 0x09AD4494u, "control flow in delay slot"); return;
L_09B0FB8C:
    rt.unsupported(0x09B0FB90u, 0x09AD44B4u, "control flow in delay slot"); return;
L_09B0FB90:
    rt.unsupported(0x09B0FB94u, 0x09AD4514u, "control flow in delay slot"); return;
L_09B0FB94:
    rt.unsupported(0x09B0FB98u, 0x09AD4560u, "control flow in delay slot"); return;
L_09B0FB98:
    rt.unsupported(0x09B0FB9Cu, 0x09AD45D0u, "control flow in delay slot"); return;
L_09B0FB9C:
    rt.unsupported(0x09B0FBA0u, 0x09AD46A4u, "control flow in delay slot"); return;
L_09B0FBA0:
    rt.unsupported(0x09B0FBA4u, 0x09AD4634u, "control flow in delay slot"); return;
L_09B0FBA4:
    rt.unsupported(0x09B0FBA8u, 0x09AD45D0u, "control flow in delay slot"); return;
L_09B0FBA8:
    rt.unsupported(0x09B0FBACu, 0x09AD46A4u, "control flow in delay slot"); return;
L_09B0FBAC:
    rt.unsupported(0x09B0FBB0u, 0x09AD4634u, "control flow in delay slot"); return;
L_09B0FBB0:
    rt.unsupported(0x09B0FBB4u, 0x09AD45D0u, "control flow in delay slot"); return;
L_09B0FBB4:
    rt.unsupported(0x09B0FBB8u, 0x09AD4690u, "control flow in delay slot"); return;
L_09B0FBB8:
    rt.unsupported(0x09B0FBBCu, 0x09AD46A4u, "control flow in delay slot"); return;
L_09B0FBBC:
    rt.unsupported(0x09B0FBC0u, 0x09AD4190u, "control flow in delay slot"); return;
L_09B0FBC0:
    rt.unsupported(0x09B0FBC4u, 0x09AD46A4u, "control flow in delay slot"); return;
L_09B0FBC4:
    rt.unsupported(0x09B0FBC8u, 0x09AD41DCu, "control flow in delay slot"); return;
L_09B0FBC8:
    rt.unsupported(0x09B0FBCCu, 0x09AD42BCu, "control flow in delay slot"); return;
L_09B0FBCC:
    rt.unsupported(0x09B0FBD0u, 0x09AD46A4u, "control flow in delay slot"); return;
L_09B0FBD0:
    rt.unsupported(0x09B0FBD4u, 0x09AD41DCu, "control flow in delay slot"); return;
L_09B0FBD4:
    rt.unsupported(0x09B0FBD8u, 0x09AD42BCu, "control flow in delay slot"); return;
L_09B0FBD8:
    rt.unsupported(0x09B0FBDCu, 0x09AD46A4u, "control flow in delay slot"); return;
L_09B0FBDC:
    rt.unsupported(0x09B0FBE0u, 0x09AD41DCu, "control flow in delay slot"); return;
L_09B0FBE0:
    rt.unsupported(0x09B0FBE4u, 0x09AD4310u, "control flow in delay slot"); return;
L_09B0FBE4:
    rt.unsupported(0x09B0FBE8u, 0x09AD4358u, "control flow in delay slot"); return;
L_09B0FBE8:
    rt.unsupported(0x09B0FBECu, 0x09AD43CCu, "control flow in delay slot"); return;
L_09B0FBEC:
    rt.unsupported(0x09B0FBF0u, 0x09AD46A4u, "control flow in delay slot"); return;
L_09B0FBF0:
    // nop
    ctx.pc = 0x06B51A90u; (void)rt.invoke_chained_call(ctx, &aot_mem); return;
L_09B0FBF4:
    // nop
    goto L_09B0FBF8;
L_09B0FBF8:
    rt.unsupported(0x09B0FBFCu, 0x09ADCFA4u, "control flow in delay slot"); return;
L_09B0FBFC:
    rt.unsupported(0x09B0FC00u, 0x09ADCFB8u, "control flow in delay slot"); return;
L_09B0FC00:
    rt.unsupported(0x09B0FC04u, 0x09ADCFCCu, "control flow in delay slot"); return;
L_09B0FC04:
    rt.unsupported(0x09B0FC08u, 0x09ADCFE0u, "control flow in delay slot"); return;
L_09B0FC08:
    rt.unsupported(0x09B0FC0Cu, 0x09ADCFF4u, "control flow in delay slot"); return;
L_09B0FC0C:
    rt.unsupported(0x09B0FC10u, 0x09ADECB4u, "control flow in delay slot"); return;
L_09B0FC10:
    rt.unsupported(0x09B0FC14u, 0x09ADECDCu, "control flow in delay slot"); return;
L_09B0FC14:
    rt.unsupported(0x09B0FC18u, 0x09ADEE04u, "control flow in delay slot"); return;
L_09B0FC18:
    rt.unsupported(0x09B0FC1Cu, 0x09ADF008u, "control flow in delay slot"); return;
L_09B0FC1C:
    rt.unsupported(0x09B0FC20u, 0x09ADF360u, "control flow in delay slot"); return;
L_09B0FC20:
    rt.unsupported(0x09B0FC24u, 0x09ADF46Cu, "control flow in delay slot"); return;
L_09B0FC24:
    rt.unsupported(0x09B0FC28u, 0x09ADF500u, "control flow in delay slot"); return;
L_09B0FC28:
    // nop
    ctx.pc = 0x06B7D400u; (void)rt.invoke_chained_call(ctx, &aot_mem); return;
L_09B0FC2C:
    // nop
    goto L_09B0FC30;
L_09B0FC30:
    rt.unsupported(0x09B0FC34u, 0x09ADFB2Cu, "control flow in delay slot"); return;
L_09B0FC34:
    rt.unsupported(0x09B0FC38u, 0x09ADFB40u, "control flow in delay slot"); return;
L_09B0FC38:
    rt.unsupported(0x09B0FC3Cu, 0x09ADFB54u, "control flow in delay slot"); return;
L_09B0FC3C:
    rt.unsupported(0x09B0FC40u, 0x09ADFB68u, "control flow in delay slot"); return;
L_09B0FC40:
    rt.unsupported(0x09B0FC44u, 0x09ADFB7Cu, "control flow in delay slot"); return;
L_09B0FC44:
    rt.unsupported(0x09B0FC48u, 0x09ADF948u, "control flow in delay slot"); return;
L_09B0FC48:
    rt.unsupported(0x09B0FC4Cu, 0x09ADF988u, "control flow in delay slot"); return;
L_09B0FC4C:
    rt.unsupported(0x09B0FC50u, 0x09ADF9F0u, "control flow in delay slot"); return;
L_09B0FC50:
    rt.unsupported(0x09B0FC54u, 0x09ADFA20u, "control flow in delay slot"); return;
L_09B0FC54:
    rt.unsupported(0x09B0FC58u, 0x09ADFA50u, "control flow in delay slot"); return;
L_09B0FC58:
    rt.unsupported(0x09B0FC5Cu, 0x09ADFA74u, "control flow in delay slot"); return;
L_09B0FC5C:
    rt.unsupported(0x09B0FC60u, 0x09ADFD3Cu, "control flow in delay slot"); return;
L_09B0FC60:
    rt.unsupported(0x09B0FC64u, 0x09ADFD44u, "control flow in delay slot"); return;
L_09B0FC64:
    rt.unsupported(0x09B0FC68u, 0x09ADFD4Cu, "control flow in delay slot"); return;
L_09B0FC68:
    rt.unsupported(0x09B0FC6Cu, 0x09ADFD54u, "control flow in delay slot"); return;
L_09B0FC6C:
    rt.unsupported(0x09B0FC70u, 0x09ADFD5Cu, "control flow in delay slot"); return;
L_09B0FC70:
    rt.unsupported(0x09B0FC74u, 0x09ADFD64u, "control flow in delay slot"); return;
L_09B0FC74:
    rt.unsupported(0x09B0FC78u, 0x09ADFD64u, "control flow in delay slot"); return;
L_09B0FC78:
    // nop
    ctx.pc = 0x06B7F590u; (void)rt.invoke_chained_call(ctx, &aot_mem); return;
L_09B0FC7C:
    // nop
    goto L_09B0FC80;
L_09B0FC80:
    rt.unsupported(0x09B0FC84u, 0x09AE0504u, "control flow in delay slot"); return;
L_09B0FC84:
    rt.unsupported(0x09B0FC88u, 0x09AE0730u, "control flow in delay slot"); return;
L_09B0FC88:
    rt.unsupported(0x09B0FC8Cu, 0x09AE0BD8u, "control flow in delay slot"); return;
L_09B0FC8C:
    rt.unsupported(0x09B0FC90u, 0x09AE0D4Cu, "control flow in delay slot"); return;
L_09B0FC90:
    rt.unsupported(0x09B0FC94u, 0x09AE0F00u, "control flow in delay slot"); return;
L_09B0FC94:
    rt.unsupported(0x09B0FC98u, 0x09AE0F3Cu, "control flow in delay slot"); return;
L_09B0FC98:
    // nop
    ctx.pc = 0x06B83CF0u; (void)rt.invoke_chained_call(ctx, &aot_mem); return;
L_09B0FC9C:
    // nop
    goto L_09B0FCA0;
L_09B0FCA0:
    rt.unsupported(0x09B0FCA4u, 0x09AE1B20u, "control flow in delay slot"); return;
L_09B0FCA4:
    rt.unsupported(0x09B0FCA8u, 0x09AE1B20u, "control flow in delay slot"); return;
L_09B0FCA8:
    rt.unsupported(0x09B0FCACu, 0x09AE1B20u, "control flow in delay slot"); return;
L_09B0FCAC:
    rt.unsupported(0x09B0FCB0u, 0x09AE1B20u, "control flow in delay slot"); return;
L_09B0FCB0:
    rt.unsupported(0x09B0FCB4u, 0x09AE1B20u, "control flow in delay slot"); return;
L_09B0FCB4:
    rt.unsupported(0x09B0FCB8u, 0x09AE1F78u, "control flow in delay slot"); return;
L_09B0FCB8:
    rt.unsupported(0x09B0FCBCu, 0x09AE1FBCu, "control flow in delay slot"); return;
L_09B0FCBC:
    rt.unsupported(0x09B0FCC0u, 0x09AE1FBCu, "control flow in delay slot"); return;
L_09B0FCC0:
    rt.unsupported(0x09B0FCC4u, 0x09AE1FBCu, "control flow in delay slot"); return;
L_09B0FCC4:
    rt.unsupported(0x09B0FCC8u, 0x09AE1FBCu, "control flow in delay slot"); return;
L_09B0FCC8:
    rt.unsupported(0x09B0FCCCu, 0x09AE1FBCu, "control flow in delay slot"); return;
L_09B0FCCC:
    rt.unsupported(0x09B0FCD0u, 0x09AE3A48u, "control flow in delay slot"); return;
L_09B0FCD0:
    rt.unsupported(0x09B0FCD4u, 0x09AE3A58u, "control flow in delay slot"); return;
L_09B0FCD4:
    rt.unsupported(0x09B0FCD8u, 0x09AE3A68u, "control flow in delay slot"); return;
L_09B0FCD8:
    rt.unsupported(0x09B0FCDCu, 0x09AE3A78u, "control flow in delay slot"); return;
L_09B0FCDC:
    rt.unsupported(0x09B0FCE0u, 0x09AE3A88u, "control flow in delay slot"); return;
L_09B0FCE0:
    rt.unsupported(0x09B0FCE4u, 0x09AE3A98u, "control flow in delay slot"); return;
L_09B0FCE4:
    rt.unsupported(0x09B0FCE8u, 0x09AE430Cu, "control flow in delay slot"); return;
L_09B0FCE8:
    rt.unsupported(0x09B0FCECu, 0x09AE4120u, "control flow in delay slot"); return;
L_09B0FCEC:
    rt.unsupported(0x09B0FCF0u, 0x09AE3F90u, "control flow in delay slot"); return;
L_09B0FCF0:
    rt.unsupported(0x09B0FCF4u, 0x09AE3F18u, "control flow in delay slot"); return;
L_09B0FCF4:
    rt.unsupported(0x09B0FCF8u, 0x09AE3F18u, "control flow in delay slot"); return;
L_09B0FCF8:
    rt.unsupported(0x09B0FCFCu, 0x09AE3F18u, "control flow in delay slot"); return;
L_09B0FCFC:
    rt.unsupported(0x09B0FD00u, 0x09AE3F18u, "control flow in delay slot"); return;
L_09B0FD00:
    // nop
    ctx.pc = 0x06B8FC60u; (void)rt.invoke_chained_call(ctx, &aot_mem); return;
L_09B0FD04:
    // nop
    goto L_09B0FD08;
L_09B0FD08:
    rt.unsupported(0x09B0FD0Cu, 0x09AE4C68u, "control flow in delay slot"); return;
L_09B0FD0C:
    rt.unsupported(0x09B0FD10u, 0x09AE4C78u, "control flow in delay slot"); return;
L_09B0FD10:
    rt.unsupported(0x09B0FD14u, 0x09AE4C88u, "control flow in delay slot"); return;
L_09B0FD14:
    rt.unsupported(0x09B0FD18u, 0x09AE4C98u, "control flow in delay slot"); return;
L_09B0FD18:
    rt.unsupported(0x09B0FD1Cu, 0x09AE4CD4u, "control flow in delay slot"); return;
L_09B0FD1C:
    rt.unsupported(0x09B0FD20u, 0x09AE51B4u, "control flow in delay slot"); return;
L_09B0FD20:
    rt.unsupported(0x09B0FD24u, 0x09AE51DCu, "control flow in delay slot"); return;
L_09B0FD24:
    rt.unsupported(0x09B0FD28u, 0x09AE5244u, "control flow in delay slot"); return;
L_09B0FD28:
    rt.unsupported(0x09B0FD2Cu, 0x09AE52B0u, "control flow in delay slot"); return;
L_09B0FD2C:
    rt.unsupported(0x09B0FD30u, 0x09AE5318u, "control flow in delay slot"); return;
L_09B0FD30:
    rt.unsupported(0x09B0FD34u, 0x09AE5330u, "control flow in delay slot"); return;
L_09B0FD34:
    rt.unsupported(0x09B0FD38u, 0x09AE5328u, "control flow in delay slot"); return;
L_09B0FD38:
    // nop
    ctx.pc = 0x06B94CA0u; (void)rt.invoke_chained_call(ctx, &aot_mem); return;
L_09B0FD3C:
    // nop
    goto L_09B0FD40;
L_09B0FD40:
    rt.unsupported(0x09B0FD44u, 0x09AE5678u, "control flow in delay slot"); return;
L_09B0FD44:
    rt.unsupported(0x09B0FD48u, 0x09AE5678u, "control flow in delay slot"); return;
L_09B0FD48:
    rt.unsupported(0x09B0FD4Cu, 0x09AE5678u, "control flow in delay slot"); return;
L_09B0FD4C:
    rt.unsupported(0x09B0FD50u, 0x09AE5678u, "control flow in delay slot"); return;
L_09B0FD50:
    rt.unsupported(0x09B0FD54u, 0x09AE5678u, "control flow in delay slot"); return;
L_09B0FD54:
    rt.unsupported(0x09B0FD58u, 0x09AE5BB4u, "control flow in delay slot"); return;
L_09B0FD58:
    rt.unsupported(0x09B0FD5Cu, 0x09AE5BD8u, "control flow in delay slot"); return;
L_09B0FD5C:
    rt.unsupported(0x09B0FD60u, 0x09AE5BD8u, "control flow in delay slot"); return;
L_09B0FD60:
    rt.unsupported(0x09B0FD64u, 0x09AE5BD8u, "control flow in delay slot"); return;
L_09B0FD64:
    rt.unsupported(0x09B0FD68u, 0x09AE5BD8u, "control flow in delay slot"); return;
L_09B0FD68:
    rt.unsupported(0x09B0FD6Cu, 0x09AE5BD8u, "control flow in delay slot"); return;
L_09B0FD6C:
    rt.unsupported(0x09B0FD70u, 0x09AE9C80u, "control flow in delay slot"); return;
L_09B0FD70:
    rt.unsupported(0x09B0FD74u, 0x09AE9C8Cu, "control flow in delay slot"); return;
L_09B0FD74:
    rt.unsupported(0x09B0FD78u, 0x09AE9C98u, "control flow in delay slot"); return;
L_09B0FD78:
    rt.unsupported(0x09B0FD7Cu, 0x09AE9CA4u, "control flow in delay slot"); return;
L_09B0FD7C:
    rt.unsupported(0x09B0FD80u, 0x09AE9CB0u, "control flow in delay slot"); return;
L_09B0FD80:
    rt.unsupported(0x09B0FD84u, 0x09AE9CBCu, "control flow in delay slot"); return;
L_09B0FD84:
    rt.unsupported(0x09B0FD88u, 0x09AE9CBCu, "control flow in delay slot"); return;
L_09B0FD88:
    // nop
    ctx.pc = 0x06BA72F0u; (void)rt.invoke_chained_call(ctx, &aot_mem); return;
L_09B0FD8C:
    // nop
    goto L_09B0FD90;
L_09B0FD90:
    rt.unsupported(0x09B0FD94u, 0x09AEE6DCu, "control flow in delay slot"); return;
L_09B0FD94:
    rt.unsupported(0x09B0FD98u, 0x09AEE6E4u, "control flow in delay slot"); return;
L_09B0FD98:
    rt.unsupported(0x09B0FD9Cu, 0x09AEE6ECu, "control flow in delay slot"); return;
L_09B0FD9C:
    rt.unsupported(0x09B0FDA0u, 0x09AEE6F4u, "control flow in delay slot"); return;
L_09B0FDA0:
    rt.unsupported(0x09B0FDA4u, 0x09AEE6FCu, "control flow in delay slot"); return;
L_09B0FDA4:
    rt.unsupported(0x09B0FDA8u, 0x09AEF8D4u, "control flow in delay slot"); return;
L_09B0FDA8:
    rt.unsupported(0x09B0FDACu, 0x09AEF8F0u, "control flow in delay slot"); return;
L_09B0FDAC:
    rt.unsupported(0x09B0FDB0u, 0x09AEF93Cu, "control flow in delay slot"); return;
L_09B0FDB0:
    rt.unsupported(0x09B0FDB4u, 0x09AEF960u, "control flow in delay slot"); return;
L_09B0FDB4:
    rt.unsupported(0x09B0FDB8u, 0x09AEF980u, "control flow in delay slot"); return;
L_09B0FDB8:
    rt.unsupported(0x09B0FDBCu, 0x09AEFA6Cu, "control flow in delay slot"); return;
L_09B0FDBC:
    rt.unsupported(0x09B0FDC0u, 0x09AEFAF0u, "control flow in delay slot"); return;
L_09B0FDC0:
    rt.unsupported(0x09B0FDC4u, 0x09AEFCDCu, "control flow in delay slot"); return;
L_09B0FDC4:
    rt.unsupported(0x09B0FDC8u, 0x09AF1928u, "control flow in delay slot"); return;
L_09B0FDC8:
    rt.unsupported(0x09B0FDCCu, 0x09AF1864u, "control flow in delay slot"); return;
L_09B0FDCC:
    rt.unsupported(0x09B0FDD0u, 0x09AF1864u, "control flow in delay slot"); return;
L_09B0FDD0:
    rt.unsupported(0x09B0FDD4u, 0x09AF1864u, "control flow in delay slot"); return;
L_09B0FDD4:
    rt.unsupported(0x09B0FDD8u, 0x09AF189Cu, "control flow in delay slot"); return;
L_09B0FDD8:
    rt.unsupported(0x09B0FDDCu, 0x09AF189Cu, "control flow in delay slot"); return;
L_09B0FDDC:
    rt.unsupported(0x09B0FDE0u, 0x09AF189Cu, "control flow in delay slot"); return;
L_09B0FDE0:
    rt.unsupported(0x09B0FDE4u, 0x09AF189Cu, "control flow in delay slot"); return;
L_09B0FDE4:
    rt.unsupported(0x09B0FDE8u, 0x09AF42E0u, "control flow in delay slot"); return;
L_09B0FDE8:
    rt.unsupported(0x09B0FDECu, 0x09AF4528u, "control flow in delay slot"); return;
L_09B0FDEC:
    rt.unsupported(0x09B0FDF0u, 0x09AF435Cu, "control flow in delay slot"); return;
L_09B0FDF0:
    rt.unsupported(0x09B0FDF4u, 0x09AF435Cu, "control flow in delay slot"); return;
L_09B0FDF4:
    rt.unsupported(0x09B0FDF8u, 0x09AF440Cu, "control flow in delay slot"); return;
L_09B0FDF8:
    rt.unsupported(0x09B0FDFCu, 0x09AF44A0u, "control flow in delay slot"); return;
L_09B0FDFC:
    ctx.gpr[29] = (ctx.gpr[29] + static_cast<std::uint32_t>(-16));
    ctx.pc = 0x06BD1280u; (void)rt.invoke_chained_call(ctx, &aot_mem); return;
L_09B0FE00:
    ctx.gpr[29] = (ctx.gpr[29] + static_cast<std::uint32_t>(-16));
    goto L_09B0FE04;
L_09B0FE04:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(12), ctx.gpr[31]);
    goto L_09B0FE08;
L_09B0FE08:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(8), ctx.gpr[16]);
    goto L_09B0FE0C;
L_09B0FE0C:
    ctx.gpr[16] = (2481u << 16u);
    goto L_09B0FE10;
L_09B0FE10:
    ctx.gpr[16] = (ctx.gpr[16] + static_cast<std::uint32_t>(192));
    goto L_09B0FE14;
L_09B0FE14:
    ctx.gpr[31] = (0x09B0FE1Cu);
    ctx.gpr[4] = (ctx.gpr[16] + 0u);
    ctx.pc = 0x09A7C908u;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09B0FE1Cu) goto L_09B0FE1C;
    return;
L_09B0FE18:
    ctx.gpr[4] = (ctx.gpr[16] + 0u);
    goto L_09B0FE1C;
L_09B0FE1C:
    ctx.gpr[4] = (2481u << 16u);
    goto L_09B0FE20;
L_09B0FE20:
    ctx.gpr[31] = (0x09B0FE28u);
    ctx.gpr[4] = (ctx.gpr[4] + static_cast<std::uint32_t>(256));
    ctx.pc = 0x09A7C8F8u;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09B0FE28u) goto L_09B0FE28;
    return;
L_09B0FE24:
    ctx.gpr[4] = (ctx.gpr[4] + static_cast<std::uint32_t>(256));
    goto L_09B0FE28;
L_09B0FE28:
    ctx.gpr[4] = (ctx.gpr[16] + static_cast<std::uint32_t>(7780));
    goto L_09B0FE2C;
L_09B0FE2C:
    ctx.gpr[31] = (0x09B0FE34u);
    ctx.gpr[4] = (ctx.gpr[4] + static_cast<std::uint32_t>(32760));
    ctx.pc = 0x09A7C8E8u;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09B0FE34u) goto L_09B0FE34;
    return;
L_09B0FE30:
    ctx.gpr[4] = (ctx.gpr[4] + static_cast<std::uint32_t>(32760));
    goto L_09B0FE34;
L_09B0FE34:
    ctx.gpr[4] = (ctx.gpr[16] + static_cast<std::uint32_t>(10120));
    goto L_09B0FE38;
L_09B0FE38:
    ctx.gpr[31] = (0x09B0FE40u);
    ctx.gpr[4] = (ctx.gpr[4] + static_cast<std::uint32_t>(32760));
    ctx.pc = 0x09A7C8D8u;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09B0FE40u) goto L_09B0FE40;
    return;
L_09B0FE3C:
    ctx.gpr[4] = (ctx.gpr[4] + static_cast<std::uint32_t>(32760));
    goto L_09B0FE40;
L_09B0FE40:
    ctx.gpr[4] = (ctx.gpr[16] + static_cast<std::uint32_t>(10384));
    goto L_09B0FE44;
L_09B0FE44:
    ctx.gpr[31] = (0x09B0FE4Cu);
    ctx.gpr[4] = (ctx.gpr[4] + static_cast<std::uint32_t>(32760));
    ctx.pc = 0x09A7C8D0u;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09B0FE4Cu) goto L_09B0FE4C;
    return;
L_09B0FE48:
    ctx.gpr[4] = (ctx.gpr[4] + static_cast<std::uint32_t>(32760));
    goto L_09B0FE4C;
L_09B0FE4C:
    ctx.gpr[4] = (ctx.gpr[16] + static_cast<std::uint32_t>(22548));
    goto L_09B0FE50;
L_09B0FE50:
    ctx.gpr[31] = (0x09B0FE58u);
    ctx.gpr[4] = (ctx.gpr[4] + static_cast<std::uint32_t>(32760));
    ctx.pc = 0x09A7C8C0u;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09B0FE58u) goto L_09B0FE58;
    return;
L_09B0FE54:
    ctx.gpr[4] = (ctx.gpr[4] + static_cast<std::uint32_t>(32760));
    goto L_09B0FE58;
L_09B0FE58:
    ctx.gpr[4] = (ctx.gpr[16] + static_cast<std::uint32_t>(23184));
    goto L_09B0FE5C;
L_09B0FE5C:
    ctx.gpr[31] = (0x09B0FE64u);
    ctx.gpr[4] = (ctx.gpr[4] + static_cast<std::uint32_t>(32760));
    ctx.pc = 0x09A7C8B0u;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09B0FE64u) goto L_09B0FE64;
    return;
L_09B0FE60:
    ctx.gpr[4] = (ctx.gpr[4] + static_cast<std::uint32_t>(32760));
    goto L_09B0FE64;
L_09B0FE64:
    ctx.gpr[4] = (ctx.gpr[16] + static_cast<std::uint32_t>(23272));
    goto L_09B0FE68;
L_09B0FE68:
    ctx.gpr[31] = (0x09B0FE70u);
    ctx.gpr[4] = (ctx.gpr[4] + static_cast<std::uint32_t>(32760));
    ctx.pc = 0x09A7C8A0u;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09B0FE70u) goto L_09B0FE70;
    return;
L_09B0FE6C:
    ctx.gpr[4] = (ctx.gpr[4] + static_cast<std::uint32_t>(32760));
    goto L_09B0FE70;
L_09B0FE70:
    ctx.gpr[1] = (1u << 16u);
    goto L_09B0FE74;
L_09B0FE74:
    ctx.gpr[1] = (ctx.gpr[1] | 11024u);
    goto L_09B0FE78;
L_09B0FE78:
    ctx.gpr[31] = (0x09B0FE80u);
    ctx.gpr[4] = (ctx.gpr[16] + ctx.gpr[1]);
    ctx.pc = 0x09A7C890u;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09B0FE80u) goto L_09B0FE80;
    return;
L_09B0FE7C:
    ctx.gpr[4] = (ctx.gpr[16] + ctx.gpr[1]);
    goto L_09B0FE80;
L_09B0FE80:
    ctx.gpr[1] = (1u << 16u);
    goto L_09B0FE84;
L_09B0FE84:
    ctx.gpr[1] = (ctx.gpr[1] | 11096u);
    goto L_09B0FE88;
L_09B0FE88:
    ctx.gpr[31] = (0x09B0FE90u);
    ctx.gpr[4] = (ctx.gpr[16] + ctx.gpr[1]);
    ctx.pc = 0x09A7C880u;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09B0FE90u) goto L_09B0FE90;
    return;
L_09B0FE8C:
    ctx.gpr[4] = (ctx.gpr[16] + ctx.gpr[1]);
    goto L_09B0FE90;
L_09B0FE90:
    ctx.gpr[1] = (1u << 16u);
    goto L_09B0FE94;
L_09B0FE94:
    ctx.gpr[1] = (ctx.gpr[1] | 11156u);
    goto L_09B0FE98;
L_09B0FE98:
    ctx.gpr[31] = (0x09B0FEA0u);
    ctx.gpr[4] = (ctx.gpr[16] + ctx.gpr[1]);
    ctx.pc = 0x09A7C870u;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09B0FEA0u) goto L_09B0FEA0;
    return;
L_09B0FE9C:
    ctx.gpr[4] = (ctx.gpr[16] + ctx.gpr[1]);
    goto L_09B0FEA0;
L_09B0FEA0:
    ctx.gpr[1] = (1u << 16u);
    goto L_09B0FEA4;
L_09B0FEA4:
    ctx.gpr[1] = (ctx.gpr[1] | 11176u);
    goto L_09B0FEA8;
L_09B0FEA8:
    ctx.gpr[31] = (0x09B0FEB0u);
    ctx.gpr[4] = (ctx.gpr[16] + ctx.gpr[1]);
    ctx.pc = 0x09A7C860u;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09B0FEB0u) goto L_09B0FEB0;
    return;
L_09B0FEAC:
    ctx.gpr[4] = (ctx.gpr[16] + ctx.gpr[1]);
    goto L_09B0FEB0;
L_09B0FEB0:
    ctx.gpr[1] = (1u << 16u);
    goto L_09B0FEB4;
L_09B0FEB4:
    ctx.gpr[1] = (ctx.gpr[1] | 11184u);
    goto L_09B0FEB8;
L_09B0FEB8:
    ctx.gpr[31] = (0x09B0FEC0u);
    ctx.gpr[4] = (ctx.gpr[16] + ctx.gpr[1]);
    ctx.pc = 0x09A7C850u;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09B0FEC0u) goto L_09B0FEC0;
    return;
L_09B0FEBC:
    ctx.gpr[4] = (ctx.gpr[16] + ctx.gpr[1]);
    goto L_09B0FEC0;
L_09B0FEC0:
    ctx.gpr[1] = (1u << 16u);
    goto L_09B0FEC4;
L_09B0FEC4:
    ctx.gpr[1] = (ctx.gpr[1] | 11356u);
    goto L_09B0FEC8;
L_09B0FEC8:
    ctx.gpr[31] = (0x09B0FED0u);
    ctx.gpr[4] = (ctx.gpr[16] + ctx.gpr[1]);
    ctx.pc = 0x09A7C840u;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09B0FED0u) goto L_09B0FED0;
    return;
L_09B0FECC:
    ctx.gpr[4] = (ctx.gpr[16] + ctx.gpr[1]);
    goto L_09B0FED0;
L_09B0FED0:
    ctx.gpr[1] = (1u << 16u);
    goto L_09B0FED4;
L_09B0FED4:
    ctx.gpr[1] = (ctx.gpr[1] | 11364u);
    goto L_09B0FED8;
L_09B0FED8:
    ctx.gpr[31] = (0x09B0FEE0u);
    ctx.gpr[4] = (ctx.gpr[16] + ctx.gpr[1]);
    ctx.pc = 0x09A7C830u;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09B0FEE0u) goto L_09B0FEE0;
    return;
L_09B0FEDC:
    ctx.gpr[4] = (ctx.gpr[16] + ctx.gpr[1]);
    goto L_09B0FEE0;
L_09B0FEE0:
    ctx.gpr[1] = (1u << 16u);
    goto L_09B0FEE4;
L_09B0FEE4:
    ctx.gpr[1] = (ctx.gpr[1] | 11380u);
    goto L_09B0FEE8;
L_09B0FEE8:
    ctx.gpr[31] = (0x09B0FEF0u);
    ctx.gpr[4] = (ctx.gpr[16] + ctx.gpr[1]);
    ctx.pc = 0x09A7C818u;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09B0FEF0u) goto L_09B0FEF0;
    return;
L_09B0FEEC:
    ctx.gpr[4] = (ctx.gpr[16] + ctx.gpr[1]);
    goto L_09B0FEF0;
L_09B0FEF0:
    ctx.gpr[5] = (2472u << 16u);
    goto L_09B0FEF4;
L_09B0FEF4:
    ctx.gpr[6] = (2481u << 16u);
    goto L_09B0FEF8;
L_09B0FEF8:
    ctx.gpr[4] = (ctx.gpr[16] + 0u);
    goto L_09B0FEFC;
L_09B0FEFC:
    ctx.gpr[5] = (ctx.gpr[5] + static_cast<std::uint32_t>(-14872));
    goto L_09B0FF00;
L_09B0FF00:
    ctx.gpr[31] = (0x09B0FF08u);
    ctx.gpr[6] = (ctx.gpr[6] + static_cast<std::uint32_t>(176));
    ctx.pc = 0x0890C3F8u;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09B0FF08u) goto L_09B0FF08;
    return;
L_09B0FF04:
    ctx.gpr[6] = (ctx.gpr[6] + static_cast<std::uint32_t>(176));
    goto L_09B0FF08;
L_09B0FF08:
    ctx.gpr[31] = (aot_mem.aot_load32(ctx.gpr[29] + static_cast<std::uint32_t>(12)));
    goto L_09B0FF0C;
L_09B0FF0C:
    ctx.gpr[16] = (aot_mem.aot_load32(ctx.gpr[29] + static_cast<std::uint32_t>(8)));
    goto L_09B0FF10;
L_09B0FF10:
    jump_target = ctx.gpr[31];
    ctx.gpr[29] = (ctx.gpr[29] + static_cast<std::uint32_t>(16));
    local_pc = jump_target;
    if (++local_transfers < 2048u) { entry_id = 0u; goto LOCAL_DISPATCH; }
    ctx.pc = jump_target;
    return;
L_09B0FF14:
    ctx.gpr[29] = (ctx.gpr[29] + static_cast<std::uint32_t>(16));
    goto L_09B0FF18;
L_09B0FF18:
    ctx.gpr[29] = (ctx.gpr[29] + static_cast<std::uint32_t>(-16));
    goto L_09B0FF1C;
L_09B0FF1C:
    ctx.gpr[4] = (2482u << 16u);
    goto L_09B0FF20;
L_09B0FF20:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(12), ctx.gpr[31]);
    goto L_09B0FF24;
L_09B0FF24:
    ctx.gpr[31] = (0x09B0FF2Cu);
    ctx.gpr[4] = (ctx.gpr[4] + static_cast<std::uint32_t>(13160));
    ctx.pc = 0x09A7C918u;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09B0FF2Cu) goto L_09B0FF2C;
    return;
L_09B0FF28:
    ctx.gpr[4] = (ctx.gpr[4] + static_cast<std::uint32_t>(13160));
    goto L_09B0FF2C;
L_09B0FF2C:
    ctx.gpr[4] = (2482u << 16u);
    goto L_09B0FF30;
L_09B0FF30:
    ctx.gpr[5] = (2472u << 16u);
    goto L_09B0FF34;
L_09B0FF34:
    ctx.gpr[6] = (2482u << 16u);
    goto L_09B0FF38;
L_09B0FF38:
    ctx.gpr[4] = (ctx.gpr[4] + static_cast<std::uint32_t>(13160));
    goto L_09B0FF3C;
L_09B0FF3C:
    ctx.gpr[5] = (ctx.gpr[5] + static_cast<std::uint32_t>(-14008));
    goto L_09B0FF40;
L_09B0FF40:
    ctx.gpr[31] = (0x09B0FF48u);
    ctx.gpr[6] = (ctx.gpr[6] + static_cast<std::uint32_t>(13144));
    ctx.pc = 0x0890C3F8u;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09B0FF48u) goto L_09B0FF48;
    return;
L_09B0FF44:
    ctx.gpr[6] = (ctx.gpr[6] + static_cast<std::uint32_t>(13144));
    goto L_09B0FF48;
L_09B0FF48:
    ctx.gpr[31] = (aot_mem.aot_load32(ctx.gpr[29] + static_cast<std::uint32_t>(12)));
    goto L_09B0FF4C;
L_09B0FF4C:
    jump_target = ctx.gpr[31];
    ctx.gpr[29] = (ctx.gpr[29] + static_cast<std::uint32_t>(16));
    local_pc = jump_target;
    if (++local_transfers < 2048u) { entry_id = 0u; goto LOCAL_DISPATCH; }
    ctx.pc = jump_target;
    return;
L_09B0FF50:
    ctx.gpr[29] = (ctx.gpr[29] + static_cast<std::uint32_t>(16));
    goto L_09B0FF54;
L_09B0FF54:
    ctx.gpr[29] = (ctx.gpr[29] + static_cast<std::uint32_t>(-32));
    goto L_09B0FF58;
L_09B0FF58:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(12), ctx.gpr[31]);
    goto L_09B0FF5C;
L_09B0FF5C:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(8), ctx.gpr[16]);
    goto L_09B0FF60;
L_09B0FF60:
    ctx.gpr[16] = (2482u << 16u);
    goto L_09B0FF64;
L_09B0FF64:
    ctx.gpr[16] = (ctx.gpr[16] + static_cast<std::uint32_t>(15152));
    goto L_09B0FF68;
L_09B0FF68:
    ctx.gpr[2] = (2482u << 16u);
    goto L_09B0FF6C;
L_09B0FF6C:
    aot_mem.aot_store32(ctx.gpr[2] + static_cast<std::uint32_t>(15256), ctx.gpr[16]);
    goto L_09B0FF70;
L_09B0FF70:
    ctx.gpr[4] = (0u + 0u);
    goto L_09B0FF74;
L_09B0FF74:
    ctx.gpr[3] = (ctx.gpr[16] + 0u);
    goto L_09B0FF78;
L_09B0FF78:
    aot_mem.aot_store32(ctx.gpr[3] + static_cast<std::uint32_t>(12), 0u);
    goto L_09B0FF7C;
L_09B0FF7C:
    aot_mem.aot_store32(ctx.gpr[3] + static_cast<std::uint32_t>(16), 0u);
    goto L_09B0FF80;
L_09B0FF80:
    aot_mem.aot_store32(ctx.gpr[3] + static_cast<std::uint32_t>(20), 0u);
    goto L_09B0FF84;
L_09B0FF84:
    aot_mem.aot_store32(ctx.gpr[3] + static_cast<std::uint32_t>(24), 0u);
    goto L_09B0FF88;
L_09B0FF88:
    ctx.gpr[4] = (ctx.gpr[4] + static_cast<std::uint32_t>(5));
    goto L_09B0FF8C;
L_09B0FF8C:
    aot_mem.aot_store32(ctx.gpr[3] + static_cast<std::uint32_t>(28), 0u);
    goto L_09B0FF90;
L_09B0FF90:
    ctx.gpr[2] = (static_cast<std::int32_t>(ctx.gpr[4]) < 20 ? 1u : 0u);
    goto L_09B0FF94;
L_09B0FF94:
    { const bool branch_taken = ctx.gpr[2] != 0u;
    ctx.gpr[3] = (ctx.gpr[3] + static_cast<std::uint32_t>(20));
      if (branch_taken) {
          goto L_09B0FF78;
      }
      goto L_09B0FF9C;
    }
L_09B0FF98:
    ctx.gpr[3] = (ctx.gpr[3] + static_cast<std::uint32_t>(20));
    goto L_09B0FF9C;
L_09B0FF9C:
    ctx.gpr[2] = (2481u << 16u);
    goto L_09B0FFA0;
L_09B0FFA0:
    ctx.fpr[1] = std::bit_cast<float>(aot_mem.aot_load32(ctx.gpr[2] + static_cast<std::uint32_t>(-29856)));
    goto L_09B0FFA4;
L_09B0FFA4:
    ctx.gpr[4] = (ctx.gpr[29] + static_cast<std::uint32_t>(20));
    goto L_09B0FFA8;
L_09B0FFA8:
    aot_mem.aot_store32(ctx.gpr[16] + static_cast<std::uint32_t>(92), 0u);
    goto L_09B0FFAC;
L_09B0FFAC:
    ctx.gpr[2] = (2481u << 16u);
    goto L_09B0FFB0;
L_09B0FFB0:
    ctx.fpr[0] = std::bit_cast<float>(aot_mem.aot_load32(ctx.gpr[2] + static_cast<std::uint32_t>(-29852)));
    goto L_09B0FFB4;
L_09B0FFB4:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(20), std::bit_cast<std::uint32_t>(ctx.fpr[1]));
    goto L_09B0FFB8;
L_09B0FFB8:
    ctx.gpr[2] = (2481u << 16u);
    goto L_09B0FFBC;
L_09B0FFBC:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(24), std::bit_cast<std::uint32_t>(ctx.fpr[0]));
    goto L_09B0FFC0;
L_09B0FFC0:
    ctx.fpr[0] = std::bit_cast<float>(aot_mem.aot_load32(ctx.gpr[2] + static_cast<std::uint32_t>(-29848)));
    goto L_09B0FFC4;
L_09B0FFC4:
    ctx.gpr[31] = (0x09B0FFCCu);
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(28), std::bit_cast<std::uint32_t>(ctx.fpr[0]));
    ctx.pc = 0x0890C4F8u;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09B0FFCCu) goto L_09B0FFCC;
    return;
L_09B0FFC8:
    aot_mem.aot_store32(ctx.gpr[29] + static_cast<std::uint32_t>(28), std::bit_cast<std::uint32_t>(ctx.fpr[0]));
    goto L_09B0FFCC;
L_09B0FFCC:
    { const bool branch_taken = ctx.gpr[2] == 0u;
    // nop
      if (branch_taken) {
          goto L_09B0FFEC;
      }
      goto L_09B0FFD4;
    }
L_09B0FFD0:
    // nop
    goto L_09B0FFD4;
L_09B0FFD4:
    ctx.fpr[2] = std::bit_cast<float>(aot_mem.aot_load32(ctx.gpr[29] + static_cast<std::uint32_t>(20)));
    goto L_09B0FFD8;
L_09B0FFD8:
    ctx.fpr[1] = std::bit_cast<float>(aot_mem.aot_load32(ctx.gpr[29] + static_cast<std::uint32_t>(24)));
    goto L_09B0FFDC;
L_09B0FFDC:
    ctx.fpr[0] = std::bit_cast<float>(aot_mem.aot_load32(ctx.gpr[29] + static_cast<std::uint32_t>(28)));
    goto L_09B0FFE0;
L_09B0FFE0:
    aot_mem.aot_store32(ctx.gpr[16] + static_cast<std::uint32_t>(0), std::bit_cast<std::uint32_t>(ctx.fpr[2]));
    goto L_09B0FFE4;
L_09B0FFE4:
    aot_mem.aot_store32(ctx.gpr[16] + static_cast<std::uint32_t>(4), std::bit_cast<std::uint32_t>(ctx.fpr[1]));
    goto L_09B0FFE8;
L_09B0FFE8:
    aot_mem.aot_store32(ctx.gpr[16] + static_cast<std::uint32_t>(8), std::bit_cast<std::uint32_t>(ctx.fpr[0]));
    goto L_09B0FFEC;
L_09B0FFEC:
    ctx.gpr[4] = (2482u << 16u);
    goto L_09B0FFF0;
L_09B0FFF0:
    ctx.gpr[5] = (2478u << 16u);
    goto L_09B0FFF4;
L_09B0FFF4:
    ctx.gpr[6] = (2482u << 16u);
    goto L_09B0FFF8;
L_09B0FFF8:
    ctx.gpr[4] = (ctx.gpr[4] + static_cast<std::uint32_t>(15152));
    goto L_09B0FFFC;
L_09B0FFFC:
    ctx.gpr[5] = (ctx.gpr[5] + static_cast<std::uint32_t>(-13752));
    goto L_09B10000;
L_09B10000:
    ctx.gpr[31] = (0x09B10008u);
    ctx.gpr[6] = (ctx.gpr[6] + static_cast<std::uint32_t>(15136));
    ctx.pc = 0x0890C3F8u;
    if (rt.invoke_chained_call(ctx, &aot_mem) && ctx.pc == 0x09B10008u) goto L_09B10008;
    return;
L_09B10004:
    ctx.gpr[6] = (ctx.gpr[6] + static_cast<std::uint32_t>(15136));
    goto L_09B10008;
L_09B10008:
    ctx.gpr[31] = (aot_mem.aot_load32(ctx.gpr[29] + static_cast<std::uint32_t>(12)));
    goto L_09B1000C;
L_09B1000C:
    ctx.gpr[16] = (aot_mem.aot_load32(ctx.gpr[29] + static_cast<std::uint32_t>(8)));
    goto L_09B10010;
L_09B10010:
    jump_target = ctx.gpr[31];
    ctx.gpr[29] = (ctx.gpr[29] + static_cast<std::uint32_t>(32));
    local_pc = jump_target;
    if (++local_transfers < 2048u) { entry_id = 0u; goto LOCAL_DISPATCH; }
    ctx.pc = jump_target;
    return;
L_09B10014:
    ctx.gpr[29] = (ctx.gpr[29] + static_cast<std::uint32_t>(32));
    goto L_09B10018;
L_09B10018:
    ctx.gpr[4] = (2482u << 16u);
    goto L_09B1001C;
L_09B1001C:
    ctx.gpr[5] = (2479u << 16u);
    goto L_09B10020;
L_09B10020:
    ctx.gpr[6] = (2482u << 16u);
    goto L_09B10024;
L_09B10024:
    ctx.gpr[4] = (ctx.gpr[4] + static_cast<std::uint32_t>(15280));
    goto L_09B10028;
L_09B10028:
    ctx.gpr[2] = (2482u << 16u);
    goto L_09B1002C;
L_09B1002C:
    aot_mem.aot_store32(ctx.gpr[2] + static_cast<std::uint32_t>(15376), ctx.gpr[4]);
    goto L_09B10030;
L_09B10030:
    ctx.gpr[5] = (ctx.gpr[5] + static_cast<std::uint32_t>(-2712));
    goto L_09B10034;
L_09B10034:
    ctx.gpr[6] = (ctx.gpr[6] + static_cast<std::uint32_t>(15264));
    ctx.pc = 0x0890C3F8u; (void)rt.invoke_chained_call(ctx, &aot_mem); return;
L_09B10038:
    ctx.gpr[6] = (ctx.gpr[6] + static_cast<std::uint32_t>(15264));
    goto L_09B1003C;
L_09B1003C:
    // nop
    goto L_09B10040;
L_09B10040:
    rt.unsupported(0x09B10044u, 0x09B0FF18u, "control flow in delay slot"); return;
L_09B10044:
    rt.unsupported(0x09B10048u, 0x09B0FF54u, "control flow in delay slot"); return;
L_09B10048:
    rt.unsupported(0x09B1004Cu, 0x09B10018u, "control flow in delay slot"); return;
L_09B1004C:
    // nop
    ctx.pc = 0x06C40060u; (void)rt.invoke_chained_call(ctx, &aot_mem); return;
L_09B10050:
    // nop
    goto L_09B10054;
L_09B10054:
    // nop
    goto L_09B10058;
L_09B10058:
    // nop
    goto L_09B1005C;
L_09B1005C:
    // nop
    goto L_09B10060;
L_09B10060:
    // nop
    goto L_09B10064;
L_09B10064:
    // nop
    goto L_09B10068;
L_09B10068:
    // nop
    goto L_09B1006C;
L_09B1006C:
    // nop
    goto L_09B10070;
L_09B10070:
    // nop
    goto L_09B10074;
L_09B10074:
    // nop
    goto L_09B10078;
L_09B10078:
    // nop
    goto L_09B1007C;
L_09B1007C:
    // nop
    ctx.pc = 0x09B10080u; return;
}

void ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044(Runtime &rt, AllegrexContext &ctx) {
    auto aot_mem = rt.memory().aot_fast_view();
    ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044_entry(rt, ctx, 0u, aot_mem);
}

void register_ovl09A5F300_lobby_task_F431CAA134157E88_unit_44(Runtime &runtime) {
    runtime.register_function(0x09B0F300u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F304u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F308u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F30Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F310u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F314u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F318u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F31Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F320u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F324u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F328u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F32Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F330u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F334u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F338u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F33Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F340u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F344u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F348u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F34Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F350u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F354u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F358u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F35Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F360u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F364u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F368u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F36Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F370u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F374u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F378u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F37Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F380u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F384u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F388u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F38Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F390u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F394u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F398u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F39Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F3A0u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F3A4u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F3A8u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F3ACu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F3B0u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F3B4u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F3B8u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F3BCu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F3C0u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F3C4u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F3C8u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F3CCu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F3D0u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F3D4u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F3D8u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F3DCu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F3E0u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F3E4u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F3E8u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F3ECu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F3F0u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F3F4u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F3F8u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F3FCu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F400u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F404u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F408u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F40Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F410u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F414u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F418u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F41Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F420u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F424u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F428u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F42Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F430u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F434u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F438u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F43Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F440u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F444u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F448u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F44Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F450u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F454u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F458u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F45Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F460u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F464u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F468u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F46Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F470u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F474u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F478u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F47Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F480u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F484u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F488u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F48Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F490u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F494u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F498u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F49Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F4A0u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F4A4u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F4A8u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F4ACu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F4B0u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F4B4u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F4B8u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F4BCu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F4C0u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F4C4u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F4C8u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F4CCu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F4D0u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F4D4u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F4D8u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F4DCu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F4E0u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F4E4u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F4E8u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F4ECu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F4F0u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F4F4u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F4F8u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F4FCu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F500u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F504u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F508u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F50Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F510u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F514u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F518u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F51Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F520u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F524u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F528u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F52Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F530u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F534u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F538u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F53Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F540u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F544u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F548u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F54Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F550u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F554u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F558u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F55Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F560u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F564u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F568u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F56Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F570u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F574u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F578u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F57Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F580u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F584u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F588u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F58Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F590u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F594u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F598u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F59Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F5A0u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F5A4u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F5A8u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F5ACu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F5B0u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F5B4u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F5B8u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F5BCu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F5C0u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F5C4u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F5C8u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F5CCu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F5D0u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F5D4u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F5D8u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F5DCu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F5E0u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F5E4u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F5E8u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F5ECu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F5F0u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F5F4u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F5F8u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F5FCu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F600u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F604u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F608u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F60Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F610u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F614u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F618u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F61Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F620u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F624u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F628u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F62Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F630u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F634u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F638u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F63Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F640u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F644u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F648u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F64Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F650u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F654u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F658u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F65Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F660u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F664u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F668u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F66Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F670u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F674u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F678u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F67Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F680u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F684u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F688u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F68Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F690u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F694u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F698u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F69Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F6A0u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F6A4u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F6A8u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F6ACu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F6B0u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F6B4u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F6B8u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F6BCu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F6C0u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F6C4u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F6C8u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F6CCu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F6D0u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F6D4u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F6D8u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F6DCu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F6E0u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F6E4u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F6E8u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F6ECu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F6F0u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F6F4u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F6F8u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F6FCu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F700u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F704u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F708u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F70Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F710u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F714u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F718u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F71Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F720u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F724u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F728u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F72Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F730u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F734u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F738u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F73Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F740u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F744u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F748u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F74Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F750u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F754u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F758u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F75Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F760u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F764u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F768u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F76Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F770u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F774u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F778u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F77Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F780u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F784u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F788u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F78Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F790u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F794u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F798u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F79Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F7A0u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F7A4u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F7A8u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F7ACu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F7B0u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F7B4u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F7B8u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F7BCu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F7C0u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F7C4u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F7C8u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F7CCu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F7D0u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F7D4u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F7D8u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F7DCu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F7E0u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F7E4u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F7E8u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F7ECu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F7F0u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F7F4u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F7F8u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F7FCu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F800u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F804u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F808u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F80Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F810u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F814u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F818u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F81Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F820u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F824u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F828u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F82Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F830u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F834u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F838u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F83Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F840u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F844u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F848u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F84Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F850u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F854u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F858u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F85Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F860u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F864u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F868u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F86Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F870u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F874u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F878u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F87Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F880u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F884u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F888u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F88Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F890u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F894u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F898u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F89Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F8A0u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F8A4u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F8A8u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F8ACu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F8B0u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F8B4u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F8B8u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F8BCu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F8C0u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F8C4u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F8C8u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F8CCu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F8D0u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F8D4u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F8D8u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F8DCu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F8E0u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F8E4u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F8E8u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F8ECu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F8F0u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F8F4u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F8F8u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F8FCu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F900u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F904u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F908u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F90Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F910u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F914u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F918u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F91Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F920u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F924u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F928u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F92Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F930u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F934u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F938u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F93Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F940u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F944u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F948u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F94Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F950u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F954u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F958u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F95Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F960u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F964u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F968u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F96Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F970u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F974u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F978u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F97Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F980u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F984u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F988u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F98Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F990u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F994u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F998u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F99Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F9A0u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F9A4u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F9A8u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F9ACu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F9B0u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F9B4u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F9B8u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F9BCu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F9C0u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F9C4u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F9C8u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F9CCu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F9D0u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F9D4u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F9D8u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F9DCu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F9E0u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F9E4u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F9E8u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F9ECu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F9F0u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F9F4u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F9F8u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0F9FCu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FA00u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FA04u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FA08u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FA0Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FA10u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FA14u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FA18u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FA1Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FA20u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FA24u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FA28u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FA2Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FA30u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FA34u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FA38u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FA3Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FA40u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FA44u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FA48u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FA4Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FA50u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FA54u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FA58u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FA5Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FA60u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FA64u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FA68u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FA6Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FA70u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FA74u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FA78u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FA7Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FA80u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FA84u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FA88u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FA8Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FA90u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FA94u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FA98u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FA9Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FAA0u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FAA4u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FAA8u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FAACu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FAB0u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FAB4u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FAB8u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FABCu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FAC0u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FAC4u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FAC8u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FACCu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FAD0u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FAD4u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FAD8u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FADCu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FAE0u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FAE4u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FAE8u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FAECu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FAF0u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FAF4u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FAF8u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FAFCu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FB00u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FB04u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FB08u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FB0Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FB10u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FB14u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FB18u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FB1Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FB20u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FB24u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FB28u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FB2Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FB30u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FB34u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FB38u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FB3Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FB40u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FB44u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FB48u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FB4Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FB50u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FB54u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FB58u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FB5Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FB60u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FB64u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FB68u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FB6Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FB70u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FB74u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FB78u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FB7Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FB80u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FB84u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FB88u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FB8Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FB90u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FB94u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FB98u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FB9Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FBA0u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FBA4u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FBA8u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FBACu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FBB0u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FBB4u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FBB8u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FBBCu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FBC0u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FBC4u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FBC8u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FBCCu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FBD0u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FBD4u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FBD8u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FBDCu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FBE0u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FBE4u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FBE8u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FBECu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FBF0u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FBF4u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FBF8u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FBFCu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FC00u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FC04u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FC08u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FC0Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FC10u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FC14u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FC18u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FC1Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FC20u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FC24u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FC28u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FC2Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FC30u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FC34u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FC38u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FC3Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FC40u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FC44u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FC48u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FC4Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FC50u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FC54u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FC58u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FC5Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FC60u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FC64u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FC68u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FC6Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FC70u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FC74u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FC78u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FC7Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FC80u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FC84u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FC88u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FC8Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FC90u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FC94u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FC98u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FC9Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FCA0u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FCA4u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FCA8u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FCACu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FCB0u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FCB4u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FCB8u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FCBCu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FCC0u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FCC4u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FCC8u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FCCCu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FCD0u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FCD4u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FCD8u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FCDCu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FCE0u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FCE4u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FCE8u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FCECu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FCF0u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FCF4u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FCF8u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FCFCu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FD00u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FD04u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FD08u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FD0Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FD10u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FD14u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FD18u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FD1Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FD20u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FD24u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FD28u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FD2Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FD30u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FD34u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FD38u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FD3Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FD40u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FD44u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FD48u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FD4Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FD50u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FD54u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FD58u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FD5Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FD60u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FD64u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FD68u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FD6Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FD70u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FD74u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FD78u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FD7Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FD80u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FD84u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FD88u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FD8Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FD90u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FD94u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FD98u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FD9Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FDA0u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FDA4u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FDA8u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FDACu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FDB0u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FDB4u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FDB8u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FDBCu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FDC0u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FDC4u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FDC8u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FDCCu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FDD0u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FDD4u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FDD8u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FDDCu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FDE0u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FDE4u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FDE8u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FDECu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FDF0u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FDF4u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FDF8u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FDFCu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FE00u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FE04u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FE08u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FE0Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FE10u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FE14u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FE18u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FE1Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FE20u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FE24u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FE28u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FE2Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FE30u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FE34u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FE38u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FE3Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FE40u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FE44u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FE48u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FE4Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FE50u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FE54u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FE58u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FE5Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FE60u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FE64u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FE68u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FE6Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FE70u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FE74u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FE78u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FE7Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FE80u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FE84u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FE88u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FE8Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FE90u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FE94u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FE98u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FE9Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FEA0u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FEA4u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FEA8u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FEACu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FEB0u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FEB4u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FEB8u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FEBCu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FEC0u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FEC4u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FEC8u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FECCu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FED0u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FED4u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FED8u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FEDCu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FEE0u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FEE4u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FEE8u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FEECu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FEF0u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FEF4u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FEF8u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FEFCu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FF00u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FF04u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FF08u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FF0Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FF10u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FF14u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FF18u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FF1Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FF20u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FF24u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FF28u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FF2Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FF30u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FF34u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FF38u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FF3Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FF40u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FF44u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FF48u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FF4Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FF50u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FF54u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FF58u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FF5Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FF60u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FF64u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FF68u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FF6Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FF70u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FF74u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FF78u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FF7Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FF80u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FF84u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FF88u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FF8Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FF90u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FF94u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FF98u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FF9Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FFA0u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FFA4u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FFA8u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FFACu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FFB0u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FFB4u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FFB8u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FFBCu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FFC0u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FFC4u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FFC8u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FFCCu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FFD0u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FFD4u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FFD8u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FFDCu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FFE0u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FFE4u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FFE8u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FFECu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FFF0u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FFF4u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FFF8u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B0FFFCu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B10000u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B10004u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B10008u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B1000Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B10010u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B10014u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B10018u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B1001Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B10020u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B10024u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B10028u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B1002Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B10030u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B10034u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B10038u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B1003Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B10040u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B10044u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B10048u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B1004Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B10050u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B10054u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B10058u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B1005Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B10060u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B10064u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B10068u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B1006Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B10070u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B10074u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B10078u, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
    runtime.register_function(0x09B1007Cu, &ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044, "recomp_unit_ovl09A5F300_lobby_task_F431CAA134157E88_unit_0044");
}
} // namespace psprecomp
