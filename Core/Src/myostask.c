#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "zmotor.h"
#include "cancommand.h"
#include "canqueue.h"
/*-------------------------类型定义声明----------------------------*/

/*-------------------------全局变量声明(定义)--------------------------*/

// void Send_Command(void *argument)
// {
//     //  LED_Pop();

//     for (;;)
//     {

//     }
// }

void Read_Command(void *argument)
{

    for (;;)
    {
        Motor_Data_Read(zmotorp);
        osDelay(2);
    }
}