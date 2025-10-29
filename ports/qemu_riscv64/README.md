# QEMU RISCV Port for libmpix

This port provides support for running libmpix on QEMU's RISCV emulation.

## Requirements

- RISCV GNU toolchain at `/opt/risv/riscv64-unknown-elf/bin`
- QEMU with RISCV support
- CMake 3.20 or later

## Features

- Able to run all libmpx tests on the riscv environment

```sh
cmake -DCMAKE_TOOLCHAIN_FILE=$(pwd)/ports/qemu_riscv64/toolchain-riscv64.cmake -B build ports/qemu_riscv64
```