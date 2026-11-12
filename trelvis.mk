# SPDX-License-Identifier: MIT
# Copyright (c) 2026 Timothy Mitchell
#
# Source list for non-CMake builds (plain Makefiles).
#
#   include path/to/trelvis/trelvis.mk
#
# appends TreLVis to CSRCS and CFLAGS. LVGL's include directory and any
# lv_conf.h / trlv_conf.h settings must be provided by your build as usual.
#
# IDE-managed builds (e.g. STM32CubeIDE) don't need this file: add
# trelvis/src/*.c to the sources and trelvis/include to the include paths.

TRELVIS_DIR ?= $(patsubst %/,%,$(dir $(lastword $(MAKEFILE_LIST))))

TRELVIS_CSRCS := $(wildcard $(TRELVIS_DIR)/src/*.c)
TRELVIS_INC   := $(TRELVIS_DIR)/include

CSRCS  += $(TRELVIS_CSRCS)
CFLAGS += -I$(TRELVIS_INC)
