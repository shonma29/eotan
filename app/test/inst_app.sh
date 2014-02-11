#!/bin/sh

MKFS=../mkfs
ROOT=../..

${MKFS}/statfs ${ROOT}/$1 write /test test
${MKFS}/statfs ${ROOT}/$1 chmod 777 /test
${MKFS}/statfs ${ROOT}/$1 write /porttest porttest
${MKFS}/statfs ${ROOT}/$1 chmod 777 /porttest
${MKFS}/statfs ${ROOT}/$1 write /lowtest lowtest
${MKFS}/statfs ${ROOT}/$1 chmod 777 /lowtest
${MKFS}/statfs ${ROOT}/$1 write /rdtest rdtest
${MKFS}/statfs ${ROOT}/$1 chmod 777 /rdtest
${MKFS}/statfs ${ROOT}/$1 write /cat cat
${MKFS}/statfs ${ROOT}/$1 chmod 777 /cat
${MKFS}/statfs ${ROOT}/$1 write /echo echo
${MKFS}/statfs ${ROOT}/$1 chmod 777 /echo
${MKFS}/statfs ${ROOT}/$1 write /dmesg dmesg
${MKFS}/statfs ${ROOT}/$1 chmod 777 /dmesg
${MKFS}/statfs ${ROOT}/$1 write /mesg mesg
${MKFS}/statfs ${ROOT}/$1 chmod 777 /mesg
${MKFS}/statfs ${ROOT}/$1 write /shell shell
${MKFS}/statfs ${ROOT}/$1 chmod 777 /shell
${MKFS}/statfs ${ROOT}/$1 write /p6l p6l
${MKFS}/statfs ${ROOT}/$1 chmod 777 /p6l
${MKFS}/statfs ${ROOT}/$1 write /bees.p6 bees.p6
${MKFS}/statfs ${ROOT}/$1 write /test_iso646 test_iso646
${MKFS}/statfs ${ROOT}/$1 chmod 777 /test_iso646
${MKFS}/statfs ${ROOT}/$1 write /test_stdbool test_stdbool
${MKFS}/statfs ${ROOT}/$1 chmod 777 /test_stdbool
${MKFS}/statfs ${ROOT}/$1 write /test_string test_string
${MKFS}/statfs ${ROOT}/$1 chmod 777 /test_string
${MKFS}/statfs ${ROOT}/$1 dir / 

