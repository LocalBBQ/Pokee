#pragma once

// The client side of a PSP ad hoc server: matchmaking over adhocctl and every
// ad hoc socket relayed through the server (see protocol.hpp).
//
// Threading. One network thread, started on first use, owns every host
// socket: it resolves the server, connects, reconnects with backoff, reads
// and writes. The emulation thread only calls the methods below, which work
// on in-memory state under one mutex and never wait for the network. A slow
// or dead server therefore costs the game nothing; it shows up as sockets that
// never get data and, after a grace period, as a normal ad hoc disconnect.
#include "adhoc/protocol.hpp"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace mhp3rd::adhoc {

struct Identity {
    std::string server;    // host name or address, optionally host:port for adhocctl
    std::string nickname;
    Mac mac{};
    std::string product;   // the game's product code, for example ULJM05800
};

// What the game sees of the matchmaking service.
enum class ServerState {
    Off,         // not started, or no server configured
    Connecting,  // resolving, connecting, or waiting to retry
    Online,      // logged in
};

struct Peer {
    Mac mac{};
    std::string nickname;
    std::uint64_t joined_ms{};  // host steady clock
};

struct GroupInfo {
    std::string name;
    Mac host{};
};

// Things the adhocctl handler reports to the game.
enum class CtlEvent {
    Connected,     // the group join finished
    Disconnected,  // left the group, or lost it for good
    ScanComplete,
    Error,         // a join or scan could not be done: the server is unreachable
};

struct Datagram {
    Mac source{};
    std::uint16_t port{};
    std::string data;
};

enum class StreamState {
    Closed,
    Listening,
    Opening,      // connecting, waiting for the other side to accept
    Established,
    Failed,       // refused or timed out while opening
    Disconnected, // established once, now gone
};

struct StreamInfo {
    StreamState state{StreamState::Closed};
    std::uint16_t local_port{};
    Mac peer{};
    std::uint16_t peer_port{};
    std::size_t readable{};    // bytes waiting to be received
    std::size_t unsent{};      // bytes queued but not yet handed to the host socket
    std::size_t capacity{};
    std::uint64_t sent{};
    std::uint64_t received{};
};

class Client {
public:
    static Client &get();
    ~Client();
    Client(const Client &) = delete;
    Client &operator=(const Client &) = delete;

    // Logs in to identity.server and stays logged in, reconnecting as needed,
    // until stop(). Calling it again with a different identity starts over.
    void start(const Identity &identity);
    // Leaves the group, closes every socket and logs out.
    void stop();

    // Matchmaking -------------------------------------------------------------
    [[nodiscard]] ServerState server_state() const;
    // Joins `group` (created if nobody is in it). Reported with
    // CtlEvent::Connected, or CtlEvent::Error if the server stays unreachable.
    void join(const std::string &group);
    void leave();
    void scan();
    [[nodiscard]] bool in_group() const;
    [[nodiscard]] std::optional<GroupInfo> group() const;
    [[nodiscard]] std::vector<Peer> peers() const;
    [[nodiscard]] std::vector<GroupInfo> scan_results() const;
    [[nodiscard]] std::vector<CtlEvent> take_events();

    // Datagram sockets (PDP) ----------------------------------------------------
    // Returns a handle, or 0 when `port` is taken.
    [[nodiscard]] int pdp_open(std::uint16_t port, std::size_t capacity);
    void pdp_close(int handle);
    // Queues one datagram; to every group member for kBroadcastMac. Dropped
    // like any datagram when not in a group. False for an unknown handle.
    bool pdp_send(int handle, const Mac &destination, std::uint16_t port, const void *data, std::size_t size);
    // The size of the next waiting datagram, if any.
    [[nodiscard]] std::optional<std::size_t> pdp_peek(int handle) const;
    [[nodiscard]] std::optional<Datagram> pdp_receive(int handle);
    [[nodiscard]] std::size_t pdp_waiting_bytes(int handle) const;

    // Stream sockets (PTP) ----------------------------------------------------
    // Returns a handle, or 0 when the port is taken.
    [[nodiscard]] int ptp_listen(std::uint16_t port, std::size_t capacity, std::size_t backlog);
    // Starts connecting at once and keeps retrying, `retry_us` apart, up to
    // `retries` more times while refused. Returns a handle, or 0.
    [[nodiscard]] int ptp_open(std::uint16_t local_port, const Mac &peer, std::uint16_t peer_port,
                               std::size_t capacity, std::uint64_t retry_us, std::uint32_t retries);
    // Takes the oldest pending connection of a listening socket.
    [[nodiscard]] int ptp_accept(int listener);
    [[nodiscard]] StreamInfo ptp_info(int handle) const;
    [[nodiscard]] bool ptp_exists(int handle) const;
    // Queues up to `size` bytes, as many as the send buffer has room for.
    [[nodiscard]] std::size_t ptp_send(int handle, const void *data, std::size_t size);
    [[nodiscard]] std::size_t ptp_receive(int handle, void *data, std::size_t size);
    void ptp_close(int handle);
    [[nodiscard]] std::vector<int> ptp_handles() const;

    [[nodiscard]] static bool tracing();

private:
    Client();
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

// Picks a free port for a socket opened on port 0.
inline constexpr std::uint16_t kFirstEphemeralPort = 0x8000u;

} // namespace mhp3rd::adhoc
