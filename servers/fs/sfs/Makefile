# B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
# 
# GNU GENERAL PUBLIC LICENSE
# Version 2, June 1991
# 
# (C) B-Free Project.
# (C) 2001, Tomohide Naniwa
#

ROOT=../../..
INC=-I$(ROOT)/kernel/lib/libkernel -I$(ROOT)/kernel/core
IDENT=-DKERNEL # -DDEBUG
CFLAGS=-O2 -fno-builtin -Wall $(INC) $(IDENT)
CC=gcc

OBJS=sfs_fs.o sfs_block.o sfs_file.o sfs_dir.o sfs_inode.o sfs_device.o \
	sfs_cache.o
SRCS=sfs_fs.c sfs_block.c sfs_file.c sfs_dir.c sfs_inode.c sfs_device.c \
	sfs_cache.c


all: libsfs.a

libsfs.a: $(OBJS)
	ar rv libsfs.a $(OBJS)
	ranlib libsfs.a

clean:
	rm -f $(OBJS) libsfs.a


depend:
	cc -M $(CFLAGS) $(SRCS) > .depend


######

-include .depend

