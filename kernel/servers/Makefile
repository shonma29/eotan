#
#$Id: Makefile,v 1.8 2000/01/29 16:14:03 naniwa Exp $
#


all: dev_console dev_fd dev_keyboard dev_ide psaux srv_port-manager

kernlib:
	${MAKE} -C ../lib/libkernel

dev_console: kernlib
	${MAKE} -C console

dev_fd: kernlib
	${MAKE} -C fd

dev_ide: kernlib
	${MAKE} -C ide

dev_keyboard: kernlib
	${MAKE} -C keyboard

dev_psaus: kernlib
	${MAKE} -C psaux

srv_port-manager: kernlib
	${MAKE} -C port-manager

clean:
	${MAKE} -C console clean
	${MAKE} -C fd clean
	${MAKE} -C ide clean
	${MAKE} -C keyboard clean
	${MAKE} -C psaux clean
	${MAKE} -C port-manager clean

