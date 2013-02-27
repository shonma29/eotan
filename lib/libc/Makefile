# This is free and unencumbered software released into the public domain.
#
# Anyone is free to copy, modify, publish, use, compile, sell, or
# distribute this software, either in source code form or as a compiled
# binary, for any purpose, commercial or non-commercial, and by any
# means.
#
# In jurisdictions that recognize copyright laws, the author or authors
# of this software dedicate any and all copyright interest in the
# software to the public domain. We make this dedication for the benefit
# of the public at large and to the detriment of our heirs and
# successors. We intend this dedication to be an overt act of
# relinquishment in perpetuity of all present and future rights to this
# software under copyright law.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
# OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.
#
# For more information, please refer to <http://unlicense.org/>

TARGET = ../libc.a
CFLAGS = -Wall -Werror -O2
ASFLAGS = 
LDFLAGS = --oformat binary

INCLUDES = -I . -I ../../include -I ../../kernel -I ../../servers/fs
OBJS = _main.o \
	errno.o \
	malloc/malloc.o \
	others/atoi.o \
	others/binary.o \
	others/stdlib.o \
	string/memcpy.o \
	string/memset.o \
	string/strcat.o \
	string/strcmp.o \
	string/strcpy.o \
	string/strlen.o \
	string/strncat.o \
	string/strncmp.o \
	string/strncpy.o \
	sys/_call_fs.o \
	sys/_exit.o \
	sys/_fork.o \
	sys/access.o \
	sys/brk.o \
	sys/chdir.o \
	sys/chmod.o \
	sys/chown.o \
	sys/close.o \
	sys/dup.o \
	sys/dup2.o \
	sys/execve.o \
	sys/fcntl.o \
	sys/fork.o \
	sys/fstat.o \
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
	sys/misc.o \
	sys/mkdir.o \
	sys/mount.o \
	sys/open.o \
	sys/read.o \
	sys/rename.o \
	sys/rmdir.o \
	sys/setgid.o \
	sys/setuid.o \
	sys/sleep.o \
	sys/time.o \
	sys/umask.o \
	sys/umount.o \
	sys/uname.o \
	sys/unlink.o \
	sys/utime.o \
	sys/waitpid.o \
	sys/write.o

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

all: $(TARGET)

$(TARGET): $(OBJS)
	$(AR) rsv $@ $?

clean:
	rm -f $(TARGET) $(OBJS)
