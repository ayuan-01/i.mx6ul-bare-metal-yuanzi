#include "bsp_keyfilter.h"
#include "bsp_gpio.h"
#include "bsp_int.h"
#include "bsp_beep.h"

/* 中断实现按键消抖初始化 */
void keyfilter_init(void)
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
    system_register_irqhandler(GPIO1_Combined_16_31_IRQn, gpio1_16_32_irqhandler, NULL);
    /* 使能中断之前需要先注册中断服务函数，否则使能后来中断可能会出问题 */
    gpio_enableint(GPIO1, 18);
    filtertimer_init(66000000/100);
}

/**
 * 初始化定时器,但不使能，要在按键下降沿中断触发之后在按键的中断服务函数中开启，开始计数，
 * 计数满足定义的时间触发定时器中断
 */
void filtertimer_init(unsigned int value)
{
    /* 配置EPIT的CR寄存器 */
    EPIT1->CR = 0;
    /**
     * bit1=1，计数器从load值或者0xffffffff开始计数
     * bit2=1使能比较中断
     * bit3=1从寄存器记录的值重新计数
     * bit4-15分配值frac
     * bit24=1设置定时器时钟源
     */
    EPIT1->CR = (1 << 1) | (1 << 2) | (1 << 3) | (1 << 24);
    EPIT1->LR = value;
    EPIT1->CMPR =0;

    /* 初始化中断 */
    GIC_EnableIRQ(EPIT1_IRQn);
    system_register_irqhandler(EPIT1_IRQn, filtertimer_irqhandler, NULL);   
}

/* 关闭EPIT1定时器 */
void filtertimer_stop(void)
{
    EPIT1->CR &= ~(1 << 0);
}
/* 开启EPIT1定时器 */
void filtertimer_restart(unsigned int value)
{
    EPIT1->CR &= ~(1 << 0);
    EPIT1->LR = value;
    EPIT1->CR |= ( 1<< 0);
}

void filtertimer_irqhandler(unsigned int gicciar, void *param)
{
    static unsigned char state = OFF;
    
    if(EPIT1->SR & (1 << 0))
    {
        /* 关闭定时器 */
        filtertimer_stop();
        if (gpio_pinread(GPIO1, 18) == 0)
        {
            state = !state;
            beep_switch(state);
        }
    }
    /* 清除中断标志位 */
    EPIT1->SR |= (1 << 0);
}

/* 按键中断服务函数 */
void gpio1_16_32_irqhandler(unsigned int gicciar, void *param)
{
    /* 开启定时器 */
    filtertimer_restart(66000000/100);
    /* 清除中断标志位 */
    gpio_clearintflags(GPIO1, 18);
}
