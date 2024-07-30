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
}

/**
 * @brief : 控制GPIO高低电平
 * @param : base,GPIO_Type
 * @param : pin,pin*
 * @param : value,config->outputLogic
 */
void gpio_pinwrite(GPIO_Type *base, int pin, int value)
{
    if(value == 0)
    {
        base->DR &= ~(1 << pin);
    }
    else
    {
        base->DR |= (1 << pin);
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

