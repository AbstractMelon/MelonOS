# MelonOS

A simple, lightweight operating system written in C and Assembly.

## Features

- Bootloader
- Kernel with basic memory management
- Simple file system
- Basic shell interface
- Device drivers for keyboard and display
- Multitasking support

## Building

To build MelonOS, run:

```bash
./scripts/build.sh
```

This will generate a bootable disk image in the `build` directory.

## Testing

To test MelonOS in QEMU, run:

```bash
./scripts/run.sh
```

## Requirements

- GCC cross-compiler for i686-elf
- NASM assembler
- QEMU for testing
- GNU Make

## Project Structure

- `src/` - Source code
  - `boot/` - Bootloader code
  - `kernel/` - Kernel code
  - `drivers/` - Device drivers
  - `fs/` - File system implementation
  - `libc/` - C library implementation
- `include/` - Header files
- `scripts/` - Build and utility scripts
- `build/` - Build output (generated)

## License

See the [LICENSE](LICENSE) file for details.
