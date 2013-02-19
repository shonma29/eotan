#
#
#

# -DDEBUG
CFLAGS=-O2 -Wall -Werror -I../../include -I../../kernel -I../../servers

OBJS= strncpy2.o malloc.o \
	port_manager.o

all: ../libserv.a

../libserv.a: $(OBJS)
	ar rv $@ $(OBJS)
	ranlib $@

clean:
	rm -f $(OBJS) $(OBJS) ../libserv.a
