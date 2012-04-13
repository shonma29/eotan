#
#	B-Free OS 作成用 makefile
#
#$Id: Makefile,v 1.2 1999/04/13 04:15:14 monaka Exp $
#

COMPONENT=kernlib posix libc

all: $(COMPONENT)


kernlib: 
	${MAKE} -C ../BTRON/kernlib

posix: kernlib
	${MAKE} -C manager

libc: kernlib
	${MAKE} -C ../../lib/libc

clean:
	${MAKE} -C ../../lib/libc clean
	${MAKE} -C manager clean
