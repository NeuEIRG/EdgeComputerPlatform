# MAVLINK

*这个项目的后台可以通过发送到TX2的指令执行相应的操作。

## Cloud ##

*后台的MavLinkCom中实现了tcp、udp等信息传输的后台功能，可以通过后台向TX2发送信息，也可以接收TX2反馈回的信息，MavLinkCom  的具体功能和结构详见MavLinkCom的readme。

*后台的server.c可以接受到TX2发送的图片。


## Edge ##

*前端中webcam-v4l2-master的功能是视屏采集，并将采集的视屏逐帧发送到后台。

*Integration edition SDK-2019-1-17是将无人机的SDK与MavLinkCom代码整合后的项目，能够与后台和无人机通信。






