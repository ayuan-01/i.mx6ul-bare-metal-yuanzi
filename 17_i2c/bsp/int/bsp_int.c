#include "bsp_int.h"

/* 记录中断嵌套 */
static unsigned int irqNesting;

/* 中断处理函数表 */
static sys_irq_handle_t irqTable[NUMBER_OF_INT_VECTORS];            /* 160个 */

/* 中断初始化函数 */
void int_init(void)
{
    GIC_Init();
    system_irqtable_init();
    /* 中断向量偏移设置 */
    __set_VBAR(0x87800000);
}

/* 初始化中断处理函数表 */
void system_irqtable_init(void)
{
    unsigned int i = 0;
    irqNesting = 0;
    for (i = 0; i < NUMBER_OF_INT_VECTORS; i++)
    {
        system_register_irqhandler((IRQn_Type)i, default_irqHandler, NULL);
    }
}

/* 注册中断处理函数 */
void system_register_irqhandler(IRQn_Type irq, system_irq_handler_t handler, void *userparam)
{
    irqTable[irq].irqHandler = handler;
    irqTable[irq].userParam  = userparam;
}

/* 具体的中断处理函数，IRQ_Handler会调用子函数 */
void system_irqhandler(unsigned int gicciar)
{
    /* 检查中断ID */
    uint32_t intNum = gicciar & 0x3ffUL;
    if(intNum >= NUMBER_OF_INT_VECTORS)
    {
        return;
    }
    irqNesting++;
    /* 根据中断ID号，读取中断处理函数，然后执行 */
    irqTable[intNum].irqHandler(intNum, irqTable[intNum].userParam);
    irqNesting--;
}

/* 默认中断处理函数 */
void default_irqHandler(unsigned int gicciar, void *userparam)
{
    while(1)
    {

    }
}



