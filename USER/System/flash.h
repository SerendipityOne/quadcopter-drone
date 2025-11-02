#ifndef __FLASH_H
#define __FLASH_H

#include "main.h"
#include "ALL_DATA.h"

#define MPU_CALIB_FLASH_PAGE_ADDR ((uint32_t)0x0800FC00u)
#define MPU_CALIB_FLASH_PAGE_SIZE (1024u)

bool MpuCalib_Load(int16_t outOffsets[6]);       // 读取，如果有效返回 true
bool MpuCalib_Save(const int16_t inOffsets[6]);  // 保存，成功返回 true
bool MpuCalib_Clear(void);                       // 擦除该页（清掉校准数据）

#endif  // !__FLASH_H
