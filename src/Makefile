#!/usr/bin/make -f
# Makefile for ACE #
# ---------------- #
# Created by falkTX
#

include ../Makefile.base.mk

# --------------------------------------------------------------
# Project name, used for binaries

NAME = kuriborosu

# --------------------------------------------------------------
# Files to build

FILES = \
	kuriborosu.c

# ---------------------------------------------------------------------------------------------------------------------
# Basic setup

TARGET_DIR = ../bin
BUILD_DIR = ../build/$(NAME)

BASE_FLAGS += $(shell pkg-config --cflags carla-host-plugin sndfile)
LINK_FLAGS += $(shell pkg-config --libs carla-host-plugin sndfile)

BASE_FLAGS += -Wno-unused-parameter

# ---------------------------------------------------------------------------------------------------------------------
# Set files to build

OBJS = $(FILES:%=$(BUILD_DIR)/%.o)

# ---------------------------------------------------------------------------------------------------------------------
# Default build target

TARGET = $(TARGET_DIR)/$(NAME)

all: build

build: $(TARGET)

# ---------------------------------------------------------------------------------------------------------------------
# Build commands

$(TARGET): $(OBJS)
	-@mkdir -p $(shell dirname $@)
	@echo "Linking $(NAME)"
	$(SILENT)$(CXX) $^ $(LINK_FLAGS) -o $@

$(BUILD_DIR)/%.c.o: %.c
	-@mkdir -p "$(shell dirname $(BUILD_DIR)/$<)"
	@echo "Compiling $<"
	$(SILENT)$(CC) $< $(BUILD_C_FLAGS) -c -o $@

$(BUILD_DIR)/%.cpp.o: %.cpp
	-@mkdir -p "$(shell dirname $(BUILD_DIR)/$<)"
	@echo "Compiling $<"
	$(SILENT)$(CXX) $< $(BUILD_CXX_FLAGS) -c -o $@

clean:
	rm -rf $(BUILD_DIR)

# ---------------------------------------------------------------------------------------------------------------------

-include $(OBJS:%.o=%.d)

# ---------------------------------------------------------------------------------------------------------------------
