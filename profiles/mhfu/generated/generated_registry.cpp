#include "psprecomp/runtime.hpp"
#include <cstdint>

namespace psprecomp {
void register_recomp_unit_0(Runtime &runtime);
void register_recomp_unit_1(Runtime &runtime);
void register_recomp_unit_2(Runtime &runtime);
void register_recomp_unit_3(Runtime &runtime);
void register_recomp_unit_4(Runtime &runtime);
void register_recomp_unit_5(Runtime &runtime);
void register_recomp_unit_6(Runtime &runtime);
void register_recomp_unit_7(Runtime &runtime);
void register_recomp_unit_8(Runtime &runtime);
void register_recomp_unit_9(Runtime &runtime);
void register_recomp_unit_10(Runtime &runtime);
void register_recomp_unit_11(Runtime &runtime);
void register_recomp_unit_12(Runtime &runtime);
void register_recomp_unit_13(Runtime &runtime);
void register_recomp_unit_14(Runtime &runtime);
void register_recomp_unit_15(Runtime &runtime);
void register_recomp_unit_16(Runtime &runtime);
void register_recomp_unit_17(Runtime &runtime);
void register_recomp_unit_18(Runtime &runtime);
void register_recomp_unit_19(Runtime &runtime);
void register_recomp_unit_20(Runtime &runtime);
void register_recomp_unit_21(Runtime &runtime);
void register_recomp_unit_22(Runtime &runtime);
void register_recomp_unit_23(Runtime &runtime);
void register_recomp_unit_24(Runtime &runtime);
void register_recomp_unit_25(Runtime &runtime);
void register_recomp_unit_26(Runtime &runtime);
void register_recomp_unit_27(Runtime &runtime);
void register_recomp_unit_28(Runtime &runtime);
void register_recomp_unit_29(Runtime &runtime);
void register_recomp_unit_30(Runtime &runtime);
void register_recomp_unit_31(Runtime &runtime);
void register_recomp_unit_32(Runtime &runtime);
void register_recomp_unit_33(Runtime &runtime);
void register_recomp_unit_34(Runtime &runtime);
void register_recomp_unit_35(Runtime &runtime);
void register_recomp_unit_36(Runtime &runtime);
void register_recomp_unit_37(Runtime &runtime);
void register_recomp_unit_38(Runtime &runtime);
void register_recomp_unit_39(Runtime &runtime);
void register_recomp_unit_40(Runtime &runtime);
void register_recomp_unit_41(Runtime &runtime);
void register_recomp_unit_42(Runtime &runtime);
void register_recomp_unit_43(Runtime &runtime);
void register_recomp_unit_44(Runtime &runtime);
void register_recomp_unit_45(Runtime &runtime);
void register_recomp_unit_46(Runtime &runtime);
void register_recomp_unit_47(Runtime &runtime);
void register_recomp_unit_48(Runtime &runtime);
void register_recomp_unit_49(Runtime &runtime);
void register_recomp_unit_50(Runtime &runtime);
void register_recomp_unit_51(Runtime &runtime);
void register_recomp_unit_52(Runtime &runtime);
void register_recomp_unit_53(Runtime &runtime);
void register_recomp_unit_54(Runtime &runtime);
void register_recomp_unit_55(Runtime &runtime);
void register_recomp_unit_56(Runtime &runtime);
void register_recomp_unit_57(Runtime &runtime);
void register_recomp_unit_58(Runtime &runtime);
void register_recomp_unit_59(Runtime &runtime);
void register_recomp_unit_60(Runtime &runtime);
void register_recomp_unit_61(Runtime &runtime);
void register_recomp_unit_62(Runtime &runtime);
void register_recomp_unit_63(Runtime &runtime);
void register_recomp_unit_64(Runtime &runtime);
void register_recomp_unit_65(Runtime &runtime);
void register_recomp_unit_66(Runtime &runtime);

static void import_0(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(0u, "InterruptManager", 0xCA04A2B9u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_1(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(1u, "InterruptManager", 0xD61E6961u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_2(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(2u, "InterruptManager", 0xFB8E22ECu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_3(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(3u, "IoFileMgrForUser", 0x42EC03ACu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_4(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(4u, "IoFileMgrForUser", 0x54F5FB11u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_5(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(5u, "IoFileMgrForUser", 0x6A638D83u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_6(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(6u, "IoFileMgrForUser", 0x779103A0u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_7(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(7u, "IoFileMgrForUser", 0x810C4BC3u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_8(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(8u, "IoFileMgrForUser", 0xACE946E8u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_9(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(9u, "IoFileMgrForUser", 0xB29DDF9Cu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_10(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(10u, "IoFileMgrForUser", 0xEB092469u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_11(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(11u, "IoFileMgrForUser", 0x109F50BCu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_12(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(12u, "IoFileMgrForUser", 0x27EB27B8u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_13(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(13u, "Kernel_Library", 0x092968F4u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_14(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(14u, "Kernel_Library", 0x5F10D406u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_15(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(15u, "LoadExecForUser", 0x05572A5Fu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_16(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(16u, "LoadExecForUser", 0x4AC57943u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_17(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(17u, "ModuleMgrForUser", 0x2E0911AAu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_18(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(18u, "ModuleMgrForUser", 0xD8B73127u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_19(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(19u, "ModuleMgrForUser", 0xF0A26395u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_20(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(20u, "ModuleMgrForUser", 0x50F0C1ECu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_21(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(21u, "ModuleMgrForUser", 0x8F2DF740u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_22(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(22u, "ModuleMgrForUser", 0xB7F46618u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_23(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(23u, "ModuleMgrForUser", 0xD1FF982Au, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_24(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(24u, "StdioForUser", 0x172D316Eu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_25(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(25u, "StdioForUser", 0xA6BAB2E9u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_26(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(26u, "StdioForUser", 0xF78BA90Au, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_27(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(27u, "SysMemUserForUser", 0x13A5ABEFu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_28(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(28u, "SysMemUserForUser", 0x237DBD4Fu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_29(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(29u, "SysMemUserForUser", 0x91DE343Cu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_30(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(30u, "SysMemUserForUser", 0x9D9A5BA1u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_31(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(31u, "SysMemUserForUser", 0xB6D61D02u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_32(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(32u, "SysMemUserForUser", 0xF77D77CBu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_33(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(33u, "ThreadManForUser", 0xC68D9437u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_34(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(34u, "ThreadManForUser", 0xCEADEB47u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_35(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(35u, "ThreadManForUser", 0xD59EAD2Fu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_36(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(36u, "ThreadManForUser", 0xD6DA4BA1u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_37(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(37u, "ThreadManForUser", 0xE1619D7Cu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_38(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(38u, "ThreadManForUser", 0x1FB15A32u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_39(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(39u, "ThreadManForUser", 0xE81CAF8Fu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_40(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(40u, "ThreadManForUser", 0xEA748E31u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_41(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(41u, "ThreadManForUser", 0xEF9E4C70u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_42(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(42u, "ThreadManForUser", 0xF475845Du, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_43(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(43u, "ThreadManForUser", 0x20FFF560u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_44(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(44u, "ThreadManForUser", 0x27E22EC2u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_45(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(45u, "ThreadManForUser", 0x28B6489Cu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_46(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(46u, "ThreadManForUser", 0x293B45B8u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_47(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(47u, "ThreadManForUser", 0x30FD48F0u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_48(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(48u, "ThreadManForUser", 0x369ED59Du, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_49(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(49u, "ThreadManForUser", 0x383F7BCCu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_50(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(50u, "ThreadManForUser", 0x3AD58B8Cu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_51(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(51u, "ThreadManForUser", 0x3B183E26u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_52(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(52u, "ThreadManForUser", 0x3F53E640u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_53(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(53u, "ThreadManForUser", 0x402FCF22u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_54(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(54u, "ThreadManForUser", 0x446D8DE6u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_55(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(55u, "ThreadManForUser", 0x4E3A1105u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_56(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(56u, "ThreadManForUser", 0x53B00E9Au, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_57(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(57u, "ThreadManForUser", 0x55C20A00u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_58(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(58u, "ThreadManForUser", 0x58B1F937u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_59(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(59u, "ThreadManForUser", 0x616403BAu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_60(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(60u, "ThreadManForUser", 0x68DA9E36u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_61(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(61u, "ThreadManForUser", 0x71BC9871u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_62(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(62u, "ThreadManForUser", 0x809CE29Bu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_63(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(63u, "ThreadManForUser", 0x812346E4u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_64(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(64u, "ThreadManForUser", 0x82826F70u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_65(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(65u, "ThreadManForUser", 0x82BC5777u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_66(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(66u, "ThreadManForUser", 0x94AA61EEu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_67(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(67u, "ThreadManForUser", 0x9ACE131Eu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_68(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(68u, "ThreadManForUser", 0x9FA03CD3u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_69(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(69u, "ThreadManForUser", 0xAA73C935u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_70(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(70u, "UtilsForUser", 0x71EC4271u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_71(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(71u, "UtilsForUser", 0x79D1C3FAu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_72(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(72u, "UtilsForUser", 0x91E4F6A7u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_73(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(73u, "UtilsForUser", 0x920F104Au, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_74(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(74u, "UtilsForUser", 0xB435DEC5u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_75(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(75u, "UtilsForUser", 0xBFA98062u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_76(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(76u, "UtilsForUser", 0x27CC57F0u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_77(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(77u, "UtilsForUser", 0x3EE30821u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_78(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(78u, "sceSuspendForUser", 0x090CCB3Fu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_79(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(79u, "sceSuspendForUser", 0x3E0271D3u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_80(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(80u, "sceSuspendForUser", 0xA569E425u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_81(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(81u, "sceGe_user", 0x03444EB4u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_82(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(82u, "sceGe_user", 0x05DB22CEu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_83(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(83u, "sceGe_user", 0x1F6752ADu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_84(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(84u, "sceGe_user", 0xA4FC06A4u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_85(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(85u, "sceGe_user", 0xAB49E76Au, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_86(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(86u, "sceGe_user", 0xB287BD61u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_87(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(87u, "sceGe_user", 0xB77905EAu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_88(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(88u, "sceGe_user", 0xE0D68148u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_89(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(89u, "sceGe_user", 0xE47E40E4u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_90(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(90u, "sceDisplay", 0x0E20F177u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_91(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(91u, "sceDisplay", 0x289D82FEu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_92(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(92u, "sceCtrl", 0x1F4011E6u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_93(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(93u, "sceCtrl", 0x1F803938u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_94(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(94u, "sceCtrl", 0x6A2774F3u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_95(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(95u, "sceLibFont", 0x099EF33Cu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_96(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(96u, "sceLibFont", 0x0DA7535Eu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_97(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(97u, "sceLibFont", 0x27F6E642u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_98(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(98u, "sceLibFont", 0x3AEA8CB6u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_99(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(99u, "sceLibFont", 0x574B6FBCu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_100(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(100u, "sceLibFont", 0x67F17ED7u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_101(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(101u, "sceLibFont", 0x980F4895u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_102(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(102u, "sceLibFont", 0xA834319Du, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_103(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(103u, "sceLibFont", 0xDCC80C2Fu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_104(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(104u, "sceSasCore", 0x267A6DD2u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_105(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(105u, "sceSasCore", 0x33D4AB37u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_106(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(106u, "sceSasCore", 0x42778A9Fu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_107(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(107u, "sceSasCore", 0x440CA7D8u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_108(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(108u, "sceSasCore", 0x68A46B95u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_109(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(109u, "sceSasCore", 0x76F01ACAu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_110(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(110u, "sceSasCore", 0x99944089u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_111(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(111u, "sceSasCore", 0xA0CF2FA4u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_112(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(112u, "sceSasCore", 0xA3589D81u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_113(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(113u, "sceSasCore", 0xAD84D37Fu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_114(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(114u, "sceSasCore", 0xCBCD4F79u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_115(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(115u, "sceSasCore", 0xD5A229C9u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_116(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(116u, "sceSasCore", 0xE175EF66u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_117(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(117u, "sceSasCore", 0xF983B186u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_118(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(118u, "sceAudio", 0x13F592BCu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_119(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(119u, "sceAudio", 0x5EC81C55u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_120(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(120u, "sceAudio", 0x6FC46853u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_121(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(121u, "sceAudio", 0x95FD0C2Du, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_122(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(122u, "sceAudio", 0xB011922Fu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_123(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(123u, "sceAudio", 0xB7E1D8E7u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_124(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(124u, "sceAudio", 0xCB2E439Eu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_125(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(125u, "sceUmdUser", 0x20628E6Fu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_126(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(126u, "sceUmdUser", 0x6B4A146Cu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_127(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(127u, "sceUmdUser", 0xC6183D47u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_128(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(128u, "sceMpeg", 0x0E3C2E9Du, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_129(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(129u, "sceMpeg", 0x13407F13u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_130(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(130u, "sceMpeg", 0x167AFD9Eu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_131(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(131u, "sceMpeg", 0x21FF80E4u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_132(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(132u, "sceMpeg", 0x37295ED8u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_133(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(133u, "sceMpeg", 0x42560F23u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_134(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(134u, "sceMpeg", 0x591A4AA2u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_135(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(135u, "sceMpeg", 0x606A4649u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_136(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(136u, "sceMpeg", 0x611E9E11u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_137(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(137u, "sceMpeg", 0x682A619Bu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_138(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(138u, "sceMpeg", 0x707B7629u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_139(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(139u, "sceMpeg", 0x740FCCD1u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_140(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(140u, "sceMpeg", 0x800C44DFu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_141(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(141u, "sceMpeg", 0x874624D6u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_142(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(142u, "sceMpeg", 0xA780CF7Eu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_143(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(143u, "sceMpeg", 0xB240A59Eu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_144(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(144u, "sceMpeg", 0xB5F6DC87u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_145(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(145u, "sceMpeg", 0xC132E22Fu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_146(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(146u, "sceMpeg", 0xCEB870B1u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_147(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(147u, "sceMpeg", 0xD7A29F46u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_148(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(148u, "sceMpeg", 0xD8C5F121u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_149(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(149u, "sceMpeg", 0xE1CE83A7u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_150(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(150u, "sceMpeg", 0xF8DCB679u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_151(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(151u, "sceMpeg", 0xFE246728u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_152(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(152u, "sceRtc", 0xE7C27D1Bu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_153(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(153u, "sceAtrac3plus", 0x5D268707u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_154(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(154u, "sceAtrac3plus", 0x61EB33F5u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_155(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(155u, "sceAtrac3plus", 0x6A8C3CD5u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_156(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(156u, "sceAtrac3plus", 0x7A20E7AFu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_157(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(157u, "sceAtrac3plus", 0x868120B5u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_158(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(158u, "sceAtrac3plus", 0x9AE849A7u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_159(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(159u, "sceAtrac3plus", 0xA2BBA8BEu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_160(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(160u, "sceAtrac3plus", 0xA554A158u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_161(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(161u, "sceAtrac3plus", 0xE23E3A35u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_162(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(162u, "sceAtrac3plus", 0xFAA4F89Bu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_163(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(163u, "sceUtility", 0x2A2B3DE0u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_164(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(164u, "sceUtility", 0x2AD8E239u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_165(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(165u, "sceUtility", 0x3DFAEBA9u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_166(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(166u, "sceUtility", 0x4B85C861u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_167(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(167u, "sceUtility", 0x4DB1E739u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_168(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(168u, "sceUtility", 0x50C4CD57u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_169(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(169u, "sceUtility", 0x6332AA39u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_170(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(170u, "sceUtility", 0x67AF3428u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_171(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(171u, "sceUtility", 0x8874DBE0u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_172(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(172u, "sceUtility", 0x91E70E35u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_173(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(173u, "sceUtility", 0x95FC253Bu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_174(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(174u, "sceUtility", 0x9790B33Cu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_175(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(175u, "sceUtility", 0x9A1C91D7u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_176(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(176u, "sceUtility", 0xA5DA2406u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_177(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(177u, "sceUtility", 0xD4B95FFBu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_178(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(178u, "sceUtility", 0xE49BFE92u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_179(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(179u, "sceUtility", 0xF3F76017u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_180(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(180u, "sceUtility", 0xF6269B82u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_181(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(181u, "sceUtility", 0xF88155F6u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_182(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(182u, "sceNetAdhoc", 0x157E6225u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_183(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(183u, "sceNetAdhoc", 0x4DA4C788u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_184(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(184u, "sceNetAdhoc", 0x6F92741Bu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_185(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(185u, "sceNetAdhoc", 0x7F27BB5Eu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_186(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(186u, "sceNetAdhoc", 0x877F6D66u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_187(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(187u, "sceNetAdhoc", 0x8BEA2B3Eu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_188(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(188u, "sceNetAdhoc", 0x9AC2EEACu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_189(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(189u, "sceNetAdhoc", 0x9DF81198u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_190(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(190u, "sceNetAdhoc", 0xA62C6F57u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_191(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(191u, "sceNetAdhoc", 0xABED3790u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_192(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(192u, "sceNetAdhoc", 0xB9685118u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_193(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(193u, "sceNetAdhoc", 0xDFE53E03u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_194(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(194u, "sceNetAdhoc", 0xE08BDAC1u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_195(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(195u, "sceNetAdhoc", 0xE1D621D7u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_196(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(196u, "sceNetAdhoc", 0xFC6FC07Bu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_197(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(197u, "sceNet", 0x0BF0A3AEu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_198(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(198u, "sceNet", 0x281928A9u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_199(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(199u, "sceNet", 0x39AF39A6u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_200(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(200u, "sceNet", 0x50647530u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_201(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(201u, "sceWlanDrv", 0x0C622081u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_202(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(202u, "sceWlanDrv", 0xD7763699u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_203(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(203u, "sceNetAdhocctl", 0x08FFF7A0u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_204(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(204u, "sceNetAdhocctl", 0x20B317A0u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_205(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(205u, "sceNetAdhocctl", 0x34401D65u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_206(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(206u, "sceNetAdhocctl", 0x6402490Bu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_207(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(207u, "sceNetAdhocctl", 0x81AEE1BEu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_208(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(208u, "sceNetAdhocctl", 0x9D689E13u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_209(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(209u, "sceNetAdhocctl", 0xE162CB14u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_210(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(210u, "sceNetAdhocctl", 0xE26F226Eu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_211(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(211u, "sceNetApctl", 0x24FE91A1u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_212(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(212u, "sceNetApctl", 0x2BEFDF23u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_213(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(213u, "sceNetApctl", 0x5963991Bu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_214(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(214u, "sceNetApctl", 0x8ABADD51u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_215(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(215u, "sceNetApctl", 0xB3EDD0ECu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_216(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(216u, "sceNetApctl", 0xE2F91F9Bu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_217(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(217u, "sceNetInet", 0x17943399u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_218(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(218u, "sceNetInet", 0x1A33F9AEu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_219(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(219u, "sceNetInet", 0x2FE71FE7u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_220(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(220u, "sceNetInet", 0x410B34AAu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_221(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(221u, "sceNetInet", 0x7AA671BCu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_222(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(222u, "sceNetInet", 0x805502DDu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_223(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(223u, "sceNetInet", 0x8B7B220Fu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_224(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(224u, "sceNetInet", 0x8CA3A97Eu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_225(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(225u, "sceNetInet", 0x8D7284EAu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_226(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(226u, "sceNetInet", 0xA9ED66B9u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_227(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(227u, "sceNetInet", 0xB3888AD4u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_228(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(228u, "sceNetInet", 0xB75D5B0Au, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_229(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(229u, "sceNetInet", 0xCDA85C99u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_230(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(230u, "sceNetInet", 0xD10A1A7Au, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_231(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(231u, "sceNetInet", 0xDB094E1Bu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_232(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(232u, "sceNetInet", 0xE247B6D6u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_233(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(233u, "scePower", 0x04B7766Eu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_234(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(234u, "sceOpenPSID", 0xC69BEBCEu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_235(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(235u, "sceImpose", 0x36AA6E91u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_236(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(236u, "sceDmac", 0x617F3FE6u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_237(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(237u, "sceNetAdhocDiscover", 0x52DE1B97u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_238(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(238u, "sceNetAdhocDiscover", 0x941B3877u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_239(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(239u, "sceNetAdhocDiscover", 0x944DDBC6u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_240(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(240u, "sceNetAdhocDiscover", 0xA2246614u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_241(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(241u, "sceNetAdhocDiscover", 0xA423A21Bu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_242(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(242u, "sceHttp", 0x0282A3BDu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_243(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(243u, "sceHttp", 0x0B12ABFBu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_244(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(244u, "sceHttp", 0x1F0FC3E3u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_245(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(245u, "sceHttp", 0x2255551Eu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_246(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(246u, "sceHttp", 0x3EABA285u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_247(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(247u, "sceHttp", 0x4CC7D78Fu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_248(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(248u, "sceHttp", 0x5152773Bu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_249(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(249u, "sceHttp", 0x87797BDDu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_250(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(250u, "sceHttp", 0x8ACD1F73u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_251(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(251u, "sceHttp", 0x9988172Du, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_252(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(252u, "sceHttp", 0x9B1F1F36u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_253(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(253u, "sceHttp", 0xA4496DE5u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_254(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(254u, "sceHttp", 0xA5512E01u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_255(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(255u, "sceHttp", 0xAB1540D5u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_256(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(256u, "sceHttp", 0xAB1ABE07u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_257(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(257u, "sceHttp", 0xAE948FEEu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_258(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(258u, "sceHttp", 0xB509B09Eu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_259(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(259u, "sceHttp", 0xBB70706Fu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_260(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(260u, "sceHttp", 0xC10B6BD9u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_261(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(261u, "sceHttp", 0xCCBD167Au, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_262(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(262u, "sceHttp", 0xCDF8ECB9u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_263(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(263u, "sceHttp", 0xD1C8945Eu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_264(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(264u, "sceHttp", 0xDB266CCFu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_265(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(265u, "sceHttp", 0xE4D21302u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_266(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(266u, "sceHttp", 0xEDEEB999u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_267(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(267u, "sceHttp", 0xF49934F6u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_268(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(268u, "sceHttp", 0xF9D8EB63u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_269(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(269u, "sceHttp", 0xFCF8C055u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_270(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(270u, "sceSsl", 0x191CDEFFu, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

static void import_271(Runtime &rt, AllegrexContext &ctx) {
    const RuntimeExecutionContextToken caller_context = capture_runtime_execution_context();
    const std::uint32_t import_pc = ctx.pc;
    const std::uint32_t return_address = ctx.gpr[31];
    rt.invoke_import_cached(271u, "sceSsl", 0x957ECBE2u, ctx);
    if (!rt.stopped() && runtime_execution_context_matches(caller_context) &&
        ctx.pc == import_pc) ctx.pc = return_address;
}

void register_generated_functions(Runtime &runtime) {
    register_recomp_unit_0(runtime);
    register_recomp_unit_1(runtime);
    register_recomp_unit_2(runtime);
    register_recomp_unit_3(runtime);
    register_recomp_unit_4(runtime);
    register_recomp_unit_5(runtime);
    register_recomp_unit_6(runtime);
    register_recomp_unit_7(runtime);
    register_recomp_unit_8(runtime);
    register_recomp_unit_9(runtime);
    register_recomp_unit_10(runtime);
    register_recomp_unit_11(runtime);
    register_recomp_unit_12(runtime);
    register_recomp_unit_13(runtime);
    register_recomp_unit_14(runtime);
    register_recomp_unit_15(runtime);
    register_recomp_unit_16(runtime);
    register_recomp_unit_17(runtime);
    register_recomp_unit_18(runtime);
    register_recomp_unit_19(runtime);
    register_recomp_unit_20(runtime);
    register_recomp_unit_21(runtime);
    register_recomp_unit_22(runtime);
    register_recomp_unit_23(runtime);
    register_recomp_unit_24(runtime);
    register_recomp_unit_25(runtime);
    register_recomp_unit_26(runtime);
    register_recomp_unit_27(runtime);
    register_recomp_unit_28(runtime);
    register_recomp_unit_29(runtime);
    register_recomp_unit_30(runtime);
    register_recomp_unit_31(runtime);
    register_recomp_unit_32(runtime);
    register_recomp_unit_33(runtime);
    register_recomp_unit_34(runtime);
    register_recomp_unit_35(runtime);
    register_recomp_unit_36(runtime);
    register_recomp_unit_37(runtime);
    register_recomp_unit_38(runtime);
    register_recomp_unit_39(runtime);
    register_recomp_unit_40(runtime);
    register_recomp_unit_41(runtime);
    register_recomp_unit_42(runtime);
    register_recomp_unit_43(runtime);
    register_recomp_unit_44(runtime);
    register_recomp_unit_45(runtime);
    register_recomp_unit_46(runtime);
    register_recomp_unit_47(runtime);
    register_recomp_unit_48(runtime);
    register_recomp_unit_49(runtime);
    register_recomp_unit_50(runtime);
    register_recomp_unit_51(runtime);
    register_recomp_unit_52(runtime);
    register_recomp_unit_53(runtime);
    register_recomp_unit_54(runtime);
    register_recomp_unit_55(runtime);
    register_recomp_unit_56(runtime);
    register_recomp_unit_57(runtime);
    register_recomp_unit_58(runtime);
    register_recomp_unit_59(runtime);
    register_recomp_unit_60(runtime);
    register_recomp_unit_61(runtime);
    register_recomp_unit_62(runtime);
    register_recomp_unit_63(runtime);
    register_recomp_unit_64(runtime);
    register_recomp_unit_65(runtime);
    register_recomp_unit_66(runtime);
    runtime.register_function(0x0890DF58u, &import_0, "InterruptManager::0xCA04A2B9");
    runtime.register_function(0x0890DF60u, &import_1, "InterruptManager::0xD61E6961");
    runtime.register_function(0x0890DF68u, &import_2, "InterruptManager::0xFB8E22EC");
    runtime.register_function(0x0890DF70u, &import_3, "IoFileMgrForUser::0x42EC03AC");
    runtime.register_function(0x0890DF78u, &import_4, "IoFileMgrForUser::0x54F5FB11");
    runtime.register_function(0x0890DF80u, &import_5, "IoFileMgrForUser::0x6A638D83");
    runtime.register_function(0x0890DF88u, &import_6, "IoFileMgrForUser::0x779103A0");
    runtime.register_function(0x0890DF90u, &import_7, "IoFileMgrForUser::0x810C4BC3");
    runtime.register_function(0x0890DF98u, &import_8, "IoFileMgrForUser::0xACE946E8");
    runtime.register_function(0x0890DFA0u, &import_9, "IoFileMgrForUser::0xB29DDF9C");
    runtime.register_function(0x0890DFA8u, &import_10, "IoFileMgrForUser::0xEB092469");
    runtime.register_function(0x0890DFB0u, &import_11, "IoFileMgrForUser::0x109F50BC");
    runtime.register_function(0x0890DFB8u, &import_12, "IoFileMgrForUser::0x27EB27B8");
    runtime.register_function(0x0890DFC0u, &import_13, "Kernel_Library::0x092968F4");
    runtime.register_function(0x0890DFC8u, &import_14, "Kernel_Library::0x5F10D406");
    runtime.register_function(0x0890DFD0u, &import_15, "LoadExecForUser::0x05572A5F");
    runtime.register_function(0x0890DFD8u, &import_16, "LoadExecForUser::0x4AC57943");
    runtime.register_function(0x0890DFE0u, &import_17, "ModuleMgrForUser::0x2E0911AA");
    runtime.register_function(0x0890DFE8u, &import_18, "ModuleMgrForUser::0xD8B73127");
    runtime.register_function(0x0890DFF0u, &import_19, "ModuleMgrForUser::0xF0A26395");
    runtime.register_function(0x0890DFF8u, &import_20, "ModuleMgrForUser::0x50F0C1EC");
    runtime.register_function(0x0890E000u, &import_21, "ModuleMgrForUser::0x8F2DF740");
    runtime.register_function(0x0890E008u, &import_22, "ModuleMgrForUser::0xB7F46618");
    runtime.register_function(0x0890E010u, &import_23, "ModuleMgrForUser::0xD1FF982A");
    runtime.register_function(0x0890E018u, &import_24, "StdioForUser::0x172D316E");
    runtime.register_function(0x0890E020u, &import_25, "StdioForUser::0xA6BAB2E9");
    runtime.register_function(0x0890E028u, &import_26, "StdioForUser::0xF78BA90A");
    runtime.register_function(0x0890E030u, &import_27, "SysMemUserForUser::0x13A5ABEF");
    runtime.register_function(0x0890E038u, &import_28, "SysMemUserForUser::0x237DBD4F");
    runtime.register_function(0x0890E040u, &import_29, "SysMemUserForUser::0x91DE343C");
    runtime.register_function(0x0890E048u, &import_30, "SysMemUserForUser::0x9D9A5BA1");
    runtime.register_function(0x0890E050u, &import_31, "SysMemUserForUser::0xB6D61D02");
    runtime.register_function(0x0890E058u, &import_32, "SysMemUserForUser::0xF77D77CB");
    runtime.register_function(0x0890E060u, &import_33, "ThreadManForUser::0xC68D9437");
    runtime.register_function(0x0890E068u, &import_34, "ThreadManForUser::0xCEADEB47");
    runtime.register_function(0x0890E070u, &import_35, "ThreadManForUser::0xD59EAD2F");
    runtime.register_function(0x0890E078u, &import_36, "ThreadManForUser::0xD6DA4BA1");
    runtime.register_function(0x0890E080u, &import_37, "ThreadManForUser::0xE1619D7C");
    runtime.register_function(0x0890E088u, &import_38, "ThreadManForUser::0x1FB15A32");
    runtime.register_function(0x0890E090u, &import_39, "ThreadManForUser::0xE81CAF8F");
    runtime.register_function(0x0890E098u, &import_40, "ThreadManForUser::0xEA748E31");
    runtime.register_function(0x0890E0A0u, &import_41, "ThreadManForUser::0xEF9E4C70");
    runtime.register_function(0x0890E0A8u, &import_42, "ThreadManForUser::0xF475845D");
    runtime.register_function(0x0890E0B0u, &import_43, "ThreadManForUser::0x20FFF560");
    runtime.register_function(0x0890E0B8u, &import_44, "ThreadManForUser::0x27E22EC2");
    runtime.register_function(0x0890E0C0u, &import_45, "ThreadManForUser::0x28B6489C");
    runtime.register_function(0x0890E0C8u, &import_46, "ThreadManForUser::0x293B45B8");
    runtime.register_function(0x0890E0D0u, &import_47, "ThreadManForUser::0x30FD48F0");
    runtime.register_function(0x0890E0D8u, &import_48, "ThreadManForUser::0x369ED59D");
    runtime.register_function(0x0890E0E0u, &import_49, "ThreadManForUser::0x383F7BCC");
    runtime.register_function(0x0890E0E8u, &import_50, "ThreadManForUser::0x3AD58B8C");
    runtime.register_function(0x0890E0F0u, &import_51, "ThreadManForUser::0x3B183E26");
    runtime.register_function(0x0890E0F8u, &import_52, "ThreadManForUser::0x3F53E640");
    runtime.register_function(0x0890E100u, &import_53, "ThreadManForUser::0x402FCF22");
    runtime.register_function(0x0890E108u, &import_54, "ThreadManForUser::0x446D8DE6");
    runtime.register_function(0x0890E110u, &import_55, "ThreadManForUser::0x4E3A1105");
    runtime.register_function(0x0890E118u, &import_56, "ThreadManForUser::0x53B00E9A");
    runtime.register_function(0x0890E120u, &import_57, "ThreadManForUser::0x55C20A00");
    runtime.register_function(0x0890E128u, &import_58, "ThreadManForUser::0x58B1F937");
    runtime.register_function(0x0890E130u, &import_59, "ThreadManForUser::0x616403BA");
    runtime.register_function(0x0890E138u, &import_60, "ThreadManForUser::0x68DA9E36");
    runtime.register_function(0x0890E140u, &import_61, "ThreadManForUser::0x71BC9871");
    runtime.register_function(0x0890E148u, &import_62, "ThreadManForUser::0x809CE29B");
    runtime.register_function(0x0890E150u, &import_63, "ThreadManForUser::0x812346E4");
    runtime.register_function(0x0890E158u, &import_64, "ThreadManForUser::0x82826F70");
    runtime.register_function(0x0890E160u, &import_65, "ThreadManForUser::0x82BC5777");
    runtime.register_function(0x0890E168u, &import_66, "ThreadManForUser::0x94AA61EE");
    runtime.register_function(0x0890E170u, &import_67, "ThreadManForUser::0x9ACE131E");
    runtime.register_function(0x0890E178u, &import_68, "ThreadManForUser::0x9FA03CD3");
    runtime.register_function(0x0890E180u, &import_69, "ThreadManForUser::0xAA73C935");
    runtime.register_function(0x0890E188u, &import_70, "UtilsForUser::0x71EC4271");
    runtime.register_function(0x0890E190u, &import_71, "UtilsForUser::0x79D1C3FA");
    runtime.register_function(0x0890E198u, &import_72, "UtilsForUser::0x91E4F6A7");
    runtime.register_function(0x0890E1A0u, &import_73, "UtilsForUser::0x920F104A");
    runtime.register_function(0x0890E1A8u, &import_74, "UtilsForUser::0xB435DEC5");
    runtime.register_function(0x0890E1B0u, &import_75, "UtilsForUser::0xBFA98062");
    runtime.register_function(0x0890E1B8u, &import_76, "UtilsForUser::0x27CC57F0");
    runtime.register_function(0x0890E1C0u, &import_77, "UtilsForUser::0x3EE30821");
    runtime.register_function(0x0890E1C8u, &import_78, "sceSuspendForUser::0x090CCB3F");
    runtime.register_function(0x0890E1D0u, &import_79, "sceSuspendForUser::0x3E0271D3");
    runtime.register_function(0x0890E1D8u, &import_80, "sceSuspendForUser::0xA569E425");
    runtime.register_function(0x0890E1E0u, &import_81, "sceGe_user::0x03444EB4");
    runtime.register_function(0x0890E1E8u, &import_82, "sceGe_user::0x05DB22CE");
    runtime.register_function(0x0890E1F0u, &import_83, "sceGe_user::0x1F6752AD");
    runtime.register_function(0x0890E1F8u, &import_84, "sceGe_user::0xA4FC06A4");
    runtime.register_function(0x0890E200u, &import_85, "sceGe_user::0xAB49E76A");
    runtime.register_function(0x0890E208u, &import_86, "sceGe_user::0xB287BD61");
    runtime.register_function(0x0890E210u, &import_87, "sceGe_user::0xB77905EA");
    runtime.register_function(0x0890E218u, &import_88, "sceGe_user::0xE0D68148");
    runtime.register_function(0x0890E220u, &import_89, "sceGe_user::0xE47E40E4");
    runtime.register_function(0x0890E228u, &import_90, "sceDisplay::0x0E20F177");
    runtime.register_function(0x0890E230u, &import_91, "sceDisplay::0x289D82FE");
    runtime.register_function(0x0890E238u, &import_92, "sceCtrl::0x1F4011E6");
    runtime.register_function(0x0890E240u, &import_93, "sceCtrl::0x1F803938");
    runtime.register_function(0x0890E248u, &import_94, "sceCtrl::0x6A2774F3");
    runtime.register_function(0x0890E250u, &import_95, "sceLibFont::0x099EF33C");
    runtime.register_function(0x0890E258u, &import_96, "sceLibFont::0x0DA7535E");
    runtime.register_function(0x0890E260u, &import_97, "sceLibFont::0x27F6E642");
    runtime.register_function(0x0890E268u, &import_98, "sceLibFont::0x3AEA8CB6");
    runtime.register_function(0x0890E270u, &import_99, "sceLibFont::0x574B6FBC");
    runtime.register_function(0x0890E278u, &import_100, "sceLibFont::0x67F17ED7");
    runtime.register_function(0x0890E280u, &import_101, "sceLibFont::0x980F4895");
    runtime.register_function(0x0890E288u, &import_102, "sceLibFont::0xA834319D");
    runtime.register_function(0x0890E290u, &import_103, "sceLibFont::0xDCC80C2F");
    runtime.register_function(0x0890E298u, &import_104, "sceSasCore::0x267A6DD2");
    runtime.register_function(0x0890E2A0u, &import_105, "sceSasCore::0x33D4AB37");
    runtime.register_function(0x0890E2A8u, &import_106, "sceSasCore::0x42778A9F");
    runtime.register_function(0x0890E2B0u, &import_107, "sceSasCore::0x440CA7D8");
    runtime.register_function(0x0890E2B8u, &import_108, "sceSasCore::0x68A46B95");
    runtime.register_function(0x0890E2C0u, &import_109, "sceSasCore::0x76F01ACA");
    runtime.register_function(0x0890E2C8u, &import_110, "sceSasCore::0x99944089");
    runtime.register_function(0x0890E2D0u, &import_111, "sceSasCore::0xA0CF2FA4");
    runtime.register_function(0x0890E2D8u, &import_112, "sceSasCore::0xA3589D81");
    runtime.register_function(0x0890E2E0u, &import_113, "sceSasCore::0xAD84D37F");
    runtime.register_function(0x0890E2E8u, &import_114, "sceSasCore::0xCBCD4F79");
    runtime.register_function(0x0890E2F0u, &import_115, "sceSasCore::0xD5A229C9");
    runtime.register_function(0x0890E2F8u, &import_116, "sceSasCore::0xE175EF66");
    runtime.register_function(0x0890E300u, &import_117, "sceSasCore::0xF983B186");
    runtime.register_function(0x0890E308u, &import_118, "sceAudio::0x13F592BC");
    runtime.register_function(0x0890E310u, &import_119, "sceAudio::0x5EC81C55");
    runtime.register_function(0x0890E318u, &import_120, "sceAudio::0x6FC46853");
    runtime.register_function(0x0890E320u, &import_121, "sceAudio::0x95FD0C2D");
    runtime.register_function(0x0890E328u, &import_122, "sceAudio::0xB011922F");
    runtime.register_function(0x0890E330u, &import_123, "sceAudio::0xB7E1D8E7");
    runtime.register_function(0x0890E338u, &import_124, "sceAudio::0xCB2E439E");
    runtime.register_function(0x0890E340u, &import_125, "sceUmdUser::0x20628E6F");
    runtime.register_function(0x0890E348u, &import_126, "sceUmdUser::0x6B4A146C");
    runtime.register_function(0x0890E350u, &import_127, "sceUmdUser::0xC6183D47");
    runtime.register_function(0x0890E358u, &import_128, "sceMpeg::0x0E3C2E9D");
    runtime.register_function(0x0890E360u, &import_129, "sceMpeg::0x13407F13");
    runtime.register_function(0x0890E368u, &import_130, "sceMpeg::0x167AFD9E");
    runtime.register_function(0x0890E370u, &import_131, "sceMpeg::0x21FF80E4");
    runtime.register_function(0x0890E378u, &import_132, "sceMpeg::0x37295ED8");
    runtime.register_function(0x0890E380u, &import_133, "sceMpeg::0x42560F23");
    runtime.register_function(0x0890E388u, &import_134, "sceMpeg::0x591A4AA2");
    runtime.register_function(0x0890E390u, &import_135, "sceMpeg::0x606A4649");
    runtime.register_function(0x0890E398u, &import_136, "sceMpeg::0x611E9E11");
    runtime.register_function(0x0890E3A0u, &import_137, "sceMpeg::0x682A619B");
    runtime.register_function(0x0890E3A8u, &import_138, "sceMpeg::0x707B7629");
    runtime.register_function(0x0890E3B0u, &import_139, "sceMpeg::0x740FCCD1");
    runtime.register_function(0x0890E3B8u, &import_140, "sceMpeg::0x800C44DF");
    runtime.register_function(0x0890E3C0u, &import_141, "sceMpeg::0x874624D6");
    runtime.register_function(0x0890E3C8u, &import_142, "sceMpeg::0xA780CF7E");
    runtime.register_function(0x0890E3D0u, &import_143, "sceMpeg::0xB240A59E");
    runtime.register_function(0x0890E3D8u, &import_144, "sceMpeg::0xB5F6DC87");
    runtime.register_function(0x0890E3E0u, &import_145, "sceMpeg::0xC132E22F");
    runtime.register_function(0x0890E3E8u, &import_146, "sceMpeg::0xCEB870B1");
    runtime.register_function(0x0890E3F0u, &import_147, "sceMpeg::0xD7A29F46");
    runtime.register_function(0x0890E3F8u, &import_148, "sceMpeg::0xD8C5F121");
    runtime.register_function(0x0890E400u, &import_149, "sceMpeg::0xE1CE83A7");
    runtime.register_function(0x0890E408u, &import_150, "sceMpeg::0xF8DCB679");
    runtime.register_function(0x0890E410u, &import_151, "sceMpeg::0xFE246728");
    runtime.register_function(0x0890E418u, &import_152, "sceRtc::0xE7C27D1B");
    runtime.register_function(0x0890E420u, &import_153, "sceAtrac3plus::0x5D268707");
    runtime.register_function(0x0890E428u, &import_154, "sceAtrac3plus::0x61EB33F5");
    runtime.register_function(0x0890E430u, &import_155, "sceAtrac3plus::0x6A8C3CD5");
    runtime.register_function(0x0890E438u, &import_156, "sceAtrac3plus::0x7A20E7AF");
    runtime.register_function(0x0890E440u, &import_157, "sceAtrac3plus::0x868120B5");
    runtime.register_function(0x0890E448u, &import_158, "sceAtrac3plus::0x9AE849A7");
    runtime.register_function(0x0890E450u, &import_159, "sceAtrac3plus::0xA2BBA8BE");
    runtime.register_function(0x0890E458u, &import_160, "sceAtrac3plus::0xA554A158");
    runtime.register_function(0x0890E460u, &import_161, "sceAtrac3plus::0xE23E3A35");
    runtime.register_function(0x0890E468u, &import_162, "sceAtrac3plus::0xFAA4F89B");
    runtime.register_function(0x0890E470u, &import_163, "sceUtility::0x2A2B3DE0");
    runtime.register_function(0x0890E478u, &import_164, "sceUtility::0x2AD8E239");
    runtime.register_function(0x0890E480u, &import_165, "sceUtility::0x3DFAEBA9");
    runtime.register_function(0x0890E488u, &import_166, "sceUtility::0x4B85C861");
    runtime.register_function(0x0890E490u, &import_167, "sceUtility::0x4DB1E739");
    runtime.register_function(0x0890E498u, &import_168, "sceUtility::0x50C4CD57");
    runtime.register_function(0x0890E4A0u, &import_169, "sceUtility::0x6332AA39");
    runtime.register_function(0x0890E4A8u, &import_170, "sceUtility::0x67AF3428");
    runtime.register_function(0x0890E4B0u, &import_171, "sceUtility::0x8874DBE0");
    runtime.register_function(0x0890E4B8u, &import_172, "sceUtility::0x91E70E35");
    runtime.register_function(0x0890E4C0u, &import_173, "sceUtility::0x95FC253B");
    runtime.register_function(0x0890E4C8u, &import_174, "sceUtility::0x9790B33C");
    runtime.register_function(0x0890E4D0u, &import_175, "sceUtility::0x9A1C91D7");
    runtime.register_function(0x0890E4D8u, &import_176, "sceUtility::0xA5DA2406");
    runtime.register_function(0x0890E4E0u, &import_177, "sceUtility::0xD4B95FFB");
    runtime.register_function(0x0890E4E8u, &import_178, "sceUtility::0xE49BFE92");
    runtime.register_function(0x0890E4F0u, &import_179, "sceUtility::0xF3F76017");
    runtime.register_function(0x0890E4F8u, &import_180, "sceUtility::0xF6269B82");
    runtime.register_function(0x0890E500u, &import_181, "sceUtility::0xF88155F6");
    runtime.register_function(0x0890E508u, &import_182, "sceNetAdhoc::0x157E6225");
    runtime.register_function(0x0890E510u, &import_183, "sceNetAdhoc::0x4DA4C788");
    runtime.register_function(0x0890E518u, &import_184, "sceNetAdhoc::0x6F92741B");
    runtime.register_function(0x0890E520u, &import_185, "sceNetAdhoc::0x7F27BB5E");
    runtime.register_function(0x0890E528u, &import_186, "sceNetAdhoc::0x877F6D66");
    runtime.register_function(0x0890E530u, &import_187, "sceNetAdhoc::0x8BEA2B3E");
    runtime.register_function(0x0890E538u, &import_188, "sceNetAdhoc::0x9AC2EEAC");
    runtime.register_function(0x0890E540u, &import_189, "sceNetAdhoc::0x9DF81198");
    runtime.register_function(0x0890E548u, &import_190, "sceNetAdhoc::0xA62C6F57");
    runtime.register_function(0x0890E550u, &import_191, "sceNetAdhoc::0xABED3790");
    runtime.register_function(0x0890E558u, &import_192, "sceNetAdhoc::0xB9685118");
    runtime.register_function(0x0890E560u, &import_193, "sceNetAdhoc::0xDFE53E03");
    runtime.register_function(0x0890E568u, &import_194, "sceNetAdhoc::0xE08BDAC1");
    runtime.register_function(0x0890E570u, &import_195, "sceNetAdhoc::0xE1D621D7");
    runtime.register_function(0x0890E578u, &import_196, "sceNetAdhoc::0xFC6FC07B");
    runtime.register_function(0x0890E580u, &import_197, "sceNet::0x0BF0A3AE");
    runtime.register_function(0x0890E588u, &import_198, "sceNet::0x281928A9");
    runtime.register_function(0x0890E590u, &import_199, "sceNet::0x39AF39A6");
    runtime.register_function(0x0890E598u, &import_200, "sceNet::0x50647530");
    runtime.register_function(0x0890E5A0u, &import_201, "sceWlanDrv::0x0C622081");
    runtime.register_function(0x0890E5A8u, &import_202, "sceWlanDrv::0xD7763699");
    runtime.register_function(0x0890E5B0u, &import_203, "sceNetAdhocctl::0x08FFF7A0");
    runtime.register_function(0x0890E5B8u, &import_204, "sceNetAdhocctl::0x20B317A0");
    runtime.register_function(0x0890E5C0u, &import_205, "sceNetAdhocctl::0x34401D65");
    runtime.register_function(0x0890E5C8u, &import_206, "sceNetAdhocctl::0x6402490B");
    runtime.register_function(0x0890E5D0u, &import_207, "sceNetAdhocctl::0x81AEE1BE");
    runtime.register_function(0x0890E5D8u, &import_208, "sceNetAdhocctl::0x9D689E13");
    runtime.register_function(0x0890E5E0u, &import_209, "sceNetAdhocctl::0xE162CB14");
    runtime.register_function(0x0890E5E8u, &import_210, "sceNetAdhocctl::0xE26F226E");
    runtime.register_function(0x0890E5F0u, &import_211, "sceNetApctl::0x24FE91A1");
    runtime.register_function(0x0890E5F8u, &import_212, "sceNetApctl::0x2BEFDF23");
    runtime.register_function(0x0890E600u, &import_213, "sceNetApctl::0x5963991B");
    runtime.register_function(0x0890E608u, &import_214, "sceNetApctl::0x8ABADD51");
    runtime.register_function(0x0890E610u, &import_215, "sceNetApctl::0xB3EDD0EC");
    runtime.register_function(0x0890E618u, &import_216, "sceNetApctl::0xE2F91F9B");
    runtime.register_function(0x0890E620u, &import_217, "sceNetInet::0x17943399");
    runtime.register_function(0x0890E628u, &import_218, "sceNetInet::0x1A33F9AE");
    runtime.register_function(0x0890E630u, &import_219, "sceNetInet::0x2FE71FE7");
    runtime.register_function(0x0890E638u, &import_220, "sceNetInet::0x410B34AA");
    runtime.register_function(0x0890E640u, &import_221, "sceNetInet::0x7AA671BC");
    runtime.register_function(0x0890E648u, &import_222, "sceNetInet::0x805502DD");
    runtime.register_function(0x0890E650u, &import_223, "sceNetInet::0x8B7B220F");
    runtime.register_function(0x0890E658u, &import_224, "sceNetInet::0x8CA3A97E");
    runtime.register_function(0x0890E660u, &import_225, "sceNetInet::0x8D7284EA");
    runtime.register_function(0x0890E668u, &import_226, "sceNetInet::0xA9ED66B9");
    runtime.register_function(0x0890E670u, &import_227, "sceNetInet::0xB3888AD4");
    runtime.register_function(0x0890E678u, &import_228, "sceNetInet::0xB75D5B0A");
    runtime.register_function(0x0890E680u, &import_229, "sceNetInet::0xCDA85C99");
    runtime.register_function(0x0890E688u, &import_230, "sceNetInet::0xD10A1A7A");
    runtime.register_function(0x0890E690u, &import_231, "sceNetInet::0xDB094E1B");
    runtime.register_function(0x0890E698u, &import_232, "sceNetInet::0xE247B6D6");
    runtime.register_function(0x0890E6A0u, &import_233, "scePower::0x04B7766E");
    runtime.register_function(0x0890E6A8u, &import_234, "sceOpenPSID::0xC69BEBCE");
    runtime.register_function(0x0890E6B0u, &import_235, "sceImpose::0x36AA6E91");
    runtime.register_function(0x0890E6B8u, &import_236, "sceDmac::0x617F3FE6");
    runtime.register_function(0x0890E6C0u, &import_237, "sceNetAdhocDiscover::0x52DE1B97");
    runtime.register_function(0x0890E6C8u, &import_238, "sceNetAdhocDiscover::0x941B3877");
    runtime.register_function(0x0890E6D0u, &import_239, "sceNetAdhocDiscover::0x944DDBC6");
    runtime.register_function(0x0890E6D8u, &import_240, "sceNetAdhocDiscover::0xA2246614");
    runtime.register_function(0x0890E6E0u, &import_241, "sceNetAdhocDiscover::0xA423A21B");
    runtime.register_function(0x0890E6E8u, &import_242, "sceHttp::0x0282A3BD");
    runtime.register_function(0x0890E6F0u, &import_243, "sceHttp::0x0B12ABFB");
    runtime.register_function(0x0890E6F8u, &import_244, "sceHttp::0x1F0FC3E3");
    runtime.register_function(0x0890E700u, &import_245, "sceHttp::0x2255551E");
    runtime.register_function(0x0890E708u, &import_246, "sceHttp::0x3EABA285");
    runtime.register_function(0x0890E710u, &import_247, "sceHttp::0x4CC7D78F");
    runtime.register_function(0x0890E718u, &import_248, "sceHttp::0x5152773B");
    runtime.register_function(0x0890E720u, &import_249, "sceHttp::0x87797BDD");
    runtime.register_function(0x0890E728u, &import_250, "sceHttp::0x8ACD1F73");
    runtime.register_function(0x0890E730u, &import_251, "sceHttp::0x9988172D");
    runtime.register_function(0x0890E738u, &import_252, "sceHttp::0x9B1F1F36");
    runtime.register_function(0x0890E740u, &import_253, "sceHttp::0xA4496DE5");
    runtime.register_function(0x0890E748u, &import_254, "sceHttp::0xA5512E01");
    runtime.register_function(0x0890E750u, &import_255, "sceHttp::0xAB1540D5");
    runtime.register_function(0x0890E758u, &import_256, "sceHttp::0xAB1ABE07");
    runtime.register_function(0x0890E760u, &import_257, "sceHttp::0xAE948FEE");
    runtime.register_function(0x0890E768u, &import_258, "sceHttp::0xB509B09E");
    runtime.register_function(0x0890E770u, &import_259, "sceHttp::0xBB70706F");
    runtime.register_function(0x0890E778u, &import_260, "sceHttp::0xC10B6BD9");
    runtime.register_function(0x0890E780u, &import_261, "sceHttp::0xCCBD167A");
    runtime.register_function(0x0890E788u, &import_262, "sceHttp::0xCDF8ECB9");
    runtime.register_function(0x0890E790u, &import_263, "sceHttp::0xD1C8945E");
    runtime.register_function(0x0890E798u, &import_264, "sceHttp::0xDB266CCF");
    runtime.register_function(0x0890E7A0u, &import_265, "sceHttp::0xE4D21302");
    runtime.register_function(0x0890E7A8u, &import_266, "sceHttp::0xEDEEB999");
    runtime.register_function(0x0890E7B0u, &import_267, "sceHttp::0xF49934F6");
    runtime.register_function(0x0890E7B8u, &import_268, "sceHttp::0xF9D8EB63");
    runtime.register_function(0x0890E7C0u, &import_269, "sceHttp::0xFCF8C055");
    runtime.register_function(0x0890E7C8u, &import_270, "sceSsl::0x191CDEFF");
    runtime.register_function(0x0890E7D0u, &import_271, "sceSsl::0x957ECBE2");
}
} // namespace psprecomp
