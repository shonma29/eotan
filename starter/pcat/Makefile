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

CFLAGS += -I kernel
LDFLAGS = --oformat binary
LIB = libc

BLK_SIZE = 512
LOADER_ADDR = 0x7c00
STARTER_SEG = 0x0800
STARTER_ADDR = 0x8000
STARTER_SIZE = 16384
BOOT_ADDR = 0x8000c000
BOOT_SIZE = 4096
BIN_SIZE = `echo "$(STARTER_SIZE) + $(BOOT_SIZE)" | bc`
MAX_KERNEL = `echo "(640 - 32 - 20 - 4) * 1024" | bc`

OBJS = build/boot.img $(WD)/pad $(WD)/append $(WD)/elf2module \
	$(WD)/loader $(WD)/loader.o $(WD)/loader.inc \
	$(WD)/kernel.img \
	$(WD)/starter.bin $(WD)/starter $(WD)/protect.o $(WD)/starter.o \
	$(WD)/memory.o $(WD)/decode.o \
	$(WD)/boot $(WD)/boot.o

$(TARGET): build/boot.img
	mkisofs -R -b boot.img -o $(TARGET) build

build/boot.img: $(WD)/pad $(WD)/loader $(WD)/kernel.img
	[ `wc -c < $(WD)/kernel.img` -le $(MAX_KERNEL) ]
	cat $(WD)/loader $(WD)/kernel.img > $@
	$(WD)/pad $@ 1474560

$(WD)/loader: $(WD)/loader.o
	$(LD) $(LDFLAGS) -L$(LIB) -Ttext $(LOADER_ADDR) -o $@ $^

$(WD)/loader.o: $(WD)/loader.s $(WD)/loader.inc

$(WD)/loader.inc: $(WD)/kernel.img
	echo ".set STARTER_SEGMENT, $(STARTER_SEG)" > $@
	echo ".set STARTER_ADDRESS, $(STARTER_ADDR)" >> $@
	echo -n ".set STARTER_SECTORS, " >> $@
	echo "(`wc -c < $(WD)/kernel.img` + $(BLK_SIZE) - 1) / $(BLK_SIZE)" | bc >> $@

$(WD)/starter: $(WD)/protect.o $(WD)/starter.o $(WD)/memory.o $(WD)/decode.o \
		kernel/arch/libarch.a lib/librc.a \
		kernel/mpu/libmpu.a lib/libnc.a
	$(LD) $(LDFLAGS) -Ttext $(STARTER_ADDR) -L$(LIB) -o $@ $^

$(WD)/boot: $(WD)/boot.o
	$(LD) $(LDFLAGS) -Ttext $(BOOT_ADDR) -o $@ $^

$(WD)/starter.bin: $(WD)/starter $(WD)/boot
	[ `wc -c < $(WD)/starter` -le $(STARTER_SIZE) ]
	cp $(WD)/starter $@
	$(WD)/pad $@ $(STARTER_SIZE)
	[ `wc -c < $(WD)/boot` -le $(BOOT_SIZE) ]
	cat $(WD)/boot >> $@
	$(WD)/pad $@ $(BIN_SIZE)

$(WD)/starter.o: $(WD)/starter.c

$(WD)/pad: $(WD)/pad.c
	$(CC) -o $@ $^

$(WD)/append: $(WD)/append.c
	$(CC) -o $@ $^ lib/libnc.a lib/librc.a

$(WD)/elf2module: $(WD)/elf2module.c
	$(CC) -o $@ $^ lib/libnc.a

$(WD)/kernel.img: $(WD)/append $(WD)/elf2module \
		$(WD)/starter.bin kernel/core.bin \
		servers/kernlog/kernlog \
		servers/interrupt/interrupt \
		servers/timer/timer \
		servers/dev/dev \
		servers/mm/mm \
		servers/fs/fs \
		servers/hmi/hmi \
		drivers/ramdisk/ramdisk \
		drivers/monitor/monitor \
		initrd.img
	[ `wc -c < $(WD)/starter.bin` -le $(BIN_SIZE) ]
	cp $(WD)/starter.bin $@
	$(WD)/elf2module kernel/core.bin | $(WD)/append $@ 1
	$(WD)/elf2module servers/interrupt/interrupt | $(WD)/append $@ 3 3
	$(WD)/elf2module servers/timer/timer | $(WD)/append $@ 3 4
	$(WD)/elf2module servers/kernlog/kernlog | $(WD)/append $@ 3 5
	$(WD)/elf2module servers/dev/dev | $(WD)/append $@ 3 6
	$(WD)/elf2module servers/hmi/hmi | $(WD)/append $@ 3 9
	$(WD)/elf2module servers/fs/fs | $(WD)/append $@ 3 8
	$(WD)/elf2module servers/mm/mm | $(WD)/append $@ 3 7
	$(WD)/elf2module drivers/ramdisk/ramdisk | $(WD)/append $@ 2
	$(WD)/elf2module drivers/monitor/monitor | $(WD)/append $@ 2
	cat initrd.img | $(WD)/append $@ 5
	$(WD)/append $@ 0
