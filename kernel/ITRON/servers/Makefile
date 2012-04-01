#
# @(#) $Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/ITRON/servers/Makefile,v 1.2 1999/12/26 11:13:24 naniwa Exp $
#
#

TARGET =	port-manager
LOAD_POINT =	0x00010000
ENTRY =		_main
#LD = /usr/local/bin/ld		# ZIMAGE タイプの実行ファイルを作成する
				# ことができる LD を使う。
#LD=/usr/bin/ld
LD=ld

CC=gcc
INC=-I../../ITRON/h -I../../ITRON/i386
CFLAGS=$(INC) -O2 -fno-builtin


OBJS=port-manager.o database.o


all: $(TARGET)


$(TARGET): $(OBJS)
	$(LD) -static -o $(TARGET) -e $(ENTRY) -Ttext=$(LOAD_POINT)  $(OBJS) ../kernlib/libkernel.a	


clean:
	rm -f $(OBJS) $(TARGET)


database.o: port-manager.h

port-manager.o: port-manager.h
