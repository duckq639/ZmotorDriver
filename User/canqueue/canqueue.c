#include "canqueue.h"

/*-------------------------------------结构体定义--------------------------------------------------------*/
CANQueue CAN_TxQueue;
CANQueue CAN_RxQueue;
/*--------------------------------------------函数定义--------------------------------------------------------*/
/*------------队列初始化函数---------------*/
int CAN_Frame_Init(CANFrame *frame)
{
    frame->StdId = 0x1;
    frame->ExtId = 0x0;
    frame->IDE = 0x0;
    frame->RTR = 0x0;
    frame->DLC = ZMOTOR_STD_DATASIZE;
    memset(frame->Data, 0, sizeof(frame->Data));
    return 0;
}

CANFrame CAN_Creat_Frame(uint32_t ID, uint8_t IDE, uint8_t DLC)
{
    CANFrame tempframe;
    CAN_Frame_Init(&tempframe); // can_frame需要频繁创建,所以封装了一个创建函数
    tempframe.IDE = IDE;
    if (IDE)
    {
        tempframe.ExtId = ID;
    }
    else
    {
        tempframe.StdId = ID;
    }
    tempframe.DLC = DLC;
    return tempframe;
}
int CAN_Queue_Init(CANQueue *queue)
{
    queue->QueueHead = 0;
    queue->QueueTail = 0;
    queue->QueueSize = 0;
    memset(queue->CANTxQueueElm, 0, sizeof(queue->CANTxQueueElm));
    return 0;
}
void CAN_Queue_System_Init()
{
    CAN_Queue_Init(&CAN_TxQueue);
    CAN_Queue_Init(&CAN_RxQueue);
}
/*------------队列核心函数---------------*/

bool isQueue_Empty(CANQueue *queue)
{
    if (queue->QueueSize == 0)
        return true;
    return false;
}

bool isQueue_Full(CANQueue *queue)
{
    if (queue->QueueSize == CAN_QUEUE_MAX_SIZE)
        return true;
    return false;
}

int Queue_Pop(CANQueue *queue, CANFrame *frame)
{
    if (!isQueue_Empty(queue))
    {
        memcpy(frame, &queue->CANTxQueueElm[queue->QueueHead], sizeof(CANFrame));
        queue->QueueHead = (queue->QueueHead + 1) % CAN_QUEUE_MAX_SIZE;
        queue->QueueSize--;
        return 0;
    }
    else
    {
        return 1;
    }
}
int Queue_Push(CANQueue *queue, CANFrame *frame)
{
    if (!isQueue_Full(queue))
    {
        memcpy(&queue->CANTxQueueElm[queue->QueueTail], frame, sizeof(CANFrame));
        queue->QueueTail = (queue->QueueTail + 1) % CAN_QUEUE_MAX_SIZE;
        queue->QueueSize++;
        return 0;
    }
    else
    {
        return 1;
    }
}

/*------------集和函数---------------*/
