# Last modified by KSH 2023-12-06
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
INCDIR=-I../../inc -I../../src

# 플래그
CFLAGS=-O3 -flto -pipe -fPIC -fvisibility=hidden $(EMM) $(WARNS) $(DEFS) $(INCDIR)
LDFLAGS=-flto
#
DEST=libqs.so.3
SOST=libqs.so

#
VPATH=src:../../src

#
SRC=pch.c \
	qn/PatrickPowell_snprintf.c \
	qn/zlib/adler32.c qn/zlib/compress.c qn/zlib/crc32.c qn/zlib/deflate.c qn/zlib/gzclose.c qn/zlib/gzlib.c qn/zlib/gzread.c qn/zlib/gzwrite.c qn/zlib/infback.c qn/zlib/inffast.c qn/zlib/inflate.c qn/zlib/inftrees.c qn/zlib/trees.c qn/zlib/uncompr.c qn/zlib/zutil.c \
	qn/qn.c qn/qn_debug.c qn/qn_file.c qn/qn_gam.c qn/qn_hash.c qn/qn_math.c qn/qn_mem.c qn/qn_mlu.c qn/qn_sort.c qn/qn_str.c qn/qn_thd.c qn/qn_time.c
#	qg/qges2_rdh.c qg/qges2_rdo.c qg/qges2_shd.c qg/qgsdl_stub.c \
#	qg/qg_kmc.c qg/qg_rdh.c qg/qg_rdo.c qg/qg_stub.c
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
	$(CC) $(LDFLAGS) -shared -Wl,-export-dynamic -Wl,-soname,$@ -o $@ $(LNK)
