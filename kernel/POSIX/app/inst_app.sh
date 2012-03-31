#!/bin/sh
../mkfs/statfs ../mkfs/$1 write /test ../app/test
../mkfs/statfs ../mkfs/$1 write /psauxtst2 ../app/psauxtst2
../mkfs/statfs ../mkfs/$1 chmod 777 /test
../mkfs/statfs ../mkfs/$1 chmod 777 /psauxtst2
../mkfs/statfs ../mkfs/$1 dir / 
