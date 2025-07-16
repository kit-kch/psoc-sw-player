NEORV32_HOME ?= ext/neorv32
PSOC_LIB_HOME ?= ext/psoc-sw-lib

APP_SRC = $(wildcard ./*.c) $(wildcard ./*.s) $(wildcard ./*.cpp) $(wildcard ./*.S)

include $(PSOC_LIB_HOME)/psoc_lib.mk
include $(NEORV32_HOME)/sw/common/common.mk
