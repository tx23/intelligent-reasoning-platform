#include "./include/cndev.h"
#include "stdio.h"
#include<netinet/in.h>
#include<unistd.h>   //for fork and read
#include<sys/types.h>   //for socket
#include<sys/socket.h>  //for socket
#include<string.h> // for bzero
#include<arpa/inet.h>
#include<pthread.h>

void *readPower(void* arg) {
    int client_fd = *(int*)arg;


    //初始化libcndev
    cndevRet_t ret = cndevInit(CNDEV_VERSION_5);
    cndevCardInfo_t cardInfo;
    cndevPowerInfo_t powerInfo;
    cardInfo.version = CNDEV_VERSION_5;
    powerInfo.version = CNDEV_VERSION_5;
    ret = cndevGetDeviceCount(&cardInfo);
    printf("板卡数量是%d\n", cardInfo.Number);
    int32_t totalPower = 0;
    for (int i = 0; i < cardInfo.Number; i++)
    {
        cndevCheckErrors(cndevGetPowerInfo(&powerInfo, i));
        totalPower += powerInfo.Usage;
        printf("power:Usage:%dW, cap:%dW\n", powerInfo.Usage, powerInfo.Cap);
    }

    //发送功耗
    int sendLen = sizeof(totalPower);
    char *sendData = (char*)malloc(sizeof(char) * sendLen);
    memset(sendData, 0, sendLen);
    *(int32_t*)sendData = (int32_t)totalPower;
    if( send(client_fd, sendData, sendLen, 0) == -1)
        printf("send error!\n");
    close(client_fd);

    //释放libcndev
    cndevRelease();
}

int main() {
    //创建socket连接
    const unsigned short SERVERPORT = 9089;
    const int BACKLOG = 5; //5 个最大的连接数
    
    int sock, client_fd;
    sockaddr_in myAddr;
    sockaddr_in remoteAddr;
    sock = socket(AF_INET, SOCK_STREAM, 0);
    //create socket
    if( sock == -1)
    {
        printf("socket create fail!\n");
    }

    //bind
    myAddr.sin_family = AF_INET;
    myAddr.sin_port = htons(SERVERPORT);
    myAddr.sin_addr.s_addr = INADDR_ANY;
    bzero( &(myAddr.sin_zero), 8);
    if(bind(sock, (sockaddr*)(&myAddr), sizeof(sockaddr)) ==-1 )
    {
        printf("bind error!\n");
    }

    //listen
    if(listen(sock, BACKLOG) == -1)
    {
        printf("listen error\n");
    }

    while (true)
    {
        unsigned int sin_size = sizeof(sockaddr_in);
        if( (client_fd = accept(sock, (sockaddr*)(&remoteAddr), &sin_size)) ==-1 )
        {
            printf("accept error!\n");
            continue;
        }

        pthread_t nPid;
        int newT = pthread_create(&nPid, NULL, readPower, &client_fd);
        if(newT != 0) {
            printf("creat thread failed!\n");
        }
        pthread_join(nPid, NULL);
        
    }  
}
