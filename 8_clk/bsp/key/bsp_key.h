#ifndef __BSP_KEY_H
#define __BSP_KEY_H

#include "MCIMX6Y2.h"
#include "fsl_common.h"
#include "fsl_iomuxc.h"
#include "bsp_delay.h"
#include "bsp_gpio.h"

enum key_Value
{
    KEY_NONE = 0,
    KEY0_VALUE
};

void key_init(void);
int read_key(void);
int key_getvalue(void);


#endif // !__BSP_KEY_H