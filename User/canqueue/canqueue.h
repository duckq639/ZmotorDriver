#ifndef CANQUEUE_H
#define CANQUEUE_H
/*============================ 宏定义区 ============================*/
#define CAN_QUEUE_MAX_SIZE 15

/*============================ 头文件包含区 ============================*/
#include "can.h"
#include "string.h"
#include "stdbool.h"
/*============================ 数据结构区 ============================*/
typedef enum
{
    CAN_1 = 0x1u,
    CAN_2 = 0x2u,
}CANCHANNLE;

typedef struct
{
    uint8_t CAN_X;
    CAN_TxHeaderTypeDef CAN_TxHeader;
    uint8_t Data[8];
} CAN_TxFrame;

typedef struct
{
    uint8_t CAN_X;
    CAN_RxHeaderTypeDef CAN_RxHeader;
    uint8_t Data[8];
} CAN_RxFrame;

typedef struct
{
    uint32_t QueueHead;
    uint32_t QueueTail;
    uint32_t QueueSize;
    CAN_TxFrame CANTxQueueElm[CAN_QUEUE_MAX_SIZE];
} CANQueue;

/*============================ 常量定义区 ============================*/
/*============================ 类型定义区 ============================*/
extern CANQueue CAN1_TxQueue;
extern CANQueue CAN2_TxQueue;
/*============================ 函数声明区 ============================*/

bool isCAN_Queue_Empty(CANQueue *queue);
bool isCAN_Queue_Full(CANQueue *queue);
int CAN_Queue_Pop(CANQueue *queue, CAN_TxFrame *frame); // 队列核心
int CAN_Queue_Push(CANQueue *queue, CAN_TxFrame *frame);

#endif // !CANQUEUE_H
