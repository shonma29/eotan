

#DEBUG=-DDEBUG


INC=../kernel/core
BUILDSRC=build.c $(INC)/boot.h
CFLAGS=$(DEBUG) -I$(INC) -I../include
CC=gcc

MKERNEL=core
KSERVERS=console keyboard fd ide psaux port-manager
LOWLIB=posix_lowlib
USERLIBS=posix_libc
USERVERS = posix_manager ramdisk
USERAP=init


PROGRAMS=$(MKERNEL) $(DRIVERS) $(LOWLIB) $(USERLIBS) \
		$(KSERVERS) $(USERVERS) $(USERAP)

all: $(PROGRAMS) kern.bin 

kern.bin: build config.tab dummy
	./build config.tab $@

build: $(BUILDSRC)
	$(CC) $(CFLAGS) -o $@ build.c

clean:
	rm -f build build.o kern.bin
	${MAKE} -C ../kernel/servers/console clean
	${MAKE} -C ../kernel/servers/keyboard clean
	${MAKE} -C ../kernel/servers/fd clean
	${MAKE} -C ../kernel/servers/ide clean
	${MAKE} -C ../kernel/servers/psaux clean
	${MAKE} -C ../kernel/core clean
	${MAKE} -C ../kernel/init clean
	${MAKE} -C ../kernel/lib/libnc clean
	${MAKE} -C ../servers/fs clean
	${MAKE} -C ../servers/ramdisk clean
	${MAKE} -C ../lib/lowlib clean
	${MAKE} -C ../lib/libc clean

dummy:
	echo '*dummy*'

$(KSERVERS):
	${MAKE} -C ../kernel/servers/$@

core:
	${MAKE} -C ../kernel/core

posix_lowlib:
	${MAKE} -C ../lib/lowlib

$(USERVERS):
	${MAKE} -C ../servers/$@

posix_manager:
	${MAKE} -C ../servers/fs

posix_libc:
	${MAKE} -C ../lib/libc

init:
	${MAKE} -C ../kernel/init

