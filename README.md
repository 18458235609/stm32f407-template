# STM32F407VG 最小 Makefile 模板

ARM Cortex-M4 bare-metal 项目模板，基于 Makefile 构建，使用 OpenOCD + ST-Link 烧录。

## 目标芯片

- **STM32F407VGT6**（1MB Flash / 192KB SRAM / 168MHz）
- 开发板：神舟号系列（配套 IO 引脚分配表）

## 快速开始

### 编译

```bash
make          # 编译
make size     # 查看段大小
make clean    # 清理
```

### 烧录

```bash
make boot     # 通过 OpenOCD 烧录并运行
make reset    # 复位
make debug    # 启动 OpenOCD + GDB 调试
```

### 硬件连接

```
ST-Link V2  -->  STM32F407VG
-------------------------
SWCLK        -->  SWCLK
SWDIO        -->  SWDIO
GND          -->  GND
3.3V         -->  3.3V
```

## 项目结构

```
stm32f407-template/
├── Makefile              # 编译/烧录/调试入口
├── ld/
│   └── STM32F407VG.ld     # 链接脚本
├── src/
│   └── main.c             # 应用入口（含中断向量表）
├── include/
│   ├── stm32f4xx.h        # 最小外设寄存器定义 + PLL 配置
│   └── pindef.h           # 全部引脚定义 + 快速别名
└── doc/
    └── pinout.md          # 引脚速查表
```

## 引脚快速别名（pindef.h）

```c
// LED
LED_RED    // PF9
LED_GREEN  // PF10
BEEP       // PF6

// 按键（低电平按下）
KEY0       // PE4
KEY1       // PE3
KEY2       // PE2
KEY_UP     // PA0（高电平按下，有上拉）

// 串口
USART1_TX  // PA9
USART1_RX  // PA10
```

## 工具链

- `arm-none-eabi-gcc` — 编译
- `openocd` — 烧录/调试
- `stlink-tools` — 可选烧录工具

## 编译参数

| 参数 | 值 |
|------|-----|
| CPU | Cortex-M4 |
| FPU | FPv4-SP-D16（硬浮点）|
| 优化 | `-O0`（默认，可改）|
| HSE | 8 MHz（外置晶振）|
| PLL | 168 MHz（主频）|

## 文档

- 全部引脚定义见 [`doc/pinout.md`](doc/pinout.md)
- 寄存器级别操作见 [`include/stm32f4xx.h`](include/stm32f4xx.h)
