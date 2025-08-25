#include "canqueue.h"

/*-------------------------------------结构体定义--------------------------------------------------------*/
CANQueue CAN_TxQueue;
CANQueue CAN_RxQueue;
/*--------------------------------------------函数定义--------------------------------------------------------*/
/*------------队列初始化函数---------------*/
int CAN_Frame_Init(CANFrame *frame)
{
    frame->StdId = 0x1;
    frame->IDE = 0x0;
    frame->RTR = 0x0;
    frame->DLC = CAN_STD_DATASIZE;
    memset(frame->Data, 0, sizeof(frame->Data));
    return 0;
}

CANFrame CAN_Creat_Frame()
{
    CANFrame tempframe;
    CAN_Frame_Init(&tempframe); // can_frame需要频繁创建,所以封装了一个创建函数
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
/*------------命令运行函数---------------*/
int CAN_Write_Cmd(MotorTxCMD cmd, float data, uint32_t motorID, uint32_t datalength)
{
    CANFrame frame = CAN_Creat_Frame();
    frame.Data[0] = cmd;
    if (datalength == 5)
        memcpy(frame.Data + 1, &data, sizeof(float)); // 命令传输中间层
    Queue_Push(&CAN_TxQueue, &frame);
    return 0;
}

int CAN_Send_Cmd()
{
    uint32_t mailbox;
    CANFrame frame = CAN_Creat_Frame();
    if (Queue_Pop(&CAN_TxQueue, &frame) == HAL_OK) // 出队发送,使用addtxmessage + 固定间隔发送,命令实现层
    {
        CAN_TxHeaderTypeDef CAN_TxHeader = CAN_Frame_ToTxHeader(&frame);
        return HAL_CAN_AddTxMessage(&hcan1, &CAN_TxHeader, frame.Data, &mailbox);
    }
    else
    {
        return 1;
    }
}

int CAN_Recieve_Cmd(CAN_RxHeaderTypeDef *hdr, uint8_t *data)
{
    CANFrame frame = CAN_TxHeader_ToFrame(hdr); // 将函数放入CAN FIFO接收中断,负责将反馈的消息存入接收队列
    memcpy(frame.Data, data, CAN_STD_DATASIZE * sizeof(uint8_t));
    if (Queue_Push(&CAN_RxQueue, &frame))
        return 0;
    else
        return 1;
}
int CAN_Read_Cmd(MotorPtr motorp) // 指令解包
{
    CANFrame frame = CAN_Creat_Frame();
    float temp = 0;
    if (Queue_Pop(&CAN_RxQueue, &frame) == HAL_OK)
    {
        switch (frame.Data[0] + 0x1)
        {
        case Mode:
            memcpy(&temp, &frame.Data[1], sizeof(float));
            motorp->moderead = (uint8_t)temp;
            break;
        case Error:
            memcpy(&temp, &frame.Data[1], sizeof(float));
            motorp->motorErr = (uint8_t)temp;
            break;
        case PositionSet:
            motorp->valueSetLast.angle = motorp->valueSetNow.angle;
            memcpy(&motorp->valueSetNow.round, &frame.Data[1], sizeof(float));
            motorp->valueSetNow.angle = motorp->valueSetNow.round * 360.f / motorp->param.ratio;
            break;
        case PositionReal:
            memcpy(&motorp->valueReal.round, &frame.Data[1], sizeof(float));
            motorp->valueReal.angle = motorp->valueReal.round * 360.f / motorp->param.ratio;
            break;
        case CurrentSet:
            motorp->valueSetLast.current = motorp->valueSetNow.current;
            memcpy(&motorp->valueSetNow.current, &frame.Data[1], sizeof(float));
            break;
        case CurrentReal:
            memcpy(&motorp->valueReal.current, &frame.Data[1], sizeof(float));
        case SpeedSet:
            motorp->valueSetLast.speed = motorp->valueSetNow.speed;
            memcpy(&motorp->valueSetNow.speed, &frame.Data[1], sizeof(float));
            motorp->valueSetNow.speed *= 60.f;
            break;
        case SpeedReal:
            memcpy(&motorp->valueReal.speed, &frame.Data[1], sizeof(float));
            motorp->valueReal.speed *= 60.f;
            break;

        default:
            motorp->motorErr = UnkownCommand;
            break;
        }
        return 0;
    }
    else
    {
        return 1;
    }
}
/*------------命令封装函数---------------*/
int Motor_Set_Mode(MotorPtr motorp, MotorMode mode)
{
    return CAN_Write_Cmd(Mode, (float)mode, motorp->param.ID, CAN_STD_DATASIZE); // 电机模式设置
}
int Motor_Set_Value(MotorPtr motorp, MotorTxCMD cmd, float value)
{
    float temp;
    switch (cmd)
    {
    case SpeedSet:
        temp = value / 60.f * motorp->param.ratio;
        break;
    case PositionSet:
        temp = value / 360.f * motorp->param.ratio; // 电机运行参数设置
        break;
    case PositionReal:
        temp = value / 360.f * motorp->param.ratio;
        break;
    default:
        return 1;
        break;
    }
    return CAN_Write_Cmd(cmd, temp, motorp->param.ID, CAN_STD_DATASIZE);
}
int Motor_Request_Data(MotorPtr motorp, MotorTxCMD cmd)
{
    return CAN_Write_Cmd(cmd - 0x1, 0.f, motorp->param.ID, CAN_CMD_DATASIZE); // 请求电机参数,请求参数需要将DLC置为1(通信协议需要注意DLC设置)
}
/*------------集和函数---------------*/
void Motor_Func(MotorPtr motorp) // 控制逻辑容易出错!
{
    if (motorp->enable == 0)
    {
        if (motorp->modeset != Disable)
        {
            Motor_Set_Mode(motorp, motorp->modeset);
            motorp->enable = 1;
        }
        return; // 未启用时不进入控制逻辑
    }
    else if (motorp->modeset != motorp->moderead)
    {
        Motor_Set_Mode(motorp, motorp->modeset);
    }
    else
    {
        switch (motorp->moderead)
        {
        case Disable:
            //            Motor_Set_Mode(motorp, motorp->moderead);
            motorp->enable = 0;
            break;
        case Position:
            if (!isMotor_On_Setposition(motorp) && ABS(motorp->valueSetNow.angle - motorp->valueSetLast.angle) > POSITION_TOLERANCE_ANGLE) // 电机行止逻辑注意
            {
                Motor_Set_Value(motorp, PositionSet, motorp->valueSetNow.angle);
                motorp->valueSetLast.angle = motorp->valueSetNow.angle;
            }
            break;
        case Speed:
            if (!isMotor_On_Setspeed(motorp) && ABS(motorp->valueSetNow.speed - motorp->valueSetLast.speed) > SPEED_TOLERANCE_RPM)
            {
                Motor_Set_Value(motorp, SpeedSet, motorp->valueSetNow.speed);
            }
            break;
        // case PositionReal:
        //     if (ABS(motorp->valueSetNow.angle - motorp->valueReal.angle) < POSITION_TOLERANCE_ANGLE)
        //     {
        //         Motor_Set_Value(PositionReal, motorp->valueReal.angle, motorp->param.ID);
        //         Motor_Set_Value(PositionSet, motorp->valueReal.angle, motorp->param.ID);
        //     }
        default:
            motorp->motorErr = UnkownCommand;
            break;
        }
    }
    Motor_Request_Data(motorp, SpeedReal);
    Motor_Request_Data(motorp, SpeedSet);
    Motor_Request_Data(motorp, PositionReal);
    Motor_Request_Data(motorp, PositionSet);
    Motor_Request_Data(motorp, Mode);
}
/*------------工具函数---------------*/
CAN_TxHeaderTypeDef CAN_Frame_ToTxHeader(CANFrame *frame) // 用于发送时出队,在发送函数中临时定义txheader传参
{
    CAN_TxHeaderTypeDef hdr = {0};
    hdr.StdId = frame->StdId;
    hdr.IDE = frame->IDE;
    hdr.RTR = frame->RTR;
    hdr.DLC = frame->DLC;
    hdr.ExtId = 0;
    return hdr;
}
CANFrame CAN_TxHeader_ToFrame(CAN_RxHeaderTypeDef *hdr) // 用于接收时入队,在接收函数中临时定义rxheader传参
{
    CANFrame frame;
    frame.StdId = hdr->StdId;
    frame.IDE = hdr->IDE;
    frame.RTR = hdr->RTR;
    frame.DLC = hdr->DLC;
    return frame;
}