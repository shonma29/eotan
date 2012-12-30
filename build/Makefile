

#DEBUG=-DDEBUG


INC=../kernel
BUILDSRC=build.c $(INC)/boot.h
CFLAGS=$(DEBUG) -Wall -Werror -I$(INC)
CC=gcc

KERNEL=core
SERVERS=console keyboard fd ide psaux port-manager fs ramdisk dumb
KERNLIB=libserv nano-c itron
LOWLIB=posix_lowlib
USERLIBS=posix_libc
USERAPP=init


PROGRAMS=$(KERNEL) $(KERNLIB) $(LOWLIB) $(USERLIBS) \
		$(SERVERS) $(USERAPP)

all: $(PROGRAMS) kern.bin 

kern.bin: build config.tab
	./build config.tab $@

build: $(BUILDSRC)
	$(CC) $(CFLAGS) -o $@ build.c

clean:
	rm -f build build.o kern.bin
	${MAKE} -C ../kernel clean
	${MAKE} -C ../app/init clean
	${MAKE} -C ../lib/libserv clean
	${MAKE} -C ../lib/itron clean
	${MAKE} -C ../lib/nano-c clean
	${MAKE} -C ../lib/lowlib clean
	${MAKE} -C ../lib/libc clean
	${MAKE} -C ../servers/fs clean
	${MAKE} -C ../servers/ramdisk clean
	${MAKE} -C ../servers/console clean
	${MAKE} -C ../servers/keyboard clean
	${MAKE} -C ../servers/fd clean
	${MAKE} -C ../servers/ide clean
	${MAKE} -C ../servers/psaux clean
	${MAKE} -C ../servers/port-manager clean

$(SERVERS):
	${MAKE} -C ../servers/$@

core:
	${MAKE} -C ../kernel

libserv:
	${MAKE} -C ../lib/libserv

itron:
	${MAKE} -C ../lib/itron

nano-c:
	${MAKE} -C ../lib/nano-c

posix_lowlib:
	${MAKE} -C ../lib/lowlib

posix_libc:
	${MAKE} -C ../lib/libc

init:
	${MAKE} -C ../app/init

