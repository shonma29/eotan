#
#
#

# -DDEBUG
COPT=-O2 -fno-builtin
CFLAGS=$(COPT) -I../../core -I../../servers
CC=gcc

OBJS= strncpy2.o interrupt.o malloc.o dma.o \
	port_manager.o message_port.o eventflag.o

all: ../libkernel.a

../libkernel.a: $(OBJS)
	ar rv $@ $(OBJS)
	ranlib $@

clean:
	rm -f $(OBJS) $(OBJS) ../libkernel.a
