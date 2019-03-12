#pragma once

#include "global_include.h"
#include "mavlink_include.h"
#include "mavlink_parameters.h"
#include "mavlink_commands.h"
#include "timeout_handler.h"
#include "call_every_handler.h"
#include "thread_pool.h"
#include "system.h"
#include <cstdint>
#include <functional>
#include <atomic>
#include <vector>
#include <unordered_set>
#include <map>
#include <thread>
#include <mutex>
#include <future>

// TODO: Figure out what to do with systems without UUID.
//#define ENABLE_FALLBACK_TO_SYSTEM_ID

namespace dronecode_sdk {

class DronecodeSDKImpl;
class PluginImplBase;

// GCS: Ground Control Station
// Type that represents DronecodeSDK client application which is a GCS.
struct GCSClient {
    static constexpr uint8_t system_id = 0;
    // FIXME: This is a workaround for now. We should revert it later or add a compid for the SDK.
    static constexpr uint8_t component_id = MAV_COMP_ID_SYSTEM_CONTROL + 1;
    static constexpr MAV_TYPE type = MAV_TYPE_GCS;
};

// This class is the pimpl of System. This is to hide the private methods
// and functionality from the public library API.
class SystemImpl {
public:
    enum class FlightMode {
        HOLD = 0,
        RETURN_TO_LAUNCH,
        TAKEOFF,
        LAND,
        MISSION,
        FOLLOW_ME,
        OFFBOARD,
    };

    explicit SystemImpl(DronecodeSDKImpl &parent, uint8_t system_id, uint8_t component_id);
    ~SystemImpl();

    void process_mavlink_message(const mavlink_message_t &message);

    typedef std::function<void(const mavlink_message_t &)> mavlink_message_handler_t;

    void register_mavlink_message_handler(uint16_t msg_id,
                                          mavlink_message_handler_t callback,
                                          const void *cookie);

    void unregister_all_mavlink_message_handlers(const void *cookie);

    void register_timeout_handler(std::function<void()> callback, double duration_s, void **cookie);
    void refresh_timeout_handler(const void *cookie);
    void unregister_timeout_handler(const void *cookie);

    void add_call_every(std::function<void()> callback, float interval_s, void **cookie);
    void change_call_every(float interval_s, const void *cookie);
    void reset_call_every(const void *cookie);
    void remove_call_every(const void *cookie);

    bool send_message(const mavlink_message_t &message);

    typedef std::function<void(MAVLinkCommands::Result, float)> command_result_callback_t;

    MAVLinkCommands::Result send_command(MAVLinkCommands::CommandLong &command);
    MAVLinkCommands::Result send_command(MAVLinkCommands::CommandInt &command);

    void send_command_async(MAVLinkCommands::CommandLong &command,
                            const command_result_callback_t callback);
    void send_command_async(MAVLinkCommands::CommandInt &command,
                            const command_result_callback_t callback);

    MAVLinkCommands::Result set_msg_rate(uint16_t message_id,
                                         double rate_hz,
                                         uint8_t component_id = MAV_COMP_ID_AUTOPILOT1);

    void set_msg_rate_async(uint16_t message_id,
                            double rate_hz,
                            command_result_callback_t callback,
                            uint8_t component_id = MAV_COMP_ID_AUTOPILOT1);

    // Adds unique component ids
    void add_new_component(uint8_t component_id);
    size_t total_components() const;

    void register_component_discovered_callback(discover_callback_t callback);

    uint8_t get_autopilot_id() const;
    std::vector<uint8_t> get_camera_ids() const;
    uint8_t get_gimbal_id() const;

    bool is_standalone() const;
    bool has_autopilot() const;
    bool has_camera(int camera_id = -1) const;
    bool has_gimbal() const;

    uint64_t get_uuid() const;
    uint8_t get_system_id() const;

    void set_system_id(uint8_t system_id);

    bool does_support_mission_int() const { return _supports_mission_int; }

    bool is_armed() const { return _armed; }

    MAVLinkParameters::Result set_param_float(const std::string &name, float value);
    MAVLinkParameters::Result set_param_int(const std::string &name, int32_t value);
    MAVLinkParameters::Result set_param_ext_float(const std::string &name, float value);
    MAVLinkParameters::Result set_param_ext_int(const std::string &name, int32_t value);

    typedef std::function<void(MAVLinkParameters::Result result)> success_t;
    void set_param_float_async(const std::string &name, float value, success_t callback);
    void set_param_int_async(const std::string &name, int32_t value, success_t callback);
    void set_param_ext_float_async(const std::string &name, float value, success_t callback);
    void set_param_ext_int_async(const std::string &name, int32_t value, success_t callback);

    MAVLinkCommands::Result set_flight_mode(FlightMode mode,
                                            uint8_t component_id = MAV_COMP_ID_AUTOPILOT1);

    void set_flight_mode_async(FlightMode mode,
                               command_result_callback_t callback,
                               uint8_t component_id = MAV_COMP_ID_AUTOPILOT1);

    typedef std::function<void(MAVLinkParameters::Result result, float value)>
        get_param_float_callback_t;
    typedef std::function<void(MAVLinkParameters::Result result, int32_t value)>
        get_param_int_callback_t;

    std::pair<MAVLinkParameters::Result, float> get_param_float(const std::string &name);
    std::pair<MAVLinkParameters::Result, int> get_param_int(const std::string &name);
    std::pair<MAVLinkParameters::Result, float> get_param_ext_float(const std::string &name);
    std::pair<MAVLinkParameters::Result, int> get_param_ext_int(const std::string &name);

    // These methods can be used to cache a parameter when a system connects. For that
    // the callback can just be set to nullptr.
    void get_param_float_async(const std::string &name, get_param_float_callback_t callback);
    void get_param_int_async(const std::string &name, get_param_int_callback_t callback);
    void get_param_ext_float_async(const std::string &name, get_param_float_callback_t callback);
    void get_param_ext_int_async(const std::string &name, get_param_int_callback_t callback);

    typedef std::function<void(MAVLinkParameters::Result result,
                               MAVLinkParameters::ParamValue value)>
        get_param_callback_t;

    void set_param_async(const std::string &name,
                         MAVLinkParameters::ParamValue value,
                         success_t callback,
                         bool extended = false);

    MAVLinkParameters::Result
    set_param(const std::string &name, MAVLinkParameters::ParamValue value, bool extended = false);

    void get_param_async(const std::string &name,
                         MAVLinkParameters::ParamValue value_type,
                         get_param_callback_t callback,
                         bool extended);

    void param_changed(const std::string &name);

    typedef std::function<void(const std::string &name)> param_changed_callback_t;
    void register_param_changed_handler(const param_changed_callback_t callback,
                                        const void *cookie);
    void unregister_param_changed_handler(const void *cookie);

    bool is_connected() const;

    Time &get_time() { return _time; };

    void register_plugin(PluginImplBase *plugin_impl);
    void unregister_plugin(PluginImplBase *plugin_impl);

    void call_user_callback(const std::function<void()> &func);

    void send_autopilot_version_request();

    // Non-copyable
    SystemImpl(const SystemImpl &) = delete;
    const SystemImpl &operator=(const SystemImpl &) = delete;

private:
    // Helper methods added to increase readablity
    static bool is_autopilot(uint8_t comp_id);
    static bool is_camera(uint8_t comp_id);

    void request_autopilot_version();

    bool have_uuid() const { return _uuid != 0 && _uuid_initialized; }

    void process_heartbeat(const mavlink_message_t &message);
    void process_autopilot_version(const mavlink_message_t &message);
    void process_statustext(const mavlink_message_t &message);
    void heartbeats_timed_out();
    void set_connected();
    void set_disconnected();

    static std::string component_name(uint8_t component_id);
    static ComponentType component_type(uint8_t component_id);

    void system_thread();
    void send_heartbeat();

    // We use std::pair instead of a std::optional.
    std::pair<MAVLinkCommands::Result, MAVLinkCommands::CommandLong>
    make_command_flight_mode(FlightMode mode, uint8_t component_id);

    // We use std::pair instead of a std::optional.
    std::pair<MAVLinkCommands::Result, MAVLinkCommands::CommandLong>
    make_command_msg_rate(uint16_t message_id, double rate_hz, uint8_t component_id);

    static void receive_float_param(MAVLinkParameters::Result result,
                                    MAVLinkParameters::ParamValue value,
                                    get_param_float_callback_t callback);
    static void receive_int_param(MAVLinkParameters::Result result,
                                  MAVLinkParameters::ParamValue value,
                                  get_param_int_callback_t callback);

    discover_callback_t _component_discovered_callback{nullptr};

    struct MAVLinkHandlerTableEntry {
        uint16_t msg_id;
        mavlink_message_handler_t callback;
        const void *cookie; // This is the identification to unregister.
    };

    std::mutex _mavlink_handler_table_mutex{};
    std::vector<MAVLinkHandlerTableEntry> _mavlink_handler_table{};

    std::atomic<uint8_t> _system_id;

    uint64_t _uuid{0};

#if defined(ENABLE_FALLBACK_TO_SYSTEM_ID)
    int _uuid_retries = 0;
    uint8_t _non_autopilot_heartbeats = 0;
#endif
    std::atomic<bool> _uuid_initialized{false};

    bool _supports_mission_int{false};
    std::atomic<bool> _armed{false};
    std::atomic<bool> _hitl_enabled{false};

    DronecodeSDKImpl &_parent;

    command_result_callback_t _command_result_callback{nullptr};

    std::thread *_system_thread{nullptr};
    std::atomic<bool> _should_exit{false};

    static constexpr double _HEARTBEAT_TIMEOUT_S = 3.0;

    std::mutex _connection_mutex{};
    bool _connected{false};
    void *_heartbeat_timeout_cookie = nullptr;

    std::atomic<bool> _autopilot_version_pending{false};
    void *_autopilot_version_timed_out_cookie = nullptr;

    static constexpr double _HEARTBEAT_SEND_INTERVAL_S = 1.0;

    MAVLinkParameters _params;

    MAVLinkCommands _commands;

    TimeoutHandler _timeout_handler;
    CallEveryHandler _call_every_handler;

    Time _time{};

    std::mutex _plugin_impls_mutex{};
    std::vector<PluginImplBase *> _plugin_impls{};

    // We used set to maintain unique component ids
    std::unordered_set<uint8_t> _components{};

    ThreadPool _thread_pool{3};

    bool _iterator_invalidated{false};

    std::mutex _param_changed_callbacks_mutex{};
    std::map<const void *, param_changed_callback_t> _param_changed_callbacks{};
};

} // namespace dronecode_sdk
