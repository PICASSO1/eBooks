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
OBJS := main.o get_station.o
ASS := main.S get_station.S
INSTRUCTION := main.i get_station.i

# CFLAGS := -o
DEBUG := -ggdb -Wall -Wextra -O2

# for netlink nl80211
INCS := /usr/include/libnl3
LIB := nl-3 -lnl-genl-3
STATIC_LIB := 

.PHONY: all
all: $(BIN)

$(BIN): $(OBJS)
	$(GCC) -o $(BIN) $(OBJS) $(DEBUG) -I$(INCS) -l$(LIB)
#	/usr/bin/gcc -o main main.o get_station.o -ggdb -Wall -Wextra -O2 -I$(INCS) -l$(LIB)

$(OBJS): $(ASS)
#	$(GCC) -c $< -fverbose-asm -o $@ $(DEBUG)
	/usr/bin/gcc -c main.S -fverbose-asm -o main.o -ggdb -Wall -Wextra -O2
	/usr/bin/gcc -c get_station.S -fverbose-asm -o get_station.o -ggdb -Wall -Wextra -O2

$(ASS): $(INSTRUCTION)
#	$(GCC) -S $< -o $@ $(DEBUG)
	/usr/bin/gcc -S main.i -o main.S -ggdb -Wall -Wextra -O2
	/usr/bin/gcc -S get_station.i -o get_station.S -ggdb -Wall -Wextra -O2

$(INSTRUCTION): %.i: %.c
	$(GCC) -E $< -o $@ $(DEBUG) -I$(INCS)
#	/usr/bin/gcc -E main.c -o main.i -ggdb -Wall -Wextra -O2 -I$(INCS)
#	/usr/bin/gcc -E get_station.c -o get_station.i -ggdb -Wall -Wextra -O2 -I$(INCS)

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
