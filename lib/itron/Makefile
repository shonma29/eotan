#
#
#

# -DDEBUG
CFLAGS=-O2 -Wall -Werror -fno-builtin -I../../include -I../../kernel -I../../servers
CC=gcc

OBJS=	task.o vcre_tsk.o message.o debug.o dbg_puts.o \
	interrupt.o \
	virtual.o vnew_reg.o misc.o  eventflag.o time.o \
	acre_flg.o acre_mbf.o \
	cre_por.o acre_por.o del_por.o \
	cal_por.o acp_por.o rpl_rdv.o

all: ../libitron.a

../libitron.a: $(OBJS)
	ar rv $@ $(OBJS)
	ranlib $@


clean:
	rm -f $(OBJS) ../libitron.a
