# $Revision: 1.14 $
#
#

IDENT=
DEBUG=

ROOT=../..
CFLAGS=-O2 -Wall -Werror -fno-builtin -I. -I$(ROOT)/include -I$(ROOT)/kernel \
		-I$(ROOT)/servers/fs $(IDENT)
CC=gcc
MKDEPEND=$(CC) -M
LD=ld
AS=as

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.S
	$(CC) $(CFLAGS) -c $< -o $@

OTHEROBJS= others/atoi.o others/stdlib.o _main.o \
	others/binary.o malloc/malloc.o

STRINGOBJS = string/strcat.o string/strcmp.o string/strcpy.o string/strlen.o

OBJS=   sys/call_fs.o \
	sys/access.o \
	sys/brk.o \
	sys/chdir.o \
	sys/chmod.o \
	sys/chown.o \
	sys/close.o \
	sys/dup.o \
	sys/dup2.o \
	sys/execve.o \
	sys/_exit.o \
	sys/fcntl.o \
	sys/_fork.o \
	sys/fork.o \
	sys/getcwd.o \
	sys/getdents.o \
	sys/getegid.o \
	sys/geteuid.o \
	sys/getgid.o \
	sys/getpid.o \
	sys/getppid.o \
	sys/getuid.o \
	sys/kill.o \
	sys/link.o \
	sys/lseek.o \
	sys/mkdir.o \
	sys/misc.o \
	sys/mount.o \
	sys/umount.o \
	sys/open.o \
	sys/read.o \
	sys/rename.o \
	sys/rmdir.o \
	sys/setgid.o \
	sys/setuid.o \
	sys/sleep.o \
	sys/fstat.o \
	sys/time.o \
	sys/umask.o \
	sys/uname.o \
	sys/unlink.o \
	sys/utime.o \
	sys/waitpid.o \
	sys/write.o \
	errno.o

all: ../libc.a
	ranlib ../libc.a

../libc.a: $(OBJS) $(OTHEROBJS) $(STRINGOBJS)
	ar rv $@ $(OBJS) $(OTHEROBJS) $(STRINGOBJS)

clean:
	rm -f $(OBJS)
	rm -f $(OTHEROBJS)
	rm -f $(STRINGOBJS)
	rm -f ../libc.a
