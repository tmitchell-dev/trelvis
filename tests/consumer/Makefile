# SPDX-License-Identifier: MIT
# Copyright (c) 2026 Timothy Mitchell
#
# The consumer project built from a plain Makefile with LVGL's lvgl.mk and
# TreLVis's trelvis.mk, as the README's quick start describes. CI builds
# and runs it.
#
#   make -C tests/consumer LVGL_DIR=path/to/lvgl run

ifndef LVGL_DIR
$(error Set LVGL_DIR to an LVGL 9.6 source tree)
endif

CC     ?= cc
BUILD  ?= build-make
TARGET := $(BUILD)/consumer
GEN    := $(abspath $(BUILD))

# Where a product keeps the submodules.
LVGL_PATH   := $(abspath $(LVGL_DIR))
TRELVIS_DIR := $(abspath ../..)

CFLAGS ?= -O1
include $(LVGL_PATH)/lvgl.mk
include $(TRELVIS_DIR)/trelvis.mk

# lvgl.mk puts only LVGL's own directory on the include path; TreLVis
# includes <lvgl/lvgl.h> from LVGL's include/ directory.
CFLAGS += -I$(LVGL_PATH)/include
# lv_conf.h and trlv_conf.h, next to each other.
CFLAGS += -Iconfig -DLV_CONF_INCLUDE_SIMPLE
# The application: the copied tokens and the demo.
CFLAGS += -I$(BUILD) -I$(TRELVIS_DIR)/examples/demo -DCONSUMER_EXPECT_CONF=1

CSRCS += main.c $(GEN)/tokens.c $(wildcard $(TRELVIS_DIR)/examples/demo/*.c)

# One object per source, under $(BUILD) at the source's absolute path.
OBJS := $(patsubst %.c,$(BUILD)/obj%.o,$(abspath $(CSRCS)))

all: $(TARGET)

run: $(TARGET)
	./$(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ -lm

$(BUILD)/obj%.o: %.c | $(GEN)/tokens.h
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c $< -o $@

# The copied token template (quick start, step 3).
$(GEN)/tokens.c: $(TRELVIS_DIR)/templates/trlv_tokens_template.c
	@mkdir -p $(GEN)
	sed 's/#include "trlv_tokens_template.h"/#include "tokens.h"/' $< > $@

$(GEN)/tokens.h: $(TRELVIS_DIR)/templates/trlv_tokens_template.h
	@mkdir -p $(GEN)
	cp $< $@

clean:
	rm -rf $(BUILD)

.PHONY: all run clean
