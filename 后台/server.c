#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>


#define MAXLINE 4096

int main(int argc, char** argv){
    int  listenfd, connfd;
    struct sockaddr_in  servaddr;
    char  buff[4096];
    FILE *fp;
    int  n;

    if( (listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1 ){
        printf("create socket error: %s(errno: %d)\n",strerror(errno),errno);
        return 0;
    }
    printf("----init socket----\n");

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(6666);
    //设置端口可重用
    int contain;
    setsockopt(listenfd,SOL_SOCKET, SO_REUSEADDR, &contain, sizeof(int));

    if( bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1){
        printf("bind socket error: %s(errno: %d)\n",strerror(errno),errno);
        return 0;
    }
    printf("----bind sucess----\n");

    if( listen(listenfd, 10) == -1){
        printf("listen socket error: %s(errno: %d)\n",strerror(errno),errno);
        return 0;
    }
    //if((fp = fopen("new1.jpg","ab") ) == NULL )
    int i=1;
  
    
    while(1){
    char DataName[1000]="/home/wz/Desktop/image/";
    char data1[500]=" ";
    char data_jpg[40]=".jpg";
    sprintf(data1,"%05X",i++);
    
    strcat(data1,data_jpg);
    printf("%s",data1);
    strcat(DataName,data1);
    //gets(DataName);
    //    if(!strcmp(DataName,"end")){
    //       break;
    //    }

    if((fp = fopen(DataName,"ab") ) == NULL )
    {
        printf("File.\n");
        close(listenfd);
        exit(1);
    }

    printf("======waiting for client's request======\n");
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
        buff[n] = '\0';
        printf("recv msg from client: %s\n", buff);
        close(connfd);
        fclose(fp);
    }
    close(listenfd);
    return 0;
}
