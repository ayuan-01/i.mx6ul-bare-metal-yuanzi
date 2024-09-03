#include "bsp_ft5426.h"
#include "bsp_gpio.h"
#include "bsp_int.h"
#include "bsp_delay.h"
#include "bsp_i2c.h"
#include "stdio.h"

struct ft5426_dev_struc ft5426_dev;

/* 初始化FT5426 */
void ft5426_init(void)
{
    unsigned char reg_value[2];
    // 1.IO初始化
    IOMUXC_SetPinMux(IOMUXC_UART5_TX_DATA_I2C2_SCL, 1);
    IOMUXC_SetPinMux(IOMUXC_UART5_RX_DATA_I2C2_SDA, 1);

    IOMUXC_SetPinConfig(IOMUXC_UART5_TX_DATA_I2C2_SCL, 0x70b0);
    IOMUXC_SetPinConfig(IOMUXC_UART5_RX_DATA_I2C2_SDA, 0x70b0);
    
    // 初始化INT引脚使能中断
    IOMUXC_SetPinMux(IOMUXC_GPIO1_IO09_GPIO1_IO09, 0);
    IOMUXC_SetPinConfig(IOMUXC_GPIO1_IO09_GPIO1_IO09,0xf080);

    gpio_pin_config_t ctint_config;
    ctint_config.direction = kGPIO_DigitalInput;
    ctint_config.interruptMode = kGPIO_IntRisingOrFallingEdge;
    ctint_config.outputLogic = 1;
    gpio_init(GPIO1, 9, &ctint_config);      //设置引脚为输入

    GIC_EnableIRQ(GPIO1_Combined_0_15_IRQn);
    system_register_irqhandler(GPIO1_Combined_0_15_IRQn, gpio1_io09_irqhandler, NULL);
    /* 使能中断之前需要先注册中断服务函数，否则使能后来中断可能会出问题 */
    gpio_enableint(GPIO1, 9);

    // 初始化复位引脚
    IOMUXC_SetPinMux(IOMUXC_SNVS_SNVS_TAMPER9_GPIO5_IO09, 0);
    IOMUXC_SetPinConfig(IOMUXC_SNVS_SNVS_TAMPER9_GPIO5_IO09,0x10b0);

    gpio_pin_config_t ctrst_config;
    ctrst_config.direction = kGPIO_DigitalOutput;
    ctrst_config.outputLogic = 1;
    gpio_init(GPIO5, 9, &ctrst_config);
    gpio_pinwrite(GPIO5, 9, 0);     // 复位FT5426芯片
    delay_ms(50);
    gpio_pinwrite(GPIO5, 9, 1);     // 停止复位
    delay_ms(50);

    // 2.IIC接口的初始化
    i2c_init(I2C2);
    // 3.FT5426初始化
    ft5426_read_len(FT5426_ADDR, FT5426_IDGLIB_VERSION, 2, reg_value);      // 读取固件版本号
    printf("Touch Frimware Version:%#x\r\n", (unsigned short)(reg_value[0] << 8 | reg_value[1]));

    ft5426_write_byte(FT5426_ADDR, FT5426_DEVICE_MODE, 0); 	/* 进入正常模式 				*/
	ft5426_write_byte(FT5426_ADDR, FT5426_IDG_MODE, 1); 	/* FT5426中断模式 			*/
    ft5426_dev.initfalg = FT5426_INIT_FINISHED;	            /* 标记FT5426初始化完成 */
}

/* 中断服务函数 */
void gpio1_io09_irqhandler(unsigned int gicciar, void *param)
{

    if(ft5426_dev.initfalg == FT5426_INIT_FINISHED)
	{
		//ft5426_dev.intflag = 1;
		ft5426_read_tpcoord();
	}
    gpio_clearintflags(GPIO1, 9);
}


/* 向FT5426写一个字节的数据 */
unsigned char ft5426_write_byte(unsigned char addr, unsigned char reg, unsigned char data)
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
    if (i2c_master_transfer(I2C2, &masterXfer))
        status = 1;
    
    return status;
}

/* 从FT5426读取一个字节的数据 */
unsigned char ft5426_read_byte(unsigned char addr,unsigned char reg)
{
	unsigned char val=0;
	
	struct i2c_transfer masterXfer;	
	masterXfer.slaveAddress = addr;				/* 设备地址 				*/
    masterXfer.direction = kI2C_Read;			/* 读取数据 				*/
    masterXfer.subaddress = reg;				/* 要读取的寄存器地址 			*/
    masterXfer.subaddressSize = 1;				/* 地址长度一个字节 			*/
    masterXfer.data = &val;						/* 接收数据缓冲区 				*/
    masterXfer.dataSize = 1;					/* 读取数据长度1个字节			*/

	i2c_master_transfer(I2C2, &masterXfer);
    
	return val;
}

/* 从ft5426读取多个寄存器数据 */
void ft5426_read_len(unsigned char addr,unsigned char reg, unsigned char len, unsigned char *buf)
{	
	struct i2c_transfer masterXfer;	
	masterXfer.slaveAddress = addr;				/* 设备地址 				*/
    masterXfer.direction = kI2C_Read;			/* 读取数据 				*/
    masterXfer.subaddress = reg;				/* 要读取的寄存器地址 			*/
    masterXfer.subaddressSize = 1;				/* 地址长度一个字节 			*/
    masterXfer.data = buf;						/* 接收数据缓冲区 				*/
    masterXfer.dataSize = len;					/* 读取数据长度1个字节			*/

	i2c_master_transfer(I2C2, &masterXfer);
}


/*
 * @description	: 读取当前所有触摸点的坐标
 * @param 		: 无
 * @return 		: 无
 */
void ft5426_read_tpcoord(void)
{
	unsigned char i = 0;
	unsigned char type = 0;
	//unsigned char id = 0;
	unsigned char pointbuf[FT5426_XYCOORDREG_NUM];
	
	ft5426_dev.point_num = ft5426_read_byte(FT5426_ADDR, FT5426_TD_STATUS);

	/*
  	 * 从寄存器FT5426_TOUCH1_XH开始，连续读取30个寄存器的值，这30个寄存器
  	 * 保存着5个点的触摸值，每个点占用6个寄存器。
	 */
	ft5426_read_len(FT5426_ADDR, FT5426_TOUCH1_XH, FT5426_XYCOORDREG_NUM, pointbuf);
		
	for(i = 0; i < ft5426_dev.point_num ; i++)
	{
		unsigned char *buf = &pointbuf[i * 6];
		/* 以第一个触摸点为例，寄存器TOUCH1_XH(地址0X03),各位描述如下：
		 * bit7:6  Event flag  0:按下 1:释放 2：接触 3：没有事件
		 * bit5:4  保留
		 * bit3:0  X轴触摸点的11~8位。
		 */
		ft5426_dev.x[i] = ((buf[2] << 8) | buf[3]) & 0x0fff;
		ft5426_dev.y[i] = ((buf[0] << 8) | buf[1]) & 0x0fff;
		
		type = buf[0] >> 6;	/* 获取触摸类型 */
		

		/* 以第一个触摸点为例，寄存器TOUCH1_YH(地址0X05),各位描述如下：
		 * bit7:4  Touch ID  触摸ID，表示是哪个触摸点
		 * bit3:0  Y轴触摸点的11~8位。
		 */
		//id = (buf[2] >> 4) & 0x0f;
		
		if(type == FT5426_TOUCH_EVENT_DOWN || type == FT5426_TOUCH_EVENT_ON )/* 按下 	*/
		{
		
		} else  {	/* 释放 */	
			
		}
	}	
}

