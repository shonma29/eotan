# This is free and unencumbered software released into the public domain.
#
# Anyone is free to copy, modify, publish, use, compile, sell, or
# distribute this software, either in source code form or as a compiled
# binary, for any purpose, commercial or non-commercial, and by any
# means.
#
# In jurisdictions that recognize copyright laws, the author or authors
# of this software dedicate any and all copyright interest in the
# software to the public domain. We make this dedication for the benefit
# of the public at large and to the detriment of our heirs and
# successors. We intend this dedication to be an overt act of
# relinquishment in perpetuity of all present and future rights to this
# software under copyright law.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
# OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.
#
# For more information, please refer to <http://unlicense.org/>

TARGET = boot.img
MAKE = gmake
CC = cc
CFLAGS = -Wall -Werror -O2 -I ../include
AS = as
ASFLAGS = 
LD = ld
LDFLAGS = --oformat binary
LIB = ../libc/
TOOL=./

OBJS = boot.bin starter.bin

all: pad append $(TARGET)

$(TARGET): $(OBJS)
	cat $(OBJS) > $@
	$(TOOL)pad $@ 1474560

boot.bin: boot.o
	$(LD) $(LDFLAGS) -L$(LIB) -Ttext 0x7c00 -o $@ $^

boot.o: boot.s starter.inc

starter.inc: starter.bin
	echo -n ".set STARTUP_SECTORS, " > $@
	echo "(`ls -l starter.bin | sed -e 's/ \{1,\}/ /g' | cut -f5 -d ' '` + 511 ) / 512" | bc >> $@

starter.bin: starter.o main.o ../kernel/arch/libarch.a ../lib/libnc.a
	$(LD) $(LDFLAGS) -Ttext 0x8000 -L$(LIB) -o $@ $^
	$(TOOL)pad $@ 512

main.o: main.c
console.o: console.c

$(TOOL)pad: $(TOOL)pad.c
	$(CC) -o $@ $^

$(TOOL)append: $(TOOL)append.c
	$(CC) -o $@ $^

kern.bin:
	$(RM) $@
	$(TOOL)append 1 ../kernel/core.bin 4 >> $@
	$(TOOL)append 3 ../servers/fs/fs 4 >> $@
	$(TOOL)append 2 ../servers/port-manager/port-manager 4 >> $@
	$(TOOL)append 2 ../servers/console/console 4 >> $@
	$(TOOL)append 2 ../servers/keyboard/keyboard 4 >> $@
	$(TOOL)append 2 ../servers/psaux/psaux 4 >> $@
	$(TOOL)append 2 ../servers/ramdisk/ramdisk 4 >> $@
	$(TOOL)append 2 ../servers/fd765a/fd765a 4 >> $@
	$(TOOL)append 2 ../servers/ide/ide 4 >> $@
	$(TOOL)append 3 ../app/init/init 4 >> $@
	$(TOOL)append 4 ../initrd.img 4 >> $@
	$(TOOL)append 0 >> $@

.PHONY: clean kern.bin
clean:
	$(RM) -f $(TARGET) *.bin *.o *.inc
	$(RM) -f pad append
