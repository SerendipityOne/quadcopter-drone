#ifndef __MPU6050_H
#define __MPU6050_H

#include "ALL_DATA.h"
#include "MPU6050_Regs.h"

uint8_t MPU6050_Init(void);
void MPU_GetData(void);

#endif  // !__MPU6050_H
