/*
 * @Author: LiTang
 * @Date: 2022-11-13 21:47:10
 * @LastEditTime: 2023-11-11 16:44:39
 */
#ifndef PIONEER_PAL_ST7789_H
#define PIONEER_PAL_ST7789_H

#include "pal_global.h"
#include "pal_utils.h"

#define LCD_DC_H() HAL_GPIO_WritePin(GPIOB,GPIO_PIN_5,GPIO_PIN_SET)
#define LCD_DC_L() HAL_GPIO_WritePin(GPIOB,GPIO_PIN_5,GPIO_PIN_RESET)

#define LCD_CS_H() HAL_GPIO_WritePin(GPIOB,GPIO_PIN_4,GPIO_PIN_SET)
#define LCD_CS_L() HAL_GPIO_WritePin(GPIOB,GPIO_PIN_4,GPIO_PIN_RESET)

#define LCD_RST_H() HAL_GPIO_WritePin(GPIOA,GPIO_PIN_8,GPIO_PIN_SET)
#define LCD_RST_L() HAL_GPIO_WritePin(GPIOA,GPIO_PIN_8,GPIO_PIN_RESET)

#define LCD_BLK_H() HAL_GPIO_WritePin(GPIOC,GPIO_PIN_5,GPIO_PIN_SET)
#define LCD_BLK_L() HAL_GPIO_WritePin(GPIOC,GPIO_PIN_5,GPIO_PIN_RESET)

#define LCD_W 240
#define LCD_H 240

void LCD_Init();
void LCD_AddrSet(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2);
void LCD_Fill(uint16_t xsta,uint16_t ysta,uint16_t xend,uint16_t yend,uint16_t color);

#endif //PIONEER_PAL_ST7789_H
