#!/bin/csh
# make sfsboot fd image
# 

cp -f ../../sfsboot/1st/1stboot sfsboot.bin
./mksfs sfsboot.bin 2880 512 12
./statfs sfsboot.bin mkdir /system
./statfs sfsboot.bin write /system/2ndboot ../../sfsboot/2nd/2ndboot
./statfs sfsboot.bin write /system/btron386 ../../build/btron
./statfs sfsboot.bin write /system/wconsole ../../servers/wconsole/wconsole
./statfs sfsboot.bin chmod 755 /system/wconsole
./statfs sfsboot.bin write /system/ramdisk ../../servers/ramdisk/ramdisk
./statfs sfsboot.bin chmod 755 /system/ramdisk
./statfs sfsboot.bin mkdir /dev
./statfs sfsboot.bin mknod /dev/console	0x00000000
./statfs sfsboot.bin mknod /dev/win1	0x00000001
./statfs sfsboot.bin mknod /dev/win2	0x00000002
./statfs sfsboot.bin mknod /dev/keyboard	0x00010000
./statfs sfsboot.bin mknod /dev/psaux	0x00020000
./statfs sfsboot.bin mknod /dev/fd		0x80000000
./statfs sfsboot.bin mknod /dev/ide0	0x80010000
./statfs sfsboot.bin mknod /dev/ide1	0x80010001
./statfs sfsboot.bin mknod /dev/ide2	0x80010002
./statfs sfsboot.bin mknod /dev/ide3	0x80010003
./statfs sfsboot.bin mknod /dev/ide4	0x80010004
./statfs sfsboot.bin mknod /dev/ide5	0x80010005
./statfs sfsboot.bin mknod /dev/ide6	0x80010006
./statfs sfsboot.bin mknod /dev/ide7	0x80010007
./statfs sfsboot.bin mknod /dev/ide8	0x80010008
./statfs sfsboot.bin mknod /dev/ide9	0x80010009
./statfs sfsboot.bin mknod /dev/rd	0x80020000
echo dir /dev
./statfs sfsboot.bin dir /dev
./statfs sfsboot.bin write /init.fm init.fm
(cd ../app && make)
../app/inst_app.sh sfsboot.bin 
../../sfsboot/mkmap sfsboot.bin /system/2ndboot | ../../sfsboot/mkboot sfsboot.bin
