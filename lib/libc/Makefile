# $Revision: 1.14 $
#
#

IDENT=-DI386
DEBUG=

CPP=/lib/cpp
ROOT=../..
CFLAGS=-O2 -fno-builtin -I. -I$(ROOT)/kernel/core $(IDENT)
CC=gcc
MKDEPEND=$(CC) -M
LD=ld
AS=as

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.s
	$(CPP) $(ASFLAGS) $*.s | $(AS) -o $*.o

%.o: %.S
	$(CC) $(CFLAGS) -c $< -o $@

OTHEROBJS= others/atoi.o others/stdlib.o _main.o \
	others/binary.o malloc/malloc.o

STRINGOBJS = string/strcat.o string/strcmp.o string/strcpy.o string/strlen.o

OBJS=   call_lowlib.o \
	native/sys_access.o \
	native/sys_brk.o \
	native/sys_chdir.o \
	native/sys_chmod.o \
	native/sys_chown.o \
	native/sys_close.o \
	native/sys_creat.o \
	native/sys_dup.o \
	native/sys_dup2.o \
	native/sys_exec.o \
	native/sys_exit.o \
	native/sys_fcntl.o \
	native/sys_fork.o \
	native/fork.o \
	native/sys_getcwd.o \
	native/sys_getdents.o \
	native/sys_getegid.o \
	native/sys_geteuid.o \
	native/sys_getgid.o \
	native/sys_getgrgid.o \
	native/sys_getpid.o \
	native/sys_getppid.o \
	native/sys_getuid.o \
	native/sys_kill.o \
	native/sys_link.o \
	native/sys_lseek.o \
	native/sys_mkdir.o \
	native/sys_misc.o \
	native/sys_mount.o \
	native/sys_umount.o \
	native/sys_open.o \
	native/sys_read.o \
	native/sys_remove.o \
	native/sys_rename.o \
	native/sys_rmdir.o \
	native/sys_setgid.o \
	native/sys_setpgid.o \
	native/sys_setuid.o \
	native/sys_sleep.o \
	native/sys_fstat.o \
	native/sys_time.o \
	native/sys_umask.o \
	native/sys_uname.o \
	native/sys_unlink.o \
	native/sys_utime.o \
	native/sys_waitpid.o \
	native/sys_write.o \
	errno.o

GTOBJS = others/tty.o 

all: libc.a libnative.a libgt.a
	ranlib libc.a
	ranlib libnative.a
	ranlib libgt.a

libc.a: $(OBJS) $(OTHEROBJS) $(STRINGOBJS)
	ar rv libc.a $(OBJS) $(OTHEROBJS) $(STRINGOBJS)

libnative.a: $(OBJS)
	ar rv libnative.a $(OBJS)

libgt.a: $(GTOBJS)
	ar rv $@ $(GTOBJS)

syscall.o: syscall.c

call_lowlib.o: call_lowlib.S


clean:
	rm -f $(OBJS)
	rm -f $(OTHEROBJS)
	rm -f $(STRINGOBJS)
	rm -f $(GTOBJS)
	rm -f libc.a
	rm -f libnative.a
	rm -f libgt.a
