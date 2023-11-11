#include "pal_st7789.h"

extern SPI_HandleTypeDef hspi2;

void LCD_SendByte(uint8_t data){
  LCD_CS_L();
  HAL_SPI_Transmit(&hspi2,&data,1,1000);
  LCD_CS_H();
}

void LCD_Data8(uint8_t data){
  LCD_DC_H();
  LCD_SendByte(data);
}

void LCD_Data16(uint16_t data){
  LCD_DC_H();
  LCD_SendByte(data>>8);
  LCD_SendByte(data);
}

void LCD_Command(uint8_t cmd){
  LCD_DC_L();
  LCD_SendByte(cmd);
}

void LCD_AddrSet(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2){
  LCD_Command(0x2a);//列地址设置
  LCD_Data16(x1);
  LCD_Data16(x2);
  LCD_Command(0x2b);//行地址设置
  LCD_Data16(y1);
  LCD_Data16(y2);
  LCD_Command(0x2c);//储存器写
}

void LCD_Init(){
  LCD_BLK_L();//close background

  LCD_RST_L();
  HAL_Delay(50);
  LCD_RST_H();
  HAL_Delay(50);//reset

  LCD_BLK_H();//open background

  LCD_Command(0x11);
  HAL_Delay(10);
  LCD_Command(0x36);
  LCD_Data8(0x00);

  LCD_Command(0x3A);
  LCD_Data8(0x05);

  LCD_Command(0xB2);
  LCD_Data8(0x0C);
  LCD_Data8(0x0C);
  LCD_Data8(0x00);
  LCD_Data8(0x33);
  LCD_Data8(0x33);

  LCD_Command(0xB7);
  LCD_Data8(0x35);

  LCD_Command(0xBB);
  LCD_Data8(0x35);

  LCD_Command(0xC0);
  LCD_Data8(0x2C);

  LCD_Command(0xC2);
  LCD_Data8(0x01);

  LCD_Command(0xC3);
  LCD_Data8(0x13);

  LCD_Command(0xC4);
  LCD_Data8(0x20);

  LCD_Command(0xC6);
  LCD_Data8(0x0F);

  LCD_Command(0xD0);
  LCD_Data8(0xA4);
  LCD_Data8(0xA1);

  LCD_Command(0xD6);
  LCD_Data8(0xA1);

  LCD_Command(0xE0);
  LCD_Data8(0xF0);
  LCD_Data8(0x00);
  LCD_Data8(0x04);
  LCD_Data8(0x04);
  LCD_Data8(0x04);
  LCD_Data8(0x05);
  LCD_Data8(0x29);
  LCD_Data8(0x33);
  LCD_Data8(0x3E);
  LCD_Data8(0x38);
  LCD_Data8(0x12);
  LCD_Data8(0x12);
  LCD_Data8(0x28);
  LCD_Data8(0x30);

  LCD_Command(0xE1);
  LCD_Data8(0xF0);
  LCD_Data8(0x07);
  LCD_Data8(0x0A);
  LCD_Data8(0x0D);
  LCD_Data8(0x0B);
  LCD_Data8(0x07);
  LCD_Data8(0x28);
  LCD_Data8(0x33);
  LCD_Data8(0x3E);
  LCD_Data8(0x36);
  LCD_Data8(0x14);
  LCD_Data8(0x14);
  LCD_Data8(0x29);
  LCD_Data8(0x32);

  LCD_Command(0x21);

  LCD_Command(0x11);
  HAL_Delay(120);
  LCD_Command(0x29);
}

void LCD_Fill(uint16_t xsta,uint16_t ysta,uint16_t xend,uint16_t yend,uint16_t color)
{
  uint16_t i,j;
  LCD_AddrSet(xsta,ysta,xend-1,yend-1);//设置显示范围
  for(i=ysta;i<yend;i++)
  {
    for(j=xsta;j<xend;j++)
    {
      LCD_Data16(color);
    }
  }
}