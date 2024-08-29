.global _start

_start:
    /* 编写中断向量表，填写复位中断服务函数和IRQ中断服务函数 */
    ldr pc,= Reset_Handler              /* 复位中断服务函数 */
    ldr pc,= Undefined_Handler          /* 未定义指令中断 */
    ldr pc,= SVC_Handler                /* SVC */
    ldr pc,= PreAbort_Handler           /* 预取终止 */
    ldr pc,= DataAbort_Handler          /* 数据终止 */
    ldr pc,= NotUsed_Handler            /* 未使用 */
    ldr pc,= IRQ_Handler                /* IRQ中断 */
    ldr pc,= FIQ_Handler                /* FIQ中断 */


/**
 * 复位中断服务函数
 * 1. 关闭I.DCache和MMU。CP15协寄存器一般用于存储系统管理，但是在中断中也会使用到，一共有16个32位寄存器
 *  CP15常用指令: mrc, mcr。使用CP15修改SCTLR寄存器，采用读-改-写的方式。SCTLR寄存器控制ID Cache和MMU的关闭
 * 2. 设置处理器9种工作模式下对应的SP指针
 * 3. 清除BSS段
 * 4. 跳到main函数
 */
Reset_Handler:

    cpsid i                             /* 关闭IRQ */

    mrc p15, 0, r0, c1, c0, 0           /* 读取STCLR寄存器中的值到r0寄存器 */
    bic r0, r0, #(1 << 12)              /* 关闭I Cache */
    bic r0, r0, #(1 << 11)              /* 关闭分支预测 */
    bic r0, r0, #(1 << 2)               /* 关闭D Cache */
    bic r0, r0, #(1 << 1)               /* 关闭对齐 */
    bic r0, r0, #(1 << 0)               /* 关闭MMU */
    mcr p15, 0, r0, c1, c0, 0           /* 将r0寄存器中的数据写入到STCLR中 */
#if 0
    /* 设置中断向量偏移 */
    ldr r0,= 0x87800000
    dsb                                 /* 数据同步隔离，前面存储器访问操作都执行完毕后，才执行它后面的指令 */
    isb                                 /* 指令同步隔离，清洗流水线，以保证所有它前面的指令都执行完毕后，才执行它后面的指令 */
    mcr p15, 0, r0, c12, c0, 0          /* 设置VBAR寄存器 = 0x87800000 */
    dsb
    isb
#endif

.global _bss_start
_bss_start:
    .word __bss_start
.global _bss_end
_bss_end:
    .word __bss_end

    /* 清除BSS段 */
    bss_loop:
    stmia r0!, {r2}
    cmp r0, r1
    ble bss_loop

    /**
	 * 设置处理器进入IRQ模式
     * 将CPSR的bit4-bit0设置为0X12
     */
    mrs r0, cpsr
    bic r0, r0, #0x1f
    orr r0, r0, #0x12
    msr cpsr, r0
    ldr sp,= 0x80600000                 /* 设置IRQ格式下的sp */

    /**
	 * 设置处理器进入SYS模式
     * 将CPSR的bit4-bit0设置为0X13
     */
    mrs r0, cpsr
    bic r0, r0, #0x1f
    orr r0, r0, #0x1f
    msr cpsr, r0
    ldr sp,= 0x80400000                 /* 设置SYS格式下的sp */

    /**
	 * 设置处理器进入SVC模式
     * 将CPSR的bit4-bit0设置为0X13
     */
    mrs r0, cpsr
    bic r0, r0, #0x1f
    orr r0, r0, #0x13
    msr cpsr, r0
    ldr sp,= 0x80200000                 /* 设置SVC格式下的sp */
    cpsie i                             /* 打开IRQ */
    b main @跳转到main函数
    

/* 未定义指令中断 */
Undefined_Handler:
    ldr r0,= Undefined_Handler
    bx r0

/* SVC */
SVC_Handler:
    ldr r0,= SVC_Handler
    bx r0

/* 预取终止 */
PreAbort_Handler:
    ldr r0,= PreAbort_Handler
    bx r0

/* 数据终止 */
DataAbort_Handler:
    ldr r0,= DataAbort_Handler
    bx r0

/* 未使用 */
NotUsed_Handler:
    ldr r0,= NotUsed_Handler
    bx r0

/* IRQ中断 */
IRQ_Handler:
    /* 保存现场 触发中断的机制首先会保存一些寄存器，一些不会自动保存的需要我们手动保存 */
    push {lr}
    push {r0-r3, r12}

    mrs r0, spsr
    push {r0}

    /** 
     * GIC_BASE->GICC->GICC_IAR
     */
    mrc p15, 4, r1, c15, c0, 0;         /* 使用CP15读取CBAR寄存器，该寄存器保存了GIC控制器的寄存器组首地址GIC_BASE */

    add r1, r1, #0x2000                 /* GIC的CPU接口端基地址 */
    ldr r0, [r1, #0xc]                  /* GICC_IAR寄存器，该寄存器保存当前发生中断的中断号，我们通过这个中断号来决定调用哪个中断服务函数 */
    push {r0, r1}

    cps #0x13                           /* 进入SVC模式 */

    push {lr}
    ldr r2,= system_irqhandler          /* 加载C语言中断处理函数到r2寄存器当中 */
    blx r2                              /* 运行C语言中断处理函数，此函数有一个参数，为GICC_IAR的值 */
    pop {lr}

    cps #0x12
    pop {r0, r1}
    str r0, [r1, #0x10]                 /* 中断执行完成，写GICC_EOIR寄存器。处理完具体中断后，需要将对应的中断ID值写入到GICC_EOIR中 
                                         * 通知GIC当前处理中断已经处理完毕，允许GIC分配下一个中断。 */
    pop {r0}
    msr spsr_cxsf, r0

    pop {r0-r3, r12}
    subs pc, lr, #4                     /* 将lr-4赋值给pc，arm的指令是三级流水线：取指，译址，执行。 */


/* FIQ中断 */
FIQ_Handler:
    ldr r0,= FIQ_Handler
    bx r0

