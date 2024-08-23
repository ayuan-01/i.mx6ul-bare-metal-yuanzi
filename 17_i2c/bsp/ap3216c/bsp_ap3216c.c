#include "bsp_ap3216c.h"
#include "bsp_i2c.h"
#include "bsp_gpio.h"
#include "bsp_delay.h"
#include "stdio.h"
#include "bsp_led.h"

/**
 * @brief 初始化AP3216C
 * 
 */
unsigned char ap3216c_init(void)
{
    unsigned char value = 0;
    /* 1.IO初始化 */
    IOMUXC_SetPinMux(IOMUXC_UART4_TX_DATA_I2C1_SCL, 1);
    IOMUXC_SetPinMux(IOMUXC_UART4_RX_DATA_I2C1_SDA, 1);

    IOMUXC_SetPinConfig(IOMUXC_UART4_TX_DATA_I2C1_SCL, 0x70b0);
    IOMUXC_SetPinConfig(IOMUXC_UART4_RX_DATA_I2C1_SDA, 0x70b0);

    /* 2.I2C接口初始化 */
    i2c_init(I2C1);

    /* 3.AP3216C传感器初始化 */
    ap3216c_writeonebyte(AP3216C_ADDR, AP3216C_SYSTEMCONG, 0x4);    /* 复位 */
    delay_ms(50);
    ap3216c_writeonebyte(AP3216C_ADDR, AP3216C_SYSTEMCONG, 0x3);    /* 复位 */
    value = ap3216c_readonebyte(AP3216C_ADDR, AP3216C_SYSTEMCONG);

    printf("ap3216c system comfig reg = %#x\r\n", value);

    if (value == 0x3) {
        return 0;
    } else {
        return 1;
    }
}

/**
 * @brief ap3216c读一个字节数据, 返回值就是读取到的寄存器的值
 * 
 * @param addr 
 * @param reg 
 * @return unsigned char 
 */
unsigned char ap3216c_readonebyte(unsigned char addr,unsigned char reg)
{
	unsigned char val=0;
	
	struct i2c_transfer masterXfer;	
	masterXfer.slaveAddress = addr;				/* 设备地址 				*/
    masterXfer.direction = kI2C_Read;			/* 读取数据 				*/
    masterXfer.subaddress = reg;				/* 要读取的寄存器地址 			*/
    masterXfer.subaddressSize = 1;				/* 地址长度一个字节 			*/
    masterXfer.data = &val;						/* 接收数据缓冲区 				*/
    masterXfer.dataSize = 1;					/* 读取数据长度1个字节			*/

	i2c_master_transfer(I2C1, &masterXfer);
    
	return val;
}

/**
 * @brief 写一个字节
 * 
 * @param addr : 设备地址
 * @param reg  : 寄存器地址
 * @param data : 要写入的数据
 * @return unsigned char 
 */
unsigned char ap3216c_writeonebyte(unsigned char addr, unsigned char reg, unsigned char data)
{
    unsigned char writedata = data;
    unsigned char status = 0;

    struct i2c_transfer masterXfer;

    masterXfer.slaveAddress = addr;				/* 设备地址 */
    masterXfer.direction = kI2C_Write;			/* 读取数据 */
    masterXfer.subaddress = reg;				/* 要读取的寄存器地址 */
    masterXfer.subaddressSize = 1;				/* 地址长度一个字节 */
    masterXfer.data = &writedata;						/* 接收数据缓冲区 */
    masterXfer.dataSize = 1;					/* 读取数据长度1个字节 */
    if (i2c_master_transfer(I2C1, &masterXfer))
        status = 1;
    
    return status;
}

/**
 * @brief ap3216c读取数据
 * 
 * @param ir 
 * @param ps 
 * @param als 
 */
void ap3216c_readdata(unsigned short *ir, unsigned short *ps, unsigned short *als)
{
    unsigned char buf[6];
    unsigned char i;

	/* 循环读取所有传感器数据 */
    for(i = 0; i < 6; i++)	
    {
        buf[i] = ap3216c_readonebyte(AP3216C_ADDR, AP3216C_IRDATALOW + i);	
    }
	
    if(buf[0] & 0X80) 	/* IR_OF位为1,则数据无效 */
		*ir = 0;					
	else 				/* 读取IR传感器的数据   		*/
		*ir = ((unsigned short)buf[1] << 2) | (buf[0] & 0X03); 			
	
	*als = ((unsigned short)buf[3] << 8) | buf[2];	/* 读取ALS传感器的数据 			 */  
	
    if(buf[4] & 0x40)	/* IR_OF位为1,则数据无效 			*/
		*ps = 0;    													
	else 				/* 读取PS传感器的数据    */
		*ps = ((unsigned short)(buf[5] & 0X3F) << 4) | (buf[4] & 0X0F); 	
}