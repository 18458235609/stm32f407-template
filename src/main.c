/**
 * @file main.c
 * @brief STM32F407VG 应用程序入口 — 裸机 LED 闪烁示例
 *
 * 本文件实现了 STM32 从上电到运行的完整流程，不依赖任何第三方库：
 *   1. 中断向量表（向量表必须放在 Flash 起始地址 0x08000000）
 *   2. C 运行时初始化（.data 复制 + .bss 清零）
 *   3. 系统时钟配置（通过 SystemInit() 将主频设为 168MHz）
 *   4. 用户应用程序（GPIO 输出驱动 LED 闪烁）
 *
 * 【学习目标】
 *   - 理解嵌入式程序的启动流程（上电 → 向量表 → Reset_Handler → main）
 *   - 理解中断向量表的结构和作用
 *   - 理解 .data / .bss 段的初始化原理
 *   - 掌握 GPIO 输出控制的基本方法
 *
 * 【执行流程图】
 *
 *   芯片上电
 *     │
 *     ▼
 *   硬件自动从 vectors[0] 加载 SP（栈指针）= 0x20030000
 *   硬件自动从 vectors[1] 加载 PC（程序计数器）= Reset_Handler 地址
 *     │
 *     ▼
 *   Reset_Handler():
 *     ① 复制 .data 段（Flash → SRAM）
 *     ② 清零 .bss 段
 *     ③ 调用 SystemInit()（配置 PLL → 168MHz）
 *     ④ 调用 main()
 *     ⑤ main() 返回后死循环（正常情况 main 不应返回）
 */

#include "stm32f4xx.h"
#include "pindef.h"

void Reset_Handler(void);
int main(void);

/* ============================================================================
 * 默认中断处理函数
 * ============================================================================
 *
 * Default_Handler 是一个死循环函数，用于处理所有"未专门实现"的中断。
 *
 * 【为什么需要默认处理函数？】
 *   如果某个中断被触发但没有对应的处理函数，CPU 会跳转到一个随机地址执行，
 *   导致不可预测的行为（通常是 HardFault 或程序跑飞）。
 *   提供一个默认的死循环处理函数，可以让程序在调试时停在一个确定的位置。
 *
 * 【调试技巧】
 *   如果程序突然"卡死"不响应，在调试器中暂停，查看 PC 寄存器：
 *   - 如果停在 Default_Handler 里，说明有未处理的中断被触发了
 *   - 可以通过 LR 寄存器（返回地址）判断是哪个中断源
 */
void Default_Handler(void) {
    while (1) {
    }
}

/* ============================================================================
 * 中断服务函数（ISR）别名定义
 * ============================================================================
 *
 * 使用 GCC 的 __attribute__((alias())) 将多个中断处理函数指向同一个
 * Default_Handler，避免为每个中断都写一个空函数。
 *
 * 【ARM Cortex-M4 异常向量表（前 16 个是内核异常）】
 *
 *   编号  名称              说明
 *   ──────────────────────────────────────────────────────
 *   -2   (向量表[0])        初始 SP 值（不是函数，是栈顶地址）
 *   -1   (向量表[1])        Reset_Handler（复位向量，程序入口）
 *    0   (向量表[2])        NMI_Handler（不可屏蔽中断）
 *    1   (向量表[3])        HardFault_Handler（硬件故障）★ 最常遇到
 *    2   (向量表[4])        MemManage_Handler（内存管理故障）
 *    3   (向量表[5])        BusFault_Handler（总线故障）
 *    4   (向量表[6])        UsageFault_Handler（用法故障）
 *   5~7  (向量表[7~9])      保留
 *    8   (向量表[10])       SVC_Handler（超级用户调用，RTOS 常用）
 *    9   (向量表[11])       DebugMon_Handler（调试监控）
 *   10   (向量表[12])       保留
 *   11   (向量表[13])       PendSV_Handler（可挂起系统服务，RTOS 任务切换用）
 *   12   (向量表[14])       SysTick_Handler（系统滴答定时器）
 *
 * 【常见故障原因】
 *   HardFault 最常见的原因：
 *     - 空指针解引用（访问地址 0x00000000）
 *     - 访问未映射的内存地址
 *     - 栈溢出（SP 超出 SRAM 范围）
 *     - 除零操作
 *     - 未对齐的内存访问（某些情况）
 */
void NMI_Handler(void)        __attribute__((alias("Default_Handler")));
void HardFault_Handler(void)   __attribute__((alias("Default_Handler")));
void MemManage_Handler(void)   __attribute__((alias("Default_Handler")));
void BusFault_Handler(void)    __attribute__((alias("Default_Handler")));
void UsageFault_Handler(void) __attribute__((alias("Default_Handler")));
void SVC_Handler(void)        __attribute__((alias("Default_Handler")));
void DebugMon_Handler(void)    __attribute__((alias("Default_Handler")));
void PendSV_Handler(void)      __attribute__((alias("Default_Handler")));

/* ============================================================================
 * SysTick 中断处理函数
 * ============================================================================
 *
 * SysTick_Handler 是本项目唯一一个有实际实现的中断处理函数。
 *
 * 【为什么 SysTick_Handler 不能用 Default_Handler？】
 *   SystemInit() 中配置了 SysTick 定时器（SYST_CSR = 0x07），
 *   每 1ms 触发一次中断。如果 SysTick_Handler 是死循环，
 *   CPU 进入 main() 后不到 1ms 就会被 SysTick 中断劫持到死循环中，
 *   导致程序"卡死"——LED 亮了但不会闪烁。
 *
 *   因此 SysTick_Handler 必须是一个空函数：中断触发后立即返回，
 *   让主循环继续执行。
 *
 * 【进阶用法】
 *   可以在此函数中维护一个全局毫秒计数器，实现精确延时：
 *     volatile uint32_t systick_ms = 0;
 *     void SysTick_Handler(void) { systick_ms++; }
 *   然后在主循环中用 systick_ms 计算时间差。
 */
void SysTick_Handler(void) {
}

/* ============================================================================
 * 链接器符号声明
 * ============================================================================
 *
 * 这些符号由链接脚本（ld/STM32F407VG.ld）定义，表示各段在内存中的地址边界。
 * 它们不是 C 变量（没有存储空间），而是"地址常量"，取地址 & 即得到对应地址值。
 *
 *   _estack — 栈顶地址（SRAM 末端 + 1 = 0x20030000）
 *             同时也是向量表 [0] 的值（硬件上电后自动加载到 SP）
 *
 *   _sidata — .data 段初始值在 Flash 中的起始地址
 *             Reset_Handler 从这里读取初始值，复制到 SRAM
 *
 *   _sdata  — .data 段在 SRAM 中的起始地址（运行时地址）
 *   _edata  — .data 段在 SRAM 中的结束地址
 *
 *   _sbss   — .bss 段在 SRAM 中的起始地址
 *   _ebss   — .bss 段在 SRAM 中的结束地址
 *
 * 【内存布局图】
 *
 *   Flash:  [向量表][.text代码][.rodata常量][.data初始值]
 *                                       ↑_sidata
 *
 *   SRAM:   [.data运行时][.bss零初始化][    空闲    ][栈↓]
 *           ↑_sdata      ↑_sbss       ↑_end      ↑_estack
 *           ↑_edata      ↑_ebss
 */
extern unsigned long _estack;
extern unsigned long _sidata, _sdata, _edata, _sbss, _ebss;

/* ============================================================================
 * 中断向量表
 * ============================================================================
 *
 * 向量表是整个程序最关键的数据结构——它告诉 CPU "发生某个事件时跳转到哪里"。
 *
 * 【硬件行为】
 *   芯片上电/复位后，CPU 自动执行以下操作：
 *     1. 从地址 0x08000000 读取 32 位值 → 加载到 SP（主栈指针）
 *     2. 从地址 0x08000004 读取 32 位值 → 加载到 PC（程序计数器）
 *     3. CPU 从 PC 指向的地址开始执行（即 Reset_Handler）
 *
 * 【向量表格式】
 *   每个条目是一个 32 位函数指针（Thumb 地址，bit0=1），
 *   按异常/中断编号顺序排列。
 *
 * 【__attribute__((section(".isr_vector")))】
 *   将此数组强制放到 .isr_vector 段。
 *   链接脚本中 .isr_vector 段被放在 Flash 最开头（0x08000000），
 *   并用 KEEP() 防止被链接器优化删除。
 *
 * 【const 修饰】
 *   vectors 是 const 数组，存放在 Flash（只读），不能在运行时修改。
 *   这与 PC 程序不同——嵌入式程序的向量表通常在 Flash 中。
 *
 * 【0x20030000 的含义】
 *   向量表 [0] 不是函数指针，而是初始栈指针值。
 *   STM32F407VG 的 SRAM 范围是 0x20000000 ~ 0x2002FFFF（192KB），
 *   0x20030000 = SRAM 末端 + 1 = 栈顶。
 *   ARM 的栈向下生长，所以 SP 初始值设为最高地址。
 */
void (* const vectors[])(void) __attribute__((section(".isr_vector"))) = {
    (void (*)(void))0x20030000,   /* [0]  初始 SP = SRAM 最高地址 + 1 */
    Reset_Handler,                /* [1]  复位向量 — 程序入口 ★ */
    NMI_Handler,                  /* [2]  不可屏蔽中断 */
    HardFault_Handler,            /* [3]  硬件故障 */
    MemManage_Handler,            /* [4]  内存管理故障 */
    BusFault_Handler,             /* [5]  总线故障 */
    UsageFault_Handler,           /* [6]  用法故障 */
    0,                            /* [7]  保留 */
    0,                            /* [8]  保留 */
    0,                            /* [9]  保留 */
    0,                            /* [10] 保留 */
    SVC_Handler,                  /* [11] 超级用户调用 */
    DebugMon_Handler,             /* [12] 调试监控 */
    0,                            /* [13] 保留 */
    PendSV_Handler,               /* [14] 可挂起系统服务 */
    SysTick_Handler,              /* [15] 系统滴答定时器 */
};

/* ============================================================================
 * GPIO 时钟使能宏
 * ============================================================================
 *
 * RCC_AHB1ENR 寄存器控制 AHB1 总线上外设的时钟开关。
 * GPIO 挂在 AHB1 总线上，使用前必须先使能对应端口的时钟。
 *
 * 【RCC_AHB1ENR 位域】
 *   bit0 = GPIOAEN  使能 GPIOA 时钟
 *   bit1 = GPIOBEN  使能 GPIOB 时钟
 *   bit2 = GPIOCEN  使能 GPIOC 时钟
 *   bit3 = GPIODEN  使能 GPIOD 时钟
 *   bit4 = GPIOEEN  使能 GPIOE 时钟
 *   bit5 = GPIOFEN  使能 GPIOF 时钟 ← 本项目使用
 *   bit6 = GPIOGEN  使能 GPIOG 时钟
 *
 * 【为什么必须先使能时钟？】
 *   STM32 出于低功耗设计，所有外设时钟默认关闭。
 *   如果不使能时钟就操作 GPIO 寄存器，写入的值不会生效，
 *   读取会返回 0（因为外设没有被时钟驱动）。
 */

#define RCC_GPIO_EN(port)  (RCC_AHB1ENR |= (1 << (port)))
#define GPIOEN_PORT_A  0
#define GPIOEN_PORT_F  5

/* ============================================================================
 * delay_ms — 简易毫秒延时函数
 * ============================================================================
 *
 * 使用忙等待（busy-wait）方式实现延时，不依赖 SysTick 中断。
 *
 * 【工作原理】
 *   在 168MHz 主频下，每个循环大约消耗 10 个时钟周期
 *   （nop 指令 + 循环开销 + volatile 读写），
 *   因此 ms * 10000 次循环 ≈ ms 毫秒。
 *
 * 【局限性】
 *   - 不精确：实际延时受编译优化、缓存命中率等影响
 *   - 浪费 CPU：延时期间 CPU 无法做其他事情
 *   - 受中断影响：如果延时期间有中断，实际延时会变长
 *
 * 【更精确的替代方案】
 *   使用 SysTick 计数器实现非阻塞延时：
 *     uint32_t start = systick_ms;
 *     while ((systick_ms - start) < ms);
 *
 * 【volatile 的作用】
 *   - volatile uint32_t ms：防止编译器将 ms 缓存到寄存器
 *   - volatile uint32_t i：防止编译器优化掉循环
 *   - __asm__ volatile ("nop")：插入空指令，防止编译器删除循环体
 */
static void delay_ms(volatile uint32_t ms) {
    for (volatile uint32_t i = 0; i < ms * 10000; i++) {
        __asm__ volatile ("nop");
    }
}

/* ============================================================================
 * Reset_Handler — 复位处理函数（程序真正的入口点）
 * ============================================================================
 *
 * Reset_Handler 是整个程序最先执行的函数（由硬件自动跳转）。
 * 它负责完成 C 运行时环境的初始化，然后调用 main()。
 *
 * 【执行顺序】
 *   ① 复制 .data 段：将全局/静态变量的初始值从 Flash 复制到 SRAM
 *   ② 清零 .bss 段：将未初始化的全局/静态变量清零
 *   ③ 调用 SystemInit()：配置 PLL 时钟到 168MHz
 *   ④ 调用 main()：进入用户应用程序
 *   ⑤ main() 返回后死循环（正常情况 main 不应返回）
 *
 * 【为什么需要 .data 复制？】
 *   C 语言中 int x = 42; 这样的全局变量：
 *   - 变量 x 的"家"在 SRAM（运行时需要读写）
 *   - 初始值 42 存储在 Flash（掉电不丢失）
 *   - 上电后 SRAM 内容是随机的，必须从 Flash 把 42 复制过来
 *
 * 【为什么需要 .bss 清零？】
 *   C 标准规定：未初始化的全局变量默认值为 0。
 *   int counter; → counter 必须是 0，但上电后 SRAM 内容随机。
 *   .bss 段不占 Flash 空间（不需要存储初始值），只需在运行时清零。
 */
void Reset_Handler(void) {
    /* --- ① 复制 .data 段（Flash → SRAM）---
     * _sidata: .data 初始值在 Flash 中的起始地址
     * _sdata:  .data 在 SRAM 中的起始地址
     * _edata:  .data 在 SRAM 中的结束地址
     *
     * 逐字（4 字节）复制，直到 dest 到达 _edata */
    unsigned long *src  = &_sidata;
    unsigned long *dest = &_sdata;
    while (dest < &_edata) {
        *dest++ = *src++;
    }

    /* --- ② 清零 .bss 段 ---
     * _sbss: .bss 在 SRAM 中的起始地址
     * _ebss: .bss 在 SRAM 中的结束地址
     *
     * 逐字清零，确保所有未初始化变量为 0 */
    dest = &_sbss;
    while (dest < &_ebss) {
        *dest++ = 0;
    }

    /* --- ③ 配置系统时钟 ---
     * SystemInit() 定义在 stm32f4xx.h 中
     * 将主频从默认 16MHz (HSI) 切换到 168MHz (HSE + PLL) */
    SystemInit();

    /* --- ④ 调用用户程序 ---
     * main() 正常情况下不应返回（嵌入式程序的 main 通常是无限循环）
     * 如果意外返回，下面的死循环可以防止 CPU 跑飞 */
    main();

    /* --- ⑤ 安全死循环 ---
     * main() 返回后在此等待，防止 CPU 执行未定义内存区域
     * __asm__ volatile ("nop") 插入空指令，让 CPU 有事可做
     * （某些调试器在纯死循环中无法正常暂停） */
    while (1) {
        __asm__ volatile ("nop");
    }
}

/* ============================================================================
 * main — 用户应用程序入口
 * ============================================================================
 *
 * 本示例实现红绿 LED 交替闪烁：
 *   - PF9  = LED_RED   (DS0 红灯)
 *   - PF10 = LED_GREEN (DS1 绿灯)
 *
 * 【GPIO 操作三步曲】
 *   1. 使能时钟：RCC_AHB1ENR |= (1 << 5)  使能 GPIOF 时钟
 *   2. 配置模式：GPIOF_MODER 设为输出模式（01）
 *   3. 输出数据：GPIOF_ODR 控制引脚电平
 *
 * 【LED 硬件连接】
 *   神舟号开发板上 LED 的连接方式：
 *     MCU PF9 ──► LED0(红) ──► GND
 *     MCU PF10 ──► LED1(绿) ──► GND
 *   输出高电平 → LED 亮；输出低电平 → LED 灭
 *
 * 【MODER 寄存器位域计算方法】
 *   引脚 n 的模式位位于 bit[2n+1 : 2n]
 *
 *   PF9:  n=9, bit[19:18]
 *     清除掩码 = ~(0b11 << 18) = ~0xC0000
 *     输出模式 = 0b01 << 18 = 0x40000
 *
 *   PF10: n=10, bit[21:20]
 *     清除掩码 = ~(0b11 << 20) = ~0x300000
 *     输出模式 = 0b01 << 20 = 0x100000
 *
 * 【ODR 寄存器操作】
 *   bit9  = PF9  电平（1=高/亮，0=低/灭）
 *   bit10 = PF10 电平（1=高/亮，0=低/灭）
 *
 *   GPIOF_ODR |= (1<<9) | (1<<10)  → 两个 LED 都亮
 *   GPIOF_ODR ^= (1<<9) | (1<<10)  → 翻转两个 LED 状态
 */
int main(void) {
    /* --- 步骤 1：使能 GPIO 时钟 ---
     * RCC_AHB1ENR bit5 = GPIOFEN → 使能 GPIOF 时钟
     * RCC_AHB1ENR bit0 = GPIOAEN → 使能 GPIOA 时钟（本项目暂未使用 GPIOA，预留） */
    RCC_GPIO_EN(GPIOEN_PORT_F);
    RCC_GPIO_EN(GPIOEN_PORT_A);

    /* --- 步骤 2：配置 GPIO 模式为输出 ---
     *
     * GPIOF_MODER 复位默认值 = 0x00000000（所有引脚为输入模式）
     *
     * PF9 (LED_RED): bit[19:18]
     *   清除: & ~0xC0000  → 将 bit[19:18] 清零
     *   设置: | 0x40000   → 写入 01（通用输出模式）
     *
     * PF10 (LED_GREEN): bit[21:20]
     *   清除: & ~0x300000 → 将 bit[21:20] 清零
     *   设置: | 0x100000  → 写入 01（通用输出模式）
     *
     * 注意：两行代码顺序不能颠倒，因为第二行读到的 MODER 值
     *       必须包含第一行修改后的结果 */
    GPIOF_MODER = (GPIOF_MODER & ~0xC0000U) | 0x40000U;
    GPIOF_MODER = (GPIOF_MODER & ~0x300000U) | 0x100000U;

    /* --- 步骤 3：初始点亮两个 LED ---
     * GPIOF_ODR bit9=1 → PF9 输出高电平 → 红灯亮
     * GPIOF_ODR bit10=1 → PF10 输出高电平 → 绿灯亮 */
    GPIOF_ODR |= ((1U << 9) | (1U << 10));

    /* --- 步骤 4：主循环 — 交替闪烁 ---
     * XOR (^) 操作翻转对应位：
     *   0 → 1（灭 → 亮）
     *   1 → 0（亮 → 灭）
     *
     * 效果：红绿灯同时亮灭交替，每 500ms 切换一次
     *
     * 进阶：如果想让红绿灯交替亮（一个亮一个灭），可以分别翻转：
     *   GPIOF_ODR ^= (1U << 9);   // 翻转红灯
     *   GPIOF_ODR ^= (1U << 10);  // 翻转绿灯
     *   delay_ms(500); */
    while (1) {
        GPIOF_ODR ^= (1U << 9) | (1U << 10);
        delay_ms(500);
    }
}
