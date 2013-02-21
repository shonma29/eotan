export MAKE = gmake
export CC = gcc
export AS = as
export LD = ld
#DEBUG = -DDEBUG
export LINUX_INCLUDE = -I../../usr/include

TARGET = sfsboot_img 1st_fd_img 2nd_fd_img 3rd_fd_img initrd_img
TARGET1 = tool

all: tool kern applications boot ${TARGET}

kern:
	${MAKE} -C build

tool:
	${MAKE} -C app/mkfs

boot:
	${MAKE} -C sfsboot

applications:
	${MAKE} -C app/test
	${MAKE} -C app/contribution

1st_fd_img:
	${MAKE} -f build/hdboot.mk

2nd_fd_img:
	cp -p build/kern.bin ./2nd_fd.img

3rd_fd_img:
	${MAKE} -f build/posix.mk

initrd_img:
	${MAKE} -f build/initrd.mk

sfsboot_img:
	${MAKE} -f build/sfsboot.mk

clean:
	${MAKE} -C sfsboot clean
	${MAKE} -C build clean
	${MAKE} -C app/mkfs clean
	${MAKE} -C app/test clean
	${MAKE} -C app/contribution clean
	rm -f 1st_fd.img 2nd_fd.img 3rd_fd.img initrd.img
	rm -f sfsboot.img

