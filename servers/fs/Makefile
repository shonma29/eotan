#
# B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
# 
# GNU GENERAL PUBLIC LICENSE
# Version 2, June 1991
# 
# (C) B-Free Project.
# (C) 2001, Tomohide Naniwa
#

# デバッグ用 Define
#
# VMDEBUG	仮想記憶関連
# MMDEBUG	メモリ管理関連
# PMDEBUG	プロセス管理関連
# FMDEBUG	ファイル管理関連
#
#DEBUG=-DFMDEBUG -DEXEC_DEBUG -DDEBUG -DFKDEBUG
DEBUG=#-DEXEC_DEBUG
IDENT=$(DEBUG)
ROOT=../..
INC=-I$(ROOT)/include -I$(ROOT)/kernel
CFLAGS=-O2 -fno-builtin -Wall -Werror $(INC) $(IDENT)
CC=gcc

ENTRY=startup
STARTADDR=0x00001000


OBJS=startup.o main.o request.o filesystem.o memory.o process.o \
	api.o vmtree.o exec.o fork.o
SRCS=main.c request.c filesystem.c memory.c process.c \
	api.c vmtree.c exec.c fork.c

KERNLIB=../../lib/libserv.a ../../lib/libitron.a \
	../../lib/libnc.a
LIBS=sfs/libsfs.a posix-server/libposix-server.a $(KERNLIB)

all:
	${MAKE} -C sfs
	${MAKE} -C posix-server
	${MAKE} fs

fs:	$(OBJS) $(LIBS)
	$(LD) -static -o $@ -e $(ENTRY) -Ttext=$(STARTADDR) $(OBJS) $(LIBS)
	strip -s -R .comment -R .note -R .note.ABI-tag $@
	size $@

clean:
	rm -f $(OBJS) fs
	${MAKE} -C sfs clean
	${MAKE} -C posix-server clean

depend:
	cc -M $(CFLAGS) $(SRCS) > .depend



########################################

-include .depend

