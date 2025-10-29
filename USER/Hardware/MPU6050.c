#include "MPU6050.h"
#include "MyI2C.h"
#include "flash.h"
#include "kalman.h"
#include "my_math.h"
#include "string.h"
//**************************************************************
extern TIM_HandleTypeDef htim1;

int16_t mpuOffset[6] = {0};
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

  FLASH_Read(mpuOffset, 6);
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
    mpu[i] = (((int16_t)buffer[i << 1] << 8) | buffer[(i << 1) + 1]) - mpuOffset[i];  //整合为16bit，并减去水平静止校准值
    //以下对加速度做卡尔曼滤波
    if (i < 3) {
      {
        // static struct _1_ekf_filter ekf[3] = {{0.02, 0, 0, 0, 0.001, 0.543},
        //                                       {0.02, 0, 0, 0, 0.001, 0.543},
        //                                       {0.02, 0, 0, 0, 0.001, 0.543}};
        static struct _1_ekf_filter ekf[3] = {{0.02, 0, 0, 0, 0.0245, 0.08},
                                              {0.02, 0, 0, 0, 0.0245, 0.08},
                                              {0.02, 0, 0, 0, 0.0245, 0.08}};

        kalman_1(&ekf[i], (float)mpu[i]);  //一维卡尔曼
        mpu[i] = (int16_t)ekf[i].out;
      }
    }
    //以下对角速度做一阶低通滤波
    if (i > 2) {
      uint8_t k = i - 3;
      const float factor = 0.75f;  //滤波因素
      static float tBuff[3];

      mpu[i] = tBuff[k] = tBuff[k] * (1 - factor) + mpu[i] * factor;
    }
  }
}

/**
 * @brief 设置MPU6050传感器偏移量，用于校准传感器
 * @details 该函数通过在静止状态下多次采样传感器数据来计算偏移量，
 *          并将结果保存到Flash中供后续使用。过程包括:
 *          1. 等待陀螺仪稳定（静止状态）
 *          2. 采集大量样本数据
 *          3. 计算平均偏移量
 *          4. 将偏移量写入Flash存储
 * @note 此函数会临时停止定时器中断以确保采样准确性，在函数结束前重新启动定时器
 * @param 无
 * @retval 无
 */
void MPU_SetOffset(void) {
  int32_t buffer[6] = {0};
  uint8_t k = 30;
  uint16_t i;

  // 定义陀螺仪静止状态的最大和最小误差阈值
  const int8_t MAX_GYRO_QUIET = 5;
  const int8_t MIN_GYRO_QUIET = -5;
  int16_t lastGyro[3] = {0};  // 上一次陀螺仪读数
  int16_t errorGyro[3];       // 当前与上一次读数的误差

  // 初始化偏移数组，重力加速度轴设置初始值
  memset(mpuOffset, 0, sizeof(mpuOffset));
  mpuOffset[2] = 8192;

  // 停止定时器中断，确保数据采集不受干扰
  HAL_TIM_Base_Stop_IT(&htim1);

  // 等待陀螺仪处于静止状态，重复30次以确保稳定性
  while (k--) {
    do {
      delay_ms(10);
      MPU_GetData();
      // 计算陀螺仪三个轴的读数变化量
      for (i = 0; i < 3; i++) {
        errorGyro[i] = mpu[i + 3] - lastGyro[i];
        lastGyro[i] = mpu[i + 3];
      }
    } while ((errorGyro[0] > MAX_GYRO_QUIET) || (errorGyro[0] < MIN_GYRO_QUIET) ||  //标定静止
             (errorGyro[1] > MAX_GYRO_QUIET) || (errorGyro[1] < MIN_GYRO_QUIET) ||
             (errorGyro[2] > MAX_GYRO_QUIET) || (errorGyro[2] < MIN_GYRO_QUIET));
  }

  // 采集356组数据，其中前100组作为预热丢弃，后256组用于计算偏移量
  for (i = 0; i < 356; i++) {
    MPU_GetData();
    if (i >= 100) {
      uint8_t k;
      // 累加各轴的数据
      for (k = 0; k < 6; k++) {
        buffer[k] += mpu[k];
      }
    }
  }

  // 计算各轴的平均偏移量（256个样本的平均值，右移8位相当于除以256）
  for (i = 0; i < 6; i++) {
    mpuOffset[i] = buffer[i] >> 8;
  }

  // 恢复定时器中断
  HAL_TIM_Base_Start_IT(&htim1);

  // 将校准后的偏移量写入Flash存储
  FLASH_Write(mpuOffset, 6);
}
