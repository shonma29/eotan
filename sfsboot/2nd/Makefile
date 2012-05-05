GSTRIP = /usr/bin/strip
RM = /bin/rm

.SUFFIXES : .asm

.asm.o : 
	nasm -f elf $<


CSRC=main.c console.c keyboard.c memory.c \
     date.c idt.c cinterrupt.c fd.c \
     dma.c page.c lib.c ide.c \
     evaluate.c bootos.c time.c elf.c

OBJ32 = main.o console.o keyboard.o memory.o  \
	date.o io.o idt.o interrupt.o cinterrupt.o fd.o \
	dma.o page.o paging.o lib.o ide.o sfs.o \
	evaluate.o bootos.o time.o elf.o
LIBS =

OBJ16 = start16.o

TOOLS = build_boot2

START_ADDR = 0x380000

COMMON = ../../include
ITRON = ../../kernel/core
INCPATH = -I. -I$(COMMON) -I$(ITRON)

NOFLAGS=-c -m386 -msoft-float -fno-builtin -Wall -D__LINUX__ -DSUPPORT_144 $(DEBUG) -DIBMPC $(HDBOOT) $(INCPATH)
CFLAGS=-O $(NOFLAGS)

LDOPT32 = -n -Ttext=$(START_ADDR) -static -e _main

########################################################################
#
#	2ndboot ...... 最終的に作られる Second BOOT イメージ
#
2ndboot: $(TOOLS) init mode32
	objcopy -O binary mode32 mode32.raw
	./build_boot2 init mode32.raw 2ndboot

init: start16 start32
	cat start16 start32 > init

start16: start16.asm 
	nasm start16.asm 

start32: start32.asm
	nasm start32.asm 

mode32: $(OBJ32) 
	$(LD) $(LDOPT32) -o mode32 $(OBJ32) $(LIBS)
	cp mode32 mode32.sym
	$(GSTRIP) mode32

memory.o: memory.c
	$(CC) $(NOFLAGS) memory.c -o $@

#fd.o: fd.c
#	$(CC) $(NOFLAGS) fd.c -o $@

#fdc.o: fdc.c
#	$(CC) $(NOFLAGS) fdc.c -o $@

clean:
	$(RM) -f $(OBJ32) $(OBJ16)
	$(RM) -f core mode32 start16 start32 build_boot2 mode32.raw mode32.sym
	$(RM) -f 2ndboot init 

tags:
	$(ETAGS) $(CSRC)

mkdepend:
	$(CC) -M -D__LINUX__ $(CSRC) > DEPEND

#######################################################################
#
#		T O O L S
#

build_boot2: build_boot2.c
	$(CC) -D__LINUX__ -DGCC_USE -o build_boot2 build_boot2.c


#include DEPEND





