#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include <signal.h>
#include <sys/wait.h>
#include <time.h>
#include <string.h>
#include <iostream>
#include <mutex>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/ipc.h>

#include "task_readimg.h"
#include "task_agency.h"
#include "task_schedule.h"

#include <netinet/in.h>
#include <unistd.h>     //for fork and read
#include <sys/types.h>  //for socket
#include <sys/socket.h> //for socket
#include <string.h>     // for bzero
#include <arpa/inet.h>
#include <pthread.h>

#define imgNum 1

int TaskAgency::flagGet = 0;

//读图线程
void *redThread(void *arg)
{
    readImage *red = (readImage *)arg;
    red->run();
}

//通信子线程
void *agencyChildThread(void *arg)
{
    TaskAgency *tagchild = (TaskAgency *)arg;
    tagchild->AcceptData();
}

//通信数据处理线程
void *dataDealThread(void *arg)
{
    TaskAgency *tad = (TaskAgency *)arg;
    tad->DataDeal();
}

//通信主线程
void *agencyThread(void *arg)
{
    TaskAgency *tag = (TaskAgency *)arg;
    for (int i = 0; i < 4; i++)
    {
        tag[i].id = i;
        // pthread_t aPid, dPid, hPid;
        pthread_t aPid, dPid;
        int tac = pthread_create(&aPid, NULL, agencyChildThread, &tag[i]);
        if (tac != 0)
        {
            printf("create agencyChildThread_%d failed.\n", i);
            break;
        }
        int tad = pthread_create(&dPid, NULL, dataDealThread, &tag[i]);
        if (tac != 0)
        {
            printf("create dataDealThread_%d failed.\n", i);
            break;
        }
    }
}



int main(int argc, char *argv[])
{
    
    //1H8 PS
    void *shared_memory = NULL;
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    int shm_id = shmget((key_t)0x1122, sizeof(sm_msg), 0666 | IPC_CREAT);
    shared_memory = shmat(shm_id, NULL, 0);
    sm_msg *msg = (sm_msg *)shared_memory;
    pthread_mutex_init(&msg->sm_mutex, &attr);

    pthread_mutex_lock(&msg->sm_mutex);
    msg->buf[0] = 1;  //读图标志位
    msg->buf[1] = 0;  //自检标志位
    msg->buf[2] = 0;  //模式改变标志位
    msg->buf[17] = 0; //强化学习自检标志位
    msg->buf[18] = 0; //强化学习开始结束标志位
    int in[5] = {1, 0, 0, 0, 0};
    for (int i = 0; i < 5; i++)
    {
        msg->buf[i + 3] = in[i];
    }
    pthread_mutex_unlock(&msg->sm_mutex);

    //创建图片队列
    BlockingQueue<PictureInfor> PictureList[4];

    
    

    //初始化IP、队列等参数 ---------------------------
    // int port[4] = {7001, 7002, 7003, 7004};
    
    if(argc != 3) {
        printf("Need 2 parameters,1h8 number and model number.\n");
        printf("1h8 number:[1/2/3/4]\n");
        printf("model number:[1/2/3/4]\n");
        // return -1;
    }
    int portBackNum = 0;
    int* portBack;
    int* writer;
    unsigned short h8N = 0; // 1h8 Number
    unsigned short modelN = 0;// model Number
    
    switch ( *argv[1])
    {
    case '1':
        printf("you chose 1h8-1\n");
        portBack = (int*)malloc(sizeof(int)*4);
        writer = portBack;
        portBackNum = 4;
        h8N = 1;
        for(int i = 0; i < portBackNum; ++i) {
            *writer = 7100;
            writer++;
        }
        break;
    case '2':
        printf("you chose 1h8-2\n");
        portBack = (int*)malloc(sizeof(int)*4);
        writer = portBack;
        portBackNum = 4;
        h8N = 2;
        for(int i = 0; i < portBackNum; ++i) {
            *writer = 7200;
            writer++;
        }
        break;
    case '3':
        printf("you chose 1h8-3\n");
        portBack = (int*)malloc(sizeof(int)*4);
        writer = portBack;
        // portBackNum = 2;
        portBackNum = 4;
        h8N = 3;
        for(int i = 0; i < portBackNum; ++i) {
            *writer = 7300;
            writer++;
        }
        break;
    case '4':
        printf("you chose 1h8-4\n");
        portBack = (int*)malloc(sizeof(int)*4);
        writer = portBack;
        // portBackNum = 2;
        portBackNum = 4;
        h8N = 4;
        for(int i = 0; i < portBackNum; ++i) {
            *writer = 7400;
            writer++;
        }
        break;
    default:
        printf("error: 1h8 number [1/2/3/4]\n");
        return -1;
    }


    switch ( *argv[2])
    {
    case '1':
        printf("you chose model-1\n");
        writer = portBack;
        modelN = 1;
        for(int i = 1; i <= 1*portBackNum; ++i) {
            *writer += i;
            writer++;
        }
        break;
    case '2':
        printf("you chose model-2\n");
        writer = portBack;
        modelN = 2;
        for(int i = (1*portBackNum+1); i <= 2*portBackNum; ++i) {
            *writer += i;
            writer++;
        }
        break;
    case '3':
        printf("you chose model-3\n");
        writer = portBack;
        modelN = 3;
        for(int i = (2*portBackNum+1); i <= 3*portBackNum; ++i) {
            *writer += i;
            writer++;
        }
        break;
    case '4':
        printf("you chose model-4\n");
        writer = portBack;
        modelN = 4;
        for(int i = (3*portBackNum+1); i <= 4*portBackNum; ++i) {
            *writer += i;
            writer++;
        }
        break;
    default:
        printf("error: model number [1/2/3/4]\n");
        return -1;
    }

    int port[portBackNum];
    writer = portBack;
    for(int i = 0; i < portBackNum; ++i) {
        port[i] = *writer;
        writer++;
    }

    for(int i = 0; i < portBackNum; ++i) {
        printf("port %d: %d\n", i+1, port[i]);
    }
    free(portBack);
    //初始化IP、队列等参数 ----------------------------


    string strIP[4] = {"192.168.3.10",
                       "192.168.3.10",
                       "192.168.3.10",
                       "192.168.3.10"};
    TaskAgency *taskAg = new TaskAgency[4];
    for (int i = 0; i < 4; i++)
    {
        memset(taskAg[i].m_strClientIP, 0, 125);
        //设置队列
        pthread_mutex_lock(&msg->sm_mutex);
        cout << "1H8:" << i << " use PictureList:" << msg->buf[i + 4] << endl;
        taskAg[i].SetPictureList(&PictureList[msg->buf[i + 4]]);
        pthread_mutex_unlock(&msg->sm_mutex);
        //设置IP、端口
        taskAg[i].SetParame(port[i], (char *)strIP[i].c_str());
    }

    //通信线程(4个子线程)
    pthread_t tPid;
    int tag = pthread_create(&tPid, NULL, agencyThread, taskAg);
    if (tag != 0)
    {
        printf("create agencyThread failed.\n");
        return -1;
    }

    


    //server
    //-------------------------------------------------
    // int h8N = 2, modelN = 1;
    const unsigned short SERVERPORT = 7000 + (h8N-1)*4 + modelN;
    cout << "server port: " << SERVERPORT<< endl;

    const int BACKLOG = 5; //5 个最大的连接数
    
    int sock, client_fd;
    sockaddr_in myAddr;
    sockaddr_in remoteAddr;
    sock = socket(AF_INET, SOCK_STREAM, 0);
    //create socket
    if( sock == -1)
    {
        cerr<<"socket create fail!"<<endl;
    }

    //bind
    myAddr.sin_family = AF_INET;
    myAddr.sin_port = htons(SERVERPORT);
    myAddr.sin_addr.s_addr = INADDR_ANY;
    bzero( &(myAddr.sin_zero), 8);
    if(bind(sock, (sockaddr*)(&myAddr), sizeof(sockaddr)) ==-1 )
    {
        cerr<<"bind error!"<<endl;
    }

    //listen
    if(listen(sock, BACKLOG) == -1)
    {
        cerr<<"listen error"<<endl;
    }

    while(true)
    {

        unsigned int sin_size = sizeof(sockaddr_in);
        if( (client_fd = accept(sock, (sockaddr*)(&remoteAddr), &sin_size)) ==-1 )
        {
            cerr<<"accept error!"<<endl;
            continue;
        }
        cout<<"Received a connection from "<<static_cast<char*>(inet_ntoa(remoteAddr.sin_addr) )<<endl;

        for (int i = 0; i < 4; i++)
        {
            taskAg[i].clientFd = client_fd;
        }

        //读图线程
        readImage *readImg = new readImage(msg, PictureList);
        pthread_t rPid;
        int red = pthread_create(&rPid, NULL, redThread, readImg);
        if (red != 0)
        {
            printf("create readImageThread failed.\n");
            return -1;
        }

        pthread_join(rPid, NULL);


    }

    pthread_join(tPid, NULL);
}
