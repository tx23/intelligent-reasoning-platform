
//#ifndef __API_H
//#define __API_H

#include <stdio.h>
#include <stdlib.h>

#include "pthread.h"        // std::thread
#include "unistd.h"         // std::mutex

typedef struct{
	int GBa_Cur_J1;
	int GBa_Cur_J2;
	int GBa_Cur_J3;
	int GBa_Cur_J4;
	int GBa_Cur_J5;
	int GBa_Cur_J6;
	int GBa_Cur_J7;
	int GBa_Dst_X;
	int GBa_Dst_Y;
	int GBa_Dst_Z;
	int GBa_Dst_Alpha;
	int GBa_Dst_Beta;
	int GBa_Dst_Gammar;
	int GBb_Cur_J1;
	int GBb_Cur_J2;
	int GBb_Cur_J3;
	int GBb_Cur_J4;
	int GBb_Cur_J5;
	int GBb_Cur_J6;
	int GBb_Cur_J7;
	int GBb_Dst_X;
	int GBb_Dst_Y;
	int GBb_Dst_Z;
	int GBb_Dst_Alpha;
	int GBb_Dst_Beta;
	int GBb_Dst_Gammar;
} GB_InputData;

typedef struct{
	int GBa_Dst_J1;
	int GBa_Dst_J2;
	int GBa_Dst_J3;
	int GBa_Dst_J4;
	int GBa_Dst_J5;
	int GBa_Dst_J6;
	int GBa_Dst_J7;
	int GBb_Dst_J1;
	int GBb_Dst_J2;
	int GBb_Dst_J3;
	int GBb_Dst_J4;
	int GBb_Dst_J5;
	int GBb_Dst_J6;
	int GBb_Dst_J7;
} GB_OutputData;

typedef struct{
    void* rp_1; 
    void* rp_2; 
    void* wp_1; 
    void* wp_2; 
    pthread_mutex_t r_mtx_1;
    pthread_mutex_t r_mtx_2;
    pthread_mutex_t w_mtx_1;
    pthread_mutex_t w_mtx_2;
}rl_buffer;



extern void rl(rl_buffer* rl_buf);

typedef struct{
	char *input_data;
	char *output_data;
	int input__size;
	int output_size;
}img_detection;

extern void image_detection(img_detection *img_det);

//#endif
