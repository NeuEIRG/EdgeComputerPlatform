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



