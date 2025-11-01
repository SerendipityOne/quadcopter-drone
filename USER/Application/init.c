#include "ALL_DATA.h"
#include "ALL_DEFINE.h"

Remote_t Remote;
MPU_t MPU6050;
Ange_t Angle;

flag_state_t flagState;
task_state_t taskState;

pid_t pidRateX;
pid_t pidRateY;
pid_t pidRateZ;
pid_t pidPitch;
pid_t pidRoll;
pid_t pidYaw;

int16_t motor_pwm_value[4];

uint8_t USB_Receive_Buff[64];  // USB接收缓冲区
uint8_t USB_Send_Buff[64];     // USB发送缓冲区

void All_Init(void) {
  NVIC_Init();

  USB_HID_PowerOff();
  delay_ms(100);
  USB_HID_PowerOn();

  Motor_Init();
  MPU6050_Init();

  // delay_ms(3000);
  // MPU_SetOffset();

  Fc_Init();
}

void NVIC_Init(void) {
  /* 1) TIM1：3ms节拍（最高） */
  HAL_NVIC_SetPriority(TIM1_UP_IRQn, 0, 1);
  HAL_NVIC_EnableIRQ(TIM1_UP_IRQn);

  /* 2) I2C1 的 DMA（次高） */
  HAL_NVIC_SetPriority(DMA1_Channel7_IRQn, 1, 0);  // I2C1_RX DMA
  HAL_NVIC_EnableIRQ(DMA1_Channel7_IRQn);
  HAL_NVIC_SetPriority(DMA1_Channel6_IRQn, 1, 1);  // I2C1_TX DMA（若用）
  HAL_NVIC_EnableIRQ(DMA1_Channel6_IRQn);

  /* 3) I2C1 事件/错误 */
  HAL_NVIC_SetPriority(I2C1_ER_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(I2C1_ER_IRQn);
  HAL_NVIC_SetPriority(I2C1_EV_IRQn, 2, 1);
  HAL_NVIC_EnableIRQ(I2C1_EV_IRQn);

  /* 4) USB：如使用，可放到 3,2（或 3,1） */
  HAL_NVIC_SetPriority(USB_LP_CAN1_RX0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);

  /* 5) PendSV 最低（飞控底半部） */
  HAL_NVIC_SetPriority(PendSV_IRQn, 3, 3);
}
