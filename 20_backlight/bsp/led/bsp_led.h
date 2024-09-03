#ifndef __BSP_LED_H
#define _BSP_LED_H

#define LED0 0

#include "fsl_iomuxc.h"
#include "MCIMX6Y2.h"
#include "fsl_common.h"

void led_init(void);
void led_on(void);
void led_off(void);
void led_switch(int led, int status);

#endif // !__BSP_LED_H