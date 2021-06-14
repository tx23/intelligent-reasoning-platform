#ifndef _TASK_SCHEDULING_TYPE_H_
#define _TASK_SCHEDULING_TYPE_H_

#include <vector>
#include <string>
#include <map>

using namespace std;

typedef enum
{
	EM_COMMON_TYPE_GET_IMAGE     = 0x01, 
	EM_COMMON_TYPE_DETECT_RESULT = 0x02,	
	EM_COMMON_TYPE_SYS_INFO      = 0x03, 
}EnumCommonType,*lpEnumCommonType;


typedef struct 
{	
	int  image_label;//识别物体的类标号
        float  x1;//坐标
        float  y1;
        float  x2;
        float  y2;
        float  x3;
        float  y3;
        float  x4;
        float  y4;
        double  time;
}detect_out;

typedef struct 
{	
	EnumCommonType	emCommand;
	int 			iDatalen;
        	
}DATA_HEAD;

#endif //_ST_VENC_H_
