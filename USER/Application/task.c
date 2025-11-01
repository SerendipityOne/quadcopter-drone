#include "task.h"
#include "ALL_DEFINE.h"

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim) {
  static uint32_t ANTO_Count = 0;
  if (htim->Instance == TIM1) {  // 3ms

    Fc_RequestTickIsr();
    ANTO_Count++;
    if(ANTO_Count >= 10){ // 30ms
      ANTO_Count = 0;
      taskState.ANTO_RUN = RUN;
    }
  }
}
