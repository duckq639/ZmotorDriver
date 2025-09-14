#ifndef CANQUEUE_H
#define CANQUEUE_H
/*============================ 宏定义区 ============================*/
#define CAN_QUEUE_MAX_SIZE 15

/*============================ 头文件包含区 ============================*/
#include "can.h"
#include "string.h"
#include "stdbool.h"
/*============================ 数据结构区 ============================*/
typedef struct
{
    CAN_TxHeaderTypeDef CAN_TxHeader;
    uint8_t Data[8];
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
/*============================ 函数声明区 ============================*/

bool isQueue_Empty(CANQueue *queue);
bool isQueue_Full(CANQueue *queue);
int Queue_Pop(CANQueue *queue, CANFrame *frame); // 队列核心
int Queue_Push(CANQueue *queue, CANFrame *frame);

#endif // !CANQUEUE_H
