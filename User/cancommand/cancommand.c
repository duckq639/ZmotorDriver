#include "cancommand.h"

/*--------------初始化函数---------------*/
int CAN_Cmd_Init(CAN_CMD *cancmd)
{
    cancmd->command = 0;
    cancmd->data = 0;
    cancmd->motorID = 0;
    cancmd->datalenth = ZMOTOR_STD_DATASIZE;
    return 0;
}
/*------------命令运行函数---------------*/
int CAN_Write_Cmd(CAN_CMD *cancmd)
{
    CANFrame frame = CAN_Creat_Frame(cancmd->motorID, 0, cancmd->datalenth);
    frame.Data[0] = cancmd->command;
    if (frame.DLC == 5)
        memcpy(frame.Data + 1, &cancmd->data, sizeof(float)); // 命令传输中间层
    Queue_Push(&CAN_TxQueue, &frame);
    return 0;
}

int CAN_Send_Cmd()
{
    uint32_t mailbox;
    CANFrame frame = {0};
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
    memcpy(frame.Data, data, ZMOTOR_STD_DATASIZE * sizeof(uint8_t));
    if (Queue_Push(&CAN_RxQueue, &frame))
        return 0;
    else
        return 1;
}
int CAN_Read_Cmd(CANQueue *queue, CAN_CMD *cancmd)
{
    CANFrame frame = {0};
    if (Queue_Pop(queue, &frame) == HAL_OK)
    {
        cancmd->command = frame.Data[0];
        memcpy(&cancmd->data, frame.Data + 1, sizeof(float));
        cancmd->motorID = frame.StdId;
        return 0;
    }
    else
        return 1;
}

/*------------工具函数---------------*/
CAN_TxHeaderTypeDef CAN_Frame_ToTxHeader(CANFrame *frame) // 用于发送时出队,在发送函数中临时定义txheader传参
{
    CAN_TxHeaderTypeDef hdr = {0};
    hdr.IDE = frame->IDE;
    if (hdr.IDE)
        hdr.ExtId = frame->ExtId;
    hdr.StdId = frame->StdId;
    hdr.RTR = frame->RTR;
    hdr.DLC = frame->DLC;
    hdr.ExtId = 0;
    return hdr;
}
CANFrame CAN_TxHeader_ToFrame(CAN_RxHeaderTypeDef *hdr) // 用于接收时入队,在接收函数中临时定义frame传参
{
    CANFrame frame = {0};
    frame.IDE = hdr->IDE;
    if (frame.IDE)
        frame.ExtId = hdr->ExtId;
    frame.StdId = hdr->StdId;
    frame.RTR = hdr->RTR;
    frame.DLC = hdr->DLC;
    return frame;
}