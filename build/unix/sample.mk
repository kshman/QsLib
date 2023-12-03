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
LFLAGS=-lqn -lz -lm

#
VPATH=src:../../zconsole/sample

#
SRC=\
	test_printf.c
DEST=$(SRC:.c=)
ASM=$(SRC:.c=.s)

# build
all: $(DEST)

asm: $(ASM)

clean:
	rm -f $(DEST) $(ASM)

rebuild: clean all

install: 

%.s: %.c
	$(CC) -c $(CFLAGS) -S -fverbose-asm "$<"

$(DEST): $(SRC)
	$(CC) -o $@ $(CFLAGS) $(LFLAGS)  "$<"


