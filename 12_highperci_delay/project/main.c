#include "main.h"
#include "bsp_gpio.h"
#include "bsp_exit.h"
#include "bsp_epit.h"
#include "bsp_keyfilter.h"

int main(void)
{
    unsigned char led_state = 0;
    int_init();         // 初始化中断
    imx6u_clkinit();    // 初始化系统时钟
    clk_enable();       // 使能外设时钟
    beep_init();        // 初始化蜂鸣器
    led_init();         // 初始化led灯
    key_init();         // 初始化按键
    delay_init();
    while(1){
        led_state = !led_state;
        led_switch(LED0, led_state);
        delay_ms(100);
    }
    return 0;
}

