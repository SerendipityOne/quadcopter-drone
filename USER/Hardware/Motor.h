#ifndef __MOTOR_H
#define __MOTOR_H

#include "ALL_DATA.h"
#include "main.h"

void Motor_Init(void);
void Motor_SetPWM(int16_t PWM, uint8_t motor);

#endif  // !__MOTOR_H
