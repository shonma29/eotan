# make initrd fd image (initrd.img)

MKFS = app/mkfs

TARGET = initrd.img

all: clean
	rm -f ${TARGET}
	${MKFS}/mksfs ${TARGET} 512 512 4
	${MKFS}/statfs ${TARGET} mkdir /dev
	${MKFS}/statfs ${TARGET} mknod /dev/console	0x00000000
	${MKFS}/statfs ${TARGET} mknod /dev/keyboard	0x00010000
	${MKFS}/statfs ${TARGET} mknod /dev/rd	0x80020000
	${MKFS}/statfs ${TARGET} dir /dev
	${MKFS}/statfs ${TARGET} write /initrd.mk initrd.mk
	(cd app/test && ./inst_app.sh ${TARGET})
	(cd app/contribution && ./inst_app.sh ${TARGET})

clean:
	rm -f ${TARGET}
