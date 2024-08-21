#include "bsp_exit.h"
#include "bsp_gpio.h"
#include "bsp_int.h"
#include "bsp_delay.h"
#include "bsp_beep.h"
#include "bsp_led.h"

/* 外部中断初始化 */
void exit_init(void)
{
    gpio_pin_config_t key_config;
    IOMUXC_SetPinMux(IOMUXC_UART1_CTS_B_GPIO1_IO18, 0);                 //设置引脚复用，GPIO1_18
    IOMUXC_SetPinConfig(IOMUXC_UART1_CTS_B_GPIO1_IO18, 0xf080);         //设置引脚电气属性 设置为上拉输入

    /* GPIO初始化 */
    key_config.direction = kGPIO_DigitalInput;
    key_config.interruptMode = kGPIO_IntFallingEdge;
    key_config.outputLogic = 1;
    gpio_init(GPIO1, 18, &key_config);      //设置引脚为输入

    GIC_EnableIRQ(GPIO1_Combined_16_31_IRQn);
    system_register_irqhandler(GPIO1_Combined_16_31_IRQn, gpio1_io18_irqhandler, NULL);
    /* 使能中断之前需要先注册中断服务函数，否则使能后来中断可能会出问题 */
    gpio_enableint(GPIO1, 18);
}

/* 中断服务函数 */
void gpio1_io18_irqhandler(unsigned int gicciar, void *param)
{
    static unsigned char state = 0;
    delay(10);
    if (gpio_pinread(GPIO1, 18) == 0)
    {
        state = !state;
        beep_switch(state);
    }

    gpio_clearintflags(GPIO1, 18);
}
