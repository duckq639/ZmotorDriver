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
    CAN_TxFrame frame = {0};
    frame.CAN_TxHeader.DLC = cancmd->datalenth;
    frame.CAN_TxHeader.StdId = cancmd->motorID;
    if (frame.CAN_TxHeader.DLC == ZMOTOR_PVT_DATASIZE)
    {
        memcpy(frame.Data, &cancmd->data, sizeof(float));
        memcpy(frame.Data + 4, &cancmd->data2, sizeof(float));
    }
    else
    {
        frame.Data[0] = cancmd->command;
        if (frame.CAN_TxHeader.DLC == 5)
            memcpy(frame.Data + 1, &cancmd->data, sizeof(float)); // 命令传输中间层
    }
    CAN_Queue_Push(&CAN1_TxQueue, &frame);
    return 0;
}

int ZCAN_Send_Cmd()
{
    uint32_t mailbox;
    CAN_TxFrame frame = {0};
    if (CAN_Queue_Pop(&CAN1_TxQueue, &frame) == HAL_OK) // 出队发送,使用addtxmessage + 固定间隔发送,命令实现层
    {
        return HAL_CAN_AddTxMessage(&Z_CAN_CHANNEL, &frame.CAN_TxHeader, frame.Data, &mailbox);
    }
    else
    {
        return 1;
    }
}

/*------------工具函数---------------*/
