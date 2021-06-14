#include "system_check.h"


SystemCheck::SystemCheck(){  
    status.cpu_use     = 0;
    status.cpu_load    = 0;
    status.memory_use  = 0;
    status.disk_use    = 0;
    status.net_speed   = 0;
    status.temperature = 0;
}


void SystemCheck::init()
{
    gettimeofday(&t_pre,NULL);
    GetSendRecv_bytes(INTERFACE,recv_byte_pre,send_byte_pre);
    get_cpu_info(cpu_info1);                     
}



int SystemCheck::system_status_checking(){
    this->status.cpu_use     = get_cpu_use();
    this->status.cpu_load    = get_cpu_load();
    this->status.memory_use  = get_memory_use();
    this->status.disk_use    = get_disk_use();
    this->status.net_speed   = get_net_speed();    
    this->status.temperature   = get_temp();    
    return 0;
}

int SystemCheck::print_system_status(){
    cout<<"----------------------------------.-----------"<<endl;
    cout<<"system checking "<<endl;    
    cout<<"CPU load :"   <<status.cpu_load <<endl;
    cout<<"CPU use :"    <<status.cpu_use<< "%" <<endl;
    cout<<"Memory use :" <<status.memory_use<< "%" <<endl;
    cout<<"Disk_use :"   <<status.disk_use<< "%" <<endl;   
    cout<<"network_speed :"<<status.net_speed << " KB/S" <<endl;    
    cout<<"temperature :"<<status.temperature << " C" <<endl;    
    cout<<"----------------------------------.-----------"<<endl;
}

float SystemCheck::get_disk_use()
{
    float disk_use = 0.0;
    string filesystem,used,mounted_on;
    unsigned long avail,size,dused;
    unsigned long total_size = 0;//磁盘总容量
    unsigned long total_used = 0;//磁盘总使用量
    FILE *fstream = NULL;
    char buffer[256];
    memset(buffer,0,sizeof(buffer));

    if(NULL == (fstream = popen("df -m","r")))
    {
        printf("popen error !!!\n");
        return -1;
    }
    while(fgets(buffer,sizeof(buffer)-1,fstream) != NULL)
    {
        stringstream ss;
        ss<<buffer;
        ss>>filesystem>>size>>dused>>avail>>used>>mounted_on;
        total_size += size;
        total_used += dused;
        //printf("%s",buffer);
        //printf("%u %u\n",size,dused);
    }
    pclose(fstream);
    disk_use = (total_used * 1.0) / total_size;
    return disk_use * 100; 
}

 /* 获取内存信息 */
float SystemCheck::get_memory_use(){      
    float memory_use = 0;
    int memory_info[3];
    fstream meminfo;
    char temp[20];
    int i;
    /*  读取linux系统的文件 */
    meminfo.open("/proc/meminfo",ios::in);    
    for(i = 0;i<3;i++){
        meminfo >> temp;
        meminfo >> memory_info[i];
        meminfo >>temp;
    }
    
    memory_use = (float)(memory_info[0]-memory_info[1])/memory_info[0];

    return memory_use*100;
}


//通过系统信息得到网速
int SystemCheck::GetSendRecv_bytes(string interface,long &recv_byte,long &send_byte)
{
    string buffer;
    string temp;
    long data[12];
    ifstream fin("/proc/net/dev",ios::in);
    if(!fin)
    {
        printf("%s can't open !!!\n","/proc/net/dev");
        return -1;
    }
    getline(fin,buffer);
    while(getline(fin,buffer))
    {
        fin>>temp>>data[0]>>data[1]>>data[2]>>data[3]>>data[4]>>data[5]>>data[6]>>data[7]>>data[8]>>data[9]>>data[10]>>data[11];
        //cout<<temp<<" "<<data[0]<<" "<<data[1]<<" "<<data[2]<<endl;
        if(temp == interface)
        {
            recv_byte = data[0];
            send_byte = data[8];
            break;
        }
    }

    fin.close();
    return 0;
}

// 网络检测 单位是KB/s  现在得到是send_rate,也可以得到recv_rate
float SystemCheck::get_net_speed()
{
    float recv_rate = 0.0;
    float send_rate = 0.0;

//    usleep(1000*1000);
    gettimeofday(&t_now,NULL);
    GetSendRecv_bytes(INTERFACE,recv_byte_now,send_byte_now);

    float time_ = (t_now.tv_sec+t_now.tv_usec*0.000001)-(t_pre.tv_sec+t_pre.tv_usec*0.000001);
    recv_rate = (recv_byte_now - recv_byte_pre) / time_ /1024;
    send_rate = (send_byte_now - send_byte_pre) / time_ /1024;

	memcpy((char *)&t_pre, (char *)&t_now, sizeof(t_now));
	recv_byte_pre = recv_byte_now;
	send_byte_pre = send_byte_now;

    return send_rate < recv_rate ? recv_rate : send_rate;
}

bool SystemCheck::get_cpu_info(int *cpu_info){     //从/proc中读取CPU的状态信息
    fstream stat_file;
    char temp[20];
    int i;
    stat_file.open("/proc/stat",ios::in);
    stat_file >> temp;
    for(i=0;i<10;i++){
        stat_file >> cpu_info[i];
    }
    stat_file.close();
    return 1;
}

/* 计算CPU的利用率 */
float SystemCheck::get_cpu_use(){              
    float cpu_use = 0;
    int i;
    unsigned int total_time1 = 0,total_time2 = 0;
    /*  两次读取CPU的时间信息，计算CPU的使用率  */
//    usleep(1000*1000);
    get_cpu_info(cpu_info2);
    for(i = 0; i<10; i++){
        total_time1 = total_time1 + cpu_info1[i];
        total_time2 = total_time2 + cpu_info2[i];
    }
    cpu_use=(float)(total_time2-total_time1-(cpu_info2[3]-cpu_info1[3]))/(total_time2-total_time1);

	memcpy(cpu_info1, cpu_info2, sizeof(cpu_info2));

    return cpu_use*100;
}

/*  获得CPU负载信息    */
float SystemCheck::get_cpu_load(){               
    float cpu_load = 0;
    fstream load_file;  
    /* 读取linux系统的CPU负载信息 */ 
    load_file.open("/proc/loadavg",ios::in);    
    load_file >> cpu_load;
    load_file.close();
    return cpu_load;
}

int SystemCheck::get_temp(){
    char temp[10];
	fstream load_temp;
	load_temp.open("/sys/devices/system/cpu/cpufreq/temp_out", ios::in);
    load_temp >> temp;
    int t = (temp[5] - '0')*10 + (temp[6] - '0');
	load_temp.close();
	return t;
}
