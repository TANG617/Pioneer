/*
 * @Author: LiTang
 * @Date: 2023-11-10 11:46:28
 * @LastEditTime: 2023-11-10 11:49:08
 */
#include "utilities.h"
void DebugPrint(char *p,...){
    va_list ap;
    va_start(ap,p);
    vsprintf(formatBuf,p,ap);
    va_end(ap);	
    HAL_UART_Transmit(&huart1,(uint8_t *)formatBuf,strlen(formatBuf),1000);
}