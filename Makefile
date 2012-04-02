
#DEBUG=-DDEBUG
export LINUX_INCLUDE=-I../../usr/include

TARGET1=tool

all:
#bfree_fd:
#	(cd kernel/BTRON/make; ${MAKE})
#bfree_fd: dummy
#	(cd boot; ${MAKE})
#	(cd kernel/BTRON/make; ${MAKE})
#	(cd kernel/POSIX/mkfs; ${MAKE})
#	(cd kernel/POSIX/mkfs; ${MAKE} btron_fd)
#
#all_3fd:
	(cd sfsboot; ${MAKE})
	(cd kernel/BTRON/make; ${MAKE})
	cp kernel/BTRON/make/btron ./2nd_fd.img
	(cd kernel/POSIX/mkfs; ${MAKE})
	(cd kernel/POSIX/mkfs; ./mk_img.sh)
	cp kernel/POSIX/mkfs/posix_fd.img ./3rd_fd.img
	(cd kernel/POSIX/mkfs; ./mk_hdboot.sh)
	cp  kernel/POSIX/mkfs/hd_boot.bin ./1st_fd.img
#	(cd contribution; ${MAKE})

applications:
	(cd contribution/applications/frtm-bf/src; ${MAKE})

clean:
#	rm -f bfree_fd.img
	(cd sfsboot; ${MAKE} clean)
	(cd kernel/BTRON/make; ${MAKE} clean)
	(cd kernel/POSIX/mkfs; ${MAKE} clean)
	(cd contribution; ${MAKE} clean)
#
#clean_all:
	rm -f 1st_fd.img 2nd_fd.img 3rd_fd.img
#	(cd kernel/BTRON/make; ${MAKE} clean)

dummy:
	echo '*dummy*'

