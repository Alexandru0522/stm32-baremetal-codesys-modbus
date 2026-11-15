# ==============================================================================
#  Production Makefile for Bare-Metal STM32F103 (Cortex-M3)
# ==============================================================================

TARGET    ?= main
BUILD_DIR  = build
BUILD_TYPE ?= release

# Auto-discover all C and Assembly source files
C_SRCS    := $(wildcard *.c)
ASM_SRCS  := $(wildcard *.s) $(wildcard *.S)

# Object files
OBJS      := $(addprefix $(BUILD_DIR)/, $(C_SRCS:.c=.o))
OBJS      += $(addprefix $(BUILD_DIR)/, $(ASM_SRCS:.s=.o))
DEPS      := $(OBJS:.o=.d)

# Toolchain definitions
CC        = arm-none-eabi-gcc
AS        = arm-none-eabi-gcc -x assembler-with-cpp
OBJCOPY   = arm-none-eabi-objcopy
SIZE      = arm-none-eabi-size

# Hardware architecture
MCU       = -mcpu=cortex-m3 -mthumb

# Optimization settings based on BUILD_TYPE
ifeq ($(BUILD_TYPE), debug)
    OPTFLAGS = -O0 -g3 -DDEBUG
else
    OPTFLAGS = -Os -g -DNDEBUG
endif

# Compiler flags (Strict warnings + Garbage collection + Dep tracking)
CFLAGS    = $(MCU) $(OPTFLAGS) -std=c11 \
            -Wall -Wextra -Wpedantic -Wshadow -Wdouble-promotion \
            -ffunction-sections -fdata-sections \
            -MMD -MP

ASFLAGS   = $(MCU) $(OPTFLAGS)

# Linker script & flags
LINKER    = linker.ld
LDFLAGS   = $(MCU) -T$(LINKER) \
            -Wl,-Map=$(BUILD_DIR)/$(TARGET).map \
            -Wl,--gc-sections \
            -Wl,--print-memory-usage \
            -nostdlib -lgcc

.PHONY: all clean flash debug release

all: $(BUILD_DIR)/$(TARGET).bin $(BUILD_DIR)/$(TARGET).hex
	@echo "--- Build Complete [$(BUILD_TYPE) mode] ---"

# Compile C source files
$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	@echo "[CC] $<"
	$(CC) $(CFLAGS) -c $< -o $@

# Assemble startup files (.s / .S)
$(BUILD_DIR)/%.o: %.s | $(BUILD_DIR)
	@echo "[AS] $<"
	$(AS) $(ASFLAGS) -c $< -o $@

# Link ELF output
$(BUILD_DIR)/$(TARGET).elf: $(OBJS)
	@echo "[LD] $@"
	$(CC) $(OBJS) $(LDFLAGS) -o $@

# Binary image generation
$(BUILD_DIR)/$(TARGET).bin: $(BUILD_DIR)/$(TARGET).elf
	@echo "[OBJCOPY BIN] $@"
	$(OBJCOPY) -O binary $< $@

# Intel HEX image generation (for factory flashers)
$(BUILD_DIR)/$(TARGET).hex: $(BUILD_DIR)/$(TARGET).elf
	@echo "[OBJCOPY HEX] $@"
	$(OBJCOPY) -O ihex $< $@

$(BUILD_DIR):
	mkdir -p $@

-include $(DEPS)

# Shortcut targets for specific build configurations
debug:
	$(MAKE) BUILD_TYPE=debug all

release:
	$(MAKE) BUILD_TYPE=release all

# Flash target via OpenOCD / ST-Link
flash: $(BUILD_DIR)/$(TARGET).bin
	st-flash --reset write $(BUILD_DIR)/$(TARGET).bin 0x08000000

clean:
	rm -rf $(BUILD_DIR)
