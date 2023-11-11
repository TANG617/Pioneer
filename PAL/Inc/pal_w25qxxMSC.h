#ifndef PIONEER_PAL_W25QXXMSC_H
#define PIONEER_PAL_W25QXXMSC_H

#include "pal_global.h"
#include "./pal_w25qxx.h"

#ifdef PAL_FUNC_W25QXX_MSC

typedef struct{
  bool loaded;
  uint8_t lun;
  PAL_W25QXX** volumes;
  uint32_t* offsets;
} PAL_W25QXX_MSC;

bool PAL_W25QXX_LoadMSC(uint8_t volumes);
bool PAL_W25QXX_AddVolumeMSC(PAL_W25QXX* node,uint32_t sector_offset);
PAL_W25QXX* PAL_W25QXX_GetVolumeMSC(uint8_t index);

int8_t PAL_W25QXX_InitMSC(uint8_t lun);
int8_t PAL_W25QXX_CapacityMSC(uint8_t lun, uint32_t *block_num, uint16_t *block_size);
int8_t PAL_W25QXX_IsReadyMSC(uint8_t lun);
int8_t PAL_W25QXX_IsLockedMSC(uint8_t lun);
int8_t PAL_W25QXX_ReadMSC(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len);
int8_t PAL_W25QXX_WriteMSC(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len);
int8_t PAL_W25QXX_GetMaxLunMSC();

#endif //PAL_FUNC_W25QXX_MSC

#endif //PIONEER_PAL_W25QXXMSC_H
