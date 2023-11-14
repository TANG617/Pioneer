//
// Created by 李唐 on 2023/11/14.
//

#ifndef UART_H
#define UART_H
#include "stm32f1xx_hal.h"
#include "string.h"
#include "stdlib.h"
#include "stdarg.h"
#include "stdio.h"
#include "GLOBAL.h"
#define PAL_UART_MAX_TX_SIZE 256
#define PAL_UART_TIMEOUT 1000

typedef struct {
    UART_HandleTypeDef* handle;
    uint16_t rx_size,rx_cnt,end_size;
    uint8_t *rx_buffer,*rx_end;
    uint32_t timeout;
    uint8_t rx;
    bool onReceive;
    void (*process)(uint8_t*);
} PAL_UART;

typedef enum {
    UART_ACCEPTED = 0x00,
    UART_OVERFLOW = 0x01,
    UART_REDEFINE = 0x02,
    UART_NOTFOUND = 0x03
} PAL_UART_GlobalState;

PAL_UART_GlobalState PAL_UART_AddGlobal(PAL_UART* target);
PAL_UART_GlobalState PAL_UART_RemoveGlobal(PAL_UART* target);
void PAL_UART_SpawnBuffer(PAL_UART* target,uint16_t size);
void PAL_UART_SetEndString(PAL_UART* target,const char* end,uint16_t size);
void PAL_UART_Clear(PAL_UART* target);
HAL_StatusTypeDef PAL_UART_Printf(PAL_UART*target,const char* format,...);
void PAL_UART_StartReceive(PAL_UART* target);
void PAL_UART_StopReceive(PAL_UART* target);
bool PAL_UART_CheckEnd(PAL_UART* target);
#endif //UART_H
