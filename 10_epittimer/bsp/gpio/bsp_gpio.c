#include "bsp_gpio.h"
/**
 * @brief : 初始化GPIO
 * @param base : GPIO*
 * @param pin  : pin*
 * @param config : gpio_pin_config_t
 */ 
void gpio_init(GPIO_Type *base, int pin, gpio_pin_config_t *config)
{
    if(config->direction == kGPIO_DigitalInput)
    {
        base->GDIR &= ~(1 << pin);
    }
    else
    {
        base->GDIR |= (1 << pin);
        /* 设置默认输出电平 */
        gpio_pinwrite(base, pin, config->outputLogic);
    }
    gpio_intconfig(base, pin, config->interruptMode);
}

/**
 * @brief : 控制GPIO高低电平
 * @param : base,GPIO_Type
 * @param : pin,pin*
 * @param : value,config->outputLogic
 */
void gpio_pinwrite(GPIO_Type *base, int pin, int value)
{
    if(value == 0U)
    {
        base->DR &= ~(1U << pin);
    }
    else
    {
        base->DR |= (1U << pin);
    }
}

/**
 * @brief : 读取GPIO电平
 * @param : base,GPIO_Type
 * @param : pin,pin*
 * @return :  
 */
int gpio_pinread(GPIO_Type *base, int pin)
{
    return (((base->DR) >> pin) & 0x1);
}

/* 使能GPIO中断 */
void gpio_enableint(GPIO_Type *base, unsigned int pin)
{
    base->IMR |= (1 << pin);
}

/* 禁止GPIO中断 */
void gpio_disableint(GPIO_Type *base, unsigned int pin)
{
    base->IMR &= ~(1 << pin);
}

/* 清除中断标志位 */
void gpio_clearintflags(GPIO_Type *base, unsigned int pin)
{
    base->ISR |= (1 << pin);
}

/* GPIO中断初始化函数 */
void gpio_intconfig(GPIO_Type *base, unsigned int pin, gpio_interrupt_mode_t pin_int_mode)
{
    volatile uint32_t *icr;
    uint32_t icrShift;

    icrShift = pin;
    base->EDGE_SEL &= ~(1U << pin);      /* SEL置1时设置ICR寄存器无效，所以给此寄存器清零 */
    
    if (pin < 16)                       /* 低16位 */
    {
        icr = &(base->ICR1);
    }
    else
    {
        icr = &(base->ICR2);
        icrShift -= 16;
    }
    switch (pin_int_mode)
    {
    case kGPIO_IntLowLevel:
        *icr &= ~(3U << 2*icrShift);
        *icr |= (0U << 2*icrShift);
        break;
    case kGPIO_IntHighLevel:
        *icr &= ~(3U << 2*icrShift);
        *icr |= (1U << 2*icrShift);
        break;
    case kGPIO_IntRisingEdge:
        *icr &= ~(3U << 2*icrShift);
        *icr |= (2U << 2*icrShift);
        break;
    case kGPIO_IntFallingEdge:
        *icr &= ~(3U << 2*icrShift);
        *icr |= (3U << 2*icrShift);
        break;
    case kGPIO_IntRisingOrFallingEdge:
        base->EDGE_SEL |= (1U << pin);
        break;
    default:
        break;
    }

}