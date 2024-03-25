/*
 * @Author: LiTang
 * @Date: 2023-11-08 11:21:02
 * @LastEditTime: 2023-11-08 16:29:05
 */
#include "jumpApp.h"
void JumpApp(uint32_t AppAddress){
    pFunction Jump;
    uint32_t JumpAddress;
    JumpAddress = *(__IO uint32_t *)( AppAddress + 4);
    Jump = (pFunction)JumpAddress;
    SCB->VTOR = AppAddress;
    __set_MSP(*(__IO uint32_t *)AppAddress);
    Jump();
}