//
// Created by 李唐 on 2023/11/13.
//

#ifndef DISPLAY_H
#define DISPLAY_H
#include "stm32f1xx_hal.h"
#include "Lib/LCD.h"
#include "string.h"
#include "stdlib.h"
#include "stdarg.h"
#include "stdio.h"

HAL_StatusTypeDef ShowHelloWorld();
HAL_StatusTypeDef DisplayNum(uint32_t num);
#endif //DISPLAY_H
