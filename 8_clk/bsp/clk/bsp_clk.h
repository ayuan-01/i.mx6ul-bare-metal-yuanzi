#ifndef __BSP_CLK_H
#define __BSP_CLK_H

#include "fsl_iomuxc.h"
#include "MCIMX6Y2.h"
#include "fsl_common.h"

/* 使能外设时钟 */
void clk_enable(void);
void imx6u_clkinit(void);

#endif // !__BSP_CLK_H

