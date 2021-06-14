
#ifndef _C_TASK_REQUEST_H_
#define _C_TASK_REQUEST_H_

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <vector> 

#include "task_def.h"
#include "communicate.h"
#include "ssd_offline.h"
#include "timer.h"
#include "system_check.h"

using namespace std;

class CTaskRequester {
public:
	CTaskRequester();
	~CTaskRequester();

public:
	CNetworkClient m_netWorkClient;
	string model_file;
	
public:
    void ImageManage();
	void SetServerParame(char *szServerIP,int iServerPort);
};


#endif //_C_TASK_REQUEST_H_
