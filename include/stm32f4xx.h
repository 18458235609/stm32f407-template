/**
 * @file stm32f4xx.h
 * @brief STM32F4 最小寄存器定义 + 系统时钟初始化
 *
 * 本文件是整个项目的"硬件抽象层"，定义了 STM32F407VG 芯片的关键寄存器地址，
 * 并提供了 SystemInit() 函数将主频配置到 168MHz。
 *
 * 【为什么不使用 ST 官方 HAL 库？】
 *   - HAL 库封装层级高，初学者难以理解底层原理
 *   - 本文件用最少的代码直接操作寄存器，让你看到"每一步到底在做什么"
 *   - 理解寄存器操作后，再使用 HAL 库会事半功倍
 *
 * 【寄存器操作的基本原理】
 *   STM32 的外设（GPIO、USART、TIM 等）都是"内存映射"的：
 *   每个外设占用一段固定地址，读写这些地址就等于操作硬件寄存器。
 *
 *   例如：GPIOF_ODR 的地址是 0x40021414
 *     - 写入 0x200 → bit9=1 → PF9 输出高电平 → 红灯亮
 *     - 写入 0x000 → bit9=0 → PF9 输出低电平 → 红灯灭
 *
 *   volatile 关键字确保编译器不会优化掉对寄存器的读写操作。
 *
 * 【地址计算方法】
 *   所有外设地址从 PERIPH_BASE (0x40000000) 开始，逐级偏移：
 *
 *   PERIPH_BASE  = 0x40000000          ← ARM 规定的外设基地址
 *   APB1_BASE    = PERIPH_BASE + 0x0000 ← APB1 总线（低速外设：USART2/3, TIM2-7, I2C, SPI2/3...）
 *   APB2_BASE    = PERIPH_BASE + 0x10000 ← APB2 总线（高速外设：USART1, TIM1, ADC, SPI1...）
 *   AHB1_BASE    = PERIPH_BASE + 0x20000 ← AHB1 总线（GPIO, RCC, DMA, CRC...）
 *
 *   RCC_BASE     = AHB1_BASE + 0x3800   ← 复位和时钟控制（Reset & Clock Control）
 *   GPIOA_BASE   = AHB1_BASE + 0x0000   ← GPIO 端口 A
 *   GPIOF_BASE   = AHB1_BASE + 0x1400   ← GPIO 端口 F
 *
 * 【参考文档】
 *   STM32F407 Reference Manual (RM0090):
 *   https://www.st.com/resource/en/reference_manual/dm00031020.pdf
 */

#ifndef __STM32F4XX_H__
#define __STM32F4XX_H__

#include <stdint.h>

/* ============================================================================
 * 总线基地址定义
 * ============================================================================
 *
 * STM32F4 采用 ARM AMBA 总线架构，外设按速度分为三条总线：
 *
 *   AHB1 (Advanced High-performance Bus) — 168MHz 最大
 *     挂载：GPIOA~G, RCC, DMA1/2, CRC, Backup SRAM 等
 *     特点：带宽最高，直接连 CPU，适合高速数据搬移
 *
 *   APB2 (Advanced Peripheral Bus 2) — 84MHz 最大
 *     挂载：USART1, TIM1/8/9/10/11, ADC1/2/3, SPI1, SYSCFG 等
 *     特点：中速外设，通过 AHB-APB 桥接
 *
 *   APB1 (Advanced Peripheral Bus 1) — 42MHz 最大
 *     挂载：USART2/3/4/5, TIM2-7/12-14, I2C1/2/3, SPI2/3, CAN 等
 *     特点：低速外设，通过 AHB-APB 桥接
 *
 * 地址空间布局（简化）：
 *   0x4000_0000 ─── APB1 外设
 *   0x4001_0000 ─── APB2 外设
 *   0x4002_0000 ─── AHB1 外设（GPIO, RCC, DMA...）
 */

#define PERIPH_BASE       0x40000000U
#define APB1_BASE         (PERIPH_BASE + 0x00000000U)
#define APB2_BASE         (PERIPH_BASE + 0x00010000U)
#define AHB1_BASE         (PERIPH_BASE + 0x00020000U)

/* ============================================================================
 * RCC — 复位和时钟控制（Reset & Clock Control）
 * ============================================================================
 *
 * RCC 是 STM32 最重要的外设之一，控制所有外设的时钟开关和频率。
 *
 * 【为什么需要时钟控制？】
 *   STM32 有上百个外设，如果所有外设时钟都开着，功耗会非常高。
 *   出于省电考虑，芯片复位后所有外设时钟默认关闭。
 *   使用任何外设前，必须先在 RCC 寄存器中使能对应的时钟。
 *
 * 【关键寄存器】
 *   RCC_CR      — 时钟控制寄存器（使能 HSI/HSE/PLL，查看就绪标志）
 *   RCC_CFGR    — 时钟配置寄存器（选择系统时钟源，分频系数）
 *   RCC_PLLCFGR — PLL 配置寄存器（倍频/分频参数）
 *   RCC_AHB1ENR — AHB1 外设时钟使能（GPIOA~G, DMA, CRC 等）
 *   RCC_APB1ENR — APB1 外设时钟使能（USART2/3, TIM2-7, I2C 等）
 *   RCC_APB2ENR — APB2 外设时钟使能（USART1, TIM1, ADC, SPI1 等）
 *
 * RCC 基地址 = AHB1_BASE + 0x3800 = 0x40023800
 * （RCC 虽然控制 AHB/APB 总线，但自身挂在 AHB1 上）
 */

#define RCC_BASE          (AHB1_BASE + 0x3800U)

#define RCC_CR            (*(volatile uint32_t *)(RCC_BASE + 0x00))
#define RCC_CFGR          (*(volatile uint32_t *)(RCC_BASE + 0x08))
#define RCC_PLLCFGR       (*(volatile uint32_t *)(RCC_BASE + 0x04))
#define RCC_AHB1ENR       (*(volatile uint32_t *)(RCC_BASE + 0x30))
#define RCC_APB1ENR       (*(volatile uint32_t *)(RCC_BASE + 0x40U))
#define RCC_APB2ENR       (*(volatile uint32_t *)(RCC_BASE + 0x44U))

/* ============================================================================
 * GPIO — 通用输入输出（General Purpose I/O）
 * ============================================================================
 *
 * STM32F4 每个 GPIO 端口有 16 个引脚（Pin 0~15），每个端口占用 0x400 字节地址空间。
 *
 * 【GPIO 端口地址偏移】
 *   GPIOA = AHB1_BASE + 0x0000 = 0x40020000
 *   GPIOB = AHB1_BASE + 0x0400 = 0x40020400
 *   GPIOC = AHB1_BASE + 0x0800 = 0x40020800
 *   GPIOD = AHB1_BASE + 0x0C00 = 0x40020C00
 *   GPIOE = AHB1_BASE + 0x1000 = 0x40021000
 *   GPIOF = AHB1_BASE + 0x1400 = 0x40021400
 *   GPIOG = AHB1_BASE + 0x1800 = 0x40021800
 *
 * 【GPIO 寄存器偏移】
 *   +0x00  MODER    模式寄存器（每 2 bit 控制一个引脚：00=输入, 01=输出, 10=复用, 11=模拟）
 *   +0x04  OTYPER   输出类型（0=推挽, 1=开漏，每 1 bit 控制一个引脚）
 *   +0x08  OSPEEDR  输出速度（每 2 bit：00=2MHz, 01=25MHz, 10=50MHz, 11=100MHz）
 *   +0x0C  PUPDR    上拉/下拉（每 2 bit：00=无, 01=上拉, 10=下拉, 11=保留）
 *   +0x10  IDR      输入数据寄存器（只读，读取引脚当前电平）
 *   +0x14  ODR      输出数据寄存器（读写，设置引脚输出电平）
 *   +0x18  BSRR     位设置/复位寄存器（写 1 置位/复位，原子操作，不影响其他位）
 *   +0x1C  LCKR     配置锁存寄存器（锁定后不可修改 GPIO 配置）
 *   +0x20  AFRL     复用功能选择低（Pin 0~7，每 4 bit 选一个 AF）
 *   +0x24  AFRH     复用功能选择高（Pin 8~15，每 4 bit 选一个 AF）
 *
 * 【本项目使用的 LED 引脚】
 *   PF9  = LED_RED   (DS0 红灯)
 *   PF10 = LED_GREEN (DS1 绿灯)
 *
 * 【MODER 寄存器位域详解】
 *   PF9  的模式位 = bit[19:18]（引脚号 9 × 2 = 18）
 *   PF10 的模式位 = bit[21:20]（引脚号 10 × 2 = 20）
 *   设为输出模式(01)：对应位写入 01
 */

#define GPIOA_BASE        (AHB1_BASE + 0x0000U)
#define GPIOF_BASE        (AHB1_BASE + 0x1400U)

#define GPIOA_MODER       (*(volatile uint32_t *)(GPIOA_BASE + 0x00U))
#define GPIOA_ODR         (*(volatile uint32_t *)(GPIOA_BASE + 0x14U))
#define GPIOA_IDR         (*(volatile uint32_t *)(GPIOA_BASE + 0x10U))

#define GPIOF_MODER       (*(volatile uint32_t *)(GPIOF_BASE + 0x00U))
#define GPIOF_ODR         (*(volatile uint32_t *)(GPIOF_BASE + 0x14U))

/* ============================================================================
 * FLASH — Flash 存储器接口控制器
 * ============================================================================
 *
 * FLASH_ACR 寄存器控制 Flash 读取的等待周期和预取指。
 *
 * 【为什么需要等待周期？】
 *   Flash 是非易失性存储器，读取速度有限。CPU 主频越高，Flash 跟不上。
 *   168MHz 时需要 5 个等待周期（WS=5），即 CPU 每次读 Flash 要等 5 个时钟周期。
 *
 * 【ART Accelerator（自适应实时加速器）】
 *   STM32F4 内置的 Flash 加速模块，通过指令预取和缓存降低等待周期的影响：
 *     bit8  = PRFTEN  预取指使能（Prefetch Enable）
 *     bit9  = ICEN    指令缓存使能（Instruction Cache Enable）
 *     bit10 = DCEN    数据缓存使能（Data Cache Enable）
 *   启用后大部分指令可以 0 等待执行，性能接近 SRAM 运行。
 */

#define FLASHIF_BASE      0x40023C00U
#define FLASH_ACR         (*(volatile uint32_t *)(FLASHIF_BASE + 0x00))

/* ============================================================================
 * SCB — 系统控制块（System Control Block）
 * ============================================================================
 *
 * SCB 是 ARM Cortex-M4 内核的私有外设，不属于 STM32 外设总线。
 * 地址范围：0xE000ED00 ~ 0xE000ED8F（ARM 规定，所有 Cortex-M 芯片相同）
 *
 * 【关键寄存器】
 *   SCB_VTOR  — 向量表偏移寄存器（Vector Table Offset Register）
 *               告诉 CPU 中断向量表放在哪个地址。
 *               默认值 0x00000000（对应 0x08000000，Flash 起始地址）
 *               如果使用了 Bootloader，需要把 VTOR 指向应用程序的向量表地址。
 *
 *   SCB_CPACR — 协处理器访问控制寄存器（Coprocessor Access Control Register）
 *               bit[23:20] 控制 FPU（CP10/CP11）的访问权限。
 *               写入 0b11 = 全访问（用户态+特权态均可使用 FPU 指令）
 *
 *   SCB_SHPR3 — 系统异常优先级寄存器（System Handler Priority Register 3）
 *               控制 SysTick 和 PendSV 的优先级。
 */

#define SCB_BASE          0xE000ED00U
#define SCB_VTOR          (*(volatile uint32_t *)(SCB_BASE + 0x08U))
#define SCB_CPACR         (*(volatile uint32_t *)(SCB_BASE + 0x88U))
#define SCB_SHPR3         (*(volatile uint32_t *)(SCB_BASE + 0xD0U))

/* ============================================================================
 * SysTick — 系统定时器（System Timer）
 * ============================================================================
 *
 * SysTick 是 ARM Cortex-M 内核自带的 24 位倒计时定时器，
 * 所有 Cortex-M 芯片都有，不依赖 STM32 的外设定时器。
 *
 * 【典型用途】
 *   1. RTOS 的任务调度心跳（FreeRTOS 的 tick 中断）
 *   2. 精确延时函数（delay_ms / delay_us）
 *   3. 时间测量（性能分析）
 *
 * 【寄存器】
 *   SYST_CSR (0xE000E010) — 控制和状态寄存器
 *     bit0 = ENABLE   使能 SysTick
 *     bit1 = TICKINT  倒计到 0 时产生中断
 *     bit2 = CLKSOURCE 0=外部时钟(AHB/8), 1=CPU时钟(AHB)
 *     bit16 = COUNTFLAG 倒计到 0 后置 1（读取后自动清零）
 *
 *   SYST_RVR (0xE000E014) — 重载值寄存器
 *     倒计到 0 后自动从此值重新加载（24 位有效，最大 0xFFFFFF = 16777215）
 *
 * 【本项目配置】
 *   时钟源 = CPU 时钟 = 168MHz
 *   RVR = 168000 - 1 → 每 168000 个时钟周期倒计到 0
 *   周期 = 168000 / 168000000 = 0.001s = 1ms
 *   CSR = 0x07 (ENABLE=1, TICKINT=1, CLKSOURCE=1)
 *   → 每 1ms 触发一次 SysTick 中断
 */

#define SYST_CSR          (*(volatile uint32_t *)0xE000E010U)
#define SYST_RVR          (*(volatile uint32_t *)0xE000E014U)

/* ============================================================================
 * PWR — 电源控制（Power Control）
 * ============================================================================
 *
 * PWR_CR 寄存器控制芯片的电源模式和电压调节器。
 *
 * 【本项目中 PWR 的作用】
 *   在配置 PLL 之前，需要将内部电压调节器切换到"高功率模式"，
 *   以确保 168MHz 运行时供电稳定。
 *   bit14 = VOS (Voltage Scale Output)
 *     0 = Scale 2 (1.2V, 最高 144MHz)
 *     1 = Scale 1 (1.3V, 最高 168MHz) ← 本项目使用
 *
 *   PWR 时钟必须先使能（RCC_APB1ENR bit28 = PWREN），才能写 PWR_CR。
 */

#define PWR_BASE          (APB1_BASE + 0x7000U)
#define PWR_CR            (*(volatile uint32_t *)(PWR_BASE + 0x00U))

/* ============================================================================
 * PLL 参数配置
 * ============================================================================
 *
 * PLL（锁相环）将外部晶振的低频信号倍频到 CPU 所需的高频时钟。
 *
 * 【时钟路径】
 *   HSE (8MHz) → /M (÷8) → ×N (×336) → /P (÷2) → SYSCLK (168MHz)
 *                                    → /Q (÷7) → USB OTG FS (48MHz)
 *
 * 【参数说明】
 *   PLL_M = 8    输入分频：8MHz / 8 = 1MHz（VCO 输入频率，要求 1~2MHz）
 *   PLL_N = 336  VCO 倍频：1MHz × 336 = 336MHz（VCO 输出频率，要求 100~432MHz）
 *   PLL_P = 2    主输出分频：336MHz / 2 = 168MHz（SYSCLK，最高 168MHz）
 *   PLL_Q = 7    USB 分频：336MHz / 7 = 48MHz（USB OTG FS 需要 48MHz）
 *
 * 【约束条件（STM32F4 数据手册规定）】
 *   - VCO 输入频率 = HSE / M，必须在 1~2 MHz 之间
 *   - VCO 输出频率 = (HSE / M) × N，必须在 100~432 MHz 之间
 *   - SYSCLK = VCO / P，最高 168 MHz
 *   - USB 时钟 = VCO / Q，必须精确 48 MHz
 */

#define PLL_M             8
#define PLL_N             336
#define PLL_P             2
#define PLL_Q             7

/* ============================================================================
 * SystemInit — 系统时钟初始化
 * ============================================================================
 *
 * 本函数将 STM32F407 的主频从默认的 16MHz（HSI）切换到 168MHz（PLL × HSE）。
 * 在 Reset_Handler 中调用，早于 main() 执行。
 *
 * 【完整执行流程】
 *
 *   ① 使能 FPU（浮点运算单元）
 *      SCB_CPACR |= (3<<20) | (3<<22)
 *      → CP10/CP11 全访问，允许使用浮点指令（VMUL, VADD 等）
 *      → 如果不使能，执行浮点运算会触发 UsageFault
 *
 *   ② 使能 HSI（内部高速时钟）并等待就绪
 *      RCC_CR |= HSION → 等待 HSIRDY
 *      → HSI 是备用时钟，确保在任何情况下都有可靠时钟源
 *
 *   ③ 复位时钟配置寄存器
 *      RCC_CFGR = 0 → 系统时钟切回 HSI
 *      → 清除所有时钟选择和分频配置
 *
 *   ④ 关闭 HSE 和 PLL（准备重新配置）
 *      RCC_CR &= ~(HSEON | PLLON)
 *      → 防止在修改 PLL 参数时 PLL 仍在运行
 *
 *   ⑤ 重置 PLL 配置寄存器
 *      RCC_PLLCFGR = 0x24003010 → 恢复默认值
 *
 *   ⑥ 使能 HSE（外部高速时钟）并等待就绪
 *      RCC_CR |= HSEON → 等待 HSERDY
 *      → HSE 是 8MHz 外部晶振，精度远高于 HSI
 *      → 如果 HSE 启动失败（晶振损坏），时钟将停留在 HSI 16MHz
 *
 *   ⑦ 配置 PLL 参数（HSE → PLL → 168MHz）
 *      a. 使能 PWR 时钟 + 设置电压调节器为高功率模式
 *      b. 配置 AHB/APB 分频系数
 *      c. 写入 PLL 参数（M=8, N=336, P=2, Q=7, HSE 作为 PLL 源）
 *      d. 使能 PLL 并等待就绪
 *
 *   ⑧ 配置 Flash 等待周期
 *      FLASH_ACR = PRFTEN | ICEN | DCEN | 5_WS
 *      → 168MHz 需要 5 个等待周期
 *      → 启用预取指和缓存以弥补等待周期
 *
 *   ⑨ 切换系统时钟到 PLL
 *      RCC_CFGR |= SW_PLL → 等待 SWS_PLL
 *      → 此时 SYSCLK = 168MHz
 *
 *   ⑩ 设置向量表偏移
 *      SCB_VTOR = 0x08000000 → 向量表在 Flash 起始地址
 *
 *   ⑪ 配置 SysTick 定时器
 *      SYST_RVR = 168000-1 → 每 1ms 倒计到 0
 *      SYST_CSR = 0x07 → 使能 + 中断 + CPU 时钟源
 */
static void SystemInit(void) {

    /* --- ① 使能 FPU ---
     * CP10 = bit[21:20], CP11 = bit[23:22]
     * 写入 0b11 = 全访问（用户态和特权态均可使用浮点指令）
     * 不使能 FPU 时，执行 float/double 运算会触发 UsageFault 异常 */
    SCB_CPACR |= (3 << 20) | (3 << 22);

    /* --- ② 使能 HSI 并等待就绪 ---
     * RCC_CR bit0 = HSION（HSI 使能）
     * RCC_CR bit1 = HSIRDY（HSI 就绪标志，硬件自动置位）
     * HSI = 16MHz 内部 RC 振荡器，精度 ±1%，上电默认运行 */
    RCC_CR |= (1 << 0);
    while (!(RCC_CR & (1 << 1))) { }

    /* --- ③ 复位时钟配置 ---
     * 清零 CFGR → 系统时钟切回 HSI，所有分频器恢复默认 */
    RCC_CFGR = 0x00000000;

    /* --- ④ 关闭 HSE 和 PLL ---
     * 0xFEF6FFFF = ~(HSEON | CSSON | PLLON)
     *   bit16 = HSEON  (HSE 使能)
     *   bit19 = CSSON  (时钟安全系统使能)
     *   bit24 = PLLON  (PLL 使能)
     * 修改 PLL 参数前必须先关闭 PLL */
    RCC_CR &= 0xFEF6FFFF;

    /* --- ⑤ 重置 PLL 配置寄存器 ---
     * 0x24003010 是 RCC_PLLCFGR 的复位默认值 */
    RCC_PLLCFGR = 0x24003010;

    /* --- ⑤ 续 关闭 HSE ---
     * 0xFFFBFFFF = ~(HSEON)
     * 再次确保 HSE 关闭（前面已经清除，这里冗余但安全） */
    RCC_CR &= 0xFFFBFFFF;

    /* --- ⑥ 使能 HSE 并等待就绪 ---
     * RCC_CR bit16 = HSEON（HSE 使能）
     * RCC_CR bit17 = HSERDY（HSE 就绪标志）
     * HSE = 8MHz 外部晶振，精度 ±50ppm，远高于 HSI
     * StartUpCounter 超时机制：尝试 0x0500 次后放弃
     *   如果 HSE 启动失败（晶振损坏或未焊接），程序仍可运行（使用 HSI 16MHz） */
    volatile uint32_t StartUpCounter = 0;
    volatile uint32_t HSEStatus = 0;

    RCC_CR |= (1 << 16);
    do {
        HSEStatus = RCC_CR & (1 << 17);
        StartUpCounter++;
    } while ((HSEStatus == 0) && (StartUpCounter != 0x0500));

    if ((RCC_CR & (1 << 17)) != 0) {
        HSEStatus = 0x01;
    } else {
        HSEStatus = 0x00;
    }

    /* --- ⑦ 配置 PLL（仅当 HSE 就绪时执行）--- */
    if (HSEStatus == 0x01) {

        /* ⑦-a 使能 PWR 时钟 + 设置电压调节器为高功率模式
         * RCC_APB1ENR bit28 = PWREN（电源接口时钟使能）
         * PWR_CR bit14 = VOS（电压调节器输出范围）
         *   0 = Scale 2 (1.2V, 最高 144MHz)
         *   1 = Scale 1 (1.3V, 最高 168MHz) ← 需要 168MHz 必须选此 */
        RCC_APB1ENR |= (1 << 28);
        PWR_CR |= (1 << 14);

        /* ⑦-b 配置 AHB/APB 分频系数
         * RCC_CFGR 位域：
         *   bit[7:4]  HPRE  AHB 分频  → 0 = 不分频 → HCLK = 168MHz
         *   bit[12:10] PPRE2 APB2 分频 → 4 = 2分频 → APB2 = 84MHz
         *   bit[15:13] PPRE1 APB1 分频 → 5 = 4分频 → APB1 = 42MHz
         *
         * APB1 最高 42MHz，APB2 最高 84MHz，超过会导致外设工作异常 */
        RCC_CFGR |= (0 << 4);
        RCC_CFGR |= (4 << 13);
        RCC_CFGR |= (5 << 10);

        /* ⑦-c 写入 PLL 参数
         * RCC_PLLCFGR 位域：
         *   bit[5:0]   PLLM = 8     输入分频
         *   bit[14:6]  PLLN = 336   VCO 倍频
         *   bit[17:16] PLLP = (2>>1)-1 = 0  主输出分频（0=P/2, 1=P/4, 2=P/6, 3=P/8）
         *   bit[22]    PLLSRC = 1   PLL 时钟源选择（0=HSI, 1=HSE）
         *   bit[27:24] PLLQ = 7     USB 分频 */
        RCC_PLLCFGR = (PLL_M << 0)
                     | (PLL_N << 6)
                     | (((PLL_P >> 1) - 1) << 16)
                     | (1 << 22)
                     | (PLL_Q << 24);

        /* ⑦-d 使能 PLL 并等待就绪
         * RCC_CR bit24 = PLLON（PLL 使能）
         * RCC_CR bit25 = PLLRDY（PLL 就绪标志）
         * PLL 锁定需要一定时间（几百微秒），必须等待就绪才能切换 */
        RCC_CR |= (1 << 24);
        StartUpCounter = 0;
        while (((RCC_CR & (1 << 25)) == 0) && (StartUpCounter != 0x0500)) {
            StartUpCounter++;
        }

        if ((RCC_CR & (1 << 25)) != 0) {
            /* --- ⑧ 配置 Flash 等待周期 ---
             * FLASH_ACR 位域：
             *   bit[2:0]  LATENCY = 5    等待周期数（168MHz 需要 5 WS）
             *   bit8      PRFTEN = 1     预取指使能
             *   bit9      ICEN = 1       指令缓存使能
             *   bit10     DCEN = 1       数据缓存使能
             *
             * 必须在切换到 PLL 之前配置，否则 Flash 读取会出错 */
            FLASH_ACR = (1 << 8) | (1 << 9) | (1 << 10) | 5;

            /* --- ⑨ 切换系统时钟到 PLL ---
             * RCC_CFGR bit[1:0] = SW（系统时钟切换）
             *   0 = HSI, 1 = HSE, 2 = PLL
             * RCC_CFGR bit[3:2] = SWS（系统时钟状态，硬件自动更新）
             *   0 = HSI, 4 = HSE, 8 = PLL
             * 等待 SWS == 0x08 确认切换完成 */
            RCC_CFGR &= ~3U;
            RCC_CFGR |= 2U;
            StartUpCounter = 0;
            while (((RCC_CFGR & 0x0CU) != 0x08U) && (StartUpCounter != 0x0500)) {
                StartUpCounter++;
            }
        }
    }

    /* --- ⑩ 设置向量表偏移 ---
     * SCB_VTOR = 0x08000000 表示向量表在 Flash 的起始地址
     * 如果使用 Bootloader（从 0x08000000 启动，应用程序从 0x08008000 启动），
     * 则需要设为 0x08008000 */
    SCB_VTOR = 0x08000000U;

    /* --- ⑪ 配置 SysTick 定时器 ---
     * SYST_RVR = 168000 - 1 → 每 168000 个时钟周期倒计到 0
     *   168000 / 168000000Hz = 0.001s = 1ms
     * SYST_CSR = 0x07:
     *   bit0 = 1 (ENABLE)    使能 SysTick
     *   bit1 = 1 (TICKINT)   倒计到 0 时触发 SysTick_Handler 中断
     *   bit2 = 1 (CLKSOURCE) 使用 CPU 时钟 (168MHz) 作为时钟源
     *
     * 注意：SysTick 中断被使能后，必须提供 SysTick_Handler 实现，
     * 否则会进入 Default_Handler 死循环！ */
    SYST_RVR = 168000 - 1;
    SYST_CSR = 0x07;
}

#endif /* __STM32F4XX_H__ */
