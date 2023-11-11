/*
 * @Author: LiTang
 * @Date: 2022-11-13 21:47:10
 * @LastEditTime: 2023-11-11 16:36:35
 */
#ifndef PIONEER_PAL_GLOBAL_H
#define PIONEER_PAL_GLOBAL_H

#include "stm32f1xx_hal.h"

//PAL basic function define
#define PAL_FUNC_UART
#define PAL_FUNC_PCA9685
#define PAL_FUNC_W25QXX
#define PAL_FUNC_MPU6050
//PAL advance function define
#ifdef PAL_FUNC_W25QXX
#define PAL_FUNC_W25QXX_MSC
#endif

typedef enum {
    false = 0x00,
    true = 0x01
} bool;

#endif //PIONEER_PAL_GLOBAL_H