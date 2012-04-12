#!/bin/sh
# make posix fd image
# 

rm -f posix_fd.img
./mksfs posix_fd.img 2880 512 12
./statfs posix_fd.img mkdir /system
./statfs posix_fd.img write /system/wconsole ../../POSIX/servers/wconsole/wconsole
./statfs posix_fd.img chmod 755 /system/wconsole
./statfs posix_fd.img write /system/ramdisk ../../POSIX/servers/ramdisk/ramdisk
./statfs posix_fd.img chmod 755 /system/ramdisk
./statfs posix_fd.img mkdir /dev
./statfs posix_fd.img mknod /dev/console	0x00000000
./statfs posix_fd.img mknod /dev/win1	0x00000001
./statfs posix_fd.img mknod /dev/win2	0x00000002
./statfs posix_fd.img mknod /dev/keyboard	0x00010000
./statfs posix_fd.img mknod /dev/psaux	0x00020000
./statfs posix_fd.img mknod /dev/fd		0x80000000
./statfs posix_fd.img mknod /dev/ide0	0x80010000
./statfs posix_fd.img mknod /dev/ide1	0x80010001
./statfs posix_fd.img mknod /dev/ide2	0x80010002
./statfs posix_fd.img mknod /dev/ide3	0x80010003
./statfs posix_fd.img mknod /dev/ide4	0x80010004
./statfs posix_fd.img mknod /dev/ide5	0x80010005
./statfs posix_fd.img mknod /dev/ide6	0x80010006
./statfs posix_fd.img mknod /dev/ide7	0x80010007
./statfs posix_fd.img mknod /dev/ide8	0x80010008
./statfs posix_fd.img mknod /dev/ide9	0x80010009
./statfs posix_fd.img mknod /dev/rd	0x80020000
echo dir /dev
./statfs posix_fd.img dir /dev
./statfs posix_fd.img write /init.fm init.fm
(cd ../app;make)
../app/inst_app.sh posix_fd.img
