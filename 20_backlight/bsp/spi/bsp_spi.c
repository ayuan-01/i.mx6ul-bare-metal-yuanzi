#include "bsp_spi.h"

/**
 * @brief SPI初始化
 * 
 * @param base : ECSPIn
 */
void spi_init(ECSPI_Type *base)
{
    base->CONREG = 0;
    /* bit0置1，使能SPI。
     * bit3置1，表示当向TXFIFO写入数据后马上开启SPI突发访问，也就是发送数据。
     * bit7:4设置SPI通道主从模式，bit7为通道3，bit4为通道0。
     * 我们使用了SS0，也就是通道0.实际上本实验片选由软件实现
     * bit31:20设置突发访问长度，设置为7，即长度为8bit，一个字节。
     */
    base->CONREG |= (1 << 0) | (1 << 3) | (1 << 4) | (7 << 20);

    /**
     * bit14:0设置wait states时间，设置为0x2000;
     * bit15设置时钟源为SPI clk将此位设置为0.
     * bit21:16表示片选信号得延时，设置为0.我们采用软件片选
     */
    base->PERIODREG = 0;
    base->PERIODREG = 2000;

    /* SPI时钟ICM20608最高只能到8M，将SPI CLK = 6M */
    base->CONREG &= ~((0xf << 12) | (0xf << 8));
    base->CONREG |= (9 << 12);          /* 一级10分频 */    
}

/**
 * @brief SPI发送/接收函数
 * 
 * @param base : ECSPIn
 * @param txdata : 要发送的数据，当接收数据时，该参数给0xff
 * @return unsigned char 
 */
unsigned char spich0_readwrite_byte(ECSPI_Type *base, unsigned char txdata)
{
    uint32_t spirxdata = 0;
    uint32_t spitxdata = txdata;

    /* 选择通道0 */
    base->CONREG &= ~(3 << 18);
    base->CONREG |= (0 << 18);

    /* 数据发送，发送数据前一定要等待STATREG的bit0为空 */
    while((base->STATREG & (1 << 0)) == 0);
    base->TXDATA = spitxdata;

    /* 数据接收 */
    while((base->STATREG & (1 << 3)) == 0);
    spirxdata = base->RXDATA;

    return spirxdata;
}