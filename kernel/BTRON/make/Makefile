

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
	cd ../../BTRON/device/console; ${MAKE} clean
	cd ../../BTRON/device/keyboard; ${MAKE} clean
	cd ../../BTRON/device/fd765a; ${MAKE} clean
	cd ../../BTRON/device/ide; ${MAKE} clean
	cd ../../BTRON/device/ne2000; ${MAKE} clean
	cd ../../BTRON/device/rs232c; ${MAKE} clean
	cd ../../BTRON/device/beep; ${MAKE} clean
	cd ../../BTRON/device/hcm_pci; ${MAKE} clean
	cd ../../BTRON/device/pcmcia; ${MAKE} clean
	cd ../../ITRON/make; ${MAKE} clean
	cd ../../BTRON/init; ${MAKE} clean
	cd ../../POSIX/manager; ${MAKE} clean
	cd ../../POSIX/servers/wconsole; ${MAKE} clean
	cd ../../POSIX/servers/ramdisk; ${MAKE} clean
	cd ../../POSIX/lowlib; ${MAKE} clean
	cd ../../POSIX/libc; ${MAKE} clean
	cd ../../BTRON/device/psaux; ${MAKE} clean

dummy:
	echo '*dummy*'

$(DRIVERS):
	cd ../device/$@; ${MAKE}

itron:
	cd ../../ITRON/make; ${MAKE}

posix_lowlib:
	cd ../../POSIX/lowlib; ${MAKE}

$(SERVERS):
	cd ../../POSIX/servers/$@; ${MAKE}

posix_manager:
	cd ../../POSIX/manager; ${MAKE}

posix_libc:
	cd ../../POSIX/libc; ${MAKE}

init:
	cd ../init; ${MAKE}

