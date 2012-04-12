#
#	Makefile for boot image.
#

ROOT=../kernel
ECFLAGS=-Wall -I. -I$(ROOT) -I$(ROOT)/ITRON -I$(ROOT)/ITRON/h -I$(ROOT)/ITRON/i386 -I$(ROOT)/POSIX/libc/others -DEOTA $(IDENT)
STARTADDR=0x00001000
ENTRY=_main
KERNLIBS=$(ROOT)/POSIX/libc/libc.a $(ROOT)/ITRON/kernlib/libkernel.a 

NEWLIB=../../libc/newlib-1.8.1-eota/eota/newlib
NCFLAGS=-Wall -I$(NEWLIB)/targ-include -I$(NEWLIB)/libc/include -DEOTA
NENTRY=_start
NLIBS=$(NEWLIB)/crt0.o $(NEWLIB)/libc.a
NKERNLIBS=$(ROOT)/POSIX/libc/libnative.a

%.eo: %.c
	$(CC) $(ECFLAGS) -c $< -o $@

%.no: %.c
	$(CC) $(NCFLAGS) -c $< -o $@

CFLAGS=-D__LINUX__

all: 1stboot 2ndboot mkmap mkboot mkhdboot

1stboot:
	${MAKE} -C 1st 1stboot

2ndboot:
	${MAKE} -C 2nd 2ndboot

mkmap: mkmap.c
	$(CC) -o mkmap mkmap.c

mkboot: mkboot.c
	$(CC) -o mkboot mkboot.c

mkhdboot: mkhdboot.c
	$(CC) -o mkhdboot mkhdboot.c $(LINUX_INCLUDE)

modboot: modboot.c
	$(CC) -o modboot modboot.c

modboot-e: modboot.no $(NLIBS) $(NKERNLIBS)
	$(LD) -Bstatic -o $@ -e ${NENTRY} -Ttext=$(STARTADDR) modboot.no $(NLIBS) $(NKERNLIBS) 
	strip $@
clean:
	${MAKE} -C 1st clean
	${MAKE} -C 2nd clean
	rm -f mkmap mkboot mkhdboot

