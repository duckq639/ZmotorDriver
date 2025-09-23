
/*=====================================
 *             头文件包含区
 *=====================================*/
#include "zmotor.h"

/*=====================================
 *             全局变量区
 *=====================================*/
ZMotor zmotor[ZMOTOR_NUMBER];
ZMotorPtr zmotorp = zmotor;
uint8_t ZMotor_ID_List[MAX_ZMOTOR_ID + 1] = {0};
float a_trip_save[2] = {0};
/*=====================================
 *             函数实现区
\*=====================================*/

/*-----------------初始化函数--------------------------------*/
int ZMotor_init(ZMotorPtr motorp, uint32_t id)
{
    if (id == 0 || id > MAX_ZMOTOR_ID)
    {
        return 1;
    }

    // 初始化电机静态参数
    motorp->param.pulsePerRound = 0;
    motorp->param.reductionRatio = 1.0f;
    motorp->param.gearRatio = 1.0f;
    motorp->param.currentLimit = CURRENT_LIMIT;
    motorp->param.ID = id;

    // 初始化电机运行参数

    motorp->valueSetNow.current = 0.0f;
    motorp->valueSetNow.speed = 0.0f;
    motorp->valueSetNow.angle = 0.0f;
    motorp->valueSetNow.round = 0.0f;

    motorp->pvtvalueSetNow.deltatime = 0.01f;
    motorp->pvtvalueSetNow.pvspeed = 0.f;
    motorp->pvtvalueSetNow.pvangle = 0.f;

    motorp->valueReal.angle = 0.0f;
    motorp->valueReal.round = 0.0f;

    // 初始化电机执行状态
    motorp->status.isArrived = false;
    motorp->status.isZeroed = false;
    motorp->status.isOvertime = false;
    motorp->status.isStuck = false;
    motorp->status.findZero = false;

    // 初始化电机模式
    motorp->modeset = Disable;  // 初始化为禁用模式
    motorp->moderead = Disable; // 初始化为禁用模式

    // 计算总减速比
    motorp->param.ratio = motorp->param.reductionRatio * motorp->param.gearRatio;

    // 存入ID数据(ID命名应在1~8,用于对齐电机数组ID和电机实际ID,list的0位弃用)
    uint8_t array_idx = (uint8_t)(motorp - zmotor); // 指针减首地址 = 下标
    ZMotor_ID_List[id] = array_idx;
    // 发送CAN报文(丐版)
    ZMotor_Set_Mode(motorp, motorp->modeset);
    ZMotor_Set_Value(motorp, PositionReal, 0.f);

    motorp->begin = 1;
    motorp->brake = 0;

    return 0;
}

/*------------命令封装函数---------------*/
int ZMotor_Set_Mode(ZMotorPtr motorp, ZMotorMode mode)
{
    ZCAN_CMD cancmd;
    cancmd.datalenth = ZMOTOR_STD_DATASIZE;
    cancmd.motorID = motorp->param.ID;
    cancmd.command = Mode;
    cancmd.data = (float)mode;

    return ZCAN_Write_Cmd(&cancmd); // 电机模式设置
}
int ZMotor_Set_PVT_Mode(ZMotorPtr motorp)
{
    ZCAN_CMD cancmd;
    cancmd.datalenth = ZMOTOR_STD_DATASIZE;
    cancmd.motorID = motorp->param.ID;
    cancmd.command = a_trp_up;
    cancmd.data = 0.f;
    ZCAN_Write_Cmd(&cancmd);
    cancmd.command = a_trp_down;
    cancmd.data = motorp->pvtvalueSetNow.deltatime;
    ZCAN_Write_Cmd(&cancmd);
    return 0;
}
int ZMotor_Set_Value(ZMotorPtr motorp, ZMotorTxCMD cmd, float value)
{
    ZCAN_CMD cancmd;
    cancmd.datalenth = ZMOTOR_STD_DATASIZE;
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
    return ZCAN_Write_Cmd(&cancmd);
}
int ZMotor_Set_PVT_Value(ZMotorPtr motorp)
{
    ZCAN_CMD cancmd;
    cancmd.datalenth = ZMOTOR_PVT_DATASIZE;
    cancmd.command = 0;
    cancmd.motorID = motorp->param.ID;
    cancmd.data = motorp->pvtvalueSetNow.pvspeed / 60.f * motorp->param.ratio;
    cancmd.data2 = motorp->pvtvalueSetNow.pvangle / 360.f * motorp->param.ratio;
    return ZCAN_Write_Cmd(&cancmd);
}
int ZMotor_Request_Data(ZMotorPtr motorp, ZMotorTxCMD cmd)
{
    ZCAN_CMD cancmd = {0};
    cancmd.motorID = motorp->param.ID;
    cancmd.datalenth = ZMOTOR_CMD_DATASIZE;
    cancmd.command = cmd - 0x1;
    return ZCAN_Write_Cmd(&cancmd); // 请求电机参数,请求参数需要将DLC置为1(通信协议需要注意DLC设置)
}
int ZMotor_Read_Data(ZMotorPtr motorp, ZCAN_CMD *cancmd)
{
    float temp = 0;
    switch (cancmd->command + 0x1)
    {
    case Mode:
        memcpy(&temp, &cancmd->data, sizeof(float));
        motorp->moderead = (uint8_t)temp;
        break;
    case Error:
        memcpy(&temp, &cancmd->data, sizeof(float));
        motorp->motorErr = (uint8_t)temp;
        break;
    case PositionSet:
        motorp->valueSetLast.angle = motorp->valueSetNow.angle;
        memcpy(&motorp->valueSetNow.round, &cancmd->data, sizeof(float));
        motorp->valueSetNow.angle = motorp->valueSetNow.round * 360.f / motorp->param.ratio;
        break;
    case PositionReal:
        memcpy(&motorp->valueReal.round, &cancmd->data, sizeof(float));
        motorp->valueReal.angle = motorp->valueReal.round * 360.f / motorp->param.ratio;
        break;
    case CurrentSet:
        motorp->valueSetLast.current = motorp->valueSetNow.current;
        memcpy(&motorp->valueSetNow.current, &cancmd->data, sizeof(float));
        break;
    case CurrentReal:
        memcpy(&motorp->valueReal.current, &cancmd->data, sizeof(float));
    case SpeedSet:
        motorp->valueSetLast.speed = motorp->valueSetNow.speed;
        memcpy(&motorp->valueSetNow.speed, &cancmd->data, sizeof(float));
        motorp->valueSetNow.speed *= 60.f;
        break;
    case SpeedReal:
        memcpy(&motorp->valueReal.speed, &cancmd->data, sizeof(float));
        motorp->valueReal.speed *= 60.f;
        break;
    default:
        motorp->motorErr = UnkownCommand;
        break;
    }
    return 0;
}
int ZMotor_Update(CAN_RxHeaderTypeDef *hdr, uint8_t *data)
{
    ZCAN_CMD cancmd = {0};
    cancmd.command = data[0];
    memcpy(&cancmd.data, data + 1, sizeof(float));
    cancmd.motorID = hdr->StdId;
    return ZMotor_Read_Data(&zmotor[ZMotor_ID_List[cancmd.motorID]], &cancmd);
}
int ZMotor_Save_Position(ZMotorPtr motorp)
{
    ZCAN_CMD cancmd = {0};
    cancmd.motorID = motorp->param.ID;
    ZCAN_Write_Cmd(&cancmd);
    return 0;
}
void ZMotor_Err_Handler(ZMotorPtr motorp)
{
    motorp->begin = false;
}
/*--------------------------------集成封装函数------------------------------------*/
void ZMotor_Func(ZMotorPtr motorp) // 控制逻辑容易出错!
{
    if (motorp->motorErr)
    {
        ZMotor_Err_Handler(motorp);
        return;
    }
    if (!motorp->begin)
    {

        return; // 未启用时不进入控制逻辑
    }
    if (motorp->modeset != motorp->moderead)
    {
        if (motorp->modeset != Disable)
        {
            ZMotor_Set_Mode(motorp, motorp->modeset);
            if (motorp->modeset == PVT_Mode)
            {
                ZMotor_Set_PVT_Mode(motorp);
            }
        }
        else
        {
            return;
        }
    }
    else
    {
        switch (motorp->moderead)
        {
        case Disable:
            // ZMotor_Set_Mode(motorp, motorp->moderead);
            break;
        case Current:
            ZMotor_Set_Value(motorp, CurrentSet, motorp->valueSetNow.current);
            break;
        case Position:
            if (!isZMotor_On_Setposition(motorp) && ABS(motorp->valueSetNow.angle - motorp->valueSetLast.angle) > POSITION_TOLERANCE_ANGLE) // 电机行止逻辑注意
            {
                ZMotor_Set_Value(motorp, PositionSet, motorp->valueSetNow.angle);
            }

            else if (motorp->status.SavePositionFlag)
            {
                motorp->status.SavePositionFlag = 0;
                int ZMotor_Save_Position(ZMotorPtr motorp);
            }
            else if (motorp->status.findZero)
            {
                ZMotor_Set_Value(motorp, PositionSet, HOME_POSITION);
                if (isZMotor_On_Setposition(motorp))
                    motorp->status.findZero = false;
            }
            else if (motorp->status.isZeroed)
            {
                ZMotor_Set_Value(motorp, PositionReal, 0.f);
                if (motorp->valueReal.angle < POSITION_TOLERANCE_ANGLE)
                {
                    motorp->status.isZeroed = 0;
                }
            }
            break;
        case Speed:
            if (!isZMotor_On_Setspeed(motorp) && ABS(motorp->valueSetNow.speed - motorp->valueSetLast.speed) > SPEED_TOLERANCE_RPM)
            {
                ZMotor_Set_Value(motorp, SpeedSet, motorp->valueSetNow.speed);
            }
            break;
        case PVT_Mode:
            ZMotor_Set_PVT_Value(motorp);
            break;
        default:
            motorp->motorErr = UnkownCommand;
            break;
        }
    }
    ZMotor_Request_Data(motorp, SpeedReal);
    //    ZMotor_Request_Data(motorp, SpeedSet);
    ZMotor_Request_Data(motorp, PositionReal);
    //  ZMotor_Request_Data(motorp, PositionSet);
    ZMotor_Request_Data(motorp, Mode);
}
/*--------------------------------工具函数------------------------------------*/
