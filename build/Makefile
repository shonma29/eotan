

#DEBUG=-DDEBUG


INC=../kernel
BUILDSRC=build.c $(INC)/boot.h
CFLAGS=$(DEBUG) -Wall -Werror -I$(INC)
CC=gcc

SERVERS=fs


PROGRAMS=$(SERVERS)

all: $(PROGRAMS) kern.bin

kern.bin: build config.tab
	./build config.tab $@

build: $(BUILDSRC)
	$(CC) $(CFLAGS) -o $@ build.c

clean:
	rm -f build build.o kern.bin
	${MAKE} -C ../servers/fs clean

$(SERVERS):
	${MAKE} -C ../servers/$@
