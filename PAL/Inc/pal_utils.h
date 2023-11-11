/*
 * @Author: LiTang
 * @Date: 2022-11-13 21:47:10
 * @LastEditTime: 2023-11-11 16:32:43
 */
#ifndef PIONEER_PAL_UTILS_H
#define PIONEER_PAL_UTILS_H

#include "pal_global.h"

#define CPU_FREQUENCY_MHZ 72
void PAL_Delay(__IO uint32_t delay);
void PAL_USB_Rest();

bool PAL_StringBeginWith(char* string,char* begin);

#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))

#endif //PIONEER_PAL_UTILS_H
