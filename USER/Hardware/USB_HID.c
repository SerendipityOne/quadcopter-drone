#include "USB_HID.h"
#include "usbd_customhid.h"

extern USBD_HandleTypeDef hUsbDeviceFS;
extern uint8_t USB_Send_Buff[64];

uint8_t HID_SEND_TIMEOUT = 5;    //hid发送不足一帧时，等待HID_SEND_TIMEOUT周期进行发送
uint8_t hid_datatemp[256];       //hid环形缓冲区
uint8_t hid_datatemp_begin = 0;  //环形缓冲区数据指针，指向应当发送的数据
uint8_t hid_datatemp_end = 0;    //环形缓冲区数据结尾

void USB_HID_Adddata(uint8_t* dataToSend, uint8_t length) {
  uint8_t i;
  for (i = 0; i < length; i++) {
    hid_datatemp[hid_datatemp_end++] = dataToSend[i];
  }
}

void USB_HID_Send(void) {
  static uint8_t notfull_timeout = 0;
  uint8_t i;
  uint8_t data_len;

  // 计算缓冲区中待发送数据的长度
  if (hid_datatemp_end > hid_datatemp_begin) {
    data_len = hid_datatemp_end - hid_datatemp_begin;
  } else if (hid_datatemp_end < hid_datatemp_begin) {
    data_len = 256 - hid_datatemp_begin + hid_datatemp_end;
  } else {
    return;  // 缓冲区为空
  }

  // 如果数据达到63字节或超时，则发送
  if (data_len >= 63 || notfull_timeout >= HID_SEND_TIMEOUT) {
    notfull_timeout = 0;

    // 准备发送缓冲区
    USB_Send_Buff[0] = MIN(data_len, 63);

    // 填充数据
    for (i = 0; i < 63; i++) {
      if (i < data_len) {
        USB_Send_Buff[i + 1] = hid_datatemp[hid_datatemp_begin++];
        // 处理环形缓冲区的回绕
        if (hid_datatemp_begin >= 256) {
          hid_datatemp_begin = 0;
        }
      } else {
        USB_Send_Buff[i + 1] = 0;
      }
    }

    // 使用HAL库的USB发送函数
    USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, USB_Send_Buff, 64);
  } else {
    notfull_timeout++;
  }
}

void USB_Connect(void) {
  USBD_Stop(&hUsbDeviceFS);
  USBD_DeInit(&hUsbDeviceFS);
  __HAL_RCC_USB_CLK_DISABLE();

  // 2) 将 PA12 配置为开漏输出并拉低，模拟“拔掉”
  __HAL_RCC_GPIOA_CLK_ENABLE();
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = GPIO_PIN_12;           // D+ 引脚
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;  // 开漏
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET);

  HAL_Delay(20);  // >=2.5 ms 即可，一般取 10~50 ms 更稳

  // 3) 释放 PA12 引脚
  HAL_GPIO_DeInit(GPIOA, GPIO_PIN_12);

  // 4) 重新开 USB 时钟并按 CubeMX 方式重新初始化 USB 设备（HID）
  __HAL_RCC_USB_CLK_ENABLE();

  // 如果你用的是 CubeMX 生成的 USB_DEVICE_Init()，直接调它
  MX_USB_DEVICE_Init();
}

