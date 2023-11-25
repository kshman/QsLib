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
SRC= PatrickPowell_snprintf.c \
	qn_datetime.c qn_debug.c qn_file.c qn_hash_prime.c \
	qn_mem.c qn_mlu.c qn_sort.c qn_str.c \
	qn.c
OBJ=$(SRC:.c=.o)
LNK=$(notdir $(OBJ))

# build
all: $(DEST)

clean:
	rm -f $(DEST) $(LNK)

rebuild: clean all

install: all
	install $(DEST) /usr/local/lib
	ln -sf /usr/local/lib/$(DEST) /usr/local/lib/$(SOST)

%.o: %.c
	$(CC) -c $(CFLAG) -o "$(*F).o" "$<"

$(DEST): $(OBJ)
	$(CC) -shared -Wl,-export-dynamic -Wl,-soname,$@ -o $@ $(LNK)


