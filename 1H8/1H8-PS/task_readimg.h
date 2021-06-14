#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/sem.h>
#include "task_def.h"
#include "blocking_queue.h"
#include "data_recv.h"


class readImage
{
	public:
		readImage(sm_msg* msg, BlockingQueue<PictureInfor>* picturelsit);
		~readImage();
		void load();
		void load_test();
		void load_pl();
		void check();
		void run();
	
	private:
		int k, size;
		char* share_data;
		sm_msg* msg;
        DataRecv* dr;
		BlockingQueue<PictureInfor>* list; 
};
