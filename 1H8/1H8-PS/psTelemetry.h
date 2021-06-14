#ifndef _PS_TELEMETRY_H_
#define _PS_TELEMETRY_H_

//#pragma once
#include <vector>
#include <pthread.h>
#include "common.h"
#include <queue>

#define BUFSZ_32 (32)

using namespace std;
//5Byte
typedef struct selfhead{
    unsigned char head[4] = {0x1a,0xcf,0xfc,0x1d};//0x1a,0xcf,0xfc,0x1d
    unsigned char apid;//0x5x
}selfhead;
//6Byte
typedef struct mainHead {
    char head;//0x01
    char apid;//0x5x
    char seqlabel;//0xc0
    char seqcount;//0x00
#if 0
    uint8_t version:3;
    uint8_t type:1;
    uint8_t subhead:1;
    uint16_t apid:11;
    uint16_t seqlabel:2;
    uint16_t seqcount:14;
#endif
    uint16_t dataLength;
} mainHead;

typedef struct psInfo {
    uint8_t cpuRatio;
    uint8_t cpuPayload;
    uint8_t memRatio;
    uint8_t temperature;
    uint8_t netSpeed[3];
    uint8_t psStatus;
} psInfo;

typedef struct h8Info {
    uint8_t h8Ratio;
    uint8_t h8Payload;
    uint8_t h8MemRatio;
    uint8_t h8DiskRatio;
    uint8_t h8NetSpeed[3];
    uint8_t h8Status;
} h8Info;

typedef struct normalTele {
    mainHead mHead;
    psInfo pInfo;
    h8Info hInfo[4];
    //psModeSetting mSetting;
    uint8_t modeStatus; //on or off
} normalTele;

typedef struct updateTele {
    mainHead mHead;
    uint8_t processVersion[4];
    uint8_t modeNum[4];
    uint8_t lastCrcLabel;
    uint8_t lastCrcSeq;
    uint8_t segCorrectCount;
    uint8_t segErrorCount;
    uint8_t lastFileType;
    uint8_t lastFileLabel;
    uint8_t fileCorrectCount;
    uint8_t fileErrorCount;
} updateTele;

//object detect effective data
typedef struct picObjBase { //17Byte
    uint8_t objType; //A0,A1,A2,A3
    uint16_t zsX; //top left
    uint16_t zsY;
    uint16_t ysX; //top right
    uint16_t ysY;
    uint16_t zxX; //bottom left
    uint16_t zxY;
    uint16_t yxX; //bottom right
    uint16_t yxY;
} picObjBase;

typedef struct picObjResult {
    uint8_t frameLabel; //B5    1Byte
    uint8_t frameType; //0A,1A,00   1Byte
    uint8_t picId[8];     //8Byte
    uint8_t modelNum;    //1Byte
    uint8_t labelNum;    //1Byte
    uint8_t frameSeq;    //1Byte
    picObjBase oBase[6]; //max label num 102Byte
    uint8_t reserved[8]; //  8Byte
} picObjResult;
//pose effective data
typedef struct picPoseBase {
    int posX;
    int posY;
    int posZ;
} picPoseBase;

//25byte
typedef struct picPoseResult {
    uint8_t frameLabel; //B5
    uint8_t frameType; //0A,1A
    uint16_t picId;
    uint8_t modelNum;
    uint8_t labelNum;
    picPoseBase pBase[6]; //max label num
    uint8_t reserved[8];
} picPoseResult;
/*
typedef struct
{
	pthread_mutex_t sm_mutex;//互斥锁
	int buf[14];//存放的数据
    //buf[8]~buf[12]  
	pthread_mutex_t pic_mutex;//互斥锁
    picPoseResult picObj[5];
} sm_msg;
*/
//extern sm_msg *msg;
//图像处理
typedef struct picResultTele {
    selfhead sHead;//5Byte
    mainHead mHead;//6Byte
    picObjResult oRes;
    //picPoseResult pRes;
} picResultTele;

typedef struct codecTele {
    mainHead mHead;
    uint8_t curMode;
    uint8_t frameExtractRatio;
    uint16_t picSource;
    uint16_t recieveFrameCount;
    uint16_t decodeFrameCount;
    uint32_t sendFrameCount;
    uint8_t instructionCount;
    uint16_t lastCode; //1B,2B
    uint8_t errorInstructionCount;
    uint8_t errorInstruction[80];
} codecTele;

typedef struct reinforceBase {
    uint16_t gxbAStatus[7];
    uint16_t gxbBStatus[7];

    uint16_t gxbAPlan[7];
    uint16_t reserved1[2];

    uint16_t gxbBPlan[7];
    uint16_t reserved2[2];

    uint16_t gxbADistance[3]; //XYZ
    uint16_t gxbAPosture[3]; //XYZ
    uint16_t reserved3[2];

    uint16_t gxbBDistance[3]; //XYZ
    uint16_t gxbBPosture[3]; //XYZ
    uint16_t reserved4[2];
} reinforceBase;

typedef struct reinforceTele {
    mainHead mHead;
    uint8_t runStatus; //5A: selfcheck 5C:run 5E:over 54:error
    reinforceBase rBase;
} reinforceTele;

class psTelemetry
{
    public:
        psTelemetry();
        ~psTelemetry();

    public:
        int readShmData(char *addr, char *buf, int len);
        int getNormalTele(normalTele *nTele);
        int getPicResultTele(picResultTele *pTele);
        int getReinForceTele(reinforceTele *rTele);
        int getCodecTele(codecTele *cTele);
        int setMainHeadApid(mainHead mHead, unsigned char apid);

    public:
        char *shmAddr; //shared memory address
        pthread_mutex_t shmMutex;
};

#endif
