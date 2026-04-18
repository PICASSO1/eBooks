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
OBJS := main.o get_interface.o
ASS := main.S get_interface.S
INSTRUCTION := main.i get_interface.i

CFLAGS := -o -Wall -Wextra -O2
DEBUG := -ggdb

# for netlink nl80211
INCS := /usr/include/libnl3
LIB := nl-3 -lnl-genl-3
STATIC_LIB := 

.PHONY: all
all: $(BIN)

$(BIN): $(OBJS)
#	$(GCC) $(CFLAGS) $(BIN) $(OBJS)
	/usr/bin/gcc -o main main.o get_interface.o -I$(INCS) -l$(LIB)

$(OBJS): $(ASS)
#	$(GCC) -c $< -fverbose-asm $(CFLAGS) $@
	/usr/bin/gcc -c main.S -fverbose-asm -o main.o -I$(INCS) -l$(LIB)
	/usr/bin/gcc -c get_interface.S -fverbose-asm -o get_interface.o -I$(INCS) -l$(LIB)

$(ASS): $(INSTRUCTION)
#	$(GCC) -S $< $(CFLAGS) $@
	/usr/bin/gcc -S main.i -o main.S -I$(INCS) -l$(LIB)
	/usr/bin/gcc -S get_interface.i -o get_interface.S -I$(INCS) -l$(LIB)

$(INSTRUCTION): %.i: %.c
#	$(GCC) -E $< $(CFLAGS) $@
	/usr/bin/gcc -E main.c -o main.i -I$(INCS) -l$(LIB)
	/usr/bin/gcc -E get_interface.c -o get_interface.i -I$(INCS) -l$(LIB)

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
