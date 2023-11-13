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

HAL_StatusTypeDef DisplayNum(float num);
#endif //DISPLAY_H
