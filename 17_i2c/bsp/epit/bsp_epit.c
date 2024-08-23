#include "bsp_epit.h"
#include "bsp_int.h"
#include "bsp_led.h"


/**
 * @brief 初始化EPIT定时器设置分频值和载入值
 * @param frac : 分频值
 * @param value : 载入值
 */
void epit1_int(unsigned int frac, unsigned int value)
{
    /* 分频值最大为4095 */
    if (frac > 4095)
    {
        frac = 4095;
    }
    /* 配置EPIT的CR寄存器 */
    EPIT1->CR = 0;
    /**
     * bit1=1，计数器从load值或者0xffffffff开始计数
     * bit2=1使能比较中断
     * bit3=1从寄存器记录的值重新计数
     * bit4-15分配值frac
     * bit24=1设置定时器时钟源
     */
    EPIT1->CR = (1 << 1) | (1 << 2) | (1 << 3) | (frac << 4) | (1 << 24);
    EPIT1->LR = value;
    EPIT1->CMPR =0;

    /* 初始化中断 */
    GIC_EnableIRQ(EPIT1_IRQn);
    system_register_irqhandler(EPIT1_IRQn, epit1_irqhandler, NULL);   
    /* 打开EPIT1 */ 
    EPIT1->CR |= (1 << 0);
}

/* EPIT1中断服务函数 */
void epit1_irqhandler(unsigned int gicciar, void *param)
{
    static unsigned char state = 0;
    state = !state;
    if (EPIT1->SR & (1 << 0))   /* 中断发生了 */
    {
        led_switch(LED0, state);
    }
    /* 清除中断标志位 */
    EPIT1->SR |= (1 << 0);
}
