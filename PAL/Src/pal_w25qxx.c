/*
 * @Author: LiTang
 * @Date: 2022-11-13 21:47:10
 * @LastEditTime: 2023-11-11 16:44:07
 */
#include "pal_w25qxx.h"
#include "pal_utils.h"
#include "stdlib.h"

#ifdef PAL_FUNC_W25QXX

__weak PAL_W25QXX_State PAL_W25QXX_Init(PAL_W25QXX* target){
  if(target->chip_select==NULL) return W25QXX_ERROR;
  target->chip_select(false);
  target->sleep = false;
  PAL_Delay(W25QXX_DelayInterval);

  PAL_W25QXX_State status;
  uint8_t cmd,reg1,reg2;
  cmd = W25QXX_ReadStatus1;
  status = PAL_W25QXX_Command(target,&cmd,1,&reg1,1);
  if(status!=W25QXX_ACCEPTED) return status;
  cmd = W25QXX_ReadStatus2;
  status = PAL_W25QXX_Command(target,&cmd,1,&reg2,1);
  if(status!=W25QXX_ACCEPTED) return status;
  reg1 &= 0b01111111;
  reg1 |= 0<<(W25QXX_SR_SRP%8);
  reg2 &= 0b11111110;
  reg2 |= 0<<(W25QXX_SR_SRL%8);

  PAL_W25QXX_SetWritable(target,true);
  if(PAL_W25QXX_CanWrite(target)==false) return W25QXX_ERROR;

  uint8_t tx[3]={W25QXX_WriteStatus1,reg1,reg2};
  status = PAL_W25QXX_Command(target,tx,3,NULL,0);
  if(status!=W25QXX_ACCEPTED) return status;

  status = PAL_W25QXX_SetStrength(target,W25QXX_STRENGTH100);
  if(status!=W25QXX_ACCEPTED) return status;

  return PAL_W25QXX_GetType(target);
}

__weak void PAL_W25QXX_WaitBusy(PAL_W25QXX* target){
  while(PAL_W25QXX_IsBusy(target)) PAL_Delay(W25QXX_DelayInterval);
}

__weak bool PAL_W25QXX_NeedFastRead(PAL_W25QXX* target){
  UNUSED(target);
  return false;
}

PAL_W25QXX_State PAL_W25QXX_Command(PAL_W25QXX* target,uint8_t* txData,uint16_t txSize,uint8_t* rxData,uint16_t rxSize){
  if(target->chip_select==NULL) return W25QXX_ERROR;
  if(target->spi_tx==NULL||target->spi_rx==NULL) return W25QXX_ERROR;
  target->chip_select(true);

  HAL_StatusTypeDef status;
  if(txData!=NULL&&txSize!=0){
    if(target->sleep==true&&txData[0]!=W25QXX_PowerWake) return W25QXX_ERROR;
    status = target->spi_tx(txData,txSize);
    switch (status) {
      case HAL_BUSY    : return W25QXX_BUSY;
      case HAL_ERROR   : return W25QXX_ERROR;
      case HAL_TIMEOUT : return W25QXX_TIMEOUT;
    }
  }
  if(rxData!=NULL&&rxSize!=0){
    status = target->spi_rx(rxData,rxSize);
    switch (status) {
      case HAL_BUSY    : return W25QXX_BUSY;
      case HAL_ERROR   : return W25QXX_ERROR;
      case HAL_TIMEOUT : return W25QXX_TIMEOUT;
    }
  }

  target->chip_select(false);
  PAL_Delay(W25QXX_DelayInterval);
  return W25QXX_ACCEPTED;
}

PAL_W25QXX_State PAL_W25QXX_GetType(PAL_W25QXX* target){
  PAL_W25QXX_State status;
  uint8_t cmd[]={W25QXX_DEVICE,W25QXX_DUMMY,W25QXX_DUMMY,W25QXX_DUMMY};
  uint8_t rx[2];
  uint16_t id;
  status = PAL_W25QXX_Command(target,cmd,4,rx,2);
  id = rx[0]<<8|rx[1];
  if(status!=W25QXX_ACCEPTED) return status;
  switch(id){
    case W25QXX_ID16  : target->typeID = id;
                        target->volume = 2;
                        break;
    case W25QXX_ID32  : target->typeID = id;
                        target->volume = 4;
                        break;
    case W25QXX_ID64  : target->typeID = id;
                        target->volume = 8;
                        break;
    case W25QXX_ID128 : target->typeID = id;
                        target->volume = 16;
                        break;
    default: return W25QXX_ERROR;
  }
  return W25QXX_ACCEPTED;
}

PAL_W25QXX_State PAL_W25QXX_GetJEDEC(PAL_W25QXX* target,uint16_t* jedec){
  PAL_W25QXX_State status;
  uint8_t res[3],cmd = W25QXX_DEVICE_FULL;
  status = PAL_W25QXX_Command(target,&cmd,1,res,3);
  if(status!=W25QXX_ACCEPTED) *jedec = 0xFFu;
  else *jedec = (uint16_t)res[1]<<8|(uint16_t)res[2];
  return status;
}

PAL_W25QXX_State PAL_W25QXX_GetUnique(PAL_W25QXX* target,uint64_t* unique){
  PAL_W25QXX_State status;
  uint8_t cmd[] = {W25QXX_UNIQUE_ID,W25QXX_DUMMY,W25QXX_DUMMY,W25QXX_DUMMY,W25QXX_DUMMY};
  uint8_t rx[8];
  status = PAL_W25QXX_Command(target,cmd,5,rx,8);
  *unique = 0u;
  if(status!=W25QXX_ACCEPTED) *unique = 0xFFFFFFFFu;
  else for(uint8_t i=0;i<8;i++){
    *unique <<= 8;
    *unique |= rx[i];
  }
  return status;
}

PAL_W25QXX_State PAL_W25QXX_SetWritable(PAL_W25QXX* target,bool enable){
  bool curr = PAL_W25QXX_CanWrite(target);
  if(curr==enable) return W25QXX_ACCEPTED;

  if(PAL_W25QXX_IsBusy(target))
#ifdef WAIT_BUSY_ENABLE
    PAL_W25QXX_WaitBusy(target);
#else
    return W25QXX_BUSY;
#endif

  uint8_t cmd = enable?W25QXX_WriteEnable:W25QXX_WriteDisable;
  return PAL_W25QXX_Command(target,&cmd,1,NULL,0);
}

PAL_W25QXX_State PAL_W25QXX_Sleep(PAL_W25QXX* target,bool enable){
  if(target->sleep==enable) return W25QXX_ACCEPTED;
  uint8_t cmd = enable?W25QXX_PowerDown:W25QXX_PowerWake;
  PAL_W25QXX_State status = PAL_W25QXX_Command(target,&cmd,1,NULL,0);
  if(status==W25QXX_ACCEPTED){
    target->sleep=enable;
    PAL_Delay(W25QXX_SleepWakeupInterval);
  }
  return status;
}

PAL_W25QXX_State PAL_W25QXX_Reset(PAL_W25QXX* target){
  if(PAL_W25QXX_OnSuspend(target)) return W25QXX_SUSPEND;
  if(PAL_W25QXX_IsBusy(target))
#ifdef WAIT_BUSY_ENABLE
    PAL_W25QXX_WaitBusy(target);
#else
    return W25QXX_BUSY;
#endif
  uint8_t cmd = W25QXX_ResetEnable;
  PAL_W25QXX_State status = PAL_W25QXX_Command(target,&cmd,1,NULL,0);
  if(status!=W25QXX_ACCEPTED) return status;
  cmd = W25QXX_Reset;
  status = PAL_W25QXX_Command(target,&cmd,1,NULL,0);
  if(status!=W25QXX_ACCEPTED) return status;
  PAL_Delay(W25QXX_ResetInterval);
  return PAL_W25QXX_Init(target);
}

PAL_W25QXX_State PAL_W25QXX_Suspend(PAL_W25QXX* target,bool enable){
  bool curr = PAL_W25QXX_OnSuspend(target);
  if(curr==enable) return W25QXX_ERROR;
  uint8_t cmd = enable?W25QXX_Suspend:W25QXX_Resume;
  PAL_W25QXX_State status = PAL_W25QXX_Command(target,&cmd,1,NULL,0);
  if(status==W25QXX_ACCEPTED) PAL_Delay(W25QXX_SuspendInterval);
  return status;
}

bool PAL_W25QXX_IsBusy(PAL_W25QXX* target){
  PAL_W25QXX_State status;
  uint8_t cmd = W25QXX_ReadStatus1,reg;
  status = PAL_W25QXX_Command(target,&cmd,1,&reg,1);
  if(status!=W25QXX_ACCEPTED) return true;
  return (reg>>W25QXX_SR_BUSY)&0x01;
}

bool PAL_W25QXX_OnSuspend(PAL_W25QXX* target){
  PAL_W25QXX_State status;
  uint8_t cmd = W25QXX_ReadStatus2,reg;
  status = PAL_W25QXX_Command(target,&cmd,1,&reg,1);
  if(status!=W25QXX_ACCEPTED) return true;
  return (reg>>(W25QXX_SR_SUS%8))&0x01;
}

bool PAL_W25QXX_CanWrite(PAL_W25QXX* target){
  PAL_W25QXX_State status;
  uint8_t cmd = W25QXX_ReadStatus1,reg;
  status = PAL_W25QXX_Command(target,&cmd,1,&reg,1);
  if(status!=W25QXX_ACCEPTED) return true;
  return (reg>>W25QXX_SR_WEL)&0x01;
}

uint8_t PAL_W25QXX_GetStrength(PAL_W25QXX* target){
  uint8_t cmd = W25QXX_ReadStatus3,reg;
  if(PAL_W25QXX_Command(target,&cmd,1,&reg,1)==W25QXX_ACCEPTED) return 0xFF;
  reg &= 0b01100000;
  reg >>= 5;
  return reg;
}

PAL_W25QXX_State PAL_W25QXX_SetStrength(PAL_W25QXX* target, uint8_t strength){
  if(PAL_W25QXX_IsBusy(target))
#ifdef WAIT_BUSY_ENABLE
    PAL_W25QXX_WaitBusy(target);
#else
    return W25QXX_BUSY;
#endif

  PAL_W25QXX_SetWritable(target,true);
  if(PAL_W25QXX_CanWrite(target)==false) return W25QXX_ERROR;

  strength = (strength%4)<<(W25QXX_SR_DRV%8);
  PAL_W25QXX_State status;
  uint8_t cmd = W25QXX_ReadStatus3,reg;
  status = PAL_W25QXX_Command(target,&cmd,1,&reg,1);
  if(status!=W25QXX_ACCEPTED) return status;

  reg &= 0b10011111;
  reg |= strength;
  cmd = W25QXX_WriteStatus3;
  uint8_t tx[2]={cmd,reg};
  return PAL_W25QXX_Command(target,tx,2,NULL,0);
}

uint8_t PAL_W25QXX_GetLockMode(PAL_W25QXX* target){
  PAL_W25QXX_State status;
  uint8_t cmd = W25QXX_ReadStatus3,reg;
  status = PAL_W25QXX_Command(target,&cmd,1,&reg,1);
  if(status!=W25QXX_ACCEPTED) return W25QXX_WP_UNKNOWN;
  return (reg>>(W25QXX_SR_WPS%8))&0x01;
}

PAL_W25QXX_State PAL_W25QXX_SetLockMode(PAL_W25QXX* target, uint8_t mode){
  if(PAL_W25QXX_IsBusy(target))
#ifdef WAIT_BUSY_ENABLE
    PAL_W25QXX_WaitBusy(target);
#else
  return W25QXX_BUSY;
#endif

  PAL_W25QXX_SetWritable(target,true);
  if(PAL_W25QXX_CanWrite(target)==false) return W25QXX_ERROR;

  mode = (mode%2)<<(W25QXX_SR_WPS%8);
  PAL_W25QXX_State status;
  uint8_t cmd = W25QXX_ReadStatus3,reg;
  status = PAL_W25QXX_Command(target,&cmd,1,&reg,1);
  if(status!=W25QXX_ACCEPTED) return status;

  reg &= 0b11111011;
  reg |= mode;
  cmd = W25QXX_WriteStatus3;
  uint8_t tx[2]={cmd,reg};
  return PAL_W25QXX_Command(target,tx,2,NULL,0);
}

PAL_W25QXX_State PAL_W25QXX_LockAll(PAL_W25QXX* target,bool lock){
  if(PAL_W25QXX_GetLockMode(target)!=W25QXX_WP_INDIVIDUAL) return W25QXX_ERROR;
  PAL_W25QXX_SetWritable(target,true);
  if(PAL_W25QXX_CanWrite(target)==false) return W25QXX_ERROR;

  uint8_t cmd = lock?W25QXX_LockAll:W25QXX_UnlockAll;
  return PAL_W25QXX_Command(target,&cmd,1,NULL,0);
}

PAL_W25QXX_State PAL_W25QXX_LockIndividual(PAL_W25QXX* target, uint8_t block,uint8_t sector,bool lock){
  if(PAL_W25QXX_GetLockMode(target)!=W25QXX_WP_INDIVIDUAL) return W25QXX_ERROR;

  PAL_W25QXX_SetWritable(target,true);
  if(PAL_W25QXX_CanWrite(target)==false) return W25QXX_ERROR;

  uint8_t maxBlock = PAL_W25QXX_GetMaxBlock(target->volume);
  if(maxBlock==0) return W25QXX_ERROR;
  uint32_t addr;
  if(block>0&&block<maxBlock) addr = PAL_W25QXX_AddrEncode(block,0,0,0);
  else addr = PAL_W25QXX_AddrEncode(block,sector,0,0);
  uint8_t tx[4] = {lock?W25QXX_Lock:W25QXX_Unlock};
  PAL_W25QXX_Addr2Array(addr,tx+1);

  return PAL_W25QXX_Command(target,tx,4,NULL,0);
}

bool PAL_W25QXX_CheckIndividual(PAL_W25QXX* target, uint8_t block,uint8_t sector){
  if(PAL_W25QXX_GetLockMode(target)!=W25QXX_WP_INDIVIDUAL) return true;

  uint8_t maxBlock = PAL_W25QXX_GetMaxBlock(target->volume);
  if(maxBlock==0) return true;
  uint32_t addr;
  if(block>0&&block<maxBlock) addr = PAL_W25QXX_AddrEncode(block,0,0,0);
  else addr = PAL_W25QXX_AddrEncode(block,sector,0,0);
  uint8_t tx[4] = {W25QXX_CheckLock};
  PAL_W25QXX_Addr2Array(addr,tx+1);
  uint8_t res;
  PAL_W25QXX_State status = PAL_W25QXX_Command(target,tx,4,&res,1);
  if(status!=W25QXX_ACCEPTED) return true;
  return res;
}

PAL_W25QXX_State PAL_W25QXX_SetLockPartial(PAL_W25QXX* target,bool reverse,bool sector,bool little,uint8_t num){
  if(PAL_W25QXX_IsBusy(target))
#ifdef WAIT_BUSY_ENABLE
    PAL_W25QXX_WaitBusy(target);
#else
    return W25QXX_BUSY;
#endif

  if(PAL_W25QXX_GetLockMode(target) != W25QXX_WP_PARTIAL) return W25QXX_ERROR;

  uint8_t code = 0x00u;
  code |= (reverse%2)<<5;
  code |= (sector%2)<<4;
  code |= (little%2)<<3;
  code |= num%8;

  PAL_W25QXX_State status;
  uint8_t cmd,reg1,reg2;
  cmd = W25QXX_ReadStatus1;
  status = PAL_W25QXX_Command(target,&cmd,1,&reg1,1);
  if(status!=W25QXX_ACCEPTED) return status;
  cmd = W25QXX_ReadStatus2;
  status = PAL_W25QXX_Command(target,&cmd,1,&reg2,1);
  if(status!=W25QXX_ACCEPTED) return status;

  PAL_W25QXX_SetWritable(target,true);
  if(PAL_W25QXX_CanWrite(target)==false) return W25QXX_ERROR;

  code &= 0b00111111;
  uint8_t cmp = (code&0b00100000)>>5;
  reg2 |= cmp<<(W25QXX_SR_CMP%8);
  uint8_t set = (code&0b00011111);
  reg1 |= set<<(W25QXX_SR_BP%8);
  uint8_t tx[3]={W25QXX_WriteStatus1,reg1,reg2};
  return PAL_W25QXX_Command(target,tx,3,NULL,0);
}

uint32_t PAL_W25QXX_GetLockPartial(PAL_W25QXX* target){
  if(target->volume<2) return 0xFFFFFFFF;
  if(PAL_W25QXX_GetLockMode(target) != W25QXX_WP_PARTIAL) return 0xFFFFFFFF;

  PAL_W25QXX_State status;
  uint8_t cmd,reg1,reg2;
  cmd = W25QXX_ReadStatus1;
  status = PAL_W25QXX_Command(target,&cmd,1,&reg1,1);
  if(status!=W25QXX_ACCEPTED) return 0xFFFFFFFF;
  cmd = W25QXX_ReadStatus2;
  status = PAL_W25QXX_Command(target,&cmd,1,&reg2,1);
  if(status!=W25QXX_ACCEPTED) return 0xFFFFFFFF;

  uint8_t reverse = (reg2&0b01000000)>>(W25QXX_SR_CMP%8);
  uint8_t sector  = (reg1&0b01000000)>>(W25QXX_SR_SEC%8);
  uint8_t little  = (reg1&0b00100000)>>(W25QXX_SR_TB%8);
  uint8_t number  = (reg1&0b00011100)>>(W25QXX_SR_BP%8);

  uint32_t range = 0;
  if(sector!=0&&number>4) number = 4;
  if(sector!=0) range = 4*1024*number;
  else{
    range = target->volume*1024*1024;
    uint8_t maxNum = (target->volume==2?0x06:0x07);
    while (maxNum>number){range/=2;maxNum--;}
  }
  range = number==0?0:(range-1);
  range |= reverse<<25;
  range |= little <<24;
  return range;
}

bool PAL_W25QXX_IsLocked(PAL_W25QXX* target,uint32_t addr){
  if(addr>PAL_W25QXX_GetMaxAddr(target->volume)) return true;
  uint8_t block,sector,page,byte;
  PAL_W25QXX_AddrDecode(addr,&block,&sector,&page,&byte);
  uint8_t mode = PAL_W25QXX_GetLockMode(target);
  if(mode==W25QXX_WP_INDIVIDUAL) return PAL_W25QXX_CheckIndividual(target,block,sector);
  else if(mode==W25QXX_WP_PARTIAL){
    uint32_t code = PAL_W25QXX_GetLockPartial(target);
    bool bottom  = ((code>>24)&0x01)==1?true:false;
    bool reverse = ((code>>25)&0x01)==1?true:false;
    code &= 0x00FFFFFF;
    if(!bottom) code = PAL_W25QXX_GetMaxAddr(target->volume)-code;
    bool res = bottom?(addr<=code):(addr>=code);
    return (reverse!=0) == (!res);
  }
  else return true;
}

uint32_t PAL_W25QXX_AddrEncode(uint8_t block,uint8_t sector,uint8_t page,uint8_t byte){
  uint32_t addr = 0x00000000u;
  addr |= block;
  addr <<=8;
  addr |= (sector<<4)|(page&0x0Fu);
  addr <<=8;
  addr |= byte;
  return addr;
}

void PAL_W25QXX_AddrDecode(uint32_t addr,uint8_t* block,uint8_t* sector,uint8_t* page,uint8_t* byte){
  *byte   = (uint8_t)(addr&W25QXX_ByteMask);
  *page   = (uint8_t)((addr&W25QXX_PageMask)>>8);
  *sector = (uint8_t)((addr&W25QXX_SectorMask)>>12);
  *block  = (uint8_t)((addr&W25QXX_BlockMask)>>16);
}

void PAL_W25QXX_Addr2Array(uint32_t addr,uint8_t* data){
  addr &= 0x00FFFFFFu;
  data[0] = (uint8_t)(addr/0xFFFFu);
  addr &= 0x0000FFFFu;
  data[1] = (uint8_t)(addr/0xFFu);
  addr &= 0x000000FFu;
  data[2] = (uint8_t)addr;
}

uint32_t PAL_W25QXX_GetMaxAddr(uint8_t volume){
  return (((uint32_t)volume*16-1)<<16)|(0x0000FFFF);
}

uint8_t PAL_W25QXX_GetMaxBlock(uint8_t volume){
  return volume*16-1;
}

PAL_W25QXX_State PAL_W25QXX_ReadData(PAL_W25QXX* target,uint32_t addr,uint8_t* data,uint32_t size){
  if(data==NULL||size==0) return W25QXX_ERROR;
  if(PAL_W25QXX_IsBusy(target))
#ifdef WAIT_BUSY_ENABLE
    PAL_W25QXX_WaitBusy(target);
#else
    return W25QXX_BUSY;
#endif

  uint32_t maxAddr = PAL_W25QXX_GetMaxAddr(target->volume);
  if(addr>maxAddr) return W25QXX_ERROR;
  if((maxAddr-addr+1)<size) return W25QXX_ERROR;

  uint32_t group = size/0xFFFFu,mark=0;
  uint16_t rest = size%0xFFFFu;
  uint8_t cmd[4];
  PAL_W25QXX_State status;
  cmd[0] = PAL_W25QXX_NeedFastRead(target)?W25QXX_ReadFast:W25QXX_Read;

  while(group>0){
    PAL_W25QXX_Addr2Array(addr+mark,cmd+1);
    status = PAL_W25QXX_Command(target,cmd,4,data+mark,0xFFFFu);
    if(status!=W25QXX_ACCEPTED) return status;
    group--;
    mark+=0xFFFFu;
  }
  PAL_W25QXX_Addr2Array(addr+mark,cmd+1);
  status = PAL_W25QXX_Command(target,cmd,4,data+mark,rest);
  return status;
}

PAL_W25QXX_State PAL_W25QXX_EraseSector(PAL_W25QXX* target,uint8_t block,uint8_t sector){
  if(PAL_W25QXX_IsBusy(target))
#ifdef WAIT_BUSY_ENABLE
    PAL_W25QXX_WaitBusy(target);
#else
    return W25QXX_BUSY;
#endif

  if(PAL_W25QXX_OnSuspend(target)) return W25QXX_SUSPEND;
  if(sector>0x0F) return W25QXX_ERROR;
  if(PAL_W25QXX_IsLocked(target, PAL_W25QXX_AddrEncode(block,sector,0,0))==true) return W25QXX_LOCK;
  PAL_W25QXX_SetWritable(target,true);
  if(PAL_W25QXX_CanWrite(target)==false) return W25QXX_ERROR;

  uint8_t tx[4]={W25QXX_EraseSector};
  PAL_W25QXX_Addr2Array(PAL_W25QXX_AddrEncode(block,sector,0,0),tx+1);
  PAL_W25QXX_State status = PAL_W25QXX_Command(target,tx,4,NULL,0);
#ifdef WAIT_EXECUTE_ENABLE
  if(status==W25QXX_ACCEPTED) PAL_Delay(W25QXX_EraseInterval_4K);
#endif
  return status;
}

PAL_W25QXX_State PAL_W25QXX_EraseHalfBlock(PAL_W25QXX* target,uint8_t block,bool large){
  if(PAL_W25QXX_IsBusy(target))
#ifdef WAIT_BUSY_ENABLE
    PAL_W25QXX_WaitBusy(target);
#else
    return W25QXX_BUSY;
#endif

  if(PAL_W25QXX_OnSuspend(target)) return W25QXX_SUSPEND;
  uint8_t maxBlock = PAL_W25QXX_GetMaxBlock(target->volume);
  if(block>maxBlock) return W25QXX_ERROR;

  if(block>0&&block<maxBlock){
    if(PAL_W25QXX_IsLocked(target, PAL_W25QXX_AddrEncode(block,0,0,0))==true) return W25QXX_LOCK;
  }else{
    for(uint8_t i=0;i<8;i++)
      if(PAL_W25QXX_IsLocked(target, PAL_W25QXX_AddrEncode(block,large?(8+i):i,0,0))==true) return W25QXX_LOCK;
  }

  PAL_W25QXX_SetWritable(target,true);
  if(PAL_W25QXX_CanWrite(target)==false) return W25QXX_ERROR;

  uint8_t tx[4]={W25QXX_EraseHalf};
  PAL_W25QXX_Addr2Array(PAL_W25QXX_AddrEncode(block,large?8:0,0,0),tx+1);
  PAL_W25QXX_State status = PAL_W25QXX_Command(target,tx,4,NULL,0);
#ifdef WAIT_EXECUTE_ENABLE
  if(status==W25QXX_ACCEPTED) PAL_Delay(W25QXX_EraseInterval_32K);
#endif
  return status;
}

PAL_W25QXX_State PAL_W25QXX_EraseBlock(PAL_W25QXX* target,uint8_t block){
  if(PAL_W25QXX_IsBusy(target))
#ifdef WAIT_BUSY_ENABLE
    PAL_W25QXX_WaitBusy(target);
#else
    return W25QXX_BUSY;
#endif

  if(PAL_W25QXX_OnSuspend(target)) return W25QXX_SUSPEND;
  uint8_t maxBlock = PAL_W25QXX_GetMaxBlock(target->volume);
  if(block>maxBlock) return W25QXX_ERROR;

  if(block>0&&block<maxBlock){
    if(PAL_W25QXX_IsLocked(target, PAL_W25QXX_AddrEncode(block,0,0,0))==true) return W25QXX_LOCK;
  } else{
    for(uint8_t i=0;i<16;i++)
      if(PAL_W25QXX_IsLocked(target, PAL_W25QXX_AddrEncode(block,i,0,0))==true) return W25QXX_LOCK;
  }

  PAL_W25QXX_SetWritable(target,true);
  if(PAL_W25QXX_CanWrite(target)==false) return W25QXX_ERROR;

  uint8_t tx[4]={W25QXX_EraseBlock};
  PAL_W25QXX_Addr2Array(PAL_W25QXX_AddrEncode(block,0,0,0),tx+1);
  PAL_W25QXX_State status = PAL_W25QXX_Command(target,tx,4,NULL,0);
#ifdef WAIT_EXECUTE_ENABLE
  if(status==W25QXX_ACCEPTED) PAL_Delay(W25QXX_EraseInterval_64K);
#endif
  return status;
}

PAL_W25QXX_State PAL_W25QXX_EraseAll(PAL_W25QXX* target){
  if(PAL_W25QXX_IsBusy(target))
#ifdef WAIT_BUSY_ENABLE
    PAL_W25QXX_WaitBusy(target);
#else
    return W25QXX_BUSY;
#endif

  if(PAL_W25QXX_OnSuspend(target)) return W25QXX_SUSPEND;
  uint8_t maxBlock = PAL_W25QXX_GetMaxBlock(target->volume);

  for(uint8_t i=1;i<maxBlock;i++)
    if(PAL_W25QXX_IsLocked(target, PAL_W25QXX_AddrEncode(i,0,0,0))==true) return W25QXX_LOCK;
  for(uint8_t i=0;i<16;i++){
    if(PAL_W25QXX_IsLocked(target, PAL_W25QXX_AddrEncode(0,i,0,0))==true) return W25QXX_LOCK;
    if(PAL_W25QXX_IsLocked(target, PAL_W25QXX_AddrEncode(maxBlock,i,0,0))==true) return W25QXX_LOCK;
  }

  PAL_W25QXX_SetWritable(target,true);
  if(PAL_W25QXX_CanWrite(target)==false) return W25QXX_ERROR;

  uint8_t cmd = W25QXX_EraseAll;
  PAL_W25QXX_State status = PAL_W25QXX_Command(target,&cmd,1,NULL,0);
#ifdef WAIT_EXECUTE_ENABLE
  if(status==W25QXX_ACCEPTED) PAL_Delay(W25QXX_EraseInterval_Chip);
#endif
  return status;
}

PAL_W25QXX_State PAL_W25QXX_WriteData(PAL_W25QXX* target,uint32_t addr,const uint8_t* data,uint32_t size,bool unsecure){
  if(data==NULL||size==0) return W25QXX_ERROR;
  if(PAL_W25QXX_IsBusy(target))
#ifdef WAIT_BUSY_ENABLE
    PAL_W25QXX_WaitBusy(target);
#else
  return W25QXX_BUSY;
#endif
  if(PAL_W25QXX_OnSuspend(target)) return W25QXX_SUSPEND;
  uint32_t maxAddr = PAL_W25QXX_GetMaxAddr(target->volume);
  if(addr>maxAddr||(maxAddr-addr)<size) return W25QXX_ERROR;

  PAL_W25QXX_State status;
  uint8_t block_s,sector_s,page_s,byte_s;
  PAL_W25QXX_AddrDecode(addr,&block_s,&sector_s,&page_s,&byte_s);
  uint8_t block_e,sector_e,page_e,byte_e;
  PAL_W25QXX_AddrDecode(addr+size-1,&block_e,&sector_e,&page_e,&byte_e);

  uint8_t cnt_b=block_s,cnt_s=sector_s;
  do{
    if(PAL_W25QXX_IsLocked(target, PAL_W25QXX_AddrEncode(cnt_b, cnt_s, 0, 0)) == true) return W25QXX_LOCK;
    cnt_s++;
    if(cnt_s == 16){
      if(cnt_b == PAL_W25QXX_GetMaxBlock(target->volume)) break;
      cnt_s=0;
      cnt_b++;
    }
  }while(cnt_b < block_e || (cnt_b==block_e&&cnt_s<=sector_e));

  uint8_t *head,*tail;
  uint32_t head_size,tail_size;

  if(page_s!=0||byte_s!=0){
    head_size = page_s*256+byte_s;
    head = (uint8_t*)malloc(sizeof(uint8_t)*head_size);
    status = PAL_W25QXX_ReadData(target, PAL_W25QXX_AddrEncode(block_s,sector_s,0,0),head,head_size);
    if(status!=W25QXX_ACCEPTED) return status;
  }else{head=NULL;head_size=0;}

  if(page_e!=15||byte_e!=255){
    tail_size = (15-page_e)*256+(255-byte_e);
    tail = (uint8_t*) malloc(sizeof(uint8_t)*tail_size);
    status = PAL_W25QXX_ReadData(target, PAL_W25QXX_AddrEncode(block_e,sector_e,page_e,byte_e+1),tail,tail_size);
    if(status!=W25QXX_ACCEPTED) return status;
  }else{tail=NULL;tail_size=0;}

  if(!unsecure) {
    cnt_b = block_s, cnt_s = sector_s;
    do {
      if (PAL_W25QXX_IsBusy(target)) PAL_W25QXX_WaitBusy(target);

      PAL_W25QXX_SetWritable(target,true);
      if(PAL_W25QXX_CanWrite(target)==false) return W25QXX_ERROR;

      status = PAL_W25QXX_EraseSector(target, cnt_b, cnt_s);
      if (status != W25QXX_ACCEPTED) return status;
      cnt_s++;
      if (cnt_s == 16) {
        if (cnt_b == PAL_W25QXX_GetMaxBlock(target->volume)) break;
        cnt_s = 0;
        cnt_b++;
      }
    } while (cnt_b < block_e || (cnt_b==block_e&&cnt_s<=sector_e));
  }

  uint8_t tx[260]={W25QXX_Write};
  uint32_t cnt_addr = PAL_W25QXX_AddrEncode(block_s,sector_s,0,0);
  for(uint32_t i=0;i<(head_size+size+tail_size);i+=256){
    if(PAL_W25QXX_IsBusy(target)) PAL_W25QXX_WaitBusy(target);
    PAL_W25QXX_Addr2Array(cnt_addr,tx+1);
    for(uint16_t j=0;j<256;j++){
      if((i+j)<head_size) tx[4+j]=head[i+j];
      else if((i+j)<(head_size+size)) tx[4+j]=data[i+j-head_size];
      else tx[4+j]=tail[i+j-(head_size+size)];
    }

    PAL_W25QXX_SetWritable(target,true);
    if(PAL_W25QXX_CanWrite(target)==false) return W25QXX_ERROR;

    status = PAL_W25QXX_Command(target,tx,260,NULL,0);
    if(status!=W25QXX_ACCEPTED) return status;
    cnt_addr+=256;
  }

  return W25QXX_ACCEPTED;
}

PAL_W25QXX_State PAL_W25QXX_ProgramPage(PAL_W25QXX* target,uint32_t addr,const uint8_t* data,uint16_t size){
  if(PAL_W25QXX_IsBusy(target))
#ifdef WAIT_BUSY_ENABLE
    PAL_W25QXX_WaitBusy(target);
#else
  return W25QXX_BUSY;
#endif

  if(PAL_W25QXX_OnSuspend(target)) return W25QXX_SUSPEND;
  uint8_t block,sector,page,byte;
  PAL_W25QXX_AddrDecode(addr,&block,&sector,&page,&byte);
  if((uint16_t)byte+size>256) return W25QXX_ERROR;

  uint8_t tx[260]={W25QXX_Write};
  PAL_W25QXX_Addr2Array(addr,tx+1);
  for(uint16_t i=0;i<size;i++) tx[4+i]=data[i];

  PAL_W25QXX_SetWritable(target,true);
  if(PAL_W25QXX_CanWrite(target)==false) return W25QXX_ERROR;

  return PAL_W25QXX_Command(target,tx,4+size,NULL,0);
}

#endif //PAL_FUNC_W25QXX

//page 256Byte 0x00~0xFF
//sector = 4KByte = 4096Byte = 16 Page 0x0 00~ 0xF FF
//block = 64KByte = 16 sector 0x0 000~ 0xF FFF
//whole ?MB = ?*1024 KB = ?*16 *64KB = ?*16 block
// 1 0x00 0000~0x0F FFFF
// 2 0x00 0000~0x1F FFFF
// 4 0x00 0000~0x3F FFFF
// 8 0x00 0000~0x7F FFFF
//16 0x00 0000~0xFF FFFF
// XX(block) X(sector)X(page) XX(Byte)