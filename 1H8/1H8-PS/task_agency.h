#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <queue>
#include <pthread.h>
#include <iostream>
#include <math.h>
#include "blocking_queue.h"
#include "communicate.h"
// #include <ctime>
#include <sys/time.h>

#ifndef TASK_AGENCY
#define TASK_AGENCY

using namespace std;

class	TaskAgency 
{
  public:
  	TaskAgency();
  	~TaskAgency();
  	
  public:
  	char m_strClientIP[125];
  	char m_szServerIP[16];
	int  id;
	uint8_t timeCode[8];
	uint8_t frameSeq;
	int imgsize;
    int heart;
    pthread_mutex_t heart_mutex;
    sm_msg* msg;
	int clientFd;
	static int flagGet;
	struct timeval startTime, endTime;

	int imgNum;
    uint16_t change_order(uint16_t input);
  	BlockingQueue<PictureInfor> *m_pPictureList;
  	CNetworkServer m_netWorkServer;


  
  public:
    void* DataDeal();
    int AcceptData();
	int iClienthandle;
	pthread_mutex_t sock;
  	void SetPictureList(BlockingQueue<PictureInfor> *pPicturelist);
  	void SetParame(int iPort, char *szIPAddress);
  

};

#endif
