#
#$Id: Makefile,v 1.8 2000/01/29 16:14:03 naniwa Exp $
#


all: dev_console dev_fd765a dev_keyboard dev_ide psaux

kernlib:
	${MAKE} -C ../kernlib

dev_console: kernlib
	${MAKE} -C console

dev_fd765a: kernlib
	${MAKE} -C fd765a

dev_ide: kernlib
	${MAKE} -C ide

dev_keyboard: kernlib
	${MAKE} -C keyboard

dev_psaus: kernlib
	${MAKE} -C psaux

clean:
	${MAKE} -C console clean
	${MAKE} -C fd765a clean
	${MAKE} -C ide clean
	${MAKE} -C keyboard clean
	${MAKE} -C psaux clean

