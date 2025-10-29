#ifndef __USB_HID_H
#define __USB_HID_H

#include "main.h"

void USB_HID_Adddata(uint8_t* dataToSend, uint8_t length);
void USB_HID_Send(void);
void USB_HID_PowerOff(void);
void USB_HID_PowerOn(void);

#endif // !__USB_HID_H
