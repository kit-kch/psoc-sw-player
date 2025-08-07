# PSoC Audio Player Software

This repository contains the PSoC reference audio player to be used with psoc-soc and the PSoC extension PCB.

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

To rebuild the `splash.h` file from `splash.png`:
```bash
# Make sure that the png does not have an alpha channel:
convert splash_in.png -alpha off splash.png

python -m venv .venv
source .venv/bin/activate
pip install pillow
./ext/psoc-sw-adafruit-ssd1306/scripts/make_splash.py splash.png psoc_splash > splash.h
```

For flashing, refer to the [psoc-soc instructions](https://github.com/kit-kch/psoc-soc/).