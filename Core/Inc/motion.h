/*
 * @Author: LiTang
 * @Date: 2023-11-09 20:07:31
 * @LastEditTime: 2023-11-10 11:29:58
 */

#ifndef MOTION_H
#define MOTION_H



#include "stm32f1xx_hal.h"
#include <math.h>
typedef uint8_t bool;
#define false (0x00u)
#define true (!false)
/////////////////PCA9685
#define PCA9685_IIC_ADDR (0x40<<1)
#define PCA9685_INTERNAL_OSC 25000000

#define PCA9685_BIT_RESET   7
#define PCA9685_BIT_EXTCLK  6
#define PCA9685_BIT_AI      5
#define PCA9685_BIT_SLEEP   4
#define PCA9685_BIT_SUBADR1 3
#define PCA9685_BIT_SUBADR2 2
#define PCA9685_BIT_SUBADR3 1
#define PCA9685_BIT_ALLCALL 0

#define PCA9685_REG_MODE1       0x00
#define PCA9685_REG_MODE2       0x01
#define PCA9685_REG_SUB_ADDR1   0x02
#define PCA9685_REG_SUB_ADDR2   0x03
#define PCA9685_REG_SUB_ADDR3   0x04
#define PCA9685_REG_ALL_CALL    0x05
#define PCA9685_REG_BASE_ON_L   0x06
#define PCA9685_REG_BASE_ON_H   0x07
#define PCA9685_REG_BASE_OFF_L  0x08
#define PCA9685_REG_BASE_OFF_H  0x09
#define PCA9685_REG_ALL_ON_L    0xFA
#define PCA9685_REG_ALL_ON_H    0xFB
#define PCA9685_REG_ALL_OFF_L   0xFC
#define PCA9685_REG_ALL_OFF_H   0xFD
#define PCA9685_REG_FREQ        0xFE

#define MotorLeftFrontForward   15
#define MotorLeftFrontBackward  14
#define MotorLeftRearForward    12
#define MotorLeftRearBackward   13
#define MotorRightRearForward   4
#define MotorRightRearBackward  3
#define MotorRightFrontForward  2
#define MotorRightFrontBackward 1

#define KP 0.6f
#define KI 0.003f

#define S 0.143 //S=L/2+H/2,单位为m

/////////////////MOTOR
typedef struct{
    float Kp,Ki,Kd;
}LoopParameter;

typedef struct{
    LoopParameter Angel, Velocity;
}MotorParameter;

typedef struct{
    TIM_HandleTypeDef* Timer;
    int16_t Round;
    float Position;
    float Speed;
} EncoderNode;

typedef struct{
    uint8_t Forward, Backward;
    float Speed;
    float Position;
    MotorParameter Parameter;
}MotorNode;

typedef struct{
    MotorNode LeftFrontMotor;
    EncoderNode LeftFrontEncoder;

    MotorNode LeftRearMotor;
    EncoderNode LeftRearEncoder;

    MotorNode RightFrontMotor;
    EncoderNode RightFrontEncoder;

    MotorNode RightRearMotor;
    EncoderNode RightRearEncoder;

    float Speed;
    float Position;
}MotionNode;


void MotionInit(MotionNode *_Car, TIM_HandleTypeDef* LF, TIM_HandleTypeDef* LR, TIM_HandleTypeDef* RF, TIM_HandleTypeDef* RR);
void AdvanceIV(MotionNode *Car);
void MotionTest(MotionNode *Car);

void MotionMoveRad(MotionNode *_Car, float DirectionRad, float Speed);
void MotionMoveAng(MotionNode *_Car, float DirectionAngel, float Speed);
void MotionMoveInt(MotionNode *_Car, int X_intensity, int Y_intensity);


/////////////////ENCODER
#define REDUCE 45
#define PULSES 52
#define INTERVAL 0.5f
// Encoder eA1,eA2,eB1,eB2;



// void EncoderInit(TIM_HandleTypeDef* EA1, TIM_HandleTypeDef* EA2, TIM_HandleTypeDef* EB1, TIM_HandleTypeDef* EB2);
// float Encoder_Calculate(unsigned char which,float interval);

#endif