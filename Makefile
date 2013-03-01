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
	${MAKE} -C build

tool:
	${MAKE} -C app/mkfs

boot:
	${MAKE} -C sfsboot

applications:
	${MAKE} -C app/test
	${MAKE} -C app/contribution

initrd_img:
	${MAKE} -f build/initrd.mk

sfsboot_img:
	${MAKE} -f build/sfsboot.mk

clean:
	${MAKE} -C sfsboot clean
	${MAKE} -C build clean
	${MAKE} -f kernel/Makefile WD=kernel clean
	${MAKE} -f libs.mk clean
	${MAKE} -C app/mkfs clean
	${MAKE} -C app/test clean
	${MAKE} -C app/contribution clean
	rm -f initrd.img
	rm -f sfsboot.img

