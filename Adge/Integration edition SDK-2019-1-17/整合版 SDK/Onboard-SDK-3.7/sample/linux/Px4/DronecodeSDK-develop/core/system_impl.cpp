#include "system.h"
#include "global_include.h"
#include "dronecode_sdk_impl.h"
#include "mavlink_include.h"
#include "system_impl.h"
#include "plugin_impl_base.h"
#include "px4_custom_mode.h"
#include <functional>
#include <algorithm>
#include <future>

// Set to 1 to log incoming/outgoing mavlink messages.
#define MESSAGE_DEBUGGING 0

namespace dronecode_sdk {

using namespace std::placeholders; // for `_1`

SystemImpl::SystemImpl(DronecodeSDKImpl &parent, uint8_t system_id, uint8_t comp_id) :
    _system_id(system_id),
    _parent(parent),
    _params(*this),
    _commands(*this),
    _timeout_handler(_time),
    _call_every_handler(_time)
{
    _system_thread = new std::thread(&SystemImpl::system_thread, this);

    register_mavlink_message_handler(
        MAVLINK_MSG_ID_HEARTBEAT, std::bind(&SystemImpl::process_heartbeat, this, _1), this);

    // We're registering for Autopilot version because it is a good time do so,
    // regardless whether we deal with Autopilot.
    register_mavlink_message_handler(MAVLINK_MSG_ID_AUTOPILOT_VERSION,
                                     std::bind(&SystemImpl::process_autopilot_version, this, _1),
                                     this);

    register_mavlink_message_handler(
        MAVLINK_MSG_ID_STATUSTEXT, std::bind(&SystemImpl::process_statustext, this, _1), this);

    add_new_component(comp_id);

    // FIXME: It would be better to do things like this in a method and not
    //        in the constructor where we can't fail gracefully because we
    //        don't have exceptions.
    _thread_pool.start();
}

SystemImpl::~SystemImpl()
{
    _should_exit = true;
    unregister_all_mavlink_message_handlers(this);

    unregister_timeout_handler(_autopilot_version_timed_out_cookie);
    unregister_timeout_handler(_heartbeat_timeout_cookie);

    _thread_pool.stop();

    if (_system_thread != nullptr) {
        _system_thread->join();
        delete _system_thread;
        _system_thread = nullptr;
    }
}

bool SystemImpl::is_connected() const
{
    return _connected;
}

void SystemImpl::register_mavlink_message_handler(uint16_t msg_id,
                                                  mavlink_message_handler_t callback,
                                                  const void *cookie)
{
    std::lock_guard<std::mutex> lock(_mavlink_handler_table_mutex);

    MAVLinkHandlerTableEntry entry = {msg_id, callback, cookie};
    _mavlink_handler_table.push_back(entry);
}

void SystemImpl::unregister_all_mavlink_message_handlers(const void *cookie)
{
    std::lock_guard<std::mutex> lock(_mavlink_handler_table_mutex);

    for (auto it = _mavlink_handler_table.begin(); it != _mavlink_handler_table.end();
         /* no ++it */) {
        if (it->cookie == cookie) {
            it = _mavlink_handler_table.erase(it);
            _iterator_invalidated = true;
        } else {
            ++it;
        }
    }
}

void SystemImpl::register_timeout_handler(std::function<void()> callback,
                                          double duration_s,
                                          void **cookie)
{
    _timeout_handler.add(callback, duration_s, cookie);
}

void SystemImpl::refresh_timeout_handler(const void *cookie)
{
    _timeout_handler.refresh(cookie);
}

void SystemImpl::unregister_timeout_handler(const void *cookie)
{
    _timeout_handler.remove(cookie);
}

void SystemImpl::process_mavlink_message(const mavlink_message_t &message)
{
    _mavlink_handler_table_mutex.lock();

#if MESSAGE_DEBUGGING == 1
    bool forwarded = false;
#endif
    for (auto it = _mavlink_handler_table.begin(); it != _mavlink_handler_table.end(); /* ++it */) {
        if (it->msg_id == message.msgid) {
#if MESSAGE_DEBUGGING == 1
            LogDebug() << "Forwarding msg " << int(message.msgid) << " to " << size_t(it->cookie);
            forwarded = true;
#endif
            _mavlink_handler_table_mutex.unlock();
            it->callback(message);
            _mavlink_handler_table_mutex.lock();
        }

        if (_iterator_invalidated) {
            // Someone messed with the map while we were doing the callback.
            // We need to start over. This means that we might call something twice but that's
            // probably better than to drop the message.
            it = _mavlink_handler_table.begin();
            _iterator_invalidated = false;
        } else {
            ++it;
        }
    }
    _mavlink_handler_table_mutex.unlock();

#if MESSAGE_DEBUGGING == 1
    if (!forwarded) {
        LogDebug() << "Ignoring msg " << int(message.msgid);
    }
#endif
}

void SystemImpl::add_call_every(std::function<void()> callback, float interval_s, void **cookie)
{
    _call_every_handler.add(callback, interval_s, cookie);
}

void SystemImpl::change_call_every(float interval_s, const void *cookie)
{
    _call_every_handler.change(interval_s, cookie);
}

void SystemImpl::reset_call_every(const void *cookie)
{
    _call_every_handler.reset(cookie);
}

void SystemImpl::remove_call_every(const void *cookie)
{
    _call_every_handler.remove(cookie);
}

void SystemImpl::process_heartbeat(const mavlink_message_t &message)
{
    // FIXME: for now we ignore heartbeats from UDP_BRIDGE because that's just
    // confusing since it doesn't mean a vehicle is connected.
    if (message.compid == MAV_COMP_ID_UDP_BRIDGE) {
        return;
    }

    mavlink_heartbeat_t heartbeat;
    mavlink_msg_heartbeat_decode(&message, &heartbeat);

    if (message.compid == MAVLinkCommands::DEFAULT_COMPONENT_ID_AUTOPILOT) {
        _armed = ((heartbeat.base_mode & MAV_MODE_FLAG_SAFETY_ARMED) ? true : false);
        _hitl_enabled = ((heartbeat.base_mode & MAV_MODE_FLAG_HIL_ENABLED) ? true : false);
    }

    // We do not call on_discovery here but wait with the notification until we know the UUID.

    /* If the component is an autopilot and
     * we don't know its UUID, then try to find out. */
    if (is_autopilot(message.compid) && !have_uuid()) {
        request_autopilot_version();

#if defined(ENABLE_FALLBACK_TO_SYSTEM_ID)
    } else if (!is_autopilot(message.compid) && !have_uuid() && ++_non_autopilot_heartbeats >= 10) {
        // We've received consecutive heartbeats (atleast twice) from a
        // non-autopilot system! Lets not delay for filling UUID anymore.
        _uuid = message.sysid;
        _uuid_initialized = true;
#endif
    }

    set_connected();
}

void SystemImpl::process_autopilot_version(const mavlink_message_t &message)
{
    // Ignore if they don't come from the autopilot component
    if (message.compid != MAVLinkCommands::DEFAULT_COMPONENT_ID_AUTOPILOT) {
        return;
    }

    mavlink_autopilot_version_t autopilot_version;
    mavlink_msg_autopilot_version_decode(&message, &autopilot_version);

    _supports_mission_int =
        ((autopilot_version.capabilities & MAV_PROTOCOL_CAPABILITY_MISSION_INT) ? true : false);

    if (_uuid == 0 && autopilot_version.uid != 0) {
        // This is the best case. The system has a UUID and we were able to get it.
        _uuid = autopilot_version.uid;

    } else if (_uuid == 0 && autopilot_version.uid == 0) {
        // This is not ideal because the system has no valid UUID.
        // In this case we use the mavlink system ID as the UUID.
        _uuid = _system_id;

    } else if (_uuid != autopilot_version.uid) {
        // TODO: this is bad, we should raise a flag to invalidate system.
        LogErr() << "Error: UUID changed";
    }

    _uuid_initialized = true;
    set_connected();

    _autopilot_version_pending = false;
    unregister_timeout_handler(_autopilot_version_timed_out_cookie);
}

void SystemImpl::process_statustext(const mavlink_message_t &message)
{
    mavlink_statustext_t statustext;
    mavlink_msg_statustext_decode(&message, &statustext);

    std::string debug_str = "MAVLink: ";

    switch (statustext.severity) {
        case MAV_SEVERITY_EMERGENCY:
            debug_str += "emergency";
            break;
        case MAV_SEVERITY_ALERT:
            debug_str += "alert";
            break;
        case MAV_SEVERITY_CRITICAL:
            debug_str += "critical";
            break;
        case MAV_SEVERITY_ERROR:
            debug_str += "error";
            break;
        case MAV_SEVERITY_WARNING:
            debug_str += "warning";
            break;
        case MAV_SEVERITY_NOTICE:
            debug_str += "notice";
            break;
        case MAV_SEVERITY_INFO:
            debug_str += "info";
            break;
        case MAV_SEVERITY_DEBUG:
            debug_str += "debug";
            break;
        default:
            break;
    }

    // statustext.text is not null terminated, therefore we copy it first to
    // an array big enough that is zeroed.
    char text_with_null[sizeof(statustext.text) + 1]{};
    memcpy(text_with_null, statustext.text, sizeof(statustext.text));

    LogDebug() << debug_str << ": " << text_with_null;
}

void SystemImpl::heartbeats_timed_out()
{
    LogInfo() << "heartbeats timed out";
    set_disconnected();
}

void SystemImpl::system_thread()
{
    dl_time_t last_time{};

    while (!_should_exit) {
        if (_time.elapsed_since_s(last_time) >= SystemImpl::_HEARTBEAT_SEND_INTERVAL_S) {
            send_heartbeat();
            last_time = _time.steady_time();
        }

        _call_every_handler.run_once();
        _timeout_handler.run_once();
        _params.do_work();
        _commands.do_work();

        if (_connected) {
            // Work fairly fast if we're connected.
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        } else {
            // Be less aggressive when unconnected.
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
}

std::string SystemImpl::component_name(uint8_t component_id)
{
    switch (component_id) {
        case MAV_COMP_ID_AUTOPILOT1:
            return "Autopilot";
        case MAV_COMP_ID_CAMERA:
            return "Camera 1";
        case MAV_COMP_ID_CAMERA2:
            return "Camera 2";
        case MAV_COMP_ID_CAMERA3:
            return "Camera 3";
        case MAV_COMP_ID_CAMERA4:
            return "Camera 4";
        case MAV_COMP_ID_CAMERA5:
            return "Camera 5";
        case MAV_COMP_ID_CAMERA6:
            return "Camera 6";
        case MAV_COMP_ID_GIMBAL:
            return "Gimbal";
        default:
            return "Unsupported component";
    }
}

ComponentType SystemImpl::component_type(uint8_t component_id)
{
    switch (component_id) {
        case MAV_COMP_ID_AUTOPILOT1:
            return AUTOPILOT;
        case MAV_COMP_ID_CAMERA:
        case MAV_COMP_ID_CAMERA2:
        case MAV_COMP_ID_CAMERA3:
        case MAV_COMP_ID_CAMERA4:
        case MAV_COMP_ID_CAMERA5:
        case MAV_COMP_ID_CAMERA6:
            return CAMERA;
        case MAV_COMP_ID_GIMBAL:
            return GIMBAL;
        default:
            return UNKNOWN;
    }
}

void SystemImpl::add_new_component(uint8_t component_id)
{
    if (component_id == 0) {
        return;
    }

    auto res_pair = _components.insert(component_id);
    if (res_pair.second) {
        if (_component_discovered_callback != nullptr) {
            const ComponentType type = component_type(component_id);
            call_user_callback([this, type]() { _component_discovered_callback(type); });
        }
        LogDebug() << "Component " << component_name(component_id) << " (" << int(component_id)
                   << ") added.";
    }
}

size_t SystemImpl::total_components() const
{
    return _components.size();
}

void SystemImpl::register_component_discovered_callback(discover_callback_t callback)
{
    _component_discovered_callback = callback;

    if (total_components() > 0) {
        for (const auto &elem : _components) {
            const ComponentType type = component_type(elem);
            call_user_callback([this, type]() { _component_discovered_callback(type); });
        }
    }
}

bool SystemImpl::is_standalone() const
{
    return !has_autopilot();
}

bool SystemImpl::has_autopilot() const
{
    return get_autopilot_id() != uint8_t(0);
}

bool SystemImpl::is_autopilot(uint8_t comp_id)
{
    return comp_id == MAV_COMP_ID_AUTOPILOT1;
}

bool SystemImpl::is_camera(uint8_t comp_id)
{
    return (comp_id >= MAV_COMP_ID_CAMERA) && (comp_id <= MAV_COMP_ID_CAMERA6);
}

bool SystemImpl::has_camera(int camera_id) const
{
    int camera_comp_id = (camera_id == -1) ? camera_id : (MAV_COMP_ID_CAMERA + camera_id);

    if (camera_comp_id == -1) { // Check whether the system has any camera.
        for (auto compid : _components) {
            if (is_camera(compid)) {
                return true;
            }
        }
    } else { // Look for the camera whose id is `camera_id`.
        for (auto compid : _components) {
            if (compid == camera_comp_id) {
                return true;
            }
        }
    }
    return false;
}

bool SystemImpl::has_gimbal() const
{
    return get_gimbal_id() == MAV_COMP_ID_GIMBAL;
}

void SystemImpl::send_heartbeat()
{
    mavlink_message_t message;
    // GCSClient is not autopilot!; hence MAV_AUTOPILOT_INVALID.
    mavlink_msg_heartbeat_pack(GCSClient::system_id,
                               GCSClient::component_id,
                               &message,
                               MAV_TYPE_GCS,
                               MAV_AUTOPILOT_INVALID,
                               0,
                               0,
                               0);
    send_message(message);
}

bool SystemImpl::send_message(const mavlink_message_t &message)
{
#if MESSAGE_DEBUGGING == 1
    LogDebug() << "Sending msg " << size_t(message.msgid);
#endif
    return _parent.send_message(message);
}

void SystemImpl::request_autopilot_version()
{
    if (_uuid_initialized) {
        // Already initialized, we can exit.
        return;
    }

#if defined(ENABLE_FALLBACK_TO_SYSTEM_ID)
    if (!_autopilot_version_pending && _uuid_retries >= 3) {
        // We give up getting a UUID and use the system ID.

        LogWarn() << "No UUID received, using system ID instead.";
        _uuid = _system_id;
        _uuid_initialized = true;
        set_connected();
        return;
    }
#endif

    _autopilot_version_pending = true;

    send_autopilot_version_request();

#if defined(ENABLE_FALLBACK_TO_SYSTEM_ID)
    ++_uuid_retries;
#endif

    // We set a timeout to stay "pending" for half a second. This way, we don't give up too
    // early e.g. because multiple components send heartbeats and we receive them all at once
    // and run out of retries.

    // We create a temp reference, so we don't need to capture `this`.
    auto &pending_tmp = _autopilot_version_pending;
    register_timeout_handler(
        [&pending_tmp]() { pending_tmp = false; }, 0.5, &_autopilot_version_timed_out_cookie);
}

void SystemImpl::send_autopilot_version_request()
{
    // We don't care about an answer, we mostly care about receiving AUTOPILOT_VERSION.
    MAVLinkCommands::CommandLong command{};

    command.command = MAV_CMD_REQUEST_AUTOPILOT_CAPABILITIES;
    command.params.param1 = 1.0f;
    command.target_component_id = get_autopilot_id();

    send_command_async(command, nullptr);
}

void SystemImpl::set_connected()
{
    bool enable_needed = false;
    {
        std::lock_guard<std::mutex> lock(_connection_mutex);

        if (!_connected && _uuid_initialized) {
            LogDebug() << "Discovered " << _components.size() << " component(s) "
                       << "(UUID: " << _uuid << ")";

            _parent.notify_on_discover(_uuid);
            _connected = true;

            register_timeout_handler(std::bind(&SystemImpl::heartbeats_timed_out, this),
                                     _HEARTBEAT_TIMEOUT_S,
                                     &_heartbeat_timeout_cookie);
            enable_needed = true;

        } else if (_connected) {
            refresh_timeout_handler(_heartbeat_timeout_cookie);
        }
        // If not yet connected there is nothing to do/
    }
    if (enable_needed) {
        std::lock_guard<std::mutex> lock(_plugin_impls_mutex);
        for (auto plugin_impl : _plugin_impls) {
            plugin_impl->enable();
        }
    }
}

void SystemImpl::set_disconnected()
{
    {
        std::lock_guard<std::mutex> lock(_connection_mutex);

        // This might not be needed because this is probably called from the triggered
        // timeout anyway but it should also do no harm.
        // unregister_timeout_handler(_heartbeat_timeout_cookie);
        //_heartbeat_timeout_cookie = nullptr;

        _connected = false;
        _parent.notify_on_timeout(_uuid);
    }

    {
        std::lock_guard<std::mutex> lock(_plugin_impls_mutex);
        for (auto plugin_impl : _plugin_impls) {
            plugin_impl->disable();
        }
    }
}

uint64_t SystemImpl::get_uuid() const
{
    // We want to support UUIDs if the autopilot tells us.
    return _uuid;
}

uint8_t SystemImpl::get_system_id() const
{
    return _system_id;
}

void SystemImpl::set_system_id(uint8_t system_id)
{
    _system_id = system_id;
}

MAVLinkParameters::Result SystemImpl::set_param_float(const std::string &name, float value)
{
    MAVLinkParameters::ParamValue param_value;
    param_value.set_float(value);

    return _params.set_param(name, param_value, false);
}

MAVLinkParameters::Result SystemImpl::set_param_int(const std::string &name, int32_t value)
{
    MAVLinkParameters::ParamValue param_value;
    param_value.set_int32(value);

    return _params.set_param(name, param_value, false);
}

MAVLinkParameters::Result SystemImpl::set_param_ext_float(const std::string &name, float value)
{
    MAVLinkParameters::ParamValue param_value;
    param_value.set_float(value);

    return _params.set_param(name, param_value, true);
}

MAVLinkParameters::Result SystemImpl::set_param_ext_int(const std::string &name, int32_t value)
{
    MAVLinkParameters::ParamValue param_value;
    param_value.set_int32(value);

    return _params.set_param(name, param_value, true);
}

void SystemImpl::set_param_float_async(const std::string &name, float value, success_t callback)
{
    MAVLinkParameters::ParamValue param_value;
    param_value.set_float(value);
    _params.set_param_async(name, param_value, callback);
}

void SystemImpl::set_param_int_async(const std::string &name, int32_t value, success_t callback)
{
    MAVLinkParameters::ParamValue param_value;
    param_value.set_int32(value);
    _params.set_param_async(name, param_value, callback);
}

void SystemImpl::set_param_ext_float_async(const std::string &name, float value, success_t callback)
{
    MAVLinkParameters::ParamValue param_value;
    param_value.set_float(value);
    _params.set_param_async(name, param_value, callback, true);
}

void SystemImpl::set_param_ext_int_async(const std::string &name, int32_t value, success_t callback)
{
    MAVLinkParameters::ParamValue param_value;
    param_value.set_int32(value);
    _params.set_param_async(name, param_value, callback, true);
}

std::pair<MAVLinkParameters::Result, float> SystemImpl::get_param_float(const std::string &name)
{
    auto prom = std::promise<std::pair<MAVLinkParameters::Result, float>>();
    auto res = prom.get_future();

    MAVLinkParameters::ParamValue value_type;
    value_type.set_float(0.0f);

    _params.get_param_async(
        name,
        value_type,
        [&prom](MAVLinkParameters::Result result, MAVLinkParameters::ParamValue param) {
            float value = NAN;
            if (result == MAVLinkParameters::Result::SUCCESS) {
                value = param.get_float();
            }
            prom.set_value(std::make_pair<>(result, value));
        });

    return res.get();
}

std::pair<MAVLinkParameters::Result, int> SystemImpl::get_param_int(const std::string &name)
{
    auto prom = std::promise<std::pair<MAVLinkParameters::Result, int>>();
    auto res = prom.get_future();

    MAVLinkParameters::ParamValue value_type;
    value_type.set_int32(0);

    _params.get_param_async(
        name,
        value_type,
        [&prom](MAVLinkParameters::Result result, MAVLinkParameters::ParamValue param) {
            int value = 0;
            if (result == MAVLinkParameters::Result::SUCCESS) {
                value = param.get_int32();
            }
            prom.set_value(std::make_pair<>(result, value));
        });

    return res.get();
}

std::pair<MAVLinkParameters::Result, float> SystemImpl::get_param_ext_float(const std::string &name)
{
    auto prom = std::promise<std::pair<MAVLinkParameters::Result, float>>();
    auto res = prom.get_future();

    MAVLinkParameters::ParamValue value_type;
    value_type.set_float(0.0f);

    _params.get_param_async(
        name,
        value_type,
        [&prom](MAVLinkParameters::Result result, MAVLinkParameters::ParamValue param) {
            float value = NAN;
            if (result == MAVLinkParameters::Result::SUCCESS) {
                value = param.get_float();
            }
            prom.set_value(std::make_pair<>(result, value));
        },
        true);

    return res.get();
}

std::pair<MAVLinkParameters::Result, int> SystemImpl::get_param_ext_int(const std::string &name)
{
    auto prom = std::promise<std::pair<MAVLinkParameters::Result, int>>();
    auto res = prom.get_future();

    MAVLinkParameters::ParamValue value_type;
    value_type.set_int32(0);

    _params.get_param_async(
        name,
        value_type,
        [&prom](MAVLinkParameters::Result result, MAVLinkParameters::ParamValue param) {
            int value = 0;
            if (result == MAVLinkParameters::Result::SUCCESS) {
                value = param.get_int32();
            }
            prom.set_value(std::make_pair<>(result, value));
        },
        true);

    return res.get();
}

void SystemImpl::get_param_float_async(const std::string &name, get_param_float_callback_t callback)
{
    MAVLinkParameters::ParamValue value_type;
    value_type.set_float(0.0f);

    _params.get_param_async(
        name, value_type, std::bind(&SystemImpl::receive_float_param, _1, _2, callback));
}

void SystemImpl::get_param_int_async(const std::string &name, get_param_int_callback_t callback)
{
    MAVLinkParameters::ParamValue value_type;
    value_type.set_int32(0);

    _params.get_param_async(
        name, value_type, std::bind(&SystemImpl::receive_int_param, _1, _2, callback));
}

void SystemImpl::get_param_ext_float_async(const std::string &name,
                                           get_param_float_callback_t callback)
{
    MAVLinkParameters::ParamValue value_type;
    value_type.set_float(0.0f);

    _params.get_param_async(
        name, value_type, std::bind(&SystemImpl::receive_float_param, _1, _2, callback), true);
}

void SystemImpl::get_param_ext_int_async(const std::string &name, get_param_int_callback_t callback)
{
    MAVLinkParameters::ParamValue value_type;
    value_type.set_int32(0);

    _params.get_param_async(
        name, value_type, std::bind(&SystemImpl::receive_int_param, _1, _2, callback), true);
}

void SystemImpl::set_param_async(const std::string &name,
                                 MAVLinkParameters::ParamValue value,
                                 success_t callback,
                                 bool extended)
{
    _params.set_param_async(name, value, callback, extended);
}

MAVLinkParameters::Result
SystemImpl::set_param(const std::string &name, MAVLinkParameters::ParamValue value, bool extended)
{
    return _params.set_param(name, value, extended);
}

void SystemImpl::get_param_async(const std::string &name,
                                 MAVLinkParameters::ParamValue value_type,
                                 get_param_callback_t callback,
                                 bool extended)
{
    _params.get_param_async(name, value_type, callback, extended);
}

std::pair<MAVLinkCommands::Result, MAVLinkCommands::CommandLong>
SystemImpl::make_command_flight_mode(FlightMode flight_mode, uint8_t component_id)
{
    const uint8_t flag_safety_armed = is_armed() ? MAV_MODE_FLAG_SAFETY_ARMED : 0;
    const uint8_t flag_hitl_enabled = _hitl_enabled ? MAV_MODE_FLAG_HIL_ENABLED : 0;

    const uint8_t mode = MAV_MODE_FLAG_CUSTOM_MODE_ENABLED | flag_safety_armed | flag_hitl_enabled;

    // Note: the safety flag is not needed in future versions of the PX4 Firmware
    //       but want to be rather safe than sorry.
    uint8_t custom_mode = px4::PX4_CUSTOM_MAIN_MODE_AUTO;
    uint8_t custom_sub_mode = 0;

    switch (flight_mode) {
        case FlightMode::HOLD:
            custom_sub_mode = px4::PX4_CUSTOM_SUB_MODE_AUTO_LOITER;
            break;
        case FlightMode::RETURN_TO_LAUNCH:
            custom_sub_mode = px4::PX4_CUSTOM_SUB_MODE_AUTO_RTL;
            break;
        case FlightMode::TAKEOFF:
            custom_sub_mode = px4::PX4_CUSTOM_SUB_MODE_AUTO_TAKEOFF;
            break;
        case FlightMode::LAND:
            custom_sub_mode = px4::PX4_CUSTOM_SUB_MODE_AUTO_LAND;
            break;
        case FlightMode::MISSION:
            custom_sub_mode = px4::PX4_CUSTOM_SUB_MODE_AUTO_MISSION;
            break;
        case FlightMode::FOLLOW_ME:
            custom_sub_mode = px4::PX4_CUSTOM_SUB_MODE_AUTO_FOLLOW_TARGET;
            break;
        case FlightMode::OFFBOARD:
            custom_mode = px4::PX4_CUSTOM_MAIN_MODE_OFFBOARD;
            break;
        default:
            LogErr() << "Unknown Flight mode.";
            MAVLinkCommands::CommandLong empty_command{};
            return std::make_pair<>(MAVLinkCommands::Result::UNKNOWN_ERROR, empty_command);
    }

    MAVLinkCommands::CommandLong command{};

    command.command = MAV_CMD_DO_SET_MODE;
    command.params.param1 = float(mode);
    command.params.param2 = float(custom_mode);
    command.params.param3 = float(custom_sub_mode);
    command.target_component_id = component_id;

    return std::make_pair<>(MAVLinkCommands::Result::SUCCESS, command);
}

MAVLinkCommands::Result SystemImpl::set_flight_mode(FlightMode system_mode, uint8_t component_id)
{
    std::pair<MAVLinkCommands::Result, MAVLinkCommands::CommandLong> result =
        make_command_flight_mode(system_mode, component_id);

    if (result.first != MAVLinkCommands::Result::SUCCESS) {
        return result.first;
    }

    return send_command(result.second);
}

void SystemImpl::set_flight_mode_async(FlightMode system_mode,
                                       command_result_callback_t callback,
                                       uint8_t component_id)
{
    std::pair<MAVLinkCommands::Result, MAVLinkCommands::CommandLong> result =
        make_command_flight_mode(system_mode, component_id);

    if (result.first != MAVLinkCommands::Result::SUCCESS) {
        if (callback) {
            callback(result.first, NAN);
        }
        return;
    }

    send_command_async(result.second, callback);
}

void SystemImpl::receive_float_param(MAVLinkParameters::Result result,
                                     MAVLinkParameters::ParamValue value,
                                     get_param_float_callback_t callback)
{
    if (callback) {
        if (result == MAVLinkParameters::Result::SUCCESS) {
            callback(result, value.get_float());
        } else {
            callback(result, NAN);
        }
    }
}

void SystemImpl::receive_int_param(MAVLinkParameters::Result result,
                                   MAVLinkParameters::ParamValue value,
                                   get_param_int_callback_t callback)
{
    if (callback) {
        if (result == MAVLinkParameters::Result::SUCCESS) {
            callback(result, value.get_int32());
        } else {
            callback(result, 0);
        }
    }
}

uint8_t SystemImpl::get_autopilot_id() const
{
    for (auto compid : _components)
        if (compid == MAVLinkCommands::DEFAULT_COMPONENT_ID_AUTOPILOT) {
            return compid;
        }
    // FIXME: Not sure what should be returned if autopilot is not found
    return uint8_t(0);
}

std::vector<uint8_t> SystemImpl::get_camera_ids() const
{
    std::vector<uint8_t> camera_ids{};

    for (auto compid : _components)
        if (compid >= MAV_COMP_ID_CAMERA && compid <= MAV_COMP_ID_CAMERA6) {
            camera_ids.push_back(compid);
        }
    return camera_ids;
}

uint8_t SystemImpl::get_gimbal_id() const
{
    for (auto compid : _components)
        if (compid == MAV_COMP_ID_GIMBAL) {
            return compid;
        }
    return uint8_t(0);
}

MAVLinkCommands::Result SystemImpl::send_command(MAVLinkCommands::CommandLong &command)
{
    if (_system_id == 0 && _components.size() == 0) {
        return MAVLinkCommands::Result::NO_SYSTEM;
    }
    command.target_system_id = get_system_id();
    return _commands.send_command(command);
}

MAVLinkCommands::Result SystemImpl::send_command(MAVLinkCommands::CommandInt &command)
{
    if (_system_id == 0 && _components.size() == 0) {
        return MAVLinkCommands::Result::NO_SYSTEM;
    }
    command.target_system_id = get_system_id();
    return _commands.send_command(command);
}

void SystemImpl::send_command_async(MAVLinkCommands::CommandLong &command,
                                    const command_result_callback_t callback)
{
    if (_system_id == 0 && _components.size() == 0) {
        if (callback) {
            callback(MAVLinkCommands::Result::NO_SYSTEM, NAN);
        }
        return;
    }
    command.target_system_id = get_system_id();

    _commands.queue_command_async(command, callback);
}

void SystemImpl::send_command_async(MAVLinkCommands::CommandInt &command,
                                    const command_result_callback_t callback)
{
    if (_system_id == 0 && _components.size() == 0) {
        if (callback) {
            callback(MAVLinkCommands::Result::NO_SYSTEM, NAN);
        }
        return;
    }
    command.target_system_id = get_system_id();

    _commands.queue_command_async(command, callback);
}

MAVLinkCommands::Result
SystemImpl::set_msg_rate(uint16_t message_id, double rate_hz, uint8_t component_id)
{
    std::pair<MAVLinkCommands::Result, MAVLinkCommands::CommandLong> result =
        make_command_msg_rate(message_id, rate_hz, component_id);
    if (result.first == MAVLinkCommands::Result::SUCCESS) {
        return send_command(result.second);
    }

    return result.first;
}

void SystemImpl::set_msg_rate_async(uint16_t message_id,
                                    double rate_hz,
                                    command_result_callback_t callback,
                                    uint8_t component_id)
{
    std::pair<MAVLinkCommands::Result, MAVLinkCommands::CommandLong> result =
        make_command_msg_rate(message_id, rate_hz, component_id);
    if (result.first == MAVLinkCommands::Result::SUCCESS) {
        send_command_async(result.second, callback);
    } else {
        if (callback) {
            callback(result.first, NAN);
        }
    }
}

std::pair<MAVLinkCommands::Result, MAVLinkCommands::CommandLong>
SystemImpl::make_command_msg_rate(uint16_t message_id, double rate_hz, uint8_t component_id)
{
    MAVLinkCommands::CommandLong command{};

    // If left at -1 it will stop the message stream.
    float interval_us = -1.0f;
    if (rate_hz > 0) {
        interval_us = 1e6f / static_cast<float>(rate_hz);
    } else {
        LogErr() << "Rate(Hz) is invalid: %f" << rate_hz;
        return std::make_pair<>(MAVLinkCommands::Result::UNKNOWN_ERROR, command);
    }

    command.command = MAV_CMD_SET_MESSAGE_INTERVAL;
    command.params.param1 = float(message_id);
    command.params.param2 = interval_us;
    command.target_component_id = component_id;

    return std::make_pair<>(MAVLinkCommands::Result::SUCCESS, command);
}

void SystemImpl::register_plugin(PluginImplBase *plugin_impl)
{
    assert(plugin_impl);

    plugin_impl->init();

    {
        std::lock_guard<std::mutex> lock(_plugin_impls_mutex);
        _plugin_impls.push_back(plugin_impl);
    }

    // If we're connected already, let's enable it straightaway.
    if (_connected) {
        plugin_impl->enable();
    }
}

void SystemImpl::unregister_plugin(PluginImplBase *plugin_impl)
{
    assert(plugin_impl);

    plugin_impl->disable();
    plugin_impl->deinit();

    // Remove first, so it won't get enabled/disabled anymore.
    {
        std::lock_guard<std::mutex> lock(_plugin_impls_mutex);
        auto found = std::find(_plugin_impls.begin(), _plugin_impls.end(), plugin_impl);
        if (found != _plugin_impls.end()) {
            _plugin_impls.erase(found);
        }
    }
}

void SystemImpl::call_user_callback(const std::function<void()> &func)
{
    _thread_pool.enqueue(func);
}

void SystemImpl::param_changed(const std::string &name)
{
    _params.remove_from_cache(name);

    std::lock_guard<std::mutex> lock(_param_changed_callbacks_mutex);

    for (auto &callback : _param_changed_callbacks) {
        callback.second(name);
    }
}

void SystemImpl::register_param_changed_handler(const param_changed_callback_t callback,
                                                const void *cookie)
{
    if (!callback) {
        LogErr() << "No callback for param_changed_handler supplied.";
        return;
    }

    if (!cookie) {
        LogErr() << "No callback for param_changed_handler supplied.";
        return;
    }

    std::lock_guard<std::mutex> lock(_param_changed_callbacks_mutex);

    _param_changed_callbacks[cookie] = callback;
}

void SystemImpl::unregister_param_changed_handler(const void *cookie)
{
    std::lock_guard<std::mutex> lock(_param_changed_callbacks_mutex);

    auto it = _param_changed_callbacks.find(cookie);
    if (it == _param_changed_callbacks.end()) {
        LogWarn() << "param_changed_handler for cookie not found";
        return;
    }
    _param_changed_callbacks.erase(it);
}

} // namespace dronecode_sdk
