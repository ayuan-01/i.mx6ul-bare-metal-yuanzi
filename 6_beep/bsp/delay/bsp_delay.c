#include "bsp_delay.h"

void delay_short(volatile unsigned int n)
{
    while(n--);
}

/* 1ms */
void delay(volatile unsigned int n)
{
    while (n--)
    {
        delay_short(0x7ff);
    }
    
}