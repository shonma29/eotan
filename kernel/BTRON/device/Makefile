#
#$Id: Makefile,v 1.8 2000/01/29 16:14:03 naniwa Exp $
#


all: dev_console dev_fd765a dev_keyboard dev_ide ne2000 rs232c beep hcm_pci

kernlib:
	cd ../kernlib/; ${MAKE}

dev_console: kernlib
	cd console; ${MAKE}

dev_fd765a: kernlib
	cd fd765a; ${MAKE}

dev_ide: kernlib
	cd ide; ${MAKE}

dev_keyboard: kernlib
	cd keyboard; ${MAKE}

ne2000: kernlib
	cd ne2000; ${MAKE}

rs232c: kernlib
	cd rs232c; ${MAKE}

beep: kernlib
	cd beep; ${MAKE}

hcm_pci: kernlib
	cd hcm_pci; ${MAKE}

clean:
	cd console; ${MAKE} clean
	cd fd765a; ${MAKE} clean
	cd ide; ${MAKE} clean
	cd keyboard; ${MAKE} clean
	cd ne2000; ${MAKE} clean
	cd rs232c; ${MAKE} clean
	cd beep; ${MAKE} clean
	cd hcm_pci; ${MAKE} clean
