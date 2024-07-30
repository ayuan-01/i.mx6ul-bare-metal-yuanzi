#include "main.h"

int main(void)
{
    clk_enable();   //使能外设时钟
    led_init();
    while(1){
        led_on();
        delay(100);
        led_off();
        delay(100);
    }
    return 0;
}