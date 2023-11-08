/*
 * @Author: LiTang
 * @Date: 2023-11-08 11:21:14
 * @LastEditTime: 2023-11-08 11:27:07
 */
#ifndef JUMPAPPH
#define JUMPAPPH
#include "stm32f1xx_hal.h"
typedef void (*pFunction)(void);
void JumpApp(uint32_t AppAddress);

#endif