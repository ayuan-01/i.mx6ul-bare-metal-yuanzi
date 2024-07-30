#ifndef __BEEP_H
#define __BEEP_H

#include "fsl_iomuxc.h"
#include "fsl_common.h"
#include "MCIMX6Y2.h"

void beep_init(void);
void beep_switch(int status);

#endif // !__BEEP_H