# PSoC Audio Player Software

This repository contains the PSoC reference audio player to be used with psoc-soc and the PSoC extension PCB.

![Player Software Running on PSoC FMC PCB](psoc_player.jpg)

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

## Generating Sound Files

The player currently only plays raw 48kHz, 16 bit signed `.s16` files.
To convert an audio file to this format, use ffmpeg like this:

```bash
ffmpeg -i fest2019.mp3 -acodec pcm_s16le -ac 2 -ar 48000 -f s16le fest2019.s16
```

To generate sine waves for testing, you can use something like this:
```bash
ffmpeg -f lavfi -i "sine=frequency=400:duration=30:sample_rate=48000" \
       -f lavfi -i "sine=frequency=800:duration=30:sample_rate=48000" \
       -filter_complex "[0:a][1:a]join=inputs=2:channel_layout=stereo[aout];[aout]volume=15dB[aout2]" \
       -map "[aout2]" -acodec pcm_s16le -ac 2 -ar 48000 -f s16le sine.s16
```