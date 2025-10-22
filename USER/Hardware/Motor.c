#include "Motor.h"
#include "my_math.h"

extern TIM_HandleTypeDef htim2;

void Motor_Init(void) {
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3);
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_4);
}

/**
 * @brief 设置无人机4个电机PWM
 * @param PWM PWM值
 * @param motor 电机编号 1-4
 * @retval None
 */
void Motor_SetPWM(int16_t PWM, uint8_t motor) {
  PWM = LIMIT(PWM, 0, 1000);
  if (motor == 1) {
    TIM2->CCR1 = PWM;  // 顺时针PWM为正
  } else if (motor == 2) {
    TIM2->CCR2 = PWM;  // 逆时针PWM为正
  } else if (motor == 3) {
    TIM2->CCR3 = PWM;  // 逆时针PWM为正
  } else if (motor == 4) {
    TIM2->CCR4 = PWM;  // 顺时针PWM为正
  } else {
    TIM2->CCR1 = 0;
    TIM2->CCR2 = 0;
    TIM2->CCR3 = 0;
    TIM2->CCR4 = 0;
  }
}
