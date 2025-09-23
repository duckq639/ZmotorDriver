#ifndef ZMOTOR_H
#define ZMOTOR_H

//=============================================
//               宏定义
#define POSITION_TOLERANCE_ANGLE 0.5f
#define SPEED_TOLERANCE_RPM 20.f
#define CURRENT_LIMIT 0.f
#define HOME_POSITION 0 // 单位"度"
#define ZMOTOR_NUMBER 1
#define MAX_ZMOTOR_ID 8
//=============================================

//=============================================
//               包含头文件
#include "stdint.h"
#include "mathFunc.h"
#include "zcancommand.h"
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
} ZMotorValue; /*电机运行参数*/

typedef struct
{
    float pvspeed;
    float pvangle;
    float deltatime;
} ZMotorPVT_Value;

typedef struct
{
    uint16_t pulsePerRound; // 电机一圈脉冲数
    float reductionRatio;   // 电机减速比
    float gearRatio;        // 机构减速比
    float ratio;            // 电机减速比和机构减速比之积
    uint32_t ID;
    uint16_t currentLimit;

} ZMotorParam; // 电机静态参数,包含CAN参数

typedef struct
{
    volatile bool isArrived;  // 是否到位
    volatile bool isZeroed;   // 寻零完成
    volatile bool isOvertime; // 是否超时
    volatile bool isStuck;    // 是否堵转
    volatile bool findZero;   // 是否将当前位置置为0：
    volatile bool SavePositionFlag;
} ZMotorStatus; // 电机执行情况
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
    a_trp_up = 0x29,
    a_trp_down = 0x2B,
} ZMotorTxCMD; // 电机发送命令,接收(请求)命令为"发送命令-1"

typedef enum //  电机模式，用于指定电机的操作模式
{
    Disable,
    Current,
    Speed,
    Position,
    Test,
    RVCalibration,
    EncoderLineCalibration,
    EncodeOffsetCalibration,
    VKCalibration,
    SaveSetting,
    EraseSetting,
    ClearErr,
    Brake,
    PVT_Mode,
    /*后面的看不懂先写到这里*/
} ZMotorMode;

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
} ZMotorError;
/*---------------------------整体封装------------------------------------*/
typedef struct
{
    volatile bool begin;
    volatile bool brake;

    ZMotorParam param;                                // 电机静态参数
    ZMotorValue valueSetLast, valueSetNow, valueReal; // 电机运行参数
    ZMotorPVT_Value pvtvalueSetNow;                   // 电机pvt模式参数
    ZMotorError motorErr;                             // 电机错误
    ZMotorStatus status;                              // 电机执行状态
    ZMotorMode modeset, moderead;                     // 当前电机模式
} ZMotor, *ZMotorPtr;
//=============================================

//               类声明
//=============================================
// 电机初始化时要求输入ID
// 电机ID以结构体param中的ID为准
// ID表中填写电机ID(如"ZMotor_ID_List[1]")即可对ID为1的电机操作
// 代码太史了,目前电机操作的传入指针依然是电机结构体而不是电机ID
extern ZMotor zmotor[ZMOTOR_NUMBER];
extern ZMotorPtr zmotorp;
extern uint8_t ZMotor_ID_List[MAX_ZMOTOR_ID + 1];

//=============================================
//               函数声明
/*-----------------初始化函数--------------------------------*/

int ZMotor_init(ZMotorPtr motorp, uint32_t id);
/*-----------------命令运行函数--------------------------------*/
int ZMotor_Set_Mode(ZMotorPtr motorp, ZMotorMode mode); // 命令封装
int ZMotor_Set_PVT_Mode(ZMotorPtr motorp);
int ZMotor_Set_Value(ZMotorPtr motorp, ZMotorTxCMD cmd, float value);
int ZMotor_Set_PVT_Value(ZMotorPtr motorp); // 时间间隔只能在模式设定时设置,不支持单独更改
int ZMotor_Request_Data(ZMotorPtr motorp, ZMotorTxCMD cmd);
int ZMotor_Update(CAN_RxHeaderTypeDef *hdr, uint8_t *data);
void ZMotor_Err_Handler(ZMotorPtr motorp);
/*-----------------------工具函数-------------------------*/
static inline bool isZMotor_On_Setposition(ZMotorPtr motorp)
{
    if (ABS(motorp->valueSetNow.angle - motorp->valueReal.angle) > POSITION_TOLERANCE_ANGLE)
        return false;
    else
        return true;
}
static inline bool isZMotor_On_Setspeed(ZMotorPtr motorp)
{
    if (ABS(motorp->valueSetNow.speed - motorp->valueSetLast.speed) > SPEED_TOLERANCE_RPM)
        return false;
    else
        return true;
}
/*-----------------集和封装函数--------------------------------*/
void ZMotor_Func(ZMotorPtr motorp);
/*-----------------工具函数--------------------------------*/

//=============================================

#endif // !ZMOTOR_H
