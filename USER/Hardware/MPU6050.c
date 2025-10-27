#include "MPU6050.h"
#include "MyI2C.h"

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
    res += MyI2C_Write_One_Byte(MPU_ADDR, MPU_CFG_REG, 0x01);          // 配置DLPF，一般为采样率一半
    res += MyI2C_Write_One_Byte(MPU_ADDR, MPU_GYRO_CFG_REG, 0x18);     // 配置陀螺仪量程，±2000度/秒
    res += MyI2C_Write_One_Byte(MPU_ADDR, MPU_ACCEL_CFG_REG, 0x09);    // 配置加速度计量程，±4g，低通滤波器为5hz
  } while (res != SUCCESS);
  res = MyI2C_Read_One_Byte(MPU_ADDR, MPU_WHO_AM_I);
  if (res != MPU_ADDR) {
    return ERROR;
  }
  return SUCCESS;
}
