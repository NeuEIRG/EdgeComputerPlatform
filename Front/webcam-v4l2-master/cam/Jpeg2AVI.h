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
    unsigned char id[4];            //��ID���̶�Ϊavih
    unsigned int size;              //���С������struct avi_avih_chunkȥ��id��size�Ĵ�С
    unsigned int us_per_frame;      //��Ƶ֡���ʱ��(��΢��Ϊ��λ)
    unsigned int max_bytes_per_sec; //AVI�ļ������������
    unsigned int padding;           //��Ϊ0����
    unsigned int flags;             //AVI�ļ�ȫ�����ԣ����Ƿ��������顢����Ƶ�����Ƿ񽻲�洢��
    unsigned int total_frames;      //��֡��
    unsigned int init_frames;       //Ϊ������ʽָ����ʼ֡��(�ǽ�����ʽӦ��ָ��Ϊ0)
    unsigned int streams;           //�ļ����������ĸ�����������Ƶ��ʱΪ1
    unsigned int suggest_buff_size; //ָ����ȡ���ļ�����ʹ�õĻ�������С��ͨ��Ϊ�洢һ��ͼ��                                            //�Լ�ͬ���������������֮�ͣ���ָ��ʱ��Ϊ0
    unsigned int width;             //��Ƶ�����ڿ�ȣ���λ�����أ�
    unsigned int height;            //��Ƶ�����ڸ߶ȣ���λ�����أ�
    unsigned int reserved[4];       //�����Σ���Ϊ0����
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
    unsigned char id[4];            //��ID���̶�Ϊstrh
    unsigned int size;              //���С������struct avi_strh_chunkȥ��id��size�Ĵ�С
    unsigned char stream_type[4];   //�������ͣ�vids��ʾ��Ƶ����auds��ʾ��Ƶ��
    unsigned char codec[4];         //ָ�������������Ҫ�Ľ���������JPEG
    unsigned int flags;             //��ǣ����Ƿ�����������������ɫ���Ƿ�仯�ȣ�һ����Ϊ0����
    unsigned short priority;        //�������ȼ�����Ƶ����Ϊ0����
    unsigned short language;        //��Ƶ���Դ��ţ���Ƶ����Ϊ0����
    unsigned int init_frames;       //Ϊ������ʽָ����ʼ֡��(�ǽ�����ʽӦ��ָ��Ϊ0)
    unsigned int scale;             //
    unsigned int rate;              //������Ƶ����rate / scale = ֡��fps
    unsigned int start;             //������Ƶ������Ϊ0����
    unsigned int length;            //������Ƶ����length����֡��
    unsigned int suggest_buff_size; //��ȡ��������ݽ���ʹ�õĻ�������С
    unsigned int quality;           //�����ݵ�����ָ��
    unsigned int sample_size;       //��Ƶ������С����Ƶ����Ϊ0����
    AVI_RECT_FRAME rcFrame;         //���������Ƶ�������е���ʾλ�ã���Ϊ{0,0��width,height}����
}AVI_STRH_CHUNK;

/*������Ƶ����strf��ṹ����*/
typedef struct avi_strf_chunk
{
    unsigned char id[4];             //��ID���̶�Ϊstrf
    unsigned int size;               //���С������struct avi_strf_chunkȥ��id��size�Ĵ�С
    unsigned int size1;              //size1�����ֵͬsizeһ��
    unsigned int width;              //��Ƶ�����ڿ�ȣ���λ�����أ�
    unsigned int height;             //��Ƶ�����ڸ߶ȣ���λ�����أ�
    unsigned short planes;           //ʼ��Ϊ1  
    unsigned short bitcount;         //ÿ������ռ��λ����ֻ����1��4��8��16��24��32�е�һ��
    unsigned char compression[4];    //��Ƶ�������ʽ����JPEG��MJPG��
    unsigned int image_size;         //��Ƶͼ���С������width * height * bitcount / 8
    unsigned int x_pixels_per_meter; //��ʾ�豸��ˮƽ�ֱ��ʣ���Ϊ0����
    unsigned int y_pixels_per_meter; //��ʾ�豸�Ĵ�ֱ�ֱ��ʣ���Ϊ0����
    unsigned int num_colors;         //���岻�������Ϊ0����   
    unsigned int imp_colors;         //���岻�������Ϊ0����
}AVI_STRF_CHUNK;

typedef struct avi_strl_list
{
    unsigned char id[4];    //��ID���̶�ΪLIST    
    unsigned int size;      //���С������struct avi_strl_listȥ��id��size�Ĵ�С        
    unsigned char type[4];  //�����ͣ��̶�Ϊstrl
    AVI_STRH_CHUNK strh;      
    AVI_STRF_CHUNK strf;      
}AVI_STRL_LIST;

typedef struct avi_hdrl_list
{
    unsigned char id[4];    //��ID���̶�ΪLIST    
    unsigned int size;      //���С������struct avi_hdrl_listȥ��id��size�Ĵ�С        
    unsigned char type[4];  //�����ͣ��̶�Ϊhdrl
    AVI_AVIH_CHUNK avih;
    AVI_STRL_LIST  strl;
}AVI_HDRL_LIST;

#endif