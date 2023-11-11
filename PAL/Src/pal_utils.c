#include "pal_utils.h"

/*!
 * @brief Delay Function for microseconds
 * Based on HAL_Delay(uint32_t Delay) and SysTick
 * @param delay: microseconds you what to delay
 * @retval None
 */
__weak void PAL_Delay(__IO uint32_t delay){
  if(delay>=1000) HAL_Delay(delay/1000);
  if(delay%1000==0) return;

  int last, curr, val;
  int temp;
  while (delay != 0){
    temp = delay > 900 ? 900 : delay;
    last = SysTick->VAL;
    curr = last - CPU_FREQUENCY_MHZ * temp;
    if (curr >= 0)
      do {val = SysTick->VAL;}
      while ((val < last) && (val >= curr));
    else{
      curr += CPU_FREQUENCY_MHZ * 1000;
      do {val = SysTick->VAL;}
      while ((val <= last) || (val > curr));
    }
    delay -= temp;
  }
}

void PAL_USB_Rest(){
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  __HAL_RCC_GPIOA_CLK_ENABLE();
  GPIO_InitStruct.Pin=GPIO_PIN_12;
  GPIO_InitStruct.Mode=GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull=GPIO_NOPULL;
  GPIO_InitStruct.Speed=GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA,&GPIO_InitStruct);
  HAL_GPIO_WritePin(GPIOA,GPIO_PIN_12,GPIO_PIN_RESET);
  HAL_Delay(100);
  HAL_GPIO_WritePin(GPIOA,GPIO_PIN_12,GPIO_PIN_SET);
}

bool PAL_StringBeginWith(char* string,char* begin){
  char* s = string;
  char* b = begin;
  if(s==NULL||(*s)=='\0') return false;
  if(b==NULL||(*b)=='\0') return false;
  while((s!=NULL&&(*s)!='\0')&&(b!=NULL&&(*b)!='\0')){
    if(*s != *b) return false;
    s++;
    b++;
  }
  if((s==NULL||(*s)=='\0')&&(b!=NULL&&(*b)!='\0')) return false;
  return true;
}