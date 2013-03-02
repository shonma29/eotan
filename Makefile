export MAKE = gmake
export CC = gcc
export AS = as
export LD = ld
#DEBUG = -DDEBUG
export LINUX_INCLUDE = -I../../usr/include

TARGET = sfsboot_img initrd_img

all: tool libs kern applications boot ${TARGET}

libs:
	${MAKE} -f libs.mk

kern:
	${MAKE} -f kernel/Makefile WD=kernel
	${MAKE} -f servers.mk
	${MAKE} -f app/init/Makefile WD=app/init
	${MAKE} -C build

tool:
	${MAKE} -C app/mkfs

boot:
	${MAKE} -C sfsboot

applications:
	${MAKE} -f app/test/Makefile WD=app/test
	${MAKE} -f app/contribution/pager/Makefile WD=app/contribution/pager

initrd_img:
	${MAKE} -f build/initrd.mk

sfsboot_img:
	${MAKE} -f build/sfsboot.mk

clean:
	${MAKE} -C sfsboot clean
	${MAKE} -C build clean
	${MAKE} -f kernel/Makefile WD=kernel clean
	${MAKE} -f libs.mk clean
	${MAKE} -f servers.mk clean
	${MAKE} -C app/mkfs clean
	${MAKE} -f app/init/Makefile WD=app/init clean
	${MAKE} -f app/test/Makefile WD=app/test clean
	${MAKE} -f app/contribution/pager/Makefile WD=app/contribution/pager clean
	rm -f initrd.img
	rm -f sfsboot.img

