

#DEBUG=-DDEBUG


INC=../../ITRON/h
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
	${MAKE} -C ../../BTRON/device/console clean
	${MAKE} -C ../../BTRON/device/keyboard clean
	${MAKE} -C ../../BTRON/device/fd765a clean
	${MAKE} -C ../../BTRON/device/ide clean
	${MAKE} -C ../../ITRON/make clean
	${MAKE} -C ../../BTRON/init clean
	${MAKE} -C ../../POSIX/manager clean
	${MAKE} -C ../../POSIX/servers/wconsole clean
	${MAKE} -C ../../POSIX/servers/ramdisk clean
	${MAKE} -C ../../POSIX/lowlib clean
	${MAKE} -C ../../POSIX/libc clean
	${MAKE} -C ../../BTRON/device/psaux clean

dummy:
	echo '*dummy*'

$(DRIVERS):
	${MAKE} -C ../device/$@

itron:
	${MAKE} -C ../../ITRON/make

posix_lowlib:
	${MAKE} -C ../../POSIX/lowlib

$(SERVERS):
	${MAKE} -C ../../POSIX/servers/$@

posix_manager:
	${MAKE} -C ../../POSIX/manager

posix_libc:
	${MAKE} -C ../../POSIX/libc

init:
	${MAKE} -C ../init

