###
 # 
 # 這是一個既簡單又標準的 Makefile！任何練習都可以拿來用。
 #
######

# Compiler
GCC := /usr/bin/gcc
# Assembler
AS := /usr/bin/as
# Architecture, for create the Static Library
AR := /usr/bin/ar
# Linker
LDD := /usr/bin/ldd
# rm command .... 
RM := /usr/bin/rm -rf

BIN := main
OBJS := main.o rtm_getaddr.o
ASS := main.S rtm_getaddr.S
INSTRUCTION := main.i rtm_getaddr.i

CFLAGS := -o
DEBUG := -ggdb

INCS := 
LIB := 
STATIC_LIB := 

.PHONY: all
all: $(BIN)

$(BIN): $(OBJS)
#	$(GCC) $(CFLAGS) $(BIN) $(OBJS)
	/usr/bin/gcc -o main main.o rtm_getaddr.o

$(OBJS): $(ASS)
#	$(GCC) -c $< -fverbose-asm $(CFLAGS) $@
	/usr/bin/gcc -c main.S -fverbose-asm -o main.o
	/usr/bin/gcc -c rtm_getaddr.S -fverbose-asm -o rtm_getaddr.o

$(ASS): $(INSTRUCTION)
#	$(GCC) -S $< $(CFLAGS) $@
	/usr/bin/gcc -S main.i -o main.S
	/usr/bin/gcc -S rtm_getaddr.i -o rtm_getaddr.S

$(INSTRUCTION): %.i: %.c
	$(GCC) -E $< $(CFLAGS) $@
#	/usr/bin/gcc -E main.c -o rtm_getaddr.i
#	/usr/bin/gcc -E rtm_getaddr.c -o rtm_getaddr.i

.PHONY: clean cleanall install print tar dist TAGS check test
clean:
	@-$(RM) *.i *.S *.o

cleanall: clean
	@-$(RM) $(BIN) 

install:

print:

tar:

dist:

TAGS:

check: 

test:
