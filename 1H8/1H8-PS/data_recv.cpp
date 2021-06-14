#include "data_recv.h"


DataRecv::DataRecv()
{
    recv_buffer = (char *)malloc(MAX_PACKET_LEN*sizeof(char));    
    buffer_setup();   // init lvds image recv buffer
    buffer_reset();   // 停止接收并清空图像缓存
	buffer_self_test(1); // 自测，使用片内的LVDS源
    buffer_start();   // 开始接收
}

DataRecv::~DataRecv()
{
    free(recv_buffer);
    buffer_destroy();       
}

int DataRecv::buffer_setup(){
    PLFD = open(PL_MAP_DEV_NAME, O_RDWR|O_SYNC );
    PLFD = open(PL_MAP_DEV_NAME, O_RDWR|O_SYNC );
    
	if (PLFD == -1){
		puts("  *** open " PL_MAP_DEV_NAME " failed!");
		return -1;
	}
    
    bufferCtrlMapAddr = (unsigned*)mmap(0, BUFFER_CTRL_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, PLFD, BUFFER_CTRL_BASE );
    if (bufferCtrlMapAddr==NULL) {
        close( PLFD );
        puts("  ***  mmap failed at bufferCtrlMapAddr!" );
        return -1;
    }

    bufferMapAddr = (unsigned long long*)mmap(0, BUFFER_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, PLFD, BUFFER_BASE );
    if (bufferMapAddr==NULL) {
        munmap( bufferCtrlMapAddr, BUFFER_CTRL_SIZE );
        close( PLFD );
        puts("  ***  mmap failed at bufferMapAddr!" );
        return -1;
    }

    return 0;
}

void DataRecv::buffer_self_test(int on){
    // if(on)
    //     bufferCtrlMapAddr[1] = 0x00000001;
    // else
    //     bufferCtrlMapAddr[1] = 0x00000000;
}

void DataRecv::buffer_reset(){
    // bufferCtrlMapAddr[0] = 0x00000000;    // close fifo
    // usleep(10);
    // bufferCtrlMapAddr[2] = MAX_PACKET_LEN;
    // bufferCtrlMapAddr[4] = BUFFER_BASE & 0xffffffff;
    // bufferCtrlMapAddr[5] = BUFFER_BASE>>32;
    // bufferCtrlMapAddr[6] = BUFFER_SIZE - 1;
    usleep(10);
}

void DataRecv::buffer_start(){
    // bufferCtrlMapAddr[0] = 0x00000001;    // open fifo
}

void DataRecv::buffer_pause(){
    bufferCtrlMapAddr[0] = 0x00000000;
}

int DataRecv::buffer_full(){
    return !(bufferCtrlMapAddr[24] & 0x01);
}

int DataRecv::buffer_empty(){
    return !bufferCtrlMapAddr[9];
}

unsigned DataRecv::buffer_recv(char* recv_buffer, unsigned char *id, unsigned char *user){
    unsigned i, base, base_u64, len, len_u64;
    unsigned long long * recv_buffer_u64 = (unsigned long long *)recv_buffer;
    while( buffer_empty() );        // wait for buffer available
    base = bufferCtrlMapAddr[12];
    len  = bufferCtrlMapAddr[14];
	if(id!=NULL)
		*id  = bufferCtrlMapAddr[10];
	if(user!=NULL)		
		*user= bufferCtrlMapAddr[11];
    base_u64 = base / sizeof(unsigned long long);
    len_u64  = len  / sizeof(unsigned long long);
    if(len > MAX_PACKET_LEN)
        return 0;
    for(i=0; i<len_u64; i++)
        recv_buffer_u64[i] = bufferMapAddr[base_u64+i];
    bufferCtrlMapAddr[8] = 0x00000001;   // buffer shift
    return len;
}

void DataRecv::buffer_destroy(){
    buffer_reset();
	buffer_self_test(0);
    munmap( bufferCtrlMapAddr, BUFFER_CTRL_SIZE );
    munmap( bufferMapAddr, BUFFER_SIZE );
    close( PLFD );
}

void DataRecv::print_map_status(){
    printf("bufferCtrlMapAddr=%p\n", bufferCtrlMapAddr );
    printf("bufferMapAddr    =%p\n", bufferMapAddr );
}

void DataRecv::print_buffer_status(){
    printf("rptr:%08x   wptr:%08x", bufferCtrlMapAddr[16], bufferCtrlMapAddr[20] );
    if( buffer_full() )
        printf("   full!");
    else if( buffer_empty() )
        printf("  empty!");
    else
        printf("        ");
    printf("   ");
    //printf("\n");
}
