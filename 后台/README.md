#MAVLINK

##��̨##

*��̨��MavLinkCom��ʵ����tcp��udp����Ϣ����ĺ�̨���ܣ�����ͨ����̨��TX2������Ϣ��Ҳ���Խ���TX2�����ص���Ϣ
*��̨��server.c���Խ��ܵ�TX2���͵�ͼƬ��

###MavLinkCom###

*���Ǽ򵥵�ʵ���˺�̨����ָ����ʵ�����˻���ɡ�����ȹ��ܣ�

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

*�ܹ�����TX2������ͼƬ��Ŀǰ�����յ���ͼƬд�뵽���ء�

```
 char DataName[1000]="/home/wz/Desktop/image/";

```