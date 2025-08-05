# PSoC OLED Display Test

This repository contains a small firmware to test the OLED Display on the PSoC extension PCB.

## Compiling the Software

Make sure to setup the compiler first. The currently tested version is [xPack GNU RISC-V Embedded GCC v14.2.0-3](https://github.com/xpack-dev-tools/riscv-none-elf-gcc-xpack/releases/tag/v14.2.0-3).

To compile, `cd` into the source folders and use the Makefile:
```bash
# To set up the compiler at ITIV, otherwise do that manually
source /tools/psoc/psoc.sh riscv
# Generate executable for gdb
make elf
# Remove all generated files
make clean_all
```

For flashing, refer to the [psoc-soc instructions](https://github.com/kit-kch/psoc-soc/).