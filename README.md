# STM32F407VG 零基础入门指南

> 本项目旨在通过最少的代码量，完整呈现 STM32 从上电到运行的全过程。
> 无任何第三方库，不依赖 HAL/LL/CubeMX，仅用 Makefile + C 代码，让你真正理解嵌入式底层。

---

## 目录

1. [硬件概述](#1-硬件概述)
2. [存储器映射与地址空间](#2-存储器映射与地址空间)
3. [时钟系统：如何跑满 168 MHz](#3-时钟系统如何跑满-168-mhz)
4. [GPIO 输出：点亮 LED 的原理](#4-gpio-输出点亮-led-的原理)
5. [中断向量表：程序的心脏](#5-中断向量表程序的心脏)
6. [启动流程：上电后发生了什么](#6-启动流程上电后发生了什么)
7. [链接脚本：数据和代码放在哪](#7-链接脚本数据和代码放在哪)
8. [Makefile：自动化构建](#8-makefile自动化构建)
9. [OpenOCD + ST-Link：烧录与调试](#9-openocd--st-link烧录与调试)
10. [扩展指南](#10-扩展指南)

---

## 1. 硬件概述

### 芯片型号解析

```
STM 32 F 407 V G T 6
 │  │   │   │  │ │ │
 │  │   │   │  │ │ └─── T: 温度范围 (-40~85°C)
 │  │   │   │  │ └──── G: 封装（LQFP100）
 │  │   │   │  └────── V: Flash 1MB / SRAM 192KB
 │  │   │   └───────── 407: 高性能系列（Cortex-M4 + DSP + FPU）
 │  │   └──────────── F: 通用类型
 │  └──────────────── 32: 32位 ARM Cortex-M
 └─────────────────── ST: 意法半导体
```

### 本项目硬件配置

| 参数 | 值 | 说明 |
|------|-----|------|
| MCU | STM32F407VGT6 | LQFP100 封装 |
| 主频 | 168 MHz | Cortex-M4 @ 168MHz |
| Flash | 1 MB | 0x08000000 ~ 0x080FFFFF |
| SRAM | 192 KB | 0x20000000 ~ 0x2002FFFF |
| 调试接口 | SWD | 只需 SWDIO + SWDCLK 两根线 |
| 开发板 | 神舟号 | 配套引脚分配表见 `doc/pinout.md` |

---

## 2. 存储器映射与地址空间

STM32 采用统一的 4GB 地址空间（32位寻址），通过 APB/AHB 总线连接各类外设：

```
0x0000_0000 ─────────────────────────  Flash（主存储）  128KB alias（启动重映射）
0x0800_0000 ─────────────────────────  Flash（实际地址）  1MB @ STM32F407VG
0x1FFF_0000 ─────────────────────────  System Memory（ROM）  30KB（ST 出厂 Bootloader）
0x1FFF_7800 ─────────────────────────  Option Bytes
0x2000_0000 ─────────────────────────  SRAM  192KB
0x4000_0000 ─────────────────────────  APB Peripherals（低速）  约960MB
0x4001_0000 ─────────────────────────  APB2 Peripherals（高速）  约960MB
0x4002_0000 ─────────────────────────  AHB1 Peripherals  GPIO / RCC / ...
0x4002_3000 ─────────────────────────  AHB2 Peripherals
0x4002_3C00 ─────────────────────────  Flash Controller（FLASH_ACR）
0xE000_E000 ─────────────────────────  Core Peripherals（Cortex-M 私有）  SysTick / NVIC / SCB
```

### 关键地址解析

```
PERIPH_BASE = 0x4000_0000          所有外设的起始地址（基地址）

AHB1_BASE   = PERIPH_BASE + 0x0002_0000 = 0x4002_0000
RCC_BASE    = AHB1_BASE + 0x1000          = 0x4002_1000   （时钟控制）
GPIOX_BASE  = AHB1_BASE + 0x0000_xxxx    = AHB1 + 端口偏移

GPIOA_BASE  = 0x4002_0000  (AHB1_BASE + 0x0000)
GPIOB_BASE  = 0x4002_0400  (AHB1_BASE + 0x0400)
...
GPIOF_BASE  = 0x4002_1400  (AHB1_BASE + 0x1400)
```

> **为什么要这样设计？** ARM 采用标准化的总线协议（AHB/APB），所有外设挂载在同一组地址总线上，通过不同的基地址区分。这与 PC 上访问不同硬件设备（南桥/北桥）类似。

---

## 3. 时钟系统：如何跑满 168 MHz

STM32F4 的时钟系统是理解嵌入式运行原理的核心。

### 3.1 两种时钟源

| 时钟源 | 频率 | 用途 | 精度 |
|--------|------|------|------|
| **HSI** | 16 MHz | 备用/低成本方案 | ±1%（工厂校准） |
| **HSE** | 8 MHz（外置晶振）| 标准方案（本项目使用）| 取决于晶振，通常 ±50ppm |

### 3.2 PLL 倍频原理

PLL（锁相环）= 振荡器 × N ÷ M ÷ P，通过数学变换将低频升到高频：

```
         ┌─────────┐
HSE ────►│   ÷ M   │────► (VCO_in) ──► (乘法N) ──► (÷ P) ──► SYSCLK
8MHz    └─────────┘     = 1MHz              ×168     ÷2      = 168MHz
```

### 3.3 本项目 PLL 参数

```c
HSE_VALUE = 8_000_000 Hz（外置晶振频率）
PLL_M = 8      → VCO输入 = 8MHz / 8 = 1 MHz
PLL_N = 168    → VCO输出 = 1 MHz × 168 = 168 MHz
PLL_P = 2      → SYSCLK   = 168 MHz / 2 = er**实际为 P=2 对应 168MHz**  
// 注：PLL_P=2 时寄存器写 (P>>1)=1，输出 168MHz
//     PLL_P=4 时寄存器写 (P>>1)=2，输出 84MHz（常用于超频测试）
```

### 3.4 为什么需要 Flash 等待周期？

CPU 主频越高，从 Flash 读取指令越跟不上（Flash 本身有物理延迟）：

| SYSCLK | 需要等待周期数 | 配置 |
|--------|------------|------|
| 0 ~ 30 MHz | 0 | 几乎无延迟 |
| 30 ~ 60 MHz | 1 | 等 1 个 HCLK 周期 |
| 60 ~ 90 MHz | 2 | |
| **168 MHz** | **5** | 本项目配置 |

> **ART Accelerator**：STM32F4 的 Flash 加速模块，启用后可预取指令，降低等待周期影响。

### 3.5 SysTick 定时器

ARM Cortex-M 提供了一个内置的 24 位倒计时定时器 SysTick，常用于精确延时和 RTOS 心跳：

```c
// 本项目配置：168MHz / 168 = 1MHz，每 1μs 减 1
// SysTick_RVR = 168000 - 1 → 每 1ms 触发一次 SysTick 中断
SYST_RVR = 168000 - 1;  // 168MHz / 168 = 1kHz = 1ms 周期
SYST_CSR = 0x07;         // bit0=ENABLE, bit1=TICKINT, bit2=CLKSOURCE
```

---

## 4. GPIO 输出：点亮 LED 的原理

### 4.1 GPIO 结构

每个 GPIO 端口有 11 个寄存器（最常用 4 个）：

```
GPIOx_MODER   — 模式寄存器（输入/输出/复用/模拟）
GPIOx_OTYPER  — 输出类型（推挽/开漏）
GPIOx_OSPEEDR — 速度（2/25/50/100MHz）
GPIOx_PUPDR   — 上拉/下拉
GPIOx_IDR     — 输入数据寄存器（读取引脚电平）
GPIOx_ODR     — 输出数据寄存器（设置引脚电平）★ 最常用
GPIOx_BSRR    — 位设置/复位寄存器（原子操作，避免竞争）
GPIOx_LCKR    — 锁存配置（冻结当前 GPIO 配置）
GPIOx_AFRL/H  — 复用功能选择
```

### 4.2 MODER 寄存器（每 2 bits 控制一个引脚）

```
bit[2n+1 : 2n] → 引脚 n 的模式
  00 = 输入（默认，复位后即为此模式）
  01 = 通用输出
  10 = 复用功能
  11 = 模拟（ADC/DAC）
```

### 4.3 点亮 LED 的操作顺序

**LED 接法（典型）：**
```
MCU Pin ─────┤LED├──── GND
（输出高）    （亮）   （低）
```

**操作步骤：**
1. **使能 GPIO 时钟**：RCC_AHB1ENR |= 1 << port_bit（STM32 所有外设时钟默认关闭，省电）
2. **配置模式**：GPIOx_MODER 设为输出（01）
3. **输出数据**：GPIOx_ODR 写 1（高电平=亮）或 0（低电平=灭）

### 4.4 本项目的 LED 硬件

| LED | 引脚 | 连接方式 |
|-----|------|---------|
| LED_RED | PF9 | 推挽输出，默认灭（ODR=0） |
| LED_GREEN | PF10 | 推挽输出，默认灭（ODR=0） |

> **硬件知识**：LED 正向压降约 1.8~3.3V（红光最低，绿/蓝光较高）。STM32 GPIO 输出高电平约 3.3V，足够点亮普通 LED，无需额外限流电阻（板子上通常已串电阻）。

---

## 5. 中断向量表：程序的心脏

### 5.1 什么是中断向量表？

中断向量表是芯片启动时最先读取的一张"函数地址列表"。每个中断/异常编号对应一个处理函数的入口地址，CPU 根据当前发生的事件类型，查询这张表并跳转执行。

```
地址              编号   含义
─────────────────────────────────────────────
0x0800_0000       [0]   初始 SP（Stack Pointer，栈顶地址）
0x0800_0004       [1]   Reset_Handler（复位向量） ★ 程序入口
0x0800_0008       [2]   NMI_Handler
0x0800_000C       [3]   HardFault_Handler
...               ...
0x0800_003C       [14]  SysTick_Handler
```

> **重要**：在 Flash 的起始位置（0x08000000），CPU 上电后自动从这里取指执行。向量表就放在这里。

### 5.2 本项目的向量表

```c
void (* const vectors[])(void) __attribute__((section(".isr_vector"))) = {
    [0]  = (void (*)(void))0x20030000,   // 初始 SP = SRAM 最高地址 + 1
    [1]  = Reset_Handler,                 // ★ 复位向量 = 程序真正入口
    [2]  = NMI_Handler,
    [3]  = HardFault_Handler,
    // ...
};
```

- `__attribute__((section(".isr_vector")))` 让链接器把这个数组放到 `.isr_vector` 段
- 链接脚本里用 `KEEP(*(.isr_vector))` 确保这个段不被链接器优化删除
- `KEEP` 非常重要：没有它，链接器可能把向量表当成"未使用的函数"而丢弃

### 5.3 异常与中断的区别

| 类型 | 特点 | 数量 |
|------|------|------|
| **异常（Exception）** | ARM Core 内部产生（除零/访问违规/复位）| 15 个 |
| **中断（Interrupt）** | 外设通过 NVIC 触发 | 最多 82 个（STM32F4）|

### 5.4 `__attribute__((alias()))` 技巧

```c
void Default_Handler(void) { while (1) {} }  // 默认死循环
void NMI_Handler(void)  __attribute__((alias("Default_Handler")));
```

这里让编译器把 `NMI_Handler` 这个符号映射到 `Default_Handler` 的地址：
- 优点：不需要写很多个相同的死循环函数，节省代码量
- 缺点：调试时无法区分是哪个中断触发的（都停在 Default_Handler）

---

## 6. 启动流程：上电后发生了什么

### 6.1 完整执行序列

```
① 上电 / 复位
     │
② 芯片内部 Bootloader（可选，BOOT0 引脚决定）
     │  BOOT0=0 → 从 Flash 启动（正常使用时选这个）
     │  BOOT0=1 → 从 SRAM 启动（用于串口下载）
     │
③ 取 PC = vectors[1] = Reset_Handler
     │
④ 执行 Reset_Handler:
     │
   ④-a 复制 .data 段（全局/静态初始化变量）
       for (dest = _sdata; dest < _edata; ) *dest++ = *src++;
       说明：这些变量初始值存储在 Flash，但运行在 SRAM
       例：int flag = 5;  →  flag 的值 5 存在 Flash，
                            运行时复制到 SRAM 的 flag 变量
     │
   ④-b 清零 .bss 段（未初始化的全局/静态变量）
       for (dest = _sbss; dest < _ebss; ) *dest++ = 0;
       说明：C 标准要求未初始化变量默认为 0
       例：int counter;   →  自动被初始化为 0
     │
   ④-c 调用 SystemInit() → 配置 PLL 时钟为 168MHz
     │
   ④-d 调用 main() → ★ 进入用户代码，永不返回
     │
   ④-e main() 退出后，死循环
```

### 6.2 为什么需要 .data 和 .bss？

计算机程序的全局变量有两大类：

| 段名 | 变量示例 | 初始值存储位置 | 运行时位置 |
|------|---------|--------------|-----------|
| `.data` | `int flag = 5;` | Flash（紧跟在代码后面）| SRAM |
| `.bss` | `int counter;` | **不占 Flash**（只记录地址范围）| SRAM（运行时清零）|

> **为什么要区分？** BSS 段不存储初始值，可以节省大量 Flash 空间（一个 1MB 的程序可能只有几十字节的 BSS）。

### 6.3 `volatile` 关键字

```c
volatile uint32_t *ptr = ...;
volatile uint32_t var;
```

- `volatile` = 告诉编译器"这个值随时可能变，不要优化掉我对它的读写"
- 用在指针：外设寄存器随时可能被硬件改变
- 用在变量：多线程/SysTick 中断中共享的变量

---

## 7. 链接脚本：数据和代码放在哪

### 7.1 链接脚本的作用

链接脚本（Linker Script）告诉链接器：
- **内存区域**：Flash 和 SRAM 的地址范围
- **Section 映射**：每个代码/数据段放到哪块内存
- **符号定义**：提供 `_estack`、`_sidata` 等运行时地址

### 7.2 内存区域定义

```ld
MEMORY
{
    FLASH (rx) : ORIGIN = 0x08000000, LENGTH = 0x100000   /* 1MB */
    SRAM (rwx) : ORIGIN = 0x20000000, LENGTH = 0x30000    /* 192KB */
}
```

- `rx` = Read-Only eXecutable（代码和常量放这里）
- `rwx` = Read/Write/eXecutable（数据放这里）

### 7.3 关键 section 解析

```ld
/* .isr_vector：中断向量表，必须放在 Flash 最开头（地址 0）*/
.isr_vector : { KEEP(*(.isr_vector)) } >FLASH

/* .text：所有代码和常量 */
.text : { *(.text) *(.rodata) } >FLASH

/* .data：初始化过的全局变量
   AT>FLASH 表示"值存储在 Flash，但运行时复制到 SRAM"  */
.data : { ... } >SRAM AT>FLASH

/* _sidata = .data 的初始值在 Flash 中的位置（由 AT>FLASH 决定）*/
_sidata = LOADADDR(.data);

/* .bss：未初始化的全局变量，运行时不占 Flash */
.bss : { ... } >SRAM
```

### 7.4 `>SRAM AT>FLASH` 详解

这是嵌入式开发最重要的概念之一：

```
Flash 布局（烧录时）:
  [vectors][text][rodata][ .data初始值 ]
                      ↑_sidata 指向这里

SRAM 布局（运行时）:
  [vectors(不需要)][text(不需要)][ .data(运行时) ][ .bss ]
                                            ↑_sbss ~ _ebss
```

上电后 Reset_Handler 执行：
```c
unsigned long *src  = &_sidata;  // Flash 地址（存的是初始值）
unsigned long *dest = &_sdata;   // SRAM 地址（变量本身）
while (dest < &_edata) *dest++ = *src++;  // 复制
```

---

## 8. Makefile：自动化构建

### 8.1 编译参数解释

```makefile
CFLAGS = -mcpu=cortex-m4          # 目标 CPU 架构
         -mthumb                   # 使用 Thumb 指令集（STM32 必须）
         -mfloat-abi=hard          # 硬浮点 ABI（使用 FPU 指令）
         -mfpu=fpv4-sp-d16         # FPU 类型：Cortex-M4 FPU
         -Wall -Wextra             # 开启所有警告
         -ffunction-sections       # 每个函数单独一个 section（用于 gc-sections）
         -fdata-sections           # 每个变量单独一个 section
         -D$(CHIP)                 # 定义宏 -DSTM32F407VG
         -DHSE_VALUE=8000000       # 外部晶振频率声明（供 stm32f4xx.h 使用）
         -I$(INC_DIR)              # 头文件搜索路径
```

> **为什么要 `-ffunction-sections`？** 链接时使用 `--gc-sections` 可以删除未使用的函数，减小固件体积。

### 8.2 链接参数解释

```makefile
LDFLAGS = -T$(LD_FILE)             # 指定链接脚本
          -Wl,-Map=$(BUILD)/$(CHIP).map  # 生成 Map 文件（查看符号地址）
          --gc-sections            # 删除未使用段（减小固件）
          -mthumb -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16
          # ↑ 必须与 CFLAGS 一致，否则链接时 FPU 指令不匹配
```

### 8.3 常用命令

```bash
make          # 编译整个项目
make size     # 查看固件各段大小（.text / .data / .bss）
make openocd  # 编译并通过 OpenOCD 烧录
make flash    # 通过 st-flash 烧录
make gdb      # 启动 GDB 调试
make clean    # 清理 build/ 目录
make list     # 反汇编查看
```

---

## 9. OpenOCD + ST-Link：烧录与调试

### 9.1 SWD 接口原理

SWD（Serial Wire Debug）是 ARM 提出的两线调试协议：

```
┌─────────┐       SWDIO (双向)      ┌──────────────┐
│ ST-Link │◄──────────────────────►│   STM32      │
│   V2    │       SWDCLK (时钟)    │  (Target)    │
│         │◄──────────────────────►│              │
└─────────┘       GND               └──────────────┘
```

- **SWDIO**：双向数据线（命令 + 数据）
- **SWDCLK**：时钟线，由 ST-Link 提供（1~10 MHz）
- **仅需 4 根线：SWDIO + SWDCLK + GND + 3.3V**

### 9.2 OpenOCD 工作流程

```
OpenOCD（PC）                    芯片
─────────────────                 ────────
连接 ST-Link
  │
  ▼
打开 /dev/ttyUSB0 (USB)
  │
  ▼
发送命令重置芯片 ←→ SWD
  │
  ▼
Halt（暂停 CPU）←→ SWD
  │
  ▼
Erase Flash
  │
  ▼
Program: 按地址写入 Flash ←→ SWD（每笔 4~8 字节）
  │
  ▼
Verify: 读回比对
  │
  ▼
Reset: 重新运行 ←→ SWD
```

### 9.3 烧录命令解析

```bash
sudo openocd \
    -f interface/stlink.cfg    # ST-Link 驱动配置（V2 兼容）
    -f target/stm32f4x.cfg      # STM32F4 系列目标配置
    -c "program build/STM32F407VG.elf verify reset exit"
                                 # program  → 烧录
                                 # verify  → 烧录后验证
                                 # reset   → 复位到正常运行模式
                                 # exit    → 烧录完成后关闭 OpenOCD
```

### 9.4 常见问题排查

| 问题 | 原因 | 解决 |
|------|------|------|
| `Unable to match requested speed` | 目标不支持该 SWD 频率 | 降低 kHz，或忽略（自动降频）|
| `device id = 0x10076413` 看不到 | 线太长/电磁干扰 | 缩短 SWD 线 |
| `Verify OK` 但程序不运行 | 代码有 HardFault | 用 GDB 单步调试 |
| `JTAG/SWD communication error` | 芯片被 JTAG 禁用 | 检查 BOOT0/BOOT1 跳线 |

---

## 10. 扩展指南

### 10.1 添加新外设的步骤

以添加 USART1（串口）为例：

```c
// 1. 在 stm32f4xx.h 添加寄存器
#define USART1_BASE   (APB2_BASE + 0x1000)
#define USART1_SR     (*(volatile uint32_t *)(USART1_BASE + 0x00))
#define USART1_DR     (*(volatile uint32_t *)(USART1_BASE + 0x04))
#define USART1_BRR    (*(volatile uint32_t *)(USART1_BASE + 0x08))
#define USART1_CR1    (*(volatile uint32_t *)(USART1_BASE + 0x0C))

// 2. 使能时钟：RCC_APB2ENR |= 1 << 4（USART1EN）

// 3. 配置 GPIO 复用：PA9=TX(AF7), PA10=RX(AF7)
//    GPIOA_AFRH = (7 << 4) | (7 << 8)

// 4. 设置波特率：115200 @ 16MHz → BRR = 16MHz/115200 ≈ 0x8B

// 5. 使能 USART：USART1_CR1 |= (1 << 13)
```

### 10.2 常用调试技巧

**查看寄存器值（OpenOCD）：**
```
arm-none-eabi-gdb build/STM32F407VG.elf
(gdb) target remote localhost:3333
(gdb) monitor mdw 0x40020000 10   # 读取 10 个 32 位字（GPIOF 寄存器）
(gdb) monitor mdw 0x4002_1400 10
(gdb) print /x GPIOF_ODR         # 打印 ODR 当前值
```

**使用 st-flash（不需要配置文件）：**
```bash
st-flash --reset write build/STM32F407VG.elf 0x8000000
```

### 10.3 学习资源推荐

| 资源 | 说明 |
|------|------|
| [STM32F4xx Reference Manual](https://www.st.com/resource/en/reference_manual/dm00031020.pdf) | 最权威的芯片手册（英文，2000+页）|
| [ARM Cortex-M4 Generic User Guide](https://developer.arm.com/documentation/den0013/latest) | ARM 官方内核文档 |
| [OpenOCD Official Docs](http://openocd.org/doc/html/) | OpenOCD 使用手册 |
| 《ARM Cortex-M3/M4 权威指南》 | 宋俊德 / 李悦 / 阳春 | 中文入门必读 |

### 10.4 下一步建议

1. **添加 SysTick 中断**：实现精确 ms 延时（不用 busy wait）
2. **添加 USART**：printf 重定向到串口，实现日志输出
3. **添加定时器**：TIM3 定时器中断（替代 SysTick 做多任务）
4. **添加 DMA**：实现 UART 高速不占用 CPU 的数据收发

---

## 项目结构

```
stm32f407-template/
├── Makefile                 # 编译/烧录/调试入口
├── README.md                # ★ 学习指南（本文件）
├── ld/
│   └── STM32F407VG.ld       # 链接脚本（内存布局 + 段定义）
├── src/
│   └── main.c               # ★ 应用入口 + 中断向量表
├── include/
│   ├── stm32f4xx.h          # ★ 寄存器定义 + SystemInit (PLL)
│   └── pindef.h             # 引脚分配表 + 快速别名
└── doc/
    └── pinout.md            # 开发板 IO 速查表
```

## 快速命令

```bash
# 编译
make

# 烧录（推荐）
make openocd

# 清理
make clean

# GDB 调试
make gdb
# 在 gdb 里输入：
#   target remote localhost:3333
#   load
#   monitor reset halt
#   break main
#   continue
```
