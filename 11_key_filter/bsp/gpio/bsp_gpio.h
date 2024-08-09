#ifndef __BSP_GPIO_H
#define __BSP_GPIO_H

#include "MCIMX6Y2.h"
#include "fsl_common.h"
#include "fsl_iomuxc.h"

typedef enum _gpio_interrupt_mode
{
    kGPIO_NoIntmode                 = 0U,
    kGPIO_IntLowLevel               = 1U,
    kGPIO_IntHighLevel              = 2U,
    kGPIO_IntRisingEdge             = 3U,
    kGPIO_IntFallingEdge            = 4U,
    kGPIO_IntRisingOrFallingEdge    = 5U,
}gpio_interrupt_mode_t;

/* 定义GPIO的输入输出方向 */
typedef enum _gpio_pin_direction
{
    kGPIO_DigitalInput  = 0U,
    kGPIO_DigitalOutput = 1U,
}gpio_pin_direction_t;

/* 输入输出配置和输出电平 */
typedef struct _gpio_pin_config
{
    gpio_pin_direction_t direction;
    uint8_t outputLogic;
    gpio_interrupt_mode_t interruptMode;
}gpio_pin_config_t;

void gpio_init(GPIO_Type *base, int pin, gpio_pin_config_t *config);
void gpio_pinwrite(GPIO_Type *base, int pin, int value);
int gpio_pinread(GPIO_Type *base, int pin);
void gpio_enableint(GPIO_Type *base, unsigned int pin);
void gpio_disableint(GPIO_Type *base, unsigned int pin);
void gpio_clearintflags(GPIO_Type *base, unsigned int pin);
void gpio_intconfig(GPIO_Type *base, unsigned int pin, gpio_interrupt_mode_t pin_int_mode);

#endif // !__BSP_GPIO_H

