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
DEFS=
WARNS=-W -Wall -Wextra -Wno-missing-field-initializers
CFLAGS=-O3 -fPIC $(EMM) $(WARNS) $(DEFS)
INCDIR=-I../../inc
LIB=-lqn -lz -lm

# 플래그
CFLAG=$(CFLAGS) $(INCDIR)
LFLAG=$(LIB)

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
	$(CC) -c $(CFLAG) -S "$<"

$(DEST): $(SRC)
	$(CC) -o $@ $(CFLAG) $(LFLAG)  "$<"


