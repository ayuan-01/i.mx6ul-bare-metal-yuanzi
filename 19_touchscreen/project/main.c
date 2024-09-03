#include "main.h"
#include "bsp_ft5426.h"
#include "bsp_gt9147.h"

void imx6ul_hardfpu_enable(void);

int main(void)
{
	unsigned char i = 0;
	unsigned char state = OFF;

	imx6ul_hardfpu_enable();
    int_init();         // 初始化中断
    imx6u_clkinit();    // 初始化系统时钟
    clk_enable();       // 使能外设时钟
    beep_init();        // 初始化蜂鸣器
    led_init();         // 初始化led灯
    key_init();         // 初始化按键
    delay_init();       // 初始化GPT高精度定时
    uart_init();        // 初始化串口1
    lcd_init();         // 初始化LCD
    rtc_init();         // 初始化RTC
	ap3216c_init();     // 初始化ap3216c
	// icm20608_init();	// 初始化ICM20608

	/* 初始化触摸屏					*/ 
	gt9147_init();
	if(gt_init_fail==1) //判断gt系列初始化是否失败
	{
		// ft5426_init();
	}

	tftlcd_dev.forecolor = LCD_RED;
	tftlcd_dev.backcolor = LCD_WHITE;
	lcd_show_string(50, 10, 400, 24, 24, (char*)"IMX6U-ALPHA TOUCH SCREEN TEST");  
	lcd_show_string(50, 40, 200, 16, 16, (char*)"TOUCH SCREEN TEST");  
	lcd_show_string(50, 60, 200, 16, 16, (char*)"ATOM@ALIENTEK");  
	lcd_show_string(50, 80, 200, 16, 16, (char*)"2019/3/27");  
	
	lcd_show_string(50, 110, 400, 16, 16,	(char*)"TP Num	:");  
	lcd_show_string(50, 130, 200, 16, 16,	(char*)"Point0 X:");  
	lcd_show_string(50, 150, 200, 16, 16,	(char*)"Point0 Y:");  
	lcd_show_string(50, 170, 200, 16, 16,	(char*)"Point1 X:");  
	lcd_show_string(50, 190, 200, 16, 16,	(char*)"Point1 Y:");  
	lcd_show_string(50, 210, 200, 16, 16,	(char*)"Point2 X:");  
	lcd_show_string(50, 230, 200, 16, 16,	(char*)"Point2 Y:");  
	lcd_show_string(50, 250, 200, 16, 16,	(char*)"Point3 X:");  
	lcd_show_string(50, 270, 200, 16, 16,	(char*)"Point3 Y:");  
	lcd_show_string(50, 290, 200, 16, 16,	(char*)"Point4 X:");  
	lcd_show_string(50, 310, 200, 16, 16,	(char*)"Point4 Y:");  
	tftlcd_dev.forecolor = LCD_BLUE;
	
	while(1)					
	{
		if(gt_init_fail==1) {
			lcd_shownum(50 + 72, 110, ft5426_dev.point_num , 1, 16);
			lcd_shownum(50 + 72, 130, ft5426_dev.x[0], 5, 16);
			lcd_shownum(50 + 72, 150, ft5426_dev.y[0], 5, 16);
			lcd_shownum(50 + 72, 170, ft5426_dev.x[1], 5, 16);
			lcd_shownum(50 + 72, 190, ft5426_dev.y[1], 5, 16);
			lcd_shownum(50 + 72, 210, ft5426_dev.x[2], 5, 16);
			lcd_shownum(50 + 72, 230, ft5426_dev.y[2], 5, 16);
			lcd_shownum(50 + 72, 250, ft5426_dev.x[3], 5, 16);
			lcd_shownum(50 + 72, 270, ft5426_dev.y[3], 5, 16);
			lcd_shownum(50 + 72, 290, ft5426_dev.x[4], 5, 16);
			lcd_shownum(50 + 72, 310, ft5426_dev.y[4], 5, 16);
		} else {
			lcd_shownum(50 + 72, 110, gt9147_dev.point_num , 1, 16);
			lcd_shownum(50 + 72, 130, gt9147_dev.x[0], 5, 16);
			lcd_shownum(50 + 72, 150, gt9147_dev.y[0], 5, 16);
			lcd_shownum(50 + 72, 170, gt9147_dev.x[1], 5, 16);
			lcd_shownum(50 + 72, 190, gt9147_dev.y[1], 5, 16);
			lcd_shownum(50 + 72, 210, gt9147_dev.x[2], 5, 16);
			lcd_shownum(50 + 72, 230, gt9147_dev.y[2], 5, 16);
			lcd_shownum(50 + 72, 250, gt9147_dev.x[3], 5, 16);
			lcd_shownum(50 + 72, 270, gt9147_dev.y[3], 5, 16);
			lcd_shownum(50 + 72, 290, gt9147_dev.x[4], 5, 16);
			lcd_shownum(50 + 72, 310, gt9147_dev.y[4], 5, 16);
		}
		delay_ms(10);
		i++;
	
		if(i == 50)
		{	
			i = 0;
			state = !state;
			led_switch(LED0,state); 
		}
	}
	return 0;
}

/*
 * @description	: 使能I.MX6U的硬件NEON和FPU
 * @param 		: 无
 * @return 		: 无
 */
 void imx6ul_hardfpu_enable(void)
{
	uint32_t cpacr;
	uint32_t fpexc;

	/* 使能NEON和FPU */
	cpacr = __get_CPACR();
	cpacr = (cpacr & ~(CPACR_ASEDIS_Msk | CPACR_D32DIS_Msk))
		   |  (3UL << CPACR_cp10_Pos) | (3UL << CPACR_cp11_Pos);
	__set_CPACR(cpacr);
	fpexc = __get_FPEXC();
	fpexc |= 0x40000000UL;	
	__set_FPEXC(fpexc);
}
