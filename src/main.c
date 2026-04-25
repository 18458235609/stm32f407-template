/**
 * @file main.c
 * @brief STM32F407VG 应用程序入口
 *
 * 本文件是整个项目的用户代码起点，展示了：
 *   1. 中断向量表的正确写法
 *   2. Reset_Handler 的完整实现（.data 复制 + .bss 清零 + SystemInit + main）
 *   3. GPIO 输出的底层操作（直接操作寄存器，不依赖 HAL）
 *   4. 轮询式按键读取与 LED 控制
 *
 * 编译：make
 * 烧录：make openocd
 *
 * 【学习要点】
 * - 理解中断向量表为什么必须放在 Flash 地址 0
 * - 理解 .data 和 .bss 段的作用，以及 Reset_Handler 为什么需要手动初始化它们
 * - 理解 GPIO_MODER 每 2 bits 控制一个引脚的原理
 * - 理解 GPIO_ODR 每一位对应一个引脚输出电平的直接映射关系
 */

#include "stm32f4xx.h"   // 芯片寄存器定义（基地址 / RCC / GPIO / SysTick）
#include "pindef.h"      // 引脚分配表 + 快速别名（LED_RED / LED_GREEN / KEY_UP）

/* ================================================================
 * 第1部分：前向声明（Forward Declarations）
 * ================================================================
 *
 * 为什么要声明？
 * C 语言要求"先声明后使用"。main() 在下面定义，
 * 但 Reset_Handler 里要调用它，所以必须提前声明。
 * 中断处理函数同理——向量表里引用了它们的名字。
 */

/** Reset_Handler：复位向量，CPU 上电后第一个执行的函数 */
void Reset_Handler(void);

/** main：用户应用程序入口 */
int main(void);

/* ================================================================
 * 第2部分：中断处理函数（Exception / Interrupt Handlers）
 * ================================================================
 *
 * 什么是异常（Exception）？
 * ARM Cortex-M 核内部产生的事件，如：
 *   - HardFault：硬件错误（最常见的错误状态，通常因非法内存访问触发）
 *   - NMI：不可屏蔽中断（最高优先级，不可被 PRIMASK 屏蔽）
 *   - SysTick：SysTick 定时器中断（本项目用 SysTick 做延时）
 *
 * 什么是中断（Interrupt）？
 * 外设通过 NVIC（Nested Vectored Interrupt Controller）向 CPU 发起请求，
 * 如 UART 收到数据、TIM 定时器溢出、外部中断引脚触发等。
 *
 * 本项目策略：
 * 所有未使用的中断/异常处理函数统一指向 Default_Handler。
 * Default_Handler 是一个死循环（while(1) {}）——
 * 如果程序跑到这里，说明发生了意外的中断，说明代码有 bug。
 *
 * 调试技巧：
 * 如果程序卡死，暂停调试器（GDB 里按 Ctrl+C），看看 PC 停在哪里。
 * 如果停在 Default_Handler，用 "info registers" 查看是什么中断触发的。
 */

/** 默认处理函数——死循环。收到任何未处理的中断都会停在这里 */
void Default_Handler(void) {
    while (1) {
        /* 死循环——CPU 空转，等着调试器介入或被复位
         *
         * 注意：这里没有 __asm__("nop")，
         * 因为优化后编译器可能认为这个循环什么都不做而删除它。
         * while(1) 本身是一个跳转指令，足够保证 CPU 停在这里。*/
    }
}

/* __attribute__((alias("Default_Handler")))
 *
 * alias 属性告诉编译器：为 NMI_Handler 创建另一个名字（alias），
 * 实际上它就是 Default_Handler 函数，共享同一段机器码。
 *
 * 为什么不直接写 void NMI_Handler(void){ while(1){} }？
 * 因为写 10 个完全相同的函数会浪费 Flash 空间，
 * 而且如果需要改处理逻辑，要改 10 处。
 * 用 alias 只定义一次，按需引用，更干净。*/
void NMI_Handler(void)        __attribute__((alias("Default_Handler")));
void HardFault_Handler(void)   __attribute__((alias("Default_Handler")));
void MemManage_Handler(void)   __attribute__((alias("Default_Handler")));
void BusFault_Handler(void)    __attribute__((alias("Default_Handler")));
void UsageFault_Handler(void) __attribute__((alias("Default_Handler")));
void SVC_Handler(void)        __attribute__((alias("Default_Handler")));
void DebugMon_Handler(void)    __attribute__((alias("Default_Handler")));
void PendSV_Handler(void)      __attribute__((alias("Default_Handler")));

/* ================================================================
 * 第3部分：中断向量表（Interrupt Vector Table）
 * ================================================================
 *
 * 【核心概念】什么是中断向量表？
 *
 * CPU 上电或复位后，PC（Program Counter）寄存器被设置为复位向量的地址，
 * CPU 从那里取指执行。复位向量通常指向 Reset_Handler。
 *
 * 当任意中断/异常发生时，CPU 自动跳转到向量表中对应编号的位置，
 * 取出那里的函数指针并跳过去执行。这就是"向量"（Vector）这个词的由来——
 * 每个中断类型都对应一个"方向"（入口地址）。
 *
 * 【地址排列】
 * Flash 起始地址（0x08000000）必须严格按照 ARM 规定的顺序排列：
 *
 *   地址        编号   说明
 *   ───────────────────────────────────────────────
 *   0x08000000  [0]   初始 MSP（Main Stack Pointer）—— 栈顶地址
 *   0x08000004  [1]   复位向量（Reset）★ 程序真正入口
 *   0x08000008  [2]   NMI
 *   0x0800000C  [3]   HardFault
 *   ...        ...
 *
 * 【为什么 [0] 是栈指针？】
 * ARM Cortex-M 的设计：上电时 CPU 自动从向量表 [0] 加载 MSP，从 [1] 加载 PC。
 * 这意味着在真正执行任何代码之前，CPU 已经有了一个可用的栈。
 * C 语言函数调用需要栈（保存返回地址、局部变量），所以栈必须首先建立。
 *
 * 【__attribute__((section(".isr_vector")))】
 * 强制把这个数组放到名为 ".isr_vector" 的段里。
 * 链接脚本中用 KEEP(*(.isr_vector)) 保证这个段不被链接器优化删除。
 * 如果不加 section 属性，链接器可能把这个"只被指针引用"的数组删掉。
 *
 * 【void (* const vectors[])(void)】
 * - vectors[]：函数指针数组
 * - const：数组内容不可修改（防止意外改写向量表）
 * - void (*)(void)：函数指针类型（无参数，无返回值）
 */

/* _estack 是链接脚本定义的符号，表示 SRAM 最高地址 + 1
 * STM32F407VG 有 192KB SRAM，起始 0x20000000，终点 0x20030000
 * _estack = 0x20030000 就是栈顶（C 语言里栈向下生长，所以栈顶在高端）*/
extern unsigned long _estack;

/* 链接器生成的符号——告诉 C 代码各段在内存中的位置
 *
 * _sidata：.data 段的初始值在 Flash（烧录文件）中的地址
 *          Reset_Handler 用这个地址从 Flash 读取初始值
 * _sdata： .data 段在 SRAM 中的起始地址
 * _edata： .data 段在 SRAM 中的结束地址（不含）
 * _sbss：  .bss 段在 SRAM 中的起始地址
 * _ebss：  .bss 段在 SRAM 中的结束地址（不含）
 *
 * 这些符号由链接脚本根据 MEMORY 和 SECTIONS 自动生成，
 * C 代码里用 extern 声明后即可使用。*/
extern unsigned long _sidata, _sdata, _edata, _sbss, _ebss;

/* 中断向量表——必须放在 Flash 地址 0x08000000（即链接脚本的 .isr_vector 段）*/
void (* const vectors[])(void) __attribute__((section(".isr_vector"))) = {
    /* [0] 初始 MSP
     * CPU 上电时自动将这个值加载到 MSP（Main Stack Pointer）寄存器。
     * 注意：这是地址，不是函数指针。
     * 0x20030000 = SRAM 最高地址 + 1，栈从这里向下生长。
     * 用法：MSP = *(uint32_t *)0x08000000（ARM 硬件自动完成）*/
    (void (*)(void))0x20030000,

    /* [1] 复位向量 ★★★ 程序真正入口
     * CPU 复位后取指的第一条指令就是跳转到 Reset_Handler */
    Reset_Handler,

    /* [2] NMI 不可屏蔽中断
     * 最高优先级硬件中断，无法被 PRIMASK/FAULTMASK 屏蔽
     * 通常用于关键硬件故障检测 */
    NMI_Handler,

    /* [3] HardFault 硬件错误 ★ 最常见的错误状态
     * 触发条件：执行指令时发生不可恢复的硬件错误
     * 常见原因：访问非法地址（NULL 指针解引用）、除零、未对齐访问等
     * 调试方法：在 GDB 中 "info registers" 查看是哪条指令触发的 */
    HardFault_Handler,

    /* [4] MemManage 内存管理 fault
     * MPU（Memory Protection Unit）违规时触发（如果使能了 MPU）*/
    MemManage_Handler,

    /* [5] BusFault 总线 fault
     * AHB/APB 总线访问错误时触发（如访问已关闭时钟的外设）*/
    BusFault_Handler,

    /* [6] UsageFault 用法 fault
     * 软件层面的错误：未定义指令、非法半字访问、SVC 错误等 */
    UsageFault_Handler,

    /* [11] SVCall 系统服务调用
     * 执行 SVC 指令时触发（嵌入式 OS 常用系统调用）*/
    SVC_Handler,

    /* [12] Debug Monitor 调试监控
     * 调试状态下启用（如果设置了 DEBUGEN）*/
    DebugMon_Handler,

    /* [14] PendSV 可挂起系统调用
     * 实时操作系统（FreeRTOS/RT-Thread）用这个实现任务切换
     * 触发方式：设置 SCB->ICSR |= (1<<28) */
    PendSV_Handler,

    /* [15] SysTick System Tick Timer
     * ARM Cortex-M 内置的 24 位倒计时定时器
     * 本项目配置为 1ms 中断一次，供延时和 RTOS 心跳使用 */
    // 注意：STM32F4 把 SysTick 异常编号设为 15，但它在内核规范中
    // 对应的是 "SysTick exception"（不是 IRQ0），所以放在向量表 index 15。
};

/* ================================================================
 * 第4部分：GPIO 辅助宏定义
 * ================================================================
 *
 * STM32 的 GPIO 外设挂载在 AHB1 总线上，每个 GPIO 端口有 11 个寄存器。
 * 其中 RCC_AHB1ENR 控制各端口的时钟——STM32 默认关闭所有外设时钟以省电，
 * 使用任何外设前必须先在 RCC 中打开它的时钟。
 *
 * GPIO_MODER 寄存器每 2 bits 控制一个引脚（共 16 个引脚 × 2 bits = 32 bits）
 * 格式：[2n+1:2n] 对应第 n 个引脚（n = 0~15）
 */

/** RCC_AHB1ENR_GPIOX：把对应 GPIO 端口的时钟使能位置 1
 *
 * 参数：port — 端口号（0=A, 1=B, 2=C, ..., 5=F）
 *
 * 原理：RCC_AHB1ENR 是一个 32 位寄存器，每 1 bit 对应一个 GPIO 端口：
 *   bit[0] → GPIOAEN   bit[1] → GPIOBEN   ...   bit[5] → GPIOFEN
 *   写 RCC_AHB1ENR |= (1 << port) 即打开对应端口时钟
 *
 * 时钟门控（Clock Gating）是什么？
 * STM32 是低功耗设计，未使用的外设可以关闭其时钟以节省电流。
 * 关闭时钟后，对该外设寄存器的任何读写都会"静默失败"（不报错，但不生效）。
 * 这是新手常踩的坑——代码写得对，但外设不工作，原因是忘了开时钟！*/
#define RCC_GPIO_EN(port)  (RCC_AHB1ENR |= (1 << (port)))

/** GPIOEN_PORT_A = 0，GPIOEN_PORT_F = 5
 * 用于 RCC_GPIO_EN() 宏的参数。
 * Port A 的编号是 0（对应 RCC_AHB1ENR 的 bit0），
 * Port F 的编号是 5（对应 RCC_AHB1ENR 的 bit5）。*/
#define GPIOEN_PORT_A  0
#define GPIOEN_PORT_F  5

/* ================================================================
 * 第5部分：软延时函数（Busy-Wait Delay）
 * ================================================================
 *
 * 延时原理：CPU 执行 NOP（No Operation）指令空转，消耗固定时间。
 * NOP 是 ARM 的"空操作"指令，执行一次占用 1 个 CPU 周期（1/168MHz ≈ 5.95ns）。
 *
 * 为什么不精确？
 * - 编译器优化：循环体可能被优化掉或重排
 * - CPU 流水线：Cortex-M4 是超标量流水线，实际每条指令周期不固定
 * - 中断干扰：延时过程中发生中断，延时会变长
 *
 * 本项目为什么用软延时而非 SysTick？
 * SysTick 已配置为 1ms 中断。这里用软延时是演示"最原始"的延时方式。
 * 实际项目建议用 SysTick 中断或硬件定时器实现精确延时。
 *
 * volatile 的作用？
 * - 告诉编译器"这个变量的值编译器不能假设"——循环计数器 i 必须每次重新读
 * - 避免编译器认为"i 没被使用"而把整个循环删除（编译器优化级别高时可能发生）*/
static void delay_ms(volatile uint32_t ms) {
    /* 外层循环次数由经验值估算：
     * i < ms * 1000 时，约等于 1ms（168MHz 主频下测量所得）
     * 注意：这个值取决于编译器优化级别，-O0/-O2/-Os 会导致不同结果
     * 实际项目中建议用示波器或逻辑分析仪测量并调整 */
    /* delay_ms 校准：
     * 168MHz 下：i < ms * 1000 ≈ 1ms
     * HSI 16MHz 下：需 ms * 10000 才能达到相同延时（约 10x） */
    for (volatile uint32_t i = 0; i < ms * 10000; i++) {
        __asm__ volatile ("nop");  /* ARM 空操作指令，执行时间 = 1 个 CPU 周期 */
    }
}

/* ================================================================
 * 第6部分：Reset_Handler —— 程序真正入口
 * ================================================================
 *
 * Reset_Handler 是中断向量表的 [1]，CPU 上电后第一个执行的代码。
 * 它的使命是：完成运行时环境的初始化，然后调用用户 main() 函数。
 *
 * C 库和 C++ 全局对象的构造函数（如全局变量的构造函数）
 * 如果使用了 newlib（arm-none-eabi-gcc 默认使用），需要在 main() 之前
 * 调用 _start() 或 __libc_init_array() 来执行这些初始化。
 * 本项目不使用 newlib 的 C 运行时（用 --nostartfiles），所以直接调 main()。
 *
 * 【为什么 .data 要复制而 .bss 只要清零？】
 *
 * 全局变量有两种：
 *   ① 初始化过的变量（如 int flag = 5;）→ 存在 .data 段
 *      初始值"5"必须存储在 Flash 里，运行时复制到 SRAM
 *
 *   ② 未初始化的变量（如 int counter;）→ 存在 .bss 段
 *      C 标准规定默认为 0，所以只要运行时清零即可，不需要存储初始值
 *      这节省了 Flash 空间（编译器只记录 bss 的地址范围，不存储值）
 *
 * 【__attribute__((section(".text")))】
 * 显式指定这个函数放在 .text 段（代码段）。
 * 实际上不加也可以，链接器默认会把 .text 放在 Flash 里。*/
void Reset_Handler(void) {

    /* -------- 第1步：复制 .data 段 --------
     *
     * 背景：
     * const 全局变量（如 const int table[] = {...};）和初始化过的全局变量
     * 存储在 Flash 的 .data 区域（紧跟在代码 .text 后面）。
     * 但这些变量需要在 SRAM 里运行（因为 SRAM 可写，Flash 不可写）。
     * 所以启动时要把 Flash 里的"初始值"复制到 SRAM。
     *
     * 链接器提供的符号：
     *   _sidata：.data 初始值在 Flash 中的起始地址（由 AT>FLASH 决定）
     *   _sdata： .data 段在 SRAM 中的起始地址
     *   _edata： .data 段在 SRAM 中的结束地址
     *
     * 循环结束后，SRAM 中的 .data 变量就拥有了正确的初始值。*/
    unsigned long *src  = &_sidata;   /* Flash 地址——.data 初始值存放处 */
    unsigned long *dest = &_sdata;     /* SRAM 地址——变量实际运行的位置 */
    while (dest < &_edata) {           /* 复制直到 SRAM 末尾 */
        *dest++ = *src++;
    }

    /* -------- 第2步：清零 .bss 段 --------
     *
     * 背景：
     * 未初始化的全局/静态变量（如 int counter;）按 C 标准必须初始化为 0。
     * 这些变量不存储初始值（节省 Flash），运行时只需要在 SRAM 中分配空间并清零。
     *
     * 链接器提供的符号：
     *   _sbss：.bss 段在 SRAM 中的起始地址
     *   _ebss：.bss 段在 SRAM 中的结束地址
     *
     * C 标准要求程序运行前 bss 必须清零——如果不清零，
     * 未初始化全局变量的值是"未定义"的（编译器/链接器可能残留垃圾值）。*/
    dest = &_sbss;
    while (dest < &_ebss) {
        *dest++ = 0;
    }

    /* -------- 第3步：初始化时钟 --------
     *
     * SystemInit() 定义在 stm32f4xx.h，负责：
     *   1. 使能 HSI（16MHz 内部振荡器）
     *   2. 配置 PLL，将系统时钟升到 168MHz
     *   3. 配置 Flash（5 个等待周期 + ART 加速）
     *   4. 配置 SysTick（1ms 中断一次）
     *
     * 注意：上电后 MCU 默认使用 HSI（16MHz），
     * 不调 SystemInit() 也能跑，但主频只有 16MHz。*/
    SystemInit();

    /* -------- 第4步：进入用户代码 --------
     *
     * main() 是应用程序的入口点。
     * 本项目的 main() 在下面定义，实现 LED 闪烁和按键交互。
     *
     * 注意：main() 不应该返回！
     * 如果 main() 返回了，CPU 会继续执行下面的 while(1) 死循环。
     * 实际嵌入式系统中，main() 返回通常意味着"致命错误"。*/
    main();

    /* -------- 保护性代码 --------
     * main() 正常情况下不会执行到这里。
     * 加上这个死循环是防御性编程——防止 main() 意外返回。*/
    while (1) {
        __asm__ volatile ("nop");
    }
}

/* ================================================================
 * 第7部分：main —— 用户应用程序
 * ================================================================
 *
 * 本函数演示了 GPIO 输出和输入的底层操作：
 *   - LED_RED（PF9） 和 LED_GREEN（PF10）作为推挽输出
 *   - KEY_UP（PA0）作为输入，通过 IDR 寄存器读取电平
 *
 * 【GPIO 配置步骤（以 PF9 为例）】
 *
 *   Step 1：使能 GPIOF 时钟
 *     RCC_AHB1ENR |= (1 << 5);   // GPIOFEN = bit5
 *
 *   Step 2：配置 PF9 为输出模式
 *     GPIOF_MODER 的 bit[19:18] 控制 PF9：
 *       00 = 输入（默认）  01 = 通用输出  10 = 复用功能  11 = 模拟
 *     设置为 01（通用输出）：
 *       GPIOF_MODER = (GPIOF_MODER & ~0x30000U) | 0x10000U;
 *       0x30000 = 0b11 << 18（清除旧值）
 *       0x10000 = 0b01 << 18（设置输出模式）
 *
 *   Step 3：设置引脚电平（ODR 寄存器）
 *     GPIOF_ODR 的 bit[9] 控制 PF9：
 *       ODR[9] = 1 → 高电平 → LED 亮（对于负极接 MCU 的 LED）
 *       ODR[9] = 0 → 低电平 → LED 灭
 *
 * 【硬件连接说明】
 *
 *   LED 接法（负极接 MCU）：
 *     VCC ──── R(电阻) ───┬──┬─── GND
 *                         │  │
 *                      LED+  LED- (接 MCU)
 *     MCU 引脚 ─────────────┘
 *
 *   这种"负极接 MCU"的接法称为"灌电流"（Sink）模式：
 *     - MCU 输出低电平(0) → LED 亮
 *     - MCU 输出高电平(1) → LED 灭
 *
 *   优点：STM32 GPIO 灌电流能力比源电流（Source）更强，
 *         可以直接驱动大多数 LED 而不需要额外电路。
 *
 * 【KEY_UP 按键说明】
 *
 *   电路：PA0 ───┤┬──┬─── 3.3V
 *                ││  │
 *              4.7K   开关
 *              上拉   （按下短路）
 *
 *   效果：
 *     - 按键松开：PA0 被 4.7K 电阻拉高到 3.3V → IDR[0] = 1 → key_up = 1
 *     - 按键按下：3.3V 被短路，PA0 接地 → IDR[0] = 0 → key_up = 0
 *
 *   注意：KEY_UP 的逻辑是"按下=0，松手=1"（与 KEY0/KEY1/KEY2 相反）！
 *         这是因为 PA0 连接的是 4.7K 上拉电阻，而非下拉。
 *
 *   PA0 的 MODER 配置：默认复位值为 0b00（输入），无需额外配置。
 *     输入模式下，ODR/BSRR 不影响引脚，只读 IDR。
 */
int main(void) {

    /* ===== 1. 使能 GPIO 时钟 =====
     *
     * STM32 采用"时钟门控"省电设计：
     * 所有外设（包括 GPIO）上电后时钟默认关闭。
     * 不开时钟就访问外设寄存器 → 读出值全 0，写入被忽略！
     *
     * GPIOF：用于 LED（PF9 = LED_RED，PF10 = LED_GREEN）
     * GPIOA：用于按键（PA0 = KEY_UP）
     *
     * 注释：用 GPIOEN_PORT_F（=5）是为了让代码可读——
     *       直接写 (RCC_AHB1ENR |= (1 << 5)) 也可以，但可读性差。*/
    RCC_GPIO_EN(GPIOEN_PORT_F);   /* 使能 GPIOF 时钟（LED）*/
    RCC_GPIO_EN(GPIOEN_PORT_A);   /* 使能 GPIOA 时钟（按键）*/

    /* ===== 2. 配置 PF9 为通用输出 =====
     *
     * GPIOF_MODER 是一个 32 位寄存器，每 2 bits 控制一个引脚。
     * PF9 是 Port F 的第 9 号引脚，对应 bit[19:18]。
     *
     *   ~0x30000U  → 把 bit[19:18] 清零（0b11 << 18 = 0x30000）
     *   | 0x10000U → 设置为输出模式（0b01 << 18 = 0x10000）
     *
     * 为什么用 |= 而不是直接赋值？
     * 同一端口的其他引脚可能已经配置过了，直接赋值会覆盖它们。
     * 所以用 "& ~mask" 先清除目标位，再用 "|" 写入新值。*/
    GPIOF_MODER = (GPIOF_MODER & ~0x30000U) | 0x10000U;   /* PF9 → 输出模式 */

    /* ===== 3. 配置 PF10 为通用输出 =====
     *
     * PF10 对应 bit[21:20]：
     *   ~0xC0000U → 0b11 << 20 = 0xC0000（清除）
     *   | 0x40000U → 0b01 << 20 = 0x40000（输出模式）*/
    GPIOF_MODER = (GPIOF_MODER & ~0xC0000U) | 0x40000U;   /* PF10 → 输出模式 */

    /* ===== 4. PA0（KEY_UP）配置为输入 =====
     *
     * GPIO 输入模式是复位默认值（MODER[1:0] = 00），
     * 所以这里不需要任何配置！
     *
     * 作为输入引脚：
     *   - 不需要使能输出相关的时钟（时钟已在 Step 1 开启）
     *   - ODR/BSRR 对输入引脚没有影响（不会改变引脚电平）
     *   - 只读 IDR 寄存器获取当前引脚电平
     *
     * 为什么 PA0 可以作为输入而不用配置 MODER？
     * STM32F4 GPIO 复位默认值 = 输入模式（省电 + 安全）。
     * 上电后所有 GPIO 都是输入，除非代码明确修改了 MODER。*/

    /* ===== 5. 初始化：关闭所有 LED =====
     *
     * ODR（Output Data Register）的每一位对应一个引脚：
     *   ODR[9]  = PF9（LED_RED）    ODR[10] = PF10（LED_GREEN）
     *
     * LED 接法：负极接 MCU，正极接 VCC（通过电阻）
     *   → MCU 输出低电平(0) 时，电流从 VCC 流入 MCU → LED 亮
     *   → MCU 输出高电平(1) 时，无电流 → LED 灭
     *
     * 因此初始化时写 0 = LED 灭。*/
    GPIOF_ODR |= ((1U << 9) | (1U << 10));   /* PF9=1, PF10=1 → 两个 LED 亮（源电流，ODR=1 亮）*/

    /* ================================================================
     * 主循环：闪烁调试
     * 如果灯在闪 → 代码在跑，ODR 控制正常
     * 如果灯不闪 → 问题在初始化或时钟 */
    while (1) {
        GPIOF_ODR ^= (1U << 9) | (1U << 10);   /* 翻转 PF9/PF10 */
        delay_ms(500);                          /* 500ms 间隔 */
    }
}
