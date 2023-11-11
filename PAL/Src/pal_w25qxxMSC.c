#include "pal_w25qxxMSC.h"

#include "stdlib.h"
#include "pal_utils.h"

#ifdef PAL_FUNC_W25QXX_MSC

#define USBD_OK   0
#define USBD_BUSY 1
#define USBD_FAIL 2

static PAL_W25QXX_MSC storage={
    false,
    0,
    NULL,
    NULL,
};

bool PAL_W25QXX_LoadMSC(uint8_t volumes){
  if(storage.loaded) return false;
  if(volumes==0) return false;
  storage.lun = volumes;
  storage.volumes = (PAL_W25QXX**) malloc(sizeof(PAL_W25QXX*)*volumes);
  storage.offsets = (uint32_t*) malloc(sizeof(uint32_t)*volumes);
  return true;
}

bool PAL_W25QXX_AddVolumeMSC(PAL_W25QXX* node,uint32_t sector_offset){
  static uint8_t added = 0;
  if(storage.lun==0||storage.volumes==NULL||storage.offsets==NULL) return false;
  if(storage.loaded) return false;

  if(node==NULL||node->chip_select==NULL||
     node->spi_rx==NULL||node->spi_tx==NULL) return false;
  if(PAL_W25QXX_Init(node)!=W25QXX_ACCEPTED)
    return false;
  if(PAL_W25QXX_SetLockMode(node,W25QXX_WP_INDIVIDUAL)!=W25QXX_ACCEPTED)
    return false;
  PAL_Delay(10);
  PAL_W25QXX_LockAll(node,false);
  storage.volumes[added] = node;

  uint32_t max_offset = PAL_W25QXX_GetMaxBlock(node->volume)*16;
  if(sector_offset>max_offset) return false;
  storage.offsets[added] = sector_offset;

  added++;
  if(added==storage.lun) storage.loaded = true;
  return true;
}

PAL_W25QXX* PAL_W25QXX_GetVolumeMSC(uint8_t index){
  if(!storage.loaded) return NULL;
  if(index>storage.lun||index==0) return NULL;
  return storage.volumes[index-1];
}

/*===============MSC Control API============*/

int8_t PAL_W25QXX_InitMSC(uint8_t lun){
  if(lun>=storage.lun) return USBD_FAIL;
  PAL_W25QXX* target = storage.volumes[lun];
  if(target==NULL) return USBD_FAIL;
  return USBD_OK;
}

int8_t PAL_W25QXX_ReadMSC(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len){
  if(lun>=storage.lun) return USBD_FAIL;
  PAL_W25QXX* target = storage.volumes[lun];
  if(target==NULL) return USBD_FAIL;
  uint32_t offset = storage.offsets[lun];
  uint32_t maxSector = ((uint32_t)PAL_W25QXX_GetMaxBlock(target->volume)+1)*16;

  if((offset+blk_addr+blk_len)>maxSector) return USBD_FAIL;
  PAL_W25QXX_State read_status;
  read_status = PAL_W25QXX_ReadData(target,(offset+blk_addr)<<12,buf,blk_len*4096);
  if(read_status==W25QXX_ACCEPTED) return USBD_OK;
  else return USBD_FAIL;
}

int8_t PAL_W25QXX_WriteMSC(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len){
  if(lun>=storage.lun) return USBD_FAIL;
  PAL_W25QXX* target = storage.volumes[lun];
  if(target==NULL) return USBD_FAIL;
  uint32_t offset = storage.offsets[lun];
  uint32_t maxSector = ((uint32_t)PAL_W25QXX_GetMaxBlock(target->volume)+1)*16;

  if((offset+blk_addr+blk_len)>maxSector) return USBD_FAIL;
  PAL_W25QXX_State status;
  for(uint16_t sector=0;sector<blk_len;sector++){
    //confirm unlock sector
    while(PAL_W25QXX_IsLocked(target,(offset+blk_addr+sector)<<12)){
      PAL_W25QXX_LockIndividual(target,(offset+blk_addr+sector)/16,
                                (offset+blk_addr+sector)%16,false);
      PAL_Delay(10);
    }
    //erase sector
    PAL_W25QXX_WaitBusy(target);
    PAL_W25QXX_EraseSector(target,(offset+blk_addr+sector)/16,
                           (offset+blk_addr+sector)%16);
    //program pages
    for(uint32_t page=0;page<16;page++){
      PAL_W25QXX_WaitBusy(target);
      status = PAL_W25QXX_ProgramPage(target,((offset+blk_addr+sector)<<12)+(page<<8),buf+(sector<<12)+(page<<8),256);
      if(status!=W25QXX_ACCEPTED) return USBD_FAIL;
    }
  }
  return USBD_OK;
}

int8_t PAL_W25QXX_GetMaxLunMSC(){
  return (int8_t)(storage.lun-1);
}

int8_t PAL_W25QXX_CapacityMSC(uint8_t lun, uint32_t *block_num, uint16_t *block_size){
  if(lun>=storage.lun) return USBD_FAIL;
  *block_num = ((uint32_t)PAL_W25QXX_GetMaxBlock(storage.volumes[lun]->volume)+1)*16-storage.offsets[lun];
  *block_size = 4096u;
  return USBD_OK;
}

int8_t PAL_W25QXX_IsReadyMSC(uint8_t lun){
  if(lun>=storage.lun) return USBD_FAIL;
  PAL_W25QXX* target = storage.volumes[lun];
  if(target==NULL) return USBD_FAIL;
  if(target->volume==0) return USBD_FAIL;
  PAL_W25QXX_WaitBusy(target);
  return USBD_OK;
}

__weak int8_t PAL_W25QXX_IsLockedMSC(uint8_t lun){
  UNUSED(lun);
  return (USBD_OK);
}

#endif //PAL_FUNC_W25QXX_MSC