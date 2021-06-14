#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

#define  PL_MAP_DEV_NAME  "/dev/mem"

// these Address was assigned in Vivado block design
#define BUFFER_CTRL_BASE  (0x00A0000000ULL)  // s_axilite control registers base addr (AXI-Lite port)
#define BUFFER_CTRL_SIZE  (      0x0100ULL)

#define BUFFER_BASE       (0x0500000000ULL)  // ddr3 control registers base addr (AXI4 port)
#define BUFFER_SIZE       (0x00F0000000ULL)

#define MAX_PACKET_LEN    (2048 * 2048 + 256)



class DataRecv
{
    public:
    DataRecv();
    ~DataRecv();
    
    int buffer_setup();
	void buffer_self_test(int on);
    void buffer_reset();
    void buffer_start();
    void buffer_pause();
    int buffer_full();
    int buffer_empty();
    unsigned buffer_recv(char* recv_buffer, unsigned char *id, unsigned char *user);
    void buffer_destroy();
    void print_map_status();
    void print_buffer_status();
    unsigned long long total_bytes = 0;
    unsigned time_start = 0;
    char* recv_buffer;
    
    private: 
    int PLFD = 0;
    unsigned * bufferCtrlMapAddr = NULL;
    unsigned long long * bufferMapAddr = NULL;
}; 
