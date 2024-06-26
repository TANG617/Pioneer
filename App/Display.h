//
// Created by 李唐 on 2023/11/13.
//

#ifndef DISPLAY_H
#define DISPLAY_H
#include "stm32f1xx_hal.h"
#include "LCD.h"
#include "string.h"
#include "stdlib.h"
#include "stdarg.h"
#include "stdio.h"
#include "Motion.h"
#include "DualSenseController.h"


HAL_StatusTypeDef DisplayNum(float num);
HAL_StatusTypeDef ShowWheelStatus(MotionType *_Car);
HAL_StatusTypeDef ShowDSCStatus(int16_t *Data);
HAL_StatusTypeDef ShowHelloWorld();
#endif //DISPLAY_H
