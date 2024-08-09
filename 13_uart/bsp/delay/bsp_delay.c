#include "bsp_delay.h"
#include "bsp_int.h"
#include "bsp_led.h"


/* 延时初始化函数 */
void delay_init(void)
{
    GPT1->CR = 0;

    /* 软复位，置1复位，等待自动清零 */
    GPT1->CR = (1 << 15);
    while ((GPT1->CR >> 15) & 0x01);

    /**
     * bit1: 1：初始值为0
     * bit6-8: 1：时钟源选择ipg_clk = 66MHz
     * bit9: 0：restart模式
     */
    GPT1->CR |= (1 << 1) | (1 << 6);

    /* 分频设置,0-11位设置分频值 */
    GPT1->PR = 65;      // 66分频
    /* 1M的频率计1个数就是1us */
    GPT1->OCR[0] = 0xffffffff;

#if 0
    /* 配置输出比较通道 */
    GPT1->OCR[0] = 1000000/2;    /* 设置中断周期为500ms */

    /* 打开通道1比较中断 */
    GPT1->IR = 1 << 0;

    GIC_EnableIRQ(GPT1_IRQn);
    system_register_irqhandler(GPT1_IRQn, gpt1_irqhandler, NULL);
#endif
    GPT1->CR |= (1 << 0);   /* 打开GPT1 */
}

#if 0
void gpt1_irqhandler(unsigned int gicciar, void *param)
{
    static unsigned char state  = 0;

    /* 判断是哪个中断 */
    if (GPT1->SR & (1 << 0))
    {
        state = !state;
        led_switch(LED0, state);
    }

    /* 清除中断状态 */
    GPT1->SR |= (1 << 0);
}
#endif

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

/* GPT1实现us延时 */
void delay_us(unsigned int usdelay)
{
    unsigned long oldcnt, newcnt;
    unsigned long tcntvalue = 0;

    oldcnt = GPT1->CNT;
    while (1)
    {
        newcnt = GPT1->CNT;
        if(newcnt != oldcnt)
        {
            if(newcnt > oldcnt)
            {
                tcntvalue += newcnt - oldcnt;
            }
            else
            {
                tcntvalue += 0xffffffff - oldcnt + newcnt;
            }
            oldcnt = newcnt;
            if (tcntvalue >= usdelay)
            {
                break;
            }
        } 
    }
}

/* ms延时 */
void delay_ms(unsigned int msdelay)
{
    unsigned int i = 0;
    for (i = 0; i < msdelay; i++)
    {
        delay_us(1000);
    }    
}