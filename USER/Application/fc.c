#include "fc.h"
#include "MPU6050.h"  // 提供 g_mpu 与 MPU_GetData()
#include "tim.h"      // 需要 htim1, htim3

/* ---------- 顶/底半部共享状态（命名按你的要求） ---------- */
volatile uint8_t FcReq = 0;                    // 有一帧待执行
static volatile uint32_t TimeStampPrevUs = 0;  // 上一拍时间戳(微秒)
static volatile uint32_t TimeStampCurrUs = 0;  // 当前拍时间戳(微秒)
static volatile float LastDt = 0.003f;         // 默认3ms

void Fc_Init(void) {
  /* TIM3 作为 1MHz 自由运行计数器：Cube里 PSC=72-1, ARR=65535；这里只需启动不带中断 */
  HAL_TIM_Base_Start(&htim3);

  /* TIM1 作为 3ms 周期定时器：Cube里已配置中断，这里启动带中断 */
  HAL_TIM_Base_Start_IT(&htim1);
}

void Fc_RequestTickIsr(void) {
  /* 拍当前时间戳（微秒） */
  TimeStampCurrUs = __HAL_TIM_GET_COUNTER(&htim3);

  /* 如上次未处理，覆盖即可（也可在此统计miss） */
  FcReq = 1;

  /* 触发 PendSV 在主循环之上运行底半部 */
  SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
}

void Fc_RunOnce(void) {
  /* 计算 dt，考虑16位回绕 */
  uint32_t prev = TimeStampPrevUs;
  uint32_t curr = TimeStampCurrUs;
  uint32_t DeltaUs = (curr >= prev) ? (curr - prev) : (curr + 0x10000u - prev);
  TimeStampPrevUs = curr;

  float dt = (prev == 0) ? 0.003f : (DeltaUs * 1e-6f);
  LastDt = dt;

  /* 获取IMU最新原始数据（DMA双缓冲在 MPU6050.c 中后台运行） */
  (void)MPU_GetData();  // 先只获取原始数据，姿态解算后续再接
}

float Fc_GetLastDt(void) {
  return LastDt;
}

/* ---------- PendSV 底半部 ---------- */
/* void PendSV_Handler(void) {
  if (!FcReq) return;
  FcReq = 0;
  Fc_RunOnce();
} */
