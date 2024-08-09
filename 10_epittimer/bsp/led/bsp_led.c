#include "bsp_led.h"

/* 初始化LED */
void led_init(void)
{
    IOMUXC_SetPinMux(IOMUXC_GPIO1_IO03_GPIO1_IO03, 0);
    IOMUXC_SetPinConfig(IOMUXC_GPIO1_IO03_GPIO1_IO03, 0x10B0);

    /* GPIO初始化 */
    GPIO1->GDIR  = 0x8;          //设置为输出
    GPIO1->DR   |= (1 << 3);    //关闭LED灯
}

void led_on(void)
{
    GPIO1->DR &= ~(1 << 3);
}

void led_off(void)
{
    GPIO1->DR |= (1 << 3);
}

void led_switch(int led, int status)
{
    switch (led)
    {
    case LED0:
        if (status == ON)
        {
            led_on();
        }
        else if (status == OFF)
        {
            led_off();
        }
        break;
    
    default:
        break;
    }
}
