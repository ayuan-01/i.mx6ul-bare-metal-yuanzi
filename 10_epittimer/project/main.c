#include "main.h"
#include "bsp_gpio.h"
#include "bsp_exit.h"
#include "bsp_epit.h"

int main(void)
{
    int_init();         // 初始化中断
    imx6u_clkinit();    // 初始化系统时钟
    clk_enable();       // 使能外设时钟
    beep_init();        // 初始化蜂鸣器
    led_init();         // 初始化led灯
    key_init();         // 初始化按键
    exit_init();        // 初始化外部中断
    epit1_int(0, 66000000/2);         // 初始化定时器,frac=0即1分频，66000000为1秒，周期为500ms

    while(1){
        delay(500);
    }
    return 0;
}

