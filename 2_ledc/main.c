#include "main.h"

/* 使能外设时钟 */
void clk_enable(void)
{
    CCM_CCGR0 = 0xFFFFFFFF;
    CCM_CCGR1 = 0xFFFFFFFF;
    CCM_CCGR2 = 0xFFFFFFFF;
    CCM_CCGR3 = 0xFFFFFFFF;
    CCM_CCGR4 = 0xFFFFFFFF;
    CCM_CCGR5 = 0xFFFFFFFF;
    CCM_CCGR6 = 0xFFFFFFFF;
}

void delay_short(volatile unsigned int n)
{
    while(n--);
}

/* 1ms */
void delay(volatile unsigned int n)
{
    while (n--)
    {
        delay_short(0x7ff);
    }
    
}

void led_on(void)
{
    GPIO1_DR &= ~(1 << 3);
}

void led_off(void)
{
    GPIO1_DR |= (1 << 3);
}

/* 初始化LED */
void led_init(void)
{
    SW_MUX_GPIO1_IO3 = 0x5;     //复用为GPIO_IO3
    SW_PAD_GPIO1_IO3 = 0x10B0;  //设置电气属性

    /* GPIO初始化 */
    GPIO1_GDIR = 0x8;   //设置为输出
    GPIO1_DR   = 0x0;   //打开LED灯
}

int main(void)
{
    clk_enable();   //使能外设时钟
    led_init();
    while(1){
        led_on();
        delay(500);
        led_off();
        delay(500);
    }
    return 0;
}