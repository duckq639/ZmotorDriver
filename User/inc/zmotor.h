#ifndef ZMOTOR_H
#define ZMOTOR_H

//=============================================
//               宏定义
#define POSITION_TOLERANCE_ANGLE 0.5f
#define SPEED_TOLERANCE_RPM 20.f
#define CURRENT_LIMIT 0.f
//=============================================

//=============================================
//               包含头文件
#include "stdbool.h"
#include "mathFunc.h"
#include "stdint.h"

//=============================================

//=============================================
//               类定义
/*------------------------结构体类-------------------------------*/
typedef struct
{
    float current;
    float speed;
    float angle;
    float round;
} MotorValue; /*电机运行参数*/

typedef struct
{
    uint16_t pulsePerRound; // 电机一圈脉冲数
    float reductionRatio;   // 电机减速比
    float gearRatio;        // 机构减速比
    float ratio;            // 电机减速比和机构减速比之积
    uint32_t ID;
    uint16_t currentLimit;

} MotorParam; // 电机静态参数,包含CAN参数

typedef struct
{
    volatile bool isArrived;  // 是否到位
    volatile bool isZeroed;   // 寻零完成
    volatile bool isOvertime; // 是否超时
    volatile bool isStuck;    // 是否堵转
    volatile bool isSetzero;  // 是否将当前位置置为0：
} MotorStatus;                // 电机执行情况
/*------------------------枚举类------------------------------------*/

typedef enum
{
    Acceleration = 0x29,
    Deceleration = 0x2B,
    Mode = 0x3D,
    Error = 0x41,
    CurrentSet = 0x43,
    SpeedSet = 0x45,
    PositionSet = 0x47,
    CurrentReal = 0x53,
    SpeedReal = 0x5D,
    PositionReal = 0x5F,
} MotorTxCMD; // 电机发送命令,接收(请求)命令为"发送命令-1"

typedef enum //  电机模式，用于指定电机的操作模式
{
    Disable,
    Current,
    Speed,
    Position,
    Test,
    /*后面的看不懂先写到这里*/
} MotorMode;

typedef enum
{
    NoErr,
    LowVolt,
    OverVolt,
    InstableCurrent,
    OverCurrent,
    OverSpeed,
    UnkownCommand = 14,
    /*同理看不懂的先不写了*/
} MotorError;
/*---------------------------整体封装------------------------------------*/
typedef struct
{
    volatile bool enable;
    volatile bool begin;
    volatile bool brake;

    MotorParam param;                                // 电机静态参数
    MotorValue valueSetLast, valueSetNow, valueReal; // 电机运行参数
    MotorError motorErr;                             // 电机错误
    MotorStatus status;                              // 电机执行状态
    MotorMode modeset, moderead;                     // 当前电机模式
} Motor, *MotorPtr;
//=============================================

//=============================================
//               函数声明
/*-----------------初始化函数--------------------------------*/

int motor_init(MotorPtr motorp);
/*-----------------命令运行函数--------------------------------*/
int Motor_Set_Mode(MotorPtr motorp, MotorMode mode); // 命令封装
int Motor_Set_Value(MotorPtr motorp, MotorTxCMD cmd, float value);
int Motor_Request_Data(MotorPtr motorp, MotorTxCMD cmd);
/*-----------------------工具函数-------------------------*/

static inline bool isMotor_On_Setposition(MotorPtr motorp)
{
    if (ABS(motorp->valueSetNow.angle - motorp->valueReal.angle) > POSITION_TOLERANCE_ANGLE)
        return false;
    else
        return true;
}
static inline bool isMotor_On_Setspeed(MotorPtr motorp)
{
    if (ABS(motorp->valueSetNow.speed - motorp->valueSetLast.speed) > SPEED_TOLERANCE_RPM)
        return false;
    else
        return true;
}
//=============================================

#endif // !ZMOTOR_H
