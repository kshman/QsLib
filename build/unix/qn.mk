# Last modified by KSH 2023-11-25
#

# 기본값
DEF=
STD=gnu18
LIB=-lz -lm
INCDIR=-I../../inc -I../../qn -I/usr/include -I/usr/local/include
LIBDIR=-L/usr/lib -L/usr/local/lib

# 플래그
CFLAG=-O3 -pipe -fPIC -std=$(STD) -W -Wall -fvisibility=hidden $(DEF) $(INCDIR)
LFLAG=$(LIBDIR) $(LIB)

#
DEST=libqn.so.6
SOST=libqn.so

#
VPATH=src:../../qn

#
SRC=qn.c qn_mem.c qn_hash.c qn_sort.c qn_str.c qn_datetime.c qn_debug.c qn_file.c  qn_mlu.c \
	PatrickPowell_snprintf.c 
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
	$(CC) -c $(CFLAG) -o "$(*F).o" "$<"

%.s: %.c
	$(CC) -c $(CFLAG) -S "$<"

$(DEST): $(OBJ)
	$(CC) -shared -Wl,-export-dynamic -Wl,-soname,$@ -o $@ $(LNK)


