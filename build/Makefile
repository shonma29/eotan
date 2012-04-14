

#DEBUG=-DDEBUG


INC=../kernel/ITRON/h
BUILDSRC=build.c $(INC)/itron_module.h
CFLAGS=$(DEBUG)
CC=gcc

KERNLIB=kernlib
MKERNEL=itron
SERVER=port-manager
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
	${MAKE} -C ../kernel/drivers/console clean
	${MAKE} -C ../kernel/drivers/keyboard clean
	${MAKE} -C ../kernel/drivers/fd765a clean
	${MAKE} -C ../kernel/drivers/ide clean
	${MAKE} -C ../kernel/ITRON clean
	${MAKE} -C ../kernel/init clean
	${MAKE} -C ../servers/fs clean
	${MAKE} -C ../servers/wconsole clean
	${MAKE} -C ../servers/ramdisk clean
	${MAKE} -C ../lib/lowlib clean
	${MAKE} -C ../lib/libc clean
	${MAKE} -C ../kernel/drivers/psaux clean

dummy:
	echo '*dummy*'

$(DRIVERS):
	${MAKE} -C ../kernel/drivers/$@

itron:
	${MAKE} -C ../kernel/ITRON

posix_lowlib:
	${MAKE} -C ../lib/lowlib

$(SERVERS):
	${MAKE} -C ../servers/$@

posix_manager:
	${MAKE} -C ../servers/fs

posix_libc:
	${MAKE} -C ../lib/libc

init:
	${MAKE} -C ../kernel/init

