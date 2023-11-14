//
// Created by 李唐 on 2023/11/14.
//

#include "UART.h"

const char PAL_UART_END[]="\r\n";
const uint16_t PAL_UART_END_SIZE=2;
PAL_UART* PAL_UART_GLOBAL[5]={NULL,NULL,NULL,NULL,NULL};
uint8_t PAL_UART_CNT=0;

PAL_UART_GlobalState PAL_UART_AddGlobal(PAL_UART* target){
  for(uint8_t i=0;i<PAL_UART_CNT;i++)
    if(target->handle->Instance==PAL_UART_GLOBAL[i]->handle->Instance) return UART_REDEFINE;
  if(PAL_UART_CNT>=5) return UART_OVERFLOW;
  PAL_UART_GLOBAL[PAL_UART_CNT++] = target;
  return UART_ACCEPTED;
}

PAL_UART_GlobalState PAL_UART_RemoveGlobal(PAL_UART* target){
  uint8_t index=5;
  for(uint8_t i=0;i<PAL_UART_CNT;i++)
    if(target->handle->Instance==PAL_UART_GLOBAL[i]->handle->Instance){
      index = i;
      break;
    }
  if(index==5) return UART_NOTFOUND;
  for(uint8_t i=index;i<PAL_UART_CNT-1;i++)
    PAL_UART_GLOBAL[i] = PAL_UART_GLOBAL[i+1];
  PAL_UART_GLOBAL[PAL_UART_CNT--] = NULL;
  return UART_ACCEPTED;
}

void PAL_UART_SpawnBuffer(PAL_UART* target,uint16_t size){
  if(target->rx_buffer==NULL)
    target->rx_buffer = (uint8_t*)malloc(sizeof(uint8_t)*size);
  else
    target->rx_buffer = (uint8_t*)realloc(target->rx_buffer,sizeof(uint8_t)*size);
  target->rx_size = size;
  PAL_UART_Clear(target);
}

void PAL_UART_SetEndString(PAL_UART* target,const char* end,uint16_t size){
  if(size<1) return;
  if(target->rx_end==NULL)
    target->rx_end = (uint8_t*)malloc(sizeof(uint8_t)*size);
  else
    target->rx_end = (uint8_t*)realloc(target->rx_end,sizeof(uint8_t)*size);
  target->end_size = size;
  for(uint16_t i=0;i<target->end_size;i++)
    target->rx_end[i] = end[i];
}

void PAL_UART_Clear(PAL_UART* target){
  if(target->rx_size==0||target->rx_buffer==NULL) return;
  target->rx_cnt = 0;
  memset(target->rx_buffer,0,sizeof(uint8_t)*target->rx_size);
}

HAL_StatusTypeDef PAL_UART_Printf(PAL_UART* target,const char* format,...){
  va_list args;
  uint16_t length;
  uint8_t buffer[PAL_UART_MAX_TX_SIZE];
  va_start(args,format);
  length = vsnprintf((char*)buffer,PAL_UART_MAX_TX_SIZE,format,args);
  va_end(args);
  if(target->timeout==0) target->timeout = PAL_UART_TIMEOUT;
  return HAL_UART_Transmit(target->handle,buffer,length,target->timeout);
}

void PAL_UART_StartReceive(PAL_UART* target){
  target->onReceive = true;
  HAL_UART_Receive_IT(target->handle,&(target->rx),1);
}

void PAL_UART_StopReceive(PAL_UART* target){
  target->onReceive = false;
}

bool PAL_UART_CheckEnd(PAL_UART* target){
  if(target->rx_end==NULL||target->end_size==0)
    PAL_UART_SetEndString(target,PAL_UART_END,PAL_UART_END_SIZE);
  if(target->rx_cnt<target->end_size) return false;
  for(uint16_t i=target->end_size,j=1;i>0;i--,j++)
    if(target->rx_buffer[target->rx_cnt-j]!=target->rx_end[i-1]) return false;
  return true;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
  uint8_t index=5;
  for(uint8_t i=0;i<PAL_UART_CNT;i++)
    if(PAL_UART_GLOBAL[i]->handle->Instance==huart->Instance){
      index = i;
      break;
    }
  if(index==5) return;

  PAL_UART* target = PAL_UART_GLOBAL[index];
  if(!target->onReceive) return;
  if(target->rx_buffer==NULL||target->rx_size==0) return;

  if(target->rx_cnt>=target->rx_size-1){
    if(target->process!=NULL) target->process(target->rx_buffer);
    PAL_UART_Clear(target);
  }
  else{
    target->rx_buffer[target->rx_cnt++] = target->rx;

    if(PAL_UART_CheckEnd(target)){
      if(target->process!=NULL) target->process(target->rx_buffer);
      PAL_UART_Clear(target);
    }
  }
  if(target->onReceive) HAL_UART_Receive_IT(target->handle,&(target->rx),1);
}