#ifndef ZCANCOMMAND_H
#define ZCANCOMMAND_H
/*============================ 宏定义区 ============================*/
#define ZMOTOR_STD_DATASIZE 5 // 数据段DLC
#define ZMOTOR_CMD_DATASIZE 1 // 请求段DLC,只包含命令
/*============================ 头文件包含区 ============================*/
#include "canqueue.h"
/*============================ 数据结构区 ============================*/
typedef struct
{
    uint8_t command;
    float data;
    uint32_t motorID;
    uint8_t datalenth;
} ZCAN_CMD;
/*============================ 函数声明区 ============================*/

int ZCAN_Write_Cmd(ZCAN_CMD *cancmd);
int ZCAN_Send_Cmd();

#endif // !ZCANCOMMAND_H