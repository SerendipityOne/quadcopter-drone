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

/**
 * @brief  模拟USB HID设备断电
 * @param  None
 * @retval None
 */
void USB_HID_PowerOff(void) {
  // 停止USB设备
  USBD_Stop(&hUsbDeviceFS);
  // 清空缓冲区
  hid_datatemp_begin = 0;
  hid_datatemp_end = 0;
}

/**
 * @brief  模拟USB HID设备上电
 * @param  None
 * @retval None
 */
void USB_HID_PowerOn(void) {
  // 启动USB设备
  USBD_Start(&hUsbDeviceFS);
  // 重置缓冲区指针
  hid_datatemp_begin = 0;
  hid_datatemp_end = 0;
}
