#!/bin/sh
# make posix fd image
# 

MKFS=../mkfs
ROOT=../..

${MKFS}/statfs ${ROOT}/$1 write /frtm frtm-bf/src/frtm
${MKFS}/statfs ${ROOT}/$1 chmod 777 /frtm
${MKFS}/statfs ${ROOT}/$1 write /pager pager/pager
${MKFS}/statfs ${ROOT}/$1 chmod 777 /pager
${MKFS}/statfs ${ROOT}/$1 dir / 

