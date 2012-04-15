
IDENT=-DI386 -DIBMPC $(DEBUG)

CPP=/lib/cpp
ROOT=../..

CFLAGS=-O2 -I. -I$(ROOT)/core -I$(ROOT)/lib/libkernel -fno-builtin $(IDENT) $(DEBUG)

CC=gcc
LIBS=$(ROOT)/lib/libkernel/libkernel.a
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



