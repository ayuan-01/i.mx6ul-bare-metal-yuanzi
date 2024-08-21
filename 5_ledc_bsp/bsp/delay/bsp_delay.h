#ifndef __BSP_DELAY
#define __BSP_DELAY

#include "fsl_iomuxc.h"
#include "MCIMX6Y2.h"
#include "fsl_common.h"

void delay_short(volatile unsigned int n);
void delay(volatile unsigned int n);

#endif // !__BSP_DELAY

