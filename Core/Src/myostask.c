#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "zmotor.h"
#include "cancommand.h"
#include "canqueue.h"
/*-------------------------类型定义声明----------------------------*/
extern Motor zmotor;
extern MotorPtr zmotorp;
extern CANQueue CAN_TxQueue;
extern CANQueue CAN_RxQueue;
/*-------------------------全局变量声明(定义)--------------------------*/

void Send_Command(void *argument)
{
    //  LED_Pop();
    static int ticktime = 0;
    for (;;)
    {
        if (ticktime == 10)
        {
            Motor_Func(zmotorp);
            // LED_Pop();
            ticktime = 0;
        }
        ticktime++;
        CAN_Send_Cmd();
        osDelay(10);
    }
}

void Read_Command(void *argument)
{

    for (;;)
    {
        Motor_Data_Read(zmotorp);
        osDelay(1);
    }
}