#include "main.h"

int main(void)
{
    clk_enable();   //使能外设时钟
    beep_init();
    led_init();
    while(1){
        beep_switch(ON);
        led_switch(LED0, ON);
        delay(100);
        beep_switch(OFF);
        led_switch(LED0, OFF);
        delay(1000);
    }
    return 0;
}