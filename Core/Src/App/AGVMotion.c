//
// Created by 李唐 on 2023/12/16.
//

#include "App/AGVMotion.h"
HAL_StatusTypeDef AGV_MotionInit(AGV_MotionType *AGV_Car){
    AGV_Car->LMotor.CanHandler = &hcan;
    AGV_Car->LMotor.id = 0x01;
    AGV_Car->LMotor.Mode = Velocity;
    AGV_Car->LMotor.Velocity = 0;//nagative
    AGV_Car->LMotor.Direction = -1;

    AGV_Car->RMotor.CanHandler = &hcan;
    AGV_Car->RMotor.id = 0x02;
    AGV_Car->RMotor.Mode = Velocity;
    AGV_Car->RMotor.Velocity = 0;//positive
    AGV_Car->RMotor.Direction = 1;

    NodeMotorEnable(&AGV_Car->LMotor);
    osDelay(100);
    NodeMotorEnable(&AGV_Car->RMotor);
    osDelay(100);

    return HAL_OK;
}

HAL_StatusTypeDef AGV_MotionUpdate(AGV_MotionType *AGV_Car, float Velocity, float Rotate){
    AGV_Car->LMotor.Velocity = Velocity*AGV_Car->LMotor.Direction - Rotate*AGV_Car->LMotor.Direction;
    AGV_Car->RMotor.Velocity = Velocity*AGV_Car->RMotor.Direction + Rotate*AGV_Car->RMotor.Direction;

}

HAL_StatusTypeDef AGV_MotionMove(AGV_MotionType *AGV_Car){
    NodeMotorVelocityControl(&AGV_Car->LMotor);
    osDelay(10);
    NodeMotorVelocityControl(&AGV_Car->RMotor);
    osDelay(10);
}