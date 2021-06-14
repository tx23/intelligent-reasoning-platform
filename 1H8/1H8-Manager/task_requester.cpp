#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "task_requester.h"
#include <sys/time.h>

#include <sys/types.h>
#include <sys/wait.h>

CTaskRequester::CTaskRequester(){}
CTaskRequester::~CTaskRequester(){}

void CTaskRequester::SetServerParame(char *szServerIP,int iServerPort)
{
	m_netWorkClient.SetSvrParame(szServerIP, iServerPort);
}

void CTaskRequester::ImageManage()
{
    img_detection imd;
	while(m_netWorkClient.ConnectSvr() < 0)
	{
		printf("connect server fail !\n");
        usleep(1000*1000);
	}
    SystemCheck syscheck;
	syscheck.init();
    Detector* det = new Detector(model_file);
	Timer sys;
	sys.init();
	while(true)
	{
		DATA_HEAD stSendDataHead;
		sys.record_time();
		float check = sys.getDuration()/1000;
        if(check > 350)
		{
			sys.time_interval_ = 0;
		}

		stSendDataHead.iDatalen = 0;
		stSendDataHead.emCommand = EM_COMMON_TYPE_GET_IMAGE;
		int iSendData = m_netWorkClient.SendData((char*)&stSendDataHead,sizeof(stSendDataHead));	
		if(iSendData != sizeof(stSendDataHead))
		{			
			usleep(1000*1000);
			printf("send data to svr is err %d !\n",iSendData);
			continue;
		}
		

		DATA_HEAD stRecvDataHead;		
		int iRecive = m_netWorkClient.ReciveData((char*)&stRecvDataHead, sizeof(stRecvDataHead));
		if(iRecive != sizeof(stRecvDataHead))
		{
			usleep(1000*1000);
			printf("recive data to svr is err %d  !\n",iRecive);
			continue;
		}
	
		if(stRecvDataHead.emCommand == EM_COMMON_TYPE_GET_IMAGE)
		{	
			if(stRecvDataHead.iDatalen <= 0)
			{
				usleep(10*1000);
				continue;
			}


			char szBuffer[stRecvDataHead.iDatalen] ;	
			memset(szBuffer, 0, stRecvDataHead.iDatalen);
			int iRecive = m_netWorkClient.ReciveData(szBuffer, stRecvDataHead.iDatalen);
			if(iRecive != stRecvDataHead.iDatalen)
			{
				usleep(1000*1000);
				printf("Svr is err %d !\n",iRecive);
				continue;
			}
			//detect time
			struct timeval timeStart, timeEnd;

			char *pBuffer = szBuffer;
			int iPicLen = iRecive;
			char TMP_Buffer[iPicLen];
			vector<detect_out> vectDetectData;
			memcpy(TMP_Buffer, pBuffer, iPicLen);
			imd.input_data = TMP_Buffer;
			imd.input_size = iPicLen;
			// Detector* det = new Detector(model_file);
			gettimeofday(&timeStart, NULL );
			det->detection(&imd);
			int num = imd.output_size / 9;

			gettimeofday(&timeEnd, NULL );
			double runTime = (timeEnd.tv_sec - timeStart.tv_sec ) + (double)(timeEnd.tv_usec -timeStart.tv_usec)/1000000;
			cout<< "detect time:" << runTime << endl;

			for(int i = 0; i < num; i++)
			{
				detect_out dec;
				dec.image_label = imd.output_data[i*9];
				dec.x1 = imd.output_data[i*9 + 1];
				dec.y1 = imd.output_data[i*9 + 2];
				dec.x2 = imd.output_data[i*9 + 3];
				dec.y2 = imd.output_data[i*9 + 4];
				dec.x3 = imd.output_data[i*9 + 5];
				dec.y3 = imd.output_data[i*9 + 6];
				dec.x4 = imd.output_data[i*9 + 7];
				dec.y4 = imd.output_data[i*9 + 8];
				dec.time = runTime;
				vectDetectData.push_back(dec);
				
				for(int m = 0; m < 9; m++)
					cout<< imd.output_data[i*9 + m] <<" ";
				
				printf("\n");
			}

			stSendDataHead.emCommand= EM_COMMON_TYPE_DETECT_RESULT;	
			int iDetectSize = vectDetectData.size();
			
			if(iDetectSize <= 0)
			{
				stSendDataHead.iDatalen = 0;
				m_netWorkClient.SendData((char*)&stSendDataHead, sizeof(stSendDataHead));			
			}
			else 
			{				
				stSendDataHead.iDatalen = sizeof(detect_out)*iDetectSize;
				int iDatalen =  sizeof(detect_out)*iDetectSize + sizeof(stSendDataHead);
				char pSendBuffer[iDatalen];
				memset(pSendBuffer, 0, iDatalen);
				char *pBuffer = pSendBuffer;
				memcpy(pBuffer, &stSendDataHead, sizeof(stSendDataHead));
				pBuffer = pBuffer + sizeof(stSendDataHead);
				for(int i= 0; i < iDetectSize; i++)
				{					
					memcpy((void*)pBuffer, (void*)&vectDetectData[i], sizeof(detect_out));
					pBuffer += sizeof(detect_out);
				}
				int iRet = m_netWorkClient.SendData((char*)pSendBuffer, iDatalen);	
			}
			// delete det;
		}
	}
	m_netWorkClient.DisConnectSvr();
}
