#ifndef _ST_COMMUNITE__H_
#define _ST_COMMUNITE__H_

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "task_def.h"
class	CNetworkServer{
public:
	CNetworkServer();
	~CNetworkServer();

public:
	int   StartNetListenServer();
	int   StopNetListenServer();
	int   Accept();

	int   ReciveData(int iSocketID,char *szBuffer,int iReciveDataLen);
	int   SendData(int iSocket,char *szDataBuffer,int iDataLen);
	char* GetClientIP(int iClientConnectHandle);
	int   Close(int iClient);
	int   ReleasClientIP(char *szIPName);

	void SetSvrPort(int iSvrPort,char *szIPAddress);
	int CloseClient(int iClientHandle);

	int  m_iPort;
	char m_szServerIP[128];
	int  m_iServerSocketHandle;
};

#endif //_ST_COMMUNITE__H_
