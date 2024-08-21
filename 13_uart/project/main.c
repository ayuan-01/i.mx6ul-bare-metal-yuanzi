#include "main.h"
#include "stdio.h"

int main(void)
{
    int a, b;
    int c = 100;
    int_init();         // 初始化中断
    imx6u_clkinit();    // 初始化系统时钟
    clk_enable();       // 使能外设时钟
    beep_init();        // 初始化蜂鸣器
    led_init();         // 初始化led灯
    key_init();         // 初始化按键
    delay_init();       // 初始化GPT高精度定时
    uart_init();        // 初始化串口1
    while(1){
        printf("请输入两个整数，使用空格隔开：");
        scanf("%d %d", &a, &b);
        printf("\r\n数据%d+%d=%d\r\n", a, b, a+b);
        printf("%d的十六进制：%#x\r\n", c);
    }
    return 0;
}

