#include "main.h"
#include "bsp_gpio.h"
#include "bsp_exit.h"

int main(void)
{
    unsigned char led_state = OFF;
    
    
    int_init();         // 初始化中断
    imx6u_clkinit();    // 初始化系统时钟
    clk_enable();       // 使能外设时钟
    beep_init();        // 初始化蜂鸣器
    led_init();         // 初始化led灯
    key_init();         // 初始化按键
    exit_init();        // 初始化外部中断

    while(1){
        led_state = !led_state;
        led_switch(LED0, led_state);
        delay(500);
    }
    return 0;
}

