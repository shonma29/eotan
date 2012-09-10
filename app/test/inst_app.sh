#!/bin/sh

MKFS=../mkfs
ROOT=../..

${MKFS}/statfs ${ROOT}/$1 write /test test
${MKFS}/statfs ${ROOT}/$1 write /psauxtst2 psauxtst2
${MKFS}/statfs ${ROOT}/$1 write /porttest porttest
${MKFS}/statfs ${ROOT}/$1 chmod 777 /test
${MKFS}/statfs ${ROOT}/$1 chmod 777 /psauxtst2
${MKFS}/statfs ${ROOT}/$1 chmod 777 /porttest
${MKFS}/statfs ${ROOT}/$1 dir / 
