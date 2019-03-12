#pragma once

#include <mutex>
#include "plugin_impl_base.h"
#include "mavlink_include.h"
#include "system.h"
#include "plugins/offboard/offboard.h"

namespace dronecode_sdk {

class OffboardImpl : public PluginImplBase {
public:
    OffboardImpl(System &system);
    ~OffboardImpl();

    void init() override;
    void deinit() override;

    void enable() override;
    void disable() override;

    Offboard::Result start();
    Offboard::Result stop();

    void start_async(Offboard::result_callback_t callback);
    void stop_async(Offboard::result_callback_t callback);

    bool is_active() const;

    void set_velocity_ned(Offboard::VelocityNEDYaw velocity_ned_yaw);
    void set_velocity_body(Offboard::VelocityBodyYawspeed velocity_body_yawspeed);
    void set_attitude_rate(Offboard::AttitudeRate attitude_rate);

    OffboardImpl(const OffboardImpl &) = delete;
    OffboardImpl &operator=(const OffboardImpl &) = delete;

private:
    void send_velocity_ned();
    void send_velocity_body();
    void send_attitude_rate();

    void process_heartbeat(const mavlink_message_t &message);
    void receive_command_result(MAVLinkCommands::Result result,
                                const Offboard::result_callback_t &callback);

    static Offboard::Result offboard_result_from_command_result(MAVLinkCommands::Result result);

    void stop_sending_setpoints();

    mutable std::mutex _mutex{};
    enum class Mode {
        NOT_ACTIVE,
        VELOCITY_NED,
        VELOCITY_BODY,
        ATTITUDE_RATE
    } _mode = Mode::NOT_ACTIVE;
    Offboard::VelocityNEDYaw _velocity_ned_yaw{};
    Offboard::VelocityBodyYawspeed _velocity_body_yawspeed{};
    Offboard::AttitudeRate _attitude_rate{};

    void *_call_every_cookie = nullptr;

    const float SEND_INTERVAL_S = 0.1f;
};

} // namespace dronecode_sdk
