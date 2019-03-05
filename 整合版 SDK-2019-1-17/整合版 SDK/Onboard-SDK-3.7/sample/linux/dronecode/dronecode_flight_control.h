#pragma once

#ifndef DRONECODE_FLIGHT_CONTROL_H
#define DRONECODE_FLIGHT_CONTROL_H

#include <cmath>
// DJI OSDK includes
#include "dji_status.hpp"
#include <dji_vehicle.hpp>

// Helpers
#include <dji_linux_helpers.hpp>
class dronecode_flightcontrol{
public:
    virtual bool monitoredTakeoff(DJI::OSDK::Vehicle* vehiclePtr, int timeout){return ture;}
    virtual bool monitoredLanding(DJI::OSDK::Vehicle* vehiclePtr, int timeout){return ture;}
    virtual bool moveByPositionOffset(DJI::OSDK::Vehicle *vehicle, float xOffsetDesired,
                          float yOffsetDesired, float zOffsetDesired,
                          float yawDesired, float posThresholdInM,
                          float yawThresholdInDeg){return ture;}
    virtual bool moveByVelocity(DJI::OSDK::Vehicle* vehicle,
		      		   float32_t Vx,
                       float32_t Vy,
                       float32_t Vz,
                       float32_t yawRate,
                       double flyTime){return ture;}
    virtual void localOffsetFromGpsOffset(DJI::OSDK::Vehicle* vehicle,
                              DJI::OSDK::Telemetry::Vector3f& deltaNed,
                              void* target, void* origin){}
};
#endif
