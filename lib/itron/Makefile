#
#
#

# -DDEBUG
COPT=-O2 -fno-builtin
CFLAGS=$(COPT) -I../../kernel -I../../servers
CC=gcc

OBJS=	call_kernel.o svc_task.o svc_message.o svc_debug.o \
	svc_intrpt.o svc_lowlib.o \
	svc_virtual.o svc_misc.o  svc_evtflg.o svc_time.o \
	svc_acre_flg.o svc_acre_mbf.o \
	svc_cre_por.o svc_acre_por.o svc_del_por.o \
	svc_cal_por.o svc_acp_por.o svc_rpl_rdv.o

all: ../libitron.a

../libitron.a: $(OBJS)
	ar rv $@ $(OBJS)
	ranlib $@


clean:
	rm -f $(OBJS) ../libitron.a
