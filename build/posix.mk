# make posix fd image (posix_fd.bin)

MKFS = app/mkfs

TARGET = 3rd_fd.img

all: clean
	rm -f ${TARGET}
	${MKFS}/mksfs ${TARGET} 2880 512 12
	${MKFS}/statfs ${TARGET} mkdir /system
	${MKFS}/statfs ${TARGET} write /system/ramdisk servers/ramdisk/ramdisk
	${MKFS}/statfs ${TARGET} chmod 755 /system/ramdisk
	${MKFS}/statfs ${TARGET} mkdir /dev
	${MKFS}/statfs ${TARGET} mknod /dev/console	0x00000000
	${MKFS}/statfs ${TARGET} mknod /dev/win1	0x00000001
	${MKFS}/statfs ${TARGET} mknod /dev/win2	0x00000002
	${MKFS}/statfs ${TARGET} mknod /dev/keyboard	0x00010000
	${MKFS}/statfs ${TARGET} mknod /dev/psaux	0x00020000
	${MKFS}/statfs ${TARGET} mknod /dev/fd		0x80000000
	${MKFS}/statfs ${TARGET} mknod /dev/ide0	0x80010000
	${MKFS}/statfs ${TARGET} mknod /dev/ide1	0x80010001
	${MKFS}/statfs ${TARGET} mknod /dev/ide2	0x80010002
	${MKFS}/statfs ${TARGET} mknod /dev/ide3	0x80010003
	${MKFS}/statfs ${TARGET} mknod /dev/ide4	0x80010004
	${MKFS}/statfs ${TARGET} mknod /dev/ide5	0x80010005
	${MKFS}/statfs ${TARGET} mknod /dev/ide6	0x80010006
	${MKFS}/statfs ${TARGET} mknod /dev/ide7	0x80010007
	${MKFS}/statfs ${TARGET} mknod /dev/ide8	0x80010008
	${MKFS}/statfs ${TARGET} mknod /dev/ide9	0x80010009
	${MKFS}/statfs ${TARGET} mknod /dev/rd	0x80020000
	echo dir /dev
	${MKFS}/statfs ${TARGET} dir /dev
	${MKFS}/statfs ${TARGET} write /init.fm app/mkfs/init.fm
	(cd app/psaux && ./inst_app.sh ${TARGET})

clean:
	rm -f ${TARGET}
