//
// Created by 李唐 on 2023/12/13.
//

#include "Lib/InternalFlash.h"
static uint8_t FlashReadByte(void)
{
    uint8_t t_data, r_data;

    if(HAL_SPI_TransmitReceive(&hspi1, &t_data, &r_data, 1, 0xFFFFFF) != HAL_OK)
    {
        r_data = 0xff;
    }
    return r_data;
}

static uint8_t FlashSendByte(uint8_t byte)
{
    uint8_t r_data;

    if(HAL_SPI_TransmitReceive(&hspi1, &byte, &r_data, 1, 0xFFFFFF) != HAL_OK)
    {
        return 1;
    }
    return 0;
}

static void FlashWriteEnable(void)
{
    HAL_GPIO_WritePin(FLASH_CS_GPIO_Port,FLASH_CS_Pin,GPIO_PIN_RESET);
    FlashSendByte(0x06);
    HAL_GPIO_WritePin(FLASH_CS_GPIO_Port,FLASH_CS_Pin,GPIO_PIN_SET);
}

static void FlashWaitWriteEnd(void)
{
    uint8_t state = 0;
    HAL_GPIO_WritePin(FLASH_CS_GPIO_Port,FLASH_CS_Pin,GPIO_PIN_RESET);
    FlashSendByte(0x05);
    do
    {
        state = FlashReadByte();
    }
    while((state & 0x01) == SET);
    HAL_GPIO_WritePin(FLASH_CS_GPIO_Port,FLASH_CS_Pin,GPIO_PIN_SET);
}

uint32_t FlashReadID(void)
{
    uint32_t temp, temp0, temp1, temp2;
    HAL_GPIO_WritePin(FLASH_CS_GPIO_Port,FLASH_CS_Pin,GPIO_PIN_RESET);
    FlashSendByte(0x9F);
    temp0 = FlashReadByte();
    temp1 = FlashReadByte();
    temp2 = FlashReadByte();
    HAL_GPIO_WritePin(FLASH_CS_GPIO_Port,FLASH_CS_Pin,GPIO_PIN_SET);
    temp = (temp0 << 16) | (temp1 << 8) | temp2;
    return temp;
}

void FlashReadAddr(uint32_t addr,uint8_t *pdata, uint16_t size)
{
    HAL_GPIO_WritePin(FLASH_CS_GPIO_Port,FLASH_CS_Pin,GPIO_PIN_RESET);
    FlashSendByte(0x03);

    FlashSendByte((addr & 0xFF0000) >> 16);
    FlashSendByte((addr & 0xFF00) >> 8);
    FlashSendByte(addr  & 0xFF);

    while (size--)
    {
        *pdata = FlashReadByte();
        pdata++;
    }

    HAL_GPIO_WritePin(FLASH_CS_GPIO_Port,FLASH_CS_Pin,GPIO_PIN_SET);
}

void FlashWritePage(uint32_t addr, uint8_t *pdata, uint16_t size)
{
    uint16_t i;

    FlashWriteEnable();

    HAL_GPIO_WritePin(FLASH_CS_GPIO_Port,FLASH_CS_Pin,GPIO_PIN_RESET);

    FlashSendByte(0x02);
    FlashSendByte((uint8_t)((addr)>>16));
    FlashSendByte((uint8_t)((addr)>>8));
    FlashSendByte((uint8_t)addr);

    for(i = 0; i < size; i++)
    {
        FlashSendByte(pdata[i]);
    }

    HAL_GPIO_WritePin(FLASH_CS_GPIO_Port,FLASH_CS_Pin,GPIO_PIN_SET);
    FlashWaitWriteEnd();
}

void FlashWriteAddr(uint32_t addr, uint8_t *pdata, uint32_t size)
{
    uint32_t page_remain;

    page_remain = 256 - addr%256;

    if(size <= page_remain)
    {
        page_remain = size;
    }
    while(1)
    {
        FlashWritePage(addr, pdata, page_remain);

        if(size == page_remain)
            break;
        else
        {
            pdata += page_remain;
            addr += page_remain;

            size -= page_remain;
            if(size > 256)
                page_remain = 256;
            else
                page_remain = size;
        }
    }
}

void FlashEraseSector(uint32_t sector_addr)
{
    FlashWriteEnable();
    FlashWaitWriteEnd();

    HAL_GPIO_WritePin(FLASH_CS_GPIO_Port,FLASH_CS_Pin,GPIO_PIN_RESET);
    FlashSendByte(0x20);
    FlashSendByte((sector_addr & 0xFF0000) >> 16);
    FlashSendByte((sector_addr & 0xFF00) >> 8);
    FlashSendByte(sector_addr & 0xFF);

    HAL_GPIO_WritePin(FLASH_CS_GPIO_Port,FLASH_CS_Pin,GPIO_PIN_SET);

    FlashWaitWriteEnd();
}

void FlashEraseBlock(void)
{
    FlashWriteEnable();

    HAL_GPIO_WritePin(FLASH_CS_GPIO_Port,FLASH_CS_Pin,GPIO_PIN_RESET);
    FlashSendByte(0xC7);
    HAL_GPIO_WritePin(FLASH_CS_GPIO_Port,FLASH_CS_Pin,GPIO_PIN_SET);

    FlashWaitWriteEnd();
}