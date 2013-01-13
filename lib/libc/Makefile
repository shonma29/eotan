# $Revision: 1.14 $
#
#

IDENT=-DI386
DEBUG=

CPP=/lib/cpp
ROOT=../..
CFLAGS=-O2 -Wall -Werror -fno-builtin -I. -I../../include -I$(ROOT)/kernel $(IDENT)
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
	posix/_make_cnnct.o \
	posix/sys_access.o \
	posix/sys_brk.o \
	posix/sys_chdir.o \
	posix/sys_chmod.o \
	posix/sys_chown.o \
	posix/sys_close.o \
	posix/sys_creat.o \
	posix/sys_dup.o \
	posix/sys_dup2.o \
	posix/sys_exec.o \
	posix/sys_exit.o \
	posix/sys_fcntl.o \
	posix/sys_fork.o \
	posix/fork.o \
	posix/sys_getcwd.o \
	posix/sys_getdents.o \
	posix/sys_getegid.o \
	posix/sys_geteuid.o \
	posix/sys_getgid.o \
	posix/sys_getgrgid.o \
	posix/sys_getpid.o \
	posix/sys_getppid.o \
	posix/sys_getuid.o \
	posix/sys_kill.o \
	posix/sys_link.o \
	posix/sys_lseek.o \
	posix/sys_mkdir.o \
	posix/sys_misc.o \
	posix/sys_mount.o \
	posix/sys_umount.o \
	posix/sys_open.o \
	posix/sys_read.o \
	posix/sys_remove.o \
	posix/sys_rename.o \
	posix/sys_rmdir.o \
	posix/sys_setgid.o \
	posix/sys_setpgid.o \
	posix/sys_setuid.o \
	posix/sys_sleep.o \
	posix/sys_fstat.o \
	posix/sys_time.o \
	posix/sys_umask.o \
	posix/sys_uname.o \
	posix/sys_unlink.o \
	posix/sys_utime.o \
	posix/sys_waitpid.o \
	posix/sys_write.o \
	errno.o

GTOBJS = others/tty.o 

all: ../libc.a ../libposix.a ../libgt.a
	ranlib ../libc.a
	ranlib ../libposix.a
	ranlib ../libgt.a

../libc.a: $(OBJS) $(OTHEROBJS) $(STRINGOBJS)
	ar rv $@ $(OBJS) $(OTHEROBJS) $(STRINGOBJS)

../libposix.a: $(OBJS)
	ar rv $@ $(OBJS)

../libgt.a: $(GTOBJS)
	ar rv $@ $(GTOBJS)

syscall.o: syscall.c

call_lowlib.o: call_lowlib.S


clean:
	rm -f $(OBJS)
	rm -f $(OTHEROBJS)
	rm -f $(STRINGOBJS)
	rm -f $(GTOBJS)
	rm -f ../libc.a
	rm -f ../libposix.a
	rm -f ../libgt.a
