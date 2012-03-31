#!/bin/sh
# make sfsboot fd image
# 

cp -f ../../../sfsboot/1st/1stboot hd_boot.bin
./mksfs hd_boot.bin 120 512 12
./statfs hd_boot.bin mkdir /system
./statfs hd_boot.bin write /system/2ndboot ../../../sfsboot/2nd/2ndboot
../../../sfsboot/mkmap hd_boot.bin /system/2ndboot | ../../../sfsboot/mkboot hd_boot.bin
