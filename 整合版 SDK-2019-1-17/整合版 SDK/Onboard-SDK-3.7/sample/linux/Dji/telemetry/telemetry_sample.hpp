/*! @file telemetry_sample.hpp
 *  @version 3.3
 *  @date Jun 05 2017
 *
 *  @brief
 *  Telemetry API usage in a Linux environment.
 *  Shows example usage of the new data subscription API.
 *
 *  @Copyright (c) 2017 DJI
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

#ifndef DJIOSDK_TELEMETRYSAMPLE_HPP
#define DJIOSDK_TELEMETRYSAMPLE_HPP

// System Includes
#include <iostream>
#include <dronecode_data.h>
// DJI OSDK includes
#include <dji_vehicle.hpp>

// Helpers
#include <dji_linux_helpers.hpp>

class TelData {
public:
	int elapsedTimeInMs;
	uint8_t flight; //Status
	float64_t latitude; //GlobalPosition
	float64_t longitude;
	float32_t altitude;
	int16_t roll;     //RC
	int16_t pitch;
	int16_t yaw;
	int16_t throttle;
	float32_t x;//Vector3f
	float32_t y;
	float32_t z;
	float32_t q0; /*!< w */ //Quaternion
	float32_t q1; /*!< x */
	float32_t q2; /*!< y */
	float32_t q3; /*!< z */
};
class Dji_data:public dronecode_data{
public :
    bool subscribeToData(DJI::OSDK::Vehicle* vehiclePtr, int responseTimeout = 1);
    bool subscribeToDataForInteractivePrint(DJI::OSDK::Vehicle* vehiclePtr, int responseTimeout = 1);
    bool subscribeToDataAndSaveLogToFile(DJI::OSDK::Vehicle* vehiclePtr, int responseTimeout = 1);

// Broadcast data implementation for Matrice 100
    bool getBroadcastData(DJI::OSDK::Vehicle* vehicle, int responseTimeout = 1);
    bool getBroadcastData(DJI::OSDK::Vehicle* vehicle,
                 int                 responseTimeout,
                 TelData&            teldata);
    void putBroadcastDataInCache(DJI::OSDK::Vehicle* vehicle, int responseTimeout=1 );
        TelData getBroadcastDataWithCache();

//thread test
    void creat_thread(DJI::OSDK::Vehicle* vehicle,int responseTimeout=1);
//void get_top();

};

// cache QUEUE_

namespace xq
{
using namespace std;

const int MAX_QUEUE_SIZE = 50;
template<typename T>
class xqQueue
{
private:
  T*  base;
  int front;
  int rear;
  int size;

public:
  xqQueue();

  xqQueue(int n);

  bool push(T);

  bool pop();

  bool isEmpty();

  void print();

  T getRear();
};

template<typename T>
xqQueue<T>::xqQueue()
{
  base  = new T[MAX_QUEUE_SIZE];
  front = rear = 0;
  size         = MAX_QUEUE_SIZE;
}

template<typename T>
xqQueue<T>::xqQueue(int n)
{
  base  = new T[n];
  front = rear = 0;
  size         = n;
}

template<typename T>
bool
xqQueue<T>::isEmpty()
{
  if (rear == front)
    return true;

  return false;
}


template<typename T>
bool
xqQueue<T>::pop()
{
  if (isEmpty())
  {
    cout << "queue is empty!" << endl;
    return false;
  }

 // *num  = base[front];
  front = (front + 1) % size;
}


template<typename T>
bool
xqQueue<T>::push(T num)
{
  if ((rear + 1) % size == front)//队满，弹出队头
  {
    //cout << "queue is full!" << endl;
    //return false;
    pop();
  }

  base[rear % size] = num;
  rear              = (rear + 1) % size;
}

template<typename T>
void
xqQueue<T>::print()
{
  if (isEmpty())
  {
    cout << "queue is empty!" << endl;
    return;
  }

  int p = front;
  int q = rear;
  while (p % size != q)
  {
    cout << base[p] << ' ';
    p = (p + 1) % size;
  }
  cout << endl;
}

template<typename T>
T xqQueue<T>::getRear(){
  return base[rear];
}
}









#endif // DJIOSDK_TELEMETRYSAMPLE_HPP
