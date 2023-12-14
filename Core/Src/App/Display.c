//
// Created by 李唐 on 2023/11/13.
//

#include "App/Display.h"
#include "Lib/LCD_Image.h"

HAL_StatusTypeDef ShowHelloWorld()
{
    LCD_Show_Image(0,0,240,240,gImage_homer);
}

HAL_StatusTypeDef DisplayNum(uint32_t num)
{
    LCD_ShowNum(0, 0, num, 12, 32);
    return HAL_OK;
}