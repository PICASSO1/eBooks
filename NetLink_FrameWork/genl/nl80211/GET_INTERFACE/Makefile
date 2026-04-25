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

CFLAGS := -o
DEBUG := -ggdb -Wall -Wextra -O2

# for netlink nl80211
INCS := /usr/include/libnl3
LIB := nl-3 -lnl-genl-3
STATIC_LIB := 

.PHONY: all
all: $(BIN)

$(BIN): $(OBJS)
#	$(GCC) $(CFLAGS) $(BIN) $(OBJS) $(DEBUG) -I$(INCS) -l$(LIB)
	/usr/bin/gcc -o main main.o get_interface.o -ggdb -Wall -Wextra -O2 -I$(INCS) -l$(LIB)

$(OBJS): $(ASS)
#	$(GCC) -c $< -fverbose-asm $(CFLAGS) $@ $(DEBUG) -I$(INCS) -l$(LIB)
	/usr/bin/gcc -c main.S -fverbose-asm -o main.o -ggdb -Wall -Wextra -O2 -I$(INCS) -l$(LIB)
	/usr/bin/gcc -c get_interface.S -fverbose-asm -o get_interface.o -ggdb -Wall -Wextra -O2 -I$(INCS) -l$(LIB)

$(ASS): $(INSTRUCTION)
#	$(GCC) -S $< $(CFLAGS) $@ $(DEBUG) -I$(INCS) -l$(LIB)
	/usr/bin/gcc -S main.i -o main.S -ggdb -Wall -Wextra -O2 -I$(INCS) -l$(LIB)
	/usr/bin/gcc -S get_interface.i -o get_interface.S -ggdb -Wall -Wextra -O2 -I$(INCS) -l$(LIB)

$(INSTRUCTION): %.i: %.c
#	$(GCC) -E $< $(CFLAGS) $@ $(DEBUG) -I$(INCS) -l$(LIB)
	/usr/bin/gcc -E main.c -o main.i -ggdb -Wall -Wextra -O2 -I$(INCS) -l$(LIB)
	/usr/bin/gcc -E get_interface.c -o get_interface.i -ggdb -Wall -Wextra -O2 -I$(INCS) -l$(LIB)

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
