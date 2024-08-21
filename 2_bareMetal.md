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

#### 内联函数

在C语言中，`inline`关键字用于建议编译器将函数的代码直接插入到调用点，而不是通过常规的函数调用机制来调用。这样可以减少函数调用的开销，比如避免了调用和返回时的栈操作，以及可能的跳转指令，从而提高程序的执行效率。

`inline` 详细解释

- **减少函数调用开销**：通常，函数调用涉及将参数压栈、跳转到函数代码、执行代码、返回并清理栈。`inline`函数通过将函数代码直接插入到调用点，避免了这些开销。
- **编译器优化建议**：`inline` 只是一个建议，编译器可以选择忽略它。如果函数体过大，编译器可能会忽略 `inline` 提示，因为内联较大的函数可能会增加程序的代码量，导致性能下降。
- **注意事项**：
  - 递归函数不能被内联，因为编译器无法确定递归的结束条件。
  - 过多使用 `inline` 可能导致代码膨胀，反而降低性能。

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

### GPIO中断程序

- 汇编文件

  - 在汇编启动程序中配置好中断向量表和中断服务函数IRQ_Hanlder。

- BSP_INT(中断配置文件)

  - 创建中断函数指针类型。
  - 创建中断函数结构体，包括中断服务函数和中断服务函数参数。
  - 创建变量记录中断嵌套数量。
  - 创建中断处理函数表，这是一个数组。
  - 创建初始化中断函数表函数，中断数量清零。
  - 中断初始化函数(初始化GIC，初始化中断函数表，设置中断向量偏移)。
  - 中断处理注册函数。
  - 系统中断处理函数(IRQ跳转)，检查中断处理函数ID号，中断数加一，运行中断irqTable[intNum]。

- BSP_GPIO中断文件配置

  - 定义枚举数据中断出发类型gpio_interrupt_mode_t。
  - GPIO配置结构体中加上中断类型。
  - GPIO中断配置函数，输入参数GPIO*，pin，gpio_interrupt_mode_t。
    - 定义icr指针。并`icrShift = pin;`
    - SEL置1时设置ICR寄存器无效，所以给此寄存器清零。
    - 低16位`icr = &(base->ICR1);`高16位`icrShift -= 16;`
    - 判断中断出发类型，设置ICR寄存器。
  - GPIO初始化函数中加入GPIO中断配置函数。
  - 定义函数：使能GPIO中断，禁止GPIO中断，清除中断标志位(该函数作用位退出中断)

- BSP_EXIT外部中断配置文件

  - 外部中断初始化函数

    - 定义gpio配置结构体

    - 引脚复用，电气属性上拉输入。

    - GPIO初始化。

    - GIC使能中断。

    - 注册中断服务函数。

    - gpio中断使能。

      ```c
      GIC_EnableIRQ(GPIO1_Combined_16_31_IRQn);
      system_register_irqhandler(GPIO1_Combined_16_31_IRQn, gpio1_io18_irqhandler, NULL);
      /* 使能中断之前需要先注册中断服务函数，否则使能后来中断可能会出问题 */
      gpio_enableint(GPIO1, 18);
      ```

  - 中断服务函数。

### EPIT中断

- 中断初始化函数

  - 输入参数为分频值和载入值。

    ```c
    void epit1_int(unsigned int frac, unsigned int value)
    ```

  - 判断分频数是否大于4095。

  - 配置EPIT中断寄存器。

    ```c
    /* 配置EPIT的CR寄存器 */
    EPIT1->CR = 0;
    /**
      * bit1=1，计数器从load值或者0xffffffff开始计数
      * bit2=1使能比较中断
      * bit3=1从寄存器记录的值重新计数
      * bit4-15分配值frac
      * bit24=1设置定时器时钟源
      */
    EPIT1->CR = (1 << 1) | (1 << 2) | (1 << 3) | (frac << 4) | (1 << 24);
    EPIT1->LR = value;
    EPIT1->CMPR =0;
    ```

  - GIC使能

  - 注册中断服务函数

  - 开启中断

    ```c
    /* 初始化中断 */
    GIC_EnableIRQ(EPIT1_IRQn);
    system_register_irqhandler(EPIT1_IRQn, epit1_irqhandler, NULL);   
    /* 打开EPIT1 */ 
    EPIT1->CR |= (1 << 0);
    ```

- EPIT1中断服务函数

  ```c
  void epit1_irqhandler(unsigned int gicciar, void *param)
  {
      static unsigned char state = 0;
      state = !state;
      if (EPIT1->SR & (1 << 0))   /* 中断发生了 */
      {
          led_switch(LED0, state);
      }
      /* 清除中断标志位 */
      EPIT1->SR |= (1 << 0);
  }
  ```

### 定时器按键消抖

- 外部中断初始化

  ```c
  void keyfilter_init(void)
  {
      gpio_pin_config_t key_config;
      IOMUXC_SetPinMux(IOMUXC_UART1_CTS_B_GPIO1_IO18, 0);                 //设置引脚复用，GPIO1_18
      IOMUXC_SetPinConfig(IOMUXC_UART1_CTS_B_GPIO1_IO18, 0xf080);         //设置引脚电气属性 设置为上拉输入
  
      /* GPIO初始化 */
      key_config.direction = kGPIO_DigitalInput;
      key_config.interruptMode = kGPIO_IntFallingEdge;
      key_config.outputLogic = 1;
      gpio_init(GPIO1, 18, &key_config);      //设置引脚为输入
  
      GIC_EnableIRQ(GPIO1_Combined_16_31_IRQn);
      system_register_irqhandler(GPIO1_Combined_16_31_IRQn, gpio1_16_32_irqhandler, NULL);
      /* 使能中断之前需要先注册中断服务函数，否则使能后来中断可能会出问题 */
      gpio_enableint(GPIO1, 18);
      filtertimer_init(66000000/100); //定时器初始化放在外部中断初始化函数中
  }
  ```

  

- 定时器中断初始化，初始化定时器,但不使能，要在按键下降沿中断触发之后在按键的中断服务函数中开启，开始计数，计数满足定义的时间触发定时器中断。

  ```c
  void filtertimer_init(unsigned int value)
  {
      EPIT1->CR = 0;
      EPIT1->CR = (1 << 1) | (1 << 2) | (1 << 3) | (1 << 24);
      EPIT1->LR = value;
      EPIT1->CMPR =0;
      
      /* 初始化中断 */
      GIC_EnableIRQ(EPIT1_IRQn);
      system_register_irqhandler(EPIT1_IRQn, filtertimer_irqhandler, NULL);   
  }
  ```

- 外部中断服务函数

  ```c
  void gpio1_16_32_irqhandler(unsigned int gicciar, void *param)
  {
      /* 开启定时器 */
      filtertimer_restart(66000000/100);
      /* 清除中断标志位 */
      gpio_clearintflags(GPIO1, 18);
  }
  ```

- 定时器中断服务函数

  ```c
  void filtertimer_irqhandler(unsigned int gicciar, void *param)
  {
      static unsigned char state = OFF;
      
      if(EPIT1->SR & (1 << 0))
      {
          /* 关闭定时器 */
          filtertimer_stop();
          if (gpio_pinread(GPIO1, 18) == 0)
          {
              state = !state;
              beep_switch(state);
          }
      }
      /* 清除中断标志位 */
      EPIT1->SR |= (1 << 0);
  }
  ```

- 关闭EPIT1定时器

  ```C
  /* 关闭EPIT1定时器 */
  void filtertimer_stop(void)
  {
      EPIT1->CR &= ~(1 << 0);
  }
  ```

  

- 开启EPIT1定时器

  ```c
  /* 开启EPIT1定时器 */
  void filtertimer_restart(unsigned int value)
  {
      EPIT1->CR &= ~(1 << 0);
      EPIT1->LR = value;
      EPIT1->CR |= ( 1<< 0);
  }
  ```

### GPT定时器实现高精度延时

#### 实验一：进入GPT中断实现LED闪烁

- 延时初始化函数

  - 初始化相关寄存器

    CR寄存器，先全部清零，软件复位，设置初始值，时钟源，设置模式为restart模式即比较中断后重新计时。设置分频值，使定时器频率为1M。设置OCR[0]为1000000/2即500ms进入一次比较中断。

  - 打开通道1比较中断，GIC使能，注册中断服务函数，打开GPT1

  ```c
  /* 延时初始化函数 */
  void delay_init(void)
  {
      GPT1->CR = 0;
  
      /* 软复位，置1复位，等待自动清零 */
      GPT1->CR = (1 << 15);
      while ((GPT1->CR >> 15) & 0x01);
  
      /**
       * bit1: 1：初始值为0
       * bit6-8: 1：时钟源选择ipg_clk = 66MHz
       * bit9: 0：restart模式
       */
      GPT1->CR |= (1 << 1) | (1 << 6);
  
      /* 分频设置,0-11位设置分频值 */
      GPT1->PR = 65;      // 66分频
      /* 配置输出比较通道 */
      GPT1->OCR[0] = 1000000/2;    /* 设置中断周期为500ms */
  
      /* 打开通道1比较中断 */
      GPT1->IR = 1 << 0;
  
      GIC_EnableIRQ(GPT1_IRQn);
      system_register_irqhandler(GPT1_IRQn, gpt1_irqhandler, NULL);
      GPT1->CR |= (1 << 0);   /* 打开GPT1 */
  }
  
  void gpt1_irqhandler(unsigned int gicciar, void *param)
  {
      static unsigned char state  = 0;
  
      /* 判断是哪个中断 */
      if (GPT1->SR & (1 << 0))
      {
          state = !state;
          led_switch(LED0, state);
      }
  
      /* 清除中断状态 */
      GPT1->SR |= (1 << 0);
  }
  ```

  

#### 实验二：高精度延时

- 延时初始化函数

  ```c
  /* 延时初始化函数 */
  void delay_init(void)
  {
      GPT1->CR = 0;
  
      /* 软复位，置1复位，等待自动清零 */
      GPT1->CR = (1 << 15);
      while ((GPT1->CR >> 15) & 0x01);
  
      /**
       * bit1: 1：初始值为0
       * bit6-8: 1：时钟源选择ipg_clk = 66MHz
       * bit9: 0：restart模式
       */
      GPT1->CR |= (1 << 1) | (1 << 6);
  
      /* 分频设置,0-11位设置分频值 */
      GPT1->PR = 65;      // 66分频
      /* 1M的频率计1个数就是1us */
      GPT1->OCR[0] = 0xffffffff;
  
      GPT1->CR |= (1 << 0);   /* 打开GPT1 */
  }
  ```

  

- us延时函数

  ```c
  void delay_us(unsigned int usdelay)
  {
      unsigned long oldcnt, newcnt;
      unsigned long tcntvalue = 0;
  
      oldcnt = GPT1->CNT;
      while (1)
      {
          newcnt = GPT1->CNT;
          if(newcnt != oldcnt)
          {
              if(newcnt > oldcnt)
              {
                  tcntvalue += newcnt - oldcnt;
              }
              else
              {
                  tcntvalue += 0xffffffff - oldcnt + newcnt;
              }
              oldcnt = newcnt;
              if (tcntvalue >= usdelay)
              {
                  break;
              }
          } 
      }
  }
  ```

  

- ms延时函数

```c
void delay_ms(unsigned int msdelay)
{
    unsigned int i = 0;
    for (i = 0; i < msdelay; i++)
    {
        delay_us(1000);
    }    
}
```

- 主函数中使用高精度延时函数实现LED闪烁。

### UART

- 串口IO初始化函数

- 串口初始化

  - 初始化IO

  - 初始化复位UART1

  - 配置相关寄存器

    - UCR1为0
    - UCR2配置数据位，奇偶校验位，停止位
    - UCR3的bti2必须为1

  - 设置波特率

    Baud Rate = 𝑅𝑒𝑓 𝐹𝑟𝑒𝑞 / (16 ×(𝑈𝐵𝑀𝑅 + 1) / (𝑈𝐵𝐼𝑅 + 1)) 

  - 使能串口

  ```c
  /**
   * @brief 初始化UART
   * @brief 波特率固定115200
   * @param void:void
   */
  void uart_init(void)
  {
      /* 初始化串口IO */
      uart_io_init();
  
      /* 初始化UART1 */
      uart_disable(UART1);    /* 关闭UART1 */
      uart_softreset(UART1);   /* 复位UART1 */
  
      /* 配置UART1 */
      UART1->UCR1 = 0;
  
      /* 配置UART1的数据位，奇偶校验，停止位等 */
      /* bit1-2发送和接收的使能 bit5设置数据位长度8位 bit6一位停止位 bit14忽略RTS脚 */
      UART1->UCR2 = 0;
      UART1->UCR2 |= (1 << 1) | (1 << 2) | (1 << 5) | (1 << 14);
  
      /* bit2必须为1 */
      UART1->UCR3 |= (1 << 2);
  
      /* 设置波特率 */
      UART1->UFCR &= ~(7 << 7);   /* 对RFDIV进行清零 */
      UART1->UFCR = 5 << 7;       /* 1分频，80M */
      UART1->UBIR = 71;
      UART1->UBMR = 3124;
      /* 使能串口 */
      uart_enable(UART1);
  }
  
  ```

- 打开串口

  ```c
  /**
   * @brief 打开串口
   * @param UART_Type
   */
  void uart_enable(UART_Type *base)
  {
      base->UCR1 |= (1 << 0);
  }
  ```

- 关闭串口

  ```c
  /**
   * @brief 关闭串口
   * @param UART_Type
   */
  void uart_disable(UART_Type *base)
  {
      base->UCR1 &= ~(1 << 0);
  }
  ```

- 复位串口

  ```c
  /**
   * @brief 复位UART
   * @param UART_Type
   */
  void uart_softreset(UART_Type *base)
  {
      base->UCR2 &= ~(1 << 0);
      while((base->UCR2 & 0x01) == 0);
  }
  ```

- 发送字符

  ```c
  void putc(unsigned char c)
  {
      /* 在判断上次数据是否发送完成 */
      while (((UART1->USR2 >> 3) & 0x01) == 0); /* 数据是否在发送中 */
      UART1->UTXD = c;
  }
  ```

- 接收字符

  ```c
  unsigned char getc(void)
  {
      while ((UART1->USR2 & 0x01) == 0); /* 是否准备好接收 */
      return UART1->URXD;
  }
  ```

  

- 发送字符串

  ```c
  /**
   * @brief 通过串口发送一串字符
   * @param str char *
   */
  void puts(char *str)
  {
      char *p = str;
      while(*p != '\0')
      {
          putc(*p++);
      }
  }
  ```

### printf配置

- 添加官方的波特率计算设置函数

  ```c
  /**
   * @brief        	    : 波特率计算公式，可以用此函数计算出指定串口对应的UFCR，UBIR和UBMR这三个寄存器的值
   * @param base		    : 要计算的串口。
   * @param baudrate	    : 要使用的波特率。
   * @param srcclock_hz	: 串口时钟源频率，单位Hz
   * @return		: 无
   */
  void uart_setbaudrate(UART_Type *base, unsigned int baudrate, unsigned int srcclock_hz)
  ```

  - 函数中用到数学库，编译时需要指定路径

    ```makefile
    LIBPATH			:= -lgcc -L /usr/local/arm/gcc-linaro-4.9.4-2017.01-x86_64_arm-linux-gnueabihf/lib/gcc/arm-linux-gnueabihf/4.9.4
    ```

  - ```c
    /* raise函数防止编译报错 */
    void raise(int sig_nr)
    {
        
    }
    ```

- 添加stdio文件

  - 修改Makefile，编译c文件时加入指令-Wa,-mimplicit-it=thumb避免报错

    ```makefile
    $(COBJS): obj/%.o : %.c
    	$(CC) $(INCLUDE) -Wa,-mimplicit-it=thumb -fno-builtin -c -Wall -nostdlib -O2 -o $@ $<
    ```

- 主函数

  ```c
  while(1){
          printf("请输入两个整数，使用空格隔开：");
          scanf("%d %d", &a, &b);
          printf("\r\n数据%d+%d=%d\r\n", a, b, a+b);
          printf("%d的十六进制：%#x\r\n", c);
      }
  ```

  

### DDR3

DDR3（Double Data Rate 3）是第三代双倍数据率同步动态随机存取存储器（SDRAM），广泛应用于计算机、服务器和嵌入式系统中。相比其前代 DDR2，DDR3 在速度、带宽和能效方面都有显著提升。以下是 DDR3 的基本原理和一些重要参数的介绍。

#### DDR3 的工作原理

##### 1. **双倍数据率（Double Data Rate）**:

- DDR3 的核心特点是双倍数据率，即在时钟信号的上升沿和下降沿都传输数据。这样，在相同的时钟频率下，DDR3 可以实现比传统 SDRAM 更高的数据传输速率。

##### 2. **内部结构**:

- DDR3 内部采用 8n 预取架构。每个时钟周期 DDR3 可以从内存阵列中预取 8 个字节的数据，并在 4 个时钟周期内分 2 次输出这些数据。这种架构有助于在更高频率下稳定工作。

##### 3. **时钟和时序**:

- DDR3 使用差分时钟（Differential Clock，CK 和 CK#）信号来同步内存操作。时序参数（如 CL、tRCD、tRP 等）决定了 DDR3 内存模块在特定操作下的延迟和性能。

##### 4. **低电压**:

- DDR3 运行电压为 1.5V，比 DDR2 的 1.8V 更低，这有助于降低功耗，尤其是在大型服务器和数据中心环境中。

#### DDR3 的重要参数

##### 1. **数据速率**:

- DDR3 的数据传输速率从 800 MT/s（Mega Transfers per second）到 2133 MT/s 不等，常见的速率有 DDR3-800、DDR3-1066、DDR3-1333、DDR3-1600、DDR3-1866 和 DDR3-2133。

##### 2. **时钟频率**:

- DDR3 的实际时钟频率（内核时钟）从 400 MHz 到 1066 MHz，但由于双倍数据率技术，其数据传输速率是时钟频率的两倍。

##### 3. **带宽**:

- 带宽取决于内存模块的位宽和数据速率。DDR3 的带宽计算公式为：`带宽 = 数据速率 × 位宽 ÷ 8`。例如，DDR3-1600 的单通道带宽为 `1600 × 64 ÷ 8 = 12.8 GB/s`。

##### 4. **时序（Latency）**:

- 时序是指内存模块在执行不同操作时的延迟，通常以一系列参数表示，如 CL（CAS Latency）、tRCD（Row to Column Delay）、tRP（Row Precharge Time）、tRAS（Row Active Time）。这些参数直接影响内存的访问速度。
- 例如，DDR3-1600 CL9 的时序可能标记为 `9-9-9-24`，意味着 CAS Latency 为 9 个时钟周期，tRCD 为 9 个时钟周期，tRP 为 9 个时钟周期，tRAS 为 24 个时钟周期。

##### 5. **容量**:

- DDR3 内存模块的容量通常从 512 MB 到 32 GB 不等，具体取决于内存芯片的密度和模块设计（如单通道、双通道或多通道配置）。

##### 6. **电压**:

- DDR3 标准电压为 1.5V，低电压版本 DDR3L 的电压为 1.35V，超低电压版本 DDR3U 的电压为 1.25V。这些低电压版本旨在进一步降低功耗。

##### 7. **预取长度**:

- DDR3 的预取长度为 8n，意味着每次内存访问可以预取 8 个字节的数据。这是 DDR3 相比于 DDR2 提高数据带宽的重要技术手段。

##### 8. **模块类型**:

- DDR3 内存模块包括常见的 UDIMM（Unbuffered DIMM）、RDIMM（Registered DIMM）、SO-DIMM（用于笔记本电脑的小型 DIMM）等。RDIMM 带有寄存器，用于稳定信号，适合用于服务器。

#### DDR3 相比 DDR2 和 DDR4 的优势

- **相比 DDR2**:
  - 更高的带宽：DDR3 的数据速率远高于 DDR2，带宽相应增加。
  - 更低的功耗：DDR3 的工作电压比 DDR2 低，降低了能耗。
  - 更大的容量：DDR3 支持更高密度的内存芯片，使得单个模块的容量更大。
- **相比 DDR4**:
  - DDR4 进一步提升了数据速率、降低了电压，并引入了更多先进功能（如 Bank Group 技术），但 DDR3 依然在成本敏感的应用中广泛使用。

#### 总结

DDR3 是一种性能强大且应用广泛的内存技术，凭借其高带宽、低功耗和大容量，成为了许多计算机系统的主流选择。理解 DDR3 的工作原理和重要参数，有助于在系统设计和优化中做出更明智的决策。

### LCD

- 读屏幕ID

  - 打开模拟开关，设置GPIO（引脚复用冲突解决）

    ```c
     /* 打开模拟开关，设置LCD_VSYNC为高电平 */
        gpio_pin_config_t lcdio_config;
        IOMUXC_SetPinMux(IOMUXC_LCD_VSYNC_GPIO3_IO03, 0);
        IOMUXC_SetPinConfig(IOMUXC_LCD_VSYNC_GPIO3_IO03,0x10b0);
    
        /* GPIO初始化 */
        lcdio_config.direction = kGPIO_DigitalOutput;
        lcdio_config.outputLogic = 1;
        gpio_init(GPIO3, 3, &lcdio_config);
    ```

    

  - 将3个ID口复用为GPIO输入模式

    ```c
     /* 读取ID值，设置G7 B7 R7为输入 */
    	IOMUXC_SetPinMux(IOMUXC_LCD_DATA07_GPIO3_IO12, 0);		/* B7(M2) */
    	IOMUXC_SetPinMux(IOMUXC_LCD_DATA15_GPIO3_IO20, 0);		/* G7(M1) */
    	IOMUXC_SetPinMux(IOMUXC_LCD_DATA23_GPIO3_IO28, 0);		/* R7(M0) */
    
        IOMUXC_SetPinConfig(IOMUXC_LCD_DATA07_GPIO3_IO12, 0xF080);
    	IOMUXC_SetPinConfig(IOMUXC_LCD_DATA15_GPIO3_IO20, 0xF080);
    	IOMUXC_SetPinConfig(IOMUXC_LCD_DATA23_GPIO3_IO28, 0xF080);
    
        lcdio_config.direction = kGPIO_DigitalInput;
    	gpio_init(GPIO3, 12, &lcdio_config);
    	gpio_init(GPIO3, 20, &lcdio_config);
    	gpio_init(GPIO3, 28, &lcdio_config);
    ```

  - return屏幕ID
  - 使用串口初始屏幕ID

- 初始化屏幕IO

  - 配置24个数据引脚

  - 4个功能引脚（LCDIF控制器，不用作GPIO所以不用初始换GPIO）

  - 点亮背光（GPIO）

  - 复位10ms

    ```c
    /* 初始化屏幕IO */
    	lcdgpio_init();
    	lcd_reset();
    	delay_ms(10);
    	lcd_noreset();
    ```

- 根据不同的屏幕ID来设置屏幕参数

  ```c
  if(lcdid == ATK7016) {
  		tftlcd_dev.height = 600;	
  		tftlcd_dev.width = 1024;
  		tftlcd_dev.vspw = 3;
  		tftlcd_dev.vbpd = 20;
  		tftlcd_dev.vfpd = 12;
  		tftlcd_dev.hspw = 20;
  		tftlcd_dev.hbpd = 140;
  		tftlcd_dev.hfpd = 160;
  		lcdclk_init(32, 3, 5);	/* 初始化LCD时钟 51.2MHz */
  	}
  ```

- 设置公共屏幕参数

  ```c
  tftlcd_dev.pixsize = 4;	/* 每个像素4个字节 */
  	tftlcd_dev.framebuffer = LCD_FRAMEBUF_ADDR;
  	tftlcd_dev.forecolor = LCD_RED;
  	tftlcd_dev.backcolor = LCD_BLACK;
  ```

- 配置LCDIF控制器相关寄存器 

- 使能，清屏

  ```c
  lcd_enable();			/* 使能LCD */
  
  delay_ms(20);
  lcd_clear(LCD_WHITE);
  ```

- 时钟配置函数

  ```c
  void lcdclk_init(unsigned char loopDiv, unsigned char prediv, unsigned char div)
  {
  	/* 不使用小数分频器 */
  	CCM_ANALOG->PLL_VIDEO_NUM = 0;
  	CCM_ANALOG->PLL_VIDEO_DENOM = 0;
  
  	/*
       * PLL_VIDEO寄存器设置
       * bit[13]:    1   使能VIDEO PLL时钟
       * bit[20:19]  2  设置postDivider为1分频
       * bit[6:0] : 32  设置loopDivider寄存器
  	 */
  	CCM_ANALOG->PLL_VIDEO =  (2 << 19) | (1 << 13) | (loopDiv << 0); 
  
  	/*
       * MISC2寄存器设置
       * bit[31:30]: 0  VIDEO的post-div设置，时钟源来源于postDivider，1分频
  	 */
  	CCM_ANALOG->MISC2 &= ~(3 << 30);
  
  	/* LCD时钟源来源与PLL5，也就是VIDEO           PLL  */
  	CCM->CSCDR2 &= ~(7 << 15);  	
  	CCM->CSCDR2 |= (2 << 15);			/* 设置LCDIF_PRE_CLK使用PLL5 */
  
  	/* 设置LCDIF_PRE分频 */
  	CCM->CSCDR2 &= ~(7 << 12);		
  	CCM->CSCDR2 |= (prediv - 1) << 12;	/* 设置分频  */
  
  	/* 设置LCDIF分频 */
  	CCM->CBCMR &= ~(7 << 23);					
  	CCM->CBCMR |= (div - 1) << 23;		
  
  	/* 设置LCD时钟源为LCDIF_PRE时钟 */
  	CCM->CSCDR2 &= ~(7 << 9);					/* 清除原来的设置		 	*/
  	CCM->CSCDR2 |= (0 << 9);					/* LCDIF_PRE时钟源选择LCDIF_PRE时钟 */
  }
  ```

- 功能函数
  - 画点函数
  - 读点函数
  - 清屏函数
  - 矩形填充函数

### RTC

- 时间相关的宏定义，时间相关的结构体

  ```c
  /* 跟时间有关的宏定义 */
  #define SECONDS_IN_A_DAY    (86400)
  #define SECONDS_IN_A_HOUR   (3600)
  #define SECONDS_IN_A_MINUTE (60)
  #define DAYS_IN_A_YEAR      (365)
  #define YEAR_RANGE_START    (1970)
  #define YEAR_RANGE_END      (2099)
  
  /* 跟时间有关的结构体 */
  struct rtc_datetime{
  unsigned short  year;
  unsigned char   month;
  unsigned char   day;
  unsigned char   hour;
  unsigned char   minute;
  unsigned char   second;
  };
  ```

- RTC初始化函数

  - 开启信任模式

  - 使能RTC

    ```c
    void rtc_init(void)
    {
        struct rtc_datetime rtcDate;
        SNVS->HPCOMR |= (1 << 31) | (1 << 8);
    #if 0
        rtcDate.year    = 2024;
        rtcDate.month   = 8;
        rtcDate.day     = 14;
        rtcDate.hour    = 23;
        rtcDate.minute  = 32;
        rtcDate.second  = 36;
    #endif
        rtc_setdatetime(&rtcDate);
    
        /* 开启RTC */
        rtc_enable();
    }
    ```

- 功能函数
  - 使能RTC
  - 关闭RTC
  - **时间转化为秒数**
  - **设置时间**
  - 得到秒数（读寄存器）
  - 秒数转化为时间
  - **获取时间（得到秒数，转化时间，传入时间结构体地址）**
  - 判断闰年

### IIC

SCL和SDA需要接上拉电阻一般4.7K，3.3V。

I2C总线支持多从机，通过从机地址开区分访问哪个从机

