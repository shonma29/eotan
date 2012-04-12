# make posix fd image (posix_fd.bin)

POSIXBIN = kernel/POSIX/mkfs

TARGET = 3rd_fd.img

all: clean
	rm -f ${TARGET}
	${POSIXBIN}/mksfs ${TARGET} 2880 512 12
	${POSIXBIN}/statfs ${TARGET} mkdir /system
	${POSIXBIN}/statfs ${TARGET} write /system/wconsole kernel/POSIX/servers/wconsole/wconsole
	${POSIXBIN}/statfs ${TARGET} chmod 755 /system/wconsole
	${POSIXBIN}/statfs ${TARGET} write /system/ramdisk kernel/POSIX/servers/ramdisk/ramdisk
	${POSIXBIN}/statfs ${TARGET} chmod 755 /system/ramdisk
	${POSIXBIN}/statfs ${TARGET} mkdir /dev
	${POSIXBIN}/statfs ${TARGET} mknod /dev/console	0x00000000
	${POSIXBIN}/statfs ${TARGET} mknod /dev/win1	0x00000001
	${POSIXBIN}/statfs ${TARGET} mknod /dev/win2	0x00000002
	${POSIXBIN}/statfs ${TARGET} mknod /dev/keyboard	0x00010000
	${POSIXBIN}/statfs ${TARGET} mknod /dev/psaux	0x00020000
	${POSIXBIN}/statfs ${TARGET} mknod /dev/fd		0x80000000
	${POSIXBIN}/statfs ${TARGET} mknod /dev/ide0	0x80010000
	${POSIXBIN}/statfs ${TARGET} mknod /dev/ide1	0x80010001
	${POSIXBIN}/statfs ${TARGET} mknod /dev/ide2	0x80010002
	${POSIXBIN}/statfs ${TARGET} mknod /dev/ide3	0x80010003
	${POSIXBIN}/statfs ${TARGET} mknod /dev/ide4	0x80010004
	${POSIXBIN}/statfs ${TARGET} mknod /dev/ide5	0x80010005
	${POSIXBIN}/statfs ${TARGET} mknod /dev/ide6	0x80010006
	${POSIXBIN}/statfs ${TARGET} mknod /dev/ide7	0x80010007
	${POSIXBIN}/statfs ${TARGET} mknod /dev/ide8	0x80010008
	${POSIXBIN}/statfs ${TARGET} mknod /dev/ide9	0x80010009
	${POSIXBIN}/statfs ${TARGET} mknod /dev/rd	0x80020000
	echo dir /dev
	${POSIXBIN}/statfs ${TARGET} dir /dev
	${POSIXBIN}/statfs ${TARGET} write /init.fm kernel/POSIX/mkfs/init.fm
#	${MAKE} -C app/posix
#	kernel/POSIX/app/inst_app.sh ${TARGET}

clean:
	rm -f ${TARGET}
