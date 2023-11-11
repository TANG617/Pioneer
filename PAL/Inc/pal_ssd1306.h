#ifndef PIONEER_PAL_SSD1306_H
#define PIONEER_PAL_SSD1306_H

#include "pal_global.h"

#define OLED_IIC_ADDR           0x78
#define OLED_IIC_CMD_ADDR       0x00
#define OLED_IIC_DAT_ADDR       0x40

//basic commands
#define OLED_CONTRAST           0x81
#define OLED_BRIGHTNESS         0xEF
#define OLED_LOAD_GRAM          0xA4
#define OLED_LOAD_ALL           0xA5
#define OLED_NORMAL             0xA6
#define OLED_INVERSE            0xA7
#define OLED_CLOSE              0xAE
#define OLED_START              0xAF
//scroll commands
#define OLED_SCROLL_STOP        0x2E
#define OLED_SCROLL_BEGIN       0x2F
#define OLED_DUMMY0             0x00
#define OLED_DUMMY1             0xFF
#define OLED_SCROLL_HORIZON_R   0x26
#define OLED_SCROLL_HORIZON_L   0x27
#define OLED_SCROLL_VERTICAL_R  0x29
#define OLED_SCROLL_VERTICAL_L  0x2A
#define OLED_SCROLL_OFFSET      0xA3
//address commands
#define ADDR_COL_L              0x00
#define ADDR_COL_H              0x10
#define OLED_INDEX_MEMORY       0x20
#define OLED_INDEX_HORIZON      0x00
#define OLED_INDEX_VERTICAL     Ox01
#define OLED_INDEX_PAGE         0x02
#define OLED_ADDR_COL           0x21
#define OLED_COL_BEGIN          0x00
#define OLED_COL_END            0x7F
#define OLED_ADDR_PAGE          0x22
#define OLED_PAGE_BEGIN         0x00
#define OLED_PAGE_END           0x07
#define OLED_PAGE_BASE          0xB0
//hardware commands
#define OLED_BEGIN_ROW          0x40
#define OLED_HORIZON_LEFT       0xA0
#define OLED_HORIZON_RIGHT      0xA1
#define OLED_DRIVER             0xA8
#define OLED_DUTY               0x3F
#define OLED_VERTICAL_DOWN      0xC0
#define OLED_VERTICAL_UP        0xC8
#define OLED_OFFSET             0xD3
#define OLED_OFFSET_VER         0x00
#define OLED_COM_PIN            0xDA
#define OLED_COM_REMAP          false
#define OLED_COM_CFG            true
//clock and electronic commands
#define OLED_CLOCK              0xD5
#define OLED_FREQ               0x08
#define OLED_DIVIDE             0x00
#define OLED_CHARGE_PERIOD      0xD9
#define OLED_CHARGE_PAHSE1      0x0F
#define OLED_CHARGE_PAHSE2      0x01
#define OLED_VCOMH              0xDB
#define OLED_VCOMH_65           0x00
#define OLED_VCOMH_77           0x20
#define OLED_VCOMH_83           0x30
#define OLED_NOP                0xE3
#define OLED_CHARGE_PUMP        0x8D
#define OLED_CHARGE             0x14
#define OLED_DISCHARGE          0x10

void OLED_WriteCmd(uint8_t cmd);
void OLED_WriteDat(uint8_t dat);
void OLED_Refresh();
void OLED_Update(uint8_t col1,uint8_t col2,uint8_t page1,uint8_t page2);
void OLED_Clear(uint8_t push);
void OLED_FillAll(uint8_t content);
void OLED_Init();
void OLED_Launch();
void OLED_Shutdown();
void OLED_Restart();
void OLED_ScrollStop();
void OLED_ScrollHorizon(uint8_t left,uint8_t hold1,uint8_t hold2,uint8_t speed);
void OLED_ScrollVertical(uint8_t up,uint8_t hold1,uint8_t hold2,uint8_t speed);
void OLED_DisplayMode(uint8_t normal);
void OLED_Point(uint8_t x,uint8_t y,uint8_t draw);
void OLED_Line(uint8_t x1,uint8_t y1,uint8_t x2,uint8_t y2,uint8_t draw);
void OLED_Rect(uint8_t x1,uint8_t y1,uint8_t x2,uint8_t y2,uint8_t draw);
void OLED_FillRect(uint8_t x,uint8_t y,uint8_t lx,uint8_t ly,uint8_t draw);
void OLED_Circle(uint8_t cx,uint8_t cy,uint8_t r,uint8_t draw);
void OLED_FillCircle(uint8_t cx,uint8_t cy,uint8_t r,uint8_t draw);
uint8_t OLED_LoadFontASCII(uint8_t *target,uint8_t size,uint8_t chr,uint8_t seg);
void OLED_CharASCII(uint8_t x,uint8_t y,unsigned char chr,uint8_t size,uint8_t draw);
void OLED_StringASCII(uint8_t x,uint8_t y,const unsigned char *s,uint8_t size,uint8_t draw);

#endif //PIONEER_PAL_SSD1306_H