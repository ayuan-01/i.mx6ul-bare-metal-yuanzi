#include "bsp_beep.h"

/* beep初始化 */
void beep_init(void)
{
    IOMUXC_SetPinMux(IOMUXC_SNVS_SNVS_TAMPER1_GPIO5_IO01, 0);
    IOMUXC_SetPinConfig(IOMUXC_SNVS_SNVS_TAMPER1_GPIO5_IO01,0x10b0);

    /* GPIO初始化 */
    GPIO5->GDIR |= (1 << 1);
    GPIO5->DR   |= (1 << 1);
}

/* beep控制函数 */
void beep_switch(int status)
{
    switch (status)
    {
    case ON:
        GPIO5->DR &= ~(1 << 1);
        break;
    case OFF:
        GPIO5->DR |= (1 << 1);
        break;
    default:
        break;
    }
}
