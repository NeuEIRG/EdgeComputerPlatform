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
    TelData getBroadcastDataWithCache();//��ȡ������������µ�ң������
    void creat_cachedata_thread(DJI::OSDK::Vehicle* vehicle,int responseTimeout);//����ң�������̣߳�ʵ��ÿ1/60��ң�����ݴ��뻺��
};


#endif
