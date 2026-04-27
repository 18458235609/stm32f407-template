# ============================================================================
# Makefile — STM32F407VG 项目构建脚本
# ============================================================================
#
# 本 Makefile 实现了编译、链接、烧录、调试的完整自动化流程。
#
# 【Makefile 基础知识】
#   - Makefile 由"规则"组成，格式：目标: 依赖 \n \t 命令
#   - make 默认执行第一个规则（all）
#   - .PHONY 声明的目标不代表真实文件，每次都会重新执行
#   - $@ = 目标名，$< = 第一个依赖，$^ = 所有依赖
#   - $(VAR) 引用变量，:= 即时赋值，= 延迟赋值
#
# 【交叉编译工具链】
#   arm-none-eabi-gcc 是 ARM 专用的 GCC 交叉编译器：
#     arm      = 目标架构（ARM）
#     none     = 无操作系统（裸机）
#     eabi     = 嵌入式应用二进制接口（Embedded ABI）
#   与 PC 上的 gcc 不同，它生成 ARM 机器码而非 x86 机器码。
#
# 【常用命令】
#   make          编译项目
#   make flash    通过 st-flash 烧录
#   make openocd  通过 OpenOCD 烧录
#   make gdb      启动 GDB 调试会话
#   make clean    清理构建产物
#   make size     查看固件各段大小
#   make list     反汇编查看
# ============================================================================

# ============================================================================
# 目标芯片配置
# ============================================================================
# CHIP   = 芯片型号，用于命名输出文件和定义编译宏
# FLASH  = Flash 大小（字节），1MB = 1048576
# SRAM   = SRAM 大小（字节），192KB = 196608
CHIP      = STM32F407VG
FLASH     = 1048576
SRAM      = 196608

# ============================================================================
# 交叉编译工具链
# ============================================================================
# PREFIX   = 工具链前缀，arm-none-eabi- 是 ARM 裸机 GCC 的标准前缀
# CC       = C 编译器（将 .c 编译为 .o）
# OBJCOPY  = 格式转换工具（将 .elf 转为 .bin/.hex）
# OBJDUMP  = 反汇编工具（将 .elf 转为可读的汇编代码）
# SIZE     = 段大小查看工具（显示 .text/.data/.bss 大小）
# GDB      = 调试器（GDB 多架构版本，支持 ARM）
# OPENOCD  = 片上调试器（通过 ST-Link 与芯片通信）
# STFLY    = st-flash 工具（简单的 ST-Link 烧录工具）
PREFIX    = arm-none-eabi-
CC        = $(PREFIX)gcc
OBJCOPY   = $(PREFIX)objcopy
OBJDUMP   = $(PREFIX)objdump
SIZE      = $(PREFIX)size
GDB       = gdb-multiarch
OPENOCD   = openocd
STFLY     = st-flash

# ============================================================================
# 源码与输出路径
# ============================================================================
# BUILD    = 构建输出目录（存放 .o, .elf, .bin, .map）
# SRC_DIR  = C 源文件目录
# INC_DIR  = 头文件目录
# DOC_DIR  = 文档目录
# LD_FILE  = 链接脚本路径
#
# SRC      = 自动收集 src/ 下所有 .c 文件（wildcard 展开通配符）
# OBJ      = 将 SRC 中的 .c 替换为 .o（路径从 src/ 变为 build/）
#            例如：src/main.c → build/main.o
BUILD     = build
SRC_DIR   = src
INC_DIR   = include
DOC_DIR   = doc
LD_FILE   = ld/STM32F407VG.ld

SRC       = $(wildcard $(SRC_DIR)/*.c)
OBJ       = $(SRC:$(SRC_DIR)/%.c=$(BUILD)/%.o)

# ============================================================================
# 编译参数（CFLAGS）
# ============================================================================
#
# 【目标架构参数】
#   -mcpu=cortex-m4      目标 CPU 为 Cortex-M4（STM32F407 的内核）
#   -mthumb              使用 Thumb 指令集（ARM 的 16/32 位混合指令集，
#                        代码密度更高，Cortex-M 系列只支持 Thumb 模式）
#   -mfloat-abi=hard     使用硬件浮点 ABI（函数调用通过 FPU 寄存器传参，
#                        性能最好，但与 soft-fp 不兼容）
#   -mfpu=fpv4-sp-d16    FPU 类型：Cortex-M4 内置的 FPv4-SP，
#                        单精度浮点，16 个 64 位寄存器
#
# 【警告参数】
#   -Wall                启用常见警告（未使用变量、隐式声明等）
#   -Wextra              启用额外警告（更严格的类型检查）
#
# 【优化与段分离参数】
#   -ffunction-sections   每个函数单独一个 section（如 .text.main,
#                         .text.delay_ms），配合 --gc-sections 删除未使用函数
#   -fdata-sections       每个数据单独一个 section，配合 --gc-sections 删除未使用数据
#
# 【调试与宏定义】
#   -g                   生成调试信息（GDB 调试必需，不影响固件大小）
#   -D$(CHIP)            定义宏 STM32F407VG（代码中可用 #ifdef 判断芯片型号）
#   -DHSE_VALUE=8000000  定义外部晶振频率为 8MHz（供 SystemInit 使用）
#
# 【头文件搜索路径】
#   -I$(INC_DIR)         添加 include/ 到头文件搜索路径
#                        （使 #include "stm32f4xx.h" 能找到文件）
CFLAGS    = -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 \
            -Wall -Wextra -ffunction-sections -fdata-sections \
            -g -D$(CHIP) -DHSE_VALUE=8000000 \
            -I$(INC_DIR)

# ============================================================================
# 链接参数（LDFLAGS）
# ============================================================================
#
#   -T$(LD_FILE)         指定链接脚本（告诉链接器内存布局和段分配）
#
#   -Wl,-Map=$(BUILD)/$(CHIP).map
#                        生成 Map 文件（记录每个符号的地址和大小，
#                        调试时非常有用：查看函数地址、段大小等）
#
#   -Wl,--gc-sections    链接时删除未引用的 section（垃圾回收），
#                        配合 -ffunction-sections 和 -fdata-sections 使用，
#                        可以大幅减小固件体积（去掉未使用的函数和变量）
#
#   -mthumb -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16
#                        链接时也必须指定架构参数，否则链接器可能
#                        选择不兼容的运行时库（如 soft-float 版本）
#
#   -lc -lm -lnosys      链接 C 库、数学库、无系统调用库
#                        -lnosys 提供 stub 函数（_write, _read 等），
#                        避免因缺少系统调用而链接失败
LDFLAGS   = -T$(LD_FILE) \
            -Wl,-Map=$(BUILD)/$(CHIP).map,--gc-sections \
            -mthumb -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 \
            -lc -lm -lnosys

# ============================================================================
# OpenOCD 配置
# ============================================================================
# OOCD_IF  = 调试器接口配置（ST-Link V2 驱动）
# OOCD_TGT = 目标芯片配置（STM32F4 系列通用配置）
OOCD_IF   = interface/stlink.cfg
OOCD_TGT  = target/stm32f4x.cfg

# ============================================================================
# 伪目标声明
# ============================================================================
# .PHONY 声明这些目标不对应真实文件，make 不会因为文件已存在而跳过
.PHONY: all flash flash_only openocd gdb size clean

# ============================================================================
# 默认目标：编译整个项目
# ============================================================================
all: $(BUILD)/$(CHIP).elf

# ============================================================================
# 链接规则：.o + 链接脚本 → .elf → .bin
# ============================================================================
#
# 依赖：所有 .o 文件 + 链接脚本
# 步骤：
#   1. 创建 build/ 目录（如果不存在）
#   2. 链接所有 .o 生成 .elf（包含调试信息的完整可执行文件）
#   3. objcopy 将 .elf 转为 .bin（纯二进制，用于 st-flash 烧录）
#   4. size 显示各段大小（.text + .data + .bss）
#
# 【.elf vs .bin 的区别】
#   .elf = 可执行链接格式，包含段信息、符号表、调试信息
#   .bin = 纯二进制，只包含 CPU 要执行的机器码和数据
#   OpenOCD 可以直接烧录 .elf，st-flash 需要烧录 .bin
$(BUILD)/$(CHIP).elf: $(OBJ) $(LD_FILE)
	@mkdir -p $(BUILD)
	$(CC) $(OBJ) $(LDFLAGS) -o $@
	$(OBJCOPY) -O binary $@ $(BUILD)/$(CHIP).bin
	$(SIZE) $@

# ============================================================================
# 编译规则：.c → .o
# ============================================================================
#
# 模式规则：$(BUILD)/%.o 依赖于 $(SRC_DIR)/%.c
# 例如：build/main.o 依赖于 src/main.c
#
# -c  只编译不链接（生成目标文件 .o）
# $<  第一个依赖（.c 文件）
# $@  目标（.o 文件）
#
# | $(BUILD) 表示 order-only 依赖：只在目录不存在时创建，不会导致重新编译
$(BUILD)/%.o: $(SRC_DIR)/%.c | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

# ============================================================================
# 创建构建目录
# ============================================================================
$(BUILD):
	mkdir -p $@

# ============================================================================
# 烧录命令
# ============================================================================

# flash: 先擦除整片 Flash，再写入 .bin 文件
#   st-flash erase              擦除整片 Flash（确保没有残留数据）
#   st-flash write <bin> <addr> 从地址 0x08000000 开始写入
#   0x08000000 = STM32 Flash 的起始地址（硬件固定）
flash: $(BUILD)/$(CHIP).elf
	$(STFLY) erase
	$(STFLY) write $(BUILD)/$(CHIP).bin 0x8000000

# flash_only: 不擦除直接写入（速度更快，但可能残留旧数据）
#   适用于固件大小不变且确定 Flash 已擦除的情况
flash_only: $(BUILD)/$(CHIP).elf
	$(STFLY) write $(BUILD)/$(CHIP).bin 0x8000000

# openocd: 通过 OpenOCD + ST-Link 烧录（推荐方式）
#   -f interface/stlink.cfg   加载 ST-Link 驱动配置
#   -f target/stm32f4x.cfg    加载 STM32F4 目标配置
#   -c "program ... verify reset exit"
#     program  → 烧录 .elf 文件
#     verify   → 烧录后读回比对，确保写入正确
#     reset    → 复位芯片，开始运行新程序
#     exit     → 烧录完成后关闭 OpenOCD
openocd: $(BUILD)/$(CHIP).elf
	sudo $(OPENOCD) -f $(OOCD_IF) -f $(OOCD_TGT) \
		-c "program $(BUILD)/$(CHIP).elf verify reset exit"

# ============================================================================
# GDB 调试
# ============================================================================
#
# 启动流程：
#   1. 后台启动 OpenOCD（作为 GDB 和芯片之间的桥梁）
#   2. 等待 2 秒让 OpenOCD 初始化
#   3. 启动 GDB 并连接到 OpenOCD
#
# GDB 命令：
#   target remote localhost:3333  通过 TCP 连接 OpenOCD（默认端口 3333）
#   load                          将 .elf 加载到芯片 Flash
#   monitor reset halt            复位芯片并暂停（停在 Reset_Handler 第一条指令）
#   break main                    在 main() 设置断点
#
# 常用 GDB 命令：
#   continue (c)    继续运行
#   step (s)        单步进入函数
#   next (n)        单步跳过函数
#   print /x var    以十六进制打印变量
#   info registers  查看所有寄存器
#   x/10x 0x20000000  查看内存（从 0x20000000 开始，10 个 32 位字）
gdb: $(BUILD)/$(CHIP).elf
	@echo "启动 OpenOCD..."
	sudo $(OPENOCD) -f $(OOCD_IF) -f $(OOCD_TGT) &
	@sleep 2
	@echo "连接 GDB..."
	sudo $(GDB) -ex "target remote localhost:3333" \
	         -ex "load" \
	         -ex "monitor reset halt" \
	         -ex "break main" \
	         $(BUILD)/$(CHIP).elf

# ============================================================================
# 辅助工具
# ============================================================================

# size: 显示固件各段大小
#   text  = 代码 + 常量（占 Flash）
#   data  = 已初始化全局变量（占 Flash 存初始值 + 占 SRAM 存运行时值）
#   bss   = 未初始化全局变量（只占 SRAM）
size: $(BUILD)/$(CHIP).elf
	$(SIZE) $(BUILD)/$(CHIP).elf

# clean: 清理构建目录
clean:
	rm -rf $(BUILD)

# list: 反汇编查看
#   -h  显示段头信息（各段的地址、大小、属性）
#   -d  反汇编 .text 段（将机器码转为汇编代码）
#   | less  分页显示（按 q 退出，按 / 搜索）
list:
	$(OBJDUMP) -h $(BUILD)/$(CHIP).elf
	@echo "--- .text disassembly ---"
	$(OBJDUMP) -d $(BUILD)/$(CHIP).elf | less
