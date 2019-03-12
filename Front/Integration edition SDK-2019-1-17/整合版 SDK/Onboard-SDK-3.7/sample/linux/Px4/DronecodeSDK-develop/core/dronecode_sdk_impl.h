#pragma once

#include <map>
#include <mutex>
#include <vector>
#include <atomic>

#include "connection.h"
#include "dronecode_sdk.h"
#include "system.h"
#include "mavlink_include.h"

namespace dronecode_sdk {

class DronecodeSDKImpl {
public:
    DronecodeSDKImpl();
    ~DronecodeSDKImpl();

    void receive_message(const mavlink_message_t &message);
    bool send_message(const mavlink_message_t &message);

    ConnectionResult add_any_connection(const std::string &connection_url);
    ConnectionResult
    add_link_connection(const std::string &protocol, const std::string &ip, int port);
    ConnectionResult add_udp_connection(const std::string &local_ip, int local_port_number);
    ConnectionResult add_tcp_connection(const std::string &remote_ip, int remote_port);
    ConnectionResult add_serial_connection(const std::string &dev_path, int baudrate);

    std::vector<uint64_t> get_system_uuids() const;
    System &get_system();
    System &get_system(uint64_t uuid);

    bool is_connected() const;
    bool is_connected(uint64_t uuid) const;

    void register_on_discover(DronecodeSDK::event_callback_t callback);
    void register_on_timeout(DronecodeSDK::event_callback_t callback);

    void notify_on_discover(uint64_t uuid);
    void notify_on_timeout(uint64_t uuid);

private:
    void add_connection(std::shared_ptr<Connection>);
    void make_system_with_component(uint8_t system_id, uint8_t component_id);
    bool does_system_exist(uint8_t system_id);

    using system_entry_t = std::pair<uint8_t, std::shared_ptr<System>>;

    std::mutex _connections_mutex;
    std::vector<std::shared_ptr<Connection>> _connections;

    mutable std::recursive_mutex _systems_mutex;
    std::map<uint8_t, std::shared_ptr<System>> _systems;

    DronecodeSDK::event_callback_t _on_discover_callback;
    DronecodeSDK::event_callback_t _on_timeout_callback;

    std::atomic<bool> _should_exit = {false};
};

} // namespace dronecode_sdk
