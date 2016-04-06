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

TARGET = boot.iso

include Makefile.in

CFLAGS = -Wall -Werror -O2 -I include -I kernel
LDFLAGS = --oformat binary
LIB = libc

STARTER_SIZE = 16384
BLK_SIZE = 512
BOOT_ADDR = 0x7c00
STARTER_SEG = 0x0800
STARTER_ADDR = 0x8000

OBJS = build/boot.img $(WD)/pad $(WD)/append \
	$(WD)/boot.bin $(WD)/boot.o $(WD)/starter.inc \
	$(WD)/kern.bin \
	$(WD)/starter.bin $(WD)/starter.o $(WD)/main.o

$(TARGET): build/boot.img
	mkisofs -R -b boot.img -o $(TARGET) build

build/boot.img: $(WD)/pad $(WD)/boot.bin $(WD)/kern.bin
	cat $(WD)/boot.bin $(WD)/kern.bin > $@
	$(WD)/pad $@ 1474560

$(WD)/boot.bin: $(WD)/boot.o
	$(LD) $(LDFLAGS) -L$(LIB) -Ttext $(BOOT_ADDR) -o $@ $^

$(WD)/boot.o: $(WD)/boot.s $(WD)/starter.inc

$(WD)/starter.inc: $(WD)/kern.bin
	echo ".set STARTER_SEGMENT, $(STARTER_SEG)" > $@
	echo ".set STARTER_ADDRESS, $(STARTER_ADDR)" >> $@
	echo -n ".set STARTER_SECTORS, " >> $@
	echo "(`wc -c < $(WD)/kern.bin` + $(BLK_SIZE) - 1) / $(BLK_SIZE)" | bc >> $@

$(WD)/starter.bin: $(WD)/starter.o $(WD)/main.o kernel/arch/libarch.a \
		kernel/mpu/libmpu.a lib/libnc.a
	$(LD) $(LDFLAGS) -Ttext $(STARTER_ADDR) -L$(LIB) -o $@ $^
	$(WD)/pad $@ $(STARTER_SIZE) 

$(WD)/main.o: $(WD)/main.c

$(WD)/pad: $(WD)/pad.c
	$(CC) -o $@ $^

$(WD)/append: $(WD)/append.c
	$(CC) -o $@ $^

$(WD)/kern.bin: $(WD)/append \
		$(WD)/starter.bin kernel/core.bin \
		servers/kernlog/kernlog \
		servers/timer/timer \
		servers/mm/mm \
		servers/fs/fs \
		servers/hmi/hmi \
		servers/ramdisk/ramdisk \
		initrd.img
	[ `wc -c < $(WD)/starter.bin` -le $(STARTER_SIZE) ]
	cp $(WD)/starter.bin $@
	$(WD)/append 1 kernel/core.bin 4 >> $@
	$(WD)/append 2 servers/kernlog/kernlog 4 >> $@
	$(WD)/append 2 servers/timer/timer 4 >> $@
	$(WD)/append 2 servers/mm/mm 4 >> $@
	$(WD)/append 2 servers/fs/fs 4 >> $@
	$(WD)/append 2 servers/hmi/hmi 4 >> $@
	$(WD)/append 4 initrd.img 4 >> $@
	$(WD)/append 2 servers/ramdisk/ramdisk 4 >> $@
	$(WD)/append 0 >> $@
