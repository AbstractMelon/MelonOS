#!/bin/bash

# MelonOS build script

set -e

# Configuration
export PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
export BUILD_DIR="$PROJECT_ROOT/build"
export SRC_DIR="$PROJECT_ROOT/src"
export INCLUDE_DIR="$PROJECT_ROOT/include"

# Tools configuration
export AS="nasm"
export CC="gcc"
export LD="ld"

# Flags
export ASFLAGS="-f elf32"
export CFLAGS="-std=gnu99 -ffreestanding -O2 -Wall -Wextra -I$INCLUDE_DIR -m32 -fno-stack-protector -fno-pie"
export LDFLAGS="-ffreestanding -O2 -nostdlib -m32 -no-pie"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
NC='\033[0m' # No Color

# Check if cross-compiler is installed
if ! command -v $CC &> /dev/null; then
    echo -e "${RED}Error: $CC is not installed.${NC}"
    exit 1
fi

# Create build directory if it doesn't exist
mkdir -p "$BUILD_DIR"

echo -e "${YELLOW}Building MelonOS...${NC}"

# Build bootloader
echo -e "${GREEN}Building bootloader...${NC}"
$AS -f bin "$SRC_DIR/boot/boot.asm" -o "$BUILD_DIR/boot.bin"

# Build kernel
echo -e "${GREEN}Building kernel...${NC}"
$AS $ASFLAGS "$SRC_DIR/kernel/kernel_entry.asm" -o "$BUILD_DIR/kernel_entry.o"

# Compile C kernel files
echo -e "${GREEN}Compiling kernel C files...${NC}"
$CC $CFLAGS -c "$SRC_DIR/kernel/kernel.c" -o "$BUILD_DIR/kernel.o"
$CC $CFLAGS -c "$SRC_DIR/drivers/screen.c" -o "$BUILD_DIR/screen.o"
$CC $CFLAGS -c "$SRC_DIR/drivers/keyboard.c" -o "$BUILD_DIR/keyboard.o"
$CC $CFLAGS -c "$SRC_DIR/libc/string.c" -o "$BUILD_DIR/string.o"
$CC $CFLAGS -c "$SRC_DIR/libc/mem.c" -o "$BUILD_DIR/mem.o"
$CC $CFLAGS -c "$SRC_DIR/fs/fs.c" -o "$BUILD_DIR/fs.o"

# Link the kernel
echo -e "${GREEN}Linking kernel...${NC}"
$CC $LDFLAGS -T "$SRC_DIR/kernel/linker.ld" -o "$BUILD_DIR/kernel.bin" \
    "$BUILD_DIR/kernel_entry.o" "$BUILD_DIR/kernel.o" "$BUILD_DIR/screen.o" \
    "$BUILD_DIR/keyboard.o" "$BUILD_DIR/string.o" "$BUILD_DIR/mem.o" \
    "$BUILD_DIR/fs.o" \
    -lgcc

# Create disk image
echo -e "${GREEN}Creating disk image...${NC}"
cat "$BUILD_DIR/boot.bin" > "$BUILD_DIR/os-image.bin"

# Pad the boot sector to exactly 512 bytes
BOOT_SIZE=$(stat -c%s "$BUILD_DIR/boot.bin")
BOOT_PADDING=$((512 - BOOT_SIZE))
if [ $BOOT_PADDING -gt 0 ]; then
    dd if=/dev/zero bs=1 count=$BOOT_PADDING >> "$BUILD_DIR/os-image.bin"
fi

# Append the kernel
cat "$BUILD_DIR/kernel.bin" >> "$BUILD_DIR/os-image.bin"

# Pad the entire image to a multiple of 512 bytes
FILESIZE=$(stat -c%s "$BUILD_DIR/os-image.bin")
PADDING=$((512 - $FILESIZE % 512))
if [ $PADDING -lt 512 ]; then
    dd if=/dev/zero bs=1 count=$PADDING >> "$BUILD_DIR/os-image.bin"
fi

echo -e "${GREEN}Build completed successfully!${NC}"
echo -e "${YELLOW}OS image: $BUILD_DIR/os-image.bin${NC}"
