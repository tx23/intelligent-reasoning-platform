#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <time.h>
#include <iostream>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include "task_requester.h"

using namespace std;

//通信线程
void *comThread(void *arg)
{
    CTaskRequester* tar =  (CTaskRequester *)arg;
	tar->ImageManage();
}


pid_t transanction(char** argv)
{
	pid_t child = fork();
	if(child == 0)
	{
	    CTaskRequester req;
		req.model_file = argv[3];
	    req.SetServerParame(argv[1], atoi(argv[2]));
	
		//通信线程
		pthread_t cPid;
		int com = pthread_create(&cPid, NULL, comThread, &req);
	    if (com != 0) {
			printf("create comThread failed.\n");
			return -1;
		}
		pthread_join(cPid, NULL);	
     }
	return child;
}

int main(int argc, char** argv)
{
	if(argc != 4)
	{
		printf("Format error. correct: ./transaction_process IP PORT MODEL_FILE\n");
		return 0;
	}
	
	pid_t child_transanction;
    int status, tmp_pid;
	child_transanction = transanction(argv);

	while(true)
	{
		tmp_pid = waitpid(-1,&status,WUNTRACED);
	    if(tmp_pid == -1)
			break;
		else if(tmp_pid == child_transanction)
		{
			printf("业务进程退出 !\n");
			child_transanction = transanction(argv);
			printf("业务进程加载成功 !\n");
		}
	}
}
