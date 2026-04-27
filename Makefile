# ============ 目标芯片 ============
CHIP      = STM32F407VG
FLASH     = 1048576
SRAM      = 196608

# ============ 工具链 ============
PREFIX    = arm-none-eabi-
CC        = $(PREFIX)gcc
OBJCOPY   = $(PREFIX)objcopy
OBJDUMP   = $(PREFIX)objdump
SIZE      = $(PREFIX)size
GDB       = gdb-multiarch
OPENOCD   = openocd
STFLY     = st-flash

# ============ 源码 & 输出 ============
BUILD     = build
SRC_DIR   = src
INC_DIR   = include
DOC_DIR   = doc
LD_FILE   = ld/STM32F407VG.ld

SRC       = $(wildcard $(SRC_DIR)/*.c)
OBJ       = $(SRC:$(SRC_DIR)/%.c=$(BUILD)/%.o)

# ============ 编译参数 ============
CFLAGS    = -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 \
            -Wall -Wextra -ffunction-sections -fdata-sections \
            -g -D$(CHIP) -DHSE_VALUE=8000000 \
            -I$(INC_DIR)
LDFLAGS   = -T$(LD_FILE) \
            -Wl,-Map=$(BUILD)/$(CHIP).map,--gc-sections \
            -mthumb -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 \
            -lc -lm -lnosys

# ============ 烧录用 OpenOCD ============
OOCD_IF   = interface/stlink.cfg
OOCD_TGT  = target/stm32f4x.cfg

.PHONY: all flash flash_only openocd gdb size clean

all: $(BUILD)/$(CHIP).elf

$(BUILD)/$(CHIP).elf: $(OBJ) $(LD_FILE)
	@mkdir -p $(BUILD)
	$(CC) $(OBJ) $(LDFLAGS) -o $@
	$(OBJCOPY) -O binary $@ $(BUILD)/$(CHIP).bin
	$(SIZE) $@

$(BUILD)/%.o: $(SRC_DIR)/%.c | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD):
	mkdir -p $@

# ============ 烧录 ============
flash: $(BUILD)/$(CHIP).elf
	$(STFLY) erase
	$(STFLY) write $(BUILD)/$(CHIP).bin 0x8000000

flash_only: $(BUILD)/$(CHIP).elf
	$(STFLY) write $(BUILD)/$(CHIP).bin 0x8000000

openocd: $(BUILD)/$(CHIP).elf
	sudo $(OPENOCD) -f $(OOCD_IF) -f $(OOCD_TGT) \
		-c "program $(BUILD)/$(CHIP).elf verify reset exit"

# ============ GDB 调试 ============
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

# ============ 工具 ============
size: $(BUILD)/$(CHIP).elf
	$(SIZE) $(BUILD)/$(CHIP).elf

clean:
	rm -rf $(BUILD)

list:
	$(OBJDUMP) -h $(BUILD)/$(CHIP).elf
	@echo "--- .text disassembly ---"
	$(OBJDUMP) -d $(BUILD)/$(CHIP).elf | less
