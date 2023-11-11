#include "pal_ssd1306.h"
#include "pal_oledFont.h"
#include "string.h"
#include "math.h"

const uint8_t scroll_speed[8]={0x03,0x02,0x01,0x06,
                               0x00,0x05,0x04,0x07};

uint8_t OLED_GRAM[128][8]; //128*64 GRAM buffer

__weak void OLED_WriteCmd(uint8_t cmd){}

__weak void OLED_WriteDat(uint8_t dat){}

__weak void OLED_Restart(){}

__weak uint8_t OLED_LoadFontASCII(uint8_t *target,uint8_t size,uint8_t chr,uint8_t seg){
return false;
}

__weak void OLED_Init(){
  //maybe should restart panel
  OLED_Restart();
  //close panel at first
  OLED_WriteCmd(OLED_CLOSE);
  //set OLED frequency and divider
  OLED_WriteCmd(OLED_CLOCK);
  OLED_WriteCmd(OLED_FREQ<<4|(OLED_DIVIDE&0x0F));
  //set OLED driver duty(row number)
  OLED_WriteCmd(OLED_DRIVER);
  OLED_WriteCmd(OLED_DUTY&0x3F);
  //set display vertical offset
  OLED_WriteCmd(OLED_OFFSET);
  OLED_WriteCmd(OLED_OFFSET_VER&0x3F);
  //set display begin row
  OLED_WriteCmd(OLED_BEGIN_ROW);
  //turn up charge pump
  OLED_WriteCmd(OLED_CHARGE_PUMP);
  OLED_WriteCmd(OLED_CHARGE);
  //set address index method of GRAM
  OLED_WriteCmd(OLED_INDEX_MEMORY);
  OLED_WriteCmd(OLED_INDEX_PAGE);
  //set horizon direction
  OLED_WriteCmd(OLED_HORIZON_RIGHT);
  //set vertical direction
  OLED_WriteCmd(OLED_VERTICAL_DOWN);
  //config COM pin settings
  OLED_WriteCmd(OLED_COM_PIN);
  OLED_WriteCmd((OLED_COM_REMAP<<5)|(OLED_COM_CFG<<4)|0x02);
  //set panel brightness
  OLED_WriteCmd(OLED_CONTRAST);
  OLED_WriteCmd(OLED_BRIGHTNESS);
  //set charge period 2 phases
  OLED_WriteCmd(OLED_CHARGE_PERIOD);
  OLED_WriteCmd(OLED_CHARGE_PAHSE1<<4|(OLED_CHARGE_PAHSE2&0x0F));
  //set voltage of VCOMH
  OLED_WriteCmd(OLED_VCOMH);
  OLED_WriteCmd(OLED_VCOMH_83);
  //set display from GRAM
  OLED_WriteCmd(OLED_LOAD_GRAM);
  //set panel display normally
  OLED_WriteCmd(OLED_NORMAL);
  //launch panel at last
  OLED_WriteCmd(OLED_START);
  //clear whole panel
  OLED_Clear(true);
  //stop all scroll
  OLED_ScrollStop();
}

void OLED_Refresh(){
  uint8_t page,col;
  for(page=0;page<8;page++){
    OLED_WriteCmd(OLED_PAGE_BASE+page);
    OLED_WriteCmd(ADDR_COL_L);
    OLED_WriteCmd(ADDR_COL_H);
    for(col=0;col<128;col++)
      OLED_WriteDat(OLED_GRAM[col][page]);
  }
}

void OLED_FillAll(uint8_t content){
  memset(OLED_GRAM,content,sizeof(OLED_GRAM));
}

void OLED_Update(uint8_t col1,uint8_t col2,uint8_t page1,uint8_t page2){
  if(col1>127||col2>127||page1>7||page2>7) return;
  page1=7-page1;
  page2=7-page2;
  uint8_t tmp;
  if(col1>col2){tmp=col1;col1=col2;col2=tmp;}
  if(page1>page2){tmp=page1;page1=page2;page2=tmp;}
  uint8_t page,col;
  for(page=page1;page<=page2;page++){
    OLED_WriteCmd(OLED_PAGE_BASE+page);
    OLED_WriteCmd(ADDR_COL_L|(col1&0x0F));
    OLED_WriteCmd(ADDR_COL_H|(col1>>4));
    for(col=col1;col<=col2;col++)
      OLED_WriteDat(OLED_GRAM[col][page]);
  }
}

void OLED_Clear(uint8_t push){
  memset(OLED_GRAM,0x00,sizeof(OLED_GRAM));
  if(push) OLED_Refresh();
}

void OLED_Launch(){
  OLED_WriteCmd(OLED_CHARGE_PUMP);
  OLED_WriteCmd(OLED_CHARGE);
  OLED_WriteCmd(OLED_START);
}

void OLED_Shutdown(){
  OLED_WriteCmd(OLED_CHARGE_PUMP);
  OLED_WriteCmd(OLED_DISCHARGE);
  OLED_WriteCmd(OLED_CLOSE);
}

void OLED_ScrollStop(){
  OLED_WriteCmd(OLED_SCROLL_STOP);
}

void OLED_ScrollHorizon(uint8_t left,uint8_t hold1,uint8_t hold2,uint8_t speed){
  if(hold1>7||hold2>7||hold1==hold2||speed>0x07) return;
  hold1=0x07-hold1;
  hold2=0x07-hold2;
  OLED_ScrollStop();
  if(left) OLED_WriteCmd(OLED_SCROLL_HORIZON_L);
  else OLED_WriteCmd(OLED_SCROLL_HORIZON_R);
  OLED_WriteCmd(OLED_DUMMY0);
  OLED_WriteCmd(hold1<hold2?hold1:hold2);
  OLED_WriteCmd(scroll_speed[speed]);
  OLED_WriteCmd(hold1>hold2?hold1:hold2);
  OLED_WriteCmd(OLED_DUMMY0);
  OLED_WriteCmd(OLED_DUMMY1);
  OLED_Refresh();
  OLED_WriteCmd(OLED_SCROLL_BEGIN);
}

void OLED_ScrollVertical(uint8_t up,uint8_t hold1,uint8_t hold2,uint8_t speed){
  if(hold1>0x3F||hold2>0x3F||hold1==hold2||speed>0x07) return;
  hold1=0x3F-hold1;
  hold2=0x3F-hold2;
  OLED_ScrollStop();
  OLED_WriteCmd(OLED_SCROLL_OFFSET);
  OLED_WriteCmd(hold1<hold2?hold1:hold2);
  uint8_t area=(hold1<hold2?(hold2-hold1+1):(hold1-hold2+1));
  OLED_WriteCmd(area);
  OLED_WriteCmd(OLED_SCROLL_VERTICAL_R);
  //OLED_WriteCmd(OLED_SCROLL_VERTICAL_L);
  OLED_WriteCmd(OLED_DUMMY0);
  OLED_WriteCmd(0x00);
  OLED_WriteCmd(scroll_speed[speed]);
  OLED_WriteCmd(0x07);
  if(up) OLED_WriteCmd(area-0x01);
  else OLED_WriteCmd(0x01);
  OLED_Refresh();
  OLED_WriteCmd(OLED_SCROLL_BEGIN);
}

void OLED_DisplayMode(uint8_t normal){
  if(normal) OLED_WriteCmd(OLED_NORMAL);
  else OLED_WriteCmd(OLED_INVERSE);
}

void OLED_Point(uint8_t x,uint8_t y,uint8_t draw){
  if(x>127||y>63) return;
  uint8_t page =7-y/8;
  uint8_t newByte=1<<(7-y%8);
  if(draw) OLED_GRAM[x][page]|=newByte;
  else OLED_GRAM[x][page]&=~newByte;
}

void OLED_Line(uint8_t x1,uint8_t y1,uint8_t x2,uint8_t y2,uint8_t draw){
  if(x1>127||x2>127||y1>63||y2>63) return;
  float dy=(float)y1-(float)y2,dx=(float)x1-(float)x2;
  dy>0?dy++:dy--;
  dx>0?dx++:dx--;
  float k;
  uint8_t begin,end,base;
  float tmp;
  if(fabsf(dy)<=fabsf(dx)){
    k=dy/dx;
    x1<=x2?(begin=x1,end=x2,base=y1):(begin=x2,end=x1,base=y2);
    for(uint8_t i=begin;i<=end;i++){
      tmp=(float)base+(float)(i-begin)*k;
      OLED_Point(i,(uint8_t)tmp,draw);
    }
  }else{
    k=dx/dy;
    y1<=y2?(begin=y1,end=y2,base=x1):(begin=y2,end=y1,base=x2);
    for(uint8_t i=begin;i<=end;i++){
      tmp=(float)base+(float)(i-begin)*k;
      OLED_Point((uint8_t)tmp,i,draw);
    }
  }
}

void OLED_Rect(uint8_t x1,uint8_t y1,uint8_t x2,uint8_t y2,uint8_t draw){
  OLED_Line(x1,y1,x2,y1,draw);
  OLED_Line(x1,y2,x2,y2,draw);
  OLED_Line(x1,y1,x1,y2,draw);
  OLED_Line(x2,y1,x2,y2,draw);
}

void OLED_FillRect(uint8_t x1,uint8_t y1,uint8_t x2,uint8_t y2,uint8_t draw){
  if(x1>127||x2>127||y1>63||y2>63) return;
  uint8_t lx,ly,x,y;
  x1<=x2?(x=x1,lx=x2-x1):(x=x2,lx=x1-x2);
  y1<=y2?(y=y1,ly=y2-y1):(y=y2,ly=y1-y2);
  for(uint8_t i=0;i<lx;i++)
    for(uint8_t j=0;j<ly;j++)
      OLED_Point(x+i,y+j,draw);
}

void Circle_Symmetry8(uint8_t xCtr,uint8_t yCtr,uint8_t xOff,uint8_t yOff,uint8_t draw){
  OLED_Point(xCtr+xOff,yCtr+yOff,draw);
  OLED_Point(xCtr-xOff,yCtr+yOff,draw);
  OLED_Point(xCtr+xOff,yCtr-yOff,draw);
  OLED_Point(xCtr-xOff,yCtr-yOff,draw);
  OLED_Point(xCtr+yOff,yCtr+xOff,draw);
  OLED_Point(xCtr+yOff,yCtr-xOff,draw);
  OLED_Point(xCtr-yOff,yCtr+xOff,draw);
  OLED_Point(xCtr-yOff,yCtr-xOff,draw);
}

void OLED_Circle(uint8_t cx,uint8_t cy,uint8_t r,uint8_t draw){
  if(cx<r||cy<r||cx+r>127||cy+r>63) return;
  uint8_t x=0,y=r;
  int16_t r1,r2;
  while(x<=y){
    Circle_Symmetry8(cx,cy,x,y,draw);
    x++;
    r1=x*x+y*y;
    r1-=r*r;
    r2=x*x+(y-1)*(y-1);
    r2-=r*r;
    if(r2<0) r2=-r2;
    if(r1>r2) y--;
  }
}

void OLED_FillCircle(uint8_t cx,uint8_t cy,uint8_t r,uint8_t draw){
  if(cx<r||cy<r||cx+r>127||cy+r>63) return;
  uint8_t x=0,y=r;
  int16_t r1,r2;
  while(x<=y){
    for(uint8_t i=x;i<=y;i++)
      Circle_Symmetry8(cx,cy,x,i,draw);
    x++;
    r1=x*x+y*y;
    r1-=r*r;
    r2=x*x+(y-1)*(y-1);
    r2-=r*r;
    if(r2<0) r2=-r2;
    if(r1>r2) y--;
  }
}

void OLED_CharASCII(uint8_t x,uint8_t y,unsigned char chr,uint8_t size,uint8_t draw){
  uint8_t bytes=(size/8+(size%8?1:0))*(size/2),baseY=y;
  uint8_t tmp,i,j;
  chr-=' ';
  if(chr>=95) return;

  for(i=0;i<bytes;i++){
    switch(size) {
      case 12 : tmp = ascii_1206[chr][i];break;
      case 16 : tmp = ascii_1608[chr][i];break;
      case 24 : tmp = ascii_2412[chr][i];break;
      default : if(!OLED_LoadFontASCII(&tmp,size,chr,i))
          return;
    }
    for(j=0;j<8;j++){
      if(tmp&0x80) OLED_Point(x,y,draw);
      else OLED_Point(x,y,!draw);
      tmp<<=1;
      y++;
      if((y-baseY)==size){y=baseY;x++;break;}
    }
  }
}

void OLED_StringASCII(uint8_t x,uint8_t y,const unsigned char *s,uint8_t size,uint8_t draw){
  while((*s<='~')&&(*s>=' ')){
    if(x>(128-(size/2))){x=0;y+=size;}
    if(y>(64-size)){y=0;x=0;OLED_Clear(false);}
    OLED_CharASCII(x,y,*s,size,draw);
    x+=size/2;
    s++;
  }
}
