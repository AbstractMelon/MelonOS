# MelonOS Makefile
# Build a bootable ISO with GRUB

# Tools
ASM      = nasm
CC       = gcc
LD       = ld
GRUB     = grub-mkrescue

# Flags
ASMFLAGS  = -f elf32
GCC_INCL  = $(shell gcc -m32 -print-file-name=include)
CFLAGS    = -m32 -std=gnu99 -ffreestanding -O2 -Wall -Wextra -Werror \
            -fno-exceptions -fno-stack-protector -nostdlib -nostdinc \
            -isystem $(GCC_INCL) \
			-fno-builtin -fno-PIC -fno-tree-vectorize \
			-mno-mmx -mno-sse -mno-sse2 -mno-80387 -c
LDFLAGS   = -m elf_i386 -T linker.ld -nostdlib

# Directories
SRC_DIR    = src
BOOT_DIR   = $(SRC_DIR)/boot
KERNEL_DIR = $(SRC_DIR)/kernel
INCLUDE_DIR = $(KERNEL_DIR)/include
BUILD_DIR  = build
ISO_DIR    = $(BUILD_DIR)/isodir

# Source files
ASM_SRC  = $(BOOT_DIR)/boot.asm
C_SRC    = $(wildcard $(KERNEL_DIR)/arch/*.c) \
		   $(wildcard $(KERNEL_DIR)/core/*.c) \
		   $(wildcard $(KERNEL_DIR)/drivers/*.c) \
		   $(wildcard $(KERNEL_DIR)/lib/*.c)

# Object files
ASM_OBJ  = $(BUILD_DIR)/boot.o
C_OBJ    = $(patsubst $(KERNEL_DIR)/%.c, $(BUILD_DIR)/%.o, $(C_SRC))
OBJECTS  = $(ASM_OBJ) $(C_OBJ)

# Output
KERNEL   = $(BUILD_DIR)/melonos.bin
ISO      = $(BUILD_DIR)/melonos.iso
DISK_IMG = $(BUILD_DIR)/melonos_disk.img

# Default target
.PHONY: all clean run debug iso dev

all: $(ISO)

# Assemble boot.asm
$(BUILD_DIR)/boot.o: $(BOOT_DIR)/boot.asm | $(BUILD_DIR)
	$(ASM) $(ASMFLAGS) $< -o $@

# Compile C files
$(BUILD_DIR)/%.o: $(KERNEL_DIR)/%.c | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) $< -o $@

# Link everything
$(KERNEL): $(OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $^

# Create bootable ISO
$(ISO): $(KERNEL)
	@mkdir -p $(ISO_DIR)/boot/grub
	@cp $(KERNEL) $(ISO_DIR)/boot/melonos.bin
	@cp $(SRC_DIR)/grub/grub.cfg $(ISO_DIR)/boot/grub/grub.cfg
	$(GRUB) -o $(ISO) $(ISO_DIR) 2>/dev/null

# Create build directory
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

# Persistent virtual disk image for filesystem data
$(DISK_IMG): | $(BUILD_DIR)
	@test -f $(DISK_IMG) || dd if=/dev/zero of=$(DISK_IMG) bs=1M count=16 status=none

# Run in QEMU
run: $(ISO) $(DISK_IMG)
	qemu-system-i386 -cdrom $(ISO) -m 128M -drive file=$(DISK_IMG),format=raw,if=ide,index=0,media=disk

# Run in QEMU with debug output
debug: $(ISO) $(DISK_IMG)
	qemu-system-i386 -cdrom $(ISO) -m 128M -drive file=$(DISK_IMG),format=raw,if=ide,index=0,media=disk -d int,cpu_reset -no-reboot

# Development: build and run
dev: $(ISO) $(DISK_IMG)
	qemu-system-i386 -cdrom $(ISO) -m 128M -drive file=$(DISK_IMG),format=raw,if=ide,index=0,media=disk -serial stdio

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR)

# Check dependencies
check-deps:
	@echo "Checking build dependencies..."
	@which $(ASM) > /dev/null 2>&1 || (echo "ERROR: nasm not found. Install with: sudo pacman -S nasm" && exit 1)
	@which $(CC) > /dev/null 2>&1 || (echo "ERROR: gcc not found." && exit 1)
	@which $(LD) > /dev/null 2>&1 || (echo "ERROR: ld not found." && exit 1)
	@which $(GRUB) > /dev/null 2>&1 || (echo "ERROR: grub-mkrescue not found. Install GRUB tools." && exit 1)
	@which qemu-system-i386 > /dev/null 2>&1 || (echo "ERROR: qemu not found. Install with: sudo pacman -S qemu-full" && exit 1)
	@echo "All dependencies found!"
