.global _start

_start:
    /* 设置处理器进入SVC模式
     *将CPSR的bit4-bit0设置为0X13
     */
    mrs r0, cpsr
    bic r0, r0, #0x1f
    orr r0, r0, #0x13
    msr cpsr, r0

    /* 设置SP指针 */
    ldr sp,= 0x80200000
    b main @跳转到main函数
