#include <string>
#include <map>
#include <vector>

#include "psTelemetry.h"

#ifndef TASK_DEF
#define TASK_DEF

#define  SERVER_TIME_OUT 5


using namespace std;
/*
typedef struct{
    void* rp_1; 
    void* rp_2; 
    void* wp_1; 
    void* wp_2; 
    pthread_mutex_t r_mtx_1;
    pthread_mutex_t r_mtx_2;
    pthread_mutex_t w_mtx_1;
    pthread_mutex_t w_mtx_2;
} rl_buffer;
*/

typedef struct
{
	float   cpu_use;                /* CPU使用率 */
    float   cpu_load;               /* CPU负载  */
	float   memory_use;             /* 内存使用率 */
	float   disk_use;               /* 磁盘使用率 */
	float   net_speed;              /* 网络速度 */
	float   temperature;            /* 1H8温度 */
} status_1h8;


typedef struct
{
	pthread_mutex_t sm_mutex;//互斥锁
	int buf[50];//存放的数据
	//buf[8] //环形数组front,指向第一个元素
	//buf[9] //环形数组end,指向最后一个元素的后一个位置
	//buf[13]~buf[16] //四块1H8的网络状态
	//buf[17],buf[18] //强化学习开始结束标志位、自检标志位
	pthread_mutex_t pic_mutex;
	picObjResult picObj[20];
	status_1h8 status[4]; //状态信息
    //强化学习数据缓冲区
    pthread_mutex_t ref_mutex;
    int rp1[26];
    int rp2[26];
    int wp1[13];
    int wp2[13];
} sm_msg;

typedef struct 
{	
	int  iPictureLen;
	void *iPictureData;
    int number;		
} PictureInfor;

typedef enum
{
	EM_COMMON_TYPE_GET_IMAGE     = 0x01, 
	EM_COMMON_TYPE_DETECT_RESULT = 0x02,	
	EM_COMMON_TYPE_SYS_INFO      = 0x03,	
} EnumCommonType,*lpEnumCommonType;
	
typedef struct 
{	
	EnumCommonType	emCommand;
	int 			iDatalen;	
} DATA_HEAD;

typedef struct 
{	
    int    image_label;//识别物体的类标号
    float  x1;//坐标
    float  y1;
    float  x2;
    float  y2;
    float  x3;
    float  y3;
    float  x4;
    float  y4;
}detect_out;
#endif

