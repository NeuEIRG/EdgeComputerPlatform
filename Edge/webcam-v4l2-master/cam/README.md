基于v4l2的c++视频采集demo
====================================

构建demo
--------------
在Ubuntu16.4上构建前要确保已经安装了V4L2和libjpeg库

```
对于V4L2来说，可以直接从Ubuntu的软件源中安装：
$ sudo apt install libv4l2-dev 

对于libjpeg库, 首先要在libjpeg官网上下载源代码（ http://www.ijg.org）,再按照源码中的指示安装即可
```

之后，通过以下命令即可构建demo:
```
$ g++ -std=c++11 main.cpp webcam.cpp Jpeg2AVI.cpp list.cpp -omain -lv4l2 -ljpeg
```

运行：
--------------
通过终端运行“./main”即可运行程序：
```
1.展示了如何获取图片并保持到磁盘里。
2.如何录制一段时间的视频，并保存成*.avi格式。
```


主要的函数有以下：
--------------
```
imageBuffer* getImageBuf(int imageNo=10)
获取一定数量jpeg格式的祯数据，并通过指针返回

void saveJpg(imageBuffer temp[],int imageNo)
将指针imageBuffer所保存的数据保存成jpeg格式，默认的存储位置为：“./temp/”

void creatVideo(int time,int fps=23); 
录制一段时间的视频（单位:秒,默认帧率为23fps）
```

