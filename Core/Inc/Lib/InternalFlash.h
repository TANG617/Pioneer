//
// Created by 李唐 on 2023/12/13.
//

#ifndef FREERTOS_INTERNALFLASH_H
#define FREERTOS_INTERNALFLASH_H
#include "stm32f1xx_hal.h"
#include "spi.h"


uint32_t FlashReadID(void);
void FlashReadAddr(uint32_t addr,uint8_t *pdata, uint16_t size);
void FlashWritePage(uint32_t addr, uint8_t *pdata, uint16_t size);
void FlashWriteAddr(uint32_t addr, uint8_t *pdata, uint32_t size);
void FlashEraseSector(uint32_t sector_addr);
void FlashEraseBlock(void);
#endif //FREERTOS_INTERNALFLASH_H
