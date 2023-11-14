//
// Created by 李唐 on 2023/11/13.
//

#include "Motion.h"
PAL_PCA9685 PCA9685;

static HAL_StatusTypeDef i2c_tx(uint16_t add, uint8_t mem,uint8_t data){
    return HAL_I2C_Mem_Write(&hi2c2,add,mem,
                             I2C_MEMADD_SIZE_8BIT,&data,1,1000);
}

static HAL_StatusTypeDef i2c_rx(uint16_t add,uint8_t mem,uint8_t* data){
    return HAL_I2C_Mem_Read(&hi2c2,add,mem,
                            I2C_MEMADD_SIZE_8BIT,data,1,1000);
}

static HAL_StatusTypeDef WheelInit(WheelType *_Wheel, uint8_t MoForPin, uint8_t MoBakPin, TIM_HandleTypeDef* Timer)
{
    _Wheel->Motor.ForwardPin = MoForPin;
    _Wheel->Motor.BakwardPin = MoBakPin;

    _Wheel->Encoder.Timer = Timer;
    HAL_TIM_Encoder_Start(_Wheel->Encoder.Timer,TIM_CHANNEL_1);
    HAL_TIM_Encoder_Start(_Wheel->Encoder.Timer,TIM_CHANNEL_2);
    _Wheel->Encoder.NRound=0;
    _Wheel->Encoder.RadVelocity=0;
    _Wheel->Encoder.Direction = 1;
    __HAL_TIM_SetCounter(_Wheel->Encoder.Timer,0);
    __HAL_TIM_ENABLE_IT(_Wheel->Encoder.Timer,TIM_IT_UPDATE);

    _Wheel->Encoder.RadVelocity = 0.0f;
    _Wheel->Motor.RadVelocity = 0.0f;

    _Wheel->Parameter.bias = 0;
    _Wheel->Parameter.integral = 0;
    _Wheel->Parameter.kp = (KP);
    _Wheel->Parameter.ki = (KI);
    _Wheel->Parameter.kd = (KD);

    return HAL_OK;
}

HAL_StatusTypeDef MotionInit(MotionType *_Car, TIM_HandleTypeDef* LeFr, TIM_HandleTypeDef* LeRe, TIM_HandleTypeDef* RiFr, TIM_HandleTypeDef* RiRe)
{
    WheelInit(&_Car->LeftFrot,MotorLeftFrontForward,MotorLeftFrontBackward,LeFr);
    WheelInit(&_Car->LeftRear,MotorLeftRearForward,MotorLeftRearBackward,LeRe);
    WheelInit(&_Car->RightFrot,MotorRightFrontForward,MotorRightFrontBackward,RiFr);
    WheelInit(&_Car->RightRear,MotorRightRearForward,MotorRightRearBackward,RiRe);

    _Car->LeftRear.Encoder.Direction = -1;

    _Car->Position = 0.0f;
    _Car->Volocity = 0.0f;

    PCA9685.addr = PCA9685_IIC_ADDR;
    PCA9685.osc = PCA9685_INTERNAL_OSC;
    PCA9685.i2c_rx = i2c_rx;
    PCA9685.i2c_tx = i2c_tx;


    if(PAL_PCA9685_IsSleep(&PCA9685)) PAL_PCA9685_Sleep(&PCA9685,0);
    PAL_PCA9685_SetFrequency(&PCA9685,1500);
    PAL_PCA9685_SetOutput(&PCA9685,0,1.0f,0);


    return HAL_OK;
}

HAL_StatusTypeDef WheelSetRadVolocity(WheelType *_Wheel, float targetVolocity)
{
    _Wheel->Motor.RadVelocity = targetVolocity;
    return HAL_OK;
}

static float VelocityCurve(float rawVelocity)
{
    return rawVelocity ;
}

static HAL_StatusTypeDef MotorOutSpeed(WheelType *_Wheel)
{
    float RadVelocity = _Wheel->Parameter.PID;
    if(RadVelocity > 0)
    {
        PAL_PCA9685_SetOutput(&PCA9685,_Wheel->Motor.BakwardPin,0,0);
        PAL_PCA9685_SetOutput(&PCA9685,_Wheel->Motor.ForwardPin,fabsf(VelocityCurve(RadVelocity)),0);
    }
    else
    {
        PAL_PCA9685_SetOutput(&PCA9685,_Wheel->Motor.ForwardPin,0,0);
        PAL_PCA9685_SetOutput(&PCA9685,_Wheel->Motor.BakwardPin,fabsf(VelocityCurve(RadVelocity)),0);
    }
    return HAL_OK;
}

static HAL_StatusTypeDef EncoderUpdate(WheelType *_Wheel)
{
    __HAL_TIM_SetCounter(_Wheel->Encoder.Timer,0);
    int16_t prvPulse = __HAL_TIM_GetCounter(_Wheel->Encoder.Timer);
    HAL_Delay(INTERVAL);
    int16_t curPulse = __HAL_TIM_GetCounter(_Wheel->Encoder.Timer);
    int16_t Pulse = (curPulse - prvPulse)*_Wheel->Encoder.Direction;
    // if(curPulse - prvPulse > 32767) Pulse = (-1) * (curPulse - prvPulse);
    // else Pulse = (curPulse - prvPulse);
    _Wheel->Encoder.RadPosition += Pulse/(REDUCE*PULSES)*0.17;
    _Wheel->Encoder.RadVelocity = Pulse/(REDUCE*PULSES*INTERVAL)*360*0.17;
    _Wheel->Encoder.NPulse += Pulse;
    _Wheel->Encoder.NRound = Pulse; //debug
    __HAL_TIM_SetCounter(_Wheel->Encoder.Timer,0);
    return HAL_OK;
}

static HAL_StatusTypeDef WheelRadVelocityLoop(WheelType *_Wheel)
{
    _Wheel->Parameter.bias = ( _Wheel->Motor.RadVelocity - _Wheel->Encoder.RadVelocity ) * _Wheel->Parameter.kp;
    _Wheel->Parameter.integral += _Wheel->Parameter.bias * _Wheel->Parameter.ki;
    _Wheel->Parameter.PID = _Wheel->Motor.RadVelocity + _Wheel->Parameter.bias + _Wheel->Parameter.integral;
    return HAL_OK;
}

static HAL_StatusTypeDef WheelUpdate(WheelType *_Wheel)
{
    MotorOutSpeed(_Wheel);
    EncoderUpdate(_Wheel);
    WheelRadVelocityLoop(_Wheel);
    return HAL_OK;
}

HAL_StatusTypeDef MotionUpdate(MotionType *_Car)
{
    WheelUpdate(&_Car->LeftFrot);
    WheelUpdate(&_Car->LeftRear);
    WheelUpdate(&_Car->RightFrot);
    WheelUpdate(&_Car->RightRear);
    return HAL_OK;
}

void MotionMoveRad(MotionType *_Car, float DirectionRad, float Speed){
    float Xaxis, Yaxis;
    Xaxis = Speed * cosf(DirectionRad);
    Yaxis = Speed * sinf(DirectionRad);

    WheelSetRadVolocity(&_Car->LeftFrot,Yaxis-Xaxis);
    WheelSetRadVolocity(&_Car->LeftRear,Yaxis+Xaxis);
    WheelSetRadVolocity(&_Car->RightFrot,Yaxis+Xaxis);
    WheelSetRadVolocity(&_Car->RightRear,Yaxis-Xaxis);

    MotionUpdate(_Car);
}

void MotionMoveInt(MotionType *_Car, int X_intensity, int Y_intensity){
    float Xaxis, Yaxis;
    X_intensity = X_intensity >=40 || X_intensity <= -40 ? X_intensity : 0;
    Y_intensity = Y_intensity >=40 || Y_intensity <= -40 ? Y_intensity : 0;
    Xaxis = 1.0 * X_intensity/400.0;
    Yaxis = 1.0 * Y_intensity/400.0;

    // float Speed;
    // float DirectionAngel;
    // DirectionAngel = atanf(Y_intensity/X_intensity);
    // Speed = sqrtf(2) * sqrtf(X_intensity * X_intensity + Y_intensity * Y_intensity) / 2 ;
    // Xaxis = Speed * cosf(DirectionAngel / 360.0 * 2 * 3.14);
    // Yaxis = Speed * sinf(DirectionAngel / 360.0 * 2 * 3.14);

    WheelSetRadVolocity(&_Car->LeftFrot,Yaxis-Xaxis);
    WheelSetRadVolocity(&_Car->LeftRear,Yaxis+Xaxis);
    WheelSetRadVolocity(&_Car->RightFrot,Yaxis+Xaxis);
    WheelSetRadVolocity(&_Car->RightRear,Yaxis-Xaxis);

    MotionUpdate(_Car);

}