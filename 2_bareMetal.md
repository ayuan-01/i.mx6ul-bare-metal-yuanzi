### C语言知识点，边学边记

#### 函数指针

- 函数指针是指向函数的指针。它允许动态地调用函数，即在运行中决定调用哪个函数。

  ```C
  typedef void (*system_irq_handler_t)(unsigned int gicciar, void *param);
  ```

  这条语句将一个函数指针类型定义为 `system_irq_handler_t`，它指向一个返回类型为 `void` 的函数。

  **只要满足输入参数类型，返回参数类型与函数指针定义相匹配，都可以被函数指针指向。**

  示例：

  ```c
  /* 定义函数 */
  #include <stdio.h>
  
  // 定义符合 system_irq_handler_t 类型的函数
  void my_irq_handler(unsigned int gicciar, void *param) {
      printf("IRQ Handler called with gicciar: %u, param: %p\n", gicciar, param);
  }
  
  ```

  ```C
  // 定义函数指针类型
  typedef void (*system_irq_handler_t)(unsigned int gicciar, void *param);
  ```

  ```C
  // 声明和使用函数指针
  int main() {
      // 声明一个 system_irq_handler_t 类型的函数指针
      system_irq_handler_t handler;
  
      // 将函数指针指向 my_irq_handler 函数
      handler = my_irq_handler;
  
      // 调用函数指针
      handler(123, (void *)0xDEADBEEF);
  
      return 0;
  }
  ```

  

### 启动流程

#### 汇编启动程序

- 设置中断向量表

- 定义中断服务程序
  - 复位中断服务程序
    - 关闭全局中断(cpsid i)。
    - 使用CP15协处理器管理STCLR寄存器，关闭ICache，DCache，MMU。
    - **清除BSS段。**链接脚本(.lds)中设置BSS段的起始和终止地址，使用循环清除这个地址范围内的所有内容。
    - 分别设置处理器的9种工作模式并定义对应的SP指针。查看各种工作模式下16个寄存器的共用模式。最后设置CPU工作模式为**SVC模式**
    - 打开IRQ中断(cpsie i)。
    - 跳转到main函数。
    
  - 编写IRQ中断服务函数
  
    - 1. 保护现场。触发中断的机制首先会保存(push)一些寄存器，一些不会自动保存的寄存器(r0-r3, r12)需要我们手动保存，`push {lr}` 用于保存函数调用返回地址。
  
         ```csv
         mrs r0, spsr	// 保存spsr
         push {r0}
         ```

      2. 读取GICC_IAR寄存器，这个寄存器保存着当前发生中断的中断号，通过这个中断号来决定来调用哪个中断服务函数。
  
      3. 进入svc模式，push {lr}，跳转执行system_irqhandler。
  
      4. 执行完后pop {lr}。回到irq模式。
  
      5. 中断执行完成，写GICC_EOIR寄存器。处理完具体中断后，需要将对应的中断ID值写入到GICC_EOIR中 通知GIC当前处理中断已经处理完毕，允许GIC分配下一个中断。
  
      6. 恢复现场。
  
         ```csv
         pop {r0-r3, r12}
         subs pc, lr, #4
         /* pc = lr-4是因为arm的指令是三级流水线结构，假如在执行0x2000指令时触发中断，pc此时是  * 指向0x2008的，如果中断结束后返回到0x2008开始执行那么将会漏掉一个指令，这个后果会很严重 */
         ```
  
  
  - C语言中编写中断相关函数。
  
    - 中断初始化函数。
  
    - 定义中断处理函数(函数指针)，有两个参数，包括iar，param。创建包括函数指针和参数的中断处理结构体。
  
      ```C
      typedef struct _sys_irq_handler
      {
          system_irq_handler_t irqHandler;            /* 中断处理函数 */
          void *userParam;                            /* 中断处理函数的参数 */
      }sys_irq_handle_t;
      ```
  
    - 定义中断处理函数表(存放中断处理函数结构体类型的数组)。
  
      ```c
      static sys_irq_handle_t irqTable[NUMBER_OF_INT_VECTORS];            /* 160个 */
      ```
  
      相当于给了160个中断处理函数。对应着相应的中断号。
  
    - 初始化中断处理函数表。
  
      ```c
      void system_irq_init(void)
      {
          unsigned int i = 0;
          irqNesting = 0;
          for (i = 0; i < NUMBER_OF_INT_VECTORS; i++)
          {
              irqTable[i].irqHandler = default_irqHandler;
              irqTable[i].userParam  = NULL;
          } 
      }
      ```
  
    - 注册中断处理函数
  
      ```c
      void system_register_irqhandler(IRQn_Type irq, system_irq_handler_t handler, void *userparam)
      {
          irqTable[irq].irqHandler = handler;
          irqTable[irq].userParam  = userparam;
      }
      ```
  
    - 具体的中断处理函数
  
      ```c
      void system_irqhandler(unsigned int gicciar)
      {
          /* 检查中断ID */
          uint32_t intNum = gicciar & 0x3ff;
          if(intNum >= NUMBER_OF_INT_VECTORS)
          {
              return;
          }
          irqNesting++;
          /* 根据中断ID号，读取中断处理函数，然后执行 */
          irqTable[intNum].irqHandler(intNum, irqTable[intNum].userParam);
          irqNesting--;
      }
      ```
  
      

#### 链接脚本

##### 链接脚本具体解释

这个代码段是一个GNU LD（Linker Script）脚本，它定义了如何将程序的不同部分布局到内存中。具体来说，这个脚本定义了各个段（sections）在内存中的位置和对齐方式。

1. **起始地址设置**:

   ```ld
   . = 0X87800000;
   ```

   这行代码将链接器的当前位置（即内存地址）设置为`0x87800000`。这意味着接下来的所有段都会从这个地址开始放置。

2. **.text段**:

   ```ld
   .text : 
   {
       obj/start.o
       *(.text)
   }
   ```

   `.text`段通常包含程序的代码部分。这里指定了`obj/start.o`文件中的内容首先放入`.text`段，然后将所有输入文件中的`.text`段内容依次放入。

3. **.rodata段**:

   ```ld
   .rodata ALIGN(4) : {*(.rodata*)}
   ```

   `.rodata`段用于存储只读数据。这里将所有输入文件中的`.rodata`段内容放入，并且将其对齐到4字节边界。

4. **.data段**:

   ```ld
   .data ALIGN(4) : {*(.data)}
   ```

   `.data`段用于存储已初始化的全局变量和静态变量。这里将所有输入文件中的`.data`段内容放入，并且将其对齐到4字节边界。

5. **对齐指针**:

   ```ld
   . = ALIGN(4);
   ```

   这行代码将当前位置对齐到4字节边界。

6. **__bss_start符号**:

   ```ld
   __bss_start = .;
   ```

   定义一个符号`__bss_start`，其值为当前位置（即`bss`段的起始地址）。

7. **.bss段**:

   ```ld
   .bss ALIGN(4) : {*(.bss) *(common)}
   ```

   `.bss`段用于存储未初始化的全局变量和静态变量。这里将所有输入文件中的`.bss`段和`common`段内容放入，并且将其对齐到4字节边界。

8. **__bss_end符号**:

   ```ld
   __bss_end = .;
   ```

   定义一个符号`__bss_end`，其值为当前位置（即`bss`段的结束地址）。

#### CP15协处理器寄存器

CP15寄存器是ARM架构中的协处理器15寄存器，主要用于配置和管理系统控制、内存管理、调试等功能。CP15寄存器在ARM处理器中扮演着关键角色，以下是一些主要功能和作用：

1. **系统控制**：
   - 控制寄存器（如SCTLR）：用于配置处理器的运行模式和特性，例如启用或禁用缓存、MMU、异常处理等。
2. **内存管理**：
   - TLB（Translation Lookaside Buffer）控制：管理TLB的刷新、无效化等操作。
   - 页表基地址寄存器：存储页表的基地址，用于地址转换。
3. **中断和异常处理**：
   - 中断控制寄存器：配置中断的优先级、使能状态等。
   - *异常向量表基地址寄存器：指定异常向量表的地址*。
4. **性能监控**：
   - 性能监控寄存器：用于性能计数、事件监控，帮助开发人员优化代码性能。
5. **调试和跟踪**：
   - 调试控制寄存器：配置调试功能、断点、观察点等。
   - 跟踪控制寄存器：配置跟踪功能，用于程序执行流的记录和分析。
6. **电源管理**：
   - 电源控制寄存器：配置低功耗模式、节能特性等。

CP15寄存器在ARM处理器的运行和管理中发挥着重要作用，使系统能够灵活、高效地进行配置和控制。



#### SCTLR寄存器

SCTLR（System Control Register）是ARM架构中的一个重要寄存器，位于CP15协处理器中。它用于控制处理器的各种系统级特性和操作模式。SCTLR的作用非常广泛，下面是其主要功能和控制位：

1. **MMU启用**：
   - M位：控制MMU（内存管理单元）的启用和禁用。
     - 0：禁用MMU
     - 1：启用MMU
2. **对齐检查**：
   - A位：控制未对齐数据访问的检查。
     - 0：禁用对齐检查
     - 1：启用对齐检查
3. **数据缓存启用**：
   - C位：控制数据缓存的启用和禁用。
     - 0：禁用数据缓存
     - 1：启用数据缓存
4. **指令缓存启用**：
   - I位：控制指令缓存的启用和禁用。
     - 0：禁用指令缓存
     - 1：启用指令缓存
5. **系统保护**：
   - S位：配置系统保护，使系统区和用户区的访问权限不同。
     - 0：禁用系统保护
     - 1：启用系统保护
6. **写缓冲区**：
   - B位：控制写缓冲区的启用和禁用。
     - 0：禁用写缓冲区
     - 1：启用写缓冲区
7. **小端模式/大端模式**：
   - E位：配置处理器的字节序模式。
     - 0：小端模式
     - 1：大端模式
8. **对称多处理支持**：
   - SMP位：启用或禁用对称多处理（SMP）支持。
     - 0：禁用SMP
     - 1：启用SMP
9. **指令预取**：
   - Z位：控制指令预取。
     - 0：禁用预取
     - 1：启用预取
10. **分支预测**：
    - P位：控制分支预测功能。
      - 0：禁用分支预测
      - 1：启用分支预测

SCTLR寄存器中的这些控制位使得系统能够根据需求配置处理器的运行模式和特性，从而优化性能和安全性。在实际使用中，操作系统或固件会设置这些位，以确保处理器以预期的方式运行。



#### 中断偏移地址和程序入口地址

1. 中断向量表和程序起始地址通常不会设置相同的值，这会导致两者之间的冲突。然而，在某些特定情况下特别是在嵌入式系统中，可以通过特殊的配置和使用不同方法来实现这种设置。
2. 中断向量表在_start:中设置。







