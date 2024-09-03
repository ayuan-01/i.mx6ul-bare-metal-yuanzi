#include "bsp_key.h"


/* 初始化按键 */
void key_init(void)
{
    gpio_pin_config_t key_config;
    IOMUXC_SetPinMux(IOMUXC_UART1_CTS_B_GPIO1_IO18, 0);                 //设置引脚复用，GPIO1_18
    IOMUXC_SetPinConfig(IOMUXC_UART1_CTS_B_GPIO1_IO18, 0xf080);         //设置引脚电气属性 设置为上拉输入

    /* GPIO初始化 */
    key_config.direction = kGPIO_DigitalInput;
    gpio_init(GPIO1, 18, &key_config);      //设置引脚为输入
}

/* 读取GPIO1_IO18数据寄存器值 */
int read_key(void)
{
    int red = 0;
    red = gpio_pinread(GPIO1, 18);
    return red;
}

/* 按键消抖读取按键值 */
int key_getvalue(void)
{
    int ret = 0;
    static unsigned int release = 1;    //“1”表示按键处于释放状态

    if ((release == 1) && (read_key() == 0))
    {
        delay(10);
        if (read_key() == 0)
        {
            release = 0;
            ret = KEY0_VALUE;
        }
    }
    else if(read_key() == 1)
    {
        ret = KEY_NONE;
        release = 1;
    }
    return ret;
     
}