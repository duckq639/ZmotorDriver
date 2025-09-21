#include "canqueue.h"

/*-------------------------------------结构体定义--------------------------------------------------------*/
CANQueue CAN1_TxQueue = {0};
CANQueue CAN2_TxQueue = {0};
/*--------------------------------------------函数定义--------------------------------------------------------*/
/*------------队列初始化函数---------------*/
// 暂时不需要了喵
/*------------队列核心函数---------------*/

bool isCAN_Queue_Empty(CANQueue *queue)
{
    if (queue->QueueSize == 0)
        return true;
    return false;
}

bool isCAN_Queue_Full(CANQueue *queue)
{
    if (queue->QueueSize == CAN_QUEUE_MAX_SIZE)
        return true;
    return false;
}

int CAN_Queue_Pop(CANQueue *queue, CAN_TxFrame *frame)
{
    if (!isCAN_Queue_Empty(queue))
    {
        memcpy(frame, &queue->CANTxQueueElm[queue->QueueHead], sizeof(CAN_TxFrame));
        queue->QueueHead = (queue->QueueHead + 1) % CAN_QUEUE_MAX_SIZE;
        queue->QueueSize--;
        return 0;
    }
    else
    {
        return 1;
    }
}
int CAN_Queue_Push(CANQueue *queue, CAN_TxFrame *frame)
{
    if (!isCAN_Queue_Full(queue))
    {
        memcpy(&queue->CANTxQueueElm[queue->QueueTail], frame, sizeof(CAN_TxFrame));
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
