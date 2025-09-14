#include "zcancommand.h"

/*--------------初始化函数---------------*/
int ZCAN_Cmd_Init(ZCAN_CMD *cancmd)
{
    cancmd->command = 0;
    cancmd->data = 0;
    cancmd->motorID = 0;
    cancmd->datalenth = ZMOTOR_STD_DATASIZE;
    return 0;
}
/*------------命令运行函数---------------*/
int ZCAN_Write_Cmd(ZCAN_CMD *cancmd)
{
    CANFrame frame = {0};
    frame.CAN_TxHeader.DLC = cancmd->datalenth;
    frame.CAN_TxHeader.StdId = cancmd->motorID;
    frame.Data[0] = cancmd->command;
    if (frame.CAN_TxHeader.DLC == 5)
        memcpy(frame.Data + 1, &cancmd->data, sizeof(float)); // 命令传输中间层
    Queue_Push(&CAN_TxQueue, &frame);
    return 0;
}

int ZCAN_Send_Cmd()
{
    uint32_t mailbox;
    CANFrame frame = {0};
    if (Queue_Pop(&CAN_TxQueue, &frame) == HAL_OK) // 出队发送,使用addtxmessage + 固定间隔发送,命令实现层
    {
        return HAL_CAN_AddTxMessage(&hcan1, &frame.CAN_TxHeader, frame.Data, &mailbox);
    }
    else
    {
        return 1;
    }
}

/*------------工具函数---------------*/
