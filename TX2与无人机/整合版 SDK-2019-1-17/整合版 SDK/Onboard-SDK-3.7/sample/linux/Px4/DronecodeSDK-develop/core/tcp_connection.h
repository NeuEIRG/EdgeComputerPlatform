#pragma once

#include <mutex>
#include <atomic>
#include "connection.h"
#include <sys/types.h>
#ifndef WINDOWS
#include <netdb.h>
#include <netinet/in.h>
#else
#include <winsock2.h>
#include <Ws2tcpip.h> // For InetPton
#undef SOCKET_ERROR
#endif

namespace dronecode_sdk {

class TcpConnection : public Connection {
public:
    explicit TcpConnection(Connection::receiver_callback_t receiver_callback,
                           const std::string &remote_ip,
                           int remote_port);
    ~TcpConnection();
    ConnectionResult start();
    ConnectionResult stop();

    bool send_message(const mavlink_message_t &message);

    // Non-copyable
    TcpConnection(const TcpConnection &) = delete;
    const TcpConnection &operator=(const TcpConnection &) = delete;

private:
    ConnectionResult setup_port();
    void start_recv_thread();
    int resolve_address(const std::string &ip_address, int port, struct sockaddr_in *addr);
    void receive();

    std::string _remote_ip = {};
    int _remote_port_number;

    std::mutex _mutex = {};
    int _socket_fd = -1;

    std::thread *_recv_thread = nullptr;
    std::atomic_bool _should_exit;
    std::atomic_bool _is_ok{false};
};

} // namespace dronecode_sdk
