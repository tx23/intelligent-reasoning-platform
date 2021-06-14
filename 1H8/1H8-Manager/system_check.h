#ifndef _SYSTEM_CHECKING_H
#define _SYSTEM_CHECKING_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <unistd.h>	
#include <stdio.h>
#include <stdlib.h>
#include <string.h>	
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <mntent.h>

#include <sys/vfs.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include "common.h"

struct status_1h8
{
	float	cpu_use;				/* CPU使用率 */
	float	cpu_load;				/* CPU负载  */
	float	memory_use;				/* 内存使用率 */
	float	disk_use;				/* 磁盘使用率 */
    float   net_speed;              /* 网络速度 */
	float   temperature;            /* 1H8温度 */
};

//测网速的网口
#define INTERFACE            "eth0:"

using namespace std;

class SystemCheck{
    private:
        float get_memory_use();
        float get_cpu_load();
        float get_cpu_use();
        bool  get_cpu_info(int *cpu_info);
        float get_net_speed();
        int get_temp();
        int GetSendRecv_bytes(string interface,long &recv_byte,long &send_byte);
        float get_disk_use();

        long recv_byte_pre,send_byte_pre;
        long recv_byte_now,send_byte_now;
        struct timeval t_now,t_pre;
        int cpu_info1[10],cpu_info2[10];

        
    public:
        SystemCheck();
		void init();
        /* 检测系统状态  */
        struct status_1h8 status;
        int system_status_checking();             
        /* 打印状态信息（测试） */
        int print_system_status();   
};

#endif
