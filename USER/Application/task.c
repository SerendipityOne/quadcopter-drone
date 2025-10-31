#include "task.h"
#include "ALL_DEFINE.h"

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim) {
  if (htim->Instance == TIM1) {  // 3ms
    if (taskState.state == TASK_STOP) taskState.state = TASK_RUN;
  }
}
