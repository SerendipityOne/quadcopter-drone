#ifndef __MYI2C_H
#define __MYI2C_H

#include "ALL_DATA.h"
#include "main.h"

void MyI2C_Init(void);
void MyI2C_Start(void);
void MyI2C_Stop(void);
void MyI2C_SendByte(uint8_t Byte);
uint8_t MyI2C_ReceiveByte(void);
void MyI2C_SendAck(uint8_t AckBit);
uint8_t MyI2C_ReceiveAck(void);
uint8_t MyI2C_Read_Bytes(uint8_t addr, uint8_t reg, uint8_t* data, uint8_t len);
uint8_t MyI2C_Write_Bytes(uint8_t addr, uint8_t reg, uint8_t* data, uint8_t len);
uint8_t MyI2C_Read_One_Byte(uint8_t addr, uint8_t reg);
uint8_t MyI2C_Write_One_Byte(uint8_t addr, uint8_t reg, uint8_t data);

#endif
