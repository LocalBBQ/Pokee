#pragma once

// `Pokee --join [address]`: one run that goes on line with a session already
// hosted, without changing settings.ini. main() turns the result into the
// MHFU_ADHOC, MHFU_ADHOC_SERVER and MHFU_ADHOC_MAC overrides, which the
// settings read like any variable set in the shell and never save.
#include "adhoc/protocol.hpp"

#include <cstdint>
#include <random>
#include <string>
#include <string_view>

namespace mhfu::adhoc {

struct JoinTarget {
    std::string server;
    // The session is hosted on this machine: most likely by an instance with
    // the same data folder, and so the same address in settings.ini. The run
    // takes an address of its own, or the server sees one player twice.
    bool this_machine{};
};

// The host part of "host", "host:port", "[v6]" or "[v6]:port".
[[nodiscard]] inline std::string_view join_host(std::string_view address) {
    if (!address.empty() && address.front() == '[') {
        const std::size_t close = address.find(']');
        return close == std::string_view::npos ? address.substr(1) : address.substr(1, close - 1);
    }
    const std::size_t colon = address.find(':');
    // More than one colon is a bare IPv6 address, which has no port.
    if (colon != std::string_view::npos && address.find(':', colon + 1) == std::string_view::npos)
        return address.substr(0, colon);
    return address;
}

[[nodiscard]] inline bool is_this_machine(std::string_view host) {
    if (host == "localhost" || host == "::1") return true;
    return host.size() > 4 && host.substr(0, 4) == "127.";
}

// An empty address is the session this machine hosts, on the default port.
[[nodiscard]] inline JoinTarget join_target(std::string_view address) {
    JoinTarget target;
    target.server = address.empty() ? std::string("127.0.0.1") : std::string(address);
    target.this_machine = is_this_machine(join_host(target.server));
    return target;
}

// A made-up address as settings.ini writes one: locally administered and
// unicast, like the one hle_adhoc makes on first use.
[[nodiscard]] inline std::string made_up_mac() {
    std::random_device random;
    Mac mac{};
    for (std::uint8_t &byte : mac) byte = static_cast<std::uint8_t>(random());
    mac[0] = static_cast<std::uint8_t>((mac[0] & 0xFCu) | 0x02u);
    if (mac[1] == 0u) mac[1] = 1u;
    return format_mac(mac);
}

} // namespace mhfu::adhoc
