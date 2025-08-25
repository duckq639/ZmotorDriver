#ifndef CANQUEUE_H
#define CANQUEUE_H
/*============================ 宏定义区 ============================*/
#define CAN_STD_DATASIZE 5
#define CAN_CMD_DATASIZE 1
#define CAN_QUEUE_MAX_SIZE 15
/*============================ 头文件包含区 ============================*/
#include "stdint.h"
#include "stdbool.h"
#include "string.h"
#include "can.h"
#include "zmotor.h"
/*============================ 数据结构区 ============================*/
typedef struct
{
    uint32_t StdId;
    uint8_t IDE;
    uint8_t RTR;
    uint8_t DLC;
    uint8_t Data[CAN_STD_DATASIZE];
} CANFrame;

typedef struct
{
    uint32_t QueueHead;
    uint32_t QueueTail;
    uint32_t QueueSize;
    CANFrame CANTxQueueElm[CAN_QUEUE_MAX_SIZE];
} CANQueue;

/*============================ 常量定义区 ============================*/
/*============================ 类型定义区 ============================*/
extern CANQueue CAN_TxQueue;
extern CANQueue CAN_RxQueue;
/*============================ 函数声明区 ============================*/
int CAN_Frame_Init(CANFrame *frame);
CANFrame CAN_Creat_Frame();
int CAN_Queue_Init(CANQueue *queue); // 初始化
void CAN_Queue_System_Init();

bool isQueue_Empty(CANQueue *queue);
bool isQueue_Full(CANQueue *queue);
int Queue_Pop(CANQueue *queue, CANFrame *frame); // 队列核心
int Queue_Push(CANQueue *queue, CANFrame *frame);

int CAN_Write_Cmd(MotorTxCMD cmd, float data, uint32_t motorID, uint32_t datalength);
int CAN_Send_Cmd();
int CAN_Recieve_Cmd(CAN_RxHeaderTypeDef *hdr, uint8_t *data); // 命令中间层和底层
int CAN_Read_Cmd(MotorPtr motorp);

CAN_TxHeaderTypeDef CAN_Frame_ToTxHeader(CANFrame *frame); // 转换工具
CANFrame CAN_TxHeader_ToFrame(CAN_RxHeaderTypeDef *hdr);

int Motor_Set_Mode(MotorPtr motorp, MotorMode mode); // 命令封装
int Motor_Set_Value(MotorPtr motorp, MotorTxCMD cmd, float value);
int Motor_Request_Data(MotorPtr motorp, MotorTxCMD cmd);

void Motor_Func(MotorPtr motorp); // 集成封装
#endif                            // !CANQUEUE_H
