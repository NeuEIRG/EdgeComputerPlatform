#include "plugins/telemetry/telemetry.h"

#include <limits>
#include <cmath>

#include "telemetry_impl.h"

namespace dronecode_sdk {

Telemetry::Telemetry(System &system) : PluginBase(), _impl{new TelemetryImpl(system)} {}

Telemetry::~Telemetry() {}

Telemetry::Result Telemetry::set_rate_position_velocity_ned(double rate_hz)
{
    return _impl->set_rate_position_velocity_ned(rate_hz);
}

Telemetry::Result Telemetry::set_rate_position(double rate_hz)
{
    return _impl->set_rate_position(rate_hz);
}

Telemetry::Result Telemetry::set_rate_home_position(double rate_hz)
{
    return _impl->set_rate_home_position(rate_hz);
}

Telemetry::Result Telemetry::set_rate_in_air(double rate_hz)
{
    return _impl->set_rate_in_air(rate_hz);
}

Telemetry::Result Telemetry::set_rate_attitude(double rate_hz)
{
    return _impl->set_rate_attitude(rate_hz);
}

Telemetry::Result Telemetry::set_rate_camera_attitude(double rate_hz)
{
    return _impl->set_rate_camera_attitude(rate_hz);
}

Telemetry::Result Telemetry::set_rate_ground_speed_ned(double rate_hz)
{
    return _impl->set_rate_ground_speed_ned(rate_hz);
}

Telemetry::Result Telemetry::set_rate_gps_info(double rate_hz)
{
    return _impl->set_rate_gps_info(rate_hz);
}

Telemetry::Result Telemetry::set_rate_battery(double rate_hz)
{
    return _impl->set_rate_battery(rate_hz);
}

Telemetry::Result Telemetry::set_rate_rc_status(double rate_hz)
{
    return _impl->set_rate_rc_status(rate_hz);
}

void Telemetry::set_rate_position_velocity_ned_async(double rate_hz, result_callback_t callback)
{
    _impl->set_rate_position_velocity_ned_async(rate_hz, callback);
}

void Telemetry::set_rate_position_async(double rate_hz, result_callback_t callback)
{
    _impl->set_rate_position_async(rate_hz, callback);
}

void Telemetry::set_rate_home_position_async(double rate_hz, result_callback_t callback)
{
    _impl->set_rate_home_position_async(rate_hz, callback);
}

void Telemetry::set_rate_in_air_async(double rate_hz, result_callback_t callback)
{
    _impl->set_rate_in_air_async(rate_hz, callback);
}

void Telemetry::set_rate_attitude_async(double rate_hz, result_callback_t callback)
{
    _impl->set_rate_attitude_async(rate_hz, callback);
}

void Telemetry::set_rate_camera_attitude_async(double rate_hz, result_callback_t callback)
{
    _impl->set_rate_camera_attitude_async(rate_hz, callback);
}

void Telemetry::set_rate_ground_speed_ned_async(double rate_hz, result_callback_t callback)
{
    _impl->set_rate_ground_speed_ned_async(rate_hz, callback);
}

void Telemetry::set_rate_gps_info_async(double rate_hz, result_callback_t callback)
{
    _impl->set_rate_gps_info_async(rate_hz, callback);
}

void Telemetry::set_rate_battery_async(double rate_hz, result_callback_t callback)
{
    _impl->set_rate_battery_async(rate_hz, callback);
}

void Telemetry::set_rate_rc_status_async(double rate_hz, result_callback_t callback)
{
    _impl->set_rate_rc_status_async(rate_hz, callback);
}

Telemetry::PositionVelocityNED Telemetry::position_velocity_ned() const
{
    return _impl->get_position_velocity_ned();
}

Telemetry::Position Telemetry::position() const
{
    return _impl->get_position();
}

Telemetry::Position Telemetry::home_position() const
{
    return _impl->get_home_position();
}

bool Telemetry::in_air() const
{
    return _impl->in_air();
}

bool Telemetry::armed() const
{
    return _impl->armed();
}

Telemetry::Quaternion Telemetry::attitude_quaternion() const
{
    return _impl->get_attitude_quaternion();
}

Telemetry::EulerAngle Telemetry::attitude_euler_angle() const
{
    return _impl->get_attitude_euler_angle();
}

Telemetry::Quaternion Telemetry::camera_attitude_quaternion() const
{
    return _impl->get_camera_attitude_quaternion();
}

Telemetry::EulerAngle Telemetry::camera_attitude_euler_angle() const
{
    return _impl->get_camera_attitude_euler_angle();
}

Telemetry::GroundSpeedNED Telemetry::ground_speed_ned() const
{
    return _impl->get_ground_speed_ned();
}

Telemetry::GPSInfo Telemetry::gps_info() const
{
    return _impl->get_gps_info();
}

Telemetry::Battery Telemetry::battery() const
{
    return _impl->get_battery();
}

Telemetry::FlightMode Telemetry::flight_mode() const
{
    return _impl->get_flight_mode();
}

Telemetry::Health Telemetry::health() const
{
    return _impl->get_health();
}

bool Telemetry::health_all_ok() const
{
    return _impl->get_health_all_ok();
}

Telemetry::RCStatus Telemetry::rc_status() const
{
    return _impl->get_rc_status();
}

void Telemetry::position_velocity_ned_async(position_velocity_ned_callback_t callback)
{
    return _impl->position_velocity_ned_async(callback);
}

void Telemetry::position_async(position_callback_t callback)
{
    return _impl->position_async(callback);
}

void Telemetry::home_position_async(position_callback_t callback)
{
    return _impl->home_position_async(callback);
}

void Telemetry::in_air_async(in_air_callback_t callback)
{
    return _impl->in_air_async(callback);
}

void Telemetry::armed_async(armed_callback_t callback)
{
    return _impl->armed_async(callback);
}

void Telemetry::attitude_quaternion_async(attitude_quaternion_callback_t callback)
{
    return _impl->attitude_quaternion_async(callback);
}

void Telemetry::attitude_euler_angle_async(attitude_euler_angle_callback_t callback)
{
    return _impl->attitude_euler_angle_async(callback);
}

void Telemetry::camera_attitude_quaternion_async(attitude_quaternion_callback_t callback)
{
    return _impl->camera_attitude_quaternion_async(callback);
}

void Telemetry::camera_attitude_euler_angle_async(attitude_euler_angle_callback_t callback)
{
    return _impl->camera_attitude_euler_angle_async(callback);
}

void Telemetry::ground_speed_ned_async(ground_speed_ned_callback_t callback)
{
    return _impl->ground_speed_ned_async(callback);
}

void Telemetry::gps_info_async(gps_info_callback_t callback)
{
    return _impl->gps_info_async(callback);
}

void Telemetry::battery_async(battery_callback_t callback)
{
    return _impl->battery_async(callback);
}

void Telemetry::flight_mode_async(flight_mode_callback_t callback)
{
    return _impl->flight_mode_async(callback);
}

std::string Telemetry::flight_mode_str(FlightMode flight_mode)
{
    switch (flight_mode) {
        case FlightMode::READY:
            return "Ready";
        case FlightMode::TAKEOFF:
            return "Takeoff";
        case FlightMode::HOLD:
            return "Hold";
        case FlightMode::MISSION:
            return "Mission";
        case FlightMode::RETURN_TO_LAUNCH:
            return "Return to launch";
        case FlightMode::LAND:
            return "Land";
        case FlightMode::OFFBOARD:
            return "Offboard";
        case FlightMode::FOLLOW_ME:
            return "FollowMe";
        case FlightMode::UNKNOWN:
        default:
            return "Unknown";
    }
}

void Telemetry::health_async(health_callback_t callback)
{
    return _impl->health_async(callback);
}

void Telemetry::health_all_ok_async(health_all_ok_callback_t callback)
{
    return _impl->health_all_ok_async(callback);
}

void Telemetry::rc_status_async(rc_status_callback_t callback)
{
    return _impl->rc_status_async(callback);
}

const char *Telemetry::result_str(Result result)
{
    switch (result) {
        case Result::SUCCESS:
            return "Success";
        case Result::NO_SYSTEM:
            return "No system";
        case Result::CONNECTION_ERROR:
            return "Connection error";
        case Result::BUSY:
            return "Busy";
        case Result::COMMAND_DENIED:
            return "Command denied";
        case Result::TIMEOUT:
            return "Timeout";
        case Result::UNKNOWN:
        default:
            return "Unknown";
    }
}

bool operator==(const Telemetry::PositionVelocityNED &lhs,
                const Telemetry::PositionVelocityNED &rhs)
{
    return std::fabs(lhs.position.north_m - rhs.position.north_m) <=
               std::numeric_limits<float>::epsilon() &&
           std::fabs(lhs.position.east_m - rhs.position.east_m) <=
               std::numeric_limits<float>::epsilon() &&
           std::fabs(lhs.position.down_m - rhs.position.down_m) <=
               std::numeric_limits<float>::epsilon() &&
           std::fabs(lhs.velocity.north_m_s - rhs.velocity.north_m_s) <=
               std::numeric_limits<float>::epsilon() &&
           std::fabs(lhs.velocity.east_m_s - rhs.velocity.east_m_s) <=
               std::numeric_limits<float>::epsilon() &&
           std::fabs(lhs.velocity.down_m_s - rhs.velocity.down_m_s) <=
               std::numeric_limits<float>::epsilon();
}

bool operator==(const Telemetry::Position &lhs, const Telemetry::Position &rhs)
{
    return std::abs(lhs.latitude_deg - rhs.latitude_deg) <=
               std::numeric_limits<double>::epsilon() &&
           std::abs(lhs.longitude_deg - rhs.longitude_deg) <=
               std::numeric_limits<double>::epsilon() &&
           std::abs(lhs.absolute_altitude_m - rhs.absolute_altitude_m) <=
               std::numeric_limits<float>::epsilon() &&
           std::abs(lhs.relative_altitude_m - rhs.relative_altitude_m) <=
               std::numeric_limits<float>::epsilon();
}

std::ostream &operator<<(std::ostream &str, Telemetry::Position const &position)
{
    return str << "[lat: " << position.latitude_deg << ", lon: " << position.longitude_deg
               << ", abs_alt: " << position.absolute_altitude_m
               << ", rel_alt: " << position.relative_altitude_m << "]";
}

bool operator==(const Telemetry::Health &lhs, const Telemetry::Health &rhs)
{
    return lhs.gyrometer_calibration_ok == rhs.gyrometer_calibration_ok &&
           lhs.accelerometer_calibration_ok == rhs.accelerometer_calibration_ok &&
           lhs.magnetometer_calibration_ok == rhs.magnetometer_calibration_ok &&
           lhs.level_calibration_ok == rhs.level_calibration_ok &&
           lhs.local_position_ok == rhs.local_position_ok &&
           lhs.global_position_ok == rhs.global_position_ok &&
           lhs.home_position_ok == rhs.home_position_ok;
}

std::ostream &operator<<(std::ostream &str, Telemetry::Health const &health)
{
    return str << "[gyrometer_calibration_ok: " << health.gyrometer_calibration_ok
               << ", accelerometer_calibration_ok: " << health.accelerometer_calibration_ok
               << ", magnetometer_calibration_ok: " << health.magnetometer_calibration_ok
               << ", level_calibration_ok: " << health.level_calibration_ok
               << ", local_position_ok: " << health.local_position_ok
               << ", global_position_ok: " << health.global_position_ok
               << ", home_position_ok: " << health.home_position_ok << "]";
}

bool operator==(const Telemetry::GPSInfo &lhs, const Telemetry::GPSInfo &rhs)
{
    return lhs.num_satellites == rhs.num_satellites && lhs.fix_type == rhs.fix_type;
}

std::ostream &operator<<(std::ostream &str, Telemetry::GPSInfo const &gps_info)
{
    return str << "[num_sat: " << gps_info.num_satellites << ", fix_type: " << gps_info.fix_type
               << "]";
}

bool operator==(const Telemetry::Battery &lhs, const Telemetry::Battery &rhs)
{
    return lhs.voltage_v == rhs.voltage_v && lhs.remaining_percent == rhs.remaining_percent;
}

std::ostream &operator<<(std::ostream &str, Telemetry::Battery const &battery)
{
    return str << "[voltage_v: " << battery.voltage_v
               << ", remaining_percent: " << battery.remaining_percent << "]";
}

bool operator==(const Telemetry::Quaternion &lhs, const Telemetry::Quaternion &rhs)
{
    return lhs.w == rhs.w && lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
}

std::ostream &operator<<(std::ostream &str, Telemetry::Quaternion const &quaternion)
{
    return str << "[w: " << quaternion.w << ", x: " << quaternion.x << ", y: " << quaternion.y
               << ", z: " << quaternion.z << "]";
}

bool operator==(const Telemetry::EulerAngle &lhs, const Telemetry::EulerAngle &rhs)
{
    return lhs.roll_deg == rhs.roll_deg && lhs.pitch_deg == rhs.pitch_deg &&
           lhs.yaw_deg == rhs.yaw_deg;
}

std::ostream &operator<<(std::ostream &str, Telemetry::EulerAngle const &euler_angle)
{
    return str << "[roll_deg: " << euler_angle.roll_deg << ", pitch_deg: " << euler_angle.pitch_deg
               << ", yaw_deg: " << euler_angle.yaw_deg << "]";
}

bool operator==(const Telemetry::GroundSpeedNED &lhs, const Telemetry::GroundSpeedNED &rhs)
{
    return lhs.velocity_north_m_s == rhs.velocity_north_m_s &&
           lhs.velocity_east_m_s == rhs.velocity_east_m_s &&
           lhs.velocity_down_m_s == rhs.velocity_down_m_s;
}

std::ostream &operator<<(std::ostream &str, Telemetry::GroundSpeedNED const &ground_speed)
{
    return str << "[velocity_north_m_s: " << ground_speed.velocity_north_m_s
               << ", velocity_east_m_s: " << ground_speed.velocity_east_m_s
               << ", velocity_down_m_s: " << ground_speed.velocity_down_m_s << "]";
}

bool operator==(const Telemetry::RCStatus &lhs, const Telemetry::RCStatus &rhs)
{
    return lhs.available_once == rhs.available_once && lhs.available == rhs.available &&
           lhs.signal_strength_percent == rhs.signal_strength_percent;
}

std::ostream &operator<<(std::ostream &str, Telemetry::RCStatus const &rc_status)
{
    return str << "[was_available_once: " << rc_status.available_once
               << ", is_available: " << rc_status.available
               << ", signal_strength_percent: " << rc_status.signal_strength_percent << "]";
}

} // namespace dronecode_sdk
