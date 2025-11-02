#include "task.h"
#include "ALL_DEFINE.h"

extern TIM_HandleTypeDef htim1;

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim) {
  static volatile uint32_t ANTO_Count = 0;
  if (htim->Instance == TIM1) {  // 3ms

    taskState.MPU_RUN = RUN;
    SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;  // 触发 PendSV 中断

    ANTO_Count++;
    if (ANTO_Count >= 10) {  // 30ms
      ANTO_Count = 0;
      taskState.ANTO_RUN = RUN;
    }
  }
}

/* 初始化任务 */
void Task_Init(void) {
  HAL_TIM_Base_Start_IT(&htim1);
}

/* 运行任务 */
void Task_Run(void) {
  (void)MPU_GetData();
  GetAngle(&MPU6050, &Angle, 0.003f);
}
