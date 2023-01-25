# Simple Makefile for extmodule
# 2022/12/19

MAKEFLAGS += -r -R
CC        ?= gcc
CFLAGS    ?= -Wall -fPIC -Os -D_GNU_SOURCE -ggdb
TARGETS    = libextm.so

.PHONY: all clean

all: $(TARGETS)

libextm.so: extmodule.o
	$(CC) -shared -o $@ -Wl,-soname=$@ $^

%.o: %.c extmodule.h
	$(CC) -c $(CFLAGS) -o $@ $<

clean:
	rm -rf *.o $(TARGETS)
