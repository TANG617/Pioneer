//
// Created by 李唐 on 2023/12/16.
//

#ifndef PIONEER_AGVMOTION_H
#define PIONEER_AGVMOTION_H
#include "Lib/NodeMotor.h"
#include "cmsis_os2.h"


typedef struct{
    NodeMotorType LMotor,RMotor;
    float Velocity;

}AGV_MotionType;

HAL_StatusTypeDef AGV_MotionMove(AGV_MotionType *AGV_Car);
HAL_StatusTypeDef AGV_MotionUpdate(AGV_MotionType *AGV_Car, float Velocity, float Rotate);
HAL_StatusTypeDef AGV_MotionInit(AGV_MotionType *AGV_Car);
#endif //PIONEER_AGVMOTION_H
