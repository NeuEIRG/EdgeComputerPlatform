#include <iostream>
#include "webcam.h"

#define JPEG_MAX_SIZE 1000000   //JPEG图像最大字节数

int main ()
{
    int imageNo;
    imageBuffer* p=getImageBuf(imageNo);
    saveJpg(p, imageNo);
    creatVideo(3);
    return 1;

}
