NEORV32_HOME ?= ext/neorv32
# FIXME: Clone into neosd submodule
NEOSD_HOME ?= ext/neosd
PSOC_LIB_HOME ?= ext/psoc-sw-lib
PSOC_WIRE_HOME ?= ext/psoc-sw-arduino-wire
ADAFRUIT_GFX_HOME ?= ext/psoc-sw-adafruit-gfx
ADAFRUIT_SSD1306_HOME ?= ext/psoc-sw-adafruit-ssd1306

#include <neosd.h>

APP_SRC = $(wildcard ./source/*.c) $(wildcard ./source/*.s) $(wildcard ./source/*.cpp) $(wildcard ./source/*.S)
APP_INC += -I ./include

CXXFLAGS = -fno-exceptions -fno-rtti -fno-use-cxa-atexit -fno-threadsafe-statics
LDFLAGS = -lstdc++ -lsupc++
USER_FLAGS = -Wl,--defsym,__neorv32_rom_size=256K -Wl,--defsym,__neorv32_ram_size=16K -Wl,--defsym,__neorv32_heap_size=1K -D SSD1306_NO_SPLASH

# Set base address of the NEOSD peripheral
USER_FLAGS += -D 'NEOSD_BASE=(0xFFD20000U)'

APP_INC += -I ./ext/etl/include
include $(PSOC_LIB_HOME)/psoc_lib.mk
include $(PSOC_WIRE_HOME)/neorv_lib.mk
include $(ADAFRUIT_GFX_HOME)/neorv_lib.mk
include $(ADAFRUIT_SSD1306_HOME)/neorv_lib.mk
include $(NEOSD_HOME)/sw/lib/neorv_lib.mk
include $(NEOSD_HOME)/sw/fatfs/neorv_lib.mk
include $(NEORV32_HOME)/sw/common/common.mk