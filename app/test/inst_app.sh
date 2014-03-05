#!/bin/sh

MKFS=../mkfs
ROOT=../..

${MKFS}/statfs ${ROOT}/$1 write /bees.p6 bees.p6
${MKFS}/statfs ${ROOT}/$1 dir / 

