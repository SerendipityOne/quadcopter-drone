#include <stdint.h>
#include <string.h>
#include "stm32f1xx_hal.h"

#define MPU_FLASH_ADDR ((uint32_t)0x0800FC00)

// 读：把 Flash 里的 12 字节读到 out[6]
void MpuOffset_Read(int16_t out[6]) {
  if (!out) return;
  const int16_t* src = (const int16_t*)MPU_FLASH_ADDR;
  memcpy(out, src, 6 * sizeof(int16_t));
}

// 写：先擦该页，再按半字把 in[6] 写到页起始
// 只做最基本的返回状态，不做任何校验
HAL_StatusTypeDef MpuOffset_Write(const int16_t in[6]) {
  if (!in) return HAL_ERROR;

  HAL_StatusTypeDef st;
  uint32_t page_error = 0;

  // 1) 擦页（整页 1KB）
  HAL_FLASH_Unlock();

  FLASH_EraseInitTypeDef ei = {0};
  ei.TypeErase = FLASH_TYPEERASE_PAGES;
  ei.PageAddress = MPU_FLASH_ADDR;
  ei.NbPages = 1;
  st = HAL_FLASHEx_Erase(&ei, &page_error);
  if (st != HAL_OK || page_error != 0xFFFFFFFFu) {
    HAL_FLASH_Lock();
    return HAL_ERROR;
  }

  // 2) 以半字写入 6 个 int16_t（12 字节）
  for (uint32_t i = 0; i < 6; ++i) {
    uint32_t dst = MPU_FLASH_ADDR + (i * 2u);
    uint16_t hw = (uint16_t)in[i];
    st = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, dst, hw);
    if (st != HAL_OK) {
      HAL_FLASH_Lock();
      return st;
    }
  }

  HAL_FLASH_Lock();
  return HAL_OK;
}
