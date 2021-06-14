#ifndef _COMMON_H_
#define _COMMON_H_

#include<signal.h>
#include<sys/wait.h>
#include<time.h>
#include<string.h>
#include<iostream>
#include<mutex>
#include<sys/shm.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<iostream>
#include<vector>

using namespace std;
/* 
*   定义1H8和PS的状态结构体 ，以及一些define定义
*/
#define EXCEPTION_COUNT        32            /* 异常的数量 */
#define INFORTMATION_BIT       32            /* 异常类中的information的大小 */
#define WCET_TIME              5             /*最长等待异常处理时间 */
#define INTERACTIVE_TIME       5             /* 交互线程轮训的秒数 */
#define MAX_HANDLE_COUNT       3             /* 最大处理异常的次数 */


#define TRANSACTION_RECOVER
#define TRANSACTION_ID        1             //业务进程的异常代码
#define TRANSACTION_OVERTIME  15            //业务端超时时间（秒）


#define MYFIFO			      "/tmp/myfifo"    //有名管道名       
#define MYFIFO_SEND           "/tmp/myfifo_send"

 /* 4个1H8异常的代码 */
#define FIRST_1H8_ID           11           
#define SECOND_1H8_ID          12
#define THIRD_1H8_ID           13
#define FOURTH_1H8_ID          14


#define HEART_THREAD_TIME      3              //心跳线程轮训时间
#define INTERACT_THREAD_TIME   2              //交互线程轮训的时间 
#define NO_EXCEPTION           0              //有异常
#define YES_EXCEPTION          1              //无异常
#define CPU_CHECH_TIME         1000000        //CPU使用率检测时间（微秒）
#define SYSTEM_CHECK_TIME      4              //系统检测轮训时间
#define OVERTIME_1H8           15             //1H8超时时间
#define MAX_RECOVER_TIME_1H8   25             //1H8最大等待异常恢复时间
#define MAX_RECOVER_TIME_TRAN  8              //业务进程最大等待异常恢复时间

#define QUEUE    20

//测温度相关
#define  PL_MAP_DEV_NAME  "/dev/mem"
// these Address was assigned in Vivado block design
#define SYSMON_CTRL_BASE  (0x00A0000000)  // system_mgmt control registers base addr (AXI-Lite port)
#define SYSMON_CTRL_SIZE  (      0x2000)


//测网速的网口
#define INTERFACE         "eth0:"

//1H8个数
#define COUNT_1H8           4                

//PS端串口交互
#if 1
#define PORT              "/dev/ttyUSB0"    // 串口号
#define BAUDRATE          115200          // 波特率
#define DATABITS          8                // 数据位
#define PARITY            'O'              // 奇偶校验位
#define STOPBIT           1                // 停止位
#endif

#if 0
#define PORT              "/dev/ttyPS3"    // 串口号
#define BAUDRATE          115200          // 波特率
#define DATABITS          8                // 数据位
#define PARITY            'O'              // 奇偶校验位
#define STOPBIT           1                // 停止位
#endif
//服务端socket信息
#define BUF_SIZE 1024

//服务端最大监听长度
#define queue_len 10    
//任务队列
typedef struct
{
    vector<unsigned char> command_buffer;
}command_buf;

#endif
