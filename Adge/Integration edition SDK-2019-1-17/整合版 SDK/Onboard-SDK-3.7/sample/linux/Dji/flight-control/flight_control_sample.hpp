/*! @file flight_control_sample.hpp
 *  @version 3.3
 *  @date Jun 05 2017
 *
 *  @brief
 *  Flight Control API usage in a Linux environment.
 *  Provides a number of helpful additions to core API calls,
 *  especially for position control, attitude control, takeoff,
 *  landing.
 *
 *  @Copyright (c) 2016-2017 DJI
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#ifndef DJIOSDK_FLIGHTCONTROL_HPP
#define DJIOSDK_FLIGHTCONTROL_HPP

// System Includes
#include <cmath>
#include <vector>
// DJI OSDK includes
#include "dji_status.hpp"
#include <dji_vehicle.hpp>
#include <doinecode_flight_control.h>
// Helpers
#include <dji_linux_helpers.hpp>

#define C_EARTH (double)6378137.0
#define DEG2RAD 0.01745329252

//!@note: All the default timeout parameters are for acknowledgement packets
//! from the aircraft.

/*! Monitored Takeoff
    This implementation of takeoff  with monitoring makes sure your aircraft
    actually took off and only returns when takeoff is complete.
    Use unless you want to do other stuff during takeoff - this will block
    the main thread.
!*/
class Dji_flightcontrol:public dronecode_flightcontrol{
public :
bool monitoredTakeoff(DJI::OSDK::Vehicle* vehiclePtr, int timeout = 1);

// Examples of commonly used Flight Mode APIs

/*! Position Control. Allows you to set an offset from your current
    location. The aircraft will move to that position and stay there.
    Typical use would be as a building block in an outer loop that does not
    require many fast changes, perhaps a few-waypoint trajectory. For smoother
    transition and response you should convert your trajectory to attitude
    setpoints and use attitude control or convert to velocity setpoints
    and use velocity control.
!*/
bool moveByPositionOffset(DJI::OSDK::Vehicle *vehicle,
			  float xOffsetDesired,
                          float yOffsetDesired,
			  float zOffsetDesired,
                          float yawDesired,
			  float posThresholdInM = 0.2,
                          float yawThresholdInDeg = 1.0);



/*! @brief Control the velocity and yaw rate of the vehicle.
 *  The reference frame is the DJI::OSDK::Control::HORIZONTAL_GROUND (NEU).
 *
 *  @param Vx velocity set-point in x axis of ground frame (m/s), input limit
 * see DJI::OSDK::Control::HORIZONTAL_VELOCITY
 *  @param Vy velocity set-point in y axis of ground frame (m/s), input limit
 * see DJI::OSDK::Control::HORIZONTAL_VELOCITY
 *  @param Vz velocity set-point in z axis of ground frame (m/s), input limit
 * see DJI::OSDK::Control::VERTICAL_VELOCITY
 *  @param yawRate yawRate set-point (deg/s)
 */
bool
moveByVelocity(DJI::OSDK::Vehicle* vehicle,
		       float32_t Vx=0,
                       float32_t Vy=0,
                       float32_t Vz=0,
                       float32_t yawRate=0,
			double      flyTime=1);




/*! @brief Control the attitude and vertical position of the vehicle
 *
 *  @param roll   attitude set-point in x axis of body frame (FRU) (deg),
 * input limit see DJI::OSDK::Control::HORIZONTAL_ANGLE
 *  @param pitch  attitude set-point in y axis of body frame (FRU) (deg),
 * input limit see DJI::OSDK::Control::HORIZONTAL_ANGLE
 *  @param z      z position set-point in z axis of ground frame (NED) (m),
 * input limit see DJI::OSDK::Control::VERTICAL_POSITION
 *  @param yaw    attitude set-point in z axis of ground frame (NED) (deg)
 */

/*
bool
moveByAttitude(DJI::OSDK::Vehicle* vehicle,
		       float32_t roll,
                       float32_t pitch,
                       float32_t yaw,
                       float32_t z);


*/


/*! Monitored Landing (Blocking API call). Return status as well as ack.
    This version of takeoff makes sure your aircraft actually took off
    and only returns when takeoff is complete.

!*/
bool monitoredLanding(DJI::OSDK::Vehicle* vehiclePtr, int timeout = 1);

// Helper Functions

/*! Very simple calculation of local NED offset between two pairs of GPS
 * coordinates.
 *
 * Accurate when distances are small.
!*/
void localOffsetFromGpsOffset(DJI::OSDK::Vehicle*             vehicle,
                              DJI::OSDK::Telemetry::Vector3f& deltaNed,
                              void* target, void* origin);

DJI::OSDK::Telemetry::Vector3f toEulerAngle(void* quaternionData);
bool startGlobalPositionBroadcast(DJI::OSDK::Vehicle* vehicle);
static bool missionContinue=true;

};

#endif // DJIOSDK_FLIGHTCONTROL_HPP
