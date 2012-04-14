export MAKE = gmake
export CC = gcc
export AS = as
export LD = ld
#DEBUG = -DDEBUG
export LINUX_INCLUDE = -I../../usr/include

TARGET = sfsboot.img
TARGET1 = tool

all: kern tool boot ${TARGET}

kern:
	${MAKE} -C build

tool:
	${MAKE} -C app/mkfs

boot:
	${MAKE} -C sfsboot

applications:
	${MAKE} -C app/contribution

1st_fd.img:
	${MAKE} -f build/hdboot.mk

2nd_fd.img:
	cp -p build/kern.bin ./2nd_fd.img

3rd_fd.img:
	${MAKE} -f build/posix.mk

sfsboot.img:
	${MAKE} -f build/sfsboot.mk

clean:
	${MAKE} -C sfsboot clean
	${MAKE} -C build clean
	${MAKE} -C app/mkfs clean
	${MAKE} -C app/posix clean
	${MAKE} -C app/contribution clean
	rm -f 1st_fd.img 2nd_fd.img 3rd_fd.img
	rm -f sfsboot.img

dummy:
	echo '*dummy*'

