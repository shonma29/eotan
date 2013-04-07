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
${MKFS}/statfs ${ROOT}/$1 dir / 
