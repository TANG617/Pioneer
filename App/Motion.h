//
// Created by 李唐 on 2023/11/13.
//

#ifndef MOTION_H
#define MOTION_H
#include "stm32f1xx_hal.h"
#include "PCA9685.h"
#include <math.h>

#define KP 0.5
#define KI 0
#define KD 0

#define MotorLeftFrontForward   15
#define MotorLeftFrontBackward  14
#define MotorLeftRearForward    12
#define MotorLeftRearBackward   13
#define MotorRightRearForward   4
#define MotorRightRearBackward  3
#define MotorRightFrontForward  2
#define MotorRightFrontBackward 1

#define REDUCE 45
#define PULSES 52
#define INTERVAL 0.5f

typedef struct
{
    uint8_t ForwardPin,BakwardPin;
    float RadVelocity;
}MotorType;

typedef struct
{
    TIM_HandleTypeDef* Timer;
    float RadVelocity,RadPosition;
    uint16_t NPulse,NRound;
    int8_t Direction;
}EncoderType;

typedef struct
{
    float kp,ki,kd;
    float bias,integral;
    float PID;
}ParameterType;

typedef struct
{
    MotorType Motor;
    EncoderType Encoder;
    ParameterType Parameter;

}WheelType;

typedef struct
{
    WheelType LeftFrot,RightFrot;
    WheelType LeftRear,RightRear;
    float Volocity, Position;
}MotionType;


extern I2C_HandleTypeDef hi2c2;

HAL_StatusTypeDef MotionInit(MotionType *_Car, TIM_HandleTypeDef* LeFr, TIM_HandleTypeDef* LeRe, TIM_HandleTypeDef* RiFr, TIM_HandleTypeDef* RiRe);
void MotionMoveRad(MotionType *_Car, float DirectionRad, float Speed);




#endif //MOTION_H
