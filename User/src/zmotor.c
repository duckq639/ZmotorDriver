
/*=====================================
 *             头文件包含区
 *=====================================*/
#include "zmotor.h"

/*=====================================
 *             全局变量区
 *=====================================*/

/*=====================================
 *             函数实现区
\*=====================================*/

/*-----------------初始化函数--------------------------------*/
int motor_init(MotorPtr motorp)

{
    motorp->enable = 0;
    motorp->begin = 1;
    motorp->brake = 0;

    // 初始化电机静态参数
    motorp->param.pulsePerRound = 0;
    motorp->param.reductionRatio = 1.0f;
    motorp->param.gearRatio = 1.0f;
    motorp->param.currentLimit = CURRENT_LIMIT;
    motorp->param.ID = 0x1;

    // 初始化电机运行参数

    motorp->valueSetNow.current = 0.0f;
    motorp->valueSetNow.speed = 0.0f;
    motorp->valueSetNow.angle = 0.0f;
    motorp->valueSetNow.round = 0.0f;

    motorp->valueReal.angle = 0.0f;
    motorp->valueReal.round = 0.0f;

    // 初始化电机执行状态
    motorp->status.isArrived = false;
    motorp->status.isZeroed = false;
    motorp->status.isOvertime = false;
    motorp->status.isStuck = false;
    motorp->status.isSetzero = false;

    // 初始化电机模式
    motorp->modeset = Disable;  // 初始化为禁用模式
    motorp->moderead = Disable; // 初始化为禁用模式

    // 计算总减速比
    motorp->param.ratio = motorp->param.reductionRatio * motorp->param.gearRatio;

    // 发送CAN报文(丐版)
    Motor_Set_Mode(motorp, motorp->modeset);
    Motor_Set_Value(motorp, PositionReal, 0.f);
    return 0;
}

/*-----------------命令运行函数--------------------------------*/