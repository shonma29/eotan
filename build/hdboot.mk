# make sfsboot fd image (hd_boot.bin)

MKFS = app/mkfs

TARGET = 1st_fd.img

all: clean
	cp -f sfsboot/1st/1stboot ${TARGET}
	${MKFS}/mksfs ${TARGET} 120 512 12
	${MKFS}/statfs ${TARGET} mkdir /system
	${MKFS}/statfs ${TARGET} write /system/2ndboot sfsboot/2nd/2ndboot
	sfsboot/mkmap ${TARGET} /system/2ndboot | sfsboot/mkboot ${TARGET}

clean:
	rm -f ${TARGET}
