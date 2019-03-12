# MAVLINK

## 后台

*后台的MavLinkCom是一个跨平台的C ++库，可以帮助连接到基于MavLink的车辆并与之通信。特别是这个库可以很好地与基于PX4的无人机配合使用。我们主要使用了MavLinkCom的TCP与UDP协议。

*后台的server.c可以接受TX2发来的图片，使用的是单独的TCP协议。

### MavLinkCom

*我们简单的实现了后台发送指令以实现无人机起飞、降落等功能，比如后台发送t，TX2收到后会根据指令起飞：

```
  UdpPingTest()是MavLinkCom自带的udp方法，我们对它进行了一些修改，使其能够与TX2上的前端相互通行（可发送汉字）。

  void UnitTests::UdpPingTest()

  TcpPingTest()是MavLinkCom自带的tcp方法，修改之后它可以向TX2的前端发送指令，对无人机进行一些简单的操作。 

  UnitTests::TcpPingTest ()

```

  *MavLinkCom中已有编好的CMakeLists，可以直接在linux下使用cmake的命令运行，也可以在任何支持C/C++的编译环境中使用。

### server.c

  *能够接收TX2传来的图片，目前将接收到的图片写入到本地，

```
  将收到的图片信息写入到规定的路径中
    
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
　   //其中，ptr：指向保存数据的指针；size：每个数据类型的大小；count：数据的个数；stream：文件指针
  }


  这是图片保存的路径：
 
  char DataName[1000]="/home/wz/Desktop/image/";
```