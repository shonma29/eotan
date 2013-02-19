#
# @(#) $Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/ITRON/servers/Makefile,v 1.2 1999/12/26 11:13:24 naniwa Exp $
#
#

TARGET =	port-manager
LOAD_POINT =	0x00010000
ENTRY =		_main

INC=-I../../include -I../../kernel
CFLAGS=$(INC) -O2 -Wall -Werror


OBJS=port-manager.o database.o


all: $(TARGET)


$(TARGET): $(OBJS)
	$(LD) -static -o $(TARGET) -e $(ENTRY) -Ttext=$(LOAD_POINT) \
		$(OBJS) ../../lib/libserv.a ../../lib/libitron.a \
		../../lib/libnc.a
	strip -s -R .comment -R .note -R .note.ABI-tag $@


clean:
	rm -f $(OBJS) $(TARGET)


database.o: port-manager.h

port-manager.o: port-manager.h
