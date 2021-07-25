#!/usr/bin/make -f
# Makefile for ACE #
# ---------------- #
# Created by falkTX
#

include ../Makefile.base.mk

# ---------------------------------------------------------------------------------------------------------------------
# Basic setup

TARGET_DIR = ../bin
BUILD_DIR = ../build

BASE_FLAGS += $(shell pkg-config --cflags carla-host-plugin sndfile)
LINK_FLAGS += $(shell pkg-config --libs carla-host-plugin sndfile)

BASE_FLAGS += -Wno-unused-parameter

# ---------------------------------------------------------------------------------------------------------------------
# Set files to build

FILES = host.c kuriborosu.c kuribu.c
OBJS  = $(FILES:%=$(BUILD_DIR)/%.o)

# ---------------------------------------------------------------------------------------------------------------------
# Default build targets

TARGETS = $(TARGET_DIR)/kuriborosu $(TARGET_DIR)/kuribu

all: build

build: $(TARGETS)

# ---------------------------------------------------------------------------------------------------------------------
# Build commands

$(TARGET_DIR)/kuriborosu: $(BUILD_DIR)/kuriborosu.c.o $(BUILD_DIR)/host.c.o
	-@mkdir -p $(shell dirname $@)
	@echo "Linking kuriborosu"
	$(SILENT)$(CXX) $^ $(LINK_FLAGS) -o $@

$(TARGET_DIR)/kuribu: $(BUILD_DIR)/kuribu.c.o $(BUILD_DIR)/host.c.o
	-@mkdir -p $(shell dirname $@)
	@echo "Linking kuribu"
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
	rm -rf $(BUILD_DIR) $(TARGET)

# ---------------------------------------------------------------------------------------------------------------------

-include $(OBJS:%.o=%.d)

# ---------------------------------------------------------------------------------------------------------------------
