#include "MPU6050.h"
#include "MyI2C.h"
#include "kalman.h"
#include "my_math.h"
//**************************************************************
static volatile int16_t* mpu = (int16_t*)&MPU6050;
//**************************************************************
/**
 * @brief 初始化MPU6050
 * @return 成功返回SUCCESS，失败返回ERROR
 */
uint8_t MPU6050_Init(void) {
  uint8_t res = SUCCESS;
  do {
    res = MyI2C_Write_One_Byte(MPU_ADDR, MPU_PWR_MGMT1_REG, 0x80);  // 复位唤醒MPU6050，失能温度传感器
    /* 采样率 = 陀螺仪输出率 / (1 + SMPLRT_DIV) */
    res += MyI2C_Write_One_Byte(MPU_ADDR, MPU_SAMPLE_RATE_REG, 0x02);  // 设置采样频率为333hz
    res += MyI2C_Write_One_Byte(MPU_ADDR, MPU_PWR_MGMT1_REG, 0x03);    // 设置设备时钟源，陀螺仪Z轴
    res += MyI2C_Write_One_Byte(MPU_ADDR, MPU_CFG_REG, 0x03);          // 配置DLPF，一般为采样率一半，0x03(42Hz)
    res += MyI2C_Write_One_Byte(MPU_ADDR, MPU_GYRO_CFG_REG, 0x18);     // 配置陀螺仪量程，±2000度/秒
    res += MyI2C_Write_One_Byte(MPU_ADDR, MPU_ACCEL_CFG_REG, 0x09);    // 配置加速度计量程，±4g，低通滤波器为5hz
  } while (res != SUCCESS);
  return SUCCESS;
}

void MPU_GetAcc(uint8_t* data) {
  MyI2C_Read_Bytes(MPU_ADDR, MPU_ACCEL_XOUTH_REG, data, 6);
}

void MPU_GetGyro(uint8_t* data) {
  MyI2C_Read_Bytes(MPU_ADDR, MPU_GYRO_XOUTH_REG, data, 6);
}

void MPU_GetData(void) {
  uint8_t i, buffer[12];
  MPU_GetAcc(buffer);
  MPU_GetGyro(buffer + 6);

  for (i = 0; i < 6; i++) {
    mpu[i] = (((int16_t)buffer[i << 1] << 8) | buffer[(i << 1) + 1]) /*  - MpuOffset[i] */;  //整合为16bit，并减去水平静止校准值
    //以下对加速度做卡尔曼滤波
    if (i < 3) {
      {
        static struct _1_ekf_filter ekf[3] = {{0.02, 0, 0, 0, 0.001, 0.543},
                                              {0.02, 0, 0, 0, 0.001, 0.543},
                                              {0.02, 0, 0, 0, 0.001, 0.543}};
        kalman_1(&ekf[i], (float)mpu[i]);  //一维卡尔曼
        mpu[i] = (int16_t)ekf[i].out;
      }
    }
    //以下对角速度做一阶低通滤波
    if (i > 2) {
      uint8_t k = i - 3;
      const float factor = 0.15f;  //滤波因素
      static float tBuff[3];

      mpu[i] = tBuff[k] = tBuff[k] * (1 - factor) + mpu[i] * factor;
    }
  }
}
