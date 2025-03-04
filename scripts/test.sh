#!/bin/bash

# MelonOS test script

set -e

# Configuration
export PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
export BUILD_DIR="$PROJECT_ROOT/build"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
NC='\033[0m' # No Color

# Check if the OS image exists
if [ ! -f "$BUILD_DIR/os-image.bin" ]; then
    echo -e "${RED}OS image not found. Please build MelonOS first.${NC}"
    echo -e "${YELLOW}Run: ./scripts/build.sh${NC}"
    exit 1
fi

echo -e "${YELLOW}Testing MelonOS in QEMU (non-interactive mode)...${NC}"
qemu-system-i386 -fda "$BUILD_DIR/os-image.bin" -display none -no-reboot -serial stdio
