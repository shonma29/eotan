

#DEBUG=-DDEBUG


INC=../kernel
BUILDSRC=build.c $(INC)/boot.h
CFLAGS=$(DEBUG) -Wall -Werror -I$(INC)
CC=gcc

SERVERS=console keyboard fd ide psaux port-manager fs ramdisk dumb
USERAPP=init


PROGRAMS=$(SERVERS) $(USERAPP)

all: $(PROGRAMS) kern.bin

kern.bin: build config.tab
	./build config.tab $@

build: $(BUILDSRC)
	$(CC) $(CFLAGS) -o $@ build.c

clean:
	rm -f build build.o kern.bin
	${MAKE} -C ../app/init clean
	${MAKE} -C ../servers/fs clean
	${MAKE} -C ../servers/ramdisk clean
	${MAKE} -C ../servers/console clean
	${MAKE} -C ../servers/keyboard clean
	${MAKE} -C ../servers/fd clean
	${MAKE} -C ../servers/ide clean
	${MAKE} -C ../servers/psaux clean
	${MAKE} -C ../servers/dumb clean
	${MAKE} -C ../servers/port-manager clean

$(SERVERS):
	${MAKE} -C ../servers/$@

init:
	${MAKE} -C ../app/init

