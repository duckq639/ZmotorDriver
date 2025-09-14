#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "zmotor.h"
/*-------------------------类型定义声明----------------------------*/

/*-------------------------全局变量声明(定义)--------------------------*/

void Send_Command(void *argument)
{
    //  LED_Pop();
    static uint8_t ticktime;
    for (;;)
    {
        if (ticktime == 10)
        {
            Motor_Func(zmotorp);
            ticktime = 0;
        }
        ticktime++;
        ZCAN_Send_Cmd();
        osDelay(10);
    }
}
