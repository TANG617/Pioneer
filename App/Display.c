//
// Created by 李唐 on 2023/11/13.
//

#include "Display.h"
HAL_StatusTypeDef DisplayNum(float num)
{
    LCD_ShowNum(0, 0, num*10000, 6, 32);
    return HAL_OK;
}