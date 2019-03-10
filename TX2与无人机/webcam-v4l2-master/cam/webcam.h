/** Small C++ wrapper around V4L example code to access the webcam
**/

#ifndef _WEBCAM_H_
#define _WEBCAM_H_


#include <string>
#include <memory> // unique_ptr
#include <iostream>
#include <fstream>
#include<string>
#include <errno.h>

#include <stdio.h>
#include <unistd.h>

#include <jpeglib.h>
#include <setjmp.h>
#define XRES 640
#define YRES 480



using imageBuffer=std::pair<unsigned char*,unsigned long>;

using namespace std;
imageBuffer& 
CompressJPEG(
          unsigned int uWidth, 
          unsigned int uHeight,
          unsigned char* pImg, 
          int iQuality = 75 ); 

imageBuffer* getImageBuf(int imageNo=10);            
void saveJpg(imageBuffer temp[],int imageNo);
void creatVideo(int time,int fps=23);       //帧率默认为23

struct buffer {
      void   *data;
      size_t  size;
};

typedef struct RGBImage {
      unsigned char   *data; // RGB888 <=> RGB24
      size_t          width;
      size_t          height;
      size_t          size; // width * height * 3
}Rgbimage;

void rgb_copy(Rgbimage * _new,Rgbimage*  old);
void jpg_copy(std::pair<unsigned char*,unsigned long>* _new,std::pair<unsigned char*,unsigned long> * old);

class Webcam {

public:
    Webcam(const std::string& device = "/dev/video0", 
           int width = 640, 
           int height = 480);

    ~Webcam();

    /** Captures and returns a frame from the webcam.
     *
     * The returned object contains a field 'data' with the image data in RGB888
     * format (ie, RGB24), as well as 'width', 'height' and 'size' (equal to
     * width * height * 3)
     *
     * This call blocks until a frame is available or until the provided
     * timeout (in seconds). 
     *
     * Throws a runtime_error if the timeout is reached.
     */
     RGBImage& frame(int timeout = 1);


    void init_mmap();

    void open_device();
    void close_device();

    void init_device();
    void uninit_device();

    void start_capturing();
    void stop_capturing();

    bool read_frame();

    void init();

    std::string device;
    int fd;

    RGBImage rgb_frame;
    struct buffer          *buffers;
    unsigned int     n_buffers;

    size_t xres, yres;
    size_t stride;

    bool force_format = true;
};


#endif

