#include "follow_me_impl.h"
#include "system.h"
#include "global_include.h"
#include "px4_custom_mode.h"

namespace dronecode_sdk {

FollowMeImpl::FollowMeImpl(System &system) : PluginImplBase(system)
{
    // (Lat, Lon, Alt) => double, (vx, vy, vz) => float
    _last_location = _target_location =
        FollowMe::TargetLocation{double(NAN), double(NAN), double(NAN), NAN, NAN, NAN};
    _parent->register_plugin(this);
}

FollowMeImpl::~FollowMeImpl()
{
    _parent->unregister_plugin(this);
}

void FollowMeImpl::init()
{
    _parent->register_mavlink_message_handler(
        MAVLINK_MSG_ID_HEARTBEAT,
        std::bind(&FollowMeImpl::process_heartbeat, this, std::placeholders::_1),
        static_cast<void *>(this));
}

void FollowMeImpl::deinit()
{
    _parent->unregister_all_mavlink_message_handlers(this);
}

void FollowMeImpl::enable()
{
    _parent->get_param_float_async("NAV_MIN_FT_HT",
                                   [this](MAVLinkParameters::Result result, float value) {
                                       if (result == MAVLinkParameters::Result::SUCCESS) {
                                           _config.min_height_m = value;
                                       }
                                   });
    _parent->get_param_float_async("NAV_FT_DST",
                                   [this](MAVLinkParameters::Result result, float value) {
                                       if (result == MAVLinkParameters::Result::SUCCESS) {
                                           _config.follow_distance_m = value;
                                       }
                                   });
    _parent->get_param_int_async(
        "NAV_FT_FS", [this](MAVLinkParameters::Result result, int32_t value) {
            if (result == MAVLinkParameters::Result::SUCCESS) {
                _config.follow_direction = static_cast<FollowMe::Config::FollowDirection>(value);
            }
        });
    _parent->get_param_float_async("NAV_FT_RS",
                                   [this](MAVLinkParameters::Result result, float value) {
                                       if (result == MAVLinkParameters::Result::SUCCESS) {
                                           _config.responsiveness = value;
                                       }
                                   });
}

void FollowMeImpl::disable()
{
    stop_sending_target_location();
}

const FollowMe::Config &FollowMeImpl::get_config() const
{
    return _config;
}

FollowMe::Result FollowMeImpl::set_config(const FollowMe::Config &config)
{
    // Valdidate configuration
    if (!is_config_ok(config)) {
        LogErr() << debug_str << "set_config() failed. Last configuration is preserved.";
        return FollowMe::Result::SET_CONFIG_FAILED;
    }

    auto height = config.min_height_m;
    auto distance = config.follow_distance_m;
    int32_t direction = static_cast<int32_t>(config.follow_direction);
    auto responsiveness = config.responsiveness;

    LogDebug() << "Waiting for the system confirmation of the new configuration..";

    bool success = true;

    // Send configuration to Vehicle
    if (_config.min_height_m != height) {
        if (_parent->set_param_float("NAV_MIN_FT_HT", height) ==
            MAVLinkParameters::Result::SUCCESS) {
            _config.min_height_m = height;
        } else {
            success = false;
        }
    }
    if (_config.follow_distance_m != distance) {
        if (_parent->set_param_float("NAV_FT_DST", distance) ==
            MAVLinkParameters::Result::SUCCESS) {
            _config.follow_distance_m = distance;
        } else {
            success = false;
        }
    }
    if (_config.follow_direction != config.follow_direction) {
        if (_parent->set_param_int("NAV_FT_FS", direction) == MAVLinkParameters::Result::SUCCESS) {
            _config.follow_direction = static_cast<FollowMe::Config::FollowDirection>(direction);

        } else {
            success = false;
        }
    }
    if (_config.responsiveness != responsiveness) {
        if (_parent->set_param_float("NAV_FT_RS", responsiveness) ==
            MAVLinkParameters::Result::SUCCESS) {
            _config.responsiveness = responsiveness;
        } else {
            success = false;
        }
    }

    return (success ? FollowMe::Result::SUCCESS : FollowMe::Result::SET_CONFIG_FAILED);
}

void FollowMeImpl::set_target_location(const FollowMe::TargetLocation &location)
{
    _mutex.lock();
    _target_location = location;
    // We're interested only in lat, long.
    _estimatation_capabilities |= (1 << static_cast<int>(EstimationCapabilites::POS));

    if (_mode != Mode::ACTIVE) {
        _mutex.unlock();
        return;
    }
    // If set already, reschedule it.
    if (_target_location_cookie) {
        _parent->reset_call_every(_target_location_cookie);
        _target_location_cookie = nullptr;
    } else {
        // Regiter now for sending in the next cycle.
        _parent->add_call_every(
            [this]() { send_target_location(); }, SENDER_RATE, &_target_location_cookie);
    }
    _mutex.unlock();

    // Send it immediately for now.
    send_target_location();
}

const FollowMe::TargetLocation &FollowMeImpl::get_last_location() const
{
    std::lock_guard<std::mutex> lock(_mutex);
    return _last_location;
}

bool FollowMeImpl::is_active() const
{
    std::lock_guard<std::mutex> lock(_mutex);
    return _mode == Mode::ACTIVE;
}

FollowMe::Result FollowMeImpl::start()
{
    FollowMe::Result result =
        to_follow_me_result(_parent->set_flight_mode(SystemImpl::FlightMode::FOLLOW_ME));

    if (result == FollowMe::Result::SUCCESS) {
        // If location was set before, lets send it to vehicle
        std::lock_guard<std::mutex> lock(
            _mutex); // locking is not necessary here but lets do it for integrity
        if (is_target_location_set()) {
            _parent->add_call_every(
                [this]() { send_target_location(); }, SENDER_RATE, &_target_location_cookie);
        }
    }
    return result;
}

FollowMe::Result FollowMeImpl::stop()
{
    {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_mode == Mode::ACTIVE) {
            stop_sending_target_location();
        }
    }
    return to_follow_me_result(_parent->set_flight_mode(SystemImpl::FlightMode::HOLD));
}

bool FollowMeImpl::is_config_ok(const FollowMe::Config &config) const
{
    auto config_ok = false;

    if (config.min_height_m < FollowMe::Config::MIN_HEIGHT_M) {
        LogErr() << debug_str << "Err: Min height must be atleast 8.0 meters";
    } else if (config.follow_distance_m < FollowMe::Config::MIN_FOLLOW_DIST_M) {
        LogErr() << debug_str << "Err: Min Follow distance must be atleast 1.0 meter";
    } else if (config.follow_direction < FollowMe::Config::FollowDirection::FRONT_RIGHT ||
               config.follow_direction > FollowMe::Config::FollowDirection::NONE) {
        LogErr() << debug_str << "Err: Invalid Follow direction";
    } else if (config.responsiveness < FollowMe::Config::MIN_RESPONSIVENESS ||
               config.responsiveness > FollowMe::Config::MAX_RESPONSIVENESS) {
        LogErr() << debug_str << "Err: Responsiveness must be in range (0.0 to 1.0)";
    } else { // Config is OK
        config_ok = true;
    }

    return config_ok;
}

FollowMe::Result FollowMeImpl::to_follow_me_result(MAVLinkCommands::Result result) const
{
    switch (result) {
        case MAVLinkCommands::Result::SUCCESS:
            return FollowMe::Result::SUCCESS;
        case MAVLinkCommands::Result::NO_SYSTEM:
            return FollowMe::Result::NO_SYSTEM;
        case MAVLinkCommands::Result::CONNECTION_ERROR:
            return FollowMe::Result::CONNECTION_ERROR;
        case MAVLinkCommands::Result::BUSY:
            return FollowMe::Result::BUSY;
        case MAVLinkCommands::Result::COMMAND_DENIED:
            return FollowMe::Result::COMMAND_DENIED;
        case MAVLinkCommands::Result::TIMEOUT:
            return FollowMe::Result::TIMEOUT;
        default:
            return FollowMe::Result::UNKNOWN;
    }
}

bool FollowMeImpl::is_target_location_set() const
{
    // If the target's latitude is NAN, we assume that location is not set.
    // We assume that mutex was acquired by the caller
    return std::isfinite(_target_location.latitude_deg);
}

void FollowMeImpl::send_target_location()
{
    // Don't send if we're not in FollowMe mode.
    if (!is_active()) {
        return;
    }

    dl_time_t now = _time.steady_time();
    // needed by http://mavlink.org/messages/common#FOLLOW_TARGET
    uint64_t elapsed_msec =
        static_cast<uint64_t>(_time.elapsed_since_s(now) * 1000); // milliseconds

    _mutex.lock();
    //   LogDebug() << debug_str <<  "Lat: " << _target_location.latitude_deg << " Lon: " <<
    //   _target_location.longitude_deg <<
    //	" Alt: " << _target_location.absolute_altitude_m;
    const int32_t lat_int = static_cast<int32_t>(_target_location.latitude_deg * 1e7);
    const int32_t lon_int = static_cast<int32_t>(_target_location.longitude_deg * 1e7);
    const float alt = static_cast<float>(_target_location.absolute_altitude_m);
    _mutex.unlock();

    const float pos_std_dev[] = {NAN, NAN, NAN};
    const float vel[] = {NAN, NAN, NAN};
    const float accel_unknown[] = {NAN, NAN, NAN};
    const float attitude_q_unknown[] = {1.f, NAN, NAN, NAN};
    const float rates_unknown[] = {NAN, NAN, NAN};
    uint64_t custom_state = 0;

    mavlink_message_t msg{};
    mavlink_msg_follow_target_pack(GCSClient::system_id,
                                   GCSClient::component_id,
                                   &msg,
                                   elapsed_msec,
                                   _estimatation_capabilities,
                                   lat_int,
                                   lon_int,
                                   alt,
                                   vel,
                                   accel_unknown,
                                   attitude_q_unknown,
                                   rates_unknown,
                                   pos_std_dev,
                                   custom_state);

    if (!_parent->send_message(msg)) {
        LogErr() << debug_str << "send_target_location() failed..";
    } else {
        std::lock_guard<std::mutex> lock(_mutex);
        _last_location = _target_location;
    }
}

void FollowMeImpl::stop_sending_target_location()
{
    // We assume that mutex was acquired by the caller
    if (_target_location_cookie) {
        _parent->remove_call_every(_target_location_cookie);
        _target_location_cookie = nullptr;
    }
    _mode = Mode::NOT_ACTIVE;
}

void FollowMeImpl::process_heartbeat(const mavlink_message_t &message)
{
    mavlink_heartbeat_t heartbeat;
    mavlink_msg_heartbeat_decode(&message, &heartbeat);

    bool follow_me_active = false; // tells whether we're in FollowMe mode right now
    if (heartbeat.base_mode & MAV_MODE_FLAG_CUSTOM_MODE_ENABLED) {
        px4::px4_custom_mode px4_custom_mode;
        px4_custom_mode.data = heartbeat.custom_mode;

        if (px4_custom_mode.main_mode == px4::PX4_CUSTOM_MAIN_MODE_AUTO &&
            px4_custom_mode.sub_mode == px4::PX4_CUSTOM_SUB_MODE_AUTO_FOLLOW_TARGET) {
            follow_me_active = true; // we're in FollowMe mode
        }
    }

    {
        std::lock_guard<std::mutex> lock(_mutex);
        if (!follow_me_active && _mode == Mode::ACTIVE) {
            // We're NOT in FollowMe mode anymore.
            // Lets stop sending target location updates
            stop_sending_target_location();
        } else if (follow_me_active && _mode == Mode::NOT_ACTIVE) {
            // We're in FollowMe mode now
            _mode = Mode::ACTIVE;
            _mutex.unlock(); // we must unlock to avoid deadlock in send_target_location()
            return;
        }
    }
}

} // namespace dronecode_sdk
