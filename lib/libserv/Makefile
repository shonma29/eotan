#
#
#

# -DDEBUG
COPT=-O2 -fno-builtin
CFLAGS=$(COPT) -I../../kernel -I../servers
CC=gcc

OBJS= strncpy2.o interrupt.o malloc.o \
	port_manager.o message_port.o eventflag.o

all: ../libserv.a

../libserv.a: $(OBJS)
	ar rv $@ $(OBJS)
	ranlib $@

clean:
	rm -f $(OBJS) $(OBJS) ../libserv.a
