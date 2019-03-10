#MAVLINK

##后台##

*后台的MavLinkCom中实现了tcp、udp等信息传输的后台功能，可以通过后台向TX2发送信息，也可以接收TX2反馈回的信息
*后台的server.c可以接受到TX2发送的图片。

###MavLinkCom###

*我们简单的实现了后台发送指令以实现无人机起飞、降落等功能：

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

*能够接收TX2传来的图片，目前将接收到的图片写入到本地。

```
 char DataName[1000]="/home/wz/Desktop/image/";

```