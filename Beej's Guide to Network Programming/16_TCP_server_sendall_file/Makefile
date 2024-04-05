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

BIN := server client
OBJS := server.o client.o
ASS := server.S client.S
INSTRUCTION := server.i client.i

CFLAGS := -o
DEBUG := -ggdb

INCS := 
LIB := 
STATIC_LIB := 

.PHONY: all
all: $(BIN)

$(BIN): $(OBJS)
#	$(GCC) $(CFLAGS) $(BIN) $(OBJS)
	$(GCC) $(CFLAGS) server server.o
	$(GCC) $(CFLAGS) client client.o

$(OBJS): $(ASS)
#	$(GCC) -c $< -fverbose-asm $(CFLAGS) $@
	$(GCC) -c server.S -fverbose-asm $(CFLAGS) server.o
	$(GCC) -c client.S -fverbose-asm $(CFLAGS) client.o

$(ASS): $(INSTRUCTION)
#	$(GCC) -S $< $(CFLAGS) $@
	$(GCC) -S server.i $(CFLAGS) server.S
	$(GCC) -S client.i $(CFLAGS) client.S

$(INSTRUCTION): %.i: %.c include.h define.h declare.h
#	$(GCC) -E $< $(CFLAGS) $@
	$(GCC) -E server.c $(CFLAGS) server.i
	$(GCC) -E client.c $(CFLAGS) client.i

.PHONY: clean cleanall install print tar dist TAGS check test
clean:
	@-$(RM) *.i *.S *.o

cleanall: clean
	@-$(RM) $(BIN) client.file

install:

print:

tar:

dist:

TAGS:

check: 

test:
