# $Id: Makefile,v 1.9 2000/01/30 19:02:56 kishida0 Exp $
# 
# Makefile for init.
#
ROOT = ../..
CFLAGS = -O2 -fno-builtin -Wall -I.. -I../lib -I$(ROOT)/core -I$(ROOT)/lib/kernlib
CXXFLAGS = $(CFLAGS)

CC = gcc

OBJS = command.o

all: libCmd.a

libCmd.a: $(OBJS)
	ar rv libCmd.a $(OBJS)
	ranlib libCmd.a

command.o: command.c

clean:
	rm -f libCmd.a
	rm -f $(OBJS)
	rm -f *~
