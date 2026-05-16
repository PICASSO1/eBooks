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
OBJS := main.o trigger_scan.o wait_scan.o get_scan.o
ASS := main.S trigger_scan.S wait_scan.S  get_scan.S
INSTRUCTION := main.i trigger_scan.i wait_scan.i get_scan.i

DEBUG := -ggdb -Wall -Wextra -O2

# for netlink nl80211
INCS := /usr/include/libnl3
LIB := nl-3 -lnl-genl-3
STATIC_LIB := 

.PHONY: all
all: $(BIN)

$(BIN): $(OBJS)
#	$(GCC) -o $(BIN) $(OBJS) $(DEBUG) -I$(INCS) -l$(LIB)
	/usr/bin/gcc -o main main.o trigger_scan.o wait_scan.o get_scan.o -ggdb -Wall -Wextra -O2 -I$(INCS) -l$(LIB)

$(OBJS): $(ASS)
#	$(GCC) -c $< -fverbose-asm -o $@ $(DEBUG)
	/usr/bin/gcc -c main.S -fverbose-asm -o main.o -ggdb -Wall -Wextra -O2
	/usr/bin/gcc -c trigger_scan.S -fverbose-asm -o trigger_scan.o -ggdb -Wall -Wextra -O2
	/usr/bin/gcc -c get_scan.S -fverbose-asm -o get_scan.o -ggdb -Wall -Wextra -O2
	/usr/bin/gcc -c wait_scan.S -fverbose-asm -o wait_scan.o -ggdb -Wall -Wextra -O2

$(ASS): $(INSTRUCTION)
#	$(GCC) -S $< -o $@ $(DEBUG)
	/usr/bin/gcc -S main.i -o main.S -ggdb -Wall -Wextra -O2
	/usr/bin/gcc -S trigger_scan.i -o trigger_scan.S -ggdb -Wall -Wextra -O2
	/usr/bin/gcc -S get_scan.i -o get_scan.S -ggdb -Wall -Wextra -O2
	/usr/bin/gcc -S wait_scan.i -o wait_scan.S -ggdb -Wall -Wextra -O2

$(INSTRUCTION): %.i: %.c
	$(GCC) -E $< -o $@ $(DEBUG) -I$(INCS)
#	/usr/bin/gcc -E main.c -o main.i -ggdb -Wall -Wextra -O2 -I$(INCS)
#	/usr/bin/gcc -E get_scan.c -o get_scan.i -ggdb -Wall -Wextra -O2 -I$(INCS)
#	/usr/bin/gcc -E wait_scan.c -o wait_scan.i -ggdb -Wall -Wextra -O2 -I$(INCS)

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
