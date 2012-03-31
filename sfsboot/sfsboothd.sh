#! /bin/bash

mkfspath="../kernel/POSIX/mkfs"

echo "write 2ndboot"
$mkfspath/statfs $1 write /system/2ndboot ./2nd/2ndboot

echo "write 1stboot"
./mkmap $1 /system/2ndboot | ./mkhdboot ./1st/1stboot $1

dd if=./1st/1stboot of=$1 bs=512 count=1

    