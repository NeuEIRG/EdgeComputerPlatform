# MAVLINK

## webcam-v4l2-master

*�����Ŀ��Ҫ�����������ɼ�

## ���ϰ� SDK-2019-1-17

### MavLinkCom

*�����MavLinkCom���̨��MavLinkCom�ܹ��໥ͨ��

```
  UdpPingTest()�������̨��UdpPingTest()�ܹ��໥ͨ��
  
  TcpPingTest(DJI::OSDK::Vehicle* vehicle)�����ܹ����պ�̨���͵�ָ������˻����в���
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
  d��ΪTCP���ܵ�����Ϣ�������յ�����ϢΪ't'�����monitoredTakeoff()����,�������˻������
```



