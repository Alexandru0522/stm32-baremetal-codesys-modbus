# ==============================================================================
#           MAKEFILE BARE-METAL PENTRU STM32F103C8Tx (BLUE PILL)
# ==============================================================================

# Numele proiectului și al binarului final
TARGET = main

# Toolchain-ul GNU pentru ARM
CC      = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
SIZE    = arm-none-eabi-size

# Arhitectura hardware (Cortex-M3, set de instrucțiuni Thumb)
MCU     = -mcpu=cortex-m3 -mthumb

# Surse C din proiect
SRCS    = main.c startup.c
OBJS    = $(SRCS:.c=.o)

# Fișierul de Linker Script
LINKER_SCRIPT = linker.ld

# Flag-uri de compilare (C flags)
# -O0: Fără optimizări (util pentru debugging)
# -g3: Include informații extinse de debug pentru GDB
# -Wall -Wextra: Activează toate avertismentele utile
CFLAGS  = $(MCU) -Wall -Wextra -O0 -g3 -std=c11

# Flag-uri de legare (Linker flags)
# -nostdlib: Nu include bibliotecile standard C implicite (C runtime, startup OS)
# -Wl,-Map: Generează un fișier .map cu harta detaliată a memoriei
LDFLAGS = $(MCU) -T$(LINKER_SCRIPT) -Wl,-Map=$(TARGET).map -nostdlib

# ==============================================================================
#                                  REGULI MAKE
# ==============================================================================

# Regula principală (Rule de bază)
all: $(TARGET).bin
	@echo "--- Utilizare Memorie (Flash & RAM) ---"
	$(SIZE) $(TARGET).elf

# Pasul 1: Compilarea fiecărui fișier .c într-un fișier obiect .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Pasul 2: Linkit-ul fișierelor obiect într-un fișier executabil .elf
$(TARGET).elf: $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o $@

# Pasul 3: Extragerea binarului brut (.bin) din fișierul .elf pentru Flashing
$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

# Curățarea fișierelor generate
clean:
	rm -f $(OBJS) $(TARGET).elf $(TARGET).bin $(TARGET).map

.PHONY: all clean

# Programare hardware prin OpenOCD și ST-Link V2
flash: $(TARGET).bin
	st-flash write $(TARGET).bin 0x08000000

.PHONY: all clean flash
