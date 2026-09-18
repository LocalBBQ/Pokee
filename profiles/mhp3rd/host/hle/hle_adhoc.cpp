// sceNet, sceNetAdhoc, sceNetAdhocctl, sceNetAdhocDiscover and the network
// configuration dialog (sceUtilityNetconf): PSP ad hoc play.
//
// This first version only records what the game calls: with
// MHP3RD_TRACE_ADHOC=1 every call is logged with its arguments, its caller and
// its result. The calls still answer the way the logging stubs did.
#include "hle_common.hpp"

#include "psprecomp/common.hpp"

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>

namespace mhp3rd {
namespace {

bool trace_adhoc() {
    static const bool enabled = [] {
        const char *text = std::getenv("MHP3RD_TRACE_ADHOC");
        return text != nullptr && *text != '\0' && std::string(text) != "0";
    }();
    return enabled;
}

void trace_call(const char *name, const AllegrexContext &ctx, unsigned argument_count, std::uint32_t result) {
    if (!trace_adhoc()) return;
    std::ostringstream line;
    const Thread *thread = kernel().current_thread();
    line << "[adhoc] " << name << "(";
    for (unsigned i = 0; i < argument_count; ++i) line << (i != 0u ? ", " : "") << psprecomp::hex32(arg(ctx, i));
    line << ") = " << psprecomp::hex32(result) << " ra=" << psprecomp::hex32(ctx.gpr[31])
         << " thread=" << (thread != nullptr ? thread->name : "interrupt") << " t=" << kernel().now_us() / 1000u
         << "ms";
    std::cerr << line.str() << std::endl;
}

struct TracedImport {
    const char *library;
    const char *name;
    unsigned arguments;
};

constexpr TracedImport kImports[] = {
    {"sceNet", "sceNetInit", 5},
    {"sceNet", "sceNetTerm", 0},
    {"sceNet", "sceNetGetLocalEtherAddr", 1},
    {"sceNet", "sceNetFreeThreadinfo", 1},
    {"sceNetAdhoc", "sceNetAdhocInit", 0},
    {"sceNetAdhoc", "sceNetAdhocTerm", 0},
    {"sceNetAdhoc", "sceNetAdhocPdpCreate", 4},
    {"sceNetAdhoc", "sceNetAdhocPdpDelete", 2},
    {"sceNetAdhoc", "sceNetAdhocPdpSend", 7},
    {"sceNetAdhoc", "sceNetAdhocPdpRecv", 7},
    {"sceNetAdhoc", "sceNetAdhocPtpOpen", 8},
    {"sceNetAdhoc", "sceNetAdhocPtpConnect", 3},
    {"sceNetAdhoc", "sceNetAdhocPtpListen", 7},
    {"sceNetAdhoc", "sceNetAdhocPtpAccept", 5},
    {"sceNetAdhoc", "sceNetAdhocPtpSend", 5},
    {"sceNetAdhoc", "sceNetAdhocPtpRecv", 5},
    {"sceNetAdhoc", "sceNetAdhocPtpFlush", 3},
    {"sceNetAdhoc", "sceNetAdhocPtpClose", 2},
    {"sceNetAdhoc", "sceNetAdhocGetPtpStat", 2},
    {"sceNetAdhocctl", "sceNetAdhocctlInit", 3},
    {"sceNetAdhocctl", "sceNetAdhocctlTerm", 0},
    {"sceNetAdhocctl", "sceNetAdhocctlScan", 0},
    {"sceNetAdhocctl", "sceNetAdhocctlGetScanInfo", 2},
    {"sceNetAdhocctl", "sceNetAdhocctlDisconnect", 0},
    {"sceNetAdhocctl", "sceNetAdhocctlGetPeerList", 2},
    {"sceNetAdhocctl", "sceNetAdhocctlAddHandler", 2},
    {"sceNetAdhocctl", "sceNetAdhocctlDelHandler", 1},
    {"sceNetAdhocDiscover", "sceNetAdhocDiscoverInitStart", 1},
    {"sceNetAdhocDiscover", "sceNetAdhocDiscoverUpdate", 0},
    {"sceNetAdhocDiscover", "sceNetAdhocDiscoverGetStatus", 0},
    {"sceNetAdhocDiscover", "sceNetAdhocDiscoverStop", 0},
    {"sceNetAdhocDiscover", "sceNetAdhocDiscoverTerm", 0},
    {"sceUtility", "sceUtilityNetconfInitStart", 1},
    {"sceUtility", "sceUtilityNetconfUpdate", 1},
    {"sceUtility", "sceUtilityNetconfGetStatus", 0},
    {"sceUtility", "sceUtilityNetconfShutdownStart", 0},
};

} // namespace

void register_adhoc(HleRegistrar &hle) {
    for (const TracedImport &import : kImports) {
        hle.add(import.library, import.name, [import](Runtime &, AllegrexContext &ctx) {
            trace_call(import.name, ctx, import.arguments, 0u);
            kernel().finish(ctx, 0u);
        });
    }
}

} // namespace mhp3rd
