#include "Jpeg2AVI.h"
#include "list.h"
#include <stdlib.h>
#include <string.h>

static int nframes;           //��֡��
static int totalsize;         //֡���ܴ�С
static struct list_head list; //�����֡ͼ���С����������д������

/*���������ṹ�����ڱ���������ͼ���С����*/
struct ListNode
{
    int value;
    struct list_head head;
};

static void write_index_chunk(FILE *fp)
{
    unsigned char index[4] = {'i', 'd', 'x', '1'};  //������ID
    unsigned int index_chunk_size = 16 * nframes;   //�������С
    unsigned int offset = 4;                        
    struct list_head *slider = NULL;
    struct list_head *tmpslider = NULL;

    fwrite(index, 4, 1, fp);
    fwrite(&index_chunk_size, 4, 1, fp);

    list_for_each_safe(slider, tmpslider, &list)
    {
        unsigned char tmp[4] = {'0', '0', 'd', 'c'};  //00dc = ѹ������Ƶ����
        unsigned int keyframe = 0x10;                 //0x10��ʾ��ǰ֡Ϊ�ؼ�֡
        struct ListNode *node = list_entry(slider, struct ListNode, head);

        fwrite(tmp, 4, 1, fp);
        fwrite(&keyframe, 4, 1, fp);    
        fwrite(&offset, 4, 1, fp);        
        fwrite(&node->value, 4, 1, fp);
        offset = offset + node->value + 8;

        list_del(slider);
        free(node);
    }
}

static void back_fill_data(FILE *fp, int width, int height, int fps)
{
    AVI_RIFF_HEAD riff_head = 
    {
        {'R', 'I', 'F', 'F'},     
        4 + sizeof(AVI_HDRL_LIST) + sizeof(AVI_LIST_HEAD) + nframes * 8 + totalsize,  
        {'A', 'V', 'I', ' '}
    };

    AVI_HDRL_LIST hdrl_list = 
    {
        {'L', 'I', 'S', 'T'},
        sizeof(AVI_HDRL_LIST) - 8,
        {'h', 'd', 'r', 'l'},
        {
            {'a', 'v', 'i', 'h'},
            sizeof(AVI_AVIH_CHUNK) - 8,       
            1000 / fps,			//��Ƶ֡���ʱ��(��΢��Ϊ��λ)
			25000, 0, 0, nframes, 0, 1, 100000, width, height, 
            {0, 0, 0, 0}
        },
        {
            {'L', 'I', 'S', 'T'},
            sizeof(AVI_STRL_LIST) - 8,
            {'s', 't', 'r', 'l'},
            {
                {'s', 't', 'r', 'h'},
                sizeof(AVI_STRH_CHUNK) - 8,
                {'v', 'i', 'd', 's'},
                {'J', 'P', 'E', 'G'},
                0, 0, 0, 0, 1, fps, 0, nframes, 100000, 3000, 0,
                {0, 0, width, height}
            },
            {
                {'s', 't', 'r', 'f'},
                sizeof(AVI_STRF_CHUNK) - 8,
                sizeof(AVI_STRF_CHUNK) - 8,
                width, height, 1, 24,
                {'J', 'P', 'E', 'G'},
                width * height * 3, 0, 0, 0, 0
            }
        }
    };

    AVI_LIST_HEAD movi_list_head = 
    {
        {'L', 'I', 'S', 'T'},     
        4 + nframes * 8 + totalsize,           
        {'m', 'o', 'v', 'i'}    
    };

    //��λ���ļ�ͷ�������������
    fseek(fp, 0, SEEK_SET);
    fwrite(&riff_head, sizeof(riff_head), 1, fp);
    fwrite(&hdrl_list, sizeof(hdrl_list), 1, fp);
    fwrite(&movi_list_head, sizeof(movi_list_head), 1, fp);
}

void jpeg2avi_start(FILE *fp)
{
    int offset1 = sizeof(AVI_RIFF_HEAD);  //riff head��С
    int offset2 = sizeof(AVI_HDRL_LIST);  //hdrl list��С 
    int offset3 = sizeof(AVI_LIST_HEAD);  //movi list head��С

    //AVI�ļ�ƫ�������õ�movi list head�󣬴Ӹ�λ���������д��JPEG����
    fseek(fp, offset1 + offset2 + offset3, SEEK_SET); 

    //��ʼ������
    list_head_init(&list);

    nframes = 0;
    totalsize = 0;
}

void jpeg2avi_add_frame(FILE *fp, void *data, unsigned int len)
{
    unsigned char tmp[4] = {'0', '0', 'd', 'c'};  //00dc = ѹ������Ƶ����
    struct ListNode *node = (struct ListNode *)malloc(sizeof(struct ListNode));

    /*JPEGͼ���С4�ֽڶ���*/
    while (len % 4)
    {
        len++;
    }

    fwrite(tmp, 4, 1, fp);    //д���Ƿ���ѹ������Ƶ������Ϣ    
    fwrite(&len, 4, 1, fp);   //д��4�ֽڶ�����JPEGͼ���С
    fwrite(data, len, 1, fp); //д��������JPEG����

    nframes += 1;
    totalsize += len;

    /*��4�ֽڶ�����JPEGͼ���С������������*/
    if (node != NULL)
    {
        node->value = len;
        list_add_tail(&node->head, &list);
    }
}

void jpeg2avi_end(FILE *fp, int width, int height, int fps)
{ 
    //д������
    write_index_chunk(fp);

    //���ļ�ͷ��ʼ�������������
    back_fill_data(fp, width, height, fps);
}
