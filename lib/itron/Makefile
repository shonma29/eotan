#
#
#

# -DDEBUG
COPT=-O2 -fno-builtin
CFLAGS=$(COPT) -I../../kernel -I../../servers
CC=gcc

OBJS=	call_kernel.o sys_task.o sys_message.o sys_debug.o \
	sys_errno.o sys_intrpt.o sys_lowlib.o \
	sys_virtual.o sys_misc.o  sys_evtflg.o sys_time.o \
	sys_acre_flg.o sys_acre_mbf.o

all: ../libitron.a

../libitron.a: $(OBJS)
	ar rv $@ $(OBJS)
	ranlib $@


clean:
	rm -f $(OBJS) ../libitron.a
