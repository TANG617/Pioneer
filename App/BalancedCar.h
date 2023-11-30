//
// Created by 李唐 on 2023/11/30.
//

#ifndef PIONEER_BALANCEDCAR_H
#define PIONEER_BALANCEDCAR_H
#include "stm32f1xx_hal.h"
#include "NodeMotor.h"
typedef struct{
    NodeMotorType LMotor, RMotor;
}BalancedCarType;
extern BalancedCarType Car;




HAL_StatusTypeDef BalancedCarInit();
HAL_StatusTypeDef BalancedCarUpdate();
#endif //PIONEER_BALANCEDCAR_H
