Minimalistic C++ wrapper around V4L2
====================================

Build the demo
--------------

You need the V4L2 and libjpeg development package. On Debian/Ubuntu:
```
As for V4l2,you can install the libv4l2-dev like that
$ sudo apt install libv4l2-dev 

For libjpeg, you can download it from http://www.ijg.org/
```

Then:
```
$ g++ -std=c++11 main.cpp webcam.cpp Jpeg2AVI.cpp list.cpp -omain -lv4l2 -ljpeg
```
主要的功能有以下：
imageBuffer* getImageBuf(int imageNo=10);  
获取一定数量jpeg格式的祯数据，并通过指针返回；

void saveJpg(imageBuffer temp[],int imageNo);
将指针imageBuffer所保存的数据保存成jpeg格式，默认的存储位置为：“./temp/”

void creatVideo(int time,int fps=23); 
录制一段时间的视频（单位:秒,默认帧率为23fps）


