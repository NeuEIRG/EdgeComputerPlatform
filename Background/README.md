# MAVLINK

## ��̨

*��̨��MavLinkCom��һ����ƽ̨��C ++�⣬���԰������ӵ�����MavLink�ĳ�������֮ͨ�š��ر����������Ժܺõ������PX4�����˻����ʹ�á�������Ҫʹ����MavLinkCom��TCP��UDPЭ�顣

*��̨��server.c���Խ���TX2������ͼƬ��ʹ�õ��ǵ�����TCPЭ�顣

### MavLinkCom

*���Ǽ򵥵�ʵ���˺�̨����ָ����ʵ�����˻���ɡ�����ȹ��ܣ������̨����t��TX2�յ�������ָ����ɣ�

```
  UdpPingTest()��MavLinkCom�Դ���udp���������Ƕ���������һЩ�޸ģ�ʹ���ܹ���TX2�ϵ�ǰ���໥ͨ�У��ɷ��ͺ��֣���

  void UnitTests::UdpPingTest()

  TcpPingTest()��MavLinkCom�Դ���tcp�������޸�֮����������TX2��ǰ�˷���ָ������˻�����һЩ�򵥵Ĳ����� 

  UnitTests::TcpPingTest ()

```

  *MavLinkCom�����б�õ�CMakeLists������ֱ����linux��ʹ��cmake���������У�Ҳ�������κ�֧��C/C++�ı��뻷����ʹ�á�

### server.c

  *�ܹ�����TX2������ͼƬ��Ŀǰ�����յ���ͼƬд�뵽���أ�

```
  ���յ���ͼƬ��Ϣд�뵽�涨��·����
    
  struct sockaddr_in client_addr;
  socklen_t size=sizeof(client_addr);
  if( (connfd = accept(listenfd, (struct sockaddr*)&client_addr, &size)) == -1){
     printf("accept socket error: %s(errno: %d)",strerror(errno),errno);
     continue;
  }
  while(1){
     n = read(connfd, buff, MAXLINE);
     if(n == 0)
         break;
     fwrite(buff, 1, n, fp);
     //fwrite ( const void * ptr, size_t size, size_t count, FILE * stream );
��   //���У�ptr��ָ�򱣴����ݵ�ָ�룻size��ÿ���������͵Ĵ�С��count�����ݵĸ�����stream���ļ�ָ��
  }


  ����ͼƬ�����·����
 
  char DataName[1000]="/home/wz/Desktop/image/";
```