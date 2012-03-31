
IDENT=-DI386 -DIBMPC $(DEBUG)

CPP=/lib/cpp
ROOT=../../../ITRON

CFLAGS=-O2 -I. -I$(ROOT) -I$(ROOT)/h -I$(ROOT)/i386 -I$(ROOT)/kernlib -fno-builtin $(IDENT) $(DEBUG)

CC=gcc
LIBS=$(ROOT)/kernlib/libkernel.a
#LD=/usr/bin/ld
LD=ld

STARTADDR=0x80140000
ENTRY=start

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

OBJS=psaux.o  kbc.o 
SRCS=psaux.c  kbc.c

all: psaux

psaux: $(OBJS) 
	$(LD) -static -o $@ -e ${ENTRY} -Ttext=$(STARTADDR) $(OBJS) $(LIBS)

clean:
	rm -f $(OBJS) psaux



