#ifndef __ALL_USER_DATA_H
#define __ALL_USER_DATA_H
// *****************************************************************************
typedef signed char int8_t;
typedef signed short int int16_t;
typedef signed int int32_t;
typedef signed long long int64_t;

/* exact-width unsigned integer types */
typedef unsigned char uint8_t;
typedef unsigned short int uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

// *****************************************************************************
#define u8  uint8_t
#define u16 uint16_t
#define u32 uint32_t

#define TASK_RUN 1
#define TASK_STOP 0

#define __IO volatile
// *****************************************************************************
//#define NULL 0
extern volatile uint32_t uwTick;  //    系统滴答计数器
// *****************************************************************************
typedef enum {
  FALSE = 0,
  TRUE = !FALSE
} bool;

typedef struct {
  int16_t accX;   // 加速度计X轴
  int16_t accY;   // 加速度计Y轴
  int16_t accZ;   // 加速度计Z轴
  int16_t gyroX;  // 陀螺仪X轴
  int16_t gyroY;  // 陀螺仪Y轴
  int16_t gyroZ;  // 陀螺仪Z轴
} MPU_t;

typedef struct {
  float roll;   // 横滚角
  float pitch;  // 俯仰角
  float yaw;    // 偏航角
} Ange_t;

typedef struct {
  uint16_t roll;   // 横滚通道
  uint16_t pitch;  // 俯仰通道
  uint16_t thr;    // 油门通道
  uint16_t yaw;    // 偏航通道
  uint16_t AUX1;   // 辅助通道1
  uint16_t AUX2;   // 辅助通道2
  uint16_t AUX3;   // 辅助通道3
  uint16_t AUX4;   // 辅助通道4
} Remote_t;

typedef volatile struct {
  float desired;         // 期望值
  float offset;          // 偏移量
  float prevError;       // 上次误差
  float integ;           // 积分项
  float kp;              // 比例系数
  float ki;              // 积分系数
  float kd;              // 微分系数
  float IntegLimitHigh;  // 积分上限
  float IntegLimitLow;   // 积分下限
  float measured;        // 测量值
  float out;             // 输出值
  float OutLimitHigh;    // 输出上限
  float OutLimitLow;     // 输出下限
} pid_t;

typedef volatile struct {
  uint8_t unlock;
} flag_state_t;

typedef volatile struct {
  uint8_t state;
} task_state_t;
// *****************************************************************************
extern Remote_t Remote;
extern MPU_t MPU6050;
extern Ange_t Angle;

extern flag_state_t flagState;
extern task_state_t taskState;

extern pid_t pidRateX;
extern pid_t pidRateY;
extern pid_t pidRateZ;

extern pid_t pidPitch;
extern pid_t pidRoll;
extern pid_t pidYaw;

extern int16_t motor_pwm_value[4];

#endif
