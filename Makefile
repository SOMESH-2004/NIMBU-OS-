# NIMBU OS - Makefile
# Builds the NIMBU OS kernel binary
# Build environment: Ubuntu 22.04 LTS, i686-elf-gcc, NASM

# Toolchain
CC = i686-elf-gcc
AS = nasm
LD = i686-elf-ld
STRIP = i686-elf-strip

# Compiler flags
CFLAGS = -ffreestanding -nostdlib -fno-builtin -Iinclude -Wall -Wextra
ASFLAGS = -f elf32
LDFLAGS = -T linker.ld -e _start

# Source directories
KERNEL_SRC = src/kernel
BOOT_SRC = src/boot
ARCH_SRC = $(KERNEL_SRC)/arch/x86
MEM_SRC = $(KERNEL_SRC)/mm
PROC_SRC = $(KERNEL_SRC)/proc
DRV_SRC = $(KERNEL_SRC)/drivers
SHELL_SRC = $(KERNEL_SRC)/shell

# Object directory
OBJ_DIR = build/obj
BIN_DIR = build/bin

# Source files
BOOT_OBJS = $(OBJ_DIR)/boot.o
KERNEL_OBJS = $(OBJ_DIR)/kernel.o
ARCH_OBJS = $(OBJ_DIR)/gdt.o $(OBJ_DIR)/idt.o
MEM_OBJS = $(OBJ_DIR)/pmm.o $(OBJ_DIR)/vmm.o $(OBJ_DIR)/heap.o
PROC_OBJS = $(OBJ_DIR)/process.o $(OBJ_DIR)/scheduler.o
DRV_OBJS = $(OBJ_DIR)/vga.o $(OBJ_DIR)/kbd.o $(OBJ_DIR)/serial.o
SHELL_OBJS = $(OBJ_DIR)/shell.o

ALL_OBJS = $(BOOT_OBJS) $(KERNEL_OBJS) $(ARCH_OBJS) $(MEM_OBJS) $(PROC_OBJS) $(DRV_OBJS) $(SHELL_OBJS)

# Targets
.PHONY: all clean run debug

all: $(BIN_DIR)/nimbu.bin $(BIN_DIR)/nimbu.elf

# Create directories
$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	@mkdir -p $(BIN_DIR)

# Boot
$(OBJ_DIR)/boot.o: $(BOOT_SRC)/boot.asm | $(OBJ_DIR)
	$(AS) $(ASFLAGS) $< -o $@

# Kernel main
$(OBJ_DIR)/kernel.o: $(KERNEL_SRC)/kernel.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Architecture
$(OBJ_DIR)/gdt.o: $(ARCH_SRC)/gdt.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/idt.o: $(ARCH_SRC)/idt.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Memory management
$(OBJ_DIR)/pmm.o: $(MEM_SRC)/pmm.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/vmm.o: $(MEM_SRC)/vmm.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/heap.o: $(MEM_SRC)/heap.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Process management
$(OBJ_DIR)/process.o: $(PROC_SRC)/process.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/scheduler.o: $(PROC_SRC)/scheduler.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Drivers
$(OBJ_DIR)/vga.o: $(DRV_SRC)/vga.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/kbd.o: $(DRV_SRC)/kbd.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/serial.o: $(DRV_SRC)/serial.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Shell
$(OBJ_DIR)/shell.o: $(SHELL_SRC)/shell.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Link
$(BIN_DIR)/nimbu.elf: $(ALL_OBJS) | $(BIN_DIR)
	$(LD) $(LDFLAGS) $(ALL_OBJS) -o $@

# Strip and create binary
$(BIN_DIR)/nimbu.bin: $(BIN_DIR)/nimbu.elf | $(BIN_DIR)
	$(STRIP) $(BIN_DIR)/nimbu.elf
	objcopy -O binary $(BIN_DIR)/nimbu.elf $@
	@echo "Kernel binary: $(BIN_DIR)/nimbu.bin"
	@ls -lh $@

# Run in QEMU
run: $(BIN_DIR)/nimbu.elf
	qemu-system-i386 -kernel $< -serial stdio

# Debug in QEMU
debug: $(BIN_DIR)/nimbu.elf
	qemu-system-i386 -kernel $< -s -S -serial stdio &
	gdb -ex "target remote :1234" -ex "break kmain" -ex "continue" $<

# Clean
clean:
	@rm -rf build/
	@echo "Clean complete"

# Print help
help:
	@echo "NIMBU OS Build System"
	@echo ""
	@echo "Targets:"
	@echo "  all       - Build kernel (default)"
	@echo "  run       - Run in QEMU emulator"
	@echo "  debug     - Run in QEMU with GDB debugger"
	@echo "  clean     - Remove build artifacts"
	@echo "  help      - Show this help"
