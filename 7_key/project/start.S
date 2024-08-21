.global _start
.global _bss_start
.global _bss_end

_bss_start:
    .word __bss_start
_bss_end:
    .word __bss_end




_start:
    /* 设置处理器进入SVC模式
     *将CPSR的bit4-bit0设置为0X13
     */
    mrs r0, cpsr
    bic r0, r0, #0x1f
    orr r0, r0, #0x13
    msr cpsr, r0

    /* 清除BSS段 */
    ldr r0, _bss_start
    ldr r1, _bss_end
    mov r2, #0
bss_loop:
    stmia r0!, {r2}
    cmp r0, r1
    ble bss_loop

    /* 设置SP指针 */
    ldr sp,= 0x80200000
    b main @跳转到main函数
