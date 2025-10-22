#ifndef __LED_H
#define __LED_H

#include "ALL_DATA.h"

typedef struct {
  uint16_t flashTime;
  enum {
    ALWAYS_ON,
    ALWAYS_OFF,
    ALL_FLASH_LIGHT,
    ALTERNATE_FLASH,
    WARNING,
    DANGEROUS,
    GET_OFFSET
  } status;
} LED_t;

extern LED_t LED;
#define LED_TAKE_OFF_ENTER LED.status = WARNING
#define LED_TAKE_OFF_EXIT  LED.status = ALL_FLASH_LIGHT
#define LED_HEIGHT_LOCK_ENTER \
  LED.FlashTime = 50;         \
  LED.status = ALTERNATE_FLASH
#define LED_HEIGHT_LOCK_EXIT \
  LED.FlashTime = 100;       \
  LED.status = ALL_FLASH_LIGHT
#define LED_3D_ROLL_ENTER         LED.status = WARNING
#define LED_3D_ROLL_EXIT          LED.status = ALL_FLASH_LIGHT
#define LED_SAFTY_TAKE_DOWN_ENTER LED.status = DANGEROUS
#define LED_SAFTY_TAKE_DOWN_EXIT  LED.status = ALWAYS_ON
#define LED_GET_MPU_OFFSET_ENTER  LED.status = GET_OFFSET
#define LED_GO_HOME_ENTER         LED.status = WARNING
#define LED_GO_HOME_EXIT          LED.status = ALL_FLASH_LIGHT

void Pilot_LED(void);

#endif  // !__LED_H
