#ifndef __BSP_DELAY
#define __BSP_DELAY

#include "imx6u.h"

void delay_init(void);
void gpt1_irqhandler(unsigned int gicciar, void *param);
void delay_short(volatile unsigned int n);
void delay(volatile unsigned int n);
void delay_us(unsigned int usdelay);
void delay_ms(unsigned int msdelay);

#endif // !__BSP_DELAY

