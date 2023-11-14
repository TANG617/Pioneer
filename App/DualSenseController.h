//
// Created by 李唐 on 2023/11/14.
//

#ifndef DUALSENSECONTROLLER_H
#define DUALSENSECONTROLLER_H
#include "stm32f1xx_hal.h"
#include "UART.h"

typedef enum {
    LStickX = 0,
    LStickY = 1,
    RStickX = 2,
    RStickY = 3,
    L2Value = 4,
    R2Value = 5
} DSC_KEYS;
typedef struct
{
    PAL_UART DSC_UART;
    int16_t* Data;
}DSC_Type;

extern DSC_Type *_DSC;

// HAL_StatusTypeDef DSC_Init(int16_t* _Data);
// void DSC_Process(uint8_t *buffer);//debug
// void DSC_Process(uint8_t *buffer,int16_t *data);//debug
// uint8_t* DSC_GET(uint8_t buf30[30]);
uint8_t* DSC_GET(int16_t *data);
#endif //DUALSENSECONTROLLER_H
