#ifndef __FC_H
#define __FC_H

#include "main.h"

void Fc_Init(void);
void Fc_RequestTickIsr(void);
void Fc_RunOnce(void);
float Fc_GetLastDt(void);

#endif  // !__FC_H
