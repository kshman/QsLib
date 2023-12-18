# Last modified by KSH 2023-12-04
#

# 플랫폼
PLATFORM=$(shell uname -m)
ifeq ("$(PLATFORM)", "x86_64")
EMM=-mavx2
else
EMM=
endif

# 기본값
CC=cc
DEFS=
INCDIR=-I../../inc

# 플래그
CFLAGS=$(EMM) $(INCDIR)
LDFLAGS=-lqs -lz -lSDL2 -lGLESv2 -lm

#
VPATH=src:../../zconsole

#
DEST=test_printf test_stub test_rdh test_buffer test_buffer_multi

# build
all: $(DEST)

asm:

clean:
	rm -f $(DEST)

rebuild: clean all

install: 

test_printf: test_printf.c

test_stub: test_stub.c

test_rdh: test_rdh.c

test_buffer: test_buffer.c

test_buffer_multi: test_buffer_multi.c
