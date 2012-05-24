#
#
#

# -DDEBUG
COPT=-O2 -fno-builtin
CFLAGS=$(COPT) -I../../core -I../../servers
CC=gcc

OBJS=	call_kernel.o sys_task.o sys_message.o sys_debug.o \
	port_manager.o message_port.o sys_errno.o sys_intrpt.o \
	sys_virtual.o sys_misc.o eventflag.o sys_evtflg.o sys_time.o

OTHEROBJS= string.o binary.o interrupt.o sys_lowlib.o  \
	malloc.o dma.o vnprintf.o

all: libkernel.a libitron.a

libkernel.a: $(OBJS) $(OTHEROBJS)
	ar rv $@ $(OBJS) $(OTHEROBJS)
	ranlib $@

libitron.a: $(OBJS)
	ar rv $@ $(OBJS)
	ranlib $@


clean:
	rm -f $(OBJS) $(OTHEROBJS) libkernel.a libitron.a
