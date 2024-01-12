
#include "Lib/MPU6050/debug.h"

#include <stdarg.h>		  //  标准头文件
#include <stdio.h>
#include <string.h>


extern   UART_HandleTypeDef huart1;//这里打开的是串口1，可以修改为别的串口


//   格式化可变变量字符串缓存
char  formatBuf[128];
//   格式化输出的调试函数
void FS_Debug(char *p,...)   // 与 printf  一摸一样的功能
{
    /*  格式化数据  */
    va_list ap;
    va_start(ap,p);
    vsprintf(formatBuf,p,ap);
    va_end(ap);

    //  编译控制
#ifdef DEBUG_EN
    //  信息输出接口
    HAL_UART_Transmit(&huart1,(uint8_t *)formatBuf,strlen(formatBuf),1000);

#endif
}


