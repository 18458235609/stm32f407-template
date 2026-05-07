# FreeRTOS 引入方案

## 目标

在 `stm32f407-template` 裸机项目基础上引入 FreeRTOS Kernel，实现多任务管理。

## 背景

当前项目是纯裸机 Makefile 项目，无操作系统依赖，代码结构轻量。
引入 FreeRTOS 的价值：
- **多任务**：真正并行执行多个功能模块（LED、UART、传感器等）
- **任务调度**：按优先级抢占，实时性好
- **生态成熟**：STM32CubeMX 直接生成 FreeRTOS 代码，后续可与 CubeMX 集成
- **资源可控**：按需裁剪，不需要的功能不编译进来

## 芯片适配信息

| 项目 | 值 |
|------|---|
| 芯片 | STM32F407VG |
| 内核 | Cortex-M4 |
| 主频 | 168 MHz |
| FPU | FPv4-SP hard-float |
| Flash | 1 MB |
| SRAM | 192 KB |
| 工具链 | arm-none-eabi-gcc |

## 实施步骤

### Step 1 — 下载 FreeRTOS Kernel

```bash
cd /home/wings/Work/stm32f407-template
git submodule add https://github.com/FreeRTOS/FreeRTOS-Kernel.git FreeRTOS
```

目录结构：
```
FreeRTOS/
├── Source/
│   ├── include/              # FreeRTOS API 头文件
│   ├── list.c
│   ├── task.c                # 核心：任务管理
│   ├── queue.c               # 核心：队列通信
│   ├── timers.c              # 软件定时器
│   ├── event_groups.c         # 事件组
│   ├── stream_buffer.c       # 流缓冲区
│   └── portable/GCC/ARM_CM4F/  # Cortex-M4 hard-float 移植层
│       ├── port.c
│       └── portmacro.h
└── Quickstone.LICENCE
```

> 不克隆 FreeRTOS 完整仓库（含 demo），只取 kernel，体积小。

### Step 2 — 修改 Makefile

在现有 Makefile 基础上追加 FreeRTOS 相关内容：

```makefile
# ===== FreeRTOS 配置 =====
FREERTOS_DIR  = FreeRTOS
FREERTOS_SRC = $(FREERTOS_DIR)/Source
FREERTOS_PORT = $(FREERTOS_SRC)/portable/GCC/ARM_CM4F

# FreeRTOS 源文件（核心模块 + 移植层）
SRC_FREERTOS = $(wildcard $(FREERTOS_SRC)/*.c) \
               $(wildcard $(FREERTOS_PORT)/*.c)

# FreeRTOS 头文件搜索路径
CFLAGS += -I$(FREERTOS_SRC)/include \
          -I$(FREERTOS_PORT) \
          -I$(FREERTOS_DIR)

# FreeRTOS 编译宏（CubeMX 生成风格）
CFLAGS += -DconfigUSE_PREEMPTION=1 \
          -DconfigUSE_IDLE_HOOK=0 \
          -DconfigUSE_TICK_HOOK=0 \
          -DconfigCPU_CLOCK_HZ=168000000 \
          -DconfigTICK_RATE_HZ=1000 \
          -DconfigMAX_PRIORITIES=5 \
          -DconfigMINIMAL_STACK_SIZE=128 \
          -DconfigTOTAL_HEAP_SIZE=4096 \
          -DconfigSUPPORT_DYNAMIC_ALLOCATION=1 \
          -DconfigGENERATE_RUN_TIME_STATS=0 \
          -DconfigUSE_APPLICATION_TASK_TAG=0 \
          -DconfigENFORCE_SYSTEM_CALLS_FROM_KERNEL_ONLY=1
```

在 Makefile 的 OBJ 变量中追加 FreeRTOS 源文件编译规则：

```makefile
# 在 $(OBJ) 定义后追加
OBJ_FREERTOS = $(SRC_FREERTOS:$(FREERTOS_SRC)/%.c=$(BUILD)/%.o)
OBJ += $(OBJ_FREERTOS)
```

链接时添加内存分配相关 C 库：

```makefile
# 在 LDFLAGS 末尾追加（FreeRTOS 动态分配需要）
LDFLAGS += -Wl,--default-exec-stack=no
```

### Step 3 — 创建 FreeRTOSConfig.h

路径：`include/FreeRTOSConfig.h`

```c
#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/* 头文件 */
#include "stm32f4xx.h"  /* 获取 SystemCoreClock 定义 */

/* 调度器配置 */
#define configUSE_PREEMPTION              1
#define configUSE_IDLE_HOOK               0
#define configUSE_TICK_HOOK               0
#define configTICK_RATE_HZ               1000
#define configCPU_CLOCK_HZ                SystemCoreClock  /* 168000000 */

/* 内存配置 */
#define configTOTAL_HEAP_SIZE             4096
#define configSUPPORT_DYNAMIC_ALLOCATION   1
#define configMINIMAL_STACK_SIZE           128  /*Words*/

/* 任务配置 */
#define configMAX_PRIORITIES              5
#define configUSE_APPLICATION_TASK_TAG     0

/* 系统调用保护（Cortex-M4 hard-float 必须开启）*/
#define configENFORCE_SYSTEM_CALLS_FROM_KERNEL_ONLY  1

/* 中断配置 */
#define configMAX_SYSCALL_INTERRUPT_PRIORITY     5
#define configASSERT(expr)  ((expr) ? (void)0 : vAssertCalled(__FILE__, __LINE__))

/* 编译器相关 */
void vAssertCalled(const char *file, int line);
#define pdTASK_CODE     void

#endif /* FREERTOS_CONFIG_H */
```

### Step 4 — 实现 vAssertCalled

在 `src/system.c`（或新建 `src/assert.c`）中：

```c
#include <stdio.h>
void vAssertCalled(const char *file, int line) {
    (void)file; (void)line;
    while (1);
}
```

### Step 5 — 修改 main.c

裸机主函数重构为多任务结构：

```c
#include "FreeRTOS.h"
#include "task.h"

/* 任务句柄 */
static TaskHandle_t hLEDTask = NULL;
static TaskHandle_t hUartTask = NULL;

/* LED 任务 */
void vTaskLED(void *pvParams) {
    (void)pvParams;
    while (1) {
        HAL_GPIO_TogglePin(GPIOF, GPIO_PIN_9);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

/* UART 任务 */
void vTaskUart(void *pvParams) {
    (void)pvParams;
    while (1) {
        // UART 处理逻辑
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

int main(void) {
    /* 硬件初始化（保留原有裸机代码） */
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();

    /* 创建任务 */
    xTaskCreate(vTaskLED, "LED", 128, NULL, 2, &hLEDTask);
    xTaskCreate(vTaskUart, "UART", 128, NULL, 1, &hUartTask);

    /* 启动调度器 */
    vTaskStartScheduler();

    /* 正常不会到达这里 */
    while (1);
}
```

### Step 6 — 编译验证

```bash
make clean && make
```

预期：无编译错误，无链接错误（hard-float + ARM_CM4F 移植层已匹配）

---

## 文件变更清单

| 操作 | 文件 |
|------|------|
| 新增 | `FreeRTOS/` (git submodule) |
| 新增 | `include/FreeRTOSConfig.h` |
| 新增 | `src/assert.c` |
| 修改 | `Makefile` |
| 修改 | `src/main.c` |

## 风险与注意事项

| 风险 | 缓解措施 |
|------|---------|
| heap 不足导致硬故障 | 从 4KB 起，逐步调大至稳定 |
| 任务栈溢出 | `configMINIMAL_STACK_SIZE` × 4（Words→字节） |
| 中断优先级与 SysCall 冲突 | `configMAX_SYSCALL_INTERRUPT_PRIORITY ≤ 5`（数值越小优先级越高，Cortex-M 需注意） |
| 裸机 delay 与 RTOS 混用 | 任务内统一用 `vTaskDelay`，裸机部分保留 `HAL_Delay` |
| printf/semihosting | 去掉裸机的 semihosting，RTOS 下用 ITM 或 UART 实现调试输出 |
