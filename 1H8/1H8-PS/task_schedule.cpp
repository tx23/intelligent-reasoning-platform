#include "task_schedule.h"

TaskSchedule::TaskSchedule(sm_msg* msg, TaskAgency* taskAg, BlockingQueue<PictureInfor>* PictureList)
{
	this->msg = msg;
	this->taskAg = taskAg;
	this->m_PictureList = PictureList;

    rbuf.rp_1 = (int *)malloc(104);
    rbuf.rp_2 = (int *)malloc(104);
    rbuf.wp_1 = (int *)malloc(52);
    rbuf.wp_2 = (int *)malloc(52);

}


//模式切换
void TaskSchedule::ModeSwitch()
{
	usleep(1000*10);
	pthread_mutex_lock(&msg->sm_mutex);
    int swflag = msg->buf[2];
	pthread_mutex_unlock(&msg->sm_mutex);

	if(swflag == 1)
	{
		for(int i = 0;i < 4;i++)
		{	
    	    pthread_mutex_lock(&msg->sm_mutex);
		    cout<<"1H8:"<<i<<" use PictureList:"<<msg->buf[i+4]<<endl;
		    taskAg[i].SetPictureList(&m_PictureList[msg->buf[i+4]]);
			msg->buf[2] = 0;
		    pthread_mutex_unlock(&msg->sm_mutex);
		}	
	    for(int i = 0;i < 4;i++)
            if(m_PictureList[i].clear()) 
			    printf("m_PictureList: %d clear success!\n", i);
	}
}


//强化学习
void TaskSchedule::Reinforcement()
{
    pthread_mutex_lock(&msg->sm_mutex);
    int startflag = msg->buf[17];
    int checkflag = msg->buf[18];
    pthread_mutex_unlock(&msg->sm_mutex);


    if(checkflag == 1)
    {
        pthread_mutex_lock(&msg->ref_mutex);
        memcpy((int *)rbuf.rp_1, msg->rp1, 104);
        memcpy((int *)rbuf.rp_2, msg->rp2, 104);
        pthread_mutex_unlock(&msg->ref_mutex);
        //自检，强化学习8次，返回结果
        for(int i = 0; i < 8; i++)
        {
            rl(&rbuf);
            memcpy((int *)rbuf.rp_1, rbuf.wp_1, 28);
            memcpy((int *)rbuf.rp_1 + 52, rbuf.wp_1, 28);
            memcpy((int *)rbuf.rp_2, rbuf.wp_2, 28);
            memcpy((int *)rbuf.rp_2 + 52, rbuf.wp_2, 28);
        }
        
        //将结果返回共享内存
        pthread_mutex_lock(&msg->ref_mutex);
        memcpy(msg->wp1, (int *)rbuf.wp_1, 52);
        memcpy(msg->wp2, (int *)rbuf.wp_2, 52);
        pthread_mutex_unlock(&msg->ref_mutex);
       
        //自检结束，标志位置0
        checkflag = 0;
    }
    //开始强化学习
    if(startflag == 1)
    {
        printf("拷贝输入数据...\n");
        pthread_mutex_lock(&msg->ref_mutex);
        memcpy((int *)rbuf.rp_1, msg->rp1, 104);
        memcpy((int *)rbuf.rp_2, msg->rp2, 104);
        pthread_mutex_unlock(&msg->ref_mutex);

        while(true)
        {
            rl(&rbuf);
            memcpy((int *)rbuf.rp_1, rbuf.wp_1, 28);
            memcpy((int *)rbuf.rp_1 + 52, rbuf.wp_1, 28);
            memcpy((int *)rbuf.rp_2, rbuf.wp_2, 28);
            memcpy((int *)rbuf.rp_2 + 52, rbuf.wp_2, 28);

            pthread_mutex_lock(&msg->sm_mutex);
            startflag = msg->buf[17];
            pthread_mutex_unlock(&msg->sm_mutex);
            if(startflag == 0)
               break;
        }
        printf("返回结果....\n");
        //将结果返回共享内存
        pthread_mutex_lock(&msg->ref_mutex);
        memcpy(msg->wp1, (int *)rbuf.wp_1, 52);
        memcpy(msg->wp1, (int *)rbuf.wp_2, 52);
        pthread_mutex_unlock(&msg->ref_mutex);
 
    }
}
