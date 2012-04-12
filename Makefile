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
	${MAKE} -C kernel/POSIX/mkfs

boot:
	${MAKE} -C sfsboot

applications:
	${MAKE} -C app/contribution

1st_fd.img:
	${MAKE} -f hdboot.mk

2nd_fd.img:
	cp make/btron ./2nd_fd.img

3rd_fd.img:
	${MAKE} -f posix.mk

sfsboot.img:
	${MAKE} -f sfsboot.mk

clean:
	${MAKE} -C sfsboot clean
	${MAKE} -C build clean
	${MAKE} -C kernel/POSIX/mkfs clean
	${MAKE} -C app/posix clean
	${MAKE} -C app/contribution clean
	rm -f 1st_fd.img 2nd_fd.img 3rd_fd.img
	rm -f sfsboot.img

dummy:
	echo '*dummy*'

