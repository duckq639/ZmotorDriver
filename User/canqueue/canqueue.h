#ifndef CANQUEUE_H
#define CANQUEUE_H
/*============================ 宏定义区 ============================*/
#define ZMOTOR_STD_DATASIZE 5 // 数据段DLC
#define ZMOTOR_CMD_DATASIZE 1 // 请求段DLC,只包含命令
#define CAN_QUEUE_MAX_SIZE 15

#define CAN_STD(id) (id)
#define CAN_EXT(id) (id | 0x80000000u)
/*============================ 头文件包含区 ============================*/
#include "stdint.h"
#include "stdbool.h"
#include "string.h"
/*============================ 数据结构区 ============================*/
typedef struct
{
    uint32_t StdId;
    uint32_t ExtId;
    uint8_t IDE;
    uint8_t RTR;
    uint8_t DLC;
    uint8_t Data[ZMOTOR_STD_DATASIZE];
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
CANFrame CAN_Creat_Frame(uint32_t ID, uint8_t IDE, uint8_t DLC);
int CAN_Queue_Init(CANQueue *queue); // 初始化
void CAN_Queue_System_Init();

bool isQueue_Empty(CANQueue *queue);
bool isQueue_Full(CANQueue *queue);
int Queue_Pop(CANQueue *queue, CANFrame *frame); // 队列核心
int Queue_Push(CANQueue *queue, CANFrame *frame);

#endif // !CANQUEUE_H
