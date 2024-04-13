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

BIN := siocaddrt siocdelrt
OBJS := siocaddrt.o siocdelrt.o
ASS := siocaddrt.S siocdelrt.S
INSTRUCTION := siocaddrt.i siocdelrt.i

CFLAGS := -o
DEBUG := -ggdb

INCS := 
LIB := 
STATIC_LIB := 

.PHONY: all
all: $(BIN)

$(BIN): $(OBJS)
#	$(GCC) $(CFLAGS) $(BIN) $(OBJS)
	$(GCC) -o siocaddrt siocaddrt.o
	$(GCC) -o siocdelrt siocdelrt.o

$(OBJS): $(ASS)
#	$(GCC) -c $< -fverbose-asm $(CFLAGS) $@
	$(GCC) -c siocaddrt.S -fverbose-asm $(CFLAGS) siocaddrt.o
	$(GCC) -c siocdelrt.S -fverbose-asm $(CFLAGS) siocdelrt.o

$(ASS): $(INSTRUCTION)
#	$(GCC) -S $< $(CFLAGS) $@
	$(GCC) -S siocaddrt.i $(CFLAGS) siocaddrt.S
	$(GCC) -S siocdelrt.i $(CFLAGS) siocdelrt.S

$(INSTRUCTION): %.i: %.c
#	$(GCC) -E $< $(CFLAGS) $@
	$(GCC) -E siocaddrt.c $(CFLAGS) siocaddrt.i
	$(GCC) -E siocdelrt.c $(CFLAGS) siocdelrt.i

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
