
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

/*------------命令封装函数---------------*/
int Motor_Set_Mode(MotorPtr motorp, MotorMode mode)
{
    CAN_CMD cancmd;
    CAN_Cmd_Init(&cancmd);
    cancmd.command = Mode;
    cancmd.data = (float)mode;
    cancmd.motorID = motorp->param.ID;
    return CAN_Write_Cmd(&cancmd); // 电机模式设置
}
int Motor_Set_Value(MotorPtr motorp, MotorTxCMD cmd, float value)
{
    CAN_CMD cancmd;
    CAN_Cmd_Init(&cancmd);
    cancmd.command = cmd;
    cancmd.motorID = motorp->param.ID;
    switch (cancmd.command)
    {
    case SpeedSet:
        motorp->valueSetLast.speed = motorp->valueSetNow.speed;
        cancmd.data = value / 60.f * motorp->param.ratio;
        break;
    case PositionSet:
        motorp->valueSetLast.angle = motorp->valueSetNow.angle;
        motorp->valueSetLast.round = motorp->valueSetNow.round;
        cancmd.data = value / 360.f * motorp->param.ratio; // 电机运行参数设置
        break;
    case PositionReal:
        cancmd.data = value / 360.f * motorp->param.ratio;
        break;
    default:
        return 1;
        break;
    }
    return CAN_Write_Cmd(&cancmd);
}
int Motor_Request_Data(MotorPtr motorp, MotorTxCMD cmd)
{
    CAN_CMD cancmd = {0};
    cancmd.motorID = motorp->param.ID;
    cancmd.datalenth = ZMOTOR_CMD_DATASIZE;
    cancmd.command = cmd - 0x1;
    return CAN_Write_Cmd(&cancmd); // 请求电机参数,请求参数需要将DLC置为1(通信协议需要注意DLC设置)
}
int Motor_Data_Read(MotorPtr motorp)
{
    float temp = 0;
    CAN_CMD cancmd = {0};
    CAN_Read_Cmd(&CAN_RxQueue, &cancmd);
    if (cancmd.motorID == motorp->param.ID)
    {
        switch (cancmd.command + 0x1)
        {
        case Mode:
            memcpy(&temp, &cancmd.data, sizeof(float));
            motorp->moderead = (uint8_t)temp;
            break;
        case Error:
            memcpy(&temp, &cancmd.data, sizeof(float));
            motorp->motorErr = (uint8_t)temp;
            break;
        case PositionSet:
            motorp->valueSetLast.angle = motorp->valueSetNow.angle;
            memcpy(&motorp->valueSetNow.round, &cancmd.data, sizeof(float));
            motorp->valueSetNow.angle = motorp->valueSetNow.round * 360.f / motorp->param.ratio;
            break;
        case PositionReal:
            memcpy(&motorp->valueReal.round, &cancmd.data, sizeof(float));
            motorp->valueReal.angle = motorp->valueReal.round * 360.f / motorp->param.ratio;
            break;
        case CurrentSet:
            motorp->valueSetLast.current = motorp->valueSetNow.current;
            memcpy(&motorp->valueSetNow.current, &cancmd.data, sizeof(float));
            break;
        case CurrentReal:
            memcpy(&motorp->valueReal.current, &cancmd.data, sizeof(float));
        case SpeedSet:
            motorp->valueSetLast.speed = motorp->valueSetNow.speed;
            memcpy(&motorp->valueSetNow.speed, &cancmd.data, sizeof(float));
            motorp->valueSetNow.speed *= 60.f;
            break;
        case SpeedReal:
            memcpy(&motorp->valueReal.speed, &cancmd.data, sizeof(float));
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
void Motor_Err_Handler(MotorPtr motorp)
{
}
/*--------------------------------集成封装函数------------------------------------*/
void Motor_Func(MotorPtr motorp) // 控制逻辑容易出错!
{
    if (motorp->motorErr)
    {
        Motor_Err_Handler(motorp);
    }
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
            // Motor_Set_Mode(motorp, motorp->moderead);
            motorp->enable = 0;
            break;
        case Position:
            if (!isMotor_On_Setposition(motorp) && ABS(motorp->valueSetNow.angle - motorp->valueSetLast.angle) > POSITION_TOLERANCE_ANGLE) // 电机行止逻辑注意
            {
                Motor_Set_Value(motorp, PositionSet, motorp->valueSetNow.angle);
            }
            if (isMotor_On_Setposition(motorp) && !motorp->status.isZeroed)
            {
            }
            break;
        case Speed:
            if (!isMotor_On_Setspeed(motorp) && ABS(motorp->valueSetNow.speed - motorp->valueSetLast.speed) > SPEED_TOLERANCE_RPM)
            {
                Motor_Set_Value(motorp, SpeedSet, motorp->valueSetNow.speed);
            }
            break;
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
/*--------------------------------工具函数------------------------------------*/
