#pragma once

#include <string>
#include <memory>
#include <vector>
#include <functional>

#include "system.h"
#include "connection_result.h"

namespace dronecode_sdk {

class DronecodeSDKImpl;
class System;

/**
 * @brief This is the main class of Dronecode SDK (a MAVLink API Library for the Dronecode
 Platform).

 * It is used to discover vehicles and manage active connections.
 *
 * An instance of this class must be created (first) in order to use the library.
 * The instance must be destroyed after use in order to break connections and release all resources.
 */
class DronecodeSDK {
public:
    /** @brief Default UDP bind IP (accepts any incoming connections). */
    static constexpr auto DEFAULT_UDP_BIND_IP = "0.0.0.0";
    /** @brief Default UDP bind port (same port as used by MAVROS). */
    static constexpr int DEFAULT_UDP_PORT = 14540;
    /** @brief Default TCP remote IP (localhost). */
    static constexpr auto DEFAULT_TCP_REMOTE_IP = "127.0.0.1";
    /** @brief Default TCP remote port. */
    static constexpr int DEFAULT_TCP_REMOTE_PORT = 5760;
    /** @brief Default serial baudrate. */
    static constexpr int DEFAULT_SERIAL_BAUDRATE = 57600;

    /**
     * @brief Constructor.
     */
    DronecodeSDK();

    /**
     * @brief Destructor.
     *
     * Disconnects all connected vehicles and releases all resources.
     */
    ~DronecodeSDK();

    /**
     * @brief Adds Connection via URL
     *
     * Supports connection: Serial, TCP or UDP.
     * Connection URL format should be:
     * - UDP - udp://[Bind_host][:Bind_port]
     * - TCP - tcp://[Remote_host][:Remote_port]
     * - Serial - serial://Dev_Node[:Baudrate]
     *
     * @param connection_url connection URL string.
     * @return The result of adding the connection.
     */
    ConnectionResult add_any_connection(const std::string &connection_url);

    /**
     * @brief Adds a UDP connection to the specified port number.
     *
     * Any incoming connections are accepted (0.0.0.0).
     *
     * @param local_port The local UDP port to listen to (defaults to 14540, the same as MAVROS).
     * @return The result of adding the connection.
     */
    ConnectionResult add_udp_connection(int local_port = DEFAULT_UDP_PORT);

    /**
     * @brief Adds a UDP connection to the specified port number and local interface.
     *
     * To accept only local connections of the machine, use 127.0.0.1.
     * For any incoming connections, use 0.0.0.0.
     *
     * @param local_ip The local UDP IP address to listen to.
     * @param local_port The local UDP port to listen to (defaults to 14540, the same as MAVROS).
     * @return The result of adding the connection.
     */
    ConnectionResult add_udp_connection(const std::string &local_ip,
                                        int local_port = DEFAULT_UDP_PORT);

    /**
     * @brief Adds a TCP connection with a specific port number on localhost.
     *
     * @param remote_port The TCP port to connect to (defaults to 5760).
     * @return The result of adding the connection.
     */
    ConnectionResult add_tcp_connection(int remote_port = DEFAULT_TCP_REMOTE_PORT);

    /**
     * @brief Adds a TCP connection with a specific IP address and port number.
     *
     * @param remote_ip Remote IP address to connect to.
     * @param remote_port The TCP port to connect to (defaults to 5760).
     * @return The result of adding the connection.
     */
    ConnectionResult add_tcp_connection(const std::string &remote_ip,
                                        int remote_port = DEFAULT_TCP_REMOTE_PORT);

    /**
     * @brief Adds a serial connection with a specific port (COM or UART dev node) and baudrate as
     * specified.
     *
     *
     * @param dev_path COM or UART dev node name/path (e.g. "/dev/ttyS0", or "COM3" on Windows).
     * @param baudrate Baudrate of the serial port (defaults to 57600).
     * @return The result of adding the connection.
     */
    ConnectionResult add_serial_connection(const std::string &dev_path,
                                           int baudrate = DEFAULT_SERIAL_BAUDRATE);

    /**
     * @brief Get vector of system UUIDs.
     *
     * This returns a vector of the UUIDs of all systems that have been discovered.
     * If a system doesn't have a UUID then DronecodeSDK will instead use its MAVLink system ID
     * (range: 0..255).
     *
     * @return A vector containing the UUIDs.
     */
    std::vector<uint64_t> system_uuids() const;

    /**
     * @brief Get the first discovered system.
     *
     * This returns the first discovered system or a null system if no system has yet been found.
     *
     * @return A reference to a system.
     */
    System &system() const;

    /**
     * @brief Get the system with the specified UUID.
     *
     * This returns a system for a given UUID if such a system has been discovered and a null
     * system otherwise.
     *
     * @param uuid UUID of system to get.
     * @return A reference to the specified system.
     */
    System &system(uint64_t uuid) const;

    /**
     * @brief Callback type for discover and timeout notifications.
     *
     * @param uuid UUID of system (or MAVLink system ID for systems that don't have a UUID).
     */
    typedef std::function<void(uint64_t uuid)> event_callback_t;

    /**
     * @brief Returns `true` if exactly one system is currently connected.
     *
     * Connected means we are receiving heartbeats from this system.
     * It means the same as "discovered" and "not timed out".
     *
     * If multiple systems have connected, this will return `false`.
     *
     * @return `true` if exactly one system is connected.
     */
    bool is_connected() const;

    /**
     * @brief Returns `true` if a system is currently connected.
     *
     * Connected means we are receiving heartbeats from this system.
     * It means the same as "discovered" and "not timed out".
     *
     * @param uuid UUID of system to check.
     * @return `true` if system is connected.
     */
    bool is_connected(uint64_t uuid) const;

    /**
     * @brief Register callback for system discovery.
     *
     * This sets a callback that will be notified if a new system is discovered.
     *
     * If systems have been discovered before this callback is registered, they will be notified
     * at the time this callback is registered.
     *
     * @note Only one callback can be registered at a time. If this function is called several
     * times, previous callbacks will be overwritten.
     *
     * @param callback Callback to register.
     *
     */
    void register_on_discover(event_callback_t callback);

    /**
     * @brief Register callback for system timeout.
     *
     * This sets a callback that will be notified if no heartbeat of the system has been received
     * in 3 seconds.
     *
     * @note Only one callback can be registered at a time. If this function is called several
     * times, previous callbacks will be overwritten.
     *
     * @param callback Callback to register.
     */
    void register_on_timeout(event_callback_t callback);

private:
    /* @private. */
    std::unique_ptr<DronecodeSDKImpl> _impl;

    // Non-copyable
    DronecodeSDK(const DronecodeSDK &) = delete;
    const DronecodeSDK &operator=(const DronecodeSDK &) = delete;
};

} // namespace dronecode_sdk
