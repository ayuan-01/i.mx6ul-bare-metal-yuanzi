#ifndef __BSP_EXIT_H
#define __BSP_EXIT_H

#include "imx6u.h"

void exit_init(void);

void gpio1_io18_irqhandler(unsigned int gicciar, void *param);

#endif // !__BSP_EXIT_H


