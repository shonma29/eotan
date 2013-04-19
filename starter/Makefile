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

TARGET = ../boot.iso
MAKE = gmake
CC = cc
CFLAGS = -Wall -Werror -O2 -I ../include -I ../kernel #-DUSE_VESA
AS = as
ASFLAGS = 
LD = ld
LDFLAGS = --oformat binary
LIB = ../libc/
TOOL=./

STARTER_SIZE = 16384
BLK_SIZE = 512
BOOT_ADDR = 0x7c00
STARTER_SEG = 0x0800
STARTER_ADDR = 0x8000

OBJS = boot.bin kern.bin

all: pad append $(TARGET)

$(TARGET): ../build/boot.img
	mkisofs -R -b boot.img -o $(TARGET) ../build

../build/boot.img: $(OBJS)
	cat $(OBJS) > $@
	$(TOOL)pad $@ 1474560

boot.bin: boot.o
	$(LD) $(LDFLAGS) -L$(LIB) -Ttext $(BOOT_ADDR) -o $@ $^

boot.o: boot.s starter.inc

starter.inc: kern.bin
	echo ".set STARTER_SEGMENT, $(STARTER_SEG)" > $@
	echo ".set STARTER_ADDRESS, $(STARTER_ADDR)" >> $@
	echo -n ".set STARTER_SECTORS, " >> $@
	echo "(`wc -c < kern.bin` + $(BLK_SIZE) - 1) / $(BLK_SIZE)" | bc >> $@

starter.bin: starter.o main.o ../kernel/arch/libarch.a ../lib/libnc.a
	$(LD) $(LDFLAGS) -Ttext $(STARTER_ADDR) -L$(LIB) -o $@ \
		starter.o main.o ../kernel/mpu/libmpu.a \
		../kernel/arch/libarch.a ../lib/libnc.a
	$(TOOL)pad $@ $(STARTER_SIZE) 

main.o: main.c

$(TOOL)pad: $(TOOL)pad.c
	$(CC) -o $@ $^

$(TOOL)append: $(TOOL)append.c
	$(CC) -o $@ $^

kern.bin: starter.bin ../kernel/core.bin ../servers/fs/fs \
		../servers/hmi/hmi ../servers/keyboard/keyboard \
		../servers/ramdisk/ramdisk \
		../initrd.img
	[ `wc -c < starter.bin` -le $(STARTER_SIZE) ]
	cp starter.bin $@
	$(TOOL)append 1 ../kernel/core.bin 4 >> $@
	$(TOOL)append 4 ../initrd.img 4 >> $@
	$(TOOL)append 2 ../servers/kernlog/kernlog 4 >> $@
	$(TOOL)append 2 ../servers/fs/fs 4 >> $@
	$(TOOL)append 2 ../servers/hmi/hmi 4 >> $@
	$(TOOL)append 2 ../servers/keyboard/keyboard 4 >> $@
	$(TOOL)append 2 ../servers/ramdisk/ramdisk 4 >> $@
	$(TOOL)append 0 >> $@

.PHONY: clean
clean:
	$(RM) $(TARGET) ../build/*.img *.bin *.o *.inc
	$(RM) pad append
