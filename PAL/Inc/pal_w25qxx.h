#ifndef PIONEER_PAL_W25QXX_H
#define PIONEER_PAL_W25QXX_H

#include "pal_global.h"

#ifdef PAL_FUNC_W25QXX

#define WAIT_BUSY_ENABLE
//#define WAIT_EXECUTE_ENABLE

typedef enum {
    W25QXX_ACCEPTED = 0x00,
    W25QXX_BUSY = 0x01,
    W25QXX_TIMEOUT = 0x02,
    W25QXX_LOCK = 0x03,
    W25QXX_ERROR = 0x04,
    W25QXX_SUSPEND = 0x05
} PAL_W25QXX_State;

typedef struct {
    uint8_t volume;
    uint16_t typeID;
    bool sleep;
    void (*chip_select)(bool);
    HAL_StatusTypeDef (*spi_tx)(uint8_t*,uint16_t size);
    HAL_StatusTypeDef (*spi_rx)(uint8_t*,uint16_t size);
} PAL_W25QXX;

PAL_W25QXX_State PAL_W25QXX_Init(PAL_W25QXX* target);
void PAL_W25QXX_WaitBusy(PAL_W25QXX* target);
bool PAL_W25QXX_NeedFastRead(PAL_W25QXX* target);
PAL_W25QXX_State PAL_W25QXX_Command(PAL_W25QXX* target,uint8_t* txData,uint16_t txSize,uint8_t* rxData,uint16_t rxSize);
PAL_W25QXX_State PAL_W25QXX_GetType(PAL_W25QXX* target);
PAL_W25QXX_State PAL_W25QXX_GetJEDEC(PAL_W25QXX* target,uint16_t* jedec);
PAL_W25QXX_State PAL_W25QXX_GetUnique(PAL_W25QXX* target,uint64_t* unique);
bool PAL_W25QXX_IsBusy(PAL_W25QXX* target);
bool PAL_W25QXX_OnSuspend(PAL_W25QXX* target);
bool PAL_W25QXX_CanWrite(PAL_W25QXX* target);
PAL_W25QXX_State PAL_W25QXX_SetWritable(PAL_W25QXX* target,bool enable);
PAL_W25QXX_State PAL_W25QXX_Sleep(PAL_W25QXX* target,bool enable);
PAL_W25QXX_State PAL_W25QXX_Reset(PAL_W25QXX*target);
PAL_W25QXX_State PAL_W25QXX_Suspend(PAL_W25QXX* target,bool enable);
void PAL_W25QXX_AddrDecode(uint32_t addr,uint8_t* block,uint8_t* sector,uint8_t* page,uint8_t* byte);
uint32_t PAL_W25QXX_AddrEncode(uint8_t block,uint8_t sector,uint8_t page,uint8_t byte);
uint32_t PAL_W25QXX_GetMaxAddr(uint8_t volume);
uint8_t PAL_W25QXX_GetMaxBlock(uint8_t volume);
void PAL_W25QXX_Addr2Array(uint32_t addr,uint8_t* data);
PAL_W25QXX_State PAL_W25QXX_ReadData(PAL_W25QXX* target,uint32_t addr,uint8_t* data,uint32_t size);
PAL_W25QXX_State PAL_W25QXX_SetStrength(PAL_W25QXX* target, uint8_t strength);
uint8_t PAL_W25QXX_GetStrength(PAL_W25QXX* target);
uint8_t PAL_W25QXX_GetLockMode(PAL_W25QXX* target);
PAL_W25QXX_State PAL_W25QXX_SetLockMode(PAL_W25QXX* target, uint8_t mode);
PAL_W25QXX_State PAL_W25QXX_LockAll(PAL_W25QXX* target,bool lock);
PAL_W25QXX_State PAL_W25QXX_LockIndividual(PAL_W25QXX* target, uint8_t block,uint8_t sector,bool lock);
bool PAL_W25QXX_CheckIndividual(PAL_W25QXX* target, uint8_t block,uint8_t sector);
PAL_W25QXX_State PAL_W25QXX_SetLockPartial(PAL_W25QXX* target,bool reverse,bool sector,bool little,uint8_t num);
uint32_t PAL_W25QXX_GetLockPartial(PAL_W25QXX* target);
bool PAL_W25QXX_IsLocked(PAL_W25QXX* target,uint32_t addr);
PAL_W25QXX_State PAL_W25QXX_EraseSector(PAL_W25QXX* target,uint8_t block,uint8_t sector);
PAL_W25QXX_State PAL_W25QXX_EraseHalfBlock(PAL_W25QXX* target,uint8_t block,bool large);
PAL_W25QXX_State PAL_W25QXX_EraseBlock(PAL_W25QXX* target,uint8_t block);
PAL_W25QXX_State PAL_W25QXX_EraseAll(PAL_W25QXX* target);
PAL_W25QXX_State PAL_W25QXX_WriteData(PAL_W25QXX* target,uint32_t addr,const uint8_t* data,uint32_t size,bool unsecure);
PAL_W25QXX_State PAL_W25QXX_ProgramPage(PAL_W25QXX* target,uint32_t addr,const uint8_t* data,uint16_t size);

#define W25QXX_DelayInterval        1
#define W25QXX_SleepWakeupInterval  3
#define W25QXX_ResetInterval        30
#define W25QXX_SuspendInterval      20
#define W25QXX_EraseInterval_4K     400000
#define W25QXX_EraseInterval_32K    1600000
#define W25QXX_EraseInterval_64K    2000000
#define W25QXX_EraseInterval_Chip   200000000

#define W25QXX_ByteMask   0x000000FFu
#define W25QXX_PageMask   0x00000F00u
#define W25QXX_SectorMask 0x0000F000u
#define W25QXX_BlockMask  0x00FF0000u

//W25QXX Output Signal Strength
#define W25QXX_STRENGTH100 0x00u
#define W25QXX_STRENGTH075 0x01u
#define W25QXX_STRENGTH050 0x02u
#define W25QXX_STRENGTH025 0x03u

//W25QXX Write Protection
#define W25QXX_WP_PARTIAL    0x00u
#define W25QXX_WP_INDIVIDUAL  0x01u
#define W25QXX_WP_UNKNOWN     0x02u

//W25QXX Identification
#define W25QXX_ID16   0xEF14u
#define W25QXX_ID32   0xEF15u
#define W25QXX_ID64   0xEF16u
#define W25QXX_ID128  0xEF17u

//W25QXX SPI-FLASH FUNCTIONS
#define W25QXX_DUMMY        0x00u
#define W25QXX_DEVICE       0x90u
#define W25QXX_DEVICE_FULL  0x9Fu
#define W25QXX_UNIQUE_ID    0x4Bu

#define W25QXX_WriteEnable  0x06u
#define W25QXX_WriteDisable 0x04u
#define W25QXX_LockAll      0x7Eu
#define W25QXX_UnlockAll    0x98u
#define W25QXX_Lock         0x36u
#define W25QXX_Unlock       0x39u
#define W25QXX_CheckLock    0x3Du

#define W25QXX_ReadStatus1  0x05u
#define W25QXX_ReadStatus2  0x35u
#define W25QXX_ReadStatus3  0x15u
#define W25QXX_WriteStatus1 0x01u
#define W25QXX_WriteStatus2 0x31u
#define W25QXX_WriteStatus3 0x11u

#define W25QXX_Read         0x03u
#define W25QXX_ReadFast     0x0Bu
#define W25QXX_Write        0x02u
#define W25QXX_EraseSector  0x20u
#define W25QXX_EraseHalf    0x52u
#define W25QXX_EraseBlock   0xD8u
#define W25QXX_EraseAll     0xC7u
#define W25QXX_Suspend      0x75u
#define W25QXX_Resume       0x7Au

#define W25QXX_PowerDown    0xB9u
#define W25QXX_PowerWake    0xABu
#define W25QXX_ResetEnable  0x66u
#define W25QXX_Reset        0x99u

//W25QXX SPI-FLASH Status Register
#define W25QXX_SR_BUSY  0
#define W25QXX_SR_WEL   1
#define W25QXX_SR_BP    2
#define W25QXX_SR_TB    5
#define W25QXX_SR_SEC   6
#define W25QXX_SR_SRP   7
#define W25QXX_SR_SRL   8
#define W25QXX_SR_QE    9
//SR-10 reserved
#define W25QXX_SR_LB    11
//SR-LB1 12
//SR-LB2 13
#define W25QXX_SR_CMP   14
#define W25QXX_SR_SUS   15
//SR-16 reserved
//SR-17 reserved
#define W25QXX_SR_WPS   18
//SR-19 reserved
//SR-20 reserved
#define W25QXX_SR_DRV   21
//SR-DRV1 22
#define W25QXX_SR_RST   23

#endif //PAL_FUNC_W25QXX

#endif //PIONEER_PAL_W25QXX_H