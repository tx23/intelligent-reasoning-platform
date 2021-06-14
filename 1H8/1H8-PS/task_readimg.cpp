#include "task_readimg.h"
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
using namespace cv;


readImage::readImage(sm_msg* msg, BlockingQueue<PictureInfor>* picturelist)
{
	this->list = picturelist;
	this->msg = msg;
	size = sizeof(char) * (256*256 + 16);
    share_data = (char *)malloc(size);
	memset(share_data,0x01,size);
	k = 1;
	dr = new DataRecv;
}


void readImage::load()
{
	usleep(1000*10);
    pthread_mutex_lock(&msg->sm_mutex);
	int num = msg->buf[3];
	pthread_mutex_unlock(&msg->sm_mutex);
	printf("当前是第%d张图......\n", k);
	for(int i = 0; i < num; i++)
	{
		PictureInfor pict;
        pict.iPictureLen = size;
        pict.number = k;
        pict.iPictureData = new char[size];
        char* buffer = (char*)pict.iPictureData;
		memcpy(buffer, share_data, size);
		list[i].push(pict);
	}
	k++;
}

void readImage::load_test()
{
		printf("load image......\n", k);

    usleep(1000*10);
	const string imgName[6] = {"./1.jpg",
								"./2.jpg",
								"./3.jpg",
								"./4.jpg",
								"./5.jpg",
								"./6.jpg"};
	for(int t = 1; t < 2; t++) {
		Mat test = imread(imgName[t-1], -1);
		int img_width = test.cols;
	        int img_height = test.rows;
		char* p = (char*)malloc(sizeof(char)*img_width*img_height*3);
		for(int c = 0; c < 3; c++)
        	for (int i = 0; i < img_height; i++)
            	for(int j = 0; j < img_width; j++)
        {
                p[c*img_height*img_width + i*img_width+j] = test.at<Vec3b>(i, j)[c];
        }
		int size = img_height*img_width*sizeof(char)*3;
		// for(int c = 0; c < 3; c++)
		// for(int i = 0; i < 256; i++)
		// 	for(int j = 0; j <256; j++)
		// 		p[i*256 + j] = test.at<Vec3b>(i, j)[0];
		// int size = 256*256*sizeof(char);
		pthread_mutex_lock(&msg->sm_mutex);
		int num = msg->buf[3];
		pthread_mutex_unlock(&msg->sm_mutex);
		printf("当前是第%d张图......\n", k);
		for(int i = 0; i < num; i++)
		{
			PictureInfor pict;
			pict.iPictureLen = size;
			pict.number = k;
			pict.iPictureData = new char[size+16];
			char* buffer = (char*)pict.iPictureData;
			memcpy(buffer, p, size);
			list[i].push(pict);
		}
		k++;
	free(p);
	}

}


void readImage::load_pl()
{
	usleep(1000*10);
    unsigned len=0;
    while(len==0)
        len = dr->buffer_recv(dr->recv_buffer, NULL, NULL);
	printf("recieve %8x B\n", len);
    pthread_mutex_lock(&msg->sm_mutex);
	int num = msg->buf[3];
	pthread_mutex_unlock(&msg->sm_mutex);
	printf("当前是第%d张图......\n", k);
	for(int i = 0; i < num; i++)
	{
		PictureInfor pict;
        pict.iPictureLen = len;
        pict.number = k;
        pict.iPictureData = new char[len];
        char* buffer = (char*)pict.iPictureData;
		memcpy(buffer, dr->recv_buffer+16, len-16);
		memcpy(buffer+len-16, dr->recv_buffer, 16);
		list[i].push(pict);
	}
	k++;

}


void readImage::check()
{
    pthread_mutex_lock(&msg->sm_mutex);
	int num = msg->buf[3];
	pthread_mutex_unlock(&msg->sm_mutex);

	printf("当前是第%d张自检图片...\n", k);
	for(int i = 0; i < num; i++)
	{
		PictureInfor pict;
        pict.iPictureLen = size;
        pict.number = k;
        pict.iPictureData = new char[size];
        char* buffer = (char*)pict.iPictureData;
		memcpy(buffer, share_data, size);
		list[i].push(pict);
	}
}


void readImage::run()
{
	pthread_mutex_lock(&msg->sm_mutex);
    int imflag = msg->buf[0];
	int checkflag = msg->buf[1];
	pthread_mutex_unlock(&msg->sm_mutex);

	if(imflag == 1)
	{
		load_test();
	}else if(imflag == 0)
	{
		usleep(1000*10);
		k = 1;
		if(checkflag == 1)
		{
			printf("开始自检......\n");
			check();
			pthread_mutex_lock(&msg->sm_mutex);
			msg->buf[1] = 0;
			pthread_mutex_unlock(&msg->sm_mutex);
			usleep(1000*10);
			printf("自检完成......\n");
		}
	}
}


