

#DEBUG=-DDEBUG


INC=../kernel
BUILDSRC=build.c $(INC)/boot.h
CFLAGS=$(DEBUG) -Wall -Werror -I$(INC)
CC=gcc

all: kern.bin

kern.bin: build config.tab
	./build config.tab $@

build: $(BUILDSRC)
	$(CC) $(CFLAGS) -o $@ build.c

clean:
	rm -f build build.o kern.bin
