ROOT=../..
ECFLAGS=-O -Wall -I. -I$(ROOT)/kernel -I$(ROOT)/kernel/ITRON -I$(ROOT)/kernel/ITRON/h -I$(ROOT)/kernel/ITRON/i386 -I$(ROOT)/lib/libc/others -DEOTA $(IDENT)
STARTADDR=0x00001000
ENTRY=_main
KERNLIBS=$(ROOT)/lib/libc/libc.a #$(ROOT)/kernel/ITRON/kernlib/libitron.a 

NEWLIB=../../../../libc/newlib-1.8.1-eota/eota/newlib
NCFLAGS=-O -Wall -I$(NEWLIB)/targ-include -I$(NEWLIB)/libc/include -DEOTA
NENTRY=_start
NLIBS=$(NEWLIB)/crt0.o $(NEWLIB)/libc.a
NKERNLIBS=$(ROOT)/lib/libc/libnative.a

%.eo: %.c
	$(CC) $(ECFLAGS) -c $< -o $@

%.no: %.c
	$(CC) $(NCFLAGS) -c $< -o $@

CFLAGS=-Wall

all: mksfs statfs sfsck
eota: mksfs-e statfs-e sfsck-e

user_inst:
	cp mksfs mksfs.sh  /usr/local/bin/
	rm -f /usr/local/bin/mkfs.sfs
	ln mksfs.sh /usr/local/bin/mkfs.sfs
	cp statfs sfsck /usr/local/bin

mksfs: mkfs.o
	cc $(CFLAGS) -o mksfs mkfs.o

statfs: statfs.o
	cc $(CFLAGS) -o statfs statfs.o

#statfs: statfs.c
#	cc -o statfs statfs.c

mksfs-e: mkfs.no $(NLIBS) $(NKERNLIBS)
	$(LD) -Bstatic -o $@ -e ${NENTRY} -Ttext=$(STARTADDR) mkfs.no $(NLIBS) $(NKERNLIBS) 
	strip $@

statfs-e: statfs.no $(NLIBS) $(NKERNLIBS)
	$(LD) -Bstatic -o $@ -e ${NENTRY} -Ttext=$(STARTADDR) statfs.no $(NLIBS) $(NKERNLIBS)
	strip $@

sfsck-e: sfsck.no $(NLIBS) $(NKERNLIBS)
	$(LD) -Bstatic -o $@ -e ${NENTRY} -Ttext=$(STARTADDR) sfsck.no $(NLIBS) $(NKERNLIBS) 
	strip $@

clean:
	rm -f statfs mksfs sfsck
	rm -f *.o *.eo *.no
	rm -f mksfs-e statfs-e
	rm -f *.img *.bin
