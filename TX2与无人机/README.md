# MAVLINK

## webcam-v4l2-master

*这个项目主要功能是视屏采集

## 整合版 SDK-2019-1-17

### MavLinkCom

*这里的MavLinkCom与后台的MavLinkCom能够相互通信

```
  UdpPingTest()方法与后台的UdpPingTest()能够相互通信
  
  TcpPingTest(DJI::OSDK::Vehicle* vehicle)方法能够接收后台发送的指令并对无人机进行操作
  switch(d){
     case 't':
       std::cout<<"qqq"<<std::endl;
       monitoredTakeoff(vehicle);
       break;
     case 'l':
       std::cout<<"www"<<std::endl;
       monitoredLanding(vehicle);
       break;
     case 'g':
       std::cout<<"www"<<std::endl;
       moveByPositionOffset(vehicle,1,6,7,0);
       break;
     default:
       break;
   }
  d即为TCP接受到的消息，比如收到的消息为't'则调用monitoredTakeoff()方法,控制无人机的起飞
```

### Onboard-SDK-3.7
```
提供了关于无人机起飞前准备、飞行控制以及获取数据的一系列接口
```

起飞前准备：
```
通过LinuxSetup类进行初始化OSDK环境以及获取无人机控制权
LinuxSetup
```

飞行控制类有：
```
起飞函数：
bool monitoredTakeoff(DJI::OSDK::Vehicle* vehiclePtr, int timeout = 1);

降落函数：
bool monitoredLanding(DJI::OSDK::Vehicle* vehiclePtr, int timeout = 1);
通过指定方向和距离的点到点飞行：
bool moveByPositionOffset(DJI::OSDK::Vehicle *vehicle, float xOffsetDesired,
                          float yOffsetDesired, float zOffsetDesired,
                          float yawDesired, float posThresholdInM = 0.2,
                          float yawThresholdInDeg = 1.0);
                          
通过给定三维空间中的速度来控制飞行：
void localOffsetFromGpsOffset(DJI::OSDK::Vehicle*             vehicle,
                              DJI::OSDK::Telemetry::Vector3f& deltaNed,
                              void* target, void* origin);
```
获得遥测信息：
```
获取缓存队列中最新的遥测数据：
TelData getBroadcastDataWithCache();

创建遥测数：
void creat_cachedata_thread(DJI::OSDK::Vehicle* vehicle,int responseTimeout=1);
```


