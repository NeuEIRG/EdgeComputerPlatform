#MAVLINK

##��̨##

*��̨��MavLinkCom��һ����ƽ̨��C ++�⣬���԰������ӵ�����MavLink�ĳ�������֮ͨ�š��ر����������Ժܺõ������PX4�����˻����ʹ�á�������Ҫʹ����MavLinkCom��TCP��UDPЭ�顣

*��̨��server.c���Խ���TX2������ͼƬ��ʹ�õ��ǵ�����TCPЭ�顣

###MavLinkCom###

*���Ǽ򵥵�ʵ���˺�̨����ָ����ʵ�����˻���ɡ�����ȹ��ܣ������̨����t��TX2�յ�������ָ����ɣ�

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

*MavLinkCom�����б�õ�CMakeLists������ֱ����linux��ʹ��cmake���������У�Ҳ�������κ�֧��C/C++�ı��뻷����ʹ�á�

###server.c###

*�ܹ�����TX2������ͼƬ��Ŀǰ�����յ���ͼƬд�뵽���أ�

```
 char DataName[1000]="/home/wz/Desktop/image/";

```