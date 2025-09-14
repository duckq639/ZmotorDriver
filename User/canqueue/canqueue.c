#include "canqueue.h"

/*-------------------------------------结构体定义--------------------------------------------------------*/
CANQueue CAN_TxQueue = {0};
/*--------------------------------------------函数定义--------------------------------------------------------*/
/*------------队列初始化函数---------------*/
// 暂时不需要了喵
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
