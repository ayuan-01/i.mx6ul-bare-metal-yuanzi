#include "main.h"
#include "bsp_gpio.h"

int main(void)
{
    int i = 0;
    unsigned char led_state = OFF;
    unsigned char beep_state = OFF;
    int keyvalue = 0;
    clk_enable();   //使能外设时钟
    beep_init();
    led_init();
    key_init();

    while(1){
        keyvalue = key_getvalue();
        if(keyvalue)
        {
            switch (keyvalue)
            {
            case KEY0_VALUE:
                beep_state = !beep_state;
                beep_switch(beep_state);
                break;
            
            default:
                break;
            }
        }
        i++;
        if(i == 50)
        {
            i = 0;
            led_state = !led_state;
            led_switch(LED0, led_state);
        }
        delay(10);
    }
    return 0;
}