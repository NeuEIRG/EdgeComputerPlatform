#MAVLINK

##后台##

*后台的MavLinkCom是一个跨平台的C ++库，可以帮助连接到基于MavLink的车辆并与之通信。特别是这个库可以很好地与基于PX4的无人机配合使用。我们主要使用了MavLinkCom的TCP与UDP协议。

*后台的server.c可以接受TX2发来的图片，使用的是单独的TCP协议。

###MavLinkCom###

*我们简单的实现了后台发送指令以实现无人机起飞、降落等功能，比如后台发送t，TX2收到后会根据指令起飞：

```
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
```

*MavLinkCom中已有编好的CMakeLists，可以直接在linux下使用cmake的命令运行，也可以在任何支持C/C++的编译环境中使用。

###server.c###

*能够接收TX2传来的图片，目前将接收到的图片写入到本地，

```
 char DataName[1000]="/home/wz/Desktop/image/";

```