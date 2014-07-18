# make initrd fd image (initrd.img)

BIN = app/sfs

TARGET = initrd.img

all: clean
	rm -f ${TARGET}
	${BIN}/mkfs ${TARGET} 512 512 4
	${BIN}/statfs ${TARGET} mkdir /dev
	${BIN}/statfs ${TARGET} mknod /dev/console	0x00000000
	${BIN}/statfs ${TARGET} mknod /dev/keyboard	0x00010000
	${BIN}/statfs ${TARGET} mknod /dev/mouse 0x00020000
	${BIN}/statfs ${TARGET} mknod /dev/rd	0x80020000
	${BIN}/statfs ${TARGET} dir /dev

clean:
	rm -f ${TARGET}
