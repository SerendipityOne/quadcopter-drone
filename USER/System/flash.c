#include "flash.h"
#include "stm32f1xx_hal_flash.h"

uint32_t address = FLASH_Start_Addr;

/**
 * @brief  从FLASH指定地址读取多个字节
 * @param  buffer: 用于存储读取数据的缓冲区指针
 * @param  length: 要读取的数据长度(字节数)
 * @retval 无
 */
void FLASH_Read(int16_t* buffer, uint8_t length) {
  uint8_t i;

  for (i = 0; i < length; i++) {
    buffer[i] = (*(__IO int16_t*)(address + (i * 2)));
  }
}

/**
 * @brief  向FLASH写入数据
 * @param  address: 写入的FLASH起始地址
 * @param  data: 要写入的数据指针
 * @param  length: 要写入的数据长度(字数，一个字为16位)
 * @retval HAL_StatusTypeDef: 操作结果
 */
HAL_StatusTypeDef FLASH_Write(int16_t* data, uint8_t length) {
  HAL_StatusTypeDef status = HAL_OK;
  uint8_t i = 0;

  // 解锁FLASH
  HAL_FLASH_Unlock();

  // 计算需要的页数
  uint32_t pages = ((length * 2) + FLASH_PAGE_SIZE - 1) / FLASH_PAGE_SIZE;

  // 擦除页面
  FLASH_EraseInitTypeDef EraseInitStruct;
  uint32_t PageError = 0;

  EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
  EraseInitStruct.PageAddress = address;
  EraseInitStruct.NbPages = pages;

  status = HAL_FLASHEx_Erase(&EraseInitStruct, &PageError);

  if (status == HAL_OK) {
    // 写入数据
    for (i = 0; i < length; i++) {
      status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, address + (i * 2), data[i]);
      if (status != HAL_OK) {
        break;
      }
    }
  }

  // 锁定FLASH
  HAL_FLASH_Lock();

  return status;
}
