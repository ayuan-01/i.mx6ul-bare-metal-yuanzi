.global _start @全局标号


_start:
    @使能所有外设时钟
    ldr r0, = 0x020C4068    @CCGR0
    ldr r1, = 0xFFFFFFFF    @要向CCGR0写入的数据
    str r1, [r0]            @将0xffffffff写入CCGR0中

    ldr r0, = 0x020C406C    @CCGR1
    str r1, [r0]

    ldr r0, = 0x020C4070    @CCGR2
    str r1, [r0]

    ldr r0, = 0x020C4074    @CCGR3
    str r1, [r0]

    ldr r0, = 0x020C4078    @CCGR4
    str r1, [r0]

    ldr r0, = 0x020C407C    @CCGR5
    str r1, [r0]

    ldr r0, = 0x020C4080    @CCGR6
    str r1, [r0]
    
    /* 配置GPIO1_IO03 PIN的复用为GPIO，也就是设置
     * IOMUXC_SW_MUX_CTL_PAD_GPIO1_IO03 = 5
     * IOMUXC_SW_MUX_CTL_PAD_GPIO1_IO03寄存器地址为0x020e0068
     */
    ldr r0, = 0x020E0068    @CIOMUXC_SW_MUX_CTL_PAD_GPIO1_IO03地址
    ldr r1, = 0x5           @要向CIOMUXC_SW_MUX_CTL_PAD_GPIO1_IO03写入的数据
    str r1, [r0]            @将0x5写入CIOMUXC_SW_MUX_CTL_PAD_GPIO1_IO03中

    /* 配置GPIO1_IO03的电气属性
     * CIOMUXC_SW_PAD_CTL_PAD_GPIO1_IO03
     * 0x020e02f4
     * bit0:    0   低速率
     * bit3:5:  110 R0/6驱动能力
     * bit7:6:  10  100MHz速度
     * bit11:   0   关闭开漏输出
     * bit12:   1   使能pull/kepper
     * bit13:   0   keeper
     * bit15:14:    00  100K下拉
     * bit16:   0   关闭hys
     */
    ldr r0, = 0x020E02F4
    ldr r1, = 0x10B0
    str r1, [r0]

    /* 设置GPIO功能
     * 设置GPIO_GDIR寄存器，设置GPIO1_GPIO03为输出
     * GPIO_GDIR地址为0x0209c004
     */
    ldr r0, = 0x0209C004
    ldr r1, = 0x00000008
    str r1, [r0]

    /* 打开LED也就是设置GPIO1_IO03为0
     * GPIO_DR地址0x0209c000
     */
    ldr r0, = 0x0209C000
    ldr r1, = 0
    str r1, [r0]

loop:
    b loop

