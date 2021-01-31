#!/usr/bin/make -f
# Makefile for DIE-Plugins #
# ------------------------ #
# Created by falkTX
#

include Makefile.base.mk

PREFIX ?= /usr/local

# --------------------------------------------------------------

all: build

# --------------------------------------------------------------

build:
	$(MAKE) -C src

# --------------------------------------------------------------

clean:
	$(MAKE) clean -C src

# --------------------------------------------------------------

install:
	install -d $(DESTDIR)$(PREFIX)/bin
	install -m 755 bin/koriborusu $(DESTDIR)$(PREFIX)/bin/

# --------------------------------------------------------------

.PHONY: build
