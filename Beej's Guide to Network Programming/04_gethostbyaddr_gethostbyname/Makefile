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

BIN := gethostbyaddr gethostbyname
OBJS := gethostbyaddr.o gethostbyname.o
ASS := gethostbyaddr.S gethostbyname.S
INSTRUCTION := gethostbyaddr.i gethostbyname.i

CFLAGS := -o
DEBUG := -ggdb

INCS := 
LIB := 
STATIC_LIB := 

.PHONY: all
all: $(BIN)

$(BIN): $(OBJS)
#	$(GCC) $(CFLAGS) $(BIN) $(OBJS)
	$(GCC) $(CFLAGS) gethostbyaddr gethostbyaddr.o
	$(GCC) $(CFLAGS) gethostbyname gethostbyname.o

$(OBJS): $(ASS)
#	$(GCC) -c $< -fverbose-asm $(CFLAGS) $@
	$(GCC) -c gethostbyaddr.S -fverbose-asm $(CFLAGS) gethostbyaddr.o
	$(GCC) -c gethostbyname.S -fverbose-asm $(CFLAGS) gethostbyname.o

$(ASS): $(INSTRUCTION)
#	$(GCC) -S $< $(CFLAGS) $@
	$(GCC) -S gethostbyaddr.i $(CFLAGS) gethostbyaddr.S
	$(GCC) -S gethostbyname.i $(CFLAGS) gethostbyname.S

$(INSTRUCTION): %.i: %.c
#	$(GCC) -E $< $(CFLAGS) $@
	$(GCC) -E gethostbyaddr.c $(CFLAGS) gethostbyaddr.i
	$(GCC) -E gethostbyname.c $(CFLAGS) gethostbyname.i

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
