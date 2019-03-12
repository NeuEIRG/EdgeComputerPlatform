/*

   (c) 2014 Séverin Lemaignan <severin.lemaignan@epfl.ch>
   (c) 2008 Hans de Goede <hdegoede@redhat.com> for yuyv_to_rgb24

 This program is free software; you can redistribute it and/or modify it
 under the terms of the GNU Lesser General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or (at
 your option) any later version.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 License for more details.

 You should have received a copy of the GNU Lesser General Public License
 along with this program; if not, write to the Free Software Foundation,
 Inc., 51 Franklin Street, Suite 500, Boston, MA  02110-1335  USA

 */

#include <stdlib.h>
#include <assert.h>
#include <fcntl.h>              /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <string.h> // strerrno
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <stdexcept>

#include <linux/videodev2.h>

#include "webcam.h"

#include <stdio.h>
#include <errno.h>


#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>


#include <arpa/inet.h>//将客户端信息转化为字符串
#include <netdb.h>

#include "Jpeg2AVI.h"


#define MAXLINE 4096

#define CLEAR(x) memset(&(x), 0, sizeof(x))

using namespace std;
using imageBuffer=std::pair<unsigned char*,unsigned long>;
imageBuffer mRes;
imageBuffer&
CompressJPEG(
          unsigned int uWidth, 
          unsigned int uHeight,
          unsigned char* pImg, 
          int iQuality  )   
{
//  std::pair<unsigned char*,unsigned long> mRes  = std::make_pair( (unsigned char*)NULL, 0 );
  
  // setup JPEG compression structure data
  jpeg_compress_struct  jcInfo;
  jpeg_error_mgr      jErr;  // JPEG error handler
  jcInfo.err        = jpeg_std_error ( &jErr );
  
  // initialize JPEG compression object
  jpeg_create_compress( &jcInfo );
  
  // specify data destination is memory
  jpeg_mem_dest( &jcInfo, &mRes.first, &mRes.second );
  
  // image format
  jcInfo.image_width    = uWidth;
  jcInfo.image_height    = uHeight;
  jcInfo.input_components  = 3;
  jcInfo.in_color_space  = JCS_RGB;
  
  // set default compression parameters
  jpeg_set_defaults( &jcInfo );
   
  // set image quality
  jpeg_set_quality( &jcInfo, iQuality, TRUE );
  
  // start compressor
  jpeg_start_compress( &jcInfo, TRUE );
  int  iRowStride = jcInfo.image_width * jcInfo.input_components;
  while( jcInfo.next_scanline < jcInfo.image_height )  {
    JSAMPROW pData = &( pImg[ jcInfo.next_scanline * iRowStride ] );
    jpeg_write_scanlines( &jcInfo, &pData, 1 );
  }
  
  // finish compression
  jpeg_finish_compress( &jcInfo );
  
  // release JPEG compression object
  jpeg_destroy_compress( &jcInfo );
  
  return mRes;
}

imageBuffer* getImageBuf(int imageNo)
{
	Webcam webcam("/dev/video0", XRES, YRES);
    webcam.init();
    Rgbimage& frame = webcam.frame();           //   get a unCompressed JPEG image  
    
 // int imageNo = 10;         // set buffer size
    imageBuffer  *temp;    //image buffer
    temp=new imageBuffer[imageNo];
    
    imageBuffer&  Frame=CompressJPEG( XRES, YRES,frame.data,100 ); // get a Compressed JPEG image
    

    for(int i=0;i<imageNo;i++)
    {
    frame = webcam.frame();   
    Frame=CompressJPEG( XRES, YRES,frame.data,100 );
    jpg_copy(&temp[i],&Frame);    //cope jpeg to the buffer
	}
  return temp;

}

void creatVideo(int time,int fps)
{

    Webcam webcam("/dev/video0", XRES, YRES);
    webcam.init();
    FILE *fp_avi;
    fp_avi = fopen("sample.avi","wb");
    jpeg2avi_start(fp_avi);


    for(int i=0;i<=time*fps;i++)
    {
    Rgbimage& frame = webcam.frame();           //   get a unCompressed JPEG image
    usleep(1000000.0/fps);              //每1/fps秒取一张图一帧
    imageBuffer&  Frame=CompressJPEG( XRES, YRES,frame.data,100 ); // get a Compressed JPEG image
    jpeg2avi_add_frame(fp_avi, Frame.first, Frame.second);

    }

    jpeg2avi_end(fp_avi, 640, 480, fps);
    fclose(fp_avi);
    
}


void sendimage(imageBuffer temp){
    int   sockfd, len;
  //  char  buffer[MAXLINE];
    FILE *fq;
    //while(1){
    	sockfd=socket(AF_INET,SOCK_STREAM,0);  //initialize sockfd 
    	struct sockaddr_in  servaddr;
    	memset(&servaddr,0,sizeof(servaddr));
    	servaddr.sin_family=AF_INET;
    	servaddr.sin_port=htons(6666);//6675
    	servaddr.sin_addr.s_addr=inet_addr("192.168.43.88");//127.0.0.1 local?
    	memset(&(servaddr.sin_zero),0,8);  //clean server addr 
    	int res =connect(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr)); //connect server_addr
        	printf("connect res is %d\n",res); 
    	// while(1){
	//printf("please input filename\n");
    //char DataName[200];
	//int name_ex=1;
    //sprintf(DataName,"%s%d%s","./temp/",name_ex++,".jpg");
	//gets(DataName);
	//if(!strcmp(DataName,"end")){
		//close(sockfd);
		//break;	
	//}
    	// if( ( fq = fopen("/home/sjw/Desktop/1.jpg","rb") ) == NULL ){
    	// if( ( fq = fopen(DataName,"rb") ) == NULL ){
        // 	printf("File open.\n");
        // 	close(sockfd);
        // 	exit(1);
    	// }
    	//bzero((char *)temp.first,temp.second);
    	//while(!feof(fq)){
        	//len = fread((char *)temp.first, 1, temp.second, fq);
        	// if(len != write(sockfd, (char *)temp.first, len)){
            // 		printf("write.\n");
            // 		break;
        	// }
    	//}
    	// }
        write(sockfd, (char *)temp.first, temp.second);
    	close(sockfd);
    	//fclose(fq);
    //} 


 }

void saveJpg(imageBuffer temp[],int imageNo)
{
    // output the image,the dir is ./temp/  and you must premake it
    char name[40];
    int name_ex=1;
    ofstream image;
    for(int i=0;i<imageNo;i++)
    {
    //cout<<i<<endl;
    sprintf(name,"%s%d%s","./temp/",name_ex++,".jpg");
    image.open(string(name));
    image.write((char *) temp[i].first, temp[i].second);
    image.close();
    //cout<<temp[i].second<<endl;
    sendimage(temp[i]);
    }
   
}


static int xioctl(int fh, unsigned long int request, void *arg)
{
      int r;

      do {
            r = ioctl(fh, request, arg);
      } while (-1 == r && EINTR == errno);

      return r;
}

void rgb_copy(Rgbimage* _new,Rgbimage * old)
{
    _new->height=old->height;
    _new->size=old->size;
    _new->width=old->width;
    _new->data=(unsigned char *) malloc(old->size * sizeof(char));

    unsigned char   *temp;
    temp = old->data;               // memory skill
    old->data = _new->data;
    _new->data = temp;


}

void jpg_copy(std::pair<unsigned char*,unsigned long>* _new, std::pair<unsigned char*,unsigned long> * old)
{
    _new->second=old->second;
    _new->first=(unsigned char *) malloc(old->second * sizeof(char));

    unsigned char   *temp;
    temp = old->first;               // memory skill
    old->first = _new->first;
    _new->first = temp;

   
}

/*****
 * Taken from libv4l2 (in v4l-utils)
 *
 * (C) 2008 Hans de Goede <hdegoede@redhat.com>
 *
 * Released under LGPL
 */
#define CLIP(color) (unsigned char)(((color) > 0xFF) ? 0xff : (((color) < 0) ? 0 : (color)))

static void v4lconvert_yuyv_to_rgb24(const unsigned char *src, 
                                     unsigned char *dest,
                                     int width, int height, 
                                     int stride)
{
    int j;

    while (--height >= 0) {
        for (j = 0; j + 1 < width; j += 2) {
            int u = src[1];
            int v = src[3];
            int u1 = (((u - 128) << 7) +  (u - 128)) >> 6;
            int rg = (((u - 128) << 1) +  (u - 128) +
                    ((v - 128) << 2) + ((v - 128) << 1)) >> 3;
            int v1 = (((v - 128) << 1) +  (v - 128)) >> 1;

            *dest++ = CLIP(src[0] + v1);
            *dest++ = CLIP(src[0] - rg);
            *dest++ = CLIP(src[0] + u1);

            *dest++ = CLIP(src[2] + v1);
            *dest++ = CLIP(src[2] - rg);
            *dest++ = CLIP(src[2] + u1);
            src += 4;
        }
        src += stride - (width * 2);
    }
}
/*******************************************************************/


Webcam::Webcam(const string& device, int width, int height) : 
                        device(device),
                        xres(width),
                        yres(height)
{
    
    // xres and yres are set to the actual resolution provided by the cam

    // frame stored as RGB888 (ie, RGB24)
    rgb_frame.width = xres;
    rgb_frame.height = yres;
    rgb_frame.size = xres * yres * 3;
    rgb_frame.data = (unsigned char *) malloc(rgb_frame.size * sizeof(char));

}




Webcam::~Webcam()
{
      stop_capturing();
      uninit_device();
      close_device();

      free(rgb_frame.data);
}

RGBImage& Webcam::frame(int timeout)
{
    for (;;) {
        fd_set fds;
        struct timeval tv;
        int r;

        FD_ZERO(&fds);
        FD_SET(fd, &fds);

        /* Timeout. */
        tv.tv_sec = timeout;
        tv.tv_usec = 0;

        r = select(fd + 1, &fds, NULL, NULL, &tv);

        if (-1 == r) {
            if (EINTR == errno)
                continue;
            throw runtime_error("select");
        }

        if (0 == r) {
            throw runtime_error(device + ": select timeout");
        }
        if (read_frame()) {
            return rgb_frame;
        }
        /* EAGAIN - continue select loop. */
    }

}

bool Webcam::read_frame()
{

    struct v4l2_buffer buf;
    unsigned int i;

    CLEAR(buf);

    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;

    if (-1 == xioctl(fd, VIDIOC_DQBUF, &buf)) {
        switch (errno) {
            case EAGAIN:
                return false;

            case EIO:
                /* Could ignore EIO, see spec. */

                /* fall through */

            default:
                throw runtime_error("VIDIOC_DQBUF");
        }
    }

    assert(buf.index < n_buffers);

    v4lconvert_yuyv_to_rgb24((unsigned char *) buffers[buf.index].data,
                             rgb_frame.data,
                             xres,
                             yres,
                             stride);

    if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
        throw runtime_error("VIDIOC_QBUF");

    return true;
}

void Webcam::open_device(void)
{
      struct stat st;

      if (-1 == stat(device.c_str(), &st)) {
            throw runtime_error(device + ": cannot identify! " + to_string(errno) +  ": " + strerror(errno));
      }

      if (!S_ISCHR(st.st_mode)) {
            throw runtime_error(device + " is no device");
      }

      fd = open(device.c_str(), O_RDWR /* required */ | O_NONBLOCK, 0);

      if (-1 == fd) {
            throw runtime_error(device + ": cannot open! " + to_string(errno) + ": " + strerror(errno));
      }
}


void Webcam::init_mmap(void)
{
      struct v4l2_requestbuffers req;

      CLEAR(req);

      req.count = 4;
      req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      req.memory = V4L2_MEMORY_MMAP;

      if (-1 == xioctl(fd, VIDIOC_REQBUFS, &req)) {
            if (EINVAL == errno) {
                  throw runtime_error(device + " does not support memory mapping");
            } else {
                  throw runtime_error("VIDIOC_REQBUFS");
            }
      }

      if (req.count < 2) {
            throw runtime_error(string("Insufficient buffer memory on ") + device);
      }

      buffers = (buffer*) calloc(req.count, sizeof(*buffers));

      if (!buffers) {
            throw runtime_error("Out of memory");
      }

      for (n_buffers = 0; n_buffers < req.count; ++n_buffers) {
            struct v4l2_buffer buf;

            CLEAR(buf);

            buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory      = V4L2_MEMORY_MMAP;
            buf.index       = n_buffers;

            if (-1 == xioctl(fd, VIDIOC_QUERYBUF, &buf))
                  throw runtime_error("VIDIOC_QUERYBUF");

            buffers[n_buffers].size = buf.length;
            buffers[n_buffers].data =
                  mmap(NULL /* start anywhere */,
                        buf.length,
                        PROT_READ | PROT_WRITE /* required */,
                        MAP_SHARED /* recommended */,
                        fd, buf.m.offset);

            if (MAP_FAILED == buffers[n_buffers].data)
                  throw runtime_error("mmap");
      }
}

void Webcam::close_device(void)
{
      if (-1 == close(fd))
            throw runtime_error("close");

      fd = -1;
}

void Webcam::init_device(void)
{
    struct v4l2_capability cap;
    struct v4l2_cropcap cropcap;
    struct v4l2_crop crop;
    struct v4l2_format fmt;
    unsigned int min;

    if (-1 == xioctl(fd, VIDIOC_QUERYCAP, &cap)) {
        if (EINVAL == errno) {
            throw runtime_error(device + " is no V4L2 device");
        } else {
            throw runtime_error("VIDIOC_QUERYCAP");
        }
    }

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
        throw runtime_error(device + " is no video capture device");
    }

    if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
        throw runtime_error(device + " does not support streaming i/o");
    }

    /* Select video input, video standard and tune here. */


    CLEAR(cropcap);

    cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (0 == xioctl(fd, VIDIOC_CROPCAP, &cropcap)) {
        crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        crop.c = cropcap.defrect; /* reset to default */

        if (-1 == xioctl(fd, VIDIOC_S_CROP, &crop)) {
            switch (errno) {
                case EINVAL:
                    /* Cropping not supported. */
                    break;
                default:
                    /* Errors ignored. */
                    break;
            }
        }
    } else {
        /* Errors ignored. */
    }


    CLEAR(fmt);

    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (force_format) {
        fmt.fmt.pix.width       = xres;
        fmt.fmt.pix.height      = yres;
        fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
        fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;

        if (-1 == xioctl(fd, VIDIOC_S_FMT, &fmt))
            throw runtime_error("VIDIOC_S_FMT");

        if (fmt.fmt.pix.pixelformat != V4L2_PIX_FMT_YUYV)
            // note that libv4l2 (look for 'v4l-utils') provides helpers
            // to manage conversions
            throw runtime_error("Webcam does not support YUYV format. Support for more format need to be added!");

        /* Note VIDIOC_S_FMT may change width and height. */
        xres = fmt.fmt.pix.width;
        yres = fmt.fmt.pix.height;

        stride = fmt.fmt.pix.bytesperline;


    } else {
        /* Preserve original settings as set by v4l2-ctl for example */
        if (-1 == xioctl(fd, VIDIOC_G_FMT, &fmt))
            throw runtime_error("VIDIOC_G_FMT");
    }

    init_mmap();
}


void Webcam::uninit_device(void)
{
    unsigned int i;

    for (i = 0; i < n_buffers; ++i)
        if (-1 == munmap(buffers[i].data, buffers[i].size))
            throw runtime_error("munmap");

    free(buffers);
}

void Webcam::start_capturing(void)
{
    unsigned int i;
    enum v4l2_buf_type type;

    for (i = 0; i < n_buffers; ++i) {
        struct v4l2_buffer buf;

        CLEAR(buf);
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;

        if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
            throw runtime_error("VIDIOC_QBUF");
    }
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (-1 == xioctl(fd, VIDIOC_STREAMON, &type))
        throw runtime_error("VIDIOC_STREAMON");
}

void Webcam::stop_capturing(void)
{
    enum v4l2_buf_type type;

    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (-1 == xioctl(fd, VIDIOC_STREAMOFF, &type))
        throw runtime_error("VIDIOC_STREAMOFF");
}

void Webcam::init()
{
    open_device();
    init_device();
    start_capturing();
}

