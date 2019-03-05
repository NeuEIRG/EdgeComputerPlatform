#ifndef _JPEG2AVI_H_
#define _JPEG2AVI_H_

#include <stdio.h>

void jpeg2avi_start(FILE *fp);
void jpeg2avi_add_frame(FILE *fp, void *data, unsigned int len);
void jpeg2avi_end(FILE *fp, int width, int height, int fps);

typedef struct avi_riff_head
{
    unsigned char id[4];        
    unsigned int size;           
    unsigned char type[4];   
}AVI_RIFF_HEAD, AVI_LIST_HEAD;

typedef struct avi_avih_chunk
{
    unsigned char id[4];            //块ID，固定为avih
    unsigned int size;              //块大小，等于struct avi_avih_chunk去掉id和size的大小
    unsigned int us_per_frame;      //视频帧间隔时间(以微秒为单位)
    unsigned int max_bytes_per_sec; //AVI文件的最大数据率
    unsigned int padding;           //设为0即可
    unsigned int flags;             //AVI文件全局属性，如是否含有索引块、音视频数据是否交叉存储等
    unsigned int total_frames;      //总帧数
    unsigned int init_frames;       //为交互格式指定初始帧数(非交互格式应该指定为0)
    unsigned int streams;           //文件包含的流的个数，仅有视频流时为1
    unsigned int suggest_buff_size; //指定读取本文件建议使用的缓冲区大小，通常为存储一桢图像                                            //以及同步声音所需的数据之和，不指定时设为0
    unsigned int width;             //视频主窗口宽度（单位：像素）
    unsigned int height;            //视频主窗口高度（单位：像素）
    unsigned int reserved[4];       //保留段，设为0即可
}AVI_AVIH_CHUNK;

typedef struct avi_rect_frame
{
    short left;
    short top;
    short right;
    short bottom;    
}AVI_RECT_FRAME;

typedef struct avi_strh_chunk
{    
    unsigned char id[4];            //块ID，固定为strh
    unsigned int size;              //块大小，等于struct avi_strh_chunk去掉id和size的大小
    unsigned char stream_type[4];   //流的类型，vids表示视频流，auds表示音频流
    unsigned char codec[4];         //指定处理这个流需要的解码器，如JPEG
    unsigned int flags;             //标记，如是否允许这个流输出、调色板是否变化等，一般设为0即可
    unsigned short priority;        //流的优先级，视频流设为0即可
    unsigned short language;        //音频语言代号，视频流设为0即可
    unsigned int init_frames;       //为交互格式指定初始帧数(非交互格式应该指定为0)
    unsigned int scale;             //
    unsigned int rate;              //对于视频流，rate / scale = 帧率fps
    unsigned int start;             //对于视频流，设为0即可
    unsigned int length;            //对于视频流，length即总帧数
    unsigned int suggest_buff_size; //读取这个流数据建议使用的缓冲区大小
    unsigned int quality;           //流数据的质量指标
    unsigned int sample_size;       //音频采样大小，视频流设为0即可
    AVI_RECT_FRAME rcFrame;         //这个流在视频主窗口中的显示位置，设为{0,0，width,height}即可
}AVI_STRH_CHUNK;

/*对于视频流，strf块结构如下*/
typedef struct avi_strf_chunk
{
    unsigned char id[4];             //块ID，固定为strf
    unsigned int size;               //块大小，等于struct avi_strf_chunk去掉id和size的大小
    unsigned int size1;              //size1含义和值同size一样
    unsigned int width;              //视频主窗口宽度（单位：像素）
    unsigned int height;             //视频主窗口高度（单位：像素）
    unsigned short planes;           //始终为1  
    unsigned short bitcount;         //每个像素占的位数，只能是1、4、8、16、24和32中的一个
    unsigned char compression[4];    //视频流编码格式，如JPEG、MJPG等
    unsigned int image_size;         //视频图像大小，等于width * height * bitcount / 8
    unsigned int x_pixels_per_meter; //显示设备的水平分辨率，设为0即可
    unsigned int y_pixels_per_meter; //显示设备的垂直分辨率，设为0即可
    unsigned int num_colors;         //含义不清楚，设为0即可   
    unsigned int imp_colors;         //含义不清楚，设为0即可
}AVI_STRF_CHUNK;

typedef struct avi_strl_list
{
    unsigned char id[4];    //块ID，固定为LIST    
    unsigned int size;      //块大小，等于struct avi_strl_list去掉id和size的大小        
    unsigned char type[4];  //块类型，固定为strl
    AVI_STRH_CHUNK strh;      
    AVI_STRF_CHUNK strf;      
}AVI_STRL_LIST;

typedef struct avi_hdrl_list
{
    unsigned char id[4];    //块ID，固定为LIST    
    unsigned int size;      //块大小，等于struct avi_hdrl_list去掉id和size的大小        
    unsigned char type[4];  //块类型，固定为hdrl
    AVI_AVIH_CHUNK avih;
    AVI_STRL_LIST  strl;
}AVI_HDRL_LIST;

#endif