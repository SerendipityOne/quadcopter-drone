#include "LED.h"
#include "main.h"

//--------------------------------------------------------
////右前灯
#define LED1_OFF()    HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET)    //暗
#define LED1_ON()     HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET)  //亮
#define LED1_Toggle() HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin)                 //闪烁
////左前灯
#define LED3_OFF()    HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_SET)    //暗
#define LED3_ON()     HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_RESET)  //亮
#define LED3_Toggle() HAL_GPIO_TogglePin(LED3_GPIO_Port, LED3_Pin)                 //闪烁
//-------------------------------------------------
////右后灯
#define LED2_OFF()    HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET)    //暗
#define LED2_ON()     HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET)  //亮
#define LED2_Toggle() HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin)                 //闪烁
////左后灯
#define LED4_OFF()    HAL_GPIO_WritePin(LED4_GPIO_Port, LED4_Pin, GPIO_PIN_SET)    //暗
#define LED4_ON()     HAL_GPIO_WritePin(LED4_GPIO_Port, LED4_Pin, GPIO_PIN_RESET)  //亮
#define LED4_Toggle() HAL_GPIO_TogglePin(LED4_GPIO_Port, LED4_Pin)                 //闪烁
//--------------------------------------------------------

LED_t LED = {300, ALL_FLASH_LIGHT};

/**
 * @brief LED指示灯控制函数
 * @note 该函数用于控制LED指示灯的状态，根据LED.status的值来执行不同的操作
 *       闪烁间隔时间为300毫秒
 */
void Pilot_LED(void) {  // 500ms 的间隔时间
  static uint32_t lastTime = 0;

  if (uwTick - lastTime < LED.flashTime) {
    return;
  } else
    lastTime = uwTick;
  switch (LED.status) {
    case ALWAYS_OFF:  //常暗
      LED1_ON();
      LED3_ON();
      LED2_ON();
      LED4_ON();
      break;
    case ALL_FLASH_LIGHT:  //全部同时闪烁
      LED1_Toggle();
      LED3_Toggle();
      LED2_Toggle();
      LED4_Toggle();
      break;
    case ALWAYS_ON:  //常亮
      LED1_ON();
      LED3_ON();
      LED2_ON();
      LED4_ON();
      break;
    default:
      LED.status = ALWAYS_OFF;
      break;
  }
}
