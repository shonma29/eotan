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
IDENT=$(DEBUG) -DKERNEL
ROOT=../..
INC=-I$(ROOT)/kernel/core
CFLAGS=-O2 -fno-builtin -Wall $(INC) $(IDENT)
CC=gcc

ENTRY=startup
STARTADDR=0x00001000


OBJS=s0.o posix.o version.o request.o filesystem.o memory.o process.o \
	log.o syscall.o global.o vmtree.o exec.o fork.o
SRCS=posix.c version.c request.c filesystem.c memory.c process.c \
	log.c syscall.c global.c vmtree.c exec.c fork.c

KERNLIB=../../kernel/lib/libkernel/libkernel.a
LIBS=sfs/libsfs.a syscalls/libsysc.a $(KERNLIB)

all:
	${MAKE} -C sfs
	${MAKE} -C syscalls
	${MAKE} posix_manager

posix_manager:	$(OBJS) $(LIBS)
	$(LD) -static -o $@ -e $(ENTRY) -Ttext=$(STARTADDR) $(OBJS) $(LIBS)
	strip -s $@
	size $@

clean:
	rm -f $(OBJS) posix_manager
	${MAKE} -C sfs clean
	${MAKE} -C syscalls clean

depend:
	cc -M $(CFLAGS) $(SRCS) > .depend



########################################

-include .depend

