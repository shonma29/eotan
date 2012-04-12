#!/bin/sh
# make posix fd image
# 
statfs ../../../../3rd_fd.img write /frtm frtm
statfs ../../../../3rd_fd.img chmod 777 /frtm
statfs ../../../../3rd_fd.img dir / 

