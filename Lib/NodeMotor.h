//
// Created by 李唐 on 2023/11/29.
//

#ifndef PIONEER_NODEMOTOR_H
#define PIONEER_NODEMOTOR_H
#include "can.h"
#include "stm32f1xx_hal.h"

#define P_MIN     0
#define P_MAX     0
#define V_MIN     0
#define V_MAX     0
#define KP_MIN    0
#define KP_MAX    0
#define KD_MIN    0
#define KD_MAX    0
#define T_MIN     0
#define T_MAX     0



typedef struct{
    CAN_HandleTypeDef *CanHandler;
    CAN_TxHeaderTypeDef TxHeader;
    uint16_t id;
    float Position, Velocity, Kp, Kd, Torque;
}NodeMotorType;

HAL_StatusTypeDef NodeMotorClearError(NodeMotorType *Motor);
HAL_StatusTypeDef NodeMotorEnable(NodeMotorType *Motor);
HAL_StatusTypeDef NodeMotorDisable(NodeMotorType *Motor);
HAL_StatusTypeDef NodeMotorSaveZero(NodeMotorType *Motor);
HAL_StatusTypeDef NodeMotorMITControl(NodeMotorType *Motor);

#endif //PIONEER_NODEMOTOR_H
