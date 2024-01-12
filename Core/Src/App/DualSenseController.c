//
// Created by 李唐 on 2023/11/14.
//

#include "App/DualSenseController.h"
extern UART_HandleTypeDef huart2;
// DSC_Type *_DSC;
//
// void DSC_Process(uint8_t *buffer)
uint8_t* DSC_GET(int16_t *data)
{
    uint8_t buf30[30];
    uint8_t buf1[] = "\0";
    while(*buf1 != '#')
    {
        HAL_UART_Receive(&huart2,buf1,1,HAL_MAX_DELAY);
    }
    for (uint8_t i = 0; i<30; i++)
    {
        HAL_UART_Receive(&huart2, buf30+i ,1,HAL_MAX_DELAY);
    }
    // return buf30;

    uint8_t index = 0;
    DSC_KEYS dsc_keys = 0;
    int16_t value = 0;
    int8_t sign = 1;
    while(buf30[index] != '#')
        // while(index<5)
    {
        if(buf30[index] != '_')
        {
            value *= 10;
            if(buf30[index] == '-')
            {
                sign = -1;
            }
            else
            {
                value += buf30[index] - '0';
            }
        }
        else
        {
            value *= sign;
            // _DSC->Data[dsc_keys++] = value;
            data[dsc_keys++] = value;
            sign = 1;
            value = 0;
        }
        index++;
    }
    value *= sign;
    // _DSC->Data[dsc_keys++] = value;
    data[dsc_keys++] = value;
}

// void DSC_Process(uint8_t *buffer,int16_t *data)
// {
//     uint8_t index = 0;
//     DSC_KEYS dsc_keys = 0;
//     int16_t value = 0;
//     int8_t sign = 1;
//     while(buffer[index] != '#')
//     // while(index<5)
//     {
//         if(buffer[index] != '_')
//         {
//             value *= 10;
//             if(buffer[index] == '-')
//             {
//                 sign = -1;
//             }
//             else
//             {
//                 value += buffer[index] - '0';
//             }
//         }
//         else
//         {
//             value *= sign;
//             // _DSC->Data[dsc_keys++] = value;
//             data[dsc_keys++] = value;
//             sign = 1;
//             value = 0;
//         }
//         index++;
//     }
//     value *= sign;
//     // _DSC->Data[dsc_keys++] = value;
//     data[dsc_keys++] = value;
// }
//
// HAL_StatusTypeDef DSC_Init(int16_t* _Data)
// {
//     _DSC->DSC_UART.handle = &huart2;
//     _DSC->DSC_UART.timeout = HAL_MAX_DELAY;
//     _DSC->DSC_UART.process = DSC_Process;
//     PAL_UART_SetEndString(&_DSC->DSC_UART,"#",1);
//     PAL_UART_AddGlobal(&_DSC->DSC_UART);
//     _DSC->Data = _Data;
//     PAL_UART_StartReceive(&_DSC->DSC_UART);
//
//     return HAL_OK;
// }