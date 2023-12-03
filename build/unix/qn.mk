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
WARNS=-W -Wall -Wextra
INCDIR=-I../../inc -I../../qn

# 플래그
CFLAGS=-O3 -pipe -fPIC -fvisibility=hidden $(WARNS) $(EMM) $(DEFS) $(INCDIR)

#
DEST=libqn.so.6
SOST=libqn.so

#
VPATH=src:../../qn

#
SRC=\
	PatrickPowell_snprintf.c \
	qn.c qn_datetime.c qn_debug.c qn_file.c qn_gam.c qn_hash.c qn_math.c \
	qn_mem.c qn_mlu.c qn_sort.c qn_str.c
OBJ=$(SRC:.c=.o)
ASM=$(SRC:.c=.s)
LNK=$(notdir $(OBJ))

# build
all: $(DEST)

asm: $(ASM)

clean:
	rm -f $(DEST) $(LNK) $(ASM)

rebuild: clean all

install: all
	install $(DEST) /usr/lib
	ln -sf /usr/lib/$(DEST) /usr/lib/$(SOST)

%.o: %.c
	$(CC) -c $(CFLAGS) -o "$(*F).o" "$<"

%.s: %.c
	$(CC) -c $(CFLAGS) -S -fverbose-asm "$<"

$(DEST): $(OBJ)
	$(CC) -shared -Wl,-export-dynamic -Wl,-soname,$@ -o $@ $(LNK)


