

#DEBUG=-DDEBUG


INC=../kernel/ITRON/h
BUILDSRC=build.c $(INC)/itron_module.h
#CFLAGS=$(DEBUG) -I$(INC)
CFLAGS=$(DEBUG)
CC=gcc

KERNLIB=kernlib
MKERNEL=itron
SERVER=port-manager
# DRIVERS=console keyboard fd765a ide ne2000 rs232c beep hcm_pci pcmcia psaux
DRIVERS=console keyboard fd765a ide psaux 
LOWLIB=posix_lowlib
USERLIBS=posix_libc
SERVERS = wconsole ramdisk
MANAGERS=posix_manager $(SERVERS)
USERAP=init


PROGRAMS=$(MKERNEL) $(DRIVERS) $(LOWLIB) $(USERLIBS) $(MANAGERS) $(USERAP)

all: $(PROGRAMS) btron 

btron: build config.tab dummy
	./build config.tab btron

build: build.o $(BUILDSRC)
	$(CC) $(CFLAGS) -o build build.o

clean:
	rm -f build build.o btron
	${MAKE} -C ../kernel/BTRON/device/console clean
	${MAKE} -C ../kernel/BTRON/device/keyboard clean
	${MAKE} -C ../kernel/BTRON/device/fd765a clean
	${MAKE} -C ../kernel/BTRON/device/ide clean
	${MAKE} -C ../kernel/ITRON/make clean
	${MAKE} -C ../kernel/init clean
	${MAKE} -C ../kernel/POSIX/manager clean
	${MAKE} -C ../kernel/POSIX/servers/wconsole clean
	${MAKE} -C ../kernel/POSIX/servers/ramdisk clean
	${MAKE} -C ../kernel/POSIX/lowlib clean
	${MAKE} -C ../kernel/POSIX/libc clean
	${MAKE} -C ../kernel/BTRON/device/psaux clean

dummy:
	echo '*dummy*'

$(DRIVERS):
	${MAKE} -C ../kernel/BTRON/device/$@

itron:
	${MAKE} -C ../kernel/ITRON/make

posix_lowlib:
	${MAKE} -C ../kernel/POSIX/lowlib

$(SERVERS):
	${MAKE} -C ../kernel/POSIX/servers/$@

posix_manager:
	${MAKE} -C ../kernel/POSIX/manager

posix_libc:
	${MAKE} -C ../kernel/POSIX/libc

init:
	${MAKE} -C ../kernel/init

