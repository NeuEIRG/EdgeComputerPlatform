#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>//将客户端信息转化为字符串
#include <netdb.h>
#define MAXLINE 4096

int main(){
    int   sockfd, len;
    char  buffer[MAXLINE];
    FILE *fq;
    while(1){
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
	printf("please input filename\n");
    	char DataName[200];
	int name_ex=1;
sprintf(DataName,"%s%d%s","./temp/",name_ex++,".jpg");
	//gets(DataName);
	//if(!strcmp(DataName,"end")){
		//close(sockfd);
		//break;	
	//}
    	// if( ( fq = fopen("/home/sjw/Desktop/1.jpg","rb") ) == NULL ){
    	if( ( fq = fopen(DataName,"rb") ) == NULL ){
        	printf("File open.\n");
        	close(sockfd);
        	exit(1);
    	}
    	bzero(buffer,sizeof(buffer));
    	while(!feof(fq)){
        	len = fread(buffer, 1, sizeof(buffer), fq);
        	if(len != write(sockfd, buffer, len)){
            		printf("write.\n");
            		break;
        	}
    	}
    	// }
    	close(sockfd);
    	fclose(fq);
    } 
}
