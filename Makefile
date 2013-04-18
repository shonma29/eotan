export MAKE = gmake
export CC = gcc
export AS = as
export LD = ld
#DEBUG = -DDEBUG
export LINUX_INCLUDE = -I../../usr/include

TARGET = starter/boot.iso

all: tool libs kern applications starter initrd_img ${TARGET}

.PHONY: tool libs kern applications starter
libs:
	${MAKE} -f libs.mk

kern:
	${MAKE} -f kernel/Makefile WD=kernel
	${MAKE} -f servers.mk

tool:
	${MAKE} -f app/mkfs/Makefile WD=app/mkfs

starter/boot.iso:
	mkdir -p build
	${MAKE} -C starter

applications:
	${MAKE} -f app/test/Makefile WD=app/test
	${MAKE} -f app/contribution/pager/Makefile WD=app/contribution/pager

initrd_img:
	${MAKE} -f initrd.mk

clean:
	${MAKE} -C starter clean
	${MAKE} -f kernel/Makefile WD=kernel clean
	${MAKE} -f libs.mk clean
	${MAKE} -f servers.mk clean
	${MAKE} -f app/mkfs/Makefile WD=app/mkfs clean
	${MAKE} -f app/test/Makefile WD=app/test clean
	${MAKE} -f app/contribution/pager/Makefile WD=app/contribution/pager clean
	rm -f initrd.img

