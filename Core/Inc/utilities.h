/*
 * @Author: LiTang
 * @Date: 2023-11-10 11:46:36
 * @LastEditTime: 2023-11-10 11:47:57
 */
#ifndef UTILITIES_H
#define UTILITIES_H
#include "stm32f1xx_hal.h"
#include <stdarg.h>		  //  标准头文件
#include <stdio.h>
#include <string.h>


extern   UART_HandleTypeDef huart1;
char  formatBuf[128];

void DebugPrint(char *p,...);

#endif