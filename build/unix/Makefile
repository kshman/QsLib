# Makefile for 'qs'
# use 'gmake'
#
# Last modified by KSH 2023-11-25
#

MAKE=make

OS=$(shell uname)

# FreeBSD에서는 gcc46이상 쓸것 -> 기본 컴파일러가 UTF-8 BOM 지원을 안함
ifeq "$(OS)" "FreeBSD"
MAKE=gmake
endif

# 전체 빌드
all: $(DEST)
	$(MAKE) -f qn.mk all

asm: $(DEST)
	$(MAKE) -f qn.mk asm

clean:
	$(MAKE) -f qn.mk clean

rebuild: clean all

install: all
	$(MAKE) -f qn.mk install

# 참고: 컴파일러 정의 확인
# echo | cc -dM -E -
# echo | c++ -dM -E -x c++ -
