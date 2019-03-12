#pragma once

#include <functional>
#include <memory>

#include "plugin_base.h"

#include <doinecode_flight_control.h>
#include "dji_status.hpp"
#include <dji_vehicle.hpp>

namespace dronecode_sdk {

class System;
class ActionImpl;

/**
 * @brief The Action class enables simple actions for a drone
 * such as arming, taking off, and landing.
 *
 * Synchronous and asynchronous variants of the action methods are supplied.
 *
 * The action methods send their associated MAVLink commands to the vehicle and complete
 * (return synchronously or callback asynchronously) with an Action::Result value
 * indicating whether the vehicle has accepted or rejected the command, or that there has been some
 * error.
 * If the command is accepted, the vehicle will then start to perform the associated action.
 */
class Action : public PluginBase,public dronecode_flightcontrol {
public:
   //function like Dji
   bool monitoredTakeoff(DJI::OSDK::Vehicle* vehiclePtr=null, int timeout=1);
   bool monitoredLanding(DJI::OSDK::Vehicle* vehiclePtr=null, int timeout=1);
   bool moveByPositionOffset(DJI::OSDK::Vehicle *vehicle, float xOffsetDesired,
                          float yOffsetDesired, float zOffsetDesired,
                          float yawDesired, float posThresholdInM,
                          float yawThresholdInDeg);
   bool moveByVelocity(DJI::OSDK::Vehicle* vehicle,
		      		   float32_t Vx,
                       float32_t Vy,
                       float32_t Vz,
                       float32_t yawRate,
                       double flyTime);
   void localOffsetFromGpsOffset(DJI::OSDK::Vehicle* vehicle,
                              DJI::OSDK::Telemetry::Vector3f& deltaNed,
                              void* target, void* origin);















    /**
     * @brief Constructor. Creates the plugin for a specific System.
     *
     * The plugin is typically created as shown below:
     *
     *     ```cpp
     *     auto action = std::make_shared<Action>(system);
     *     ```
     *
     * @param system The specific system associated with this plugin.
     */
    explicit Action(System &system);

    /**
     * @brief Destructor (internal use only).
     */
    ~Action();

    /**
     * @brief Possible results returned for commanded actions.
     *
     * @note DronecodeSDK does not throw exceptions. Instead a result of this type will be
     * returned when you execute actions.
     */
    enum class Result {
        UNKNOWN, /**< @brief Unspecified error. */
        SUCCESS, /**< @brief Success. The action command was accepted by the vehicle. */
        NO_SYSTEM, /**< @brief No system is connected error. */
        CONNECTION_ERROR, /**< @brief %Connection error. */
        BUSY, /**< @brief Vehicle busy error. */
        COMMAND_DENIED, /**< @brief Command refused by vehicle. */
        COMMAND_DENIED_LANDED_STATE_UNKNOWN, /**< @brief Command refused because landed state is
                                                unknown. */
        COMMAND_DENIED_NOT_LANDED, /**< @brief Command refused because vehicle not landed. */
        TIMEOUT, /**< @brief Timeout waiting for command acknowledgement from vehicle. */
        VTOL_TRANSITION_SUPPORT_UNKNOWN, /**< @brief hybrid/VTOL transition refused because VTOL
                                            support is unknown. */
        NO_VTOL_TRANSITION_SUPPORT, /**< @brief Vehicle does not support hybrid/VTOL transitions. */
        PARAMETER_ERROR /**< @brief Error getting or setting parameter. */
    };

    /**
     * @brief Send command to *arm* the drone (synchronous).
     *
     * @note Arming a drone normally causes motors to spin at idle.
     * Before arming take all safety precautions and stand clear of the drone!
     *
     * @return Result of request.
     */
    Result arm() const;

    /**
     * @brief Send command to *disarm* the drone (synchronous).
     *
     * This will disarm a drone that considers itself landed. If flying, the drone should
     * reject the disarm command. Disarming means that all motors will stop.
     *
     * @return Result of request.
     */
    Result disarm() const;

    /**
     * @brief Send command to *kill* the drone (synchronous).
     *
     * This will disarm a drone irrespective of whether it is landed or flying.
     * Note that the drone will fall out of the sky if this command is used while flying.
     *
     * @return Result of request.
     */
    Result kill() const;

    /=1)**
     * @brief Send command to *reboot* the drone components.
     *
     * This will reboot the autopilot, onboard computer, camera and gimbal.
     *
     * @return Action::Result of request.
     */
    Action::Result reboot() const;

    /**
     * @brief Send command to *take off and hover* (synchronous).
     *
     * This switches the drone into position control mode and commands it to take off and hover at
     * the takeoff altitude (set using set_takeoff_altitude()).
     *
     * Note that the vehicle must be armed before it can take off.
     *
     * @return Result of request.
     */
    Result takeoff() const;

    /**
     * @brief Send command to *land* at the current position (synchronous).
     *
     * This switches the drone to
     * [Land mode](https://docs.px4.io/en/flight_modes/land.html).
     *
     * @return Result of request.
     */
    Result land() const;

    /**
     * @brief Send command to *return to the launch* (takeoff) position and *land* (asynchronous).
     *
     * This switches the drone into [RTL mode](https://docs.px4.io/en/flight_modes/rtl.html) which
     * generally means it will rise up to a certain altitude to clear any obstacles before heading
     * back to the launch (takeoff) position and land there.
     *
     * @return Result of request.
     */
    Result return_to_launch() const;

    /**
     * @brief Send command to reposition the vehicle to a specific WGS84 global position
     *
     * This sends the vehicle to a specified lattitude/longitude/altitude coordinates.
     * @param latitude_deg Latitude in degrees
     * @param longitude_deg Longitude in degrees
     * @param altitude_amsl_m Altitude AMSL in meters
     * @param yaw_deg Yaw angle in degrees
     *
     * @return Result of request.
     */
    Result
    goto_location(double latitude_deg, double longitude_deg, float altitude_amsl_m, float yaw_deg);

    /**
     * @brief Send command to transition the drone to fixedwing.
     *
     * The associated action will only be executed for VTOL vehicles (on other vehicle types the
     * command will fail with an Result). The command will succeed if called when the vehicle
     * is already in fixedwing mode.
     *
     * @return Result of request.
     */
    Result transition_to_fixedwing() const;

    /**
     * @brief Send command to transition the drone to multicopter.
     *
     * The associated action will only be executed for VTOL vehicles (on other vehicle types the
     * command will fail with an Result). The command will succeed if called when the vehicle
     * is already in multicopter mode.
     *
     * @return Result of request.
     */
    Result transition_to_multicopter() const;

    /**
     * @brief Callback type for asynchronous Action calls.
     */
    typedef std::function<void(Result)> result_callback_t;

    /**
     * @brief Send command to *arm* the drone (asynchronous).
     *
     * Note that arming a drone normally means that the motors will spin at idle.
     * Therefore, before arming take all safety precautions and stand clear of the drone.
     *
     * @param callback Function to call with result of request.
     */
    void arm_async(result_callback_t callback);

    /**
     * @brief Send command to *disarm* the drone (asynchronous).
     *
     * This will disarm a drone that considers itself landed. If flying, the drone should
     * reject the disarm command. Disarming means that all motors will stop.
     *
     * @param callback Function to call with result of request.
     */
    void disarm_async(result_callback_t callback);

    /**
     * @brief Send command to *kill* the drone (asynchronous).
     *
     * This will disarm a drone irrespective of whether it is landed or flying.
     * Note that the drone will fall out of the sky if you use this command while flying.
     *
     * @param callback Function to call with result of request.
     */
    void kill_async(result_callback_t callback);

    /**
     * @brief Send command to *take off and hover* (asynchronous).
     *
     * This switches the drone into position control mode and commands it to take off and hover at
     * the takeoff altitude set using set_takeoff_altitude().
     *
     * Note that the vehicle must be armed before it can take off.
     *
     * @param callback Function to call with result of request
     */
    void takeoff_async(result_callback_t callback);

    /**
     * @brief Send command to *land* at the current position (asynchronous).
     *
     * This switches the drone to
     * [Land mode](https://docs.px4.io/en/flight_modes/land.html).
     *
     * @param callback Function to call with result of request.
     */
    void land_async(result_callback_t callback);

    /**
     * @brief Send command to *return to the launch* (takeoff) position and *land*  (asynchronous).
     *
     * This switches the drone into [RTL mode](https://docs.px4.io/en/flight_modes/rtl.html) which
     * generally means it will rise up to a certain altitude to clear any obstacles before heading
     * back to the launch (takeoff) position and land there.
     *
     * @param callback Function to call with result of request.
     */
    void return_to_launch_async(result_callback_t callback);

    /**
     * @brief Send command to transition the drone to fixedwing (asynchronous).
     *
     * The associated action will only be executed for VTOL vehicles (on other vehicle types the
     * command will fail with an Result). The command will succeed if called when the vehicle
     * is already in fixedwing mode.
     *
     * @param callback Function to call with result of request.
     */
    void transition_to_fixedwing_async(result_callback_t callback);

    /**
     * @brief Send command to transition the drone to multicopter (asynchronous).
     *
     * The associated action will only be executed for VTOL vehicles (on other vehicle types the
     * command will fail with an Result). The command will succeed if called when the vehicle
     * is already in multicopter mode.
     *
     * @param callback Function to call with result of request.
     */
    void transition_to_multicopter_async(result_callback_t callback);

    /**
     * @brief Set takeoff altitude above ground.
     *
     * @param relative_altitude_m Takeoff altitude relative to takeoff location, in meters.
     * @return Result of request.
     */
    Result set_takeoff_altitude(float relative_altitude_m);

    /**
     * @brief Get the takeoff altitude.
     *
     * @return A pair containing the result of request and if successful, the
     * takeoff altitude relative to ground/takeoff location, in meters.
     */
    std::pair<Result, float> get_takeoff_altitude() const;

    /**
     * @brief Set vehicle maximum speed.
     *
     * @param speed_m_s Maximum speed in metres/second.
     * @return Result of request.
     */
    Result set_max_speed(float speed_m_s);

    /**
     * @brief Get the vehicle maximum speed.
     *
     * @return A pair containing the result of the request and if successful, the
     * maximum speed in metres/second.
     */
    std::pair<Result, float> get_max_speed() const;

    /**
     * @brief Set the return to launch minimum return altitude.
     *
     * When return to launch is initiated, the vehicle climbs to the return altitude if it is lower
     * and stays at the current altitude if higher than the return altitude. Then it returns to the
     * home location and lands there.
     *
     * @param relative_altitude_m Return altitude relative to takeoff location, in meters.
     * @return Result of request.
     */
    Result set_return_to_launch_return_altitude(float relative_altitude_m);

    /**
     * @brief Get the return to launch minimum return altitude.
     *
     * @sa `set_return_to_launch_return_altitude`.
     *
     * @return A pair containing the result of the request and if successful, the
     * return altitude relative to takeoff location, in meters.
     */
    std::pair<Result, float> get_return_to_launch_return_altitude() const;

    /**
     * @brief Returns a human-readable English string for an Result.
     *
     * @param result The enum value for which a human readable string is required.
     * @return Human readable string for the Result.
     */
    static const char *result_str(Result result);

    /**
     * @brief Copy constructor (object is not copyable).
     */
    Action(const Action &) = delete;
    /**
     * @brief Equality operator (object is not copyable).
     */
    const Action &operator=(const Action &) = delete;

private:
    /** @private Underlying implementation, set at instantiation */
    std::unique_ptr<ActionImpl> _impl;
};

} // namespace dronecode_sdk
