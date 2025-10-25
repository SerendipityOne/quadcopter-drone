/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : usbd_custom_hid_if.c
  * @version        : v2.0_Cube
  * @brief          : USB Device Custom HID interface file.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "usbd_custom_hid_if.h"

/* USER CODE BEGIN INCLUDE */
#include "ANO_DT.h"
/* USER CODE END INCLUDE */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
// 重定义HID报文大小
#ifdef CUSTOM_HID_EPIN_SIZE
#undef CUSTOM_HID_EPIN_SIZE
#define CUSTOM_HID_EPIN_SIZE 0x40U
#endif  // CUSTOM_HID_EPIN_SIZE

#ifdef CUSTOM_HID_EPOUT_SIZE
#undef CUSTOM_HID_EPOUT_SIZE
#define CUSTOM_HID_EPOUT_SIZE 0x40U
#endif  // CUSTOM_HID_EPOUT_SIZE
/* Private macro -------------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
  * @brief Usb device.
  * @{
  */

/** @addtogroup USBD_CUSTOM_HID
  * @{
  */

/** @defgroup USBD_CUSTOM_HID_Private_TypesDefinitions USBD_CUSTOM_HID_Private_TypesDefinitions
  * @brief Private types.
  * @{
  */

/* USER CODE BEGIN PRIVATE_TYPES */

/* USER CODE END PRIVATE_TYPES */

/**
  * @}
  */

/** @defgroup USBD_CUSTOM_HID_Private_Defines USBD_CUSTOM_HID_Private_Defines
  * @brief Private defines.
  * @{
  */

/* USER CODE BEGIN PRIVATE_DEFINES */

/* USER CODE END PRIVATE_DEFINES */

/**
  * @}
  */

/** @defgroup USBD_CUSTOM_HID_Private_Macros USBD_CUSTOM_HID_Private_Macros
  * @brief Private macros.
  * @{
  */

/* USER CODE BEGIN PRIVATE_MACRO */

/* USER CODE END PRIVATE_MACRO */

/**
  * @}
  */

/** @defgroup USBD_CUSTOM_HID_Private_Variables USBD_CUSTOM_HID_Private_Variables
  * @brief Private variables.
  * @{
  */

/** Usb HID report descriptor. */
__ALIGN_BEGIN static uint8_t CUSTOM_HID_ReportDesc_FS[USBD_CUSTOM_HID_REPORT_DESC_SIZE] __ALIGN_END =
    {
        /* USER CODE BEGIN 0 */
        0x06, 0x00, 0xff,
        0x09, 0x01,
        0xa1, 0x01,
        0x09, 0x01,
        0x15, 0x00,
        0x26, 0xff, 0x00,
        0x95, 0x40,
        0x75, 0x08,
        0x81, 0x02,
        0x09, 0x01,
        0x15, 0x00,
        0x26, 0xff, 0x00,
        0x95, 0x40,
        0x75, 0x08,
        0x91, 0x02,
        /* USER CODE END 0 */
        0xC0 /*     END_COLLECTION	             */
};

/* USER CODE BEGIN PRIVATE_VARIABLES */

/* USER CODE END PRIVATE_VARIABLES */

/**
  * @}
  */

/** @defgroup USBD_CUSTOM_HID_Exported_Variables USBD_CUSTOM_HID_Exported_Variables
  * @brief Public variables.
  * @{
  */
extern USBD_HandleTypeDef hUsbDeviceFS;

/* USER CODE BEGIN EXPORTED_VARIABLES */

/* USER CODE END EXPORTED_VARIABLES */
/**
  * @}
  */

/** @defgroup USBD_CUSTOM_HID_Private_FunctionPrototypes USBD_CUSTOM_HID_Private_FunctionPrototypes
  * @brief Private functions declaration.
  * @{
  */

static int8_t CUSTOM_HID_Init_FS(void);
static int8_t CUSTOM_HID_DeInit_FS(void);
static int8_t CUSTOM_HID_OutEvent_FS(uint8_t event_idx, uint8_t state);

/**
  * @}
  */

USBD_CUSTOM_HID_ItfTypeDef USBD_CustomHID_fops_FS =
    {
        CUSTOM_HID_ReportDesc_FS,
        CUSTOM_HID_Init_FS,
        CUSTOM_HID_DeInit_FS,
        CUSTOM_HID_OutEvent_FS};

/** @defgroup USBD_CUSTOM_HID_Private_Functions USBD_CUSTOM_HID_Private_Functions
  * @brief Private functions.
  * @{
  */

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Initializes the CUSTOM HID media low layer
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CUSTOM_HID_Init_FS(void) {
  /* USER CODE BEGIN 4 */
  return (USBD_OK);
  /* USER CODE END 4 */
}

/**
  * @brief  DeInitializes the CUSTOM HID media low layer
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CUSTOM_HID_DeInit_FS(void) {
  /* USER CODE BEGIN 5 */
  return (USBD_OK);
  /* USER CODE END 5 */
}

/**
  * @brief  Manage the CUSTOM HID class events
  * @param  event_idx: Event index
  * @param  state: Event state
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CUSTOM_HID_OutEvent_FS(uint8_t event_idx, uint8_t state) {
  /* USER CODE BEGIN 6 */
  extern uint8_t USB_Receive_Buff[64];  // USB接收缓冲区
  uint8_t USB_Receive_Count = USBD_GetRxCount(&hUsbDeviceFS, CUSTOM_HID_EPOUT_ADDR);

  USBD_CUSTOM_HID_HandleTypeDef* hhid;                               // 定义一个指向USBD_CUSTOM_HID_HandleTypeDef结构体的指针
  hhid = (USBD_CUSTOM_HID_HandleTypeDef*)hUsbDeviceFS.pClassData;    // 得到USB接收数据的储存地址
  memcpy(USB_Receive_Buff, hhid->Report_buf, USB_Receive_Count);     // 将接收到的数据复制到USB_Receive_Buff中
  if (USB_Receive_Buff[1] == 0xaa && USB_Receive_Buff[2] == 0xaF) {  // 帧头正确
    uint8_t i, check_sum = 0;
    for (i = 1; i < USB_Receive_Buff[0]; i++) {  // buf[0] 为收到PC来的数据总长度
      check_sum += USB_Receive_Buff[i];
    }
    if (check_sum == USB_Receive_Buff[USB_Receive_Buff[0]]) {            // buf[0] 为收到PC来的数据总长度 最后一个buf为PC发来的本次数据校验和，与本次收到计算出的校验和做比较
      if (USB_Receive_Buff[4] >= 0x10 && USB_Receive_Buff[5] <= 0x15) {  // 如果收到的是PID数据则要返回校验值

        checkPID = USB_Receive_Buff[4] << 8 | check_sum;  // 返回PID校验数据
        ANTO_Send(ANTO_CHECK);                            // 收到HID发来的PID则要马上返回校验值给上位机
      }
      ANO_Recive((int8_t*)(USB_Receive_Buff + 1));  // hid接收到数据会调用此函数
    }
    USB_Receive_Buff[1] = 0;  //帧头清0
  }

  return (USBD_OK);
  /* USER CODE END 6 */
}

/* USER CODE BEGIN 7 */
/**
  * @brief  Send the report to the Host
  * @param  report: The report to be sent
  * @param  len: The report length
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
/*
static int8_t USBD_CUSTOM_HID_SendReport_FS(uint8_t *report, uint16_t len)
{
  return USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, report, len);
}
*/
/* USER CODE END 7 */

/* USER CODE BEGIN PRIVATE_FUNCTIONS_IMPLEMENTATION */

/* USER CODE END PRIVATE_FUNCTIONS_IMPLEMENTATION */
/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */
