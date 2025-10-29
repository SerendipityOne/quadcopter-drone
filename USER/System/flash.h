#ifndef __FLASH_H
#define __FLASH_H

#include "main.h"

#define FLASH_Start_Addr 0x08007C00  // flash写起始地址
#define FLASH_End_Addr   0x08007FFF  // flash写结束地址

// FLASH函数声明
void FLASH_Read(int16_t* buffer, uint8_t length);
HAL_StatusTypeDef FLASH_Write(int16_t* data, uint8_t length);

#endif  // !__FLASH_H
