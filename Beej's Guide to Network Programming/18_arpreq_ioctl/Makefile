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

BIN := siocsarp siocgarp siocdarp
OBJS := siocsarp.o siocgarp.o siocdarp.o
ASS := siocsarp.S siocgarp.S siocdarp.S
INSTRUCTION := siocsarp.i siocgarp.i siocdarp.i

CFLAGS := -o
DEBUG := -ggdb

INCS := 
LIB := 
STATIC_LIB := 

.PHONY: all
all: $(BIN)

$(BIN): $(OBJS)
#	$(GCC) $(CFLAGS) $(BIN) $(OBJS)
	$(GCC) -o siocsarp siocsarp.o
	$(GCC) -o siocgarp siocgarp.o
	$(GCC) -o siocdarp siocdarp.o

$(OBJS): $(ASS)
#	$(GCC) -c $< -fverbose-asm $(CFLAGS) $@
	$(GCC) -c siocsarp.S -fverbose-asm $(CFLAGS) siocsarp.o
	$(GCC) -c siocgarp.S -fverbose-asm $(CFLAGS) siocgarp.o
	$(GCC) -c siocdarp.S -fverbose-asm $(CFLAGS) siocdarp.o

$(ASS): $(INSTRUCTION)
#	$(GCC) -S $< $(CFLAGS) $@
	$(GCC) -S siocsarp.i $(CFLAGS) siocsarp.S
	$(GCC) -S siocgarp.i $(CFLAGS) siocgarp.S
	$(GCC) -S siocdarp.i $(CFLAGS) siocdarp.S

$(INSTRUCTION): %.i: %.c
#	$(GCC) -E $< $(CFLAGS) $@
	$(GCC) -E siocsarp.c $(CFLAGS) siocsarp.i
	$(GCC) -E siocgarp.c $(CFLAGS) siocgarp.i
	$(GCC) -E siocdarp.c $(CFLAGS) siocdarp.i

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
