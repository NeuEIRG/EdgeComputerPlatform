#pragma once

#ifndef DRONECODE_data_H
#define DRONECODE_data_H


#include <cmath>
// DJI OSDK includes
#include "dji_status.hpp"
#include <dji_vehicle.hpp>

// Helpers
#include <dji_linux_helpers.hpp>


class dronecode_data{
public:
    TelData getBroadcastDataWithCache();//获取缓存队列中最新的遥测数据
    void creat_cachedata_thread(DJI::OSDK::Vehicle* vehicle,int responseTimeout);//创建遥测数据线程，实现每1/60将遥测数据存入缓存
};


#endif
