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

all: boot.iso hdboot.img

include Makefile.in

LDFLAGS = --oformat binary
LIB = libc

BLK_SIZE = 512
LOADER_ADDR = 0x7c00
STARTER_SEG = 0x0800
STARTER_ADDR = 0x8000
STARTER_SIZE = 16384
BOOT_ADDR = 0x8000c000
BOOT_SIZE = 4096
BIN_SIZE = `$(ECHO) "$(STARTER_SIZE) + $(BOOT_SIZE)" | bc`
MAX_KERNEL = `$(ECHO) "(640 - 32 - 20 - 4) * 1024" | bc`
PARTITION_BOOT_TYPE = 0x0d
PARTITION_BOOT_SECTORS = `$(ECHO) "(640 * 1024) / $(BLK_SIZE)" | bc`
SIZE_UNTIL_BOOT = `$(ECHO) "640 * 1024 + $(BLK_SIZE)" | bc`
PARTITION_TFS_TYPE = 0xfa
PARTITION_TFS_SECTORS = `$(ECHO) "(512 * 1024) / $(BLK_SIZE)" | bc`
SIZE_UNTIL_TFS = `$(ECHO) "(640 + 512) * 1024 + $(BLK_SIZE)" | bc`

OBJS = boot.iso build/fdboot.img hdboot.img BOOTX64.EFI \
	$(WD)/pad $(WD)/append $(WD)/elf2module \
	$(WD)/fdloader $(WD)/fdloader.o $(WD)/loader.inc $(WD)/hdloader.inc \
	$(WD)/hdloader $(WD)/hdloader.o \
	$(WD)/kernel.img $(WD)/hdkernel.img \
	$(WD)/starter.bin $(WD)/starter $(WD)/starter.o \
	$(WD)/protect.o $(WD)/protect.inc \
	$(WD)/memory.o $(WD)/memory_info.o $(WD)/decode.o \
	$(WD)/boot $(WD)/boot.o \
	$(WD)/BOOTX64.EFI $(WD)/uefiloader.o \
	$(WD)/container.o

boot.iso: build/fdboot.img
	mkisofs -R -b fdboot.img -o $@ build

build/fdboot.img: $(WD)/pad $(WD)/fdloader $(WD)/kernel.img
	[ `wc -c < $(WD)/kernel.img` -le $(MAX_KERNEL) ]
	cat $(WD)/fdloader $(WD)/kernel.img > $@
	$(WD)/pad $@ 1474560

$(WD)/fdloader: $(WD)/fdloader.o
	$(LD) $(LDFLAGS) -L$(LIB) -Ttext $(LOADER_ADDR) -o $@ $^

$(WD)/fdloader.o: $(WD)/fdloader.s $(WD)/loader.inc

hdboot.img: $(WD)/pad $(WD)/hdloader $(WD)/hdkernel.img initrd.img
	[ `wc -c < $(WD)/hdkernel.img` -le $(MAX_KERNEL) ]
	cat $(WD)/hdloader $(WD)/hdkernel.img > $@
	$(WD)/pad $@ $(SIZE_UNTIL_BOOT)
	cat initrd.img >> $@
	$(WD)/pad $@ $(SIZE_UNTIL_TFS)

$(WD)/hdloader: $(WD)/hdloader.o
	$(LD) $(LDFLAGS) -L$(LIB) -Ttext $(LOADER_ADDR) -o $@ $^

$(WD)/hdloader.o: $(WD)/hdloader.s $(WD)/hdloader.inc

$(WD)/loader.inc: $(WD)/kernel.img
	$(ECHO) ".set STARTER_SEGMENT, $(STARTER_SEG)" > $@
	$(ECHO) ".set STARTER_ADDRESS, $(STARTER_ADDR)" >> $@
	$(ECHO) -n ".set STARTER_SECTORS, " >> $@
	$(ECHO) "(`wc -c < $(WD)/kernel.img` + $(BLK_SIZE) - 1) / $(BLK_SIZE)" | bc >> $@

$(WD)/hdloader.inc: $(WD)/hdkernel.img
	$(ECHO) ".set STARTER_SEGMENT, $(STARTER_SEG)" > $@
	$(ECHO) ".set STARTER_ADDRESS, $(STARTER_ADDR)" >> $@
	$(ECHO) -n ".set STARTER_SECTORS, " >> $@
	$(ECHO) "(`wc -c < $(WD)/hdkernel.img` + $(BLK_SIZE) - 1) / $(BLK_SIZE)" | bc >> $@
	$(ECHO) ".set PARTITION_BOOT_TYPE, $(PARTITION_BOOT_TYPE)" >> $@
	$(ECHO) ".set PARTITION_BOOT_SECTORS, $(PARTITION_BOOT_SECTORS)" >> $@
	$(ECHO) ".set PARTITION_TFS_TYPE, $(PARTITION_TFS_TYPE)" >> $@
	$(ECHO) ".set PARTITION_TFS_SECTORS, $(PARTITION_TFS_SECTORS)" >> $@

$(WD)/protect.o: $(WD)/protect.s $(WD)/protect.inc

$(WD)/protect.inc: $(WD)/protect.h
	$(CPP) -P $(CFLAGS) $^ > $@

$(WD)/starter: $(WD)/protect.o $(WD)/starter.o $(WD)/memory.o \
		$(WD)/memory_info.o $(WD)/decode.o \
		drivers/arch/libarch.a lib/librc.a \
		drivers/mpu/libmpu.a lib/libnc.a
	$(LD) $(LDFLAGS) -Ttext $(STARTER_ADDR) -L$(LIB) -o $@ $^

$(WD)/boot: $(WD)/boot.o \
		drivers/mpu/libmpu.a lib/libnc.a
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
		servers/interrupt/interrupt \
		servers/timer/timer \
		servers/dev/dev \
		servers/mm/mm \
		servers/fs/fs \
		servers/hmi/hmi \
		servers/console/console \
		servers/pipe/pipe \
		servers/monitor/monitor \
		initrd.img
	[ `wc -c < $(WD)/starter.bin` -le $(BIN_SIZE) ]
	cp $(WD)/starter.bin $@
	$(WD)/elf2module kernel/core.bin | $(WD)/append $@ 1
	$(WD)/elf2module servers/interrupt/interrupt | $(WD)/append $@ 2 3
	$(WD)/elf2module servers/timer/timer | $(WD)/append $@ 2 4
	$(WD)/elf2module servers/dev/dev | $(WD)/append $@ 2 6
#	$(WD)/elf2module servers/monitor/monitor | $(WD)/append $@ 3 14
	$(WD)/elf2module servers/hmi/hmi | $(WD)/append $@ 3 12
	$(WD)/elf2module servers/console/console | $(WD)/append $@ 3 10
	$(WD)/elf2module servers/pipe/pipe | $(WD)/append $@ 3 11
	$(WD)/elf2module servers/fs/fs | $(WD)/append $@ 3 9
	$(WD)/elf2module servers/mm/mm | $(WD)/append $@ 3 7
	cat initrd.img | $(WD)/append $@ 5
	$(WD)/append $@ 0

$(WD)/hdkernel.img: $(WD)/append $(WD)/elf2module \
		$(WD)/starter.bin kernel/core.bin \
		servers/interrupt/interrupt \
		servers/timer/timer \
		servers/dev/dev \
		servers/mm/mm \
		servers/fs/fs \
		servers/hmi/hmi \
		servers/console/console \
		servers/pipe/pipe \
		servers/monitor/monitor
	[ `wc -c < $(WD)/starter.bin` -le $(BIN_SIZE) ]
	cp $(WD)/starter.bin $@
	$(WD)/elf2module kernel/core.bin | $(WD)/append $@ 1
	$(WD)/elf2module servers/interrupt/interrupt | $(WD)/append $@ 2 3
	$(WD)/elf2module servers/timer/timer | $(WD)/append $@ 2 4
	$(WD)/elf2module servers/dev/dev | $(WD)/append $@ 2 6
#	$(WD)/elf2module servers/monitor/monitor | $(WD)/append $@ 3 14
	$(WD)/elf2module servers/hmi/hmi | $(WD)/append $@ 3 12
	$(WD)/elf2module servers/console/console | $(WD)/append $@ 3 10
	$(WD)/elf2module servers/pipe/pipe | $(WD)/append $@ 3 11
	$(WD)/elf2module servers/fs/fs | $(WD)/append $@ 3 9
	$(WD)/elf2module servers/mm/mm | $(WD)/append $@ 3 7
	$(WD)/append $@ 0

EFI_CFLAGS = -Wall -Werror -target x86_64-pc-win32-coff -O2 -I include
EFI_ASFLAGS = --target=x86_64-pc-win32-coff
EFI_LD = /usr/local/opt/llvm/bin/ld.lld
EFI_LDFLAGS = -flavor link -subsystem:efi_application

BOOTX64.EFI: $(WD)/uefiloader.o $(WD)/container.o
	$(EFI_LD) $(EFI_LDFLAGS) -entry:start -out:$@ $^

$(WD)/uefiloader.o: $(WD)/uefiloader.c
	$(CC) $(EFI_CFLAGS) -o $@ -c $^

$(WD)/container.o: $(WD)/container.s $(WD)/kernel.img
	$(AS) $(EFI_ASFLAGS) -o $@ $(WD)/container.s
