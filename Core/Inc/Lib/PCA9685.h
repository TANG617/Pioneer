//
// Created by 李唐 on 2023/11/13.
//

#ifndef PCA9685_H
#define PCA9685_H
#include "stm32f1xx_hal.h"
#include "GLOBAL.h"


typedef enum {
    PCA9685_ACCEPTED = 0x00,
    PCA9685_ERROR = 0x01,
    PCA9685_BUSY = 0x02,
    PCA9685_TIMEOUT = 0x03,
    PCA9685_SLEEP = 0x04
} PAL_PCA9685_State;

typedef struct{
    uint8_t addr;
    uint32_t osc;
    HAL_StatusTypeDef (*i2c_tx)(uint8_t addr,uint8_t mem,uint8_t data);
    HAL_StatusTypeDef (*i2c_rx)(uint8_t addr,uint8_t mem,uint8_t *data);
} PAL_PCA9685;

#define PCA9685_IIC_ADDR (0x40<<1)
#define PCA9685_INTERNAL_OSC 25000000

#define PCA9685_BIT_RESET   7
#define PCA9685_BIT_EXTCLK  6
#define PCA9685_BIT_AI      5
#define PCA9685_BIT_SLEEP   4
#define PCA9685_BIT_SUBADR1 3
#define PCA9685_BIT_SUBADR2 2
#define PCA9685_BIT_SUBADR3 1
#define PCA9685_BIT_ALLCALL 0

#define PCA9685_REG_MODE1       0x00
#define PCA9685_REG_MODE2       0x01
#define PCA9685_REG_SUB_ADDR1   0x02
#define PCA9685_REG_SUB_ADDR2   0x03
#define PCA9685_REG_SUB_ADDR3   0x04
#define PCA9685_REG_ALL_CALL    0x05
#define PCA9685_REG_BASE_ON_L   0x06
#define PCA9685_REG_BASE_ON_H   0x07
#define PCA9685_REG_BASE_OFF_L  0x08
#define PCA9685_REG_BASE_OFF_H  0x09
#define PCA9685_REG_ALL_ON_L    0xFA
#define PCA9685_REG_ALL_ON_H    0xFB
#define PCA9685_REG_ALL_OFF_L   0xFC
#define PCA9685_REG_ALL_OFF_H   0xFD
#define PCA9685_REG_FREQ        0xFE


bool PAL_PCA9685_IsSleep(PAL_PCA9685* target);
PAL_PCA9685_State PAL_PCA9685_Sleep(PAL_PCA9685* target,bool sleep);
PAL_PCA9685_State PAL_PCA9685_SetFrequency(PAL_PCA9685* target ,float freq);
float PAL_PCA9685_GetFrequency(PAL_PCA9685* target);
PAL_PCA9685_State PAL_PCA9685_SetOutput(PAL_PCA9685* target,uint8_t channel,float ratio,float delay);
PAL_PCA9685_State PAL_PCA9685_SetAll(PAL_PCA9685* target,float ratio,float delay);
float PAL_PCA9685_GetRatio(PAL_PCA9685* target,uint8_t channel);
float PAL_PCA9685_GetDelay(PAL_PCA9685* target,uint8_t channel);
#endif //PCA9685_H
