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

extern TIM_HandleTypeDef htim1;

void All_Init(void) {
  USB_HID_PowerOff();
  delay_ms(100);
  USB_HID_PowerOn();

  Motor_Init();
  MPU6050_Init();

  // delay_ms(5000);  MPU_SetOffset();

  HAL_TIM_Base_Start_IT(&htim1);
}
