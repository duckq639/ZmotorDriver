#ifndef CANCOMMAND_H
#define CANCOMMAND_H
/*============================ 宏定义区 ============================*/

/*============================ 头文件包含区 ============================*/
#include "canqueue.h"

/*============================ 数据结构区 ============================*/
typedef struct
{
    uint8_t command;
    float data;
    uint32_t motorID;
    uint32_t datalenth;
} CAN_CMD;
/*============================ 函数声明区 ============================*/
int CAN_Cmd_Init(CAN_CMD *cancmd);

int CAN_Write_Cmd(CAN_CMD *cancmd);
int CAN_Send_Cmd();
int CAN_Recieve_Cmd(CAN_RxHeaderTypeDef *hdr, uint8_t *data); // 命令中间层和底层
int CAN_Read_Cmd(CANQueue *queue, CAN_CMD *cancmd);

CAN_TxHeaderTypeDef CAN_Frame_ToTxHeader(CANFrame *frame); // 转换工具
CANFrame CAN_TxHeader_ToFrame(CAN_RxHeaderTypeDef *hdr);

#endif // !CANCOMMAND_H