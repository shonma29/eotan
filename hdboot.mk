# make sfsboot fd image (hd_boot.bin)

POSIXBIN = kernel/POSIX/mkfs

TARGET = 1st_fd.img

all: clean
	cp -f sfsboot/1st/1stboot ${TARGET}
	${POSIXBIN}/mksfs ${TARGET} 120 512 12
	${POSIXBIN}/statfs ${TARGET} mkdir /system
	${POSIXBIN}/statfs ${TARGET} write /system/2ndboot sfsboot/2nd/2ndboot
	sfsboot/mkmap ${TARGET} /system/2ndboot | sfsboot/mkboot ${TARGET}

clean:
	rm -f ${TARGET}
