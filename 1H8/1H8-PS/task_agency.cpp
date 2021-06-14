// #include <stdio.h>
// #include <string.h>
// #include <unistd.h>
#include <sys/socket.h>
// #include <ctime>
#include "task_agency.h"

TaskAgency::TaskAgency(){
	iClienthandle = -1;
    void *shared_memory = NULL;

   shared_memory = shmat(3440702, NULL, 0);
    // shared_memory = shmat(46, NULL, 0);
    msg = (sm_msg *)shared_memory;
}




TaskAgency::~TaskAgency(){}

uint16_t TaskAgency::change_order(uint16_t input){
    uint16_t t1 = 0;
    uint16_t t2 = 0;
    uint16_t result = 0;

    t1 = input >> 8;
    t2 = input << 8;
    result = t1 | t2;

    return result;
}

void TaskAgency::SetParame(int iPort, char *szIPAddress)
{
    m_netWorkServer.SetSvrPort(iPort, szIPAddress);
}

void TaskAgency::SetPictureList(BlockingQueue<PictureInfor> *pPicturelist)
{
    if (pPicturelist != NULL)
        m_pPictureList = pPicturelist;
}

void* TaskAgency::DataDeal()
{	
	
	DATA_HEAD stDataHead;
    heart = 0;
	int count = 0;
    while (true)
	{
        int iDatalen = 0;
        usleep(1000 * 10);
        
        pthread_mutex_lock(&sock);
        int iRet = m_netWorkServer.ReciveData(iClienthandle, (char *)&stDataHead, sizeof(stDataHead));
        pthread_mutex_unlock(&sock);

        if(iRet != sizeof(stDataHead)) 
        {
            if(iRet == 0)
            {
                pthread_mutex_lock(&heart_mutex);
                heart++;
                pthread_mutex_unlock(&heart_mutex);
                continue;
            }else
            {
                pthread_mutex_lock(&heart_mutex);
                heart = 0;
                pthread_mutex_unlock(&heart_mutex);
                continue;
            }    
        }else
        {
            pthread_mutex_lock(&heart_mutex);
            heart = 0;
            pthread_mutex_unlock(&heart_mutex);
        }   

        if(stDataHead.emCommand == EM_COMMON_TYPE_SYS_INFO)
		{
        	int iDataLen = stDataHead.iDatalen;
            char sysbuf[iDataLen];
        	memset(sysbuf, 0, iDataLen);

            pthread_mutex_lock(&sock);
        	int iRet = m_netWorkServer.ReciveData(iClienthandle, sysbuf, iDataLen);
            pthread_mutex_unlock(&sock);

			if(iRet == iDataLen)
			{
				status_1h8* status = (status_1h8 *)sysbuf;
				
				if(count > 15)
				{
					count = 0;
				}
				count++;

				pthread_mutex_lock(&msg->pic_mutex);
				memcpy(&msg->status[id], status, sizeof(*status));
				pthread_mutex_unlock(&msg->pic_mutex);
			}
		}

        if(stDataHead.emCommand == EM_COMMON_TYPE_GET_IMAGE)
        {
        	DATA_HEAD sendDataHead;
        	sendDataHead.emCommand = EM_COMMON_TYPE_GET_IMAGE;
        	int iSize = m_pPictureList->size();
            PictureInfor pstPictureInfor;
        	if (iSize > 0)
        	{
        		pstPictureInfor = m_pPictureList->pop();
			    cout<<"1H8: "<<id<<"---------get image: "<<pstPictureInfor.number<<endl;
				imgNum = pstPictureInfor.number;
				gettimeofday(&startTime, NULL);
        		int iLen = pstPictureInfor.iPictureLen + sizeof(sendDataHead);
        		sendDataHead.iDatalen = pstPictureInfor.iPictureLen;
        		char szBuffer[iLen] = {0};

				//提取所需数据
				memcpy(timeCode, pstPictureInfor.iPictureData + pstPictureInfor.iPictureLen + 8, 8); //保存时间码
				frameSeq = uint8_t(pstPictureInfor.number % 255);
				imgsize = sqrt(pstPictureInfor.iPictureLen / 3);

        		memcpy(szBuffer, &sendDataHead, sizeof(sendDataHead));
        		memcpy(szBuffer + sizeof(sendDataHead), pstPictureInfor.iPictureData, pstPictureInfor.iPictureLen);

                pthread_mutex_lock(&sock);
        		iRet = m_netWorkServer.SendData(iClienthandle, szBuffer, iLen);
                pthread_mutex_unlock(&sock);
        
        		if (iRet != iLen)
        			break;

        		delete []pstPictureInfor.iPictureData;

        	}
			else if (iSize <= 0)
        	{
        		sendDataHead.iDatalen = 0;

                pthread_mutex_lock(&sock);
        		iRet = m_netWorkServer.SendData(iClienthandle, (char *)&sendDataHead, sizeof(sendDataHead));
                pthread_mutex_unlock(&sock);
        		continue;
        	}
        }
        
        if (stDataHead.emCommand == EM_COMMON_TYPE_DETECT_RESULT)
        {

			// cout << "get return sign!" << endl;
			flagGet++;
			gettimeofday(&endTime, NULL);

			double totalTime = (endTime.tv_sec - startTime.tv_sec) + (double)(endTime.tv_usec - startTime.tv_usec)/1000000;
			cout << "time: " << totalTime << endl;
			cout << "id:" << id << endl
					  << "return img: " << imgNum <<  endl
					  << " total img: " << flagGet << endl;
        	if (stDataHead.iDatalen <= 0) {
				int sendLen = 24;
				char *sendData = (char*)malloc(sizeof(char) * sendLen);
				memset(sendData, 0, sendLen);
				char *writer = sendData;
				*(int64_t*)writer = (int64_t)imgNum;
				writer+=8;
				*(int64_t*)writer = 0;
				writer+=8;
				*(double*)writer = totalTime;
				if( send(clientFd, sendData, sendLen, 0) == -1)
					cerr<<"send error!"<<endl;
				if(flagGet >= 6) {
					cout << "close fd!\n" << endl;
					close(clientFd);
					flagGet = 0;
				}
				free(sendData);
        		continue;
			}
        	int iDataLen = stDataHead.iDatalen;
        	char szBuffer[iDataLen];
        	memset(szBuffer, 0, iDataLen);

            pthread_mutex_lock(&sock);
        	int iRet = m_netWorkServer.ReciveData(iClienthandle, szBuffer, iDataLen);
            pthread_mutex_unlock(&sock);
        	if (iRet != iDataLen) {
				int sendLen = 24;
				char *sendData = (char*)malloc(sizeof(char) * sendLen);
				memset(sendData, 0, sendLen);
				char *writer = sendData;
				*(int64_t*)writer = (int64_t)imgNum;
				writer+=8;
				*(int64_t*)writer = 0;
				writer+=8;
				*(double*)writer = totalTime;
				if( send(clientFd, sendData, sendLen, 0) == -1)
					cerr<<"send error!"<<endl;
				if(flagGet >= 6) {
					close(clientFd);
					cout << "close fd!\n" << endl;
					flagGet = 0;
				}
				free(sendData);
				continue;
			}
        
        	int iVecSize = iDataLen / sizeof(detect_out);
        	
        	if (iVecSize <= 0) {
				int sendLen = 24;
				char *sendData = (char*)malloc(sizeof(char) * sendLen);
				memset(sendData, 0, sendLen);
				char *writer = sendData;
				*(int64_t*)writer = (int64_t)imgNum;
				writer+=8;
				*(int64_t*)writer = 0;
				writer+=8;
				*(double*)writer = totalTime;
				if( send(clientFd, sendData, sendLen, 0) == -1)
					cerr<<"send error!"<<endl;
				if(flagGet >= 6) {
					close(clientFd);
					cout << "close fd!\n" << endl;
					flagGet = 0;
				}
				free(sendData);
				continue;
			}
        
        	char *pBuffer = szBuffer;
        	vector<detect_out> detections;
        	for (int i = 0; i < iVecSize; i++)
        	{
        		detect_out *pDetectOut = (detect_out *)pBuffer;
        		pBuffer = pBuffer + sizeof(detect_out);
        		detections.push_back(*pDetectOut);
        	}
            for(int s = 0; s < detections.size(); s++)
            {
                cout<<detections[s].image_label<<" ";
                cout<<detections[s].x1<<" ";
                cout<<detections[s].y1<<" ";
                cout<<detections[s].x2<<" ";
                cout<<detections[s].y2<<" ";
                cout<<detections[s].x3<<" ";
                cout<<detections[s].y3<<" ";
                cout<<detections[s].x4<<" ";
                cout<<detections[s].y4<<" ";

                printf("\n");
            }
			//填充识别结果到共享内存区
            
            printf("开始填充..........\n");

		    pthread_mutex_lock(&msg->sm_mutex);
			int front = msg->buf[8]; //队首
			int rear = msg->buf[9];   //队尾的后一个元素
			int modelNum = msg->buf[4+id];
		    pthread_mutex_unlock(&msg->sm_mutex);

			//判断队列是否已满
            if( (rear + 1) % 20 == front )
			{
				printf("队列已满!!!\n");
				// continue;
			}

            pthread_mutex_lock(&msg->pic_mutex);
			memset(&msg->picObj[rear], 0x00, sizeof(msg->picObj[rear]));//清空
			msg->picObj[rear].frameLabel = 0xB5;
			msg->picObj[rear].frameType = 0x0A;
			memcpy(msg->picObj[rear].picId, timeCode, 8);
			msg->picObj[rear].modelNum = modelNum;
			msg->picObj[rear].labelNum = 0x04;
			msg->picObj[rear].frameSeq = frameSeq;
			int det;
			if(detections.size() < 6)
				det = detections.size();
			else
				det = 6;
			for(int j = 0; j < det; j++)
			{
				msg->picObj[rear].oBase[j].objType = uint8_t(detections[j].image_label);
				msg->picObj[rear].oBase[j].zsX = change_order(uint16_t(detections[j].x1*imgsize));
				msg->picObj[rear].oBase[j].zsY = change_order(uint16_t(detections[j].y1*imgsize));
				msg->picObj[rear].oBase[j].ysX = change_order(uint16_t(detections[j].x2*imgsize));
				msg->picObj[rear].oBase[j].ysY = change_order(uint16_t(detections[j].y2*imgsize));
				msg->picObj[rear].oBase[j].zxX = change_order(uint16_t(detections[j].x3*imgsize));
				msg->picObj[rear].oBase[j].zxY = change_order(uint16_t(detections[j].y3*imgsize));
				msg->picObj[rear].oBase[j].yxX = change_order(uint16_t(detections[j].x4*imgsize));
				msg->picObj[rear].oBase[j].yxY = change_order(uint16_t(detections[j].y4*imgsize));
			}
            pthread_mutex_unlock(&msg->pic_mutex);
            
			//发送回go
			
			// int lendet = det;

			int sendLen = 24+16*det;
			char *sendData = (char*)malloc(sizeof(char) * sendLen);
			memset(sendData, 0, sendLen);
			char *writer = sendData;
			*(int64_t*)writer = (int64_t)imgNum;
			writer+=8;
			*(int64_t*)writer = (int64_t)det;
			writer+=8;
			for(int t = 0; t < det; t++) {
				*(float*)writer = (float)detections[t].x1;
				writer+=4;
				*(float*)writer = (float)detections[t].y1;
				writer+=4;
				*(float*)writer = (float)detections[t].x4;
				writer+=4;
				*(float*)writer = (float)detections[t].y4;
				writer+=4;
			}	

			*(double*)writer = totalTime;
			if( send(clientFd, sendData, sendLen, 0) == -1)
				cerr<<"send error!"<<endl;
			if(flagGet >= 6) {
				close(clientFd);
				cout << "close fd!" << endl;
				flagGet = 0;
			}
			free(sendData);

			rear = (rear + 1) % 20;
            pthread_mutex_lock(&msg->sm_mutex);
			msg->buf[9] = rear; //队尾前进一
		    pthread_mutex_unlock(&msg->sm_mutex);
        
		    		
        }
        pthread_mutex_unlock(&sock);
	} //end while

}

int TaskAgency::AcceptData()
{

	if (m_netWorkServer.StartNetListenServer() != 0)
		return -1;


    int socketID;
	while (1)
	{
        socketID = -1;
		socketID = m_netWorkServer.Accept();
		if (socketID > 0)
		{
            pthread_mutex_lock(&sock);
			m_netWorkServer.CloseClient(iClienthandle);
			iClienthandle = socketID;
            pthread_mutex_unlock(&sock);
		}

        pthread_mutex_lock(&msg->sm_mutex);
        pthread_mutex_lock(&heart_mutex);
        if(heart > 5000)
        {
            msg->buf[13+id] = 0;
            printf("1H8:%d 连接超时!\n", id);
        }
        else
        {
            msg->buf[13+id] = 1;
        }
        pthread_mutex_unlock(&heart_mutex);
        pthread_mutex_unlock(&msg->sm_mutex);
	}

}


