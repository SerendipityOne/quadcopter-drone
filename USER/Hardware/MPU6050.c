#include "MPU6050.h"
#include "flash.h"
#include "i2c.h"
#include "kalman.h"
#include "my_math.h"
#include "string.h"

//**************************************************************
#define MPU_I2C_TIMEOUT   100U
#define MPU_REG_ADDR_SIZE I2C_MEMADD_SIZE_8BIT
#define FRAME_LEN         14 /* ACC(6) + TEMP(2) + GYR(6) */
//**************************************************************
int16_t MpuOffset[6];

extern MPU_t MPU6050;
//**************************************************************
// 双缓冲与状态
static uint8_t ImuBuf[2][FRAME_LEN];   /* 双缓冲 */
static volatile uint8_t CurIndex = 0;  /* 当前DMA写入缓冲索引 0/1 */
static volatile uint8_t HasFrame = 0;  /* 是否完成过至少一帧 */
static volatile uint32_t FrameSeq = 0; /* 完成帧计数 */
//**************************************************************
static HAL_StatusTypeDef MPU6050_WriteReg(uint8_t reg, uint8_t val) {
  return HAL_I2C_Mem_Write(&hi2c1, MPU_ADDR, reg, MPU_REG_ADDR_SIZE, &val, 1, MPU_I2C_TIMEOUT);
}

static HAL_StatusTypeDef MPU6050_ReadRegs(uint8_t reg, uint8_t* data, uint16_t Len) {
  return HAL_I2C_Mem_Read(&hi2c1, MPU_ADDR, reg, MPU_REG_ADDR_SIZE, data, Len, MPU_I2C_TIMEOUT);
}
//**************************************************************
/* 初始化MPU6050 */
HAL_StatusTypeDef MPU6050_Init(void) {
  HAL_StatusTypeDef res = HAL_OK;
  uint8_t who = 0;

  do {
    res = MPU6050_WriteReg(MPU_PWR_MGMT1_REG, 0x80); /* 复位唤醒，失能温度 */
    /* 采样率 = Gyro输出率 / (1 + SMPLRT_DIV) */
    res |= MPU6050_WriteReg(MPU_SAMPLE_RATE_REG, 0x02); /* ≈333 Hz */
    res |= MPU6050_WriteReg(MPU_PWR_MGMT1_REG, 0x03);   /* 时钟源 Gyro Z */
    res |= MPU6050_WriteReg(MPU_CFG_REG, 0x03);         /* DLPF 42 Hz */
    res |= MPU6050_WriteReg(MPU_GYRO_CFG_REG, 0x18);    /* ±2000 dps */
    res |= MPU6050_WriteReg(MPU_ACCEL_CFG_REG, 0x09);   /* ±4 g，LPF 5 Hz */
  } while (res != HAL_OK);

  res = MPU6050_ReadRegs(MPU_WHO_AM_I, &who, 1);

  /* 从Flash读取零偏*/
  // FLASH_Read(MpuOffset, 6);

  /* 自动开始一次DMA采样（双缓冲） */
  if (res == HAL_OK && who == MPU_ID) {
    /* 启动连续DMA采样 */
    /* 第一次启动在索引 CurIndex=0 的缓冲区 */
    (void)HAL_I2C_Mem_Read_DMA(&hi2c1, MPU_ADDR, MPU_ACCEL_XOUTH_REG, MPU_REG_ADDR_SIZE,
                               ImuBuf[CurIndex], FRAME_LEN);
  }

  return (res == HAL_OK && who == MPU_ID) ? HAL_OK : HAL_ERROR;
}

/* 连续采样：DMA完成回调里翻转缓冲并续传   */
void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef* Hi2c) {
  if (Hi2c->Instance != I2C1) return;
  FrameSeq++;
  HasFrame = 1;
  CurIndex ^= 1U; /* 切换到另一块缓冲写入下一帧 */
  (void)HAL_I2C_Mem_Read_DMA(&hi2c1, MPU_ADDR, MPU_ACCEL_XOUTH_REG, MPU_REG_ADDR_SIZE,
                             ImuBuf[CurIndex], FRAME_LEN);
}

/* 错误自恢复，避免总线卡死后停止采样  */
void HAL_I2C_ErrorCallback(I2C_HandleTypeDef* Hi2c) {
  if (Hi2c->Instance != I2C1) return;
  HAL_I2C_DeInit(Hi2c);
  HAL_Delay(2);
  HAL_I2C_Init(Hi2c);
  (void)HAL_I2C_Mem_Read_DMA(&hi2c1, MPU_ADDR, MPU_ACCEL_XOUTH_REG, MPU_REG_ADDR_SIZE,
                             ImuBuf[CurIndex], FRAME_LEN);
}

/* -------------------------------------------------------------------------- */
/* 将最近完成的一帧解析并写入全局结构体 MPU6050                              */
/* -------------------------------------------------------------------------- */
HAL_StatusTypeDef MPU_GetData(void) {
  if (!HasFrame) return HAL_BUSY;

  /* 取“上一块”完成的缓冲（当前 CurIndex 正在DMA写入） */
  uint8_t ReadyIndex;
  __disable_irq();
  ReadyIndex = CurIndex ^ 1U;
  __enable_irq();

  uint8_t buf[FRAME_LEN];
  /* 快拷贝14字节到栈上，避免被DMA覆盖 */
  __disable_irq();
  memcpy(buf, ImuBuf[ReadyIndex], FRAME_LEN);
  __enable_irq();

  MPU_t tmp;
  tmp.accX = (int16_t)(buf[0] << 8 | buf[1])/*  - MpuOffset[0] */;
  tmp.accY = (int16_t)(buf[2] << 8 | buf[3])/*  - MpuOffset[1] */;
  tmp.accZ = (int16_t)(buf[4] << 8 | buf[5])/*  - MpuOffset[2] */;
  /* 跳过温度 buf[6], buf[7] */
  tmp.gyroX = (int16_t)(buf[8] << 8 | buf[9])/*  - MpuOffset[3] */;
  tmp.gyroY = (int16_t)(buf[10] << 8 | buf[11])/*  - MpuOffset[4] */;
  tmp.gyroZ = (int16_t)(buf[12] << 8 | buf[13])/*  - MpuOffset[5] */;

  /* 原子更新全局结构体 */
  __disable_irq();
  MPU6050 = tmp;
  __enable_irq();

  return HAL_OK;
}
