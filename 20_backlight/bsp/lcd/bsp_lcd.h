#ifndef __BSP_LCD_H
#define __BSP_LCD_H

#include "imx6u.h"

extern struct tftlcd_typedef tftlcd_dev;

/* 屏幕ID */
#define ATK4342		0X4342	/* 4.3寸480*272 	*/
#define ATK4384		0X4384	/* 4.3寸800*480 	*/
#define ATK7084		0X7084	/* 7寸800*480 		*/
#define ATK7016		0X7016	/* 7寸1024*600 		*/
#define ATK1018		0X1018	/* 10.1寸1280*800 	*/

/* 颜色 */
#define LCD_BLUE		  0x000000FF
#define LCD_GREEN		  0x0000FF00
#define LCD_RED 		  0x00FF0000
#define LCD_CYAN		  0x0000FFFF
#define LCD_MAGENTA 	  0x00FF00FF
#define LCD_YELLOW		  0x00FFFF00
#define LCD_LIGHTBLUE	  0x008080FF
#define LCD_LIGHTGREEN	  0x0080FF80
#define LCD_LIGHTRED	  0x00FF8080
#define LCD_LIGHTCYAN	  0x0080FFFF
#define LCD_LIGHTMAGENTA  0x00FF80FF
#define LCD_LIGHTYELLOW   0x00FFFF80
#define LCD_DARKBLUE	  0x00000080
#define LCD_DARKGREEN	  0x00008000
#define LCD_DARKRED 	  0x00800000
#define LCD_DARKCYAN	  0x00008080
#define LCD_DARKMAGENTA   0x00800080
#define LCD_DARKYELLOW	  0x00808000
#define LCD_WHITE		  0x00FFFFFF
#define LCD_LIGHTGRAY	  0x00D3D3D3
#define LCD_GRAY		  0x00808080
#define LCD_DARKGRAY	  0x00404040
#define LCD_BLACK		  0x00000000
#define LCD_BROWN		  0x00A52A2A
#define LCD_ORANGE		  0x00FFA500
#define LCD_TRANSPARENT   0x00000000

#define LCD_FRAMEBUF_ADDR   (0x89000000)



/* LCD屏幕信息结构体 */
struct tftlcd_typedef{
    unsigned short height;      /* 屏幕高度 */
    unsigned short width;       /* 屏幕宽度 */
    unsigned char pixsize;      /* 每个像素占用的字节数 */
    unsigned short vspw;
    unsigned short vbpd;
    unsigned short vfpd;
    unsigned short hspw;
    unsigned short hbpd;
    unsigned short hfpd;
    unsigned int framebuffer;   /* 屏幕显存起始地址 */
    unsigned int forecolor;     /* 前景色 */
    unsigned int backcolor;     /* 背景色 */
};

void lcd_init(void);
unsigned short lcd_read_panelid(void);
void lcdgpio_init();
void lcd_reset(void);
void lcd_noreset();
void lcd_enable(void);
void lcdclk_init(unsigned char loopDiv, unsigned char prediv, unsigned char div);
void lcd_drawpoint(unsigned short x, unsigned short y, unsigned int color);
unsigned int lcd_readpoint(unsigned short x, unsigned short y);
void lcd_clear(unsigned int color);
void lcd_fill(unsigned short x0, unsigned short y0, unsigned short x1, unsigned short y1, unsigned int color);

#endif // !__BSP_LCD_H


