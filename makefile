NEORV32_HOME ?= ext/neorv32
PSOC_LIB_HOME ?= ext/psoc-sw-lib
PSOC_WIRE_HOME ?= ext/psoc-sw-arduino-wire
ADAFRUIT_GFX_HOME ?= ext/psoc-sw-adafruit-gfx
ADAFRUIT_SSD1306_HOME ?= ext/psoc-sw-adafruit-ssd1306

APP_SRC = $(wildcard ./*.c) $(wildcard ./*.s) $(wildcard ./*.cpp) $(wildcard ./*.S)

CXXFLAGS = -fno-exceptions -fno-rtti -fno-use-cxa-atexit -fno-threadsafe-statics
USER_FLAGS = -Wl,--defsym,__neorv32_rom_size=256K -Wl,--defsym,__neorv32_ram_size=16K -Wl,--defsym,__neorv32_heap_size=8K -D SSD1306_NO_SPLASH

include $(PSOC_LIB_HOME)/psoc_lib.mk
include $(PSOC_WIRE_HOME)/neorv_lib.mk
include $(ADAFRUIT_GFX_HOME)/neorv_lib.mk
include $(ADAFRUIT_SSD1306_HOME)/neorv_lib.mk
include $(NEORV32_HOME)/sw/common/common.mk