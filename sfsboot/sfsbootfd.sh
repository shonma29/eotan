#! /bin/bash

mkfspath="../kernel/POSIX/mkfs"

echo "write 2ndboot"
$mkfspath/statfs /dev/fd0 write /system/2ndboot ./2nd/2ndboot

echo "write 1stboot"
./mkmap /dev/fd0 /system/2ndboot | ./mkboot ./1st/1stboot 

dd if=./1st/1stboot of=/dev/fd0 bs=512 count=1

    