#include "bsp_beep.h"

/* beep初始化 */
void beep_init(void)
{
    gpio_pin_config_t beep_config;
    IOMUXC_SetPinMux(IOMUXC_SNVS_SNVS_TAMPER1_GPIO5_IO01, 0);
    IOMUXC_SetPinConfig(IOMUXC_SNVS_SNVS_TAMPER1_GPIO5_IO01,0x10b0);

    /* GPIO初始化 */
    beep_config.direction = kGPIO_DigitalOutput;
    gpio_init(GPIO5, 1, &beep_config);
    gpio_pinwrite(GPIO5, 1, 1);
}

/* beep控制函数 */
void beep_switch(int status)
{
    switch (status)
    {
    case ON:
        gpio_pinwrite(GPIO5, 1, 0);
        break;
    case OFF:
        gpio_pinwrite(GPIO5, 1, 1);
        break;
    default:
        break;
    }
}
