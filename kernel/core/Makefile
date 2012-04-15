# $Revision: 1.14 $
#
#
INC=../../include


#DEBUG=-DTSKSW_DEBUG
DEBUG=-DKEYBOARD_DEBUG
#IDENT=-DTIMER_TEST -DI386 -DAUTO_START -DIBMPC $(DEBUG)
IDENT=-DTIMER_TEST -DI386 -DIBMPC -DAUTO_START -DPORTMANAGER -DKERNLIB -DITRON $(DEBUG)

CPP=/lib/cpp
LD=ld
AS=as
CFLAGS=-O2 -fno-builtin -I. -Ih -Ii386 -Iibmpc -I$(INC) -Wall -fno-builtin $(IDENT)
CC=gcc
MKDEPEND=$(CC) -M
STARTADDR=0x80020000
ENTRY=startup

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.S
	$(CC) $(CFLAGS) -c $< -o $@

OBJS= \
	startup.o \
	devconfig.o \
	console.o \
	dummy.o \
	error.o \
	itron_memory.o \
	main.o \
	misc.o \
	kalloc.o \
	pmemory.o \
	printf.o \
	syscall.o \
	syscall-if.o \
	task.o \
	time.o \
	version.o \
	virtual_mem.o \
	semaphore.o \
	tss.o \
	fault.o \
	fpu.o \
	timer.o \
	io.o \
	gdt.o \
	task2.o \
	message.o \
	eventflag.o \
	locore.o \
	interrupt.o  \
	device.o \
	keyboard.o \
	dbg_funcs.o \
	system.o \
	lowlib.o \
	posix.o

OTHEROBJ=../lib/libkernel/port_manager.o ../lib/libkernel/message_port.o \
           ../lib/libkernel/sys_errno.o

SRCS=main.c memory.c task.c time.c version.c error.c dummy.c printf.c \
     console.c fault.c syscall.c misc.c timer.c \
     virtual_mem.c itron_memory.c pmemory.c \
     startup.S

all: core.bin

core.bin: $(OBJS) libkernel.a port_manager
	$(LD) -static -o $@ -e ${ENTRY} -Ttext=$(STARTADDR) $(OBJS) $(OTHEROBJ)

libkernel.a:
	${MAKE} -C ../lib/libkernel

port_manager:
	${MAKE} -C ../servers

tags:
	etags ../*/*.[hcs]

depend:
	$(MKDEPEND) -x c $(SRCS) > .depend

clean:
	rm -f $(OBJS)
	rm -f core.bin
	${MAKE} -C ../lib/libkernel clean
	${MAKE} -C ../servers clean

include CFILES
include SFILES
