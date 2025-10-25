#include "ALL_DATA.h"
#include "ALL_DEFINE.h"

Remote_t Remote;
MPU_t MPU6050;
Ange_t Angle;

flag_state_t flagState;

pid_t pidRateX;
pid_t pidRateY;
pid_t pidRateZ;
pid_t pidPitch;
pid_t pidRoll;
pid_t pidYaw;

int16_t motor_pwm_value[4];

uint8_t USB_Receive_Buff[64];    // USB接收缓冲区
uint8_t USB_Send_Buff[64];      // USB发送缓冲区

void All_Init(void) {
  Motor_Init();
}
