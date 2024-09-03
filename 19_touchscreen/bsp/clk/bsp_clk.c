#include "bsp_clk.h"

/* 使能外设时钟 */
void clk_enable(void)
{
    CCM->CCGR0 = 0xFFFFFFFF;
    CCM->CCGR1 = 0xFFFFFFFF;
    CCM->CCGR2 = 0xFFFFFFFF;
    CCM->CCGR3 = 0xFFFFFFFF;
    CCM->CCGR4 = 0xFFFFFFFF;
    CCM->CCGR5 = 0xFFFFFFFF;
    CCM->CCGR6 = 0xFFFFFFFF;
}

/**
 * @brief : 初始化MX6U时钟，包括PLL1.PLL2(PFD1-4)，PLL3(PFD1-4)，和其他外设常用时钟PERCLK，IPG，AHB
 * @param : void
 */
void imx6u_clkinit(void)
{
    unsigned int reg;
    /* 初始化6U的主频为528MHz */
    if (((CCM->CCSR >> 2) & 0x1) == 0)                      //当前时钟使用pll1_main_clk
    {
        /* 更改频率前首先切换系统时钟到临时时钟step_clk */
        CCM->CCSR &= ~(1 << 8);                             /* 设置step_clk = osc_clk = 24MHz */
        CCM->CCSR |= (1 << 2);                              /* 设置pll1_cw_clk = step_clk */
    }
    CCM_ANALOG->PLL_ARM = ((88 << 0) & 0x7f) | (1 << 13);   /* bit0-bit6设置为88，主频为1056，bit13置1为使能pll_arm */
    CCM->CACRR = 1;                                         /* 设置div = 2 即系统时钟为528M*/    
    CCM->CCSR &= ~(1 << 2);                                 /* 设置pll1_cw_clk = pll1_main_clk */

    /* 设置PLL2的四路PFD */
    reg = CCM_ANALOG->PFD_528;
    reg &= ~(0x3f3f3f);
    reg |= (32 << 24);                      /* PLL2_PFD3 = 297MHZ */
    reg |= (24 << 16);                      /* PLL2_PFD2 = 396MHZ */
    reg |= (16 << 8);                       /* PLL2_PFD1 = 594MHZ */
    reg |= (27 << 0);                       /* PLL2_PFD0 = 352MHZ */
    CCM_ANALOG->PFD_528=reg;

    /* 设置PLL3的四路PFD */
    reg = 0;
    reg = CCM_ANALOG->PFD_480;
    reg &= ~(0x3f3f3f);
    reg |= (19 << 24);                      /* PLL3_PFD3 = 454.7MHZ */
    reg |= (17 << 16);                      /* PLL3_PFD2 = 508.2MHZ */
    reg |= (16 << 8);                       /* PLL3_PFD1 = 504MHZ */
    reg |= (12 << 0);                       /* PLL3_PFD0 = 720MHZ */
    CCM_ANALOG->PFD_480=reg;

    /* 设置其他外设时钟 */
    /* 设置AHB_CLK_ROOT = 132MHz */
    CCM->CBCMR &= ~(3 << 18);    
    CCM->CBCMR |=  (1 << 18);               /* 设置PRE_PERIPH_CLK = 396M */

    while(CCM->CDHIPR & (1 << 5));          /* 等待握手信号 AHB_PODF_BUSY */
    CCM->CBCDR &= ~(1 << 25);               /*  */
    
    while(CCM->CDHIPR & (1 << 1));          /* 等待握手信号 PERIPH_CLK_SEL_BUSY */
    CCM->CBCDR &= ~(7 << 10);
    while(CCM->CDHIPR & (1 << 1));          /* 等待握手信号 PERIPH_CLK_SEL_BUSY */
    CCM->CBCDR |=  (2 << 10);               /* 3分频，396/3 = 132M */


    /* 设置 IPG_CLK_ROOT 最小 3Mhz，最大 66Mhz */
    CCM->CBCDR &= ~(3 << 8);                /* CBCDR 的 IPG_PODF 清零 */
    CCM->CBCDR |= 1 << 8;                   /* IPG_PODF 2 分频， IPG_CLK_ROOT=66MHz */

    /* 设置 PERCLK_CLK_ROOT 时钟 */
    CCM->CSCMR1 &= ~(1 << 6);               /* PERCLK_CLK_ROOT 时钟源为 IPG */
    CCM->CSCMR1 &= ~(0x3f << 0);               /* PERCLK_PODF 位清零，即 1 分频 */   

    /* SPI实验设置时钟源 */
    CCM->CSCDR2 &= ~(1 << 18);          /* ECSPI时钟源为PLL3_60M */
    CCM->CSCDR2 &= ~(0x3f << 19);       /* 1分频 */
}

